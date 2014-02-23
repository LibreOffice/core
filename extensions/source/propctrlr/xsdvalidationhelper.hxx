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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX

#include "eformshelper.hxx"
#include "xsddatatypes.hxx"

#include <com/sun/star/xsd/XDataType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ref.hxx>


namespace pcr
{


    class XSDDataType;

    //= XSDValidationHelper

    class XSDValidationHelper : public EFormsHelper
    {
    private:
        bool    m_bInspectingFormattedField;
    public:
        bool    isInspectingFormattedField() const { return m_bInspectingFormattedField; }

    public:
        XSDValidationHelper(
            ::osl::Mutex& _rMutex,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
        );

        /** retrieves the names of all XForms models in the document the control lives in
        */
        void    getAvailableDataTypeNames( ::std::vector< OUString >& /* [out] */ _rNames ) const SAL_THROW(());

        /** retrieves a particular data type given by name
        */
        ::rtl::Reference< XSDDataType >
                getDataTypeByName( const OUString& _rName ) const SAL_THROW(());

        /** retrieves the DataType instance which the control model is currently validated against

            If there is a binding set at our control model, which at the same time acts as validator,
            and if this validator is bound to an XDataType, then this data type is retrieved here.
        */
        ::rtl::Reference< XSDDataType >
                getValidatingDataType( ) const SAL_THROW(());

        /** retrieves the name of the data type which the control model is currently validated against

            @seealso getValidatingDataType
        */
        OUString
                getValidatingDataTypeName( ) const SAL_THROW(());

        /** binds the validator to a new data type

            To be called with an active binding only.
        */
        void    setValidatingDataTypeByName( const OUString& _rName ) const SAL_THROW(());

        /** removes the data type given by name from the data type repository
        */
        bool    removeDataTypeFromRepository( const OUString& _rName ) const SAL_THROW(());

        /** creates a new data type, which is a clone of an existing data type
        */
        bool    cloneDataType( const ::rtl::Reference< XSDDataType >& _pDataType, const OUString& _rNewName ) const SAL_THROW(());

        /** retrieves the name of the basic data type which has the given class
        */
        OUString
                getBasicTypeNameForClass( sal_Int16 _eClass ) const SAL_THROW(());

        /** copy a data type from one model to another

            If a data type with the given name already exists in the target model, then nothing
            happens. In particular, the facets of the data type are not copied.
        */
        void    copyDataType( const OUString& _rFromModel, const OUString& _rToModel,
                    const OUString& _rDataTypeName ) const SAL_THROW(());

        /** finds (and sets) a default format for the formatted field we're inspecting,
            according to the current data type the control value is evaluated against
        */
        void findDefaultFormatForIntrospectee() SAL_THROW(());

    private:
        /** retrieves the data type repository associated with the current model
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XDataTypeRepository >
                getDataTypeRepository() const SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the data type repository associated with any model
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XDataTypeRepository >
                getDataTypeRepository( const OUString& _rModelName ) const SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the data type object for the given name
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType >
            getDataType( const OUString& _rName ) const
                SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the name of the basic data type which has the given class, in the given repository
        */
        OUString
                getBasicTypeNameForClass(
                    sal_Int16 _nClass,
                    ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XDataTypeRepository > _rxRepository
                ) const SAL_THROW(());
    };


} // namespace pcr


#endif // EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
