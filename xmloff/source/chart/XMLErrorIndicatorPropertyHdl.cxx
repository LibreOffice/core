/*************************************************************************
 *
 *  $RCSfile: XMLErrorIndicatorPropertyHdl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bm $ $Date: 2000-12-15 17:45:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "XMLErrorIndicatorPropertyHdl.hxx"

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTERRORINDICATORTYPE_HPP_
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace com::sun::star;

XMLErrorIndicatorPropertyHdl::~XMLErrorIndicatorPropertyHdl()
{}

sal_Bool XMLErrorIndicatorPropertyHdl::importXML( const ::rtl::OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bValue;
    SvXMLUnitConverter::convertBool( bValue, rStrImpValue );

    // modify existing value
    chart::ChartErrorIndicatorType eType = chart::ChartErrorIndicatorType_NONE;
    if( rValue.hasValue())
        rValue >>= eType;

    if( bValue )    // enable flag
    {
        if( eType != chart::ChartErrorIndicatorType_TOP_AND_BOTTOM )
        {
            if( mbUpperIndicator )
                eType = ( eType == chart::ChartErrorIndicatorType_LOWER )
                    ? chart::ChartErrorIndicatorType_TOP_AND_BOTTOM
                    : chart::ChartErrorIndicatorType_UPPER;
            else
                eType = ( eType == chart::ChartErrorIndicatorType_UPPER )
                    ? chart::ChartErrorIndicatorType_TOP_AND_BOTTOM
                    : chart::ChartErrorIndicatorType_LOWER;
        }
    }
    else            // disable flag
    {
        if( eType != chart::ChartErrorIndicatorType_NONE )
        {
            if( mbUpperIndicator )
                eType = ( eType == chart::ChartErrorIndicatorType_UPPER )
                    ? chart::ChartErrorIndicatorType_NONE
                    : chart::ChartErrorIndicatorType_LOWER;
            else
                eType = ( eType == chart::ChartErrorIndicatorType_LOWER )
                    ? chart::ChartErrorIndicatorType_NONE
                    : chart::ChartErrorIndicatorType_UPPER;
        }
    }

    rValue <<= eType;

    return sal_True;
}

sal_Bool XMLErrorIndicatorPropertyHdl::exportXML( ::rtl::OUString& rStrExpValue,
                                                  const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    rtl::OUStringBuffer aBuffer;
    chart::ChartErrorIndicatorType eType;

    rValue >>= eType;
    sal_Bool bValue = ( eType == chart::ChartErrorIndicatorType_TOP_AND_BOTTOM ||
                        ( mbUpperIndicator
                          ? ( eType == chart::ChartErrorIndicatorType_UPPER )
                          : ( eType == chart::ChartErrorIndicatorType_LOWER )));

    if( bValue )
    {
        SvXMLUnitConverter::convertBool( aBuffer, bValue );
        rStrExpValue = aBuffer.makeStringAndClear();
    }

    // only export if set to true
    return bValue;
}
