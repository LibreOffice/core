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

#include <uielement/popuptoolbarcontroller.hxx>
#include <framework/menuconfiguration.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/miscopt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/frame/PopupMenuControllerFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#define UNO_COMMAND_RECENT_FILE_LIST    ".uno:RecentFileList"
#define SFX_REFERER_USER                "private:user"

namespace css = ::com::sun::star;

namespace framework
{

PopupMenuToolbarController::PopupMenuToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const OUString &rPopupCommand )
    : svt::ToolboxController()
    , m_xContext( xContext )
    , m_bHasController( sal_False )
    , m_aPopupCommand( rPopupCommand )
{
}

PopupMenuToolbarController::~PopupMenuToolbarController()
{
}

void SAL_CALL PopupMenuToolbarController::dispose()
throw ( css::uno::RuntimeException )
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
throw ( css::uno::Exception, css::uno::RuntimeException )
{
    ToolboxController::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_aPopupCommand.getLength() )
        m_aPopupCommand = m_aCommandURL;

    try
    {
        m_xPopupMenuFactory.set(
            css::frame::PopupMenuControllerFactory::create( m_xContext ) );
        m_bHasController = m_xPopupMenuFactory->hasController(
            m_aPopupCommand, getModuleName() );
    }
    catch (const css::uno::Exception& e)
    {
        OSL_TRACE( "PopupMenuToolbarController - caught an exception! %s",
                   rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        (void) e;
    }

    SolarMutexGuard aSolarLock;
    ToolBox* pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
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

void SAL_CALL
PopupMenuToolbarController::statusChanged(
    const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException )
{
    // TODO move to base class

    svt::ToolboxController::statusChanged( rEvent );
    enable( rEvent.IsEnabled );
}

css::uno::Reference< css::awt::XWindow > SAL_CALL
PopupMenuToolbarController::createPopupWindow()
    throw ( css::uno::RuntimeException )
{
    css::uno::Reference< css::awt::XWindow > xRet;

    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_bHasController )
        return xRet;

    createPopupMenuController();

    SolarMutexGuard aSolarLock;
    ToolBox* pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
    if ( !pToolBox )
        return xRet;

    pToolBox->SetItemDown( m_nToolBoxId, sal_True );
    sal_uInt16 nId = m_xPopupMenu->execute(
        css::uno::Reference< css::awt::XWindowPeer >( getParent(), css::uno::UNO_QUERY ),
        VCLUnoHelper::ConvertToAWTRect( pToolBox->GetItemRect( m_nToolBoxId ) ),
        css::awt::PopupMenuDirection::EXECUTE_DEFAULT );
    pToolBox->SetItemDown( m_nToolBoxId, sal_False );

    if ( nId )
        functionExecuted( m_xPopupMenu->getCommand( nId ) );

    return xRet;
}

void PopupMenuToolbarController::functionExecuted( const OUString &/*rCommand*/)
{
}

sal_uInt16 PopupMenuToolbarController::getDropDownStyle() const
{
    return TIB_DROPDOWN;
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
            OSL_TRACE( "PopupMenuToolbarController - caught an exception! %s",
                       OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            (void) e;
        }
    }
}

DEFINE_XSERVICEINFO_MULTISERVICE_2( WizardsToolbarController,
                                    ::cppu::OWeakObject,
                                    "com.sun.star.frame.ToolbarController",
                                    OUString("org.apache.openoffice.comp.framework.WizardsToolbarController")
                                   )

DEFINE_INIT_SERVICE( WizardsToolbarController, {} )

WizardsToolbarController::WizardsToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : PopupMenuToolbarController( xContext )
{
}

sal_uInt16 WizardsToolbarController::getDropDownStyle() const
{
    return TIB_DROPDOWNONLY;
}

DEFINE_XSERVICEINFO_MULTISERVICE_2( OpenToolbarController,
                                    ::cppu::OWeakObject,
                                    "com.sun.star.frame.ToolbarController",
                                    OUString("org.apache.openoffice.comp.framework.OpenToolbarController")
                                   )

DEFINE_INIT_SERVICE( OpenToolbarController, {} )

OpenToolbarController::OpenToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : PopupMenuToolbarController( xContext, UNO_COMMAND_RECENT_FILE_LIST )
{
}


