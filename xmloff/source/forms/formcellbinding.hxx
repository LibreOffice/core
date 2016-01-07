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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_FORMCELLBINDING_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_FORMCELLBINDING_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XModel.hpp>

namespace xmloff
{

    //= FormCellBindingHelper
    /** encapsulates functionality related to binding a form control to a spreadsheet cell
    */
    class FormCellBindingHelper
    {
    protected:
        css::uno::Reference< css::beans::XPropertySet >
                    m_xControlModel;    // the model we work for
        css::uno::Reference< css::sheet::XSpreadsheetDocument >
                    m_xDocument;        // the document where the model lives

    public:
        /** determines whether the given control model lives in a spreadsheet document
            <p>If this method returns <FALSE/>, you cannot instantiate a CellBindingHelper with
            this model, since then no of its functionality will be available.</p>
        */
        static  bool    livesInSpreadsheetDocument(
                                const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel
                            );

        /** ctor
            @param _rxControlModel
                the control model which is or will be bound
            @param _rxDocument
                the document. If this is <NULL/>, the document will be obtained from the model
                itself by walkon up the chain of its ancestors.<br/>
                This parameter can be used if the control model is not (yet) part of a document
                model.
        */
        FormCellBindingHelper(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel,
            const css::uno::Reference< css::frame::XModel >& _rxDocument
        );

    public:
        /** gets a cell binding for the given address
            @precond
                isCellBindingAllowed returns <TRUE/>
        */
        css::uno::Reference< css::form::binding::XValueBinding >
                        createCellBindingFromStringAddress(
                            const OUString& _rAddress,
                            bool _bUseIntegerBinding
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

        /** checks whether within the given document, it's possible to bind control models to spreadsheet cells
        */
        static bool     isCellBindingAllowed(
                            const css::uno::Reference< css::frame::XModel >& _rxDocument
                        );

        /** checks whether it's possible to bind the control model to a range of spreadsheet cells
            supplying the list entries
        */
        bool            isListCellRangeAllowed( ) const;

        /** checks whether within the given document, it's possible to bind the control model to a range of
            spreadsheet cells supplying the list entries
        */
        static bool     isListCellRangeAllowed(
                            const css::uno::Reference< css::frame::XModel >& _rxDocument
                        );

        /** checks whether a given binding is a spreadsheet cell binding
        */
        static bool     isCellBinding(
                            const css::uno::Reference< css::form::binding::XValueBinding >& _rxBinding
                        );

        /** checks whether a given binding is a spreadsheet cell binding, exchanging
            integer values
        */
        static bool     isCellIntegerBinding(
                            const css::uno::Reference< css::form::binding::XValueBinding >& _rxBinding
                        );

        /** checks whether a given list source is a spreadsheet cell list source
        */
        static bool    isCellRangeListSource(
                            const css::uno::Reference< css::form::binding::XListEntrySource >& _rxSource
                        );

    protected:
        /** creates an address object from a string representation of a cell address
        */
        bool            convertStringAddress(
                            const OUString& _rAddressDescription,
                            css::table::CellAddress& /* [out] */ _rAddress,
                            sal_Int16 _nAssumeSheet = -1
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

        /** determines if our document is a spreadsheet document, *and* can supply
            the given service
        */
        static bool     isSpreadsheetDocumentWhichSupplies(
                            const css::uno::Reference< css::sheet::XSpreadsheetDocument >& _rxDocument,
                            const OUString& _rService
                        );

        /** checkes whether a given component supports a given servive
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

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_FORMCELLBINDING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
