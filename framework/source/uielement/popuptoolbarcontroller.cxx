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

#include <bitmaps.hlst>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertyvalue.hxx>
#include <menuconfiguration.hxx>
#include <svtools/imagemgr.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/thePopupMenuControllerFactory.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/util/XModifiable.hpp>

using namespace framework;

namespace
{

typedef cppu::ImplInheritanceHelper< svt::ToolboxController,
                                    css::lang::XServiceInfo >
                ToolBarBase;

class PopupMenuToolbarController : public ToolBarBase
{
public:
    // XComponent
    virtual void SAL_CALL dispose() override;
    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
    // XToolbarController
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;
    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

protected:
    PopupMenuToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                const OUString &rPopupCommand = OUString() );
    virtual void functionExecuted( const OUString &rCommand );
    virtual ToolBoxItemBits getDropDownStyle() const;
    void createPopupMenuController();

    bool                                                    m_bHasController;
    bool                                                    m_bResourceURL;
    OUString                                                m_aPopupCommand;
    css::uno::Reference< css::awt::XPopupMenu >             m_xPopupMenu;

private:
    css::uno::Reference< css::frame::XUIControllerFactory > m_xPopupMenuFactory;
    css::uno::Reference< css::frame::XPopupMenuController > m_xPopupMenuController;
};

PopupMenuToolbarController::PopupMenuToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const OUString &rPopupCommand )
    : ToolBarBase( xContext, css::uno::Reference< css::frame::XFrame >(), /*aCommandURL*/OUString() )
    , m_bHasController( false )
    , m_bResourceURL( false )
    , m_aPopupCommand( rPopupCommand )
{
}

void SAL_CALL PopupMenuToolbarController::dispose()
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
    catch (const css::uno::Exception&)
    {
        TOOLS_INFO_EXCEPTION( "fwk.uielement", "" );
    }

    if ( !m_bHasController && m_aPopupCommand.startsWith( "private:resource/" ) )
    {
        m_bResourceURL = true;
        m_bHasController = true;
    }

    SolarMutexGuard aSolarLock;
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nItemId;
    if ( getToolboxId( nItemId, &pToolBox ) )
    {
        ToolBoxItemBits nCurStyle( pToolBox->GetItemBits( nItemId ) );
        ToolBoxItemBits nSetStyle( getDropDownStyle() );
        pToolBox->SetItemBits( nItemId,
                               m_bHasController ?
                                    nCurStyle | nSetStyle :
                                    nCurStyle & ~nSetStyle );
    }

}

void SAL_CALL PopupMenuToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if ( m_bResourceURL )
        return;

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nItemId;
    if ( getToolboxId( nItemId, &pToolBox ) )
    {
        SolarMutexGuard aSolarLock;
        pToolBox->EnableItem( nItemId, rEvent.IsEnabled );
        bool bValue;
        if ( rEvent.State >>= bValue )
            pToolBox->CheckItem( nItemId, bValue );
    }
}

