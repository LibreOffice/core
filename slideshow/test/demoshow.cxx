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

#include <rtl/ref.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/presentation/SlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#include <com/sun/star/animations/TransitionSubType.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <cppcanvas/vclfactory.hxx>
#include <cppcanvas/basegfxfactory.hxx>
#include <cppcanvas/polypolygon.hxx>

#include <canvas/canvastools.hxx>

#include <vcl/dialog.hxx>
#include <vcl/timer.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

#include <stdio.h>
#include <unistd.h>


using namespace ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper< presentation::XSlideShowView > ViewBase;
class View : public ::cppu::BaseMutex,
             public ViewBase
{
public:
    explicit View( const uno::Reference< rendering::XSpriteCanvas >& rCanvas ) :
        ViewBase( m_aMutex ),
        mxCanvas( rCanvas ),
        maPaintListeners( m_aMutex ),
        maTransformationListeners( m_aMutex ),
        maMouseListeners( m_aMutex ),
        maMouseMotionListeners( m_aMutex ),
        maTransform(),
        maSize()
    {
    }

    void resize( const ::Size& rNewSize )
    {
        maSize = rNewSize;
        const sal_Int32 nSize( std::min( rNewSize.Width(), rNewSize.Height() ) - 10);
        maTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(
            nSize, nSize, (rNewSize.Width() - nSize) / 2, (rNewSize.Height() - nSize) / 2);

        lang::EventObject aEvent( *this );
        maTransformationListeners.notifyEach( &util::XModifyListener::modified,
                                              aEvent );
    }

    void repaint()
    {
        awt::PaintEvent aEvent( *this,
                                awt::Rectangle(),
                                0 );
        maPaintListeners.notifyEach( &awt::XPaintListener::windowPaint,
                                     aEvent );
    }

private:
    virtual ~View() {}

    virtual uno::Reference< rendering::XSpriteCanvas > SAL_CALL getCanvas(  ) throw (uno::RuntimeException)
    {
        return mxCanvas;
    }

    virtual void SAL_CALL clear(  ) throw (uno::RuntimeException)
    {
        ::basegfx::B2DPolygon aPoly( ::basegfx::utils::createPolygonFromRect(
                                         ::basegfx::B2DRectangle(0.0,0.0,
                                                                 maSize.Width(),
                                                                 maSize.Height() )));
        ::cppcanvas::SpriteCanvasSharedPtr pCanvas(
            ::cppcanvas::VCLFactory::getInstance().createSpriteCanvas( mxCanvas ));
        if( !pCanvas )
            return;

        ::cppcanvas::PolyPolygonSharedPtr pPolyPoly(
            ::cppcanvas::BaseGfxFactory::getInstance().createPolyPolygon( pCanvas,
                                                                          aPoly ) );
        if( !pPolyPoly )
            return;

        if( pPolyPoly )
        {
            pPolyPoly->setRGBAFillColor( 0x808080FFU );
            pPolyPoly->draw();
        }
    }

    virtual geometry::AffineMatrix2D SAL_CALL getTransformation(  ) throw (uno::RuntimeException)
    {
        geometry::AffineMatrix2D aRes;
        return basegfx::unotools::affineMatrixFromHomMatrix( aRes,
                                                             maTransform );
    }

    virtual void SAL_CALL addTransformationChangedListener( const uno::Reference< util::XModifyListener >& xListener ) throw (uno::RuntimeException)
    {
        maTransformationListeners.addInterface( xListener );
    }

    virtual void SAL_CALL removeTransformationChangedListener( const uno::Reference< util::XModifyListener >& xListener ) throw (uno::RuntimeException)
    {
        maTransformationListeners.removeInterface( xListener );
    }

    virtual void SAL_CALL addPaintListener( const uno::Reference< awt::XPaintListener >& xListener ) throw (uno::RuntimeException)
    {
        maPaintListeners.addInterface( xListener );
    }

    virtual void SAL_CALL removePaintListener( const uno::Reference< awt::XPaintListener >& xListener ) throw (uno::RuntimeException)
    {
        maPaintListeners.removeInterface( xListener );
    }

    virtual void SAL_CALL addMouseListener( const uno::Reference< awt::XMouseListener >& xListener ) throw (uno::RuntimeException)
    {
        maMouseListeners.addInterface( xListener );
    }

    virtual void SAL_CALL removeMouseListener( const uno::Reference< awt::XMouseListener >& xListener ) throw (uno::RuntimeException)
    {
        maMouseListeners.removeInterface( xListener );
    }

    virtual void SAL_CALL addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener ) throw (uno::RuntimeException)
    {
        maMouseMotionListeners.addInterface( xListener );
    }

    virtual void SAL_CALL removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& xListener ) throw (uno::RuntimeException)
    {
        maMouseMotionListeners.removeInterface( xListener );
    }

    virtual void SAL_CALL setMouseCursor( ::sal_Int16 /*nPointerShape*/ ) throw (uno::RuntimeException)
    {
    }

    virtual awt::Rectangle SAL_CALL getCanvasArea(  ) throw (uno::RuntimeException)
    {
        return awt::Rectangle(0,0,maSize.Width(),maSize.Height());
    }

    uno::Reference< rendering::XSpriteCanvas > mxCanvas;
    ::comphelper::OInterfaceContainerHelper2          maPaintListeners;
    ::comphelper::OInterfaceContainerHelper2          maTransformationListeners;
    ::comphelper::OInterfaceContainerHelper2          maMouseListeners;
    ::comphelper::OInterfaceContainerHelper2          maMouseMotionListeners;
    basegfx::B2DHomMatrix                      maTransform;
    Size                                       maSize;
};

