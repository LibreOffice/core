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
#include "precompiled_sd.hxx"


#include "drawdoc.hxx"
#include "unoaprms.hxx"
#include "anminfo.hxx"


/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void SdAnimationPrmsUndoAction::Undo()
{
    OSL_ENSURE(pObject, "SdAnimationPrmsUndoAction with no pObject (!)");
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObject);

    // keine neu Info erzeugt: Daten restaurieren
    if (!bInfoCreated)
    {
        SdDrawDocument& rDoc = static_cast< SdDrawDocument& >(pObject->getSdrModelFromSdrObject());
        SdAnimationInfo* pInfo = rDoc.GetAnimationInfo( pObject );
        // So nicht...
        //SdAnimationInfo* pInfo = (SdAnimationInfo*)pObject->GetUserData(0);
        pInfo->mbActive     = bOldActive;
        pInfo->meEffect      = eOldEffect;
        pInfo->meTextEffect  = eOldTextEffect;
        pInfo->meSpeed      = eOldSpeed;
        pInfo->mbDimPrevious = bOldDimPrevious;
        pInfo->maDimColor    = aOldDimColor;
        pInfo->mbDimHide     = bOldDimHide;
        pInfo->mbSoundOn     = bOldSoundOn;
        pInfo->maSoundFile   = aOldSoundFile;
        pInfo->mbPlayFull    = bOldPlayFull;
//          pInfo->mSetPath(pOldPathObj);
        pInfo->meClickAction = eOldClickAction;
        pInfo->SetBookmark( aOldBookmark );
//          pInfo->mbInvisibleInPresentation = bOldInvisibleInPres;
        pInfo->mnVerb        = nOldVerb;
        pInfo->mnPresOrder   = nOldPresOrder;

        pInfo->meSecondEffect    = eOldSecondEffect;
        pInfo->meSecondSpeed     = eOldSecondSpeed;
        pInfo->mbSecondSoundOn   = bOldSecondSoundOn;
        pInfo->mbSecondPlayFull  = bOldSecondPlayFull;
    }
    // Info wurde durch Aktion erzeugt: Info loeschen
    else
    {
        pObject->DeleteUserData(0);
    }

    // Damit ein LazyReactOnObjectChanges() ausgeloest wird, um das Effekte-Window
    // auf Stand zu bringen (Animations-Reihenfolge)
    pObject->SetChanged();
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void SdAnimationPrmsUndoAction::Redo()
{
    SdAnimationInfo* pInfo = NULL;
    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pObject);

    pInfo = SdDrawDocument::GetShapeUserData(*pObject,true);

    pInfo->mbActive      = bNewActive;
    pInfo->meEffect      = eNewEffect;
    pInfo->meTextEffect  = eNewTextEffect;
    pInfo->meSpeed       = eNewSpeed;
    pInfo->mbDimPrevious = bNewDimPrevious;
    pInfo->maDimColor    = aNewDimColor;
    pInfo->mbDimHide     = bNewDimHide;
    pInfo->mbSoundOn     = bNewSoundOn;
    pInfo->maSoundFile   = aNewSoundFile;
    pInfo->mbPlayFull    = bNewPlayFull;
//  pInfo->mSetPath(pNewPathObj);
    pInfo->meClickAction = eNewClickAction;
    pInfo->SetBookmark( aNewBookmark );
//  pInfo->mbInvisibleInPresentation = bNewInvisibleInPres;
    pInfo->mnVerb        = nNewVerb;
    pInfo->mnPresOrder   = nNewPresOrder;

    pInfo->meSecondEffect    = eNewSecondEffect;
    pInfo->meSecondSpeed     = eNewSecondSpeed;
    pInfo->mbSecondSoundOn   = bNewSecondSoundOn;
    pInfo->mbSecondPlayFull  = bNewSecondPlayFull;

    // Damit ein LazyReactOnObjectChanges() ausgeloest wird, um das Effekte-Window
    // auf Stand zu bringen (Animations-Reihenfolge)
    pObject->SetChanged();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdAnimationPrmsUndoAction::~SdAnimationPrmsUndoAction()
{
}


