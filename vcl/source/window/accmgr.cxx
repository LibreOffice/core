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
#include "precompiled_vcl.hxx"
#include <tools/list.hxx>
#include <tools/debug.hxx>

#include <accel.h>
#include <vcl/accel.hxx>
#include <accmgr.hxx>



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

sal_Bool ImplAccelManager::InsertAccel( Accelerator* pAccel )
{
    if ( !mpAccelList )
        mpAccelList = new ImplAccelList;
    else
    {
        // Gibts den schon ?
        if ( mpAccelList->GetPos( pAccel ) != LIST_ENTRY_NOTFOUND )
            return sal_False;
    }

    // Am Anfang der Liste einfuegen
    mpAccelList->Insert( pAccel, (sal_uLong)0 );

    return sal_True;
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
    if ( mpSequenceList )
    {
        for (sal_uInt16 i = 0; i < pAccel->GetItemCount(); ++i)
        {
            Accelerator* pSubAccel = pAccel->GetAccel(pAccel->GetItemId(i));
            if ( mpSequenceList->GetPos( pSubAccel ) != LIST_ENTRY_NOTFOUND )
            {
                EndSequence( true );
                break;
            }
        }
    }

    // Raus damit
    mpAccelList->Remove( pAccel );
}

// -----------------------------------------------------------------------

void ImplAccelManager::EndSequence( sal_Bool bCancel )
{
    // Sind wir ueberhaupt in einer Sequenz ?
    if ( !mpSequenceList )
        return;

    // Alle Deactivate-Handler der Acceleratoren in der Sequenz rufen
    Accelerator* pTempAccel = mpSequenceList->First();
    while( pTempAccel )
    {
        sal_Bool bDel = sal_False;
        pTempAccel->mbIsCancel = bCancel;
        pTempAccel->mpDel = &bDel;
        pTempAccel->Deactivate();
        if ( !bDel )
        {
            pTempAccel->mbIsCancel = sal_False;
            pTempAccel->mpDel = NULL;
        }

        pTempAccel = mpSequenceList->Next();
    }

    // Sequenz-Liste loeschen
    delete mpSequenceList;
    mpSequenceList = NULL;
}

// -----------------------------------------------------------------------

sal_Bool ImplAccelManager::IsAccelKey( const KeyCode& rKeyCode, sal_uInt16 nRepeat )
{
    Accelerator* pAccel;

    // Haben wir ueberhaupt Acceleratoren ??
    if ( !mpAccelList )
        return sal_False;
    if ( !mpAccelList->Count() )
        return sal_False;

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
            return sal_False;
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

                mpSequenceList->Insert( pNextAccel, (sal_uLong)0 );

                // Activate-Handler vom Neuen rufen
                pNextAccel->Activate();
                return sal_True;
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
                    sal_Bool bDel = sal_False;
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

                    return sal_True;
                }
                else
                {
                    // Sequenz abbrechen, weil Acceleraor disabled
                    // Taste wird weitergeleitet (ans System)
                    FlushAccel();
                    return sal_False;
                }
            }
        }
        else
        {
            // Sequenz abbrechen wegen falscher Taste
            FlushAccel();
            return sal_False;
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
                mpSequenceList->Insert( pAccel, (sal_uLong)0 );
                mpSequenceList->Insert( pNextAccel, (sal_uLong)0 );

                // Activate-Handler vom Neuen rufen
                pNextAccel->Activate();

                return sal_True;
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
                    sal_Bool bDel = sal_False;
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

                    return sal_True;
                }
                else
                    return sal_False;
            }
        }

        // Nicht gefunden, vielleicht im naechsten Accelerator
        pAccel = mpAccelList->Next();
    }

    return sal_False;
}
