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

#ifndef FORMS_COMPONENT_FILTER_HXX
#define FORMS_COMPONENT_FILTER_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <toolkit/controls/unocontrol.hxx>

#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase5.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/componentcontext.hxx>
#include <cppuhelper/implbase4.hxx>
#include <connectivity/sqlparse.hxx>
#include <svx/ParseContext.hxx>

#include <boost/unordered_map.hpp>

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    // OFilterControl
    //=====================================================================
    typedef ::cppu::ImplHelper5 <   ::com::sun::star::awt::XTextComponent
                                ,   ::com::sun::star::awt::XFocusListener
                                ,   ::com::sun::star::awt::XItemListener
                                ,   ::com::sun::star::form::XBoundComponent
                                ,   ::com::sun::star::lang::XInitialization
                                >   OFilterControl_BASE;

    class OFilterControl    :public UnoControl
                            ,public OFilterControl_BASE
                            ,public ::svxform::OParseContextClient
    {
        TextListenerMultiplexer     m_aTextListeners;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >            m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >               m_xField;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >            m_xFormatter;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >                 m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >           m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >                      m_xMessageParent;

        typedef ::boost::unordered_map< OUString, OUString, OUStringHash > MapString2String;
        MapString2String                m_aDisplayItemToValueItem;

        OUString                 m_aText;
        ::connectivity::OSQLParser      m_aParser;
        sal_Int16                       m_nControlClass;        // which kind of control do we use?
        sal_Bool                        m_bFilterList : 1;
        sal_Bool                        m_bMultiLine : 1;
        sal_Bool                        m_bFilterListFilled : 1;

    private:
        void implInitFilterList();
        void initControlModel(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xControlModel);

    public:
        OFilterControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB );

        DECLARE_UNO3_AGG_DEFAULTS(OFilterControl,OWeakAggObject);
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

        virtual OUString GetComponentServiceName();
        virtual void SAL_CALL   createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XComponent
        virtual void SAL_CALL   dispose(void) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XTextComponent
        virtual void            SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setText( const OUString& aText ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL insertText( const ::com::sun::star::awt::Selection& rSel, const OUString& aText ) throw( ::com::sun::star::uno::RuntimeException );
        virtual OUString SAL_CALL getText() throw( ::com::sun::star::uno::RuntimeException );
        virtual OUString SAL_CALL getSelectedText() throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::awt::Selection SAL_CALL getSelection() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool        SAL_CALL isEditable() throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setEditable( sal_Bool bEditable ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL setMaxTextLen( sal_Int16 nLength ) throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Int16       SAL_CALL getMaxTextLen() throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::form::XBoundComponent
        virtual void            SAL_CALL addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener > & /*l*/) throw( ::com::sun::star::uno::RuntimeException ) {}
        virtual void            SAL_CALL removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener > & /*l*/) throw( ::com::sun::star::uno::RuntimeException ) {}
        virtual sal_Bool        SAL_CALL commit() throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::lang::XEventListener
        virtual void            SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XFocusListener
        virtual void            SAL_CALL focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );
        virtual void            SAL_CALL focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::awt::XItemListener
        virtual void            SAL_CALL itemStateChanged(const ::com::sun::star::awt::ItemEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::util::XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XServiceInfo - static version
        static  OUString SAL_CALL getImplementationName_Static();
        static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static();
        static  ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory );

    protected:
        virtual void PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );
        virtual void ImplSetPeerProperty( const OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

        sal_Bool ensureInitialized( );

        void displayException( const ::com::sun::star::sdb::SQLContext& _rExcept );
    };
//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_COMPONENT_FILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
