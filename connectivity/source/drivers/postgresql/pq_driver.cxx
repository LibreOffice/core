/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
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
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

#include "pq_driver.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::Any;

using com::sun::star::beans::PropertyValue;

using com::sun::star::sdbc::XConnection;
using com::sun::star::sdbc::DriverPropertyInfo;

using com::sun::star::sdbcx::XTablesSupplier;


namespace pq_sdbc_driver
{

Reference< XConnection > Driver::connect(
    const OUString& url,const Sequence< PropertyValue >& info )
{
    if( ! acceptsURL( url ) )  // XDriver spec tells me to do so ...
        return Reference< XConnection > ();

    Sequence< Any > seq{ Any(url), Any(info) };
    return Reference< XConnection> (
        m_smgr->createInstanceWithArgumentsAndContext(
            u"org.openoffice.comp.connectivity.pq.Connection.noext"_ustr,
            seq, m_ctx ),
        UNO_QUERY );
}

sal_Bool Driver::acceptsURL( const OUString& url )
{
    return url.startsWith( "sdbc:postgresql:" );
}

Sequence< DriverPropertyInfo > Driver::getPropertyInfo(
    const OUString&,const Sequence< PropertyValue >& )
{
    return Sequence< DriverPropertyInfo > ();
}

sal_Int32  Driver::getMajorVersion(  )
{
    return PQ_SDBC_MAJOR;
}


sal_Int32 Driver::getMinorVersion(  )
{
    return PQ_SDBC_MINOR;
}

    // XServiceInfo
OUString SAL_CALL Driver::getImplementationName()
{
    return u"org.openoffice.comp.connectivity.pq.Driver.noext"_ustr;
}

sal_Bool Driver::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > Driver::getSupportedServiceNames()
{
    return { u"com.sun.star.sdbc.Driver"_ustr };
}

// XComponent
void Driver::disposing()
{

}


Reference< XTablesSupplier > Driver::getDataDefinitionByConnection(
    const Reference< XConnection >& connection )
{
     return Reference< XTablesSupplier >( connection , UNO_QUERY );
}

Reference< XTablesSupplier > Driver::getDataDefinitionByURL(
    const OUString& url, const Sequence< PropertyValue >& info )
{
    return Reference< XTablesSupplier > ( connect( url, info ), UNO_QUERY );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_pq_sdbc_driver_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pq_sdbc_driver::Driver(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
