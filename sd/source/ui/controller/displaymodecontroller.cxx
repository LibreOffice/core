/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/DrawViewMode.hpp>

#include <osl/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <svl/languageoptions.hxx>

#include <svtools/ctrltool.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svtools/valueset.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>

#include <sfx2/imagemgr.hxx>

#include "app.hrc"
#include "facreg.hxx"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "pres.hxx"
#include "displaymodecontroller.hxx"
#include "ViewShellBase.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;

namespace sd
{

// Component to select which display mode has to be used.
// Composed of a combobox in the toolbar and a popup menu to select
// the value

class DisplayModeToolbarMenu : public svtools::ToolbarMenu
{
public:
    DisplayModeToolbarMenu( DisplayModeController& rController,
        const Reference< XFrame >& xFrame, vcl::Window* pParent );
    virtual ~DisplayModeToolbarMenu();
    virtual void dispose() SAL_OVERRIDE;

protected:
    DECL_LINK_TYPED( SelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK_TYPED( SelectValueSetHdl, ValueSet*, void );
    void SelectHdl(void*);

private:
    DisplayModeController& mrController;
    Reference< XFrame > mxFrame;
    VclPtr<ValueSet> mpDisplayModeSet1;
    VclPtr<ValueSet> mpDisplayModeSet2;
};

struct snewfoil_value_info
{
    sal_uInt16  mnId;
    sal_uInt16  mnBmpResId;
    sal_uInt16  mnStrResId;
    OUString    msUnoCommand;
};

static const snewfoil_value_info editmodes[] =
{
    {1,
        BMP_DISPLAYMODE_SLIDE,
        STR_NORMAL_MODE,
        OUString(".uno:NormalMultiPaneGUI") },
    {2,
        BMP_DISPLAYMODE_NOTES,
        STR_NOTES_MODE,
        OUString(".uno:NotesMode") },
    {3,
        BMP_DISPLAYMODE_OUTLINE,
        STR_OUTLINE_MODE,
        OUString(".uno:OutlineMode") },
    {4,
        BMP_DISPLAYMODE_SLIDE_SORTER,
        STR_SLIDE_SORTER_MODE,
        OUString(".uno:DiaMode") },
    {0, 0, 0, OUString() }
};

static const snewfoil_value_info mastermodes[] =
{
    {5,
        BMP_DISPLAYMODE_SLIDE_MASTER,
        STR_SLIDE_MASTER_MODE,
        OUString(".uno:SlideMasterPage") },
    {6,
        BMP_DISPLAYMODE_NOTES_MASTER,
        STR_NOTES_MASTER_MODE,
        OUString(".uno:NotesMasterPage") },
    {7,
        BMP_DISPLAYMODE_HANDOUT_MASTER,
        STR_HANDOUT_MASTER_MODE,
        OUString(".uno:HandoutMode") },
    {0, 0, 0, OUString() }
};


static void fillLayoutValueSet( ValueSet* pValue, const snewfoil_value_info* pInfo )
{
    Size aLayoutItemSize;
    for( ; pInfo->mnId; pInfo++ )
    {
        OUString aText( SD_RESSTR( pInfo->mnStrResId ) );
        BitmapEx aBmp(  SdResId( pInfo->mnBmpResId ) );

        pValue->InsertItem(pInfo->mnId, Image(aBmp), aText);

        aLayoutItemSize.Width()  = std::max( aLayoutItemSize.Width(),  aBmp.GetSizePixel().Width()  );
        aLayoutItemSize.Height() = std::max( aLayoutItemSize.Height(), aBmp.GetSizePixel().Height() );
    }

    aLayoutItemSize = pValue->CalcItemSizePixel( aLayoutItemSize );
    pValue->SetSizePixel( pValue->CalcWindowSizePixel( aLayoutItemSize ) );
}

DisplayModeToolbarMenu::DisplayModeToolbarMenu( DisplayModeController& rController,
    const Reference< XFrame >& xFrame, vcl::Window* pParent )
: svtools::ToolbarMenu(xFrame, pParent, WB_CLIPCHILDREN )
, mrController( rController )
, mxFrame(xFrame)
, mpDisplayModeSet1( nullptr )
, mpDisplayModeSet2( nullptr )
{
    const sal_Int32 LAYOUT_BORDER_PIX = 7;

    OUString aTitle1( SD_RESSTR( STR_DISPLAYMODE_EDITMODES ) );
    OUString aTitle2( SD_RESSTR( STR_DISPLAYMODE_MASTERMODES ) );

    SvtLanguageOptions aLanguageOptions;

    SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectToolbarMenuHdl ) );

