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

#ifndef _XMLOFF_PROPERTYHANDLER_OPAQUETYPES_HXX
#include "opaquhdl.hxx"
#endif



// --


#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::binfilter::xmloff::token;


///////////////////////////////////////////////////////////////////////////////
//
// class XMLOpaquePropHdl
//

XMLOpaquePropHdl::~XMLOpaquePropHdl()
{
    // nothing to do
}

sal_Bool XMLOpaquePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_True;

    sal_Bool bValue = IsXMLToken( rStrImpValue, XML_OPAQUE_FOREGROUND );
    rValue <<= sal_Bool(bValue);
    bRet = sal_True;

    return bRet; 
}

sal_Bool XMLOpaquePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_Bool bValue;

    if (rValue >>= bValue)
    {
        if( bValue )
            rStrExpValue = GetXMLToken( XML_OPAQUE_FOREGROUND );
        else
            rStrExpValue = GetXMLToken( XML_OPAQUE_BACKGROUND );

        bRet = sal_True;
    }
    
    return bRet;
}
}//end of namespace binfilter
