/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVTOOLS_STATUSBARCONTROLLER_HXX
#define _SVTOOLS_STATUSBARCONTROLLER_HXX

#include "svtools/svtdllapi.h"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XStatusbarController.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/broadcasthelper.hxx>

#ifndef INCLUDED_HASH_MAP
#include <hash_map>
#define INCLUDED_HASH_MAP
#endif

#include <tools/gen.hxx>

namespace svt
{

class SVT_DLLPUBLIC StatusbarController : public ::com::sun::star::frame::XStatusListener,
                            public ::com::sun::star::frame::XStatusbarController,
                            public ::com::sun::star::lang::XInitialization,
                            public ::com::sun::star::util::XUpdatable,
                            public ::com::sun::star::lang::XComponent,
                            public ::comphelper::OBaseMutex,
                            public ::cppu::OWeakObject
{
    public:
        StatusbarController( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                             const rtl::OUString& aCommandURL,
                             unsigned short       nID );
        StatusbarController();
        virtual ~StatusbarController();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > getFrameInterface() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getServiceManager() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > getLayoutManager() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > getURLTransformer() const;

        void updateStatus( const rtl::OUString aCommandURL );
        void updateStatus();

        ::Rectangle getControlRect() const;

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUpdatable
        virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusbarController
        virtual ::sal_Bool SAL_CALL mouseButtonDown( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseMove( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL mouseButtonUp( const ::com::sun::star::awt::MouseEvent& aMouseEvent ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL command( const ::com::sun::star::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       ::sal_Bool bMouseEvent,
                                       const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL paint( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
                                     const ::com::sun::star::awt::Rectangle& rOutputRectangle,
                                     ::sal_Int32 nItemId, ::sal_Int32 nStyle ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);

    protected:
        struct Listener
        {
            Listener( const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& rDispatch ) :
                aURL( rURL ), xDispatch( rDispatch ) {}

            ::com::sun::star::util::URL aURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatch;
        };

        typedef ::std::hash_map< ::rtl::OUString,
                                 com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >,
                                 ::rtl::OUStringHash,
                                 ::std::equal_to< ::rtl::OUString > > URLToDispatchMap;

        // methods to support status forwarder, known by the old sfx2 toolbox controller implementation
        void addStatusListener( const rtl::OUString& aCommandURL );
        void removeStatusListener( const rtl::OUString& aCommandURL );
        void bindListener();
        void unbindListener();
        sal_Bool isBound() const;

        // execute methods
        // execute bound status bar controller command/execute various commands
        void execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );
        void execute( const rtl::OUString& aCommand, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );

        sal_Bool                                                                            m_bInitialized : 1,
                                                                                            m_bDisposed : 1;
        unsigned short                                                                      m_nID;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                  m_xParentWindow;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceManager;
        rtl::OUString                                                                       m_aCommandURL;
        URLToDispatchMap                                                                    m_aListenerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                                          m_aListenerContainer;   /// container for ALL Listener
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xURLTransformer;
};

}

#endif // _SVTOOLS_TOOLBOXCONTROLLER_HXX
