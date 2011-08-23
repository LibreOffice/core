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

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#define _XMLOFF_PROPERTYHANDLERBASE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace com{ namespace sun{ namespace star{ namespace uno{ class Any; } } } }
namespace rtl{ class OUString; }
namespace binfilter {

class SvXMLUnitConverter;
struct XMLPropertyState;

///////////////////////////////////////////////////////////////////////////
//
/** Abstract base-class for different XML-types. Derivations of this class
    knows how to compare, im/export a special XML-type
*/
class XMLPropertyHandler
{
    // ??? const SvXMLUnitConverter& rUnitConverter			Do we need this ?

public:
    // Just needed for virtual destruction
    virtual ~XMLPropertyHandler();

    /** Compares two Any's in case of the given XML-data-type.
        By default the Any-equal-operator is called. */
    virtual sal_Bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;

    /// Imports the given value in cas of the given XML-data-type
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const =0;
    /// Exports the given value in cas of the given XML-data-type
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const =0;
};

}//end of namespace binfilter
#endif		// _XMLOFF_PROPERTYHANDLERBASE_HXX