css::uno::Reference< css::awt::XWindow > SAL_CALL
PopupMenuToolbarController::createPopupWindow()
{
    css::uno::Reference< css::awt::XWindow > xRet;

    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_bHasController )
        return xRet;

    createPopupMenuController();

    SolarMutexGuard aSolarLock;
    VclPtr< ToolBox > pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
    if ( !pToolBox )
        return xRet;

    pToolBox->SetItemDown( m_nToolBoxId, true );
    WindowAlign eAlign( pToolBox->GetAlign() );

    // If the parent ToolBox is in popup mode (e.g. sub toolbar, overflow popup),
    // its ToolBarManager can be disposed along with our controller, destroying
    // m_xPopupMenu, while the latter still in execute. This should be fixed at a
    // different level, for now just hold it here so it won't crash.
    css::uno::Reference< css::awt::XPopupMenu > xPopupMenu ( m_xPopupMenu );
    sal_uInt16 nId = xPopupMenu->execute(
        css::uno::Reference< css::awt::XWindowPeer >( getParent(), css::uno::UNO_QUERY ),
        VCLUnoHelper::ConvertToAWTRect( pToolBox->GetItemRect( m_nToolBoxId ) ),
        ( eAlign == WindowAlign::Top || eAlign == WindowAlign::Bottom ) ?
            css::awt::PopupMenuDirection::EXECUTE_DOWN :
            css::awt::PopupMenuDirection::EXECUTE_RIGHT );
    pToolBox->SetItemDown( m_nToolBoxId, false );

    if ( nId )
        functionExecuted( xPopupMenu->getCommand( nId ) );

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

    if ( m_xPopupMenuController.is() )
    {
        m_xPopupMenuController->updatePopupMenu();
    }
    else
    {
        css::uno::Sequence<css::uno::Any> aArgs {
            css::uno::makeAny(comphelper::makePropertyValue("Frame", m_xFrame)),
            css::uno::makeAny(comphelper::makePropertyValue("ModuleIdentifier", m_sModuleName)),
            css::uno::makeAny(comphelper::makePropertyValue("InToolbar", true))
        };

        try
        {
            m_xPopupMenu.set(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    "com.sun.star.awt.PopupMenu", m_xContext ),
                        css::uno::UNO_QUERY_THROW );

            if (m_bResourceURL)
            {
                sal_Int32 nAppendIndex = aArgs.getLength();
                aArgs.realloc(nAppendIndex + 1);
                aArgs.getArray()[nAppendIndex] <<= comphelper::makePropertyValue("ResourceURL", m_aPopupCommand);

                m_xPopupMenuController.set( m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    "com.sun.star.comp.framework.ResourceMenuController", aArgs, m_xContext), css::uno::UNO_QUERY_THROW );
            }
            else
            {
                m_xPopupMenuController.set( m_xPopupMenuFactory->createInstanceWithArgumentsAndContext(
                    m_aPopupCommand, aArgs, m_xContext), css::uno::UNO_QUERY_THROW );
            }

            m_xPopupMenuController->setPopupMenu( m_xPopupMenu );
        }
        catch ( const css::uno::Exception & )
        {
            TOOLS_INFO_EXCEPTION( "fwk.uielement", "" );
            m_xPopupMenu.clear();
        }
    }
}

class GenericPopupToolbarController : public PopupMenuToolbarController
{
public:
    GenericPopupToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                   const css::uno::Sequence< css::uno::Any >& rxArgs );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& rxArgs ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    bool m_bSplitButton, m_bReplaceWithLast;
    void functionExecuted(const OUString &rCommand) override;
    ToolBoxItemBits getDropDownStyle() const override;
};

GenericPopupToolbarController::GenericPopupToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const css::uno::Sequence< css::uno::Any >& rxArgs )
    : PopupMenuToolbarController( xContext )
    , m_bReplaceWithLast( false )
{
    css::beans::PropertyValue aPropValue;
    for ( const auto& arg: rxArgs )
    {
        if ( ( arg >>= aPropValue ) && aPropValue.Name == "Value" )
        {
            sal_Int32 nIdx{ 0 };
            OUString aValue;
            aPropValue.Value >>= aValue;
            m_aPopupCommand = aValue.getToken(0, ';', nIdx);
            m_bReplaceWithLast = aValue.getToken(0, ';', nIdx).toBoolean();
            break;
        }
    }
    m_bSplitButton = m_bReplaceWithLast || !m_aPopupCommand.isEmpty();
}

OUString GenericPopupToolbarController::getImplementationName()
{
    return "com.sun.star.comp.framework.GenericPopupToolbarController";
}

sal_Bool GenericPopupToolbarController::supportsService(OUString const & rServiceName)
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence<OUString> GenericPopupToolbarController::getSupportedServiceNames()
{
    return {"com.sun.star.frame.ToolbarController"};
}

void GenericPopupToolbarController::initialize( const css::uno::Sequence< css::uno::Any >& rxArgs )
{
    PopupMenuToolbarController::initialize( rxArgs );
    if ( m_bReplaceWithLast )
        // Create early, so we can use the menu is statusChanged method.
        createPopupMenuController();
}

void GenericPopupToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    SolarMutexGuard aGuard;

    if ( m_bReplaceWithLast && !rEvent.IsEnabled && m_xPopupMenu.is() )
    {
        ToolBox* pToolBox = nullptr;
        ToolBoxItemId nId;
        if ( getToolboxId( nId, &pToolBox ) && pToolBox->IsItemEnabled( nId ) )
        {
            Menu* pVclMenu = comphelper::getFromUnoTunnel<VCLXMenu>( m_xPopupMenu )->GetMenu();
            pVclMenu->Activate();
            pVclMenu->Deactivate();
        }

        for (sal_uInt16 i = 0, nCount = m_xPopupMenu->getItemCount(); i < nCount; ++i )
        {
            sal_uInt16 nItemId = m_xPopupMenu->getItemId(i);
            if (nItemId && m_xPopupMenu->isItemEnabled(nItemId) && !m_xPopupMenu->getPopupMenu(nItemId).is())
            {
                functionExecuted(m_xPopupMenu->getCommand(nItemId));
                return;
            }
        }
    }

    PopupMenuToolbarController::statusChanged( rEvent );
}

void GenericPopupToolbarController::functionExecuted( const OUString& rCommand )
{
    if ( !m_bReplaceWithLast )
        return;

    removeStatusListener( m_aCommandURL );

    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rCommand, m_sModuleName);
    OUString aRealCommand( vcl::CommandInfoProvider::GetRealCommandForCommand(aProperties) );
    m_aCommandURL = aRealCommand.isEmpty() ? rCommand : aRealCommand;
    addStatusListener( m_aCommandURL );

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( getToolboxId( nId, &pToolBox ) )
    {
        pToolBox->SetItemCommand( nId, rCommand );
        pToolBox->SetHelpText( nId, OUString() ); // Will retrieve the new one from help.
        pToolBox->SetItemText(nId, vcl::CommandInfoProvider::GetLabelForCommand(aProperties));
        pToolBox->SetQuickHelpText(nId, vcl::CommandInfoProvider::GetTooltipForCommand(rCommand, aProperties, m_xFrame));

        Image aImage = vcl::CommandInfoProvider::GetImageForCommand(rCommand, m_xFrame, pToolBox->GetImageSize());
        if ( !!aImage )
            pToolBox->SetItemImage( nId, aImage );
    }
}

ToolBoxItemBits GenericPopupToolbarController::getDropDownStyle() const
{
    return m_bSplitButton ? ToolBoxItemBits::DROPDOWN : ToolBoxItemBits::DROPDOWNONLY;
}

class SaveToolbarController : public cppu::ImplInheritanceHelper< PopupMenuToolbarController,
                                                                  css::frame::XSubToolbarController,
                                                                  css::util::XModifyListener >
{
public:
    explicit SaveToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XSubToolbarController
    // Make ToolBarManager ask our controller for updated image, in case of icon theme change.
    virtual sal_Bool SAL_CALL opensSubToolbar() override;
    virtual OUString SAL_CALL getSubToolbarName() override;
    virtual void SAL_CALL functionSelected( const OUString& aCommand ) override;
    virtual void SAL_CALL updateImage() override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;

    // XModifyListener
    virtual void SAL_CALL modified( const css::lang::EventObject& rEvent ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( OUString const & rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

private:
    bool m_bReadOnly;
    bool m_bModified;
    css::uno::Reference< css::frame::XStorable > m_xStorable;
    css::uno::Reference< css::util::XModifiable > m_xModifiable;
};

SaveToolbarController::SaveToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
    : ImplInheritanceHelper( rxContext, ".uno:SaveAsMenu" )
    , m_bReadOnly( false )
    , m_bModified( false )
{
}

void SaveToolbarController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    PopupMenuToolbarController::initialize( aArguments );

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( !getToolboxId( nId, &pToolBox ) )
        return;

    css::uno::Reference< css::frame::XController > xController = m_xFrame->getController();
    if ( xController.is() )
        m_xModifiable.set( xController->getModel(), css::uno::UNO_QUERY );

    if ( m_xModifiable.is() && pToolBox->GetItemCommand( nId ) == m_aCommandURL )
        // Will also enable the save as only mode.
        m_xStorable.set( m_xModifiable, css::uno::UNO_QUERY );
    else if ( !m_xModifiable.is() )
        // Can be in table/query design.
        m_xModifiable.set( xController, css::uno::UNO_QUERY );
    else
        // Simple save button, without the dropdown.
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) & ~ ToolBoxItemBits::DROPDOWN );

    if ( m_xModifiable.is() )
    {
        m_xModifiable->addModifyListener( this );
        modified( css::lang::EventObject() );
    }
}

sal_Bool SaveToolbarController::opensSubToolbar()
{
    return true;
}

OUString SaveToolbarController::getSubToolbarName()
{
    return OUString();
}

