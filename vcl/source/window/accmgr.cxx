/*************************************************************************
 *
 *  $RCSfile: accmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_ACCMGR_CXX

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

#pragma hdrstop

// =======================================================================

DECLARE_LIST( ImplAccelList, Accelerator* );

// =======================================================================

DBG_NAMEEX( Accelerator );

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