typedef ::cppu::WeakComponentImplHelper< drawing::XDrawPage,
                                          beans::XPropertySet > SlideBase;
class DummySlide : public ::cppu::BaseMutex,
                   public SlideBase
{
public:
    DummySlide() : SlideBase( m_aMutex ) {}

private:
    // XDrawPage
    virtual void SAL_CALL add( const uno::Reference< drawing::XShape >& /*xShape*/ ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL remove( const uno::Reference< drawing::XShape >& /*xShape*/ ) throw (uno::RuntimeException)
    {
    }

    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return 0;
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 /*Index*/ ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        return uno::Any();
    }

    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return uno::Type();
    }

    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return false;
    }

    // XPropertySet
    virtual uno::Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (uno::RuntimeException)
    {
        return uno::Reference< beans::XPropertySetInfo >();
    }

    virtual void SAL_CALL setPropertyValue( const OUString& /*aPropertyName*/,
                                            const uno::Any& /*aValue*/ ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
    {
    }

    virtual uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
    {
        typedef ::canvas::tools::ValueMap< sal_Int16 > PropMapT;

        // fixed PropertyValue map
        static const PropMapT::MapEntry lcl_propertyMap[] =
            {
                {"Height",               100},
                {"MinimalFrameNumber",   50},
                {"TransitionDuration",   10},
                {"TransitionSubtype",    animations::TransitionSubType::FROMTOPLEFT},
                {"TransitionType",       animations::TransitionType::PUSHWIPE},
                {"Width",                100}
            };

        static const PropMapT aMap( lcl_propertyMap,
                              SAL_N_ELEMENTS(lcl_propertyMap),
                              true );

        sal_Int16 aRes;
        if( !aMap.lookup( PropertyName, aRes ))
            return uno::Any();

        return uno::makeAny(aRes);
    }

    virtual void SAL_CALL addPropertyChangeListener( const OUString& /*aPropertyName*/,
                                                     const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removePropertyChangeListener( const OUString& /*aPropertyName*/,
                                                        const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
    {
    }

    virtual void SAL_CALL addVetoableChangeListener( const OUString& /*PropertyName*/,
                                                     const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removeVetoableChangeListener( const OUString& /*PropertyName*/,
                                                        const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
    {
    }
};


class DemoApp : public Application
{
public:
    virtual void Main();
    virtual void  Exception( ExceptionCategory nCategory );
};

class ChildWindow : public vcl::Window
{
public:
    explicit ChildWindow( vcl::Window* pParent );
    virtual ~ChildWindow();
    virtual void Paint( const Rectangle& rRect );
    virtual void Resize();

    void setShow( const uno::Reference< presentation::XSlideShow >& rShow ) { mxShow = rShow; init(); }

private:
    void init();

    rtl::Reference< View >                     mpView;
    uno::Reference< presentation::XSlideShow > mxShow;
};

ChildWindow::ChildWindow( vcl::Window* pParent ) :
    Window(pParent, WB_CLIPCHILDREN | WB_BORDER| WB_3DLOOK ),
    mpView(),
    mxShow()
{
    EnablePaint( true );
    Show();
}

ChildWindow::~ChildWindow()
{
    if( mxShow.is() && mpView.is() )
        mxShow->removeView( mpView.get() );
}

void ChildWindow::init()
{
    try
    {
        if( !mpView.is() )
        {
            uno::Reference< rendering::XCanvas > xCanvas( GetCanvas(),
                                                          uno::UNO_QUERY_THROW );
            uno::Reference< rendering::XSpriteCanvas > xSpriteCanvas( xCanvas,
                                                                      uno::UNO_QUERY_THROW );
            mpView = new View( xSpriteCanvas );
            mpView->resize( GetSizePixel() );

            if( mxShow.is() )
                mxShow->addView( mpView.get() );
        }
    }
    catch (const uno::Exception &e)
    {
        SAL_INFO("slideshow", e );
    }
}

void ChildWindow::Paint( const Rectangle& /*rRect*/ )
{
    try
    {
        if( mpView.is() )
            mpView->repaint();
    }
    catch (const uno::Exception &e)
    {
        SAL_INFO("slideshow", e );
    }
}

void ChildWindow::Resize()
{
    if( mpView.is() )
        mpView->resize( GetSizePixel() );
}

class DemoWindow : public Dialog
{
public:
    DemoWindow();
    virtual void Paint( const Rectangle& rRect );
    virtual void Resize();

private:
    void init();
    DECL_LINK( updateHdl, Timer*, void );

    ChildWindow                                maLeftChild;
    ChildWindow                                maRightTopChild;
    ChildWindow                                maRightBottomChild;
    uno::Reference< presentation::XSlideShow > mxShow;
    AutoTimer                                  maUpdateTimer;
    bool                                       mbSlideDisplayed;
};

DemoWindow::DemoWindow() :
    Dialog((vcl::Window*)NULL),
    maLeftChild( this ),
    maRightTopChild( this ),
    maRightBottomChild( this ),
    mxShow(),
    maUpdateTimer(),
    mbSlideDisplayed( false )
{
    SetText( OUString("Slideshow Demo" ) );
    SetSizePixel( Size( 640, 480 ) );
    EnablePaint( true );

    maLeftChild.SetPosSizePixel( Point(), Size(320,480) );
    maRightTopChild.SetPosSizePixel( Point(320,0), Size(320,240) );
    maRightBottomChild.SetPosSizePixel( Point(320,240), Size(320,240) );
    Show();

    maUpdateTimer.SetInvokeHandler(LINK(this, DemoWindow, updateHdl));
    maUpdateTimer.SetTimeout( (sal_uLong)30 );
    maUpdateTimer.Start();
}

void DemoWindow::init()
{
    try
    {
        if( !mxShow.is() )
        {
            uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            mxShow.set( presentation::SlideShow::create(xContext),
                        uno::UNO_QUERY_THROW );

            maLeftChild.setShow( mxShow );
            maRightTopChild.setShow( mxShow );
            maRightBottomChild.setShow( mxShow );
        }

        if( mxShow.is() && !mbSlideDisplayed )
        {
            uno::Reference< drawing::XDrawPage > xSlide( new DummySlide );
            uno::Reference< drawing::XDrawPages > xDrawPages;
            mxShow->displaySlide( xSlide,
                                  uno::Reference< drawing::XDrawPagesSupplier >(),
                                  uno::Reference< animations::XAnimationNode >(),
                                  uno::Sequence< beans::PropertyValue >() );
            mxShow->setProperty( beans::PropertyValue(
                                     OUString("RehearseTimings"),
                                     0,
                                     uno::makeAny( sal_True ),
                                     beans::PropertyState_DIRECT_VALUE ));
            mbSlideDisplayed = true;
        }
    }
    catch (const uno::Exception &e)
    {
        SAL_INFO("slideshow", e );
    }
}

IMPL_LINK_NOARG(DemoWindow, updateHdl, Timer*, void)
{
    init();

    if( mxShow.is() )
        mxShow->update(0);
}

void DemoWindow::Paint( const Rectangle& /*rRect*/ )
{
    init();
}

void DemoWindow::Resize()
{
    // TODO
}

void DemoApp::Exception( ExceptionCategory nCategory )
{
    switch( nCategory )
    {
        case ExceptionCategory::ResourceNotLoaded:
            Abort( "Error: could not load language resources.\nPlease check your installation.\n" );
            break;
    }
}

void DemoApp::Main()
{
    bool bHelp = false;

    for( sal_uInt16 i = 0; i < GetCommandLineParamCount(); i++ )
    {
        OUString aParam = GetCommandLineParam( i );

        if( aParam == "--help" || aParam == "-h" )
                bHelp = true;
    }

    if( bHelp )
    {
        printf( "demoshow - life Slideshow testbed\n" );
        return;
    }

    // bootstrap UNO
    uno::Reference< lang::XMultiServiceFactory > xFactory;
    try
    {
        uno::Reference< uno::XComponentContext > xCtx = ::cppu::defaultBootstrap_InitialComponentContext();
        xFactory.set(  xCtx->getServiceManager(), uno::UNO_QUERY );
        if( xFactory.is() )
            ::comphelper::setProcessServiceFactory( xFactory );
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {
        SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
    }

    if( !xFactory.is() )
    {
        SAL_INFO("slideshow", "Could not bootstrap UNO, installation must be in disorder. Exiting." );
        exit( 1 );
    }

    DemoWindow pWindow;
    pWindow.Execute();
}
}

DemoApp aApp;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
