/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdlayer.hxx>
#include <svx/svdmodel.hxx> // fuer Broadcasting
#include "svx/svdglob.hxx"  // StringCache
#include "svx/svdstr.hrc"   // Namen aus der Resource

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayer

SdrLayer::SdrLayer(SdrLayerID nNewID, const String& rNewName, SdrModel& rSdrModel)
:   maName(rNewName),
    maTitle(),
    maDescription(),
    mrModel(rSdrModel),
    mnID(nNewID),
    mbStandardlayer(false)
{
}

void SdrLayer::SetStandardLayer(bool bStd)
{
    if(IsStandardLayer() != bStd)
    {
        mbStandardlayer = bStd;

        if(IsStandardLayer())
        {
            maName = ImpGetResStr(STR_StandardLayerName);
        }

        GetSdrModel().Broadcast(SdrBaseHint(HINT_LAYERCHG));
        GetSdrModel().SetChanged();
    }
}

void SdrLayer::SetName(const XubString& rNewName)
{
    if(rNewName != GetName())
    {
        maName = rNewName;
        mbStandardlayer = false; // Userdefined

        GetSdrModel().Broadcast(SdrBaseHint(HINT_LAYERCHG));
        GetSdrModel().SetChanged();
    }
}

bool SdrLayer::operator==(const SdrLayer& rCmpLayer) const
{
    return (GetID() == rCmpLayer.GetID()
        && IsStandardLayer() == rCmpLayer.IsStandardLayer()
        && GetName() == rCmpLayer.GetName());

    // title and description is not (yet?) part of this operator
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SdrLayerAdmin

SdrLayerAdmin::SdrLayerAdmin(SdrModel& rSdrModel, SdrLayerAdmin* pNewParent)
:   maLayer(),
    mpParent(pNewParent),
    mrModel(rSdrModel),
    maControlLayerName()
{
}

SdrLayerAdmin::~SdrLayerAdmin()
{
    ClearLayer();
}

SdrLayer* SdrLayerAdmin::GetLayer(sal_uInt32 i) const
{
    if(i < maLayer.size())
    {
        return *(maLayer.begin() + i);
    }
    else
    {
        OSL_ENSURE(false, "SdrLayerAdmin::GetLayer access out of range (!)");
        return 0;
    }
}

void SdrLayerAdmin::ClearLayer()
{
    for(SdrLayerContainerType::iterator aCandidate(maLayer.begin());
        aCandidate != maLayer.end(); aCandidate++)
    {
        delete *aCandidate;
    }

    maLayer.clear();
}

bool SdrLayerAdmin::operator==(const SdrLayerAdmin& rCmpLayerAdmin) const
{
    if(GetParent() != rCmpLayerAdmin.GetParent() || maLayer.size() != rCmpLayerAdmin.maLayer.size())
    {
        return false;
    }

    bool bEqual(true);

    for(sal_uInt32 a(0); bEqual && a < GetLayerCount(); a++)
    {
        bEqual = *GetLayer(a) == *rCmpLayerAdmin.GetLayer(a);
    }

    return bEqual;
}

void SdrLayerAdmin::Broadcast() const
{
    GetSdrModel().Broadcast(SdrBaseHint(HINT_LAYERORDERCHG));
    GetSdrModel().SetChanged();
}

void SdrLayerAdmin::InsertLayerFromUndoRedo(SdrLayer* pLayer, sal_uInt32 nPos)
{
    if(pLayer && &pLayer->GetSdrModel() == &GetSdrModel())
    {
        if(nPos >= maLayer.size())
        {
            maLayer.push_back(pLayer);
        }
        else
        {
            maLayer.insert(maLayer.begin() + nPos, pLayer);
        }

        Broadcast();
    }
    else
    {
        OSL_ENSURE(pLayer, "No SdrLayer given (!)");
        OSL_ENSURE(&pLayer->GetSdrModel() == &GetSdrModel(), "SdrLayer with alien SdrModel inserted (!)");
    }
}


SdrLayer* SdrLayerAdmin::RemoveLayer(sal_uInt32 nPos)
{
    SdrLayer* pRetval = 0;

    if(nPos < maLayer.size())
    {
        const SdrLayerContainerType::iterator aCandidate(maLayer.begin() + nPos);

        pRetval = *aCandidate;
        maLayer.erase(aCandidate);
    }
    else
    {
        OSL_ENSURE(false, "SdrLayerAdmin::RemoveLayer with wrong index (!)");
    }

    if(pRetval)
    {
        Broadcast();
    }

    return pRetval;
}

SdrLayer* SdrLayerAdmin::NewLayer(const XubString& rName, sal_uInt32 nPos)
{
    const SdrLayerID nID(GetUniqueLayerID());
    SdrLayer* pLayer = new SdrLayer(nID, rName, GetSdrModel());

    if(nPos >= maLayer.size())
    {
        maLayer.push_back(pLayer);
    }
    else
    {
        maLayer.insert(maLayer.begin() + nPos, pLayer);
    }

    Broadcast();

    return pLayer;
}

void SdrLayerAdmin::DeleteLayer(SdrLayer* pLayer)
{
    if(pLayer)
    {
        bool bFound(false);

        for(SdrLayerContainerType::iterator aCandidate(maLayer.begin());
            !bFound && aCandidate != maLayer.end(); aCandidate++)
        {
            if(*aCandidate == pLayer)
            {
                bFound = true;
                maLayer.erase(aCandidate);
            }
        }

        if(bFound)
        {
            delete pLayer;
            Broadcast();
        }
    }
}

SdrLayer* SdrLayerAdmin::NewStandardLayer(sal_uInt32 nPos)
{
    const SdrLayerID nID(GetUniqueLayerID());
    SdrLayer* pLayer = new SdrLayer(nID, String(), GetSdrModel());

    pLayer->SetStandardLayer();

    if(nPos >= maLayer.size())
    {
        maLayer.push_back(pLayer);
    }
    else
    {
        maLayer.insert(maLayer.begin() + nPos, pLayer);
    }

    Broadcast();

    return pLayer;
}

sal_uInt32 SdrLayerAdmin::GetLayerPos(SdrLayer* pLayer) const
{
    if(pLayer)
    {
        sal_uInt32 a(0);

        for(SdrLayerContainerType::const_iterator aCandidate(maLayer.begin());
            aCandidate != maLayer.end(); a++, aCandidate++)
        {
            if(*aCandidate == pLayer)
            {
                return a;
            }
        }
    }

    return SDRLAYER_NOTFOUND;
}

SdrLayer* SdrLayerAdmin::GetLayer(const XubString& rName, bool /*bInherited*/) const
{
    SdrLayer* pFound = 0;

    for(SdrLayerContainerType::const_iterator aCandidate(maLayer.begin());
        !pFound && aCandidate != maLayer.end(); aCandidate++)
    {
        if(rName == (*aCandidate)->GetName())
        {
            pFound = *aCandidate;
        }
    }

    if(!pFound && GetParent())
    {
        pFound = GetParent()->GetLayer(rName, true);
    }

    return pFound;
}

SdrLayerID SdrLayerAdmin::GetLayerID(const XubString& rName, bool bInherited) const
{
    const SdrLayer* pLayer = GetLayer(rName, bInherited);

    if(pLayer)
    {
        return pLayer->GetID();
    }
    else
    {
        return SDRLAYER_NOTFOUND;
    }
}

SdrLayer* SdrLayerAdmin::GetLayerPerID(SdrLayerID nID) const
{
    for(SdrLayerContainerType::const_iterator aCandidate(maLayer.begin());
        aCandidate != maLayer.end(); aCandidate++)
    {
        if(nID == (*aCandidate)->GetID())
        {
            return *aCandidate;
        }
    }

    return 0;
}

// Globale LayerID's beginnen mit 0 aufsteigend.
// Lokale LayerID's beginnen mit 254 absteigend.
// 255 ist reserviert fuer SDRLAYER_NOTFOUND

SdrLayerID SdrLayerAdmin::GetUniqueLayerID() const
{
    SetOfByte aSet;
    bool bDown(!GetParent());

    for(SdrLayerContainerType::const_iterator aCandidate(maLayer.begin());
        aCandidate != maLayer.end(); aCandidate++)
    {
        aSet.Set((*aCandidate)->GetID());
    }

    SdrLayerID i;

    if (!bDown)
    {
        i=254;

        while (i && aSet.IsSet(sal_uInt8(i)))
        {
            --i;
        }

        if(!i)
        {
            i=254;
        }
    }
    else
    {
        i=0;

        while (i<=254 && aSet.IsSet(sal_uInt8(i)))
        {
            i++;
        }

        if (i>254)
        {
            i=0;
        }
    }

    return i;
}

const String& SdrLayerAdmin::GetControlLayerName() const
{
    if(maControlLayerName.Len())
    {
        return maControlLayerName;
    }
    else
    {
        static const sal_Char aTextControls[] = "Controls";
        static const String aString(aTextControls, sizeof(aTextControls - 1));

        return aString;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
