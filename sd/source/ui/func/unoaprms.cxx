/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoaprms.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:29:52 $
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
        bNewActive      = bOldActive     = pInfo->mbActive;
        eNewEffect      = eOldEffect     = pInfo->meEffect;
        eNewTextEffect  = eOldTextEffect = pInfo->meTextEffect;
        eNewSpeed       = eOldSpeed      = pInfo->meSpeed;
        bNewDimPrevious = bOldDimPrevious= pInfo->mbDimPrevious;
        aNewDimColor    = aOldDimColor   = pInfo->maDimColor;
        bNewDimHide     = bOldDimHide    = pInfo->mbDimHide;
        bNewSoundOn     = bOldSoundOn    = pInfo->mbSoundOn;
        aNewSoundFile   = aOldSoundFile  = pInfo->maSoundFile;
        bNewPlayFull    = bOldPlayFull   = pInfo->mbPlayFull;

        pNewPathObj     = pOldPathObj    = pInfo->mpPathObj;

        eNewClickAction     = eOldClickAction    = pInfo->meClickAction;
        aNewBookmark        = aOldBookmark       = pInfo->maBookmark;
//      bNewInvisibleInPres = bOldInvisibleInPres= pInfo->mbInvisibleInPresentation;
        nNewVerb            = nOldVerb           = pInfo->mnVerb;
        nNewPresOrder       = nOldPresOrder      = pInfo->mnPresOrder;

        eNewSecondEffect    = eOldSecondEffect   = pInfo->meSecondEffect;
        eNewSecondSpeed     = eOldSecondSpeed    = pInfo->meSecondSpeed;
        bNewSecondSoundOn   = bOldSecondSoundOn  = pInfo->mbSecondSoundOn;
        bNewSecondPlayFull  = bOldSecondPlayFull = pInfo->mbSecondPlayFull;
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
            pInfo->maBookmark    = aOldBookmark;
//          pInfo->mbInvisibleInPresentation = bOldInvisibleInPres;
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
//  pInfo->mSetPath(pNewPathObj);
    pInfo->meClickAction = eNewClickAction;
    pInfo->maBookmark    = aNewBookmark;
//  pInfo->mbInvisibleInPresentation = bNewInvisibleInPres;
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


