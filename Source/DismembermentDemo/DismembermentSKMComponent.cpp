// Fill out your copyright notice in the Description page of Project Settings.



#include "DismembermentSKMComponent.h"
#include "NiagaraComponent.h"
#include "SkeletonDataAsset.h"
#include "Structs/LimbGroupData.h"


UDismembermentSKMComponent::UDismembermentSKMComponent()
{
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("Spawner",false);
	//NiagaraComponent->SetupAttachment(SkeletalMesh);
}

void UDismembermentSKMComponent::InitialiseBones()
{
	if(!SkeletonData)
		return;
	SetSkeletalMeshAsset(SkeletonData->SkeletalMesh);
	Limbs = SkeletonData->Limbs;
}

int UDismembermentSKMComponent::GetLimbIndexFromBoneName(FName Bone)
{
	//Get All Root Bones
	FName CurrentBone = Bone;
	TArray<FName> RootBones;
	for (FLimbGroupData LimbData : Limbs)
		RootBones.Add(LimbData.LimbRootName);
	
	do //Check if Hit Bone is Connected to a Limb
	{
		if(RootBones.Contains(CurrentBone))
		{
			UE_LOG(LogTemp,Display,TEXT("%s"),*CurrentBone.ToString());
			return RootBones.IndexOfByKey(CurrentBone);		
		}
		CurrentBone = GetParentBone(CurrentBone);
	} while(CurrentBone != "none");

	return -1;
}

void UDismembermentSKMComponent::Handle_LimbHit(FName HitBoneName, float Damage)
{
	int LimbIndex = GetLimbIndexFromBoneName(HitBoneName);
	if(LimbIndex == -1 || Limbs[LimbIndex].HasDetached)
		return;
	
	//Take Damage to the Limb & Check if its hit 0
	FMath::Clamp(Limbs[LimbIndex].LimbCurrentHealth -= Damage,0,1000);
	if(!Limbs[LimbIndex].LimbCurrentHealth == 0)
		return;
	
	
	
	//Detach Limb Forever OR Detach for Repair
	if(Limbs[LimbIndex].CurrentRepairs >= Limbs[LimbIndex].MaxRepairs)
	{
		HideBoneByName(Limbs[LimbIndex].LimbRootName,PBO_Term);
		Limbs[LimbIndex].HasDetached = true;
		UE_LOG(LogTemp,Error,TEXT("LIMB IS SEVERED"));
	}
	else
	{
		HideBoneByName(Limbs[LimbIndex].LimbRootName,PBO_None);
		FBoneTransform NewTransform;
		NewTransform.Transform.SetLocation(FVector{0,0,0});
	}

	
	//TODO Spawn Mesh, Particles, & Limb Phys Constraints
	SpawnParticlesAtLocation(GetBoneLocation(Limbs[LimbIndex].LimbRootName));
	UE_LOG(LogTemp,Warning,TEXT("%s"), *GetBoneLocation(Limbs[LimbIndex].LimbRootName).ToString());

}

void UDismembermentSKMComponent::Handle_LimbRepair(int LimbIndex)
{
	if(!Limbs.IsValidIndex(LimbIndex) || Limbs[LimbIndex].HasDetached)
		return;
	
	Limbs[LimbIndex].LimbCurrentHealth += 10;
	Limbs[LimbIndex].CurrentRepairs += 1;
	Limbs[LimbIndex].HasDetached = false;
	
	UnHideBoneByName(Limbs[LimbIndex].LimbRootName);
		
}

void UDismembermentSKMComponent::SpawnParticlesAtLocation(FVector RelativeToMesh)
{
	NiagaraComponent->SetAsset(SkeletonData->ParticleSystem,true);
	NiagaraComponent->SetRelativeLocation(RelativeToMesh);
}

