/*************************************************************************
 *
 *  $RCSfile: AIndex.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-30 08:00:28 $
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

#ifndef _CONNECTIVITY_ADO_INDEX_HXX_
#include "ado/AIndex.hxx"
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
#define CONNECTIVITY_PROPERTY_NAME_SPACE ado
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
void WpADOIndex::Create()
{
    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr = -1;
    _ADOIndex* pCommand;
    hr = CoCreateInstance(ADOS::CLSID_ADOINDEX_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOINDEX_25,
                          (void**)&pCommand );


    if( !FAILED( hr ) )
    {
        pInterface = pCommand;
        pInterface->AddRef();
    }
}
// -------------------------------------------------------------------------
OAdoIndexDescriptor::OAdoIndexDescriptor(sal_Bool _bCase,   ADOIndex* _pIndex) : OIndexDescriptor_ADO(_bCase)
{
    construct();
    if(_pIndex)
        m_aIndex = WpADOIndex(_pIndex);
    else
        m_aIndex.Create();

    refreshColumns();
}
IMPLEMENT_SERVICE_INFO(OAdoIndex,"com.sun.star.sdbcx.OAdoIndex","com.sun.star.sdbcx.Index");
// -------------------------------------------------------------------------
OAdoIndex::OAdoIndex(sal_Bool _bCase,   ADOIndex* _pIndex) : OAdoIndexDescriptor(_bCase,_pIndex)
{
}
// -----------------------------------------------------------------------------
void OAdoIndexDescriptor::refreshColumns()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOColumns* pColumns = m_aIndex.get_Columns();
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
        delete m_pColumns;
    m_pColumns = new OColumns(*this,m_aMutex,aVector,pColumns,isCaseSensitive());
}

// -------------------------------------------------------------------------
Any SAL_CALL OAdoIndexDescriptor::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;
    return OIndexDescriptor_ADO::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OAdoIndexDescriptor::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OIndexDescriptor_ADO::getTypes());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoIndexDescriptor::getUnoTunnelImplementationId()
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
sal_Int64 OAdoIndexDescriptor::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoIndexDescriptor::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aIndex.IsValid())
    {

        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aIndex.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_CATALOG:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aIndex.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_ISUNIQUE:
                m_aIndex.put_Unique(getBOOL(rValue));
                break;
            case PROPERTY_ID_ISPRIMARYKEYINDEX:
                m_aIndex.put_PrimaryKey(getBOOL(rValue));
                break;
            case PROPERTY_ID_ISCLUSTERED:
                m_aIndex.put_Clustered(getBOOL(rValue));
                break;
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OAdoIndexDescriptor::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_aIndex.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aIndex.get_Name();
                break;
            case PROPERTY_ID_CATALOG:
                //  rValue <<= getResultSetType();
                break;
            case PROPERTY_ID_ISUNIQUE:
                {
                    sal_Bool _b = m_aIndex.get_Unique();
                                        rValue <<= Any(&_b, ::getBooleanCppuType());
                }
                break;
            case PROPERTY_ID_ISPRIMARYKEYINDEX:
                {
                    sal_Bool _b = m_aIndex.get_PrimaryKey();
                                        rValue <<= Any(&_b, ::getBooleanCppuType());
                }
                break;
            case PROPERTY_ID_ISCLUSTERED:
                {
                    sal_Bool _b = m_aIndex.get_Clustered();
                                        rValue <<= Any(&_b, ::getBooleanCppuType());
                }
                break;
        }
    }
}




