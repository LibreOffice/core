/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fntszctl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:39:02 $
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
#include "precompiled_svx.hxx"

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

#include <svx/fntszctl.hxx>     //
#include <svx/dialogs.hrc>

#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif
#ifndef _SVX_FLSTITEM_HXX //autogen
#include "flstitem.hxx"
#endif

#include "dlgutil.hxx"
#include <svx/dialmgr.hxx>

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

    USHORT, SfxItemState eState, const SfxPoolItem* pState )

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
                long nH = pItem->GetHeight() * 10;
                nVal = LOGIC( nH, (MapUnit)eUnit, MAP_POINT );
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
                pMenu->Fill( pList->GetFontName(0), pList );
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
    USHORT          _nId,
    Menu&           rMenu,
    SfxBindings&    rBindings
) :
    SfxMenuControl( _nId, rBindings ),

    pMenu   ( new FontSizeMenu ),
    rParent ( rMenu ),
    aFontNameForwarder( SID_ATTR_CHAR_FONT, *this )

{
    rMenu.SetPopupMenu( _nId, pMenu );
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


