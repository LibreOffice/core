/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX

#include "eformshelper.hxx"
#include "xsddatatypes.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/xsd/XDataType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/
#include <rtl/ref.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    class XSDDataType;
    //====================================================================
    //= XSDValidationHelper
    //====================================================================
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
        void    getAvailableDataTypeNames( ::std::vector< ::rtl::OUString >& /* [out] */ _rNames ) const SAL_THROW(());

        /** retrieves a particular data type given by name
        */
        ::rtl::Reference< XSDDataType >
                getDataTypeByName( const ::rtl::OUString& _rName ) const SAL_THROW(());

        /** retrieves the DataType instance which the control model is currently validated against

            If there is a binding set at our control model, which at the same time acts as validator,
            and if this validator is bound to an XDataType, then this data type is retrieved here.
        */
        ::rtl::Reference< XSDDataType >
                getValidatingDataType( ) const SAL_THROW(());

        /** retrieves the name of the data type which the control model is currently validated against

            @seealso getValidatingDataType
        */
        ::rtl::OUString
                getValidatingDataTypeName( ) const SAL_THROW(());

        /** binds the validator to a new data type

            To be called with an active binding only.
        */
        void    setValidatingDataTypeByName( const ::rtl::OUString& _rName ) const SAL_THROW(());

        /** removes the data type given by name from the data type repository
        */
        bool    removeDataTypeFromRepository( const ::rtl::OUString& _rName ) const SAL_THROW(());

        /** creates a new data type, which is a clone of an existing data type
        */
        bool    cloneDataType( const ::rtl::Reference< XSDDataType >& _pDataType, const ::rtl::OUString& _rNewName ) const SAL_THROW(());

        /** retrieves the name of the basic data type which has the given class
        */
        ::rtl::OUString
                getBasicTypeNameForClass( sal_Int16 _eClass ) const SAL_THROW(());

        /** copy a data type from one model to another

            If a data type with the given name already exists in the target model, then nothing
            happens. In particular, the facets of the data type are not copied.
        */
        void    copyDataType( const ::rtl::OUString& _rFromModel, const ::rtl::OUString& _rToModel,
                    const ::rtl::OUString& _rDataTypeName ) const SAL_THROW(());

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
                getDataTypeRepository( const ::rtl::OUString& _rModelName ) const SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the data type object for the given name
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::xsd::XDataType >
            getDataType( const ::rtl::OUString& _rName ) const
                SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the name of the basic data type which has the given class, in the given repository
        */
        ::rtl::OUString
                getBasicTypeNameForClass(
                    sal_Int16 _nClass,
                    ::com::sun::star::uno::Reference< ::com::sun::star::xforms::XDataTypeRepository > _rxRepository
                ) const SAL_THROW(());
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_XSDVALIDATIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
