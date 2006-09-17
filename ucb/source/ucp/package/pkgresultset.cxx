/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pkgresultset.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:00:17 $
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
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 - This implementation is not a dynamic result set!!! It only implements
   the necessary interfaces, but never recognizes/notifies changes!!!

 *************************************************************************/

#ifndef _PKGDATASUPPLIER_HXX
#include "pkgdatasupplier.hxx"
#endif
#ifndef _PKGRESULTSET_HXX
#include "pkgresultset.hxx"
#endif

using namespace com::sun;
using namespace com::sun::star;

using namespace package_ucp;

//=========================================================================
//=========================================================================
//
// DynamicResultSet Implementation.
//
//=========================================================================
//=========================================================================

DynamicResultSet::DynamicResultSet(
              const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
              const rtl::Reference< Content >& rxContent,
              const star::ucb::OpenCommandArgument2& rCommand,
              const uno::Reference< star::ucb::XCommandEnvironment >& rxEnv )
: ResultSetImplHelper( rxSMgr, rCommand ),
  m_xContent( rxContent ),
  m_xEnv( rxEnv )
{
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void DynamicResultSet::initStatic()
{
    m_xResultSet1
        = new ::ucb::ResultSet( m_xSMgr,
                                m_aCommand.Properties,
                                new DataSupplier( m_xSMgr,
                                                  m_xContent,
                                                  m_aCommand.Mode ),
                                m_xEnv );
}

//=========================================================================
void DynamicResultSet::initDynamic()
{
    m_xResultSet1
        = new ::ucb::ResultSet( m_xSMgr,
                                m_aCommand.Properties,
                                new DataSupplier( m_xSMgr,
                                                  m_xContent,
                                                  m_aCommand.Mode ),
                                m_xEnv );
    m_xResultSet2 = m_xResultSet1;
}

