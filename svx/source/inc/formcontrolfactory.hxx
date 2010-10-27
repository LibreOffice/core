/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*************************************************************************
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

#ifndef SVX_FORMCONTROLMANAGER_HXX
#define SVX_FORMCONTROLMANAGER_HXX

#include "fmdocumentclassification.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
/** === end UNO includes === **/

#include <memory>

class SdrUnoObj;
class Rectangle;

namespace comphelper {
    class ComponentContext;
}

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= FormControlFactory
    //====================================================================
    struct FormControlFactory_Data;

    class FormControlFactory
    {
    public:
        FormControlFactory( const ::comphelper::ComponentContext& _rContext );
        ~FormControlFactory();

        /** initializes the given control model which is to be newly inserted into a document

            @param _eDocType
                the type of the document which the control is to be inserted into
            @param _rxControlModel
                the control model to be inserted
            @param _rControlBoundRect
                the bound rect of the control, if applicable
            @return
                the class ID of the control
        */
        sal_Int16 initializeControlModel(
                    const DocumentType _eDocType,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                    const Rectangle& _rControlBoundRect
                );

        sal_Int16 initializeControlModel( const DocumentType _eDocType, const SdrUnoObj& _rObject );
        sal_Int16 initializeControlModel( const DocumentType _eDocType, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel );

        void initializeTextFieldLineEnds(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel
                );

        void    initializeFieldDependentProperties(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxDatabaseField,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >& _rxNumberFormats
                );

        static ::rtl::OUString getDefaultName(
                    const sal_Int16 nClassId,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >& _rxObject
                );

        static ::rtl::OUString getDefaultUniqueName_ByComponentType(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject
                );

        static ::rtl::OUString getUniqueName(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxContainer,
                    const ::rtl::OUString& _rBaseName
                );

    private:
        ::std::auto_ptr< FormControlFactory_Data >  m_pData;
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_FORMCONTROLMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
