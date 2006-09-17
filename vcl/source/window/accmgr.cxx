/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accmgr.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:15:24 $
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
#include "precompiled_vcl.hxx"

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_ACCEL_H
#include <accel.h>
#endif
#ifndef _SV_ACCEL_HXX
#include <accel.hxx>
#endif
#ifndef _SV_ACCMGR_HXX
#include <accmgr.hxx>
#endif



// =======================================================================

DECLARE_LIST( ImplAccelList, Accelerator* )

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
    if ( !mpAccelList )
        mpAccelList = new ImplAccelList;
    else
    {
        // Gibts den schon ?
        if ( mpAccelList->GetPos( pAccel ) != LIST_ENTRY_NOTFOUND )
            return FALSE;
    }

    // Am Anfang der Liste einfuegen
    mpAccelList->Insert( pAccel, (ULONG)0 );

    return TRUE;
}

// -----------------------------------------------------------------------

void ImplAccelManager::RemoveAccel( Accelerator* pAccel )
{
    // Haben wir ueberhaupt eine Liste ?
    if ( !mpAccelList )
        return;

    // Raus damit
    mpAccelList->Remove( pAccel );
}

// -----------------------------------------------------------------------

void ImplAccelManager::EndSequence( BOOL bCancel )
{
    // Sind wir ueberhaupt in einer Sequenz ?
    if ( !mpSequenceList )
        return;

    // Alle Deactivate-Handler der Acceleratoren in der Sequenz rufen
    Accelerator* pTempAccel = mpSequenceList->First();
    while( pTempAccel )
    {
        BOOL bDel = FALSE;
        pTempAccel->mbIsCancel = bCancel;
        pTempAccel->mpDel = &bDel;
        pTempAccel->Deactivate();
        if ( !bDel )
        {
            pTempAccel->mbIsCancel = FALSE;
            pTempAccel->mpDel = NULL;
        }

        pTempAccel = mpSequenceList->Next();
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
    if ( !mpAccelList->Count() )
        return FALSE;

    // Sind wir in einer Sequenz ?
    if ( mpSequenceList )
    {
        pAccel = mpSequenceList->GetObject( 0 );
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

                mpSequenceList->Insert( pNextAccel, (ULONG)0 );

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
    pAccel = mpAccelList->First();
    while ( pAccel )
    {
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
                mpSequenceList->Insert( pAccel, (ULONG)0 );
                mpSequenceList->Insert( pNextAccel, (ULONG)0 );

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

        // Nicht gefunden, vielleicht im naechsten Accelerator
        pAccel = mpAccelList->Next();
    }

    return FALSE;
}
