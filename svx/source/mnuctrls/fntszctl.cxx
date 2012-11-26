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
#include "precompiled_svx.hxx"

#include <string> // HACK: prevent conflict between STLPORT and Workshop headern
#include <svl/itempool.hxx>
#include <svtools/stdmenu.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/fntszctl.hxx>     //
#include <svx/dialogs.hrc>
#include <editeng/fhgtitem.hxx>
#include "editeng/fontitem.hxx"
#include "editeng/flstitem.hxx"

#include "svx/dlgutil.hxx"
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
    sal_uInt16 nWhich = rPool.GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
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

    sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    rParent.EnableItem( GetId(), SFX_ITEM_DISABLED != eState );

    if ( SFX_ITEM_AVAILABLE == eState )
    {
        if ( dynamic_cast< const SvxFontHeightItem* >(pState) )
        {
            const SvxFontHeightItem* pItem = dynamic_cast< const SvxFontHeightItem* >( pState );
            long nVal = 0;

            if ( pItem )
            {
                SfxViewFrame* pFrm = SfxViewFrame::Current();
                SfxShell* pSh = pFrm ? pFrm->GetDispatcher()->GetShell( 0 )
                                     : NULL;

                if ( !pSh )
                    return;

                const SfxItemPool& rPool = pSh->GetPool();
                sal_uInt16 nWhich = rPool.GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
                const SfxMapUnit eUnit = rPool.GetMetric( nWhich );
                long nH = pItem->GetHeight() * 10;
                nVal = LOGIC( nH, (MapUnit)eUnit, MAP_POINT );
            }
            pMenu->SetCurHeight( nVal );
        }
        else if ( dynamic_cast< const SvxFontItem* >(pState) )
        {
            const SvxFontItem* pItem = dynamic_cast< const SvxFontItem* >( pState );

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
    sal_uInt16          _nId,
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