DEFINE_XSERVICEINFO_MULTISERVICE_2( NewToolbarController,
                                    ::cppu::OWeakObject,
                                    "com.sun.star.frame.ToolbarController",
                                    OUString("org.apache.openoffice.comp.framework.NewToolbarController")
                                   )

DEFINE_INIT_SERVICE( NewToolbarController, {} )

NewToolbarController::NewToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : PopupMenuToolbarController( xContext )
{
}

void SAL_CALL
NewToolbarController::initialize(
    const css::uno::Sequence< css::uno::Any >& aArguments )
throw ( css::uno::Exception, css::uno::RuntimeException )
{
    PopupMenuToolbarController::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    createPopupMenuController();
}

void SAL_CALL
NewToolbarController::statusChanged(
    const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException )
{
    if ( rEvent.IsEnabled )
    {
        OUString aState;
        rEvent.State >>= aState;
        // set the image even if the state is not a string
        // this will set the image of the default module
        setItemImage( aState );
    }

    enable( rEvent.IsEnabled );
}

void SAL_CALL
NewToolbarController::execute( sal_Int16 /*KeyModifier*/ )
    throw ( css::uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_aLastURL.getLength() )
        return;

    OUString aTarget( RTL_CONSTASCII_USTRINGPARAM( "_default" ) );
    if ( m_xPopupMenu.is() )
    {
        // TODO investigate how to wrap Get/SetUserValue in css::awt::XMenu
        MenuConfiguration::Attributes* pMenuAttributes( 0 );
        VCLXPopupMenu*  pTkPopupMenu =
            ( VCLXPopupMenu * ) VCLXMenu::GetImplementation( m_xPopupMenu );

        SolarMutexGuard aSolarMutexGuard;
        PopupMenu* pVCLPopupMenu = dynamic_cast< PopupMenu * >( pTkPopupMenu->GetMenu() );
        if ( pVCLPopupMenu )
            pMenuAttributes = reinterpret_cast< MenuConfiguration::Attributes* >(
                pVCLPopupMenu->GetUserValue( pVCLPopupMenu->GetCurItemId() ) );

        if ( pMenuAttributes )
            aTarget = pMenuAttributes->aTargetFrame;
    }

    css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer"  ));
    aArgs[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( SFX_REFERER_USER ) );

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
    the outside code must select a valid item of the popup menu everytime ...
    and we define it here. By the way this m ethod was written to handle
    error situations gracefully. E.g. it can be called during creation time
    but then we have no valid menu. For this case we know another fallback URL.
    Then we return the private:factory/ URL of the default factory.

    @param  rPopupMenu
                pounts to the popup menu, on which item we try to locate the given URL
                Can be NULL! Search will be supressed then.

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
static sal_Bool Impl_ExistURLInMenu(
    const css::uno::Reference< css::awt::XPopupMenu > &rPopupMenu,
    OUString &sURL,
    OUString &sFallback,
    Image &aImage )
{
    sal_Bool bValidFallback( sal_False );
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
                bValidFallback = sal_True;
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
                return sal_True;
            }
        }
    }

    if ( !bValidFallback )
    {
        OUStringBuffer aBuffer;
        aBuffer.appendAscii( RTL_CONSTASCII_STRINGPARAM( "private:factory/" ) );
        aBuffer.append( SvtModuleOptions().GetDefaultModuleName() );
        sFallback = aBuffer.makeStringAndClear();
    }

    return sal_False;
}

/** We accept URL's here only, which exist as items of our internal popup menu.
    All other ones will be ignored and a fallback is used.
 */
void NewToolbarController::setItemImage( const OUString &rCommand )
{
    SolarMutexGuard aSolarLock;
    ToolBox* pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
    if ( !pToolBox )
        return;

    OUString aURL = rCommand;
    OUString sFallback;
    Image aMenuImage;

    sal_Bool bValid( Impl_ExistURLInMenu( m_xPopupMenu, aURL, sFallback, aMenuImage ) );
    if ( !bValid )
        aURL = sFallback;

    sal_Bool bBig = SvtMiscOptions().AreCurrentSymbolsLarge();

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
        aScaleBmpEx.Scale( aBigSize, BMP_SCALE_INTERPOLATE );
        pToolBox->SetItemImage( m_nToolBoxId, Image( aScaleBmpEx ) );
    }
    else
        pToolBox->SetItemImage( m_nToolBoxId, aImage );

    m_aLastURL = aURL;
}


}