    mpDisplayModeSet1 = createEmptyValueSetControl();
    mpDisplayModeSet1->SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectValueSetHdl ) );

    sal_Int16 nColCount = 2;
    mpDisplayModeSet1->SetColCount( nColCount );
    fillLayoutValueSet( mpDisplayModeSet1, &editmodes[0] );

    Size aSize( mpDisplayModeSet1->GetOutputSizePixel() );
    aSize.Width() += (mpDisplayModeSet1->GetColCount() + 1) * LAYOUT_BORDER_PIX;
    aSize.Height() += (mpDisplayModeSet1->GetLineCount() +1) * LAYOUT_BORDER_PIX;
    mpDisplayModeSet1->SetOutputSizePixel( aSize );

    appendEntry( -1, aTitle1 );
    appendEntry( 1, mpDisplayModeSet1 );

    mpDisplayModeSet2 = VclPtr<ValueSet>::Create( this, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );

    mpDisplayModeSet2->SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectValueSetHdl ) );
    mpDisplayModeSet2->SetColCount( nColCount );
    mpDisplayModeSet2->EnableFullItemMode( false );
    mpDisplayModeSet2->SetColor( GetControlBackground() );

    fillLayoutValueSet( mpDisplayModeSet2, &mastermodes[0] );

    aSize = mpDisplayModeSet2->GetOutputSizePixel();
    aSize.Width() += (mpDisplayModeSet2->GetColCount() + 1) * LAYOUT_BORDER_PIX;
    aSize.Height() += (mpDisplayModeSet2->GetLineCount() + 1) * LAYOUT_BORDER_PIX;
    mpDisplayModeSet2->SetOutputSizePixel( aSize );

    appendEntry( -1, aTitle2 );
    appendEntry( 2, mpDisplayModeSet2 );

    SetOutputSizePixel( getMenuSize() );
}

DisplayModeToolbarMenu::~DisplayModeToolbarMenu()
{
    disposeOnce();
}

void DisplayModeToolbarMenu::dispose()
{
    mpDisplayModeSet1.clear();
    mpDisplayModeSet2.clear();
    svtools::ToolbarMenu::dispose();
}

IMPL_LINK_TYPED( DisplayModeToolbarMenu, SelectValueSetHdl, ValueSet*, pControl, void )
{
    SelectHdl(pControl);
}
IMPL_LINK_TYPED( DisplayModeToolbarMenu, SelectToolbarMenuHdl, ToolbarMenu *, pControl, void )
{
    SelectHdl(pControl);
}

void DisplayModeToolbarMenu::SelectHdl(void * pControl)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    OUString sCommandURL;
    sal_uInt16 nImage = 0;

    if( pControl == mpDisplayModeSet1 ) {
        sCommandURL = editmodes[mpDisplayModeSet1->GetSelectItemId() - 1 ].msUnoCommand;
        nImage = editmodes[mpDisplayModeSet1->GetSelectItemId() - 1 ].mnBmpResId;
    }
    else if( pControl == mpDisplayModeSet2 ) {
        sCommandURL = mastermodes[mpDisplayModeSet2->GetSelectItemId() - 5 ].msUnoCommand;
        nImage = mastermodes[mpDisplayModeSet2->GetSelectItemId() - 5 ].mnBmpResId;
    }

    Sequence< PropertyValue > aArgs;
    if (!sCommandURL.isEmpty())
        mrController.dispatchCommand( sCommandURL, aArgs );

    mrController.setToolboxItemImage( nImage );
}

DisplayModeController::DisplayModeController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
: svt::PopupWindowController( rxContext, Reference< frame::XFrame >(), OUString() )
{
}

void SAL_CALL DisplayModeController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::PopupWindowController::initialize( aArguments );
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
    setToolboxItemImage( BMP_DISPLAYMODE_SLIDE );
}

VclPtr<vcl::Window> DisplayModeController::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<sd::DisplayModeToolbarMenu>::Create( *this, m_xFrame, pParent );
}

void DisplayModeController::setToolboxItemImage( sal_uInt16 nImage )
{
    sal_uInt16 nId;
    ToolBox* pToolBox = nullptr;
    if (getToolboxId( nId, &pToolBox )) {
        SdResId resId( nImage );
        BitmapEx aBmp( resId );
        int targetSize = (pToolBox->GetToolboxButtonSize() == TOOLBOX_BUTTONSIZE_LARGE) ? 32 : 16;
        double scale = 1.0f;
        Size size = aBmp.GetSizePixel();
        if (size.Width() > targetSize)
            scale = (double)targetSize / (double)size.Width();
        if (size.Height() > targetSize)
            scale = ::std::min( scale, (double)targetSize / (double)size.Height() );
        aBmp.Scale( scale, scale );
        pToolBox->SetItemImage( nId, Image( aBmp ) );
    }
}

// XServiceInfo

OUString SAL_CALL DisplayModeController::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.sd.DisplayModeController" );;
}

Sequence< OUString > SAL_CALL DisplayModeController::getSupportedServiceNames(  ) throw( RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aRet { "com.sun.star.frame.ToolbarController" };
    return aRet;
}

}


extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_sd_DisplayModeController_get_implementation(::com::sun::star::uno::XComponentContext* context,
                                                              ::com::sun::star::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::DisplayModeController(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
