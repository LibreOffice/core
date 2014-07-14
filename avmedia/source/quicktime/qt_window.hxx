/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef QT_WINDOW_HXX
#define QT_WINDOW_HXX

#include "qt_common.hxx"
#include <cppuhelper/interfacecontainer.h>

#include "com/sun/star/media/XPlayerWindow.hdl"

namespace avmedia { namespace quicktime {

// ---------------
// - Window -
// ---------------

class Player;

class Window : public ::cppu::WeakImplHelper2 < ::com::sun::star::media::XPlayerWindow,
                                                ::com::sun::star::lang::XServiceInfo >
{
public:

            Window( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_rxMgr,
                    Player& i_rPlayer,
                    NSView* i_pParentView
                    );
            ~Window();

    bool    create( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments );
    void    processGraphEvent();
    void    updatePointer();

    // XPlayerWindow
    virtual void SAL_CALL update(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setZoomLevel( ::com::sun::star::media::ZoomLevel ZoomLevel ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::media::ZoomLevel SAL_CALL getZoomLevel(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPointerType( sal_Int32 nPointerType ) throw (::com::sun::star::uno::RuntimeException);

    // XWindow
    virtual void SAL_CALL setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setVisible( sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEnable( sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFocus(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMgr;

    ::osl::Mutex                                maMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper  maListeners;
    ::com::sun::star::media::ZoomLevel          meZoomLevel;
    Player&                                     mrPlayer;
    int                                         mnPointerType;

    NSView*                                     mpParentView; // parent view for our own private movie view
    QTMovieView*                                mpMovieView;  // the view containing the movie object, output target and controller

    void                                        ImplLayoutVideoWindow();
};

} // namespace quicktime
} // namespace avmedia

#endif // QT_WINDOW_HXX

