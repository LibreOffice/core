/*************************************************************************
 *
 *  $RCSfile: propimp0.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:03 $
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

#pragma hdrstop

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _PROPIMP0_HXX
#include "propimp0.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// implementation of graphic property Stroke


//////////////////////////////////////////////////////////////////////////////
// implementation of presentation page property Change


XMLDashArrayPropertyHdl::~XMLDashArrayPropertyHdl()
{
}

sal_Bool XMLDashArrayPropertyHdl::equals(
    const ::com::sun::star::uno::Any& r1,
    const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Int32 nChange1, nChange2;

    if((r1 >>= nChange1) && (r2 >>= nChange2))
        return (nChange1 == nChange2);
    return sal_False;
}

sal_Bool XMLDashArrayPropertyHdl::importXML(
    const OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRetval(sal_False);


    return bRetval;
}

sal_Bool XMLDashArrayPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int32 nVal;
    sal_Bool bRetval(sal_False);

    if(rValue >>= nVal)
    {

    sal_Int16 Dots;
    sal_Int32 DotLen;
    sal_Int16 Dashes;
    sal_Int32 DashLen;
    sal_Int32 Distance;

    }

    return bRetval;
}

//////////////////////////////////////////////////////////////////////////////
// implementation of an effect duration property handler


XMLDurationPropertyHdl::~XMLDurationPropertyHdl()
{
}

sal_Bool XMLDurationPropertyHdl::importXML(
    const OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    const String aStr( rStrImpValue );

    sal_Int32 nVal = aStr.ToInt32();
    rValue <<= nVal;

    return sal_True;
}

sal_Bool XMLDurationPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int32 nVal;

    if(rValue >>= nVal)
    {
        OUStringBuffer aOut;
        aOut.append( nVal );
        aOut.append( OUString::createFromAscii( "s" ));
        rStrExpValue = aOut.makeStringAndClear();
        return sal_True;
    }

    return sal_False;
}


