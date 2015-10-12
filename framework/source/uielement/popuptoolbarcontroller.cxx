/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>
#include <framework/menuconfiguration.hxx>
#include <rtl/ref.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/thePopupMenuControllerFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>

#define UNO_COMMAND_RECENT_FILE_LIST    ".uno:RecentFileList"

using namespace framework;

namespace {

typedef cppu::ImplInheritanceHelper< svt::ToolboxController,
                                    css::lang::XServiceInfo >
                ToolBarBase;

class PopupMenuToolbarController : public ToolBarBase
{
public:
    virtual ~PopupMenuToolbarController();

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;
    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw (css::uno::RuntimeException, std::exception) override;
    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

protected:
    PopupMenuToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                const OUString &rPopupCommand = OUString() );
    virtual void functionExecuted( const OUString &rCommand );
    virtual ToolBoxItemBits getDropDownStyle() const;
    void createPopupMenuController();

    css::uno::Reference< css::uno::XComponentContext >      m_xContext;
    bool                                                m_bHasController;
    css::uno::Reference< css::awt::XPopupMenu >             m_xPopupMenu;

private:
    OUString m_aPopupCommand;
    css::uno::Reference< css::frame::XUIControllerFactory > m_xPopupMenuFactory;
    css::uno::Reference< css::frame::XPopupMenuController > m_xPopupMenuController;
};

PopupMenuToolbarController::PopupMenuToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const OUString &rPopupCommand )
    : m_xContext( xContext )
    , m_bHasController( false )
    , m_aPopupCommand( rPopupCommand )
{
}

PopupMenuToolbarController::~PopupMenuToolbarController()
{
}

void SAL_CALL PopupMenuToolbarController::dispose()
throw ( css::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::dispose();

    osl::MutexGuard aGuard( m_aMutex );
    if( m_xPopupMenuController.is() )
    {
        css::uno::Reference< css::lang::XComponent > xComponent(
            m_xPopupMenuController, css::uno::UNO_QUERY );
        if( xComponent.is() )
        {
            try
            {
                xComponent->dispose();
            }
            catch (...)
            {}
        }
        m_xPopupMenuController.clear();
    }

    m_xContext.clear();
    m_xPopupMenuFactory.clear();
    m_xPopupMenu.clear();
}

void SAL_CALL PopupMenuToolbarController::initialize(
    const css::uno::Sequence< css::uno::Any >& aArguments )
throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    ToolboxController::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_aPopupCommand.getLength() )
        m_aPopupCommand = m_aCommandURL;

    try
    {
        m_xPopupMenuFactory.set(
            css::frame::thePopupMenuControllerFactory::get( m_xContext ) );
        m_bHasController = m_xPopupMenuFactory->hasController(
            m_aPopupCommand, getModuleName() );
    }
    catch (const css::uno::Exception& e)
    {
        SAL_INFO( "fwk.uielement", "Caught an exception: " << e.Message );
    }

    SolarMutexGuard aSolarLock;
    VclPtr< ToolBox > pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ).get() );
    if ( pToolBox )
    {
        ToolBoxItemBits nCurStyle( pToolBox->GetItemBits( m_nToolBoxId ) );
        ToolBoxItemBits nSetStyle( getDropDownStyle() );
        pToolBox->SetItemBits( m_nToolBoxId,
                               m_bHasController ?
                                    nCurStyle | nSetStyle :
                                    nCurStyle & ~nSetStyle );
    }

}

void SAL_CALL PopupMenuToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    // TODO move to base class

    svt::ToolboxController::statusChanged( rEvent );
    enable( rEvent.IsEnabled );
}

css::uno::Reference< css::awt::XWindow > SAL_CALL
PopupMenuToolbarController::createPopupWindow()
    throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::awt::XWindow > xRet;

    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_bHasController )
        return xRet;

    createPopupMenuController();

    SolarMutexGuard aSolarLock;
    VclPtr< ToolBox > pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ).get() );
    if ( !pToolBox )
        return xRet;

    pToolBox->SetItemDown( m_nToolBoxId, true );
    WindowAlign eAlign( pToolBox->GetAlign() );
    sal_uInt16 nId = m_xPopupMenu->execute(
        css::uno::Reference< css::awt::XWindowPeer >( getParent(), css::uno::UNO_QUERY ),
        VCLUnoHelper::ConvertToAWTRect( pToolBox->GetItemRect( m_nToolBoxId ) ),
        ( eAlign == WindowAlign::Top || eAlign == WindowAlign::Bottom ) ?
            css::awt::PopupMenuDirection::EXECUTE_DOWN :
            css::awt::PopupMenuDirection::EXECUTE_RIGHT );
    pToolBox->SetItemDown( m_nToolBoxId, false );

    if ( nId )
        functionExecuted( m_xPopupMenu->getCommand( nId ) );

    return xRet;
}

void PopupMenuToolbarController::functionExecuted( const OUString &/*rCommand*/)
{
}

