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

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>


namespace pcr
{

    /** encapsulates functionality related to binding a form control to a spreadsheet cell
    */
    class CellBindingHelper final
    {
        css::uno::Reference< css::beans::XPropertySet >
                    m_xControlModel;    // the model we work for
        css::uno::Reference< css::sheet::XSpreadsheetDocument >
                    m_xDocument;        // the document where the model lives

    public:
        /** ctor
            @param _rxControlModel
                the control model which is or will be bound
        */
        CellBindingHelper(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const css::uno::Reference< css::frame::XModel >& _rxContextDocument
        );

        /** determines whether the given model is a spreadsheet document model

            <p>If this method returns <FALSE/>, you cannot instantiate a CellBindingHelper with
            the document, since then no of its functionality will be available.</p>
        */
        static  bool    isSpreadsheetDocument(
                                const css::uno::Reference< css::frame::XModel >& _rxContextDocument
                            );

        /** gets a cell binding for the given address
            @precond
                isCellBindingAllowed returns <TRUE/>
        */
        css::uno::Reference< css::form::binding::XValueBinding >
                        createCellBindingFromStringAddress(
                            const OUString& _rAddress,
                            bool _bSupportIntegerExchange
                        ) const;

        /** creates a cell binding (supporting integer exchange, if requested) for
            the given address object
        */
        css::uno::Reference< css::form::binding::XValueBinding >
                        createCellBindingFromAddress(
                            const css::table::CellAddress& _rAddress,
                            bool _bSupportIntegerExchange
                        ) const;

        /** gets a cell range list source binding for the given address
        */
        css::uno::Reference< css::form::binding::XListEntrySource >
                        createCellListSourceFromStringAddress( const OUString& _rAddress ) const;

        /** creates a string representation for the given value binding's address

            <p>If the sheet of the bound cell is the same as the sheet which our control belongs
            to, then the sheet name is omitted in the resulting string representation.</p>

            @precond
                The binding is a valid cell binding, or <NULL/>
            @see isCellBinding
        */
        OUString getStringAddressFromCellBinding(
                            const css::uno::Reference< css::form::binding::XValueBinding >& _rxBinding
                        ) const;

        /** creates an address object for the given value binding's address

            @precond
                The binding is a valid cell binding, or <NULL/>
            @return
                <FALSE/> if and only if an error occurred and no valid address could be obtained
            @see isCellBinding
        */
        bool            getAddressFromCellBinding(
                            const css::uno::Reference< css::form::binding::XValueBinding >& _rxBinding,
                            css::table::CellAddress& _rAddress
                        ) const;

        /** creates a string representation for the given list source's range address

            <p>If the sheet of the cell range which acts as list source is the same as the
            sheet which our control belongs to, then the sheet name is omitted in the
            resulting string representation.</p>

            @precond
                The object is a valid cell range list source, or <NULL/>
            @see isCellRangeListSource
        */
        OUString getStringAddressFromCellListSource(
                            const css::uno::Reference< css::form::binding::XListEntrySource >& _rxSource
                        ) const;

        /** returns the current binding of our control model, if any.
        */
        css::uno::Reference< css::form::binding::XValueBinding >
                        getCurrentBinding( ) const;

        /** returns the current external list source of the control model, if any
        */
        css::uno::Reference< css::form::binding::XListEntrySource >
                        getCurrentListSource( ) const;

        /** sets a new binding for our control model
            @precond
                the control model is bindable (which is implied by <member>isCellBindingAllowed</member>
                returning <TRUE/>)
        */
        void            setBinding(
                            const css::uno::Reference< css::form::binding::XValueBinding >& _rxBinding
                        );

        /** sets a list source for our control model
            @precond
                the control model is a list sink (which is implied by <member>isListCellRangeAllowed</member>
                returning <TRUE/>)
        */
        void            setListSource(
                            const css::uno::Reference< css::form::binding::XListEntrySource >& _rxSource
                        );

        /** checks whether it's possible to bind the control model to a spreadsheet cell
        */
        bool            isCellBindingAllowed( ) const;

        /** checks whether it's possible to bind the control model to a spreadsheet cell,
            with exchanging integer values
        */
        bool            isCellIntegerBindingAllowed( ) const;

        /** checks whether it's possible to bind the control model to range of spreadsheet cells
            supplying the list entries
        */
        bool            isListCellRangeAllowed( ) const;

        /** checks whether a given binding is a spreadsheet cell binding
        */
        static bool     isCellBinding(
                            const css::uno::Reference< css::form::binding::XValueBinding >& _rxBinding
                        );

        /** checks whether a given binding is a spreadsheet cell binding, exchanging
            integer values
        */
        static bool    isCellIntegerBinding(
                            const css::uno::Reference< css::form::binding::XValueBinding >& _rxBinding
                        );

        /** checks whether a given list source is a spreadsheet cell list source
        */
        static bool     isCellRangeListSource(
                            const css::uno::Reference< css::form::binding::XListEntrySource >& _rxSource
                        );

        /** retrieves the index of the sheet which our control belongs to
            @return the index of the sheet which our control belongs to or -1, if an error occurred
        */
        sal_Int16       getControlSheetIndex(
                            css::uno::Reference< css::sheet::XSpreadsheet >& _out_rxSheet
                        ) const;

    private:
        /** creates an address object from a string representation of a cell address
        */
        bool            convertStringAddress(
                            const OUString& _rAddressDescription,
                            css::table::CellAddress& /* [out] */ _rAddress
                        ) const;

        /** creates an address range object from a string representation of a cell range address
        */
        bool            convertStringAddress(
                            const OUString& _rAddressDescription,
                            css::table::CellRangeAddress& /* [out] */ _rAddress
                        ) const;

        /** determines if our document is a spreadsheet document, *and* can supply
            the given service
        */
        bool            isSpreadsheetDocumentWhichSupplies( const OUString& _rService ) const;

        /** checks whether a given component supports a given service
        */
        static bool     doesComponentSupport(
                            const css::uno::Reference< css::uno::XInterface >& _rxComponent,
                            const OUString& _rService
                        );

        /** uses the document (it's factory interface, respectively) to create a component instance
            @param _rService
                the service name
            @param _rArgumentName
                the name of the single argument to pass during creation. May be empty, in this case
                no arguments are passed
            @param _rArgumentValue
                the value of the instantiation argument. Not evaluated if <arg>_rArgumentName</arg>
                is empty.
        */
        css::uno::Reference< css::uno::XInterface >
                        createDocumentDependentInstance(
                            const OUString& _rService,
                            const OUString& _rArgumentName,
                            const css::uno::Any& _rArgumentValue
                        ) const;

        /** converts an address representation into another one

            @param _rInputProperty
                the input property name for the conversion service
            @param _rInputValue
                the input property value for the conversion service
            @param _rOutputProperty
                the output property name for the conversion service
            @param _rOutputValue
                the output property value for the conversion service
            @param _bIsRange
                if <TRUE/>, the RangeAddressConversion service will be used, else
                the AddressConversion service

            @return
                <TRUE/> if any only if the conversion was successful

            @see css::table::CellAddressConversion
            @see css::table::CellRangeAddressConversion
        */
        bool            doConvertAddressRepresentations(
                            const OUString& _rInputProperty,
                            const css::uno::Any& _rInputValue,
                            const OUString& _rOutputProperty,
                                  css::uno::Any& _rOutputValue,
                            bool _bIsRange
                        ) const;
    };


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
