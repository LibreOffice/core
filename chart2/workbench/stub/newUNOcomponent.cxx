/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newUNOcomponent.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:40:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