ToolBoxItemBits PopupMenuToolbarController::getDropDownStyle() const
{
    return ToolBoxItemBits::DROPDOWN;
}

void PopupMenuToolbarController::createPopupMenuController()
{
    if( !m_bHasController )
        return;

    if ( !m_xPopupMenuController.is() )
    {
        css::uno::Sequence< css::uno::Any > aArgs( 2 );
        css::beans::PropertyValue aProp;

        aProp.Name = "Frame";
        aProp.Value <<= m_xFrame;
        aArgs[0] <<= aProp;

        aProp.Name = "ModuleIdentifier";
        aProp.Value <<= getModuleName();
        aArgs[1] <<= aProp;
        try
        {
            m_xPopupMenu.set(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.awt.PopupMenu", m_xContext ),
                        css::uno::UNO_QUERY_THROW );
            m_xPopupMenuController.set(
                m_xPopupMenuFactory->createInstanceWithArgumentsAndContext(
                    m_aPopupCommand, aArgs, m_xContext), css::uno::UNO_QUERY_THROW );

            m_xPopupMenuController->setPopupMenu( m_xPopupMenu );
        }
        catch ( const css::uno::Exception &e )
        {
            m_xPopupMenu.clear();
            SAL_INFO( "fwk.uielement", "Caught an exception: " << e.Message );
        }
    }
}

class WizardsToolbarController : public PopupMenuToolbarController
{
public:
    WizardsToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) throw (css::uno::RuntimeException) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException) override;

private:
    ToolBoxItemBits getDropDownStyle() const override;
};

WizardsToolbarController::WizardsToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : PopupMenuToolbarController( xContext )
{
}

OUString WizardsToolbarController::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return OUString("org.apache.openoffice.comp.framework.WizardsToolbarController");
}

sal_Bool WizardsToolbarController::supportsService(OUString const & rServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence<OUString> WizardsToolbarController::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.frame.ToolbarController";
    return aRet;
}

ToolBoxItemBits WizardsToolbarController::getDropDownStyle() const
{
    return ToolBoxItemBits::DROPDOWNONLY;
}

class OpenToolbarController : public PopupMenuToolbarController
{
public:
    OpenToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) throw (css::uno::RuntimeException) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException) override;
};

OpenToolbarController::OpenToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : PopupMenuToolbarController( xContext, UNO_COMMAND_RECENT_FILE_LIST )
{
}

OUString OpenToolbarController::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return OUString("org.apache.openoffice.comp.framework.OpenToolbarController");
}

sal_Bool OpenToolbarController::supportsService(OUString const & rServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence<OUString> OpenToolbarController::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.frame.ToolbarController";
    return aRet;
}

class NewToolbarController : public PopupMenuToolbarController
{
public:
    NewToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) throw (css::uno::RuntimeException) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException) override;

    void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

private:
    void functionExecuted( const OUString &rCommand ) override;
    void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL execute( sal_Int16 KeyModifier ) throw (css::uno::RuntimeException, std::exception) override;
    void setItemImage( const OUString &rCommand );

    OUString m_aLastURL;
};

NewToolbarController::NewToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : PopupMenuToolbarController( xContext )
{
}

OUString NewToolbarController::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return OUString("org.apache.openoffice.comp.framework.NewToolbarController");
}

sal_Bool NewToolbarController::supportsService(OUString const & rServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence<OUString> NewToolbarController::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.frame.ToolbarController";
    return aRet;
}

void SAL_CALL NewToolbarController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    PopupMenuToolbarController::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    createPopupMenuController();
}

void SAL_CALL NewToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.IsEnabled )
    {
        OUString aState;
        rEvent.State >>= aState;
        try
        {
            // set the image even if the state is not a string
            // this will set the image of the default module
            setItemImage( aState );
        }
        catch (const css::ucb::CommandFailedException&)
        {
        }
        catch (const css::ucb::ContentCreationException&)
        {
        }
    }

    enable( rEvent.IsEnabled );
}

void SAL_CALL NewToolbarController::execute( sal_Int16 /*KeyModifier*/ )
throw ( css::uno::RuntimeException, std::exception )
{
    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_aLastURL.getLength() )
        return;

    OUString aTarget( "_default" );
    if ( m_xPopupMenu.is() )
    {
        // TODO investigate how to wrap Get/SetUserValue in css::awt::XMenu
        MenuAttributes* pMenuAttributes( 0 );
        VCLXPopupMenu*  pTkPopupMenu =
            static_cast<VCLXPopupMenu *>( VCLXMenu::GetImplementation( m_xPopupMenu ) );

        SolarMutexGuard aSolarMutexGuard;
        PopupMenu* pVCLPopupMenu = pTkPopupMenu ?
            dynamic_cast< PopupMenu * >( pTkPopupMenu->GetMenu() ) : NULL;

        if ( pVCLPopupMenu )
            pMenuAttributes = reinterpret_cast< MenuAttributes* >(
                pVCLPopupMenu->GetUserValue( pVCLPopupMenu->GetCurItemId() ) );

        if ( pMenuAttributes )
            aTarget = pMenuAttributes->aTargetFrame;
    }

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = "Referer";
    aArgs[0].Value <<= OUString( "private:user" );

    dispatchCommand( m_aLastURL, aArgs, aTarget );
}

