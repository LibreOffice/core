/*************************************************************************
 *
 *  $RCSfile: CIndexes.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:58:26 $
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
#ifndef DBACCESS_INDEXES_HXX_
#include "CIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#include "connectivity/sdbcx/VIndex.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_INDEXTYPE_HPP_
#include <com/sun/star/sdbc/IndexType.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbaccess;
using namespace cppu;


Reference< XNamed > OIndexes::createObject(const ::rtl::OUString& _rName)
{
    Reference< XNamed > xRet = NULL;
    if ( m_xIndexes.is() && m_xIndexes->hasByName(_rName) )
        xRet.set(m_xIndexes->getByName(_rName),UNO_QUERY);
    else
        xRet = OIndexesHelper::createObject(_rName);

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createEmptyObject()
{
    Reference<XDataDescriptorFactory> xData( m_xIndexes,UNO_QUERY);
    if(xData.is())
        return xData->createDataDescriptor();
    else
        return OIndexesHelper::createEmptyObject();
}
// -------------------------------------------------------------------------
// XAppend
void OIndexes::appendObject( const Reference< XPropertySet >& descriptor )
{
    Reference<XAppend> xData( m_xIndexes,UNO_QUERY);
    if ( xData.is() )
        xData->appendByDescriptor(descriptor);
    else
        OIndexesHelper::appendObject(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void OIndexes::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if ( m_xIndexes.is() )
    {
        Reference<XDrop> xData( m_xIndexes,UNO_QUERY);
        if ( xData.is() )
            xData->dropByName(_sElementName);
    }
    else
        OIndexesHelper::dropObject(_nPos,_sElementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OIndexes::disposing(void)
{
    if ( m_xIndexes.is() )
        clear_NoDispose();
    else
        OIndexesHelper::disposing();
}
// -----------------------------------------------------------------------------




