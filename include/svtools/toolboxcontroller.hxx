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

#ifndef _SVTOOLS_TOOLBOXCONTROLLER_HXX
#define _SVTOOLS_TOOLBOXCONTROLLER_HXX

#include "svtools/svtdllapi.h"
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/propertycontainer.hxx>
#include <cppuhelper/propshlp.hxx>
#include <tools/link.hxx>

#include <boost/unordered_map.hpp>

class ToolBox;

namespace svt
{

class SVT_DLLPUBLIC ToolboxController : public ::com::sun::star::frame::XStatusListener,
                          public ::com::sun::star::frame::XToolbarController,
                          public ::com::sun::star::lang::XInitialization,
                          public ::com::sun::star::util::XUpdatable,
                          public ::com::sun::star::lang::XComponent,
                          public ::comphelper::OMutexAndBroadcastHelper,//shizhoubo
                          public ::comphelper::OPropertyContainer,//shizhoubo
                          public ::comphelper::OPropertyArrayUsageHelper< ToolboxController >,//shizhoubo
                          public ::cppu::OWeakObject
{
    private:
        sal_Bool  m_bSupportVisible; //shizhoubo
    public:
        ToolboxController( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext,
                           const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                           const OUString& aCommandURL );
        ToolboxController();
        virtual ~ToolboxController();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > getFrameInterface() const;
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& getContext() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager > getLayoutManager() const;

        void updateStatus( const OUString aCommandURL );
        void updateStatus();

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
        using cppu::OPropertySetHelper::disposing;
        virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& Source ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException ) = 0;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL doubleClick() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);
        // OPropertySetHelper //shizhoubo
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any& rValue ) throw(com::sun::star::uno::Exception);
        virtual sal_Bool SAL_CALL convertFastPropertyValue( com::sun::star::uno::Any& rConvertedValue, com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const com::sun::star::uno::Any& rValue) throw(com::sun::star::lang::IllegalArgumentException);
        // XPropertySet //shizhoubo
        virtual ::com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        // OPropertyArrayUsageHelper //shizhoubo
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;


        const OUString& getCommandURL() const { return  m_aCommandURL; }
        const OUString& getModuleName() const { return m_sModuleName; }

        void dispatchCommand( const OUString& sCommandURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs, const OUString &rTarget = OUString() );

        void enable( bool bEnable );

    protected:
        bool getToolboxId( sal_uInt16& rItemId, ToolBox** ppToolBox );
        void setSupportVisibleProperty(sal_Bool bValue); //shizhoubo
        struct Listener
        {
            Listener( const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& rDispatch ) :
                aURL( rURL ), xDispatch( rDispatch ) {}

            ::com::sun::star::util::URL aURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatch;
        };

        struct DispatchInfo
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > mxDispatch;
            const ::com::sun::star::util::URL maURL;
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > maArgs;

            DispatchInfo( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& xDispatch,
                          const ::com::sun::star::util::URL& rURL,
                          const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs )
                : mxDispatch( xDispatch )
                , maURL( rURL )
                , maArgs( rArgs )
                {}
        };

        DECL_STATIC_LINK( ToolboxController, ExecuteHdl_Impl, DispatchInfo* );

        typedef ::boost::unordered_map< OUString,
                                 com::sun::star::uno::Reference< com::sun::star::frame::XDispatch >,
                                 OUStringHash,
                                 ::std::equal_to< OUString > > URLToDispatchMap;

        // methods to support status forwarder, known by the old sfx2 toolbox controller implementation
        void addStatusListener( const OUString& aCommandURL );
        void removeStatusListener( const OUString& aCommandURL );
        void bindListener();
        void unbindListener();
        sal_Bool isBound() const;
        // TODO remove
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > getURLTransformer() const;
        // TODO remove
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > getParent() const;

        sal_Bool                                                                            m_bInitialized : 1,
                                                                                            m_bDisposed : 1;
        sal_uInt16                                                                          m_nToolBoxId;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
        OUString                                                                            m_aCommandURL;
        URLToDispatchMap                                                                    m_aListenerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                                          m_aListenerContainer;   /// container for ALL Listener

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >          m_xParentWindow;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xUrlTransformer;
        OUString m_sModuleName;
};

}

#endif // _SVTOOLS_TOOLBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
