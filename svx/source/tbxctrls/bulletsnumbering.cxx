/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>

#include <comphelper/propertysequence.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <sfx2/imagemgr.hxx>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/numvset.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>

class NumberingToolBoxControl;

class NumberingPopup : public svtools::ToolbarMenu
{
    NumberingPageType mePageType;
    NumberingToolBoxControl& mrController;
    VclPtr<SvxNumValueSet> mpValueSet;
    DECL_LINK_TYPED( VSSelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK_TYPED( VSSelectValueSetHdl, ValueSet*, void );
    void VSSelectHdl(void *);
public:
    NumberingPopup( NumberingToolBoxControl& rController,
                    const css::uno::Reference< css::frame::XFrame >& rFrame,
                    vcl::Window* pParent, NumberingPageType ePageType );
    virtual ~NumberingPopup();
    virtual void dispose() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& rEvent )
        throw ( css::uno::RuntimeException ) override;
};

class NumberingToolBoxControl : public svt::PopupWindowController
{
    NumberingPageType mePageType;

public:
    explicit NumberingToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;
    bool IsInImpressDraw();

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    using svt::ToolboxController::createPopupWindow;
};

//class NumberingPopup
NumberingPopup::NumberingPopup( NumberingToolBoxControl& rController,
                                const css::uno::Reference< css::frame::XFrame >& rFrame,
                                vcl::Window* pParent, NumberingPageType ePageType ) :
    ToolbarMenu( rFrame, pParent, WB_STDPOPUP ),
    mePageType( ePageType ),
    mrController( rController )
{
    WinBits nBits = WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NO_DIRECTSELECT;
    mpValueSet = VclPtr<SvxNumValueSet>::Create( this, nBits );
    mpValueSet->init( mePageType );

    if ( mePageType != NumberingPageType::BULLET )
    {
        css::uno::Reference< css::text::XDefaultNumberingProvider > xDefNum = css::text::DefaultNumberingProvider::create( mrController.getContext() );
        if ( xDefNum.is() )
        {
            css::lang::Locale aLocale = GetSettings().GetLanguageTag().getLocale();
            css::uno::Reference< css::text::XNumberingFormatter > xFormat( xDefNum, css::uno::UNO_QUERY );

            if ( mePageType == NumberingPageType::SINGLENUM )
            {
                css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aNumberings(
                    xDefNum->getDefaultContinuousNumberingLevels( aLocale ) );
                mpValueSet->SetNumberingSettings( aNumberings, xFormat, aLocale );
            }
            else if ( mePageType == NumberingPageType::OUTLINE )
            {
                css::uno::Sequence< css::uno::Reference< css::container::XIndexAccess > > aOutline(
                    xDefNum->getDefaultOutlineNumberings( aLocale ) );
                mpValueSet->SetOutlineNumberingSettings( aOutline, xFormat, aLocale );
            }
        }
    }

    Size aItemSize( LogicToPixel( Size( 30, 42 ), MAP_APPFONT ) );
    mpValueSet->SetExtraSpacing( 2 );
    mpValueSet->SetOutputSizePixel( mpValueSet->CalcWindowSizePixel( aItemSize ) );
    mpValueSet->SetColor( GetSettings().GetStyleSettings().GetFieldColor() );

    appendEntry( 0, mpValueSet );
    appendSeparator();

    OUString aMoreItemText;
    if ( mePageType == NumberingPageType::BULLET )
    {
        aMoreItemText = SVX_RESSTR( RID_SVXSTR_MOREBULLETS );
        AddStatusListener( ".uno:CurrentBulletListType" );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        aMoreItemText = SVX_RESSTR( RID_SVXSTR_MORENUMBERING );
        AddStatusListener( ".uno:CurrentNumListType" );
    }
    else
    {
        aMoreItemText = SVX_RESSTR( RID_SVXSTR_MORE );
        AddStatusListener( ".uno:CurrentOutlineType" );
    }

    appendEntry( 1, aMoreItemText, ::GetImage( rFrame, ".uno:OutlineBullet", false ) );

    SetOutputSizePixel( getMenuSize() );
    mpValueSet->SetSelectHdl( LINK( this, NumberingPopup, VSSelectValueSetHdl ) );
    SetSelectHdl( LINK( this, NumberingPopup, VSSelectToolbarMenuHdl ) );
}

