/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoaprms.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:00:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
|* 2. Ctor, der den ersten (inline) nach der Version 4.0 einmal ersetzen
|* soll (mit 3. Parameter dann)
|* Hier werden die Member mit den Animations-Informationen vorbelegt,
|* um nicht immer alle inline-Methoden aufrufen zu muessen, auch im
|* Hinblick auf zukuenftige Erweiterungen (neue Member etc.)
|*
\************************************************************************/

SdAnimationPrmsUndoAction::SdAnimationPrmsUndoAction(
                                SdDrawDocument* pTheDoc,
                                SdrObject* pObj ) :
    SdUndoAction    ( pTheDoc ),
    pObject         ( pObj ),
    bInfoCreated    ( FALSE ) // Fuer Animationsreihenfolge existiert Info
{
    SdAnimationInfo* pInfo = pTheDoc->GetAnimationInfo( pObject );
    if( pInfo )
    {
        bNewActive      = bOldActive     = pInfo->bActive;
        eNewEffect      = eOldEffect     = pInfo->eEffect;
        eNewTextEffect  = eOldTextEffect = pInfo->eTextEffect;
        eNewSpeed       = eOldSpeed      = pInfo->eSpeed;
        bNewDimPrevious = bOldDimPrevious= pInfo->bDimPrevious;
        aNewDimColor    = aOldDimColor   = pInfo->aDimColor;
        bNewDimHide     = bOldDimHide    = pInfo->bDimHide;
        bNewSoundOn     = bOldSoundOn    = pInfo->bSoundOn;
        aNewSoundFile   = aOldSoundFile  = pInfo->aSoundFile;
        bNewPlayFull    = bOldPlayFull   = pInfo->bPlayFull;

        pNewPathObj     = pOldPathObj    = pInfo->pPathObj;

        eNewClickAction     = eOldClickAction    = pInfo->eClickAction;
        aNewBookmark        = aOldBookmark       = pInfo->aBookmark;
//      bNewInvisibleInPres = bOldInvisibleInPres= pInfo->bInvisibleInPresentation;
        nNewVerb            = nOldVerb           = pInfo->nVerb;
        nNewPresOrder       = nOldPresOrder      = pInfo->nPresOrder;

        eNewSecondEffect    = eOldSecondEffect   = pInfo->eSecondEffect;
        eNewSecondSpeed     = eOldSecondSpeed    = pInfo->eSecondSpeed;
        bNewSecondSoundOn   = bOldSecondSoundOn  = pInfo->bSecondSoundOn;
        bNewSecondPlayFull  = bOldSecondPlayFull = pInfo->bSecondPlayFull;
    }
}

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
            // So nicht...
            //SdAnimationInfo* pInfo = (SdAnimationInfo*)pObject->GetUserData(0);
            pInfo->bActive      = bOldActive;
            pInfo->eEffect      = eOldEffect;
            pInfo->eTextEffect  = eOldTextEffect;
            pInfo->eSpeed       = eOldSpeed;
            pInfo->bDimPrevious = bOldDimPrevious;
            pInfo->aDimColor    = aOldDimColor;
            pInfo->bDimHide     = bOldDimHide;
            pInfo->bSoundOn     = bOldSoundOn;
            pInfo->aSoundFile   = aOldSoundFile;
            pInfo->bPlayFull    = bOldPlayFull;
//          pInfo->SetPath(pOldPathObj);
            pInfo->eClickAction = eOldClickAction;
            pInfo->aBookmark    = aOldBookmark;
//          pInfo->bInvisibleInPresentation = bOldInvisibleInPres;
            pInfo->nVerb        = nOldVerb;
            pInfo->nPresOrder   = nOldPresOrder;

            pInfo->eSecondEffect    = eOldSecondEffect;
            pInfo->eSecondSpeed     = eOldSecondSpeed;
            pInfo->bSecondSoundOn   = bOldSecondSoundOn;
            pInfo->bSecondPlayFull  = bOldSecondPlayFull;
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

    pInfo->bActive      = bNewActive;
    pInfo->eEffect      = eNewEffect;
    pInfo->eTextEffect  = eNewTextEffect;
    pInfo->eSpeed       = eNewSpeed;
    pInfo->bDimPrevious = bNewDimPrevious;
    pInfo->aDimColor    = aNewDimColor;
    pInfo->bDimHide     = bNewDimHide;
    pInfo->bSoundOn     = bNewSoundOn;
    pInfo->aSoundFile   = aNewSoundFile;
    pInfo->bPlayFull    = bNewPlayFull;
//  pInfo->SetPath(pNewPathObj);
    pInfo->eClickAction = eNewClickAction;
    pInfo->aBookmark    = aNewBookmark;
//  pInfo->bInvisibleInPresentation = bNewInvisibleInPres;
    pInfo->nVerb        = nNewVerb;
    pInfo->nPresOrder   = nNewPresOrder;

    pInfo->eSecondEffect    = eNewSecondEffect;
    pInfo->eSecondSpeed     = eNewSecondSpeed;
    pInfo->bSecondSoundOn   = bNewSecondSoundOn;
    pInfo->bSecondPlayFull  = bNewSecondPlayFull;

    // Damit ein ModelHasChanged() ausgeloest wird, um das Effekte-Window
    // auf Stand zu bringen (Animations-Reihenfolge)
    pObject->SetChanged();
    pObject->BroadcastObjectChange();
}

/*************************************************************************
|*
|* Repeat()
|*
\************************************************************************/

void SdAnimationPrmsUndoAction::Repeat()
{
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdAnimationPrmsUndoAction::~SdAnimationPrmsUndoAction()
{
}


