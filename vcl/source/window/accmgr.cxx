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
#include "precompiled_vcl.hxx"

#include <tools/debug.hxx>
#include <vcl/accel.h>
#include <vcl/accel.hxx>
#include <vcl/accmgr.hxx>

// =======================================================================

DBG_NAMEEX( Accelerator )

// =======================================================================

ImplAccelManager::~ImplAccelManager()
{
    if ( mpAccelList )
        delete mpAccelList;
    if ( mpSequenceList )
        delete mpSequenceList;
}

// -----------------------------------------------------------------------

BOOL ImplAccelManager::InsertAccel( Accelerator* pAccel )
{
    if ( !mpAccelList ) {
        mpAccelList = new ImplAccelList;
    } else {
        for ( size_t i = 0, n = mpAccelList->size(); i < n; ++i ) {
            if ( (*mpAccelList)[ i ] == pAccel ) {
                return FALSE;
            }
        }
    }

    mpAccelList->insert( mpAccelList->begin(), pAccel );
    return TRUE;
}

// -----------------------------------------------------------------------

void ImplAccelManager::RemoveAccel( Accelerator* pAccel )
{
    // Haben wir ueberhaupt eine Liste ?
    if ( !mpAccelList )
        return;

    //e.g. #i90599#. Someone starts typing a sequence in a dialog, but doesn't
    //end it, and then closes the dialog, deleting the accelerators. So if
    //we're removing an accelerator that a sub-accelerator which is in the
    //sequence list, throw away the entire sequence
    if ( mpSequenceList ) {
        for (USHORT i = 0; i < pAccel->GetItemCount(); ++i) {
            Accelerator* pSubAccel = pAccel->GetAccel( pAccel->GetItemId(i) );
            for ( size_t j = 0, n = mpSequenceList->size(); j < n; ++j ) {
                if ( (*mpSequenceList)[ j ] == pSubAccel ) {
                    EndSequence( true );
                    i = pAccel->GetItemCount();
                    break;
                }
            }
        }
    }

    // Raus damit
    for ( ImplAccelList::iterator it = mpAccelList->begin();
          it < mpAccelList->end();
          ++it
    ) {
        if ( *it == pAccel ) {
            mpAccelList->erase( it );
            break;
        }
    }
}

// -----------------------------------------------------------------------

void ImplAccelManager::EndSequence( BOOL bCancel )
{
    // Sind wir ueberhaupt in einer Sequenz ?
    if ( !mpSequenceList )
        return;

    // Alle Deactivate-Handler der Acceleratoren in der Sequenz rufen
    for ( size_t i = 0, n = mpSequenceList->size(); i < n; ++i )
    {
        Accelerator* pTempAccel = (*mpSequenceList)[ i ];
        BOOL bDel = FALSE;
        pTempAccel->mbIsCancel = bCancel;
        pTempAccel->mpDel = &bDel;
        pTempAccel->Deactivate();
        if ( !bDel )
        {
            pTempAccel->mbIsCancel = FALSE;
            pTempAccel->mpDel = NULL;
        }
    }

    // Sequenz-Liste loeschen
    delete mpSequenceList;
    mpSequenceList = NULL;
}

// -----------------------------------------------------------------------

