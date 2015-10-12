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

#ifndef INCLUDED_SC_SOURCE_UI_UNOOBJ_CELLLISTSOURCE_HXX
#define INCLUDED_SC_SOURCE_UI_UNOOBJ_CELLLISTSOURCE_HXX

#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <cppuhelper/compbase4.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

namespace calc
{

    //= OCellListSource

    class OCellListSource;
    // the base for our interfaces
    typedef ::cppu::WeakAggComponentImplHelper4 <   ::com::sun::star::form::binding::XListEntrySource
                                                ,   ::com::sun::star::util::XModifyListener
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::lang::XInitialization
                                                >   OCellListSource_Base;
    // the base for the property handling
    typedef ::comphelper::OPropertyContainer        OCellListSource_PBase;
    // the second base for property handling
    typedef ::comphelper::OPropertyArrayUsageHelper< OCellListSource >
                                                    OCellListSource_PABase;

    class OCellListSource :public ::comphelper::OBaseMutex
                            ,public OCellListSource_Base      // order matters! before OCellListSource_PBase, so rBHelper gets initialized
                            ,public OCellListSource_PBase
                            ,public OCellListSource_PABase
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >
                    m_xDocument;            /// the document where our cell lives
        ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                    m_xRange;               /// the range of cells we're bound to
        ::cppu::OInterfaceContainerHelper
                    m_aListEntryListeners;  /// our listeners
        bool        m_bInitialized;         /// has XInitialization::initialize been called?

    public:
        OCellListSource(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& _rxDocument
        );

        using OCellListSource_PBase::getFastPropertyValue;

    protected:
        virtual ~OCellListSource( );

    protected:
        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XListEntrySource
        virtual sal_Int32 SAL_CALL getListEntryCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getListEntry( sal_Int32 Position ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAllListEntries(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL addListEntryListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntryListener >& Listener ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeListEntryListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XListEntryListener >& Listener ) throw (::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException, std::exception) override;

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

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        void    checkDisposed( ) const;
        void    checkInitialized();

        /** retrieves the actual address of our cell range
            @precond
                our m_xRange is not <NULL/>
        */
        ::com::sun::star::table::CellRangeAddress
                getRangeAddress( ) const;

        /** retrievs the text of a cell within our range
            @param _nRangeRelativeColumn
                the relative column index of the cell within our range
            @param _nRangeRelativeRow
                the relative row index of the cell within our range
            @precond
                our m_xRange is not <NULL/>
        */
        OUString
                getCellTextContent_noCheck(
                    sal_Int32 _nRangeRelativeColumn,
                    sal_Int32 _nRangeRelativeRow
                );

        void    notifyModified();

    private:
        OCellListSource( const OCellListSource& ) = delete;
        OCellListSource& operator=( const OCellListSource& ) = delete;
    };

}   // namespace calc

#endif // INCLUDED_SC_SOURCE_UI_UNOOBJ_CELLLISTSOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
