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

#ifndef INCLUDED_SC_SOURCE_UI_UNOOBJ_CELLVALUEBINDING_HXX
#define INCLUDED_SC_SOURCE_UI_UNOOBJ_CELLVALUEBINDING_HXX

#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

namespace calc
{

    //= OCellValueBinding

    class OCellValueBinding;
    // the base for our interfaces
    typedef ::cppu::WeakAggComponentImplHelper5 <   ::com::sun::star::form::binding::XValueBinding
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::util::XModifyBroadcaster
                                                ,   ::com::sun::star::util::XModifyListener
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   OCellValueBinding_Base;
    // the base for the property handling
    typedef ::comphelper::OPropertyContainer        OCellValueBinding_PBase;
    // the second base for property handling
    typedef ::comphelper::OPropertyArrayUsageHelper< OCellValueBinding >
                                                    OCellValueBinding_PABase;

    class OCellValueBinding :public ::comphelper::OBaseMutex
                            ,public OCellValueBinding_Base      // order matters! before OCellValueBinding_PBase, so rBHelper gets initialized
                            ,public OCellValueBinding_PBase
                            ,public OCellValueBinding_PABase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                    m_xDocument;            /// the document where our cell lives
        ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                    m_xCell;                /// the cell we're bound to, for double value access
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >
                    m_xCellText;            /// the cell we're bound to, for text access
        ::cppu::OInterfaceContainerHelper
                    m_aModifyListeners;     /// our modify listeners
        bool        m_bInitialized;         /// has XInitialization::initialize been called?
        bool        m_bListPos;             /// constructed as ListPositionCellBinding?

    public:
        OCellValueBinding(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& _rxDocument,
            bool _bListPos
        );

        using OCellValueBinding_PBase::getFastPropertyValue;

    protected:
        virtual ~OCellValueBinding( );

    protected:
        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XValueBinding
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getSupportedValueTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsType( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL getValue( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::form::binding::IncompatibleTypesException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::form::binding::IncompatibleTypesException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) override;

        // OComponentHelper/XComponent
        virtual void SAL_CALL disposing() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const override;

        // ::comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        void    checkDisposed( ) const;
        void    checkValueType( const ::com::sun::star::uno::Type& _rType ) const;
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

#endif // INCLUDED_SC_SOURCE_UI_UNOOBJ_CELLVALUEBINDING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