BOOL ImplAccelManager::IsAccelKey( const KeyCode& rKeyCode, USHORT nRepeat )
{
    Accelerator* pAccel;

    // Haben wir ueberhaupt Acceleratoren ??
    if ( !mpAccelList )
        return FALSE;
    if ( mpAccelList->empty() )
        return FALSE;

    // Sind wir in einer Sequenz ?
    if ( mpSequenceList )
    {
        pAccel = mpSequenceList->empty() ? NULL : (*mpSequenceList)[ 0 ];
        DBG_CHKOBJ( pAccel, Accelerator, NULL );

        // Nicht Gefunden ?
        if ( !pAccel )
        {
            // Sequenz abbrechen
            FlushAccel();
            return FALSE;
        }

        // Ist der Eintrag da drin ?
        ImplAccelEntry* pEntry = pAccel->ImplGetAccelData( rKeyCode );
        if ( pEntry )
        {
            Accelerator* pNextAccel = pEntry->mpAccel;

            // Ist da ein Accelerator hinter ?
            if ( pNextAccel )
            {
                DBG_CHKOBJ( pNextAccel, Accelerator, NULL );

                mpSequenceList->insert( mpSequenceList->begin(), pNextAccel );

                // Activate-Handler vom Neuen rufen
                pNextAccel->Activate();
                return TRUE;
            }
            else
            {
                // Hat ihn schon !
                if ( pEntry->mbEnabled )
                {
                    // Sequence beenden (Deactivate-Handler vorher rufen)
                    EndSequence();

                    // Dem Accelerator das aktuelle Item setzen
                    // und Handler rufen
                    BOOL bDel = FALSE;
                    pAccel->maCurKeyCode    = rKeyCode;
                    pAccel->mnCurId         = pEntry->mnId;
                    pAccel->mnCurRepeat     = nRepeat;
                    pAccel->mpDel           = &bDel;
                    pAccel->Select();

                    // Hat Accel den Aufruf ueberlebt
                    if ( !bDel )
                    {
                        DBG_CHKOBJ( pAccel, Accelerator, NULL );
                        pAccel->maCurKeyCode    = KeyCode();
                        pAccel->mnCurId         = 0;
                        pAccel->mnCurRepeat     = 0;
                        pAccel->mpDel           = NULL;
                    }

                    return TRUE;
                }
                else
                {
                    // Sequenz abbrechen, weil Acceleraor disabled
                    // Taste wird weitergeleitet (ans System)
                    FlushAccel();
                    return FALSE;
                }
            }
        }
        else
        {
            // Sequenz abbrechen wegen falscher Taste
            FlushAccel();
            return FALSE;
        }
    }

    // Durch die Liste der Acceleratoren wuehlen
    for ( size_t i = 0, n = mpAccelList->size(); i < n; ++i )
    {
        pAccel = (*mpAccelList)[ i ];
        DBG_CHKOBJ( pAccel, Accelerator, NULL );

        // Ist der Eintrag da drin ?
        ImplAccelEntry* pEntry = pAccel->ImplGetAccelData( rKeyCode );
        if ( pEntry )
        {
            Accelerator* pNextAccel = pEntry->mpAccel;

            // Ist da ein Accelerator hinter ?
            if ( pNextAccel )
            {
                DBG_CHKOBJ( pNextAccel, Accelerator, NULL );

                // Sequenz-Liste erzeugen
                mpSequenceList = new ImplAccelList;
                mpSequenceList->insert( mpSequenceList->begin(), pAccel     );
                mpSequenceList->insert( mpSequenceList->begin(), pNextAccel );

                // Activate-Handler vom Neuen rufen
                pNextAccel->Activate();

                return TRUE;
            }
            else
            {
                // Hat ihn schon !
                if ( pEntry->mbEnabled )
                {
                    // Activate/Deactivate-Handler vorher rufen
                    pAccel->Activate();
                    pAccel->Deactivate();

                    // Dem Accelerator das aktuelle Item setzen
                    // und Handler rufen
                    BOOL bDel = FALSE;
                    pAccel->maCurKeyCode    = rKeyCode;
                    pAccel->mnCurId         = pEntry->mnId;
                    pAccel->mnCurRepeat     = nRepeat;
                    pAccel->mpDel           = &bDel;
                    pAccel->Select();

                    // Hat Accel den Aufruf ueberlebt
                    if ( !bDel )
                    {
                        DBG_CHKOBJ( pAccel, Accelerator, NULL );
                        pAccel->maCurKeyCode    = KeyCode();
                        pAccel->mnCurId         = 0;
                        pAccel->mnCurRepeat     = 0;
                        pAccel->mpDel           = NULL;
                    }

                    return TRUE;
                }
                else
                    return FALSE;
            }
        }
    }

    return FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
