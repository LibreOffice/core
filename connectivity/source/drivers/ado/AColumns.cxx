/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "ado/AColumns.hxx"
#include "ado/AColumn.hxx"
#include "ado/AConnection.hxx"
#include "ado/Awrapado.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <algorithm>
#include "resource/ado_res.hrc"

using namespace connectivity::ado;
using namespace connectivity;
using namespace comphelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

sdbcx::ObjectType OColumns::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoColumn(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createDescriptor()
{
    return new OAdoColumn(isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OColumns::appendObject( const ::rtl::OUString&, const Reference< XPropertySet >& descriptor )
{
    OAdoColumn* pColumn = NULL;
    Reference< XPropertySet > xColumn;
    if ( !getImplementation( pColumn, descriptor ) || pColumn == NULL )
    {
        // m_pConnection->throwGenericSQLException( STR_INVALID_COLUMN_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );
        pColumn = new OAdoColumn(isCaseSensitive(),m_pConnection);
        xColumn = pColumn;
        ::comphelper::copyProperties(descriptor,xColumn);
    }

    WpADOColumn aColumn = pColumn->getColumnImpl();

#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nPrecision;
    sal_Int32 nScale;
    sal_Int32 nType;
    nPrecision = aColumn.get_Precision();
    nScale = aColumn.get_NumericScale();
    nType = ADOS::MapADOType2Jdbc(aColumn.get_Type());
#endif

    ::rtl::OUString sTypeName;
    pColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)) >>= sTypeName;

    const OTypeInfoMap* pTypeInfoMap = m_pConnection->getTypeInfo();
    ::comphelper::TStringMixEqualFunctor aCase(sal_False);
    // search for typeinfo where the typename is equal sTypeName
    OTypeInfoMap::const_iterator aFind = ::std::find_if(pTypeInfoMap->begin(),
                                                        pTypeInfoMap->end(),
                                                        ::std::compose1(
                                                            ::std::bind2nd(aCase, sTypeName),
                                                            ::std::compose1(
                                                                ::std::mem_fun(&OExtendedTypeInfo::getDBName),
                                                                ::std::select2nd<OTypeInfoMap::value_type>())
                                                            )

                                                );

    if ( aFind != pTypeInfoMap->end() ) // change column type if necessary
        aColumn.put_Type(aFind->first);

    if ( SUCCEEDED(((ADOColumns*)m_aCollection)->Append(OLEVariant(aColumn.get_Name()),aColumn.get_Type(),aColumn.get_DefinedSize())) )
    {
        WpADOColumn aAddedColumn = m_aCollection.GetItem(OLEVariant(aColumn.get_Name()));
        if ( aAddedColumn.IsValid() )
        {
            sal_Bool bAutoIncrement = sal_False;
            pColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;
            if ( bAutoIncrement )
                OTools::putValue( aAddedColumn.get_Properties(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Autoincrement")), bAutoIncrement );

            if ( aFind != pTypeInfoMap->end() &&  aColumn.get_Type() != aAddedColumn.get_Type() ) // change column type if necessary
                aColumn.put_Type(aFind->first);
            aAddedColumn.put_Precision(aColumn.get_Precision());
            aAddedColumn.put_NumericScale(aColumn.get_NumericScale());
            aAddedColumn.put_Attributes(aColumn.get_Attributes());
            aAddedColumn.put_SortOrder(aColumn.get_SortOrder());
            aAddedColumn.put_RelatedColumn(aColumn.get_RelatedColumn());
        }
    }
    ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));

    return new OAdoColumn(isCaseSensitive(),m_pConnection,pColumn->getColumnImpl());
}
// -------------------------------------------------------------------------
// XDrop
void OColumns::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    if(!m_aCollection.Delete(_sElementName))
        ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
}
// -----------------------------------------------------------------------------



