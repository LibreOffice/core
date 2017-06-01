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
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/numvset.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>

class NumberingToolBoxControl;

class NumberingPopup : public svtools::ToolbarMenu
{
    NumberingPageType mePageType;
    NumberingToolBoxControl& mrController;
    VclPtr<SvxNumValueSet> mpValueSet;
    DECL_LINK( VSSelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK( VSSelectValueSetHdl, ValueSet*, void );
    void VSSelectHdl(void *);
public:
    NumberingPopup( NumberingToolBoxControl& rController,
                    vcl::Window* pParent, NumberingPageType ePageType );
    virtual ~NumberingPopup() override;
    virtual void dispose() override;

    virtual void statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
};

class NumberingToolBoxControl : public svt::PopupWindowController
{
    NumberingPageType mePageType;

public:
    explicit NumberingToolBoxControl( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    using svt::ToolboxController::createPopupWindow;
};

//class NumberingPopup
NumberingPopup::NumberingPopup( NumberingToolBoxControl& rController,
                                vcl::Window* pParent, NumberingPageType ePageType ) :
    ToolbarMenu( rController.getFrameInterface(), pParent, WB_STDPOPUP ),
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

    Size aItemSize( LogicToPixel( Size( 30, 42 ), MapUnit::MapAppFont ) );
    mpValueSet->SetExtraSpacing( 2 );
    mpValueSet->SetOutputSizePixel( mpValueSet->CalcWindowSizePixel( aItemSize ) );
    mpValueSet->SetColor( GetSettings().GetStyleSettings().GetFieldColor() );

    appendEntry( 0, mpValueSet );
    appendSeparator();

    OUString aMoreItemText;
    if ( mePageType == NumberingPageType::BULLET )
    {
        aMoreItemText = SvxResId( RID_SVXSTR_MOREBULLETS );
        AddStatusListener( ".uno:CurrentBulletListType" );
    }
    else if ( mePageType == NumberingPageType::SINGLENUM )
    {
        aMoreItemText = SvxResId( RID_SVXSTR_MORENUMBERING );
        AddStatusListener( ".uno:CurrentNumListType" );
    }
    else
    {
        aMoreItemText = SvxResId( RID_SVXSTR_MORE );
        AddStatusListener( ".uno:CurrentOutlineType" );
    }

    appendEntry( 1, aMoreItemText,
        vcl::CommandInfoProvider::GetImageForCommand( ".uno:OutlineBullet", mrController.getFrameInterface() ) );

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
{
    mpValueSet->SetNoSelection();

    sal_Int32 nSelItem;
    if ( rEvent.State >>= nSelItem )
        mpValueSet->SelectItem( nSelItem );
}

IMPL_LINK( NumberingPopup, VSSelectValueSetHdl, ValueSet*, pControl, void )
{
    VSSelectHdl(pControl);
}
IMPL_LINK( NumberingPopup, VSSelectToolbarMenuHdl, ToolbarMenu*, pControl, void )
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
        auto aArgs( comphelper::InitPropertySequence( { { "Page", css::uno::makeAny( OUString("customize") ) } } ) );
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
    return VclPtr<NumberingPopup>::Create( *this, pParent, mePageType );
}


void SAL_CALL NumberingToolBoxControl::statusChanged( const css::frame::FeatureStateEvent& rEvent )
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
{
    return OUString( "com.sun.star.comp.svx.NumberingToolBoxControl" );
}

css::uno::Sequence< OUString > SAL_CALL NumberingToolBoxControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svx_NumberingToolBoxControl_get_implementation(
    css::uno::XComponentContext *rxContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire( new NumberingToolBoxControl( rxContext ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
