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
#include "ado/ADatabaseMetaData.hxx"
#include "ado/ADatabaseMetaDataResultSetMetaData.hxx"
#include "ado/Awrapado.hxx"
#include "ado/AGroup.hxx"
#include "ado/adoimp.hxx"
#include "ado/AIndex.hxx"
#include "ado/AKey.hxx"
#include "ado/ATable.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ProcedureResult.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#ifdef DELETE
#undef DELETE
#endif
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>

using namespace connectivity::ado;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::uno;

// -------------------------------------------------------------------------
void ODatabaseMetaData::fillLiterals()
{
    ADORecordset *pRecordset = NULL;
    OLEVariant  vtEmpty;
    vtEmpty.setNoArg();
    m_pADOConnection->OpenSchema(adSchemaDBInfoLiterals,vtEmpty,vtEmpty,&pRecordset);

    ADOS::ThrowException(*m_pADOConnection,*this);

    OSL_ENSURE(pRecordset,"fillLiterals: no resultset!");
    if ( pRecordset )
    {
        WpADORecordset aRecordset(pRecordset);

        aRecordset.MoveFirst();
        OLEVariant  aValue;
        LiteralInfo aInfo;
        while(!aRecordset.IsAtEOF())
        {
            WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(aRecordset.GetFields());
            WpADOField aField(aFields.GetItem(1));
            aInfo.pwszLiteralValue = aField.get_Value();
            aField = aFields.GetItem(5);
            aInfo.fSupported = aField.get_Value();
            aField = aFields.GetItem(6);
            aInfo.cchMaxLen = aField.get_Value().getUInt32();

            aField = aFields.GetItem(4);
            sal_uInt32 nId = aField.get_Value().getUInt32();
            m_aLiteralInfo[nId] = aInfo;

            aRecordset.MoveNext();
        }
        aRecordset.Close();
    }
}
// -------------------------------------------------------------------------
sal_Int32 ODatabaseMetaData::getMaxSize(sal_uInt32 _nId)
{
    if(!m_aLiteralInfo.size())
        fillLiterals();

    sal_Int32 nSize = 0;
    ::std::map<sal_uInt32,LiteralInfo>::const_iterator aIter = m_aLiteralInfo.find(_nId);
    if(aIter != m_aLiteralInfo.end() && (*aIter).second.fSupported)
        nSize = ((*aIter).second.cchMaxLen == (-1)) ? 0 : (*aIter).second.cchMaxLen;
    return nSize;
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaData::isCapable(sal_uInt32 _nId)
{
    if(!m_aLiteralInfo.size())
        fillLiterals();
    sal_Bool bSupported = sal_False;
    ::std::map<sal_uInt32,LiteralInfo>::const_iterator aIter = m_aLiteralInfo.find(_nId);
    if(aIter != m_aLiteralInfo.end())
        bSupported = (*aIter).second.fSupported;
    return bSupported;
}

// -------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaData::getLiteral(sal_uInt32 _nId)
{
    if(!m_aLiteralInfo.size())
        fillLiterals();
    ::rtl::OUString sStr;
    ::std::map<sal_uInt32,LiteralInfo>::const_iterator aIter = m_aLiteralInfo.find(_nId);
    if(aIter != m_aLiteralInfo.end() && (*aIter).second.fSupported)
        sStr = (*aIter).second.pwszLiteralValue;
    return sStr;
}
// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setColumnPrivilegesMap()
{
    m_mColumns[8] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IS_GRANTABLE")),
        ColumnValue::NULLABLE,
        3,3,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setColumnsMap()
{
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TYPE_NAME")),
        ColumnValue::NO_NULLS,
        0,0,0,
        DataType::VARCHAR);
    m_mColumns[11] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NULLABLE")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REMARKS")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
    m_mColumns[13] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("COLUMN_DEF")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
    m_mColumns[14] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQL_DATA_TYPE")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[15] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQL_DATETIME_SUB")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[16] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CHAR_OCTET_LENGTH")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTablesMap()
{
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REMARKS")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setProcedureColumnsMap()
{
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NULLABLE")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setPrimaryKeysMap()
{
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KEY_SEQ")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PK_NAME")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setIndexInfoMap()
{
    m_mColumns[4] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NON_UNIQUE")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::BIT);
    m_mColumns[5] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INDEX_QUALIFIER")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
    m_mColumns[10] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ASC_OR_DESC")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTablePrivilegesMap()
{
    m_mColumns[6] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PRIVILEGE")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IS_GRANTABLE")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setCrossReferenceMap()
{
    m_mColumns[9] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KEY_SEQ")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setTypeInfoMap()
{
    m_mColumns[3] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PRECISION")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NULLABLE")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[12] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTO_INCREMENT")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::BIT);
    m_mColumns[16] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQL_DATA_TYPE")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[17] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SQL_DATETIME_SUB")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
    m_mColumns[18] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NUM_PREC_RADIX")),
        ColumnValue::NO_NULLS,
        1,1,0,
        DataType::INTEGER);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSetMetaData::setProceduresMap()
{
    m_mColumns[7] = OColumn(::rtl::OUString(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REMARKS")),
        ColumnValue::NULLABLE,
        0,0,0,
        DataType::VARCHAR);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isSearchable();
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isAutoIncrement();
    return sal_False;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnServiceName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getTableName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getCatalogName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getColumnTypeName();
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.isCaseSensitive();
    return sal_True;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    if(m_mColumns.size() && (m_mColumnsIter = m_mColumns.find(column)) != m_mColumns.end())
        return (*m_mColumnsIter).second.getSchemaName();
    return ::rtl::OUString();
}
// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------
ObjectTypeEnum OAdoGroup::MapObjectType(sal_Int32 _ObjType)
{
    ObjectTypeEnum eNumType= adPermObjTable;
    switch(_ObjType)
    {
        case PrivilegeObject::TABLE:
            break;
        case PrivilegeObject::VIEW:
            eNumType = adPermObjView;
            break;
        case PrivilegeObject::COLUMN:
            eNumType = adPermObjColumn;
            break;
    }
    return eNumType;
}
// -------------------------------------------------------------------------
sal_Int32 OAdoGroup::MapRight(RightsEnum _eNum)
{
    sal_Int32 nRight = 0;
    if(_eNum & adRightRead)
                nRight |= Privilege::SELECT;
    if(_eNum & adRightInsert)
                nRight |= Privilege::INSERT;
    if(_eNum & adRightUpdate)
                nRight |= Privilege::UPDATE;
    if(_eNum & adRightDelete)
                nRight |= Privilege::DELETE;
    if(_eNum & adRightReadDesign)
                nRight |= Privilege::READ;
    if(_eNum & adRightCreate)
                nRight |= Privilege::CREATE;
    if(_eNum & adRightWriteDesign)
                nRight |= Privilege::ALTER;
    if(_eNum & adRightReference)
                nRight |= Privilege::REFERENCE;
    if(_eNum & adRightDrop)
                nRight |= Privilege::DROP;

    return nRight;
}
// -------------------------------------------------------------------------
RightsEnum OAdoGroup::Map2Right(sal_Int32 _eNum)
{
    sal_Int32 nRight = adRightNone;
        if(_eNum & Privilege::SELECT)
        nRight |= adRightRead;

        if(_eNum & Privilege::INSERT)
        nRight |= adRightInsert;

        if(_eNum & Privilege::UPDATE)
        nRight |= adRightUpdate;

        if(_eNum & Privilege::DELETE)
        nRight |= adRightDelete;

        if(_eNum & Privilege::READ)
        nRight |= adRightReadDesign;

        if(_eNum & Privilege::CREATE)
        nRight |= adRightCreate;

        if(_eNum & Privilege::ALTER)
        nRight |= adRightWriteDesign;

        if(_eNum & Privilege::REFERENCE)
        nRight |= adRightReference;

        if(_eNum & Privilege::DROP)
        nRight |= adRightDrop;

    return (RightsEnum)nRight;
}
// -------------------------------------------------------------------------
void WpADOIndex::Create()
{
    HRESULT         hr = -1;

    _ADOIndex* pIndex = NULL;
    hr = CoCreateInstance(ADOS::CLSID_ADOINDEX_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOINDEX_25,
                          (void**)&pIndex );


    if( !FAILED( hr ) )
    {
        operator=( pIndex );
        pIndex->Release();
    }
}
// -------------------------------------------------------------------------
void OAdoIndex::fillPropertyValues()
{
    if(m_aIndex.IsValid())
    {
        m_Name              = m_aIndex.get_Name();
        m_IsUnique          = m_aIndex.get_Unique();
        m_IsPrimaryKeyIndex = m_aIndex.get_PrimaryKey();
        m_IsClustered       = m_aIndex.get_Clustered();
    }
}
// -----------------------------------------------------------------------------
void WpADOKey::Create()
{
    HRESULT         hr = -1;
    _ADOKey* pKey = NULL;
    hr = CoCreateInstance(ADOS::CLSID_ADOKEY_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOKEY_25,
                          (void**)&pKey );


    if( !FAILED( hr ) )
    {
        operator=( pKey );
        pKey->Release();
    }
}
// -------------------------------------------------------------------------
void OAdoKey::fillPropertyValues()
{
    if(m_aKey.IsValid())
    {
        m_aProps->m_Type            = MapKeyRule(m_aKey.get_Type());
        m_Name                      = m_aKey.get_Name();
        m_aProps->m_ReferencedTable = m_aKey.get_RelatedTable();
        m_aProps->m_UpdateRule      = MapRule(m_aKey.get_UpdateRule());
        m_aProps->m_DeleteRule      = MapRule(m_aKey.get_DeleteRule());
    }
}
// -------------------------------------------------------------------------
sal_Int32 OAdoKey::MapRule(const RuleEnum& _eNum)
{
        sal_Int32 eNum = KeyRule::NO_ACTION;
    switch(_eNum)
    {
        case adRICascade:
            eNum = KeyRule::CASCADE;
            break;
        case adRISetNull:
            eNum = KeyRule::SET_NULL;
            break;
        case adRINone:
            eNum = KeyRule::NO_ACTION;
            break;
        case adRISetDefault:
            eNum = KeyRule::SET_DEFAULT;
            break;
    }
    return eNum;
}
// -------------------------------------------------------------------------
RuleEnum OAdoKey::Map2Rule(const sal_Int32& _eNum)
{
    RuleEnum eNum = adRINone;
    switch(_eNum)
    {
        case KeyRule::CASCADE:
            eNum = adRICascade;
            break;
        case KeyRule::SET_NULL:
            eNum = adRISetNull;
            break;
        case KeyRule::NO_ACTION:
            eNum = adRINone;
            break;
        case KeyRule::SET_DEFAULT:
            eNum = adRISetDefault;
            break;
    }
    return eNum;
}
// -------------------------------------------------------------------------
sal_Int32 OAdoKey::MapKeyRule(const KeyTypeEnum& _eNum)
{
    sal_Int32 nKeyType = KeyType::PRIMARY;
    switch(_eNum)
    {
        case adKeyPrimary:
            nKeyType = KeyType::PRIMARY;
            break;
        case adKeyForeign:
            nKeyType = KeyType::FOREIGN;
            break;
        case adKeyUnique:
            nKeyType = KeyType::UNIQUE;
            break;
    }
    return nKeyType;
}
// -------------------------------------------------------------------------
KeyTypeEnum OAdoKey::Map2KeyRule(const sal_Int32& _eNum)
{
    KeyTypeEnum eNum( adKeyPrimary );
    switch(_eNum)
    {
        case KeyType::PRIMARY:
            eNum = adKeyPrimary;
            break;
        case KeyType::FOREIGN:
            eNum = adKeyForeign;
            break;
        case KeyType::UNIQUE:
            eNum = adKeyUnique;
            break;
        default:
            OSL_ENSURE( false, "OAdoKey::Map2KeyRule: invalid key type!" );
    }
    return eNum;
}
// -----------------------------------------------------------------------------
void WpADOTable::Create()
{
    HRESULT         hr = -1;
    _ADOTable* pTable = NULL;
    hr = CoCreateInstance(ADOS::CLSID_ADOTABLE_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOTABLE_25,
                          (void**)&pTable );


    if( !FAILED( hr ) )
    {
        operator=( pTable );
        pTable->Release();
    }
}
// -------------------------------------------------------------------------
::rtl::OUString WpADOCatalog::GetObjectOwner(const ::rtl::OUString& _rName, ObjectTypeEnum _eNum)
{
    OLEVariant _rVar;
    _rVar.setNoArg();
    OLEString aBSTR;
    OLEString sStr1(_rName);
    pInterface->GetObjectOwner(sStr1,_eNum,_rVar,&aBSTR);
    return aBSTR;
}
// -----------------------------------------------------------------------------
void OAdoTable::fillPropertyValues()
{
    if(m_aTable.IsValid())
    {
        m_Name  = m_aTable.get_Name();
        m_Type  = m_aTable.get_Type();
        {
            WpADOCatalog aCat(m_aTable.get_ParentCatalog());
            if(aCat.IsValid())
                m_CatalogName = aCat.GetObjectOwner(m_aTable.get_Name(),adPermObjTable);
        }
        {
            WpADOProperties aProps = m_aTable.get_Properties();
            if(aProps.IsValid())
                m_Description = OTools::getValue(aProps,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")));
        }
    }
}
// -----------------------------------------------------------------------------
void WpADOUser::Create()
{
    HRESULT         hr = -1;
    _ADOUser* pUser = NULL;
    hr = CoCreateInstance(ADOS::CLSID_ADOUSER_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOUSER_25,
                          (void**)&pUser );


    if( !FAILED( hr ) )
    {
        operator=( pUser );
        pUser->Release();
    }
}
// -------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
