/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "drawdoc.hxx"
#include "unoaprms.hxx"
#include "anminfo.hxx"


void SdAnimationPrmsUndoAction::Undo()
{
    // no new info created: restore data
    if (!bInfoCreated)
    {
        SdDrawDocument* pDoc   = static_cast<SdDrawDocument*>(pObject->GetModel());
        if( pDoc )
        {
            SdAnimationInfo* pInfo = pDoc->GetAnimationInfo( pObject );

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
            pInfo->meClickAction = eOldClickAction;
            pInfo->SetBookmark( aOldBookmark );
            pInfo->mnVerb        = nOldVerb;
            pInfo->mnPresOrder   = nOldPresOrder;

            pInfo->meSecondEffect    = eOldSecondEffect;
            pInfo->meSecondSpeed     = eOldSecondSpeed;
            pInfo->mbSecondSoundOn   = bOldSecondSoundOn;
            pInfo->mbSecondPlayFull  = bOldSecondPlayFull;
        }
    }
    // info was created by action: delete info
    else
    {
        pObject->DeleteUserData(0);
    }
    // force ModelHasChanged() in order to update effect window (animation order)
    pObject->SetChanged();
    pObject->BroadcastObjectChange();
}

void SdAnimationPrmsUndoAction::Redo()
{
    SdAnimationInfo* pInfo = nullptr;

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
    pInfo->meClickAction = eNewClickAction;
    pInfo->SetBookmark( aNewBookmark );
    pInfo->mnVerb        = nNewVerb;
    pInfo->mnPresOrder   = nNewPresOrder;

    pInfo->meSecondEffect    = eNewSecondEffect;
    pInfo->meSecondSpeed     = eNewSecondSpeed;
    pInfo->mbSecondSoundOn   = bNewSecondSoundOn;
    pInfo->mbSecondPlayFull  = bNewSecondPlayFull;

    // force ModelHasChanged() in order to update effect window (animation order)
    pObject->SetChanged();
    pObject->BroadcastObjectChange();
}

SdAnimationPrmsUndoAction::~SdAnimationPrmsUndoAction()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
