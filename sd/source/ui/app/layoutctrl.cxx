/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tmplctrl.cxx,v $
 *
 * $Revision: 1.3 $
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

#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <boost/scoped_ptr.hpp>

#include <vcl/menu.hxx>
#include <vcl/status.hxx>
#include <vcl/toolbox.hxx>

#include <svl/languageoptions.hxx>
#include <svl/style.hxx>
#include <svl/stritem.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/valueset.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/dockwin.hxx>

#include <svx/toolbarmenu.hxx>

#include "layoutdialog.hxx"
#include "sdresid.hxx"
#include "res_bmp.hrc"
#include "strings.hrc"
#include "layoutctrl.hxx"
#include "ViewShellBase.hxx"
#include "drawdoc.hxx"
#include "sdattr.hrc"
#include "app.hrc"
#include "glob.hrc"

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

SFX_IMPL_TOOLBOX_CONTROL( SdLayoutControl, SfxVoidItem );

#if 0
class SdLayoutDialogContent : public SfxDockingWindow
{
    using DockingWindow::StateChanged;

private:
    ToolBox* mpToolBox;
    ValueSet* mpLayoutSet;
    AutoLayout meCurrentLayout;

    /*boost::scoped_ptr<FixedText> mpTitle;
    boost::scoped_ptr<FixedLine> mpSeperator;
    boost::scoped_ptr<Menu> mpMenu;*/

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;

    bool        mbPopupMode;

    DECL_LINK( SelectHdl, void * );

protected:
    virtual void    PopupModeEnd();

    /** This function is called when the window gets the focus.  It grabs
        the focus to the line ends value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);

public:
    SdLayoutDialogContent( SfxBindings* pBindings, SfxChildWindow *pCW, ::Window* pParent,  const SdResId& rSdResId, ViewShellBase& rBase);
    ~SdLayoutDialogContent();

    virtual SfxDockingWindow* Clone() const;

    virtual void        Paint( const Rectangle& rRect );

    virtual void StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};
#endif
// class SdLayoutControl ------------------------------------------

SdLayoutControl::SdLayoutControl( USHORT _nSlotId, USHORT _nId, ToolBox& rTbx )
: SfxToolBoxControl( _nSlotId, _nId, rTbx )
{
    rTbx.SetItemBits( _nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( _nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------

SdLayoutControl::~SdLayoutControl()
{
}

// -----------------------------------------------------------------------

void SdLayoutControl::Select( BOOL bMod1 )
{
    (void)bMod1;
}

// -----------------------------------------------------------------------

void SdLayoutControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

SfxPopupWindowType SdLayoutControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SdLayoutControl::CreatePopupWindow()
{
    /*
    OUString sResourceName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/dockingwindow/" ) );
    sResourceName += OUString::valueOf( static_cast<sal_Int32>(GetId()) );
    createAndPositionSubToolBar( sResourceName );
    */

    SfxPopupWindow* pWin = 0;

    ToolBox& rTbx = GetToolBox();
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if( pViewFrame )
    {
        pWin = sd::LayoutDialogChildWindow::createChildWindow( *pViewFrame, &rTbx );
        if( pWin )
        {
            pWin->EnableDocking(true);
            Window::GetDockingManager()->StartPopupMode( &rTbx, pWin );
        }
     }
    SetPopupWindow( pWin );
    return 0;
}

