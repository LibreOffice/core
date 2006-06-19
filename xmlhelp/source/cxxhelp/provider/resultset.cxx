/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resultset.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:39:39 $
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

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _RESULTSET_HXX
#include <provider/resultset.hxx>
#endif
#ifndef _RESULTSETFACTORY_HXX
#include <provider/resultsetfactory.hxx>
#endif

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;


using namespace chelp;

//=========================================================================
//=========================================================================
//
// DynamicResultSet Implementation.
//
//=========================================================================
//=========================================================================

DynamicResultSet::DynamicResultSet(
    const Reference< XMultiServiceFactory >& rxSMgr,
    const vos::ORef< Content >& rxContent,
    const OpenCommandArgument2& rCommand,
    const Reference< XCommandEnvironment >& rxEnv,
    ResultSetFactory* pFactory )
    : ResultSetImplHelper( rxSMgr, rCommand ),
      m_xContent( rxContent ),
      m_xEnv( rxEnv ),
      m_pFactory( pFactory )
{
}

DynamicResultSet::~DynamicResultSet()
{
    delete m_pFactory;
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void DynamicResultSet::initStatic()
{
    m_xResultSet1 = Reference< XResultSet >( m_pFactory->createResultSet() );
}

//=========================================================================
void DynamicResultSet::initDynamic()
{
    m_xResultSet1 = Reference< XResultSet >( m_pFactory->createResultSet() );

    m_xResultSet2 = m_xResultSet1;
}




