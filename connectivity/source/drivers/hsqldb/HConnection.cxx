/*************************************************************************
 *
 *  $RCSfile: HConnection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:39:26 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/
#include "hsqldb/HConnection.hxx"
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::embed;

void SAL_CALL OConnectionWeakWrapper::disposing(void)
{
    OConnectionWeakWrapper_BASE::disposing();
    OConnectionWrapper::disposing();
}
// -----------------------------------------------------------------------------
OConnectionWeakWrapper::OConnectionWeakWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB) : OConnectionWeakWrapper_BASE(m_aMutex)
{
    setDelegation(_xConnection,_xORB,m_refCount);
}
// -----------------------------------------------------------------------------
OConnectionWeakWrapper::~OConnectionWeakWrapper()
{
    if ( !OConnectionWeakWrapper_BASE::rBHelper.bDisposed )
    {
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)
IMPLEMENT_SERVICE_INFO(OConnectionWeakWrapper, "com.sun.star.sdbc.drivers.hsqldb.OConnectionWeakWrapper", "com.sun.star.sdbc.Connection")
IMPLEMENT_FORWARD_XTYPEPROVIDER2(OConnectionWeakWrapper,OConnectionWeakWrapper_BASE,OConnectionWrapper)
// -----------------------------------------------------------------------------
// XTransactedObject
void SAL_CALL OConnectionWeakWrapper::commit(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    try
    {
        if ( m_xConnection.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement> xStmt = m_xConnection->createStatement();
            if ( xStmt.is() )
                xStmt->execute(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CHECKPOINT")));
        }
    }
    catch(::com::sun::star::uno::Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OConnectionWeakWrapper::revert(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------

