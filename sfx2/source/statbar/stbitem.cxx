/*************************************************************************
 *
 *  $RCSfile: stbitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:35 $
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

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#pragma hdrstop

#include "stbitem.hxx"
#include "sfxtypes.hxx"
#include "stbmgr.hxx"
#include "msg.hxx"
#include "arrdecl.hxx"
#include "bindings.hxx"
#include "msgdescr.hxx"
#include "msgpool.hxx"
#include "module.hxx"
#include "dispatch.hxx"

#include "sfx.hrc"

//--------------------------------------------------------------------

SfxStatusBarControl::SfxStatusBarControl
(
    USHORT      nId,                /* Slot-Id, mit der diese Instanz
                                       verbunden wird. Wurde bei der
                                       Registrierung eine Slot-Id != 0
                                       angegeben, ist dies immer die dort
                                       angegebene. */

    StatusBar&  rBar,               /* Referenz auf die StatusBar, f"ur die
                                       dieses Control erzeugt wurde. */
    SfxBindings& rBindings          // Die Bindings dieses Controls
)

/*  [Beschreibung]

    Konstruktor der Klasse SfxStatusBarControl. Die Subclasses werden
    bei Bedarf per Factory vom SFx erzeugt.

    Instanzen dieser Basisklasse werden f"ur alle StatusBar-Felder
    erzeugt, f"ur die keine speziellen registriert wurden.
*/

:   SfxControllerItem(nId, rBindings),
    pBar(&rBar)
{
    DBG( CheckConfigure_Impl(SFX_SLOT_STATUSBARCONFIG) );
}

//--------------------------------------------------------------------

SfxStatusBarControl::~SfxStatusBarControl()

/*  [Beschreibung]

    Destruktor der Klasse SfxStatusBarControl. Die Instanzen dieser
    Klasse und deren Subklassen werden vom SFx zerst"ort.
*/

{}

//--------------------------------------------------------------------

void SfxStatusBarControl::StateChanged
(
    USHORT              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState  /* Zeiger auf ein SfxPoolItem, welches nur
                                   innerhalb dieses Methodenaufrufs g"ultig
                                   ist. Es kann ein 0-Pointer, ein Pointer
                                   auf ein SfxVoidItem oder auf den Typ, f"ur
                                   den die Subclass von SfxStatusBarControl
                                   registriert ist vorkommen. */
)

/*  [Beschreibung]

    Die Basisimplementation versteht Items vom Type SfxStringItem, bei
    denen der Text in das Status-Zeilen-Feld eingetragen wird und
    SfxVoidItem, bei denen das Feld geleert wird. Die Basisimplementierng
    sollte in "uberladenen Methoden nicht gerufen werden.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pBar != 0, "setting state to dangling StatusBar" );

    if ( eState == SFX_ITEM_AVAILABLE && pState->ISA(SfxStringItem) )
        pBar->SetItemText( nSID,
                    PTR_CAST(SfxStringItem,pState)->GetValue() );
    else
    {
        DBG_ASSERT( eState != SFX_ITEM_AVAILABLE|| pState->ISA(SfxVoidItem),
                    "wrong SfxPoolItem subclass in SfxStatusBarControl" );
        pBar->SetItemText( nSID, String() );
    }
}

//--------------------------------------------------------------------

BOOL SfxStatusBarControl::MouseButtonDown( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseButtonDown() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [RÅckgabewert]

    BOOL                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return FALSE;
}

//--------------------------------------------------------------------

BOOL SfxStatusBarControl::MouseMove( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseMove() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [RÅckgabewert]

    BOOL                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return FALSE;
}

//--------------------------------------------------------------------

BOOL SfxStatusBarControl::MouseButtonUp( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseButtonUp() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [RÅckgabewert]

    BOOL                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return FALSE;
}

//--------------------------------------------------------------------

void SfxStatusBarControl::Command( const CommandEvent &rEvt )

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, wenn f"ur dieses SfxStatusBarControl
    ein CommandEvent f"ur erkannt wurde.

    Die Defaultimplementierung ist leer.
*/

{
}

//--------------------------------------------------------------------

void SfxStatusBarControl::Click()

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, wenn der Anwender mit der Maus
    in das zu diesem Control geh"orige Feld der Statuszeile klickt.

    Die Defaultimplementierung ist leer.
*/

{
}

//--------------------------------------------------------------------

void SfxStatusBarControl::DoubleClick()

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, wenn der Anwender mit der Maus
    in das zu diesem Control geh"orige Feld der Statuszeile doppel-klickt.

    Die Defaultimplementierung ist leer.
*/

{
    GetBindings().Execute( GetId() );
}

//--------------------------------------------------------------------

void SfxStatusBarControl::Paint
(
    const UserDrawEvent& rUDEvt  /* Referenz auf einen UserDrawEvent */
)

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, falls das betreffende Feld
    mit SIB_USERDRAW gekennzeichnet ist, um den Inhalt zu zeichnen.
    Die Ausgabe mu"s auf dem in durch rUDEvt.GetDevice() erh"altlichen
    OutputDevice innerhalb des durch rUDEvt.GetRect() angegebenenen
    Rechtecks erfolgen.

    Die Defaultimplementierung ist leer.
*/

{
}

//--------------------------------------------------------------------

SfxStatusBarControl* SfxStatusBarControl::CreateControl
(
    USHORT          nId,
    StatusBar*      pBar,
    SfxBindings&    rBindings,
    SfxModule*      pMod
)
{
    SfxApplication *pApp = SFX_APP();
    SfxSlotPool *pSlotPool;
    if ( pMod )
        pSlotPool = pMod->GetSlotPool();
    else
        pSlotPool = &pApp->GetSlotPool();
    TypeId aSlotType = pSlotPool->GetSlotType(nId);
    if ( aSlotType )
    {
        SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
        if ( pMod )
        {
            SfxStbCtrlFactArr_Impl *pFactories = pMod->GetStbCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxStbCtrlFactArr_Impl &rFactories = *pFactories;
                for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                if ( rFactories[nFactory]->nTypeId == aSlotType &&
                     ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                     ( rFactories[nFactory]->nSlotId == nId) ) )
                    return rFactories[nFactory]->pCtor( nId, *pBar, rBindings );
            }
        }

        SfxStbCtrlFactArr_Impl &rFactories = pApp->GetStbCtrlFactories_Impl();
        for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
        if ( rFactories[nFactory]->nTypeId == aSlotType &&
             ( ( rFactories[nFactory]->nSlotId == 0 ) ||
             ( rFactories[nFactory]->nSlotId == nId) ) )
            return rFactories[nFactory]->pCtor( nId, *pBar, rBindings );
    }
    return new SfxStatusBarControl( nId, *pBar, rBindings );
}
                                                                                                                        //--------------------------------------------------------------------

