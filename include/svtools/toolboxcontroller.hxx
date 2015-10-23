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

#ifndef INCLUDED_SVTOOLS_TOOLBOXCONTROLLER_HXX
#define INCLUDED_SVTOOLS_TOOLBOXCONTROLLER_HXX

#include <svtools/svtdllapi.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/propertycontainer.hxx>
#include <cppuhelper/propshlp.hxx>
#include <tools/link.hxx>

#include <unordered_map>

class ToolBox;

namespace svt
{

typedef cppu::WeakImplHelper5<
        css::frame::XStatusListener, css::frame::XToolbarController,
        css::lang::XInitialization, css::util::XUpdatable,
        css::lang::XComponent >
    ToolboxController_Base;

class SVT_DLLPUBLIC ToolboxController :
                          public ToolboxController_Base,
                          public ::comphelper::OMutexAndBroadcastHelper,
                          public ::comphelper::OPropertyContainer,
                          public ::comphelper::OPropertyArrayUsageHelper< ToolboxController >
{
    private:
        bool  m_bSupportVisible;
    public:
        ToolboxController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                           const css::uno::Reference< css::frame::XFrame >& xFrame,
                           const OUString& aCommandURL );
        ToolboxController();
        virtual ~ToolboxController();

        css::uno::Reference< css::frame::XFrame > getFrameInterface() const;
        const css::uno::Reference< css::uno::XComponentContext >& getContext() const;
        css::uno::Reference< css::frame::XLayoutManager > getLayoutManager() const;

        void updateStatus( const OUString& aCommandURL );
        void updateStatus();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;
        virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
            throw (css::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XUpdatable
        virtual void SAL_CALL update() throw (css::uno::RuntimeException, std::exception) override;

        // XComponent
        virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        using cppu::OPropertySetHelper::disposing;
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception ) override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override = 0;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL click() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL doubleClick() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) throw (css::uno::RuntimeException, std::exception) override;
        // OPropertySetHelper
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) throw(css::uno::Exception, std::exception) override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& rConvertedValue, css::uno::Any& rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue) throw(css::lang::IllegalArgumentException) override;
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;


        const OUString& getCommandURL() const { return  m_aCommandURL; }
        const OUString& getModuleName() const { return m_sModuleName; }

        void dispatchCommand( const OUString& sCommandURL, const css::uno::Sequence< css::beans::PropertyValue >& rArgs, const OUString &rTarget = OUString() );

        void enable( bool bEnable );

    protected:
        bool getToolboxId( sal_uInt16& rItemId, ToolBox** ppToolBox );
        void setSupportVisibleProperty(bool bValue);
        struct Listener
        {
            Listener( const css::util::URL& rURL, const css::uno::Reference< css::frame::XDispatch >& rDispatch ) :
                aURL( rURL ), xDispatch( rDispatch ) {}

            css::util::URL aURL;
            css::uno::Reference< css::frame::XDispatch > xDispatch;
        };

        struct DispatchInfo
        {
            css::uno::Reference< css::frame::XDispatch > mxDispatch;
            const css::util::URL maURL;
            const css::uno::Sequence< css::beans::PropertyValue > maArgs;

            DispatchInfo( const css::uno::Reference< css::frame::XDispatch >& xDispatch,
                          const css::util::URL& rURL,
                          const css::uno::Sequence< css::beans::PropertyValue >& rArgs )
                : mxDispatch( xDispatch )
                , maURL( rURL )
                , maArgs( rArgs )
                {}
        };

        DECL_STATIC_LINK_TYPED( ToolboxController, ExecuteHdl_Impl, void*, void );

        typedef std::unordered_map< OUString,
                                    css::uno::Reference< css::frame::XDispatch >,
                                    OUStringHash,
                                    std::equal_to< OUString > > URLToDispatchMap;

        // methods to support status forwarder, known by the old sfx2 toolbox controller implementation
        void addStatusListener( const OUString& aCommandURL );
        void removeStatusListener( const OUString& aCommandURL );
        void bindListener();
        void unbindListener();
        bool isBound() const;
        // TODO remove
        css::uno::Reference< css::util::XURLTransformer > getURLTransformer() const { return m_xUrlTransformer;}
        // TODO remove
        css::uno::Reference< css::awt::XWindow > getParent() const { return m_xParentWindow;}

        bool                                                      m_bInitialized : 1,
                                                                  m_bDisposed : 1;
        sal_uInt16                                                m_nToolBoxId;
        css::uno::Reference< css::frame::XFrame >                 m_xFrame;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        OUString                                                  m_aCommandURL;
        URLToDispatchMap                                          m_aListenerMap;
        ::cppu::OMultiTypeInterfaceContainerHelper                m_aListenerContainer;   /// container for ALL Listener

        css::uno::Reference< css::awt::XWindow >                  m_xParentWindow;
        css::uno::Reference< css::util::XURLTransformer >         m_xUrlTransformer;
        OUString                                                  m_sModuleName;
};

}

#endif // INCLUDED_SVTOOLS_TOOLBOXCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
