/*************************************************************************
 *
 *  $RCSfile: fntszctl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headern

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _STDMENU_HXX //autogen
#include <svtools/stdmenu.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#pragma hdrstop

#define ITEMID_FONTHEIGHT   1
#define ITEMID_FONTLIST     2
#define ITEMID_FONT         3

#include "fntszctl.hxx"     // ITEMID_FONTHEIGHT muss vorher definiert sein!
#include "dialogs.hrc"

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include "flstitem.hxx"
#endif

#include "dlgutil.hxx"
#include "dialmgr.hxx"

#define LOGIC   OutputDevice::LogicToLogic

SFX_IMPL_MENU_CONTROL(SvxFontSizeMenuControl, SvxFontHeightItem);

//--------------------------------------------------------------------

/*  [Beschreibung]

    Select-Handler des Men"us; die aktuelle Fontgr"o\se
    wird in einem SvxFontHeightItem verschickt.
*/

IMPL_LINK( SvxFontSizeMenuControl, MenuSelect, FontSizeMenu*, pMen )
{
    SfxViewFrame* pFrm = SfxViewFrame::Current();
    SfxShell* pSh = pFrm ? pFrm->GetDispatcher()->GetShell( 0 ) : NULL;

    if ( !pSh )
        return 0;

    const SfxItemPool& rPool = pSh->GetPool();
    USHORT nWhich = rPool.GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    const SfxMapUnit eUnit = rPool.GetMetric( nWhich );
    long nH = LOGIC( pMen->GetCurHeight(), MAP_POINT, (MapUnit)eUnit ) / 10;
    SvxFontHeightItem aItem( nH, 100, GetId() );
    GetBindings().GetDispatcher()->Execute( GetId(), SFX_CALLMODE_RECORD, &aItem, 0L );
    return 1;
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Statusbenachrichtigung;
    Ist die Funktionalit"at disabled, wird der entsprechende
    Men"ueintrag im Parentmenu disabled, andernfalls wird er enabled.
    die aktuelle Fontgr"o\se wird mit einer Checkmark versehen.
*/

void SvxFontSizeMenuControl::StateChanged(

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    rParent.EnableItem( GetId(), SFX_ITEM_DISABLED != eState );

    if ( SFX_ITEM_AVAILABLE == eState )
    {
        if ( pState->ISA(SvxFontHeightItem) )
        {
            const SvxFontHeightItem* pItem =
                PTR_CAST( SvxFontHeightItem, pState );
            long nVal = 0;

            if ( pItem )
            {
                SfxViewFrame* pFrm = SfxViewFrame::Current();
                SfxShell* pSh = pFrm ? pFrm->GetDispatcher()->GetShell( 0 )
                                     : NULL;

                if ( !pSh )
                    return;

                const SfxItemPool& rPool = pSh->GetPool();
                USHORT nWhich = rPool.GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
                const SfxMapUnit eUnit = rPool.GetMetric( nWhich );
                long nH = pItem->GetHeight();
                nVal = LOGIC( nH, (MapUnit)eUnit, MAP_POINT ) * 10;
            }
            pMenu->SetCurHeight( nVal );
        }
        else if ( pState->ISA(SvxFontItem) )
        {
            const SvxFontItem* pItem = PTR_CAST( SvxFontItem, pState );

            if ( pItem )
            {
                SfxObjectShell *pDoc = SfxObjectShell::Current();

                if ( pDoc )
                {
                    const SvxFontListItem* pFonts = (const SvxFontListItem*)
                        pDoc->GetItem( SID_ATTR_CHAR_FONTLIST );
                    const FontList* pList = pFonts ? pFonts->GetFontList(): 0;

                    if ( pList )
                    {
                        FontInfo aFntInf = pList->Get( pItem->GetFamilyName(),
                                                       pItem->GetStyleName() );
                        pMenu->Fill( aFntInf, pList );
                    }
                    // else manche Shells haben keine Fontliste (z.B. Image)
                }
            }
        }
    }
    else
    {
        // irgendwie muss man ja das Men"u f"ullen
        SfxObjectShell* pSh = SfxObjectShell::Current();

        if ( pSh )
        {
            // daf"ur von der Shell eine Fontliste besorgen
            const SvxFontListItem* pFonts =
                (const SvxFontListItem*)pSh->GetItem( SID_ATTR_CHAR_FONTLIST );
            const FontList* pList = pFonts ? pFonts->GetFontList(): NULL;
            if ( pList )
#if SUPD < 547
                pMenu->Fill( pList->Get(0), pList );
#else
                pMenu->Fill( pList->GetFontName(0), pList );
#endif
        }
    }
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Ctor; setzt den Select-Handler am Men"u und tr"agt Men"u
    in seinen Parent ein.
*/

SvxFontSizeMenuControl::SvxFontSizeMenuControl
(
    USHORT          nId,
    Menu&           rMenu,
    SfxBindings&    rBindings
) :
    SfxMenuControl( nId, rBindings ),

    pMenu   ( new FontSizeMenu ),
    rParent ( rMenu ),
    aFontNameForwarder( SID_ATTR_CHAR_FONT, *this )

{
    rMenu.SetPopupMenu( nId, pMenu );
    pMenu->SetSelectHdl( LINK( this, SvxFontSizeMenuControl, MenuSelect ) );
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Dtor; gibt das Men"u frei.
*/

SvxFontSizeMenuControl::~SvxFontSizeMenuControl()
{
    delete pMenu;
}

//--------------------------------------------------------------------

/*  [Beschreibung]

    Gibt das Men"u zur"uck
*/

PopupMenu* SvxFontSizeMenuControl::GetPopup() const
{
    return pMenu;
}


