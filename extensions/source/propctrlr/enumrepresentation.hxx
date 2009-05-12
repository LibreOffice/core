/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: enumrepresentation.hxx,v $
 * $Revision: 1.4 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Any.hxx>
/** === end UNO includes === **/
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <vector>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= IPropertyEnumRepresentation
    //====================================================================
    class SAL_NO_VTABLE IPropertyEnumRepresentation : public ::rtl::IReference
    {
    public:
        /** retrieves all descriptions of all possible values of the enumeration property
        */
        virtual ::std::vector< ::rtl::OUString > SAL_CALL getDescriptions(
            ) const = 0;

        /** converts a given description into a property value
        */
        virtual void SAL_CALL getValueFromDescription(
                const ::rtl::OUString& _rDescription,
                ::com::sun::star::uno::Any& _out_rValue
            ) const = 0;

        /** converts a given property value into a description
        */
        virtual ::rtl::OUString SAL_CALL getDescriptionForValue(
                const ::com::sun::star::uno::Any& _rEnumValue
            ) const = 0;

        virtual ~IPropertyEnumRepresentation() { };
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX

