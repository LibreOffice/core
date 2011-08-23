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

#ifndef _XMLOFF_PROPERTYHANDLER_CROSSEDOUTTYPES_HXX
#include <cdouthdl.hxx>
#endif


#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_ 
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _VCL_VCLENUM_HXX 
#include <vcl/vclenum.hxx>
#endif


namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

SvXMLEnumMapEntry pXML_Crossedout_Enum[] =
{
    { XML_CROSSEDOUT_NONE,		STRIKEOUT_NONE },
    { XML_CROSSEDOUT_SINGLE,	STRIKEOUT_SINGLE },
    { XML_CROSSEDOUT_DOUBLE,	STRIKEOUT_DOUBLE },
    { XML_CROSSEDOUT_THICK,	    STRIKEOUT_BOLD },
    { XML_CROSSEDOUT_SLASH,	    STRIKEOUT_SLASH },
    { XML_CROSSEDOUT_CROSS,	    STRIKEOUT_X }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLPosturePropHdl
//

XMLCrossedOutPropHdl::~XMLCrossedOutPropHdl()
{
    // nothing to do
}

sal_Bool XMLCrossedOutPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_uInt16 eCross;

    if( ( bRet = rUnitConverter.convertEnum( eCross, rStrImpValue, pXML_Crossedout_Enum ) ) )
        rValue <<= (sal_Int16)eCross;

    return bRet; 
}

sal_Bool XMLCrossedOutPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_Int16 nValue;
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_uInt16)nValue, pXML_Crossedout_Enum ) ) )
            rStrExpValue = aOut.makeStringAndClear();
    }
    
    return bRet;
}

}//end of namespace binfilter
