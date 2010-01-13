/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LayerDialogContent.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_sd.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/text/WritingMode.hpp>

#include <vcl/image.hxx>

#include <sfx2/dockwin.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <sfx2/tbxctrl.hxx>

#include <svl/languageoptions.hxx>

#include <svtools/valueset.hxx>

#include <svx/toolbarmenu.hxx>

#include "app.hrc"
#include "layoutdialog.hxx"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "View.hxx"
#include "drawdoc.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

namespace sd {

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------

struct snewfoil_value_info
{
    USHORT mnBmpResId;
    USHORT mnHCBmpResId;
    USHORT mnStrResId;
    WritingMode meWritingMode;
    AutoLayout maAutoLayout;
};
static snewfoil_value_info standard[] =
{
    {BMP_LAYOUT_EMPTY_57X71, BMP_LAYOUT_EMPTY_57X71_H, STR_AUTOLAYOUT_NONE, WritingMode_LR_TB,        AUTOLAYOUT_NONE},
    {BMP_LAYOUT_HEAD03_57X71, BMP_LAYOUT_HEAD03_57X71_H, STR_AUTOLAYOUT_TITLE, WritingMode_LR_TB,       AUTOLAYOUT_TITLE},
    {BMP_LAYOUT_HEAD02_57X71, BMP_LAYOUT_HEAD02_57X71_H, STR_AUTOLAYOUT_CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_ENUM},
    {BMP_LAYOUT_HEAD02A_57X71, BMP_LAYOUT_HEAD02A_57X71_H, STR_AUTOLAYOUT_2CONTENT, WritingMode_LR_TB,       AUTOLAYOUT_2TEXT},
    {BMP_LAYOUT_HEAD01_57X71, BMP_LAYOUT_HEAD01_57X71_H, STR_AUTOLAYOUT_ONLY_TITLE, WritingMode_LR_TB,  AUTOLAYOUT_ONLY_TITLE},
    {BMP_LAYOUT_TEXTONLY_57X71, BMP_LAYOUT_TEXTONLY_57X71_H, STR_AUTOLAYOUT_ONLY_TEXT, WritingMode_LR_TB,   AUTOLAYOUT_ONLY_TEXT},
//--
    {BMP_LAYOUT_HEAD02B_57X71, BMP_LAYOUT_HEAD02B_57X71_H, STR_AUTOLAYOUT_CONTENT_2CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TEXT2OBJ},
    {BMP_LAYOUT_HEAD02B_57X71, BMP_LAYOUT_HEAD02B_57X71_H, STR_AUTOLAYOUT_2CONTENT_CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_2OBJTEXT},
//--
    {BMP_LAYOUT_HEAD03A_57X71, BMP_LAYOUT_HEAD03A_57X71_H, STR_AUTOLAYOUT_2CONTENT_OVER_CONTENT,WritingMode_LR_TB, AUTOLAYOUT_2OBJOVERTEXT},
    {BMP_LAYOUT_HEAD02B_57X71, BMP_LAYOUT_HEAD02B_57X71_H, STR_AUTOLAYOUT_CONTENT_OVER_CONTENT, WritingMode_LR_TB, AUTOLAYOUT_OBJOVERTEXT},
    {BMP_LAYOUT_HEAD04_57X71, BMP_LAYOUT_HEAD04_57X71_H, STR_AUTOLAYOUT_4CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_4OBJ},
    {BMP_LAYOUT_HEAD06_57X71, BMP_LAYOUT_HEAD06_57X71_H, STR_AUTOLAYOUT_6CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_6CLIPART},
/*
    // vertical
    {BMP_LAYOUT_VERTICAL02_57X71, BMP_LAYOUT_VERTICAL02_57X71_H, STR_AL_VERT_TITLE_TEXT_CHART, WritingMode_TB_RL,AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART},
    {BMP_LAYOUT_VERTICAL01_57X71, BMP_LAYOUT_VERTICAL01_57X71_H, STR_AL_VERT_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02_57X71, BMP_LAYOUT_HEAD02_57X71_H, STR_AL_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02A_57X71, BMP_LAYOUT_HEAD02A_57X71_H, STR_AL_TITLE_VERT_OUTLINE_CLIPART,   WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART},
*/
    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

// -----------------------------------------------------------------------


SdLayoutDialogContent::SdLayoutDialogContent( ViewShellBase& rBase, ::Window* pParent )
: ToolbarMenu(SID_ASSIGN_LAYOUT, rBase.GetFrame()->GetTopFrame()->GetFrameInterface(), pParent, WB_NOBORDER|WB_CLIPCHILDREN|WB_DIALOGCONTROL|WB_NOSHADOW|WB_SYSTEMWINDOW)
, mrBase(rBase)
{
    String sResetSlideLayout( SdResId( STR_RESET_LAYOUT ) );

//  FreeResource();

    //SetStyle( 2 );

//  SetHelpId( HID_POPUP_LAYOUT );

    const Color aMenuColor( GetSettings().GetStyleSettings().GetMenuColor() );
    const Color aMenuBarColor( GetSettings().GetStyleSettings().GetMenuBarColor() );
    const Color aMenuTextColor( GetSettings().GetStyleSettings().GetMenuBarColor() );
    const Color aMenuBarTextColor( GetSettings().GetStyleSettings().GetMenuBarTextColor() );

    const bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SvtLanguageOptions aLanguageOptions;
    const bool bVerticalEnabled = aLanguageOptions.IsVerticalTextEnabled();

//  mpToolbarMenu.reset( new ToolbarMenu( this, WB_CLIPCHILDREN ) );
//  mpToolbarMenu->SetHelpId( HID_MENU_EXTRUSION_DIRECTION );
    /* mpToolbarMenu->*/ SetSelectHdl( LINK( this, SdLayoutDialogContent, SelectHdl ) );


    mpLayoutSet = new ValueSet( /* mpToolbarMenu->.get()*/ this, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
//  mpLayoutSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLayoutSet->SetSelectHdl( LINK( this, SdLayoutDialogContent, SelectHdl ) );
    mpLayoutSet->SetColCount( 4 );
    mpLayoutSet->EnableFullItemMode( FALSE );
    mpLayoutSet->SetColor( GetControlBackground() );

    Size aLayoutItemSize;

    USHORT nItem = 1;
    for( snewfoil_value_info* pInfo = &standard[0]; pInfo->mnBmpResId; pInfo++ )
    {
        if( !bVerticalEnabled && (pInfo->meWritingMode == WritingMode_TB_RL) )
            continue;

        String aText( SdResId( pInfo->mnStrResId ) );
        BitmapEx aBmp( SdResId( bHighContrast ? pInfo->mnHCBmpResId : pInfo->mnBmpResId ) );
        aBmp.Expand( 4, 4, 0, TRUE );
        mpLayoutSet->InsertItem( static_cast<USHORT>(pInfo->maAutoLayout)+1, aBmp, aText );

        aLayoutItemSize.Width() = std::max( aLayoutItemSize.Width(), aBmp.GetSizePixel().Width() );
        aLayoutItemSize.Height() = std::max( aLayoutItemSize.Height(), aBmp.GetSizePixel().Height() );
    }

    aLayoutItemSize = mpLayoutSet->CalcItemSizePixel( aLayoutItemSize );
    mpLayoutSet->SetSizePixel( mpLayoutSet->CalcWindowSizePixel( aLayoutItemSize ) );

    SetText( String( SdResId( STR_UNDO_MODIFY_PAGE ) ) );
    //mpToolbarMenu->appendEntry( -1,  );
    /*mpToolbarMenu->*/appendEntry( 0, mpLayoutSet );
    /*mpToolbarMenu->*/appendSeparator();

    Reference< XFrame > xFrame( GetFrame(), UNO_QUERY );
    if( xFrame.is() )
    {
        Image aImg( ::GetImage( xFrame, OUString( RTL_CONSTASCII_USTRINGPARAM(".uno:Undo") ), FALSE, FALSE ) );
        /*mpToolbarMenu->*/appendEntry( 1, sResetSlideLayout, aImg);
    }
    else
    {
        /*mpToolbarMenu->*/appendEntry( 1, sResetSlideLayout);
    }

/*
    Size aSize( mpToolbarMenu->getMenuSize() );
    mpToolbarMenu->SetPosSizePixel( Point( 1,1 ), aSize );

    aSize.Width() += 2;
    aSize.Height() += 2;
*/
    SetOutputSizePixel( getMenuSize() );

//  mpToolbarMenu->Show();
}

// -----------------------------------------------------------------------

SdLayoutDialogContent::~SdLayoutDialogContent()
{
}


// -----------------------------------------------------------------------


/*
void SdLayoutDialogContent::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxDockingWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        // const bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

        // todo switch images
    }
}
*/

// -----------------------------------------------------------------------

/*
void SdLayoutDialogContent::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    if( (nSID == SID_ASSIGN_LAYOUT) && (eState != SFX_ITEM_DISABLED) )
    {
        const SfxUInt32Item* pStateItem = dynamic_cast< const SfxUInt32Item* >(pState);
        if( pStateItem )
        {
            meCurrentLayout = static_cast< AutoLayout >( pStateItem->GetValue() );
            mpLayoutSet->SelectItem( static_cast<USHORT>(meCurrentLayout)+1 );
        }
    }
}
*/

// -----------------------------------------------------------------------

IMPL_LINK( SdLayoutDialogContent, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    if( mrBase.GetMainViewShell().get() && mrBase.GetMainViewShell()->GetViewFrame() )
    {
        if( pControl == mpLayoutSet )
        {
            AutoLayout eLayout = static_cast< AutoLayout >(mpLayoutSet->GetSelectItemId()-1);

            const SfxUInt32Item aItem(ID_VAL_WHATLAYOUT, eLayout);
            mrBase.GetMainViewShell()->GetViewFrame()->GetBindings().GetDispatcher()->Execute(SID_ASSIGN_LAYOUT,SFX_CALLMODE_ASYNCHRON,&aItem,0);
        }
        else
        {
            // reset autolayout
            mrBase.GetMainViewShell()->GetViewFrame()->GetBindings().GetDispatcher()->Execute(SID_ASSIGN_LAYOUT,SFX_CALLMODE_ASYNCHRON);
        }
    }

    return 0;
}

} // end of namespace sd