void SaveToolbarController::functionSelected( const OUString& /*aCommand*/ )
{
}

void SaveToolbarController::updateImage()
{
    SolarMutexGuard aGuard;
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( !getToolboxId( nId, &pToolBox ) )
        return;

    vcl::ImageType eImageType = pToolBox->GetImageSize();

    Image aImage;

    if ( m_bReadOnly )
    {
        aImage = vcl::CommandInfoProvider::GetImageForCommand(".uno:SaveAs", m_xFrame, eImageType);
    }
    else if ( m_bModified )
    {
        if (eImageType == vcl::ImageType::Size26)
            aImage = Image(StockImage::Yes, BMP_SAVEMODIFIED_LARGE);
        else if (eImageType == vcl::ImageType::Size32)
            aImage = Image(StockImage::Yes, BMP_SAVEMODIFIED_EXTRALARGE);
        else
            aImage = Image(StockImage::Yes, BMP_SAVEMODIFIED_SMALL);
    }

    if ( !aImage )
        aImage = vcl::CommandInfoProvider::GetImageForCommand(m_aCommandURL, m_xFrame, eImageType);

    if ( !!aImage )
        pToolBox->SetItemImage( nId, aImage );
}

void SaveToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if ( !getToolboxId( nId, &pToolBox ) )
        return;

    bool bLastReadOnly = m_bReadOnly;
    m_bReadOnly = m_xStorable.is() && m_xStorable->isReadonly();
    if ( bLastReadOnly != m_bReadOnly )
    {
        OUString sCommand = m_bReadOnly ? OUString( ".uno:SaveAs" ) : m_aCommandURL;
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(sCommand,
            vcl::CommandInfoProvider::GetModuleIdentifier(m_xFrame));
        pToolBox->SetQuickHelpText( nId,
            vcl::CommandInfoProvider::GetTooltipForCommand(sCommand, aProperties, m_xFrame) );
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) & ~( m_bReadOnly ? ToolBoxItemBits::DROPDOWN : ToolBoxItemBits::DROPDOWNONLY ) );
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) |  ( m_bReadOnly ? ToolBoxItemBits::DROPDOWNONLY : ToolBoxItemBits::DROPDOWN ) );
        updateImage();
    }

    if ( !m_bReadOnly )
        pToolBox->EnableItem( nId, rEvent.IsEnabled );
}

void SaveToolbarController::modified( const css::lang::EventObject& /*rEvent*/ )
{
    bool bLastModified = m_bModified;
    m_bModified = m_xModifiable->isModified();
    if ( bLastModified != m_bModified )
        updateImage();
}

void SaveToolbarController::disposing( const css::lang::EventObject& rEvent )
{
    if ( rEvent.Source == m_xModifiable )
    {
        m_xModifiable.clear();
        m_xStorable.clear();
    }
    else
        PopupMenuToolbarController::disposing( rEvent );
}

void SaveToolbarController::dispose()
{
    PopupMenuToolbarController::dispose();
    if ( m_xModifiable.is() )
    {
        m_xModifiable->removeModifyListener( this );
        m_xModifiable.clear();
    }
    m_xStorable.clear();
}

OUString SaveToolbarController::getImplementationName()
{
    return "com.sun.star.comp.framework.SaveToolbarController";
}

sal_Bool SaveToolbarController::supportsService( OUString const & rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SaveToolbarController::getSupportedServiceNames()
{
    return {"com.sun.star.frame.ToolbarController"};
}

class NewToolbarController : public cppu::ImplInheritanceHelper<PopupMenuToolbarController, css::frame::XSubToolbarController>
{
public:
    explicit NewToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & rServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XSubToolbarController
    // Make ToolBarManager ask our controller for updated image, in case of icon theme change.
    sal_Bool SAL_CALL opensSubToolbar() override { return true; }
    OUString SAL_CALL getSubToolbarName() override { return OUString(); }
    void SAL_CALL functionSelected( const OUString& ) override {}
    void SAL_CALL updateImage() override;

private:
    void functionExecuted( const OUString &rCommand ) override;
    void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) override;
    void SAL_CALL execute( sal_Int16 KeyModifier ) override;
    sal_uInt16 getMenuIdForCommand( std::u16string_view rCommand );

    sal_uInt16 m_nMenuId;
};

