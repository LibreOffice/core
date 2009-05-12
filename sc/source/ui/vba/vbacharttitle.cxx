/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbacharttitle.cxx,v $
 * $Revision: 1.3 $
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
#include "vbacharttitle.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaChartTitle::ScVbaChartTitle( const uno::Reference< XHelperInterface >& xParent,  const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XShape >& _xTitleShape ) :  ChartTitleBase( xParent, xContext, _xTitleShape )
{
}

rtl::OUString&
ScVbaChartTitle::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaChartTitle") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaChartTitle::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        uno::Sequence< rtl::OUString > BaseServiceNames = ChartTitleBase::getServiceNames();
        aServiceNames.realloc( BaseServiceNames.getLength() + 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Chart" ) );
        for ( sal_Int32 index = 1; index < (BaseServiceNames.getLength() + 1); ++index )
            aServiceNames[ index ] = BaseServiceNames[ index ];
    }
    return aServiceNames;
}


