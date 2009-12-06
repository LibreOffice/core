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

#include "layoutdialog.hxx"
#include <sfx2/dockwin.hxx>
#include "app.hrc"
#include <sfx2/app.hxx>

// Instantiate the implementation of the docking window before files
// are included that define ::sd::Window.  The ... macros are not really
// namespace proof.
namespace sd {
SFX_IMPL_DOCKINGWINDOW(LayoutDialogChildWindow, SID_LAYOUT_DIALOG_WIN)
}

#include <vcl/image.hxx>

#include <sfx2/dispatch.hxx>

#include <svtools/languageoptions.hxx>
#include <svtools/valueset.hxx>

#include <svx/toolbarmenu.hxx>

#include <com/sun/star/text/WritingMode.hpp>

#include "layoutdialog.hrc"
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

namespace sd {

///
class SdLayoutDialogContent : public SfxDockingWindow
{
public:
    SdLayoutDialogContent( SfxBindings* pBindings, SfxChildWindow *pCW, ::Window* pParent, ViewShellBase& rBase);
    virtual ~SdLayoutDialogContent();

protected:
    virtual void Resize (void);

    DECL_LINK( SelectHdl, void * );

private:
    boost::scoped_ptr<ToolbarMenu> mpToolbarMenu;
    ValueSet* mpLayoutSet;
    AutoLayout meCurrentLayout;
    ViewShellBase& mrBase;
};

// -----------------------------------------------------------------------

LayoutDialogChildWindow::LayoutDialogChildWindow(::Window* _pParent, USHORT nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo)
: SfxChildWindow (_pParent, nId)
{
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(pBindings->GetDispatcher()->GetFrame()));
    SdLayoutDialogContent* pContent = new SdLayoutDialogContent (pBindings, this, _pParent, rBase);
    pWindow = pContent;
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    pContent->Initialize(pInfo);
}

LayoutDialogChildWindow::~LayoutDialogChildWindow (void)
{
}

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

// -----------------------------------------------------------------------

SdLayoutDialogContent::SdLayoutDialogContent( SfxBindings* pInBindings, SfxChildWindow *pCW, Window* pParent, ViewShellBase& rBase)
: SfxDockingWindow(pInBindings, pCW, pParent, SdResId( FLT_WIN_LAYOUT_DIALOG ))
, meCurrentLayout( AUTOLAYOUT_NONE )
, mrBase(rBase)
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

//    AddStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AssignLayout" )));

    FreeResource();
}

void SdLayoutDialogContent::Resize (void)
{
    SfxDockingWindow::Resize();
    mpToolbarMenu->SetPosSizePixel( Point(0,0), GetSizePixel() );
}

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
/*
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
*/
    return 0;
}

} // end of namespace sd
