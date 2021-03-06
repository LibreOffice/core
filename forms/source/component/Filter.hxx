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

#pragma once

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <toolkit/controls/unocontrol.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase5.hxx>
#include <comphelper/uno3.hxx>
#include <svx/ParseContext.hxx>

#include <unordered_map>


namespace frm
{


    // OFilterControl

    typedef ::cppu::ImplHelper5 <   css::awt::XTextComponent
                                ,   css::awt::XFocusListener
                                ,   css::awt::XItemListener
                                ,   css::form::XBoundComponent
                                ,   css::lang::XInitialization
                                >   OFilterControl_BASE;

    class OFilterControl final :public UnoControl
                            ,public OFilterControl_BASE
                            ,public ::svxform::OParseContextClient
    {
        TextListenerMultiplexer     m_aTextListeners;

        css::uno::Reference< css::uno::XComponentContext >            m_xContext;
        css::uno::Reference< css::beans::XPropertySet >               m_xField;
        css::uno::Reference< css::util::XNumberFormatter >            m_xFormatter;
        css::uno::Reference< css::sdbc::XConnection >                 m_xConnection;
        css::uno::Reference< css::awt::XWindow >                      m_xMessageParent;

        typedef std::unordered_map< OUString, OUString > MapString2String;
        MapString2String                m_aDisplayItemToValueItem;

        OUString                        m_aText;
        sal_Int16                       m_nControlClass;        // which kind of control do we use?
        bool                            m_bFilterList : 1;
        bool                            m_bMultiLine : 1;
        bool                            m_bFilterListFilled : 1;

        void implInitFilterList();
        void initControlModel(css::uno::Reference< css::beans::XPropertySet > const & xControlModel);

    public:
        explicit OFilterControl( const css::uno::Reference< css::uno::XComponentContext >& _rxORB );

        DECLARE_UNO3_AGG_DEFAULTS(OFilterControl,OWeakAggObject)
        css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;

        virtual OUString GetComponentServiceName() override;
        virtual void SAL_CALL   createPeer( const css::uno::Reference< css::awt::XToolkit > & rxToolkit, const css::uno::Reference< css::awt::XWindowPeer >  & rParentPeer ) override;

    // css::lang::XComponent
        virtual void SAL_CALL   dispose() override;

    // css::awt::XTextComponent
        virtual void            SAL_CALL addTextListener( const css::uno::Reference< css::awt::XTextListener > & l ) override;
        virtual void            SAL_CALL removeTextListener( const css::uno::Reference< css::awt::XTextListener > & l ) override;
        virtual void            SAL_CALL setText( const OUString& aText ) override;
        virtual void            SAL_CALL insertText( const css::awt::Selection& rSel, const OUString& aText ) override;
        virtual OUString SAL_CALL getText() override;
        virtual OUString SAL_CALL getSelectedText() override;
        virtual void            SAL_CALL setSelection( const css::awt::Selection& aSelection ) override;
        virtual css::awt::Selection SAL_CALL getSelection() override;
        virtual sal_Bool        SAL_CALL isEditable() override;
        virtual void            SAL_CALL setEditable( sal_Bool bEditable ) override;
        virtual void            SAL_CALL setMaxTextLen( sal_Int16 nLength ) override;
        virtual sal_Int16       SAL_CALL getMaxTextLen() override;

    // css::form::XBoundComponent
        virtual void            SAL_CALL addUpdateListener(const css::uno::Reference< css::form::XUpdateListener > & /*l*/) override {}
        virtual void            SAL_CALL removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener > & /*l*/) override {}
        virtual sal_Bool        SAL_CALL commit() override;

    // css::lang::XEventListener
        virtual void            SAL_CALL disposing(const css::lang::EventObject& Source) override;

    // css::awt::XFocusListener
        virtual void            SAL_CALL focusGained(const css::awt::FocusEvent& e) override;
        virtual void            SAL_CALL focusLost(const css::awt::FocusEvent& e) override;

    // css::awt::XItemListener
        virtual void            SAL_CALL itemStateChanged(const css::awt::ItemEvent& rEvent) override;

    // css::util::XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    private:
        virtual void PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc ) override;
        virtual void ImplSetPeerProperty( const OUString& rPropName, const css::uno::Any& rVal ) override;

        bool ensureInitialized( );

        void displayException( const css::sdb::SQLContext& _rExcept );
    };

}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
