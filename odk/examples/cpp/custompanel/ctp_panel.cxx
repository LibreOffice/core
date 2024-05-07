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


#include "ctp_panel.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <osl/diagnose.h>


namespace sd { namespace colortoolpanel
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::awt::XWindowPeer;
    using ::com::sun::star::lang::XMultiComponentFactory;
    using ::com::sun::star::awt::WindowDescriptor;
    using ::com::sun::star::awt::WindowClass_SIMPLE;
    using ::com::sun::star::awt::Rectangle;
    using ::com::sun::star::awt::PaintEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::awt::XDevice;
    using ::com::sun::star::awt::XGraphics;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::frame::XFrame;

    namespace WindowAttribute = ::com::sun::star::awt::WindowAttribute;
    namespace PosSize = ::com::sun::star::awt::PosSize;
    namespace UIElementType = ::com::sun::star::ui::UIElementType;


    //= helpers

    namespace
    {
        Reference< XWindow > lcl_createPlainWindow_nothrow( const Reference< XComponentContext >& i_rContext,
            const Reference< XWindowPeer >& i_rParentWindow )
        {
            try
            {
                OSL_ENSURE( i_rContext.is(), "illegal component context" );
                Reference< XMultiComponentFactory > xFactory( i_rContext->getServiceManager(), UNO_SET_THROW );
                Reference< XToolkit2 > xToolkit = Toolkit::create(i_rContext);

                WindowDescriptor aWindow;
                aWindow.Type = WindowClass_SIMPLE;
                aWindow.WindowServiceName = "window";
                aWindow.Parent = i_rParentWindow;
                aWindow.WindowAttributes = WindowAttribute::BORDER;

                Reference< XWindowPeer > xWindow( xToolkit->createWindow( aWindow ), UNO_SET_THROW );
                return Reference< XWindow >( xWindow, UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
            }
            return NULL;
        }
    }

    SingleColorPanel::SingleColorPanel( const Reference< XComponentContext >& i_rContext, const Reference< XWindow >& i_rParentWindow, const ::sal_Int32 i_nPanelColor )
        :SingleColorPanel_Base( m_aMutex )
        ,m_xWindow()
        ,m_nPanelColor( i_nPanelColor )
    {
        // retrieve the parent window for our to-be-created pane window
        Reference< XWindowPeer > xParentPeer( i_rParentWindow, UNO_QUERY );

        osl_atomic_increment( &m_refCount );
        if ( xParentPeer.is() )
        {
            m_xWindow = lcl_createPlainWindow_nothrow( i_rContext, xParentPeer );
            m_xWindow->addPaintListener( this );
            if ( m_xWindow.is() )
            {
                const Rectangle aPanelAnchorSize( i_rParentWindow->getPosSize() );
                m_xWindow->setPosSize( 0, 0, aPanelAnchorSize.Width, aPanelAnchorSize.Height, PosSize::POSSIZE );
                m_xWindow->setVisible( sal_True );
            }
        }
        osl_atomic_decrement( &m_refCount );
    }


    SingleColorPanel::~SingleColorPanel()
    {
    }


    Reference< XWindow > SAL_CALL SingleColorPanel::getWindow()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            throw DisposedException( OUString(), *this );
        return m_xWindow;
    }


    Reference< XAccessible > SAL_CALL SingleColorPanel::createAccessible( const Reference< XAccessible >& i_rParentAccessible )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            throw DisposedException( OUString(), *this );

        // TODO: the following is wrong, since it doesn't respect i_rParentAccessible. In a real extension, you should
        // implement this correctly :)
        (void)i_rParentAccessible;
        return Reference< XAccessible >( getWindow(), UNO_QUERY );
    }


    void SAL_CALL SingleColorPanel::windowPaint( const PaintEvent& i_rEvent )
    {
        try
        {
            const Reference< XDevice > xDevice( i_rEvent.Source, UNO_QUERY_THROW );
            const Reference< XGraphics > xGraphics( xDevice->createGraphics(), UNO_SET_THROW );
            xGraphics->setFillColor( m_nPanelColor );
            xGraphics->setLineColor( 0x00FFFFFF );

            const Reference< XWindow > xWindow( i_rEvent.Source, UNO_QUERY_THROW );
            const Rectangle aWindowRect( xWindow->getPosSize() );
            xGraphics->drawRect( 0, 0, aWindowRect.Width - 1, aWindowRect.Height - 1 );
        }
        catch( const Exception& )
        {
        }
    }


    void SAL_CALL SingleColorPanel::disposing( const EventObject& i_rSource )
    {
        (void)i_rSource;
    }


    void SAL_CALL SingleColorPanel::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xWindow.is() )
            // already disposed
            return;
        m_xWindow->removePaintListener( this );
        try
        {
            Reference< XComponent > xWindowComp( m_xWindow, UNO_QUERY_THROW );
            xWindowComp->dispose();
        }
        catch( const Exception& )
        {
        }
        m_xWindow.clear();
    }

    PanelUIElement::PanelUIElement( const Reference< XComponentContext >& i_rContext, const Reference< XWindow >& i_rParentWindow,
        const OUString& i_rResourceURL, const ::sal_Int32 i_nPanelColor )
        :PanelUIElement_Base( m_aMutex )
        ,m_sResourceURL( i_rResourceURL )
        ,m_xToolPanel( new SingleColorPanel( i_rContext, i_rParentWindow, i_nPanelColor ) )
    {
    }


    PanelUIElement::~PanelUIElement()
    {
    }


    Reference< XFrame > SAL_CALL PanelUIElement::getFrame()
    {
        // TODO
        return NULL;
    }


    OUString SAL_CALL PanelUIElement::getResourceURL()
    {
        return m_sResourceURL;
    }


    ::sal_Int16 SAL_CALL PanelUIElement::getType()
    {
        return UIElementType::TOOLPANEL;
    }


    Reference< XInterface > SAL_CALL PanelUIElement::getRealInterface(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_xToolPanel.is() )
            throw DisposedException();
        return m_xToolPanel;
    }


    void SAL_CALL PanelUIElement::disposing()
    {
        Reference< XComponent > xPanelComponent( m_xToolPanel, UNO_QUERY_THROW );
        m_xToolPanel.clear();
        xPanelComponent->dispose();
    }


} } // namespace sd::colortoolpanel


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
