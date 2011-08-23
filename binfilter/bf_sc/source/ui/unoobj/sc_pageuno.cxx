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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "pageuno.hxx"
#include "shapeuno.hxx"

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

ScPageObj::ScPageObj( SdrPage* pPage ) :
    SvxFmDrawPage( pPage )
{
}

ScPageObj::~ScPageObj() throw()
{
}

uno::Reference<drawing::XShape > ScPageObj::_CreateShape( SdrObject *pObj ) const throw()
{
    uno::Reference<drawing::XShape> xShape = SvxFmDrawPage::_CreateShape( pObj );

    new ScShapeObj( xShape );		// aggregates object and modifies xShape

    return xShape;
}

::rtl::OUString SAL_CALL ScPageObj::getImplementationName()
                                throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScPageObj"));
}

sal_Bool SAL_CALL ScPageObj::supportsService( const ::rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr = rServiceName;
    return aServiceStr.EqualsAscii( "com.sun.star.sheet.SpreadsheetDrawPage" );
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScPageObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aRet(1);
    ::rtl::OUString* pArray = aRet.getArray();
    pArray[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDrawPage"));
    return aRet;
}
}
