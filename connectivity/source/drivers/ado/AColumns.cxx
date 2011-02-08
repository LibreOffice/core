/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#ifdef __MINGW32__
#include <algorithm>
#endif
#include "resource/ado_res.hrc"

#include <o3tl/compat_functional.hxx>

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
    if ( !getImplementation( pColumn, descriptor ) || pColumn == NULL )
        m_pConnection->throwGenericSQLException( STR_INVALID_COLUMN_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

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
                                                        ::o3tl::compose1(
                                                            ::std::bind2nd(aCase, sTypeName),
                                                            ::o3tl::compose1(
                                                                ::std::mem_fun(&OExtendedTypeInfo::getDBName),
                                                                ::o3tl::select2nd<OTypeInfoMap::value_type>())
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