NumberingPopup::~NumberingPopup()
{
    disposeOnce();
}

void NumberingPopup::dispose()
{
    mpValueSet.clear();
    ToolbarMenu::dispose();
}

void NumberingPopup::statusChanged( const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException )
{
    mpValueSet->SetNoSelection();

    sal_Int32 nSelItem;
    if ( rEvent.State >>= nSelItem )
        mpValueSet->SelectItem( nSelItem );
}

IMPL_LINK_TYPED( NumberingPopup, VSSelectValueSetHdl, ValueSet*, pControl, void )
{
    VSSelectHdl(pControl);
}
IMPL_LINK_TYPED( NumberingPopup, VSSelectToolbarMenuHdl, ToolbarMenu*, pControl, void )
{
    VSSelectHdl(pControl);
}

void NumberingPopup::VSSelectHdl(void* pControl)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    if ( pControl == mpValueSet )
    {
        sal_uInt16 nSelItem = mpValueSet->GetSelectItemId();
        if ( mePageType == NumberingPageType::BULLET )
        {
            auto aArgs( comphelper::InitPropertySequence( { { "SetBullet", css::uno::makeAny( nSelItem ) } } ) );
            mrController.dispatchCommand( ".uno:SetBullet", aArgs );
        }
        else if ( mePageType == NumberingPageType::SINGLENUM )
        {
            auto aArgs( comphelper::InitPropertySequence( { { "SetNumber", css::uno::makeAny( nSelItem ) } } ) );
            mrController.dispatchCommand( ".uno:SetNumber", aArgs );
        }
        else
        {
            auto aArgs( comphelper::InitPropertySequence( { { "SetOutline", css::uno::makeAny( nSelItem ) } } ) );
            mrController.dispatchCommand( ".uno:SetOutline", aArgs );
        }
    }
    else if ( getSelectedEntryId() == 1 )
    {
        OUString aPageName;
        if ( mrController.IsInImpressDraw() )
            aPageName = "customize";
        else
            // Writer variants
            aPageName = "options";

        auto aArgs( comphelper::InitPropertySequence( { { "Page", css::uno::makeAny( aPageName ) } } ) );
        mrController.dispatchCommand( ".uno:OutlineBullet", aArgs );
    }
}


//class NumberingToolBoxControl
NumberingToolBoxControl::NumberingToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext ):
    svt::PopupWindowController( rxContext, css::uno::Reference< css::frame::XFrame >(), OUString() ),
    mePageType( NumberingPageType::SINGLENUM )
{
}

VclPtr<vcl::Window> NumberingToolBoxControl::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<NumberingPopup>::Create( *this, m_xFrame, pParent, mePageType );
}

bool NumberingToolBoxControl::IsInImpressDraw()
{
    return ( m_sModuleName == "com.sun.star.presentation.PresentationDocument" ||
             m_sModuleName == "com.sun.star.drawing.DrawingDocument" );
}

void SAL_CALL NumberingToolBoxControl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
    {
        pToolBox->EnableItem( nId, rEvent.IsEnabled );
        bool bChecked;
        if ( rEvent.State >>= bChecked )
            pToolBox->CheckItem( nId, bChecked );
    }
}

void SAL_CALL NumberingToolBoxControl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
    throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::PopupWindowController::initialize( aArguments );

    if ( m_aCommandURL == ".uno:DefaultBullet" )
        mePageType = NumberingPageType::BULLET;
    else if ( m_aCommandURL == ".uno:SetOutline" )
        mePageType = NumberingPageType::OUTLINE;

    ToolBoxItemBits nBits = ( mePageType == NumberingPageType::OUTLINE ) ? ToolBoxItemBits::DROPDOWNONLY : ToolBoxItemBits::DROPDOWN;
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | nBits );
}

OUString SAL_CALL NumberingToolBoxControl::getImplementationName()
    throw ( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.svx.NumberingToolBoxControl" );
}

css::uno::Sequence< OUString > SAL_CALL NumberingToolBoxControl::getSupportedServiceNames()
    throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< OUString > aRet( 1 );
    aRet[0] = "com.sun.star.frame.ToolbarController";
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation(
    css::uno::XComponentContext *rxContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new NumberingToolBoxControl( rxContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