NewToolbarController::NewToolbarController(
    const css::uno::Reference< css::uno::XComponentContext >& xContext )
    : ImplInheritanceHelper( xContext )
    , m_nMenuId( 0 )
{
}

OUString NewToolbarController::getImplementationName()
{
    return "org.apache.openoffice.comp.framework.NewToolbarController";
}

sal_Bool NewToolbarController::supportsService(OUString const & rServiceName)
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence<OUString> NewToolbarController::getSupportedServiceNames()
{
    return {"com.sun.star.frame.ToolbarController"};
}

void SAL_CALL NewToolbarController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    PopupMenuToolbarController::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    createPopupMenuController();
}

void SAL_CALL NewToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    if ( rEvent.IsEnabled )
    {
        OUString aState;
        rEvent.State >>= aState;
        try
        {
            // set the image even if the state is not a string
            // the toolbar item command will be used as a fallback
            functionExecuted( aState );
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
{
    osl::MutexGuard aGuard( m_aMutex );

    OUString aURL, aTarget;
    if ( m_xPopupMenu.is() && m_nMenuId )
    {
        SolarMutexGuard aSolarMutexGuard;
        aURL = m_xPopupMenu->getCommand(m_nMenuId);

        // TODO investigate how to wrap Get/SetUserValue in css::awt::XMenu
        VCLXMenu* pMenu = comphelper::getFromUnoTunnel<VCLXMenu>(m_xPopupMenu);
        MenuAttributes* pMenuAttributes(static_cast<MenuAttributes*>(pMenu->getUserValue(m_nMenuId)));
        if ( pMenuAttributes )
            aTarget = pMenuAttributes->aTargetFrame;
        else
            aTarget = "_default";
    }
    else
        aURL = m_aCommandURL;

    css::uno::Sequence< css::beans::PropertyValue > aArgs{ comphelper::makePropertyValue(
        "Referer", OUString( "private:user" )) };

    dispatchCommand( aURL, aArgs, aTarget );
}

void NewToolbarController::functionExecuted( const OUString &rCommand )
{
    m_nMenuId = getMenuIdForCommand( rCommand );
    updateImage();
}

sal_uInt16 NewToolbarController::getMenuIdForCommand( std::u16string_view rCommand )
{
    if ( m_xPopupMenu.is() && !rCommand.empty() )
    {
        sal_uInt16 nCount = m_xPopupMenu->getItemCount();
        for ( sal_uInt16 n = 0; n < nCount; ++n )
        {
            sal_uInt16 nId = m_xPopupMenu->getItemId(n);
            OUString aCmd(m_xPopupMenu->getCommand(nId));

            // match even if the menu command is more detailed
            // (maybe an additional query) #i28667#
            if ( aCmd.match( rCommand ) )
                return nId;
        }
    }

    return 0;
}

void SAL_CALL NewToolbarController::updateImage()
{
    SolarMutexGuard aSolarLock;
    VclPtr< ToolBox> pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );
    if ( !pToolBox )
        return;

    OUString aURL, aImageId;
    if ( m_xPopupMenu.is() && m_nMenuId )
    {
        aURL = m_xPopupMenu->getCommand(m_nMenuId);
        VCLXMenu* pMenu = comphelper::getFromUnoTunnel<VCLXMenu>(m_xPopupMenu);
        MenuAttributes* pMenuAttributes(static_cast<MenuAttributes*>(pMenu->getUserValue(m_nMenuId)));
        if ( pMenuAttributes )
            aImageId = pMenuAttributes->aImageId;
    }
    else
        aURL = m_aCommandURL;

    INetURLObject aURLObj( aImageId.isEmpty() ? aURL : aImageId );
    vcl::ImageType eImageType( pToolBox->GetImageSize() );
    Image aImage = SvFileInformationManager::GetImageNoDefault( aURLObj, eImageType );
    if ( !aImage )
        aImage = vcl::CommandInfoProvider::GetImageForCommand( aURL, m_xFrame, eImageType );

    if ( !aImage )
        return;

    pToolBox->SetItemImage( m_nToolBoxId, aImage );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_GenericPopupToolbarController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &args)
{
    return cppu::acquire(new GenericPopupToolbarController(context, args));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_SaveToolbarController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SaveToolbarController(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
org_apache_openoffice_comp_framework_NewToolbarController_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new NewToolbarController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
