/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "vbapivottable.hxx"
#include "vbapivotcache.hxx"


using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaPivotTable::ScVbaPivotTable( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XDataPilotTable >& xTable ) : PivotTableImpl_BASE( uno::Reference< XHelperInterface >(),  xContext), m_xTable( xTable )
{
}

uno::Reference< excel::XPivotCache >
ScVbaPivotTable::PivotCache() throw (uno::RuntimeException)
{
    // #FIXME with a quick example failed to determine what the parent
    // should be, leaving as null at the moment
    return new ScVbaPivotCache( uno::Reference< XHelperInterface >(), mxContext, m_xTable );
}

rtl::OUString&
ScVbaPivotTable::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaPivotTable") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaPivotTable::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.PivotTable" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
