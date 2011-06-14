/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "drawdoc.hxx"
#include "unoaprms.hxx"
#include "anminfo.hxx"


TYPEINIT1(SdAnimationPrmsUndoAction, SdUndoAction);


/*************************************************************************
|*
|* Undo()
|*
\************************************************************************/

void SdAnimationPrmsUndoAction::Undo()
{
    // keine neu Info erzeugt: Daten restaurieren
    if (!bInfoCreated)
    {
        SdDrawDocument* pDoc   = (SdDrawDocument*)pObject->GetModel();
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
    // Info wurde durch Aktion erzeugt: Info loeschen
    else
    {
        pObject->DeleteUserData(0);
    }
    // Damit ein ModelHasChanged() ausgeloest wird, um das Effekte-Window
    // auf Stand zu bringen (Animations-Reihenfolge)
    pObject->SetChanged();
    pObject->BroadcastObjectChange();
}

/*************************************************************************
|*
|* Redo()
|*
\************************************************************************/

void SdAnimationPrmsUndoAction::Redo()
{
    SdAnimationInfo* pInfo = NULL;

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

    // Damit ein ModelHasChanged() ausgeloest wird, um das Effekte-Window
    // auf Stand zu bringen (Animations-Reihenfolge)
    pObject->SetChanged();
    pObject->BroadcastObjectChange();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdAnimationPrmsUndoAction::~SdAnimationPrmsUndoAction()
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
