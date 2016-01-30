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

#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakref.hxx>
#include <framework/imageproducer.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/gen.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <com/sun/star/awt/XDockableWindow.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/theUIElementFactoryManager.hpp>

typedef cppu::ImplInheritanceHelper< svt::ToolboxController,
                                     css::frame::XSubToolbarController,
                                     css::awt::XDockableWindowListener,
                                     css::lang::XServiceInfo > ToolBarBase;

class SubToolBarController : public ToolBarBase
{
    OUString m_aSubTbName;
    OUString m_aLastCommand;
    css::uno::Reference< css::ui::XUIElement > m_xUIElement;
    void disposeUIElement();
public:
    explicit SubToolBarController( const css::uno::Sequence< css::uno::Any >& rxArgs );
    virtual ~SubToolBarController();

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& rxArgs ) throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XToolbarController
    virtual void SAL_CALL execute( sal_Int16 nKeyModifier ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw ( css::uno::RuntimeException, std::exception ) override;

    // XSubToolbarController
    virtual sal_Bool SAL_CALL opensSubToolbar() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getSubToolbarName() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL functionSelected( const OUString& rCommand ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL updateImage() throw ( css::uno::RuntimeException, std::exception ) override;

    //  XDockableWindowListener
    virtual void SAL_CALL startDocking( const css::awt::DockingEvent& e ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::awt::DockingData SAL_CALL docking( const css::awt::DockingEvent& e ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endDocking( const css::awt::EndDockingEvent& e ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL prepareToggleFloatingMode( const css::lang::EventObject& e ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL toggleFloatingMode( const css::lang::EventObject& e ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL closed( const css::lang::EventObject& e ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL endPopupMode( const css::awt::EndPopupModeEvent& e ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& e ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XComponent
    virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( css::uno::RuntimeException ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw ( css::uno::RuntimeException ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( css::uno::RuntimeException ) override;
};

SubToolBarController::SubToolBarController( const css::uno::Sequence< css::uno::Any >& rxArgs )
{
    css::beans::PropertyValue aPropValue;
    for ( sal_Int32 i = 0; i < rxArgs.getLength(); ++i )
    {
        rxArgs[i] >>= aPropValue;
        if ( aPropValue.Name == "Value" )
        {
            OUString aValue;
            aPropValue.Value >>= aValue;
            m_aSubTbName = aValue.getToken(0, ';');
            m_aLastCommand = aValue.getToken(1, ';');
            break;
        }
    }
    if ( !m_aLastCommand.isEmpty() )
        addStatusListener( m_aLastCommand );
}

SubToolBarController::~SubToolBarController()
{
    disposeUIElement();
    m_xUIElement = nullptr;
}

void SubToolBarController::disposeUIElement()
{
    if ( m_xUIElement.is() )
    {
        css::uno::Reference< css::lang::XComponent > xComponent( m_xUIElement, css::uno::UNO_QUERY );
        xComponent->dispose();
    }
}

void SubToolBarController::statusChanged( const css::frame::FeatureStateEvent& Event )
    throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
    {
        ToolBoxItemBits nItemBits = pToolBox->GetItemBits( nId );
        nItemBits &= ~ToolBoxItemBits::CHECKABLE;
        TriState eTri = TRISTATE_FALSE;

        if ( Event.FeatureURL.Complete == m_aCommandURL )
        {
            pToolBox->EnableItem( nId, Event.IsEnabled );

            OUString aStrValue;
            css::frame::status::Visibility aItemVisibility;
            if ( Event.State >>= aStrValue )
            {
                // Enum command, such as the current custom shape,
                // toggle checked state.
                if ( m_aLastCommand == ( m_aCommandURL + "." + aStrValue ) )
                {
                    eTri = TRISTATE_TRUE;
                    nItemBits |= ToolBoxItemBits::CHECKABLE;
                }
            }
            else if ( Event.State >>= aItemVisibility )
            {
                pToolBox->ShowItem( nId, aItemVisibility.bVisible );
            }
        }
        else
        {
            bool bValue;
            if ( Event.State >>= bValue )
            {
                // Boolean, treat it as checked/unchecked
                if ( bValue )
                    eTri = TRISTATE_TRUE;
                nItemBits |= ToolBoxItemBits::CHECKABLE;
            }
        }

        pToolBox->SetItemState( nId, eTri );
        pToolBox->SetItemBits( nId, nItemBits );
    }
}

void SubToolBarController::execute( sal_Int16 nKeyModifier )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( !m_aLastCommand.isEmpty() )
    {
        auto aArgs( comphelper::InitPropertySequence( {
            { "KeyModifier", css::uno::makeAny( nKeyModifier ) }
        } ) );
        dispatchCommand( m_aLastCommand, aArgs );
    }
}

css::uno::Reference< css::awt::XWindow > SubToolBarController::createPopupWindow()
    throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
    {
        css::uno::Reference< css::frame::XFrame > xFrame ( getFrameInterface() );

        // create element with factory
        static css::uno::WeakReference< css::ui::XUIElementFactoryManager > xWeakUIElementFactory;
        css::uno::Reference< css::ui::XUIElement > xUIElement;
        css::uno::Reference< css::ui::XUIElementFactoryManager > xUIElementFactory;

        xUIElementFactory = xWeakUIElementFactory;
        if ( !xUIElementFactory.is() )
        {
            xUIElementFactory = css::ui::theUIElementFactoryManager::get( m_xContext );
            xWeakUIElementFactory = xUIElementFactory;
        }

        auto aPropSeq( comphelper::InitPropertySequence( {
            { "Frame", css::uno::makeAny( xFrame ) },
            { "Persistent", css::uno::makeAny( false ) },
            { "PopupMode", css::uno::makeAny( true ) }
        } ) );

        try
        {
            xUIElement = xUIElementFactory->createUIElement( "private:resource/toolbar/" + m_aSubTbName, aPropSeq );
        }
        catch ( css::container::NoSuchElementException& )
        {}
        catch ( css::lang::IllegalArgumentException& )
        {}

        if ( xUIElement.is() )
        {
            css::uno::Reference< css::awt::XWindow > xParent = xFrame->getContainerWindow();
            css::uno::Reference< css::awt::XWindow > xSubToolBar( xUIElement->getRealInterface(), css::uno::UNO_QUERY );
            if ( xSubToolBar.is() )
            {
                css::uno::Reference< css::awt::XDockableWindow > xDockWindow( xSubToolBar, css::uno::UNO_QUERY );
                xDockWindow->addDockableWindowListener( css::uno::Reference< css::awt::XDockableWindowListener >(
                                                        static_cast< OWeakObject * >( this ), css::uno::UNO_QUERY ) );
                xDockWindow->enableDocking( sal_True );

                // keep reference to UIElement to avoid its destruction
                disposeUIElement();
                m_xUIElement = xUIElement;

                vcl::Window* pTbxWindow = VCLUnoHelper::GetWindow( xSubToolBar );
                if ( pTbxWindow && pTbxWindow->GetType() == WINDOW_TOOLBOX )
                {
                    ToolBox* pToolBar = static_cast< ToolBox* >( pTbxWindow );
                    pToolBar->SetParent( pToolBox );
                    // calc and set size for popup mode
                    Size aSize = pToolBar->CalcPopupWindowSizePixel();
                    pToolBar->SetSizePixel( aSize );
                    // open subtoolbox in popup mode
                    vcl::Window::GetDockingManager()->StartPopupMode( pToolBox, pToolBar );
                }
            }
        }
    }
    return css::uno::Reference< css::awt::XWindow >();
}

sal_Bool SubToolBarController::opensSubToolbar()
    throw ( css::uno::RuntimeException, std::exception )
{
    return !m_aLastCommand.isEmpty();
}

OUString SubToolBarController::getSubToolbarName()
    throw ( css::uno::RuntimeException, std::exception )
{
    return m_aSubTbName;
}

void SubToolBarController::functionSelected( const OUString& rCommand )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( !m_aLastCommand.isEmpty() && m_aLastCommand != rCommand )
    {
        removeStatusListener( m_aLastCommand );
        m_aLastCommand = rCommand;
        addStatusListener( m_aLastCommand );
        updateImage();
    }
}

void SubToolBarController::updateImage()
    throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if ( !m_aLastCommand.isEmpty() )
    {
        ToolBox* pToolBox = nullptr;
        sal_uInt16 nId = 0;
        if ( getToolboxId( nId, &pToolBox ) )
        {
            Image aImage = framework::GetImageFromURL( getFrameInterface(), m_aLastCommand, pToolBox->GetToolboxButtonSize() == TOOLBOX_BUTTONSIZE_LARGE );
            if ( !!aImage )
                pToolBox->SetItemImage( nId, aImage );
        }
    }
}

void SubToolBarController::startDocking( const css::awt::DockingEvent& )
    throw ( css::uno::RuntimeException, std::exception )
{
}

css::awt::DockingData SubToolBarController::docking( const css::awt::DockingEvent& )
    throw ( css::uno::RuntimeException, std::exception )
{
    return css::awt::DockingData();
}

void SubToolBarController::endDocking( const css::awt::EndDockingEvent& )
    throw ( css::uno::RuntimeException, std::exception )
{
}

sal_Bool SubToolBarController::prepareToggleFloatingMode( const css::lang::EventObject& )
    throw ( css::uno::RuntimeException, std::exception )
{
    return sal_False;
}

void SubToolBarController::toggleFloatingMode( const css::lang::EventObject& )
    throw ( css::uno::RuntimeException, std::exception )
{
}

void SubToolBarController::closed( const css::lang::EventObject& )
    throw ( css::uno::RuntimeException, std::exception )
{
}

void SubToolBarController::endPopupMode( const css::awt::EndPopupModeEvent& e )
    throw ( css::uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    OUString aSubToolBarResName;
    if ( m_xUIElement.is() )
    {
        css::uno::Reference< css::beans::XPropertySet > xPropSet( m_xUIElement, css::uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                xPropSet->getPropertyValue("ResourceURL") >>= aSubToolBarResName;
            }
            catch ( css::beans::UnknownPropertyException& )
            {}
            catch ( css::lang::WrappedTargetException& )
            {}
        }
        disposeUIElement();
    }
    m_xUIElement = nullptr;

    // if the toolbar was teared-off recreate it and place it at the given position
    if( e.bTearoff )
    {
        css::uno::Reference< css::ui::XUIElement > xUIElement;
        css::uno::Reference< css::frame::XLayoutManager > xLayoutManager = getLayoutManager();

        if ( !xLayoutManager.is() )
            return;

        xLayoutManager->createElement( aSubToolBarResName );
        xUIElement = xLayoutManager->getElement( aSubToolBarResName );
        if ( xUIElement.is() )
        {
            css::uno::Reference< css::awt::XWindow > xParent = getFrameInterface()->getContainerWindow();
            css::uno::Reference< css::awt::XWindow > xSubToolBar( xUIElement->getRealInterface(), css::uno::UNO_QUERY );
            css::uno::Reference< css::beans::XPropertySet > xProp( xUIElement, css::uno::UNO_QUERY );
            if ( xSubToolBar.is() && xProp.is() )
            {
                OUString aPersistentString( "Persistent" );
                try
                {
                    vcl::Window*  pTbxWindow = VCLUnoHelper::GetWindow( xSubToolBar );
                    if ( pTbxWindow && pTbxWindow->GetType() == WINDOW_TOOLBOX )
                    {
                        css::uno::Any a = xProp->getPropertyValue( aPersistentString );
                        xProp->setPropertyValue( aPersistentString, css::uno::makeAny( false ) );

                        xLayoutManager->hideElement( aSubToolBarResName );
                        xLayoutManager->floatWindow( aSubToolBarResName );

                        xLayoutManager->setElementPos( aSubToolBarResName, e.FloatingPosition );
                        xLayoutManager->showElement( aSubToolBarResName );

                        xProp->setPropertyValue("Persistent", a );
                    }
                }
                catch ( css::uno::RuntimeException& )
                {
                    throw;
                }
                catch ( css::uno::Exception& )
                {}
            }
        }
    }
}

void SubToolBarController::disposing( const css::lang::EventObject& e )
    throw ( css::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::disposing( e );
}

void SubToolBarController::initialize( const css::uno::Sequence< css::uno::Any >& rxArgs )
    throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::ToolboxController::initialize( rxArgs );

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
    {
        if ( m_aLastCommand.isEmpty() )
            pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
        else
            pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWN );
    }
    updateImage();
}

void SubToolBarController::dispose()
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( m_bDisposed )
        return;

    svt::ToolboxController::dispose();
    disposeUIElement();
    m_xUIElement = nullptr;
}

OUString SubToolBarController::getImplementationName()
    throw ( css::uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.framework.SubToolBarController" );
}

sal_Bool SubToolBarController::supportsService( const OUString& rServiceName )
    throw ( css::uno::RuntimeException )
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SubToolBarController::getSupportedServiceNames()
    throw ( css::uno::RuntimeException )
{
    css::uno::Sequence<OUString> aRet { "com.sun.star.frame.ToolbarController" };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_SubToolBarController_get_implementation(
    css::uno::XComponentContext*,
    css::uno::Sequence<css::uno::Any> const & rxArgs )
{
    return cppu::acquire( new SubToolBarController( rxArgs ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
