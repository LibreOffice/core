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



#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLPERCENTORMEASUREPROPERTYHANDLER_HXX
#include "XMLPercentOrMeasurePropertyHandler.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

XMLPercentOrMeasurePropertyHandler::XMLPercentOrMeasurePropertyHandler( sal_Bool bPercent )
: mbPercent( bPercent )
{
}

XMLPercentOrMeasurePropertyHandler::~XMLPercentOrMeasurePropertyHandler()
{
}

sal_Bool XMLPercentOrMeasurePropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    if( (rStrImpValue.indexOf( sal_Unicode('%') ) != -1) != mbPercent )
        return sal_False;

    sal_Int32 nValue;

    if( mbPercent )
    {
        if( !rUnitConverter.convertPercent( nValue, rStrImpValue ) )
            return sal_False;
    }
    else
    {
        if( !rUnitConverter.convertMeasure( nValue, rStrImpValue ) )
            return sal_False;
    }

    rValue <<= nValue;
    return sal_True;
}

sal_Bool XMLPercentOrMeasurePropertyHandler::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
      OUStringBuffer aOut;

    sal_Int32 nValue;
    if( !(rValue >>= nValue ) )
        return sal_False;

    if( mbPercent )
    {
         rUnitConverter.convertPercent( aOut, nValue );
    }
    else
    {
         rUnitConverter.convertMeasure( aOut, nValue );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}
}//end of namespace binfilter