void NewToolbarController::functionExecuted( const OUString &rCommand )
{
    setItemImage( rCommand );
}

/**
    it return the existing state of the given URL in the popupmenu of this toolbox control.

    If the given URL can be located as an action command of one menu item of the
    popup menu of this control, we return sal_True. Otherwhise we return sal_False.
    Further we return a fallback URL, in case we have to return sal_False. Because
    the outside code must select a valid item of the popup menu every time ...
    and we define it here. By the way this method was written to handle
    error situations gracefully. E.g. it can be called during creation time
    but then we have no valid menu. For this case we know another fallback URL.
    Then we return the private:factory/ URL of the default factory.

    @param  rPopupMenu
                points to the popup menu, on which item we try to locate the given URL
                Can be NULL! Search will be suppressed then.

    @param  sURL
                the URL for searching

    @param  sFallback
                contains the fallback URL in case we return FALSE
                Must point to valid memory!

    @param  aImage
                contains the image of the menu for the URL.

    @return sal_True - if URL could be located as an item of the popup menu.
            sal_False - otherwhise.
*/
static bool Impl_ExistURLInMenu(
    const css::uno::Reference< css::awt::XPopupMenu > &rPopupMenu,
    OUString &sURL,
    OUString &sFallback,
    Image &aImage )
{
    bool bValidFallback( false );
    sal_uInt16 nCount( 0 );
    if ( rPopupMenu.is() && ( nCount = rPopupMenu->getItemCount() ) != 0 && sURL.getLength() )
    {
        for ( sal_uInt16 n = 0; n < nCount; ++n )
        {
            sal_uInt16 nId = rPopupMenu->getItemId( n );
            OUString aCmd( rPopupMenu->getCommand( nId ) );

            if ( !bValidFallback && aCmd.getLength() )
            {
                sFallback = aCmd;
                bValidFallback = true;
            }

            // match even if the menu command is more detailed
            // (maybe an additional query) #i28667#
            if ( aCmd.match( sURL ) )
            {
                sURL = aCmd;
                const css::uno::Reference< css::graphic::XGraphic > xGraphic(
                    rPopupMenu->getItemImage( nId ) );
                if ( xGraphic.is() )
                    aImage = Image( xGraphic );
                return true;
            }
        }
    }

    if ( !bValidFallback )
    {
        OUStringBuffer aBuffer;
        aBuffer.append( "private:factory/" );
        aBuffer.append( SvtModuleOptions().GetDefaultModuleName() );
        sFallback = aBuffer.makeStringAndClear();
    }

    return false;
}

/** We accept URL's here only, which exist as items of our internal popup menu.
    All other ones will be ignored and a fallback is used.
 */
void NewToolbarController::setItemImage( const OUString &rCommand )
{
    SolarMutexGuard aSolarLock;
    VclPtr< ToolBox> pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ).get() );
    if ( !pToolBox )
        return;

    OUString aURL = rCommand;
    OUString sFallback;
    Image aMenuImage;

    bool bValid( Impl_ExistURLInMenu( m_xPopupMenu, aURL, sFallback, aMenuImage ) );
    if ( !bValid )
        aURL = sFallback;

    bool bBig = SvtMiscOptions().AreCurrentSymbolsLarge();

    INetURLObject aURLObj( aURL );
    Image aImage = SvFileInformationManager::GetImageNoDefault( aURLObj, bBig );
    if ( !aImage )
        aImage = !!aMenuImage ?
            aMenuImage :
            SvFileInformationManager::GetImage( aURLObj, bBig );

    // if everything failed, just use the image associated with the toolbar item command
    if ( !aImage )
        return;

    Size aBigSize( pToolBox->GetDefaultImageSize() );
    if ( bBig && aImage.GetSizePixel() != aBigSize )
    {
        BitmapEx aScaleBmpEx( aImage.GetBitmapEx() );
        aScaleBmpEx.Scale( aBigSize, BmpScaleFlag::Interpolate );
        pToolBox->SetItemImage( m_nToolBoxId, Image( aScaleBmpEx ) );
    }
    else
        pToolBox->SetItemImage( m_nToolBoxId, aImage );

    m_aLastURL = aURL;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_apache_openoffice_comp_framework_WizardsToolbarController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new WizardsToolbarController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_apache_openoffice_comp_framework_OpenToolbarController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OpenToolbarController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
org_apache_openoffice_comp_framework_NewToolbarController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new NewToolbarController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
