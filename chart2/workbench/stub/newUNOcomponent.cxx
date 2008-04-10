/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: newUNOcomponent.cxx,v $
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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
//replace XXXX and YYYY and ...
#ifndef _CHART2_XXXX_HXX
#include "XXXX.hxx"
#endif
#ifndef _CHART2_SERVICENAMES_CHARTTYPES_HXX
#include "servicenames_ ... .hxx"
#endif

//.............................................................................
namespace chart
{
//.............................................................................

XXXX::XXXX(
        uno::Reference<uno::XComponentContext> const & xContext)
{
    m_xMCF = xContext->getServiceManager();
}

XXXX::~XXXX()
{
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------

APPHELPER_XSERVICEINFO_IMPL(XXXX,CHART2_VIEW_XXXX_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > XXXX
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_XXXX_SERVICE_NAME;
    return aSNS;
}

//-----------------------------------------------------------------
// chart2::YYYY
//-----------------------------------------------------------------


//.............................................................................
} //namespace chart
//.............................................................................
