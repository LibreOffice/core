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

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif


#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HDL_ 
#include <com/sun/star/awt/Rectangle.hdl>
#endif

#ifndef _XMLOFF_XMLRECTANGLEMEMBERSHANDLER_HXX
#include "XMLRectangleMembersHandler.hxx"
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

XMLRectangleMembersHdl::XMLRectangleMembersHdl( sal_Int32 nType )
: mnType( nType )
{
}

    sal_Int32 X;
    sal_Int32 Y;
    sal_Int32 Width;
    sal_Int32 Height;

XMLRectangleMembersHdl::~XMLRectangleMembersHdl()
{
}

sal_Bool XMLRectangleMembersHdl::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    awt::Rectangle aRect( 0, 0, 0, 0 );
    if( rValue.hasValue() )
        rValue >>= aRect;

    sal_Int32 nValue;

    if( rUnitConverter.convertMeasure( nValue, rStrImpValue ) )
    {
        switch( mnType )
        {
            case XML_TYPE_RECTANGLE_LEFT :
                aRect.X = nValue;
                break;
            case XML_TYPE_RECTANGLE_TOP :
                aRect.Y = nValue;
                break;
            case XML_TYPE_RECTANGLE_WIDTH :
                aRect.Width = nValue;
                break;
            case XML_TYPE_RECTANGLE_HEIGHT :
                aRect.Height = nValue;
                break;
        }

        rValue <<= aRect;
        return sal_True;
    }

    return sal_False;
}

sal_Bool XMLRectangleMembersHdl::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    awt::Rectangle aRect( 0, 0, 0, 0 );
    rValue >>= aRect;

    sal_Int32 nValue;

    switch( mnType )
    {
        case XML_TYPE_RECTANGLE_LEFT :
            nValue = aRect.X;
            break;
        case XML_TYPE_RECTANGLE_TOP :
            nValue = aRect.Y;
            break;
        case XML_TYPE_RECTANGLE_WIDTH :
            nValue = aRect.Width;
            break;
        case XML_TYPE_RECTANGLE_HEIGHT :
            nValue = aRect.Height;
            break;
        default:
            nValue = 0;  // TODO What value should this be?
            break;

    }

    ::rtl::OUStringBuffer sBuffer;
    rUnitConverter.convertMeasure( sBuffer, nValue );
    rStrExpValue = sBuffer.makeStringAndClear();
    return sal_True;
}

}//end of namespace binfilter
