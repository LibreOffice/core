/*************************************************************************
 *
 *  $RCSfile: AColumn.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: oj $ $Date: 2002-07-11 06:56:36 $
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

#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#include "ado/AColumn.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

void WpADOColumn::Create()
{
    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr = -1;

    _ADOColumn* pColumn = NULL;
    hr = CoCreateInstance(ADOS::CLSID_ADOCOLUMN_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOCOLUMN_25,
                          (void**)&pColumn );


    if( !FAILED( hr ) )
    {
        operator=( pColumn );
        pColumn->Release( );
    }
}
// -------------------------------------------------------------------------
OAdoColumn::OAdoColumn(sal_Bool _bCase,OConnection* _pConnection,_ADOColumn* _pColumn)
    : connectivity::sdbcx::OColumn(::rtl::OUString(),::rtl::OUString(),::rtl::OUString(),0,0,0,0,sal_False,sal_False,sal_False,_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    OSL_ENSURE(_pColumn,"Column can not be null!");
    m_aColumn = WpADOColumn(_pColumn);
    fillPropertyValues();
}
// -------------------------------------------------------------------------
OAdoColumn::OAdoColumn(sal_Bool _bCase,OConnection* _pConnection)
    : connectivity::sdbcx::OColumn(_bCase)
    ,m_pConnection(_pConnection)
{
    m_aColumn.Create();
    construct();
    fillPropertyValues();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoColumn::getUnoTunnelImplementationId()
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
sal_Int64 OAdoColumn::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            OColumn_ADO::getSomething(rId);
}
// -------------------------------------------------------------------------
void OAdoColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISASCENDING),     PROPERTY_ID_ISASCENDING,    nAttrib,&m_IsAscending, ::getBooleanCppuType());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN),   PROPERTY_ID_RELATEDCOLUMN,  nAttrib,&m_ReferencedColumn,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -----------------------------------------------------------------------------
void OAdoColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aColumn.IsValid())
    {
        const sal_Char* pAdoPropertyName = NULL;

        switch(nHandle)
        {
            case PROPERTY_ID_ISASCENDING:
                m_aColumn.put_SortOrder(::cppu::any2bool(rValue) ? adSortAscending : adSortDescending);
                break;
            case PROPERTY_ID_RELATEDCOLUMN:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aColumn.put_RelatedColumn(aVal);
                }
                break;
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aColumn.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_TYPE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aColumn.put_Type(ADOS::MapJdbc2ADOType(nVal,m_pConnection->getEngineType()));
                }
                break;
            case PROPERTY_ID_TYPENAME:
                //  rValue <<= m_pTable->getCatalog()->getConnection()->getTypeInfo()->find();
                break;
            case PROPERTY_ID_PRECISION:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aColumn.put_Precision(nVal);
                }
                break;
            case PROPERTY_ID_SCALE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aColumn.put_NumericScale((sal_Int8)nVal);
                }
                break;
            case PROPERTY_ID_ISNULLABLE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    if ( nVal == ColumnValue::NULLABLE )
                        m_aColumn.put_Attributes( adColNullable );
                }
                break;
            case PROPERTY_ID_ISROWVERSION:
                break;

            case PROPERTY_ID_ISAUTOINCREMENT:
                pAdoPropertyName = "Autoincrement";
                break;

            case PROPERTY_ID_DESCRIPTION:
                pAdoPropertyName = "Default";
                break;

            case PROPERTY_ID_DEFAULTVALUE:
                pAdoPropertyName = "Description";
                break;

            case PROPERTY_ID_ISCURRENCY:
                m_aColumn.put_Type(adCurrency);
                break;
        }

        if ( pAdoPropertyName )
            OTools::putValue( m_aColumn.get_Properties(), ::rtl::OUString::createFromAscii( pAdoPropertyName ), getString( rValue ) );

    }
    OColumn_ADO::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}
namespace
{
    const ::connectivity::OTypeInfo* getTypeInfoFromType(const OTypeInfoMap& _rTypeInfo,
                               sal_Int32 _nType,
                               const ::rtl::OUString& _sTypeName,
                               sal_Int32 _nPrecision,
                               sal_Int32 _nScale,
                               sal_Bool& _brForceToType)
    {
        const ::connectivity::OTypeInfo* pTypeInfo = NULL;
        _brForceToType = sal_False;
        // search for type
        ::std::pair<OTypeInfoMap::const_iterator, OTypeInfoMap::const_iterator> aPair = _rTypeInfo.equal_range(_nType);
        OTypeInfoMap::const_iterator aIter = aPair.first;
        if(aIter != _rTypeInfo.end()) // compare with end is correct here
        {
            for(;aIter != aPair.second;++aIter)
            {
                // search the best matching type
        #ifdef DBG_UTIL
                ::rtl::OUString sDBTypeName = aIter->second->aTypeName;
                sal_Int32       nDBTypePrecision = aIter->second->nPrecision;
                sal_Int32       nDBTypeScale = aIter->second->nMaximumScale;
        #endif
                if  (   (   !_sTypeName.getLength()
                        ||  (aIter->second->aTypeName.equalsIgnoreAsciiCase(_sTypeName))
                        )
                    &&  (aIter->second->nPrecision      >= _nPrecision)
                    &&  (aIter->second->nMaximumScale   >= _nScale)
                    )
                    break;
            }

            if (aIter == aPair.second)
            {
                for(aIter = aPair.first; aIter != aPair.second; ++aIter)
                {
                    // search the best matching type (now comparing the local names)
                    if  (   (aIter->second->aLocalTypeName.equalsIgnoreAsciiCase(_sTypeName))
                        &&  (aIter->second->nPrecision      >= _nPrecision)
                        &&  (aIter->second->nMaximumScale   >= _nScale)
                        )
                    {
    // we can not assert here because we could be in d&d
    /*
                        OSL_ENSURE(sal_False,
                            (   ::rtl::OString("getTypeInfoFromType: assuming column type ")
                            +=  ::rtl::OString(aIter->second->aTypeName.getStr(), aIter->second->aTypeName.getLength(), gsl_getSystemTextEncoding())
                            +=  ::rtl::OString("\" (expected type name ")
                            +=  ::rtl::OString(_sTypeName.getStr(), _sTypeName.getLength(), gsl_getSystemTextEncoding())
                            +=  ::rtl::OString(" matches the type's local name).")).getStr());
    */
                        break;
                    }
                }
            }

            if (aIter == aPair.second)
            {   // no match for the names, no match for the local names
                // -> drop the precision and the scale restriction, accept any type with the property
                // type id (nType)

                // we can not assert here because we could be in d&d
    /*
                OSL_ENSURE(sal_False,
                    (   ::rtl::OString("getTypeInfoFromType: did not find a matching type")
                    +=  ::rtl::OString(" (expected type name: ")
                    +=  ::rtl::OString(_sTypeName.getStr(), _sTypeName.getLength(), gsl_getSystemTextEncoding())
                    +=  ::rtl::OString(")! Defaulting to the first matching type.")).getStr());
    */
                pTypeInfo = aPair.first->second;
                _brForceToType = sal_True;
            }
            else
                pTypeInfo = aIter->second;
        }
        else
        {
            ::comphelper::TStringMixEqualFunctor aCase(sal_False);
            // search for typeinfo where the typename is equal _sTypeName
            OTypeInfoMap::const_iterator aFind = ::std::find_if(_rTypeInfo.begin(),
                                                                _rTypeInfo.end(),
                                                                ::std::compose1(
                                                                    ::std::bind2nd(aCase, _sTypeName),
                                                                    ::std::compose1(
                                                                        ::std::mem_fun(&::connectivity::OTypeInfo::getDBName),
                                                                        ::std::select2nd<OTypeInfoMap::value_type>())
                                                                    )
                                                                );
            if(aFind != _rTypeInfo.end())
                pTypeInfo = aFind->second;
        }

    // we can not assert here because we could be in d&d
    //  OSL_ENSURE(pTypeInfo, "getTypeInfoFromType: no type info found for this type!");
        return pTypeInfo;
    }
}
// -------------------------------------------------------------------------
void OAdoColumn::fillPropertyValues()
{
    if(m_aColumn.IsValid())
    {
        m_IsAscending = m_aColumn.get_SortOrder() == adSortAscending;
        m_ReferencedColumn = m_aColumn.get_RelatedColumn();
        m_Name = m_aColumn.get_Name();
        m_Precision     = m_aColumn.get_Precision();
        m_Scale         = m_aColumn.get_NumericScale();
        m_IsNullable    = ((m_aColumn.get_Attributes() & adColNullable) == adColNullable) ? ColumnValue::NULLABLE : ColumnValue::NO_NULLS;
        m_IsCurrency    = (m_aColumn.get_Type() == adCurrency);
        m_Type = ADOS::MapADOType2Jdbc(m_aColumn.get_Type());

        sal_Bool bForceTo = sal_True;
        const OTypeInfoMap* pTypeInfoMap = m_pConnection->getTypeInfo();
        const ::connectivity::OTypeInfo* pTypeInfo = getTypeInfoFromType(*m_pConnection->getTypeInfo(),m_Type,::rtl::OUString(),m_Precision,m_Scale,bForceTo);
        if ( pTypeInfo )
                m_TypeName = pTypeInfo->aTypeName;

        // fill some specific props
        {
            WpADOProperties aProps( m_aColumn.get_Properties() );

            if ( aProps.IsValid() )
            {
                m_IsAutoIncrement = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Autoincrement")) );

                m_Description = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")) );

                m_DefaultValue = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Default")) );
            }
        }
    }
}
// -----------------------------------------------------------------------------
WpADOColumn OAdoColumn::getColumnImpl() const
{
    return m_aColumn;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OAdoColumn::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OColumn_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoColumn::release() throw(::com::sun::star::uno::RuntimeException)
{
    OColumn_ADO::release();
}
// -----------------------------------------------------------------------------



