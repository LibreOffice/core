/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// NOTE: Work in progress, most likely makes little sense

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XToolkitExperimental.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/view/XRenderable.hpp>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <org/libreoffice/touch/Document.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::osl::MutexGuard;

namespace org { namespace libreoffice { namespace touch
{

class DocumentImpl:
    public XDocument
{

private:
    OUString m_sURI;
    uno::Reference< uno::XComponentContext > m_rContext;
    uno::Reference< lang::XComponent > m_xComponent;
    uno::Reference< awt::XToolkitExperimental > m_xToolkit;
    uno::Reference< frame::XController > m_xController;

    // XRenderable.getRendererCount() and .render() need an XController in the
    // properties, at least in the test Java code it seemed that a totally
    // dummy one works, so try that here, too.

    typedef ::cppu::WeakImplHelper1< frame::XController > MyXController_Base;

    class MyXController:
        public MyXController_Base,
        public ::cppu::BaseMutex
    {
    private:
        uno::Reference< frame::XFrame > m_xFrame;
        uno::Reference< frame::XModel > m_xModel;

    public:
        virtual void SAL_CALL
        attachFrame( const uno::Reference< frame::XFrame >& xFrame )
            throw( uno::RuntimeException )
        {
            m_xFrame = xFrame;
        }

        virtual sal_Bool SAL_CALL
        attachModel( const uno::Reference< frame::XModel >& xModel )
            throw( uno::RuntimeException )
        {
            m_xModel = xModel;
            return sal_True;
        }

        virtual sal_Bool SAL_CALL
        suspend( sal_Bool /* bSuspend */ )
            throw( uno::RuntimeException )
        {
            return sal_False;
        }

        virtual uno::Any SAL_CALL getViewData()
            throw( uno::RuntimeException )
        {
            return uno::Any();
        }

        virtual void SAL_CALL restoreViewData( const uno::Any& /* data */ )
            throw ( uno::RuntimeException )
        {
        }

        virtual uno::Reference< frame::XModel > SAL_CALL
        getModel()
            throw ( uno::RuntimeException )
        {
            return m_xModel;
        }

        virtual uno::Reference< frame::XFrame > SAL_CALL
        getFrame()
            throw ( uno::RuntimeException )
        {
            return m_xFrame;
        }

        virtual void SAL_CALL
        dispose()
            throw ( uno::RuntimeException )
        {
        }

        virtual void SAL_CALL
        addEventListener( const uno::Reference< lang::XEventListener >& /* xListener */ )
            throw ( uno::RuntimeException )
        {
        }

        virtual void SAL_CALL
        removeEventListener( const uno::Reference< lang::XEventListener >& /* xListener */ )
            throw ( uno::RuntimeException )
        {
        }
    };

protected:
    DocumentImpl( const uno::Reference< uno::XComponentContext > context ):
        m_rContext( context )
    {
    }

    virtual ~DocumentImpl()
    {
    }

public:
    // XInitialization
    virtual void SAL_CALL
    initialize( const uno::Sequence< uno::Any >& arguments )
        throw ( uno::Exception, uno::RuntimeException )
    {
        if ( arguments.getLength() != 1 )
            throw lang::IllegalArgumentException( OUString(), static_cast<uno::Reference< uno::XInterface> >(this), 1 );

        uno::Sequence< beans::NamedValue > settings;
        if ( arguments[0] >>= m_sURI )
        {
            // create( [in] string uri );
            uno::Reference< frame::XDesktop > desktop( m_rContext->getServiceManager()->createInstanceWithContext( "com.sun.star.frame.Desktop", m_rContext ), uno::UNO_QUERY_THROW );
            uno::Reference< frame::XComponentLoader > componentLoader( desktop, uno::UNO_QUERY_THROW );

            (void) componentLoader;

            beans::PropertyValues loadProps(3);
            loadProps[0].Name = "Hidden";
            loadProps[0].Value <<= sal_Bool(true);
            loadProps[1].Name = "ReadOnly";
            loadProps[1].Value <<= sal_Bool(true);
            loadProps[2].Name = "Preview";
            loadProps[2].Value <<= sal_Bool(true);

            m_xComponent = componentLoader->loadComponentFromURL( m_sURI, "_blank", 0, loadProps );

            m_xToolkit = uno::Reference< awt::XToolkitExperimental >(  m_rContext->getServiceManager()->createInstanceWithContext( "com.sun.star.awt.ToolkitExperimental", m_rContext ), uno::UNO_QUERY_THROW );

            m_xController = new MyXController();
        }
    }

    // XDocument
    virtual sal_Int32 SAL_CALL
    getNumberOfPages()
        throw ( uno::RuntimeException )
    {
        uno::Any selection;
        selection <<= m_xComponent;

        uno::Reference< awt::XDevice > device;
        uno::Reference< view::XRenderable > renderable;

        beans::PropertyValues renderProps;

        device = m_xToolkit->createScreenCompatibleDevice( 128, 128 );

        renderable = uno::Reference< view::XRenderable >( m_rContext->getServiceManager()->createInstanceWithContext( "com.sun.star.view.Renderable", m_rContext ), uno::UNO_QUERY_THROW );

        renderProps.realloc( 3 );
        renderProps[0].Name = "IsPrinter";
        renderProps[0].Value <<= sal_Bool(true);
        renderProps[1].Name = "RenderDevice";
        renderProps[1].Value <<= device;
        renderProps[2].Name = "View";
        renderProps[2].Value <<= m_xController;

        return renderable->getRendererCount( selection, renderProps );
    }

    virtual void SAL_CALL
    render( sal_Int64 buffer,
            sal_Int32 width,
            sal_Int32 height,
            const uno::Reference< XDocumentRenderCallback >& listener,
            sal_Int32 pageNo,
            sal_Int32 zoomLevel,
            sal_Int32 x,
            sal_Int32 y )
        throw ( lang::IllegalArgumentException, uno::RuntimeException)
    {
        (void) listener;
        (void) zoomLevel;
        (void) x;
        (void) y;

        uno::Any selection;

        selection <<= m_xComponent;

        uno::Reference< awt::XDevice > device( m_xToolkit->createScreenCompatibleDeviceUsingBuffer( width, height, 1, 1, 0, 0, buffer ) );

        beans::PropertyValues renderProps;

        renderProps.realloc( 3 );
        renderProps[0].Name = "IsPrinter";
        renderProps[0].Value <<= sal_Bool(true);
        renderProps[1].Name = "RenderDevice";
        renderProps[1].Value <<= device;
        renderProps[2].Name = "View";
        renderProps[2].Value <<= m_xController;

        uno::Reference< view::XRenderable > renderable( m_rContext->getServiceManager()->createInstanceWithContext( "com.sun.star.view.Renderable", m_rContext ), uno::UNO_QUERY_THROW );

        renderable->render( pageNo, selection, renderProps );
    }

};

} } } // namespace org::libreoffice::touch

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
