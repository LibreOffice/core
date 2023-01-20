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

#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

namespace com::sun::star::table { class XCell; }
namespace com::sun::star::sheet { class XSpreadsheetDocument; }
namespace com::sun::star::text { class XTextRange; }

namespace calc
{

    //= OCellValueBinding

    class OCellValueBinding;
    // the base for our interfaces
    typedef ::cppu::WeakComponentImplHelper <   css::form::binding::XValueBinding
                                            ,   css::lang::XServiceInfo
                                            ,   css::util::XModifyBroadcaster
                                            ,   css::util::XModifyListener
                                            ,   css::lang::XInitialization
                                            >   OCellValueBinding_Base;
    // the base for the property handling
    typedef ::comphelper::OPropertyContainer        OCellValueBinding_PBase;
    // the second base for property handling
    typedef ::comphelper::OPropertyArrayUsageHelper< OCellValueBinding >
                                                    OCellValueBinding_PABase;

    class OCellValueBinding :public ::cppu::BaseMutex
                            ,public OCellValueBinding_Base      // order matters! before OCellValueBinding_PBase, so rBHelper gets initialized
                            ,public OCellValueBinding_PBase
                            ,public OCellValueBinding_PABase
    {
    private:
        css::uno::Reference< css::sheet::XSpreadsheetDocument >
                    m_xDocument;            /// the document where our cell lives
        css::uno::Reference< css::table::XCell >
                    m_xCell;                /// the cell we're bound to, for double value access
        css::uno::Reference< css::text::XTextRange >
                    m_xCellText;            /// the cell we're bound to, for text access
        ::comphelper::OInterfaceContainerHelper3<css::util::XModifyListener>
                    m_aModifyListeners;     /// our modify listeners
        bool        m_bInitialized;         /// has XInitialization::initialize been called?
        bool        m_bListPos;             /// constructed as ListPositionCellBinding?

    public:
        OCellValueBinding(
            const css::uno::Reference< css::sheet::XSpreadsheetDocument >& _rxDocument,
            bool _bListPos
        );

        using OCellValueBinding_PBase::getFastPropertyValue;

    protected:
        virtual ~OCellValueBinding( ) override;

    protected:
        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XValueBinding
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getSupportedValueTypes(  ) override;
        virtual sal_Bool SAL_CALL supportsType( const css::uno::Type& aType ) override;
        virtual css::uno::Any SAL_CALL getValue( const css::uno::Type& aType ) override;
        virtual void SAL_CALL setValue( const css::uno::Any& aValue ) override;

        // OComponentHelper/XComponent
        virtual void SAL_CALL disposing() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        virtual void SAL_CALL getFastPropertyValue( css::uno::Any& _rValue, sal_Int32 _nHandle ) const override;

        // ::comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
        virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

        // XModifyListener
        virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    private:
        void    checkDisposed( ) const;
        void    checkValueType( const css::uno::Type& _rType ) const;
        void    checkInitialized();

        /** notifies our modify listeners
            @precond
                our mutex is <em>not</em> locked
        */
        void    notifyModified();

        void    setBooleanFormat();

    private:
        OCellValueBinding( const OCellValueBinding& ) = delete;
        OCellValueBinding& operator=( const OCellValueBinding& ) = delete;
    };

}   // namespace calc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
