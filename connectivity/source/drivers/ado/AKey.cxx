/*************************************************************************
 *
 *  $RCSfile: AKey.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-02 10:49:41 $
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

#ifndef _CONNECTIVITY_ADO_KEY_HXX_
#include "ado/AKey.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

// -------------------------------------------------------------------------
OAdoKey::OAdoKey(sal_Bool _bCase,OConnection* _pConnection, ADOKey* _pKey)
    : OKey_ADO(_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aKey = WpADOKey(_pKey);
    fillPropertyValues();
}
// -------------------------------------------------------------------------
OAdoKey::OAdoKey(sal_Bool _bCase,OConnection* _pConnection)
    : OKey_ADO(_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    m_aKey.Create();
}
// -------------------------------------------------------------------------
void OAdoKey::refreshColumns()
{
    TStringVector aVector;

    ADOColumns* pColumns = m_aKey.get_Columns();
    if(pColumns)
    {
        pColumns->Refresh();

        sal_Int32 nCount = 0;
        pColumns->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOColumn* pColumn = NULL;
            pColumns->get_Item(OLEVariant(i),&pColumn);
            if(pColumn)
            {
                WpADOColumn aColumn(pColumn);
                aVector.push_back(aColumn.get_Name());
            }
        }
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns = new OColumns(*this,m_aMutex,aVector,pColumns,isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoKey::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OAdoKey::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return OKey_ADO::getSomething(rId);
}
// -------------------------------------------------------------------------
void OAdoKey::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aKey.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aKey.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_TYPE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_Type(Map2KeyRule(nVal));
                }
                break;
            case PROPERTY_ID_REFERENCEDTABLE:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aKey.put_RelatedTable(aVal);
                }
                break;
            case PROPERTY_ID_UPDATERULE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_UpdateRule(Map2Rule(nVal));
                }
                break;
            case PROPERTY_ID_DELETERULE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_DeleteRule(Map2Rule(nVal));
                }
                break;
        }
    }
    OKey_ADO::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}
// -------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OAdoKey::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OKey_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoKey::release() throw(::com::sun::star::uno::RuntimeException)
{
    OKey_ADO::release();
}
// -----------------------------------------------------------------------------


