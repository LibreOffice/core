/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AColumns.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:23:13 $
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

#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#include "ado/AColumn.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

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
sdbcx::ObjectType OColumns::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OAdoColumn* pColumn = NULL;
    if ( !getImplementation( pColumn, descriptor ) || pColumn == NULL )
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not append column: invalid column descriptor." ),
            static_cast<XTypeProvider*>(this)
        );

    WpADOColumn aColumn = pColumn->getColumnImpl();
    DataTypeEnum eType = aColumn.get_Type();

#if OSL_DEBUG_LEVEL > 0
    sal_Int32 nPrecision    = aColumn.get_Precision();  (void)nPrecision;
    sal_Int32 nScale        = aColumn.get_NumericScale(); (void)nScale;
    sal_Int32 nType         = ADOS::MapADOType2Jdbc(eType); (void)nType;
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



