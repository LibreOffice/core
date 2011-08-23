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

#ifndef _XMLOFF_PROPERTYHANDLER_POSTURETYPES_HXX
#include <postuhdl.hxx>
#endif


#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_ 
#include <rtl/ustrbuf.hxx>
#endif


#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif

#ifndef _VCL_VCLENUM_HXX 
#include <vcl/vclenum.hxx>
#endif

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

SvXMLEnumMapEntry __READONLY_DATA aPostureGenericMapping[] =
{
    { XML_POSTURE_NORMAL,		ITALIC_NONE		},
    { XML_POSTURE_ITALIC,		ITALIC_NORMAL	},
    { XML_POSTURE_OBLIQUE,		ITALIC_OBLIQUE	},
    { XML_TOKEN_INVALID,		0 				}
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLPosturePropHdl
//

XMLPosturePropHdl::~XMLPosturePropHdl()
{
    // nothing to do
}

sal_Bool XMLPosturePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    sal_uInt16 ePosture;

    if( ( bRet = rUnitConverter.convertEnum( ePosture, rStrImpValue, aPostureGenericMapping ) ) )
        rValue <<= (awt::FontSlant)ePosture;

    return bRet; 
}

sal_Bool XMLPosturePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    OUStringBuffer aOut;
    awt::FontSlant eSlant;

    if( !( rValue >>= eSlant ) )
    {
        sal_Int32 nValue;
        
        if( !( rValue >>= nValue ) )
            return sal_False;

        eSlant = (awt::FontSlant)nValue;
    }

    if( ( bRet = rUnitConverter.convertEnum( aOut, (sal_Int32)eSlant, aPostureGenericMapping ) ) )
        rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

}//end of namespace binfilter