#if 0
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
    {BMP_FOIL_20, BMP_FOIL_20_H, STR_AUTOLAYOUT_NONE, WritingMode_LR_TB,        AUTOLAYOUT_NONE},
    {BMP_FOIL_00, BMP_FOIL_00_H, STR_AUTOLAYOUT_TITLE, WritingMode_LR_TB,       AUTOLAYOUT_TITLE},
    {BMP_FOIL_01, BMP_FOIL_01_H, STR_AUTOLAYOUT_CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_ENUM},
    {BMP_FOIL_03, BMP_FOIL_03_H, STR_AUTOLAYOUT_2CONTENT, WritingMode_LR_TB,       AUTOLAYOUT_2TEXT},
    {BMP_FOIL_19, BMP_FOIL_19_H, STR_AUTOLAYOUT_ONLY_TITLE, WritingMode_LR_TB,  AUTOLAYOUT_ONLY_TITLE},
    {BMP_FOIL_25, BMP_FOIL_25_H, STR_AUTOLAYOUT_ONLY_TEXT, WritingMode_LR_TB,   AUTOLAYOUT_ONLY_TEXT},
    {BMP_FOIL_12, BMP_FOIL_12_H, STR_AUTOLAYOUT_CONTENT_2CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TEXT2OBJ},
    {BMP_FOIL_15, BMP_FOIL_15_H, STR_AUTOLAYOUT_2CONTENT_CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_2OBJTEXT},
    {BMP_FOIL_16, BMP_FOIL_16_H, STR_AUTOLAYOUT_2CONTENT_OVER_CONTENT,WritingMode_LR_TB, AUTOLAYOUT_2OBJOVERTEXT},
    {BMP_FOIL_14, BMP_FOIL_14_H, STR_AUTOLAYOUT_CONTENT_OVER_CONTENT, WritingMode_LR_TB, AUTOLAYOUT_OBJOVERTEXT},
    {BMP_FOIL_18, BMP_FOIL_18_H, STR_AUTOLAYOUT_4CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_4OBJ},
    {BMP_FOIL_27, BMP_FOIL_27_H, STR_AUTOLAYOUT_6CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_6CLIPART},

    // vertical
    {BMP_FOIL_21, BMP_FOIL_21_H, STR_AL_VERT_TITLE_TEXT_CHART, WritingMode_TB_RL,AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART},
    {BMP_FOIL_22, BMP_FOIL_22_H, STR_AL_VERT_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE},
    {BMP_FOIL_23, BMP_FOIL_23_H, STR_AL_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE},
    {BMP_FOIL_24, BMP_FOIL_24_H, STR_AL_TITLE_VERT_OUTLINE_CLIPART,   WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART},

    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

SfxPopupWindowType SdLayoutControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------

SfxPopupWindow* SdLayoutControl::CreatePopupWindow()
{
    ToolBox& rTbx = GetToolBox();
    SdLayoutDialogContent* pWin = new SdLayoutDialogContent( GetId(), m_xFrame, &rTbx );
//  pWin->StartPopupMode( &rTbx, FLOATWIN_POPUPMODE_GRABFOCUS|FLOATWIN_POPUPMODE_NOKEYCLOSE );
    pWin->EnableDocking(true);

    Window::GetDockingManager()->StartPopupMode( &rTbx, pWin );
//    SetPopupWindow( pWin );
//  return pWin;
    return 0;
}

// -----------------------------------------------------------------------

