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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FILTER_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FILTER_HXX

#include <config_features.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <toolkit/controls/unocontrol.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/sqlparse.hxx>
#include <svx/ParseContext.hxx>

#include <unordered_map>


namespace frm
{



    // OFilterControl

    typedef ::cppu::ImplHelper  <   css::awt::XTextComponent
                                ,   css::awt::XFocusListener
                                ,   css::awt::XItemListener
                                ,   css::form::XBoundComponent
                                ,   css::lang::XInitialization
                                >   OFilterControl_BASE;

    class OFilterControl    :public UnoControl
                            ,public OFilterControl_BASE
                            ,public ::svxform::OParseContextClient
    {
        TextListenerMultiplexer     m_aTextListeners;

        css::uno::Reference< css::uno::XComponentContext >            m_xContext;
        css::uno::Reference< css::beans::XPropertySet >               m_xField;
        css::uno::Reference< css::util::XNumberFormatter >            m_xFormatter;
        css::uno::Reference< css::sdbc::XConnection >                 m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData >           m_xMetaData;
        css::uno::Reference< css::awt::XWindow >                      m_xMessageParent;

        typedef std::unordered_map< OUString, OUString, OUStringHash > MapString2String;
        MapString2String                m_aDisplayItemToValueItem;

        OUString                        m_aText;
#if HAVE_FEATURE_DBCONNECTIVITY
        ::connectivity::OSQLParser      m_aParser;
#endif
        sal_Int16                       m_nControlClass;        // which kind of control do we use?
        bool                            m_bFilterList : 1;
        bool                            m_bMultiLine : 1;
        bool                            m_bFilterListFilled : 1;

    private:
        void implInitFilterList();
        void initControlModel(css::uno::Reference< css::beans::XPropertySet >& xControlModel);

    public:
        explicit OFilterControl( const css::uno::Reference< css::uno::XComponentContext >& _rxORB );

        DECLARE_UNO3_AGG_DEFAULTS(OFilterControl,OWeakAggObject)
        css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;

        virtual OUString GetComponentServiceName() override;
        virtual void SAL_CALL   createPeer( const css::uno::Reference< css::awt::XToolkit > & rxToolkit, const css::uno::Reference< css::awt::XWindowPeer >  & rParentPeer ) throw( css::uno::RuntimeException, std::exception ) override;

    // css::lang::XComponent
        virtual void SAL_CALL   dispose() throw( css::uno::RuntimeException, std::exception ) override;

    // css::awt::XTextComponent
        virtual void            SAL_CALL addTextListener( const css::uno::Reference< css::awt::XTextListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void            SAL_CALL removeTextListener( const css::uno::Reference< css::awt::XTextListener > & l ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void            SAL_CALL setText( const OUString& aText ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void            SAL_CALL insertText( const css::awt::Selection& rSel, const OUString& aText ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual OUString SAL_CALL getText() throw( css::uno::RuntimeException, std::exception ) override;
        virtual OUString SAL_CALL getSelectedText() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void            SAL_CALL setSelection( const css::awt::Selection& aSelection ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::awt::Selection SAL_CALL getSelection() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool        SAL_CALL isEditable() throw( css::uno::RuntimeException, std::exception ) override;
        virtual void            SAL_CALL setEditable( sal_Bool bEditable ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void            SAL_CALL setMaxTextLen( sal_Int16 nLength ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Int16       SAL_CALL getMaxTextLen() throw( css::uno::RuntimeException, std::exception ) override;

    // css::form::XBoundComponent
        virtual void            SAL_CALL addUpdateListener(const css::uno::Reference< css::form::XUpdateListener > & /*l*/) throw( css::uno::RuntimeException, std::exception ) override {}
        virtual void            SAL_CALL removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener > & /*l*/) throw( css::uno::RuntimeException, std::exception ) override {}
        virtual sal_Bool        SAL_CALL commit() throw( css::uno::RuntimeException, std::exception ) override;

    // css::lang::XEventListener
        virtual void            SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

    // css::awt::XFocusListener
        virtual void            SAL_CALL focusGained(const css::awt::FocusEvent& e) throw( css::uno::RuntimeException, std::exception ) override;
        virtual void            SAL_CALL focusLost(const css::awt::FocusEvent& e) throw( css::uno::RuntimeException, std::exception ) override;

    // css::awt::XItemListener
        virtual void            SAL_CALL itemStateChanged(const css::awt::ItemEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

    // css::util::XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

        // css::lang::XServiceInfo - static version
        static  OUString SAL_CALL getImplementationName_Static();
        static  css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();

    protected:
        virtual void PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc ) override;
        virtual void ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal ) override;

        bool ensureInitialized( );

        void displayException( const css::sdb::SQLContext& _rExcept );
    };

}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_FILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
