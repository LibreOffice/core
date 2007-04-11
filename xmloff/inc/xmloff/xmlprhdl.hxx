/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlprhdl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:35:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#define _XMLOFF_PROPERTYHANDLERBASE_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace com{ namespace sun{ namespace star{ namespace uno{ class Any; } } } }
namespace rtl{ class OUString; }

class SvXMLUnitConverter;
struct XMLPropertyState;

///////////////////////////////////////////////////////////////////////////
//
/** Abstract base-class for different XML-types. Derivations of this class
    knows how to compare, im/export a special XML-type
*/
class XMLOFF_DLLPUBLIC XMLPropertyHandler
{
    // ??? const SvXMLUnitConverter& rUnitConverter         Do we need this ?

public:
    // Just needed for virtual destruction
    virtual ~XMLPropertyHandler();

    /** Compares two Any's in case of the given XML-data-type.
        By default the Any-equal-operator is called. */
    virtual bool equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const;

    /// Imports the given value in cas of the given XML-data-type
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const =0;
    /// Exports the given value in cas of the given XML-data-type
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const =0;
};

#endif      // _XMLOFF_PROPERTYHANDLERBASE_HXX
