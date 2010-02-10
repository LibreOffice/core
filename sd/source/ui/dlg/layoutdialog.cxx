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
#include "layoutdialog.hrc"

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
    {BMP_LAYOUT_EMPTY, BMP_LAYOUT_EMPTY_H, STR_AUTOLAYOUT_NONE, WritingMode_LR_TB,        AUTOLAYOUT_NONE},
    {BMP_LAYOUT_HEAD03, BMP_LAYOUT_HEAD03_H, STR_AUTOLAYOUT_TITLE, WritingMode_LR_TB,       AUTOLAYOUT_TITLE},
    {BMP_LAYOUT_HEAD02, BMP_LAYOUT_HEAD02_H, STR_AUTOLAYOUT_CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_ENUM},
    {BMP_LAYOUT_HEAD02A, BMP_LAYOUT_HEAD02A_H, STR_AUTOLAYOUT_2CONTENT, WritingMode_LR_TB,       AUTOLAYOUT_2TEXT},
    {BMP_LAYOUT_HEAD01, BMP_LAYOUT_HEAD01_H, STR_AUTOLAYOUT_ONLY_TITLE, WritingMode_LR_TB,  AUTOLAYOUT_ONLY_TITLE},
    {BMP_LAYOUT_TEXTONLY, BMP_LAYOUT_TEXTONLY_H, STR_AUTOLAYOUT_ONLY_TEXT, WritingMode_LR_TB,   AUTOLAYOUT_ONLY_TEXT},
    {BMP_LAYOUT_HEAD03B, BMP_LAYOUT_HEAD03B_H, STR_AUTOLAYOUT_2CONTENT_CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_2OBJTEXT},
    {BMP_LAYOUT_HEAD03C, BMP_LAYOUT_HEAD03C_H, STR_AUTOLAYOUT_CONTENT_2CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TEXT2OBJ},
    {BMP_LAYOUT_HEAD03A, BMP_LAYOUT_HEAD03A_H, STR_AUTOLAYOUT_2CONTENT_OVER_CONTENT,WritingMode_LR_TB, AUTOLAYOUT_2OBJOVERTEXT},
    {BMP_LAYOUT_HEAD02B, BMP_LAYOUT_HEAD02B_H, STR_AUTOLAYOUT_CONTENT_OVER_CONTENT, WritingMode_LR_TB, AUTOLAYOUT_OBJOVERTEXT},
    {BMP_LAYOUT_HEAD04, BMP_LAYOUT_HEAD04_H, STR_AUTOLAYOUT_4CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_4OBJ},
    {BMP_LAYOUT_HEAD06, BMP_LAYOUT_HEAD06_H, STR_AUTOLAYOUT_6CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_6CLIPART},
    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

static snewfoil_value_info v_standard[] =
{
    // vertical
    {BMP_LAYOUT_VERTICAL02, BMP_LAYOUT_VERTICAL02_H, STR_AL_VERT_TITLE_TEXT_CHART, WritingMode_TB_RL,AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART},
    {BMP_LAYOUT_VERTICAL01, BMP_LAYOUT_VERTICAL01_H, STR_AL_VERT_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02, BMP_LAYOUT_HEAD02_H, STR_AL_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02A, BMP_LAYOUT_HEAD02A_H, STR_AL_TITLE_VERT_OUTLINE_CLIPART,   WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART},
    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

// -----------------------------------------------------------------------

static void fillLayoutValueSet( ValueSet* pValue, snewfoil_value_info* pInfo, const bool bHighContrast )
{
    Size aLayoutItemSize;
    for( ; pInfo->mnBmpResId; pInfo++ )
    {
        String aText( SdResId( pInfo->mnStrResId ) );
        BitmapEx aBmp( SdResId( (bHighContrast ? pInfo->mnHCBmpResId : pInfo->mnBmpResId) ) );

        pValue->InsertItem( static_cast<USHORT>(pInfo->maAutoLayout)+1, aBmp, aText );

        aLayoutItemSize.Width() = std::max( aLayoutItemSize.Width(), aBmp.GetSizePixel().Width() );
        aLayoutItemSize.Height() = std::max( aLayoutItemSize.Height(), aBmp.GetSizePixel().Height() );
    }

    aLayoutItemSize = pValue->CalcItemSizePixel( aLayoutItemSize );
    pValue->SetSizePixel( pValue->CalcWindowSizePixel( aLayoutItemSize ) );
}

// -----------------------------------------------------------------------


SdLayoutDialogContent::SdLayoutDialogContent( ViewShellBase& rBase, ::Window* pParent, const bool bInsertPage )
: ToolbarMenu(rBase.GetFrame()->GetTopFrame()->GetFrameInterface(), pParent, SdResId( DLG_LAYOUTDIALOG ) /*WB_CLIPCHILDREN|WB_DIALOGCONTROL|WB_SYSTEMWINDOW|WB_MOVEABLE|WB_SIZEABLE|WB_CLOSEABLE*/)
, mrBase(rBase)
, mbInsertPage( bInsertPage )
, mpLayoutSet1( 0 )
, mpLayoutSet2( 0 )
, msAssignLayout( RTL_CONSTASCII_USTRINGPARAM( ".uno:AssignLayout" ) )
{
    String aTitle1( SdResId( STR_HORIZONTAL_LAYOUTS ) );
    String aTitle2( SdResId( STR_VERTICAL_LAYOUTS ) );

    FreeResource();

    const Color aMenuColor( GetSettings().GetStyleSettings().GetMenuColor() );
    const Color aMenuBarColor( GetSettings().GetStyleSettings().GetMenuBarColor() );
    const Color aMenuTextColor( GetSettings().GetStyleSettings().GetMenuBarColor() );
    const Color aMenuBarTextColor( GetSettings().GetStyleSettings().GetMenuBarTextColor() );

    const bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SvtLanguageOptions aLanguageOptions;
    const bool bVerticalEnabled = aLanguageOptions.IsVerticalTextEnabled();

    SetSelectHdl( LINK( this, SdLayoutDialogContent, SelectHdl ) );

    mpLayoutSet1 = new ValueSet( this, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
//  mpLayoutSet1->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLayoutSet1->SetSelectHdl( LINK( this, SdLayoutDialogContent, SelectHdl ) );
    mpLayoutSet1->SetColCount( 4 );
    mpLayoutSet1->EnableFullItemMode( FALSE );
    mpLayoutSet1->SetColor( GetControlBackground() );

    fillLayoutValueSet( mpLayoutSet1, &standard[0], bHighContrast );

    if( bVerticalEnabled )
        appendEntry( -1, aTitle1 );
    appendEntry( 0, mpLayoutSet1 );

    if( bVerticalEnabled )
    {
        mpLayoutSet2 = new ValueSet( this, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
    //  mpLayoutSet2->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

        mpLayoutSet2->SetSelectHdl( LINK( this, SdLayoutDialogContent, SelectHdl ) );
        mpLayoutSet2->SetColCount( 4 );
        mpLayoutSet2->EnableFullItemMode( FALSE );
        mpLayoutSet2->SetColor( GetControlBackground() );

        fillLayoutValueSet( mpLayoutSet2, &v_standard[0], bHighContrast );

        appendEntry( -1, aTitle2 );
        appendEntry( 1, mpLayoutSet2 );
    }

    //appendSeparator();

    OUString sSlotStr;
    Image aSlotImage;
    Reference< XFrame > xFrame( GetFrame(), UNO_QUERY );
    if( xFrame.is() )
    {
        if( bInsertPage )
            sSlotStr = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DuplicatePage" ) );
        else
            sSlotStr = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Undo" ) );
        aSlotImage = ::GetImage( xFrame, sSlotStr, FALSE, FALSE );
    }

    String sSlotTitle;
    if( bInsertPage )
        sSlotTitle = mrBase.RetrieveLabelFromCommand( sSlotStr );
    else
        sSlotTitle = String( SdResId( STR_RESET_LAYOUT ) );
    appendEntry( 2, sSlotTitle, aSlotImage);

    SetOutputSizePixel( getMenuSize() );

    AddStatusListener( msAssignLayout );
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
            mpLayoutSet1->SelectItem( static_cast<USHORT>(meCurrentLayout)+1 );
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
        if( pControl == mpLayoutSet1 )
        {
            AutoLayout eLayout = static_cast< AutoLayout >(mpLayoutSet1->GetSelectItemId()-1);

            const SfxUInt32Item aItem(ID_VAL_WHATLAYOUT, eLayout);
            mrBase.GetMainViewShell()->GetViewFrame()->GetBindings().GetDispatcher()->Execute(mbInsertPage ? SID_INSERTPAGE : SID_ASSIGN_LAYOUT,SFX_CALLMODE_ASYNCHRON,&aItem,0);
        }
        else if( pControl == mpLayoutSet2 )
        {
            AutoLayout eLayout = static_cast< AutoLayout >(mpLayoutSet2->GetSelectItemId()-1);

            const SfxUInt32Item aItem(ID_VAL_WHATLAYOUT, eLayout);
            mrBase.GetMainViewShell()->GetViewFrame()->GetBindings().GetDispatcher()->Execute(mbInsertPage ? SID_INSERTPAGE : SID_ASSIGN_LAYOUT,SFX_CALLMODE_ASYNCHRON,&aItem,0);
        }
        else
        {
            // reset autolayout
            mrBase.GetMainViewShell()->GetViewFrame()->GetBindings().GetDispatcher()->Execute( mbInsertPage ? SID_DUPLICATE_PAGE : SID_ASSIGN_LAYOUT,SFX_CALLMODE_ASYNCHRON);
        }
    }

    return 0;
}

void SAL_CALL SdLayoutDialogContent::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException )
{
    if( Event.FeatureURL.Main.equals( msAssignLayout ) )
    {
        sal_Int32 nLayout = 0;
        Event.State >>= nLayout;
        OSL_TRACE("SdLayoutDialogContent::statusChanged(%ld)", nLayout );
    }
}

} // end of namespace sd