SdLayoutDialogContent::SdLayoutDialogContent( USHORT nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame, Window* pParentWindow )
: SfxDockingWindow( nId, rFrame, pParentWindow, WB_SYSTEMWINDOW )
, mxFrame( rFrame )
, mbPopupMode(true)
, meCurrentLayout( AUTOLAYOUT_NONE )
{
//  SetHelpId( HID_POPUP_LAYOUT );

    const Color aMenuColor( GetSettings().GetStyleSettings().GetMenuColor() );
    const Color aMenuBarColor( GetSettings().GetStyleSettings().GetMenuBarColor() );
    const Color aMenuTextColor( GetSettings().GetStyleSettings().GetMenuBarColor() );
    const Color aMenuBarTextColor( GetSettings().GetStyleSettings().GetMenuBarTextColor() );

    const bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
    SvtLanguageOptions aLanguageOptions;
    const bool bVerticalEnabled = aLanguageOptions.IsVerticalTextEnabled();

    mpToolbarMenu.reset( new ToolbarMenu( this, WB_CLIPCHILDREN ) );
//  mpToolbarMenu->SetHelpId( HID_MENU_EXTRUSION_DIRECTION );
    mpToolbarMenu->SetSelectHdl( LINK( this, SdLayoutDialogContent, SelectHdl ) );


    mpLayoutSet = new ValueSet( mpToolbarMenu.get(), WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
//  mpLayoutSet->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

    mpLayoutSet->SetSelectHdl( LINK( this, SdLayoutDialogContent, SelectHdl ) );
    mpLayoutSet->SetColCount( 4 );
    mpLayoutSet->EnableFullItemMode( FALSE );

    Size aLayoutItemSize;

    USHORT nItem = 1;
    for( snewfoil_value_info* pInfo = &standard[0]; pInfo->mnBmpResId; pInfo++ )
    {
        if( !bVerticalEnabled && (pInfo->meWritingMode == WritingMode_TB_RL) )
            continue;

        String aText( SdResId( pInfo->mnStrResId ) );
        BitmapEx aBmp( SdResId( bHighContrast ? pInfo->mnHCBmpResId : pInfo->mnBmpResId ) );
        mpLayoutSet->InsertItem( static_cast<USHORT>(pInfo->maAutoLayout)+1, aBmp, aText );

        aLayoutItemSize.Width() = std::max( aLayoutItemSize.Width(), aBmp.GetSizePixel().Width() );
        aLayoutItemSize.Height() = std::max( aLayoutItemSize.Height(), aBmp.GetSizePixel().Height() );
    }

    aLayoutItemSize = mpLayoutSet->CalcItemSizePixel( aLayoutItemSize );
    Size aValueSize = mpLayoutSet->CalcWindowSizePixel( aLayoutItemSize );
    mpLayoutSet->SetSizePixel( aValueSize );

    mpToolbarMenu->appendEntry( -1, String( SdResId( STR_UNDO_MODIFY_PAGE ) ) );
    mpToolbarMenu->appendEntry( 0, mpLayoutSet );
    mpToolbarMenu->appendSeparator();
    mpToolbarMenu->appendEntry( 1, String( RTL_CONSTASCII_USTRINGPARAM("Reset Slide Layout") ));

    SetOutputSizePixel( mpToolbarMenu->getMenuSize() );
    mpToolbarMenu->SetOutputSizePixel( GetOutputSizePixel() );

    mpToolbarMenu->Show();

    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AssignLayout" )));
}

SfxDockingWindow* SdLayoutDialogContent::Clone() const
{
    return new SdLayoutDialogContent( GetId(), mxFrame );
}

void SdLayoutDialogContent::Paint( const Rectangle& rRect )
{
    SfxDockingWindow::Paint(rRect);
}

void SdLayoutDialogContent::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxDockingWindow::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        // const bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

        // todo switch images
    }
}

// -----------------------------------------------------------------------

SdLayoutDialogContent::~SdLayoutDialogContent()
{
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

IMPL_LINK( SdLayoutDialogContent, SelectHdl, void *, pControl )
{
    if ( IsInPopupMode() )
        EndPopupMode();

    AutoLayout eLayout = meCurrentLayout;

    if( pControl == mpLayoutSet )
        eLayout = static_cast< AutoLayout >(mpLayoutSet->GetSelectItemId()-1);

    const rtl::OUString   aCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:AssignLayout" ));

    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("WhatLayout") );
    aArgs[0].Value <<= static_cast<sal_Int32>(eLayout);

    SfxToolBoxControl::Dispatch( Reference< ::com::sun::star::frame::XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),  aCommand, aArgs );

    return 0;
}

// -----------------------------------------------------------------------

void SdLayoutDialogContent::PopupModeEnd()
{
    if ( IsVisible() )
        mbPopupMode = FALSE;
    SfxDockingWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------

void SdLayoutDialogContent::GetFocus (void)
{
    SfxDockingWindow::GetFocus();
    if( mpToolbarMenu )
        mpToolbarMenu->GrabFocus();
}
#endif


