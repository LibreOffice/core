/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#include <connectivity/dbexception.hxx>
#include <ado/Awrapado.hxx>
#include <ado/adoimp.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbc/DataType.hpp>


using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;


#define MYADOID(l) {l, 0,0x10,{0x80,0,0,0xAA,0,0x6D,0x2E,0xA4}};

const CLSID ADOS::CLSID_ADOCONNECTION_21    =   MYADOID(0x00000514);
const IID   ADOS::IID_ADOCONNECTION_21      =   MYADOID(0x00000550);

const CLSID ADOS::CLSID_ADOCOMMAND_21       =   MYADOID(0x00000507);
const IID   ADOS::IID_ADOCOMMAND_21         =   MYADOID(0x0000054E);

const CLSID ADOS::CLSID_ADORECORDSET_21     =   MYADOID(0x00000535);
const IID   ADOS::IID_ADORECORDSET_21       =   MYADOID(0x0000054F);

const CLSID ADOS::CLSID_ADOCATALOG_25       =   MYADOID(0x00000602);
const IID ADOS::IID_ADOCATALOG_25           =   MYADOID(0x00000603);

const CLSID ADOS::CLSID_ADOINDEX_25         =   MYADOID(0x0000061E);
const IID ADOS::IID_ADOINDEX_25             =   MYADOID(0x0000061F);

const CLSID ADOS::CLSID_ADOTABLE_25         =   MYADOID(0x00000609);
const IID ADOS::IID_ADOTABLE_25             =   MYADOID(0x00000610);

const CLSID ADOS::CLSID_ADOKEY_25           =   MYADOID(0x00000621);
const IID ADOS::IID_ADOKEY_25               =   MYADOID(0x00000622);

const CLSID ADOS::CLSID_ADOCOLUMN_25        =   MYADOID(0x0000061B);
const IID ADOS::IID_ADOCOLUMN_25            =   MYADOID(0x0000061C);

const CLSID ADOS::CLSID_ADOGROUP_25         =   MYADOID(0x00000615);
const IID ADOS::IID_ADOGROUP_25             =   MYADOID(0x00000616);

const CLSID ADOS::CLSID_ADOUSER_25          =   MYADOID(0x00000618);
const IID ADOS::IID_ADOUSER_25              =   MYADOID(0x00000619);

const CLSID ADOS::CLSID_ADOVIEW_25          =   MYADOID(0x00000612);
const IID ADOS::IID_ADOVIEW_25              =   MYADOID(0x00000613);

OLEString& ADOS::GetKeyStr()
{
    static OLEString sKeyStr(u"gxwaezucfyqpwjgqbcmtsncuhwsnyhiohwxz");
    return sKeyStr;
}


sal_Int32 ADOS::MapADOType2Jdbc(DataTypeEnum eType)
{
    sal_Int32 nType = DataType::VARCHAR;
    switch (eType)
    {
        case adUnsignedSmallInt:
        case adSmallInt:            nType = DataType::SMALLINT;     break;
        case adUnsignedInt:
        case adInteger:             nType = DataType::INTEGER;      break;
        case adUnsignedBigInt:
        case adBigInt:              nType = DataType::BIGINT;       break;
        case adSingle:              nType = DataType::FLOAT;        break;
        case adDouble:              nType = DataType::DOUBLE;       break;
        case adCurrency:            nType = DataType::DOUBLE;       break;
        case adVarNumeric:
        case adNumeric:             nType = DataType::NUMERIC;      break;
        case adDecimal:             nType = DataType::DECIMAL;      break;
        case adDBDate:              nType = DataType::DATE;         break;
        case adDBTime:              nType = DataType::TIME;         break;
        case adDate:
        case adDBTimeStamp:         nType = DataType::TIMESTAMP;    break;
        case adBoolean:             nType = DataType::BOOLEAN;      break;
//      case adArray:               nType = DataType::ARRAY;        break;
        case adBinary:              nType = DataType::BINARY;       break;
        case adGUID:                nType = DataType::OBJECT;       break;
        case adBSTR:
        case adVarWChar:
        case adWChar:
        case adVarChar:             nType = DataType::VARCHAR;      break;
        case adLongVarWChar:
        case adLongVarChar:         nType = DataType::LONGVARCHAR;  break;
        case adVarBinary:           nType = DataType::VARBINARY;    break;
        case adLongVarBinary:       nType = DataType::LONGVARBINARY;break;
        case adChar:                nType = DataType::CHAR;         break;
        case adUnsignedTinyInt:
        case adTinyInt:             nType = DataType::TINYINT;      break;
        case adEmpty:               nType = DataType::SQLNULL;      break;
        case adUserDefined:
        case adPropVariant:
        case adFileTime:
        case adChapter:
        case adIDispatch:
        case adIUnknown:
        case adError:
        case adVariant:
                                    nType = DataType::OTHER;        break;
        default:
            OSL_FAIL("MapADOType2Jdbc: Unknown Type!");
            ;
    }
    return nType;
}

DataTypeEnum ADOS::MapJdbc2ADOType(sal_Int32 _nType,sal_Int32 _nJetEngine)
{
    switch (_nType)
    {
        case DataType::SMALLINT:        return adSmallInt;          break;
        case DataType::INTEGER:         return adInteger;           break;
        case DataType::BIGINT:          return adBigInt;            break;
        case DataType::FLOAT:           return adSingle;            break;
        case DataType::DOUBLE:          return adDouble;            break;
        case DataType::NUMERIC:         return adNumeric;           break;
        case DataType::DECIMAL:         return adDecimal;           break;
        case DataType::DATE:            return isJetEngine(_nJetEngine) ? adDate : adDBDate;            break;
        case DataType::TIME:            return adDBTime;            break;
        case DataType::TIMESTAMP:       return isJetEngine(_nJetEngine) ? adDate : adDBTimeStamp;       break;
        case DataType::BOOLEAN:
        case DataType::BIT:             return adBoolean;           break;
        case DataType::BINARY:          return adBinary;            break;
        case DataType::VARCHAR:         return adVarWChar;          break;
        case DataType::CLOB:
        case DataType::LONGVARCHAR:     return adLongVarWChar;      break;
        case DataType::VARBINARY:       return adVarBinary;         break;
        case DataType::BLOB:
        case DataType::LONGVARBINARY:   return adLongVarBinary;     break;
        case DataType::CHAR:            return adWChar;             break;
        case DataType::TINYINT:         return isJetEngine(_nJetEngine) ? adUnsignedTinyInt : adTinyInt;break;
        case DataType::OBJECT:          return adGUID;      break;
    default:
        OSL_FAIL("MapJdbc2ADOType: Unknown Type!");
            ;
    }
    return adEmpty;
}

const int JET_ENGINETYPE_UNKNOWN    = 0;
const int JET_ENGINETYPE_JET10      = 1;
const int JET_ENGINETYPE_JET11      = 2;
const int JET_ENGINETYPE_JET20      = 3;
const int JET_ENGINETYPE_JET3X      = 4;
const int JET_ENGINETYPE_JET4X      = 5;
const int JET_ENGINETYPE_DBASE3     = 10;
const int JET_ENGINETYPE_DBASE4     = 11;
const int JET_ENGINETYPE_DBASE5     = 12;
const int JET_ENGINETYPE_EXCEL30    = 20;
const int JET_ENGINETYPE_EXCEL40    = 21;
const int JET_ENGINETYPE_EXCEL50    = 22;
const int JET_ENGINETYPE_EXCEL80    = 23;
const int JET_ENGINETYPE_EXCEL90    = 24;
const int JET_ENGINETYPE_EXCHANGE4  = 30;
const int JET_ENGINETYPE_LOTUSWK1   = 40;
const int JET_ENGINETYPE_LOTUSWK3   = 41;
const int JET_ENGINETYPE_LOTUSWK4   = 42;
const int JET_ENGINETYPE_PARADOX3X  = 50;
const int JET_ENGINETYPE_PARADOX4X  = 51;
const int JET_ENGINETYPE_PARADOX5X  = 52;
const int JET_ENGINETYPE_PARADOX7X  = 53;
const int JET_ENGINETYPE_TEXT1X     = 60;
const int JET_ENGINETYPE_HTML1X     = 70;

bool ADOS::isJetEngine(sal_Int32 _nEngineType)
{
    bool bRet = false;
    switch(_nEngineType)
    {
        case JET_ENGINETYPE_UNKNOWN:
        case JET_ENGINETYPE_JET10:
        case JET_ENGINETYPE_JET11:
        case JET_ENGINETYPE_JET20:
        case JET_ENGINETYPE_JET3X:
        case JET_ENGINETYPE_JET4X:
        case JET_ENGINETYPE_DBASE3:
        case JET_ENGINETYPE_DBASE4:
        case JET_ENGINETYPE_DBASE5:
        case JET_ENGINETYPE_EXCEL30:
        case JET_ENGINETYPE_EXCEL40:
        case JET_ENGINETYPE_EXCEL50:
        case JET_ENGINETYPE_EXCEL80:
        case JET_ENGINETYPE_EXCEL90:
        case JET_ENGINETYPE_EXCHANGE4:
        case JET_ENGINETYPE_LOTUSWK1:
        case JET_ENGINETYPE_LOTUSWK3:
        case JET_ENGINETYPE_LOTUSWK4:
        case JET_ENGINETYPE_PARADOX3X:
        case JET_ENGINETYPE_PARADOX4X:
        case JET_ENGINETYPE_PARADOX5X:
        case JET_ENGINETYPE_PARADOX7X:
        case JET_ENGINETYPE_TEXT1X:
        case JET_ENGINETYPE_HTML1X:
            bRet = true;
            break;
    }
    return bRet;
}

ObjectTypeEnum ADOS::mapObjectType2Ado(sal_Int32 objType)
{
    ObjectTypeEnum eType = adPermObjTable;
    switch(objType)
    {
        case PrivilegeObject::TABLE:
            eType = adPermObjTable;
            break;
        case PrivilegeObject::VIEW:
            eType = adPermObjView;
            break;
        case PrivilegeObject::COLUMN:
            eType = adPermObjColumn;
            break;
    }
    return eType;
}

sal_Int32 ADOS::mapAdoType2Object(ObjectTypeEnum objType)
{
    sal_Int32 nType = PrivilegeObject::TABLE;
    switch(objType)
    {
        case adPermObjTable:
            nType = PrivilegeObject::TABLE;
            break;
        case adPermObjView:
            nType = PrivilegeObject::VIEW;
            break;
        case adPermObjColumn:
            nType = PrivilegeObject::COLUMN;
            break;
        default:
            OSL_FAIL( "ADOS::mapAdoType2Object: privilege type cannot be translated!" );
            break;
    }
    return nType;
}
#ifdef DELETE
#undef DELETE
#endif

sal_Int32 ADOS::mapAdoRights2Sdbc(RightsEnum eRights)
{
    sal_Int32 nRights = 0;
    if((eRights & adRightInsert) == adRightInsert)
        nRights |= Privilege::INSERT;
    if((eRights & adRightDelete) == adRightDelete)
        nRights |= css::sdbcx::Privilege::DELETE;
    if((eRights & adRightUpdate) == adRightUpdate)
        nRights |= Privilege::UPDATE;
    if((eRights & adRightWriteDesign) == adRightWriteDesign)
        nRights |= Privilege::ALTER;
    if((eRights & adRightRead) == adRightRead)
        nRights |= Privilege::SELECT;
    if((eRights & adRightReference) == adRightReference)
        nRights |= Privilege::REFERENCE;
    if((eRights & adRightDrop) == adRightDrop)
        nRights |= Privilege::DROP;

    return nRights;
}

sal_Int32 ADOS::mapRights2Ado(sal_Int32 nRights)
{
    sal_Int32 eRights = adRightNone;

    if((nRights & Privilege::INSERT) == Privilege::INSERT)
        eRights |= adRightInsert;
    if((nRights & Privilege::DELETE) == Privilege::DELETE)
        eRights |= adRightDelete;
    if((nRights & Privilege::UPDATE) == Privilege::UPDATE)
        eRights |= adRightUpdate;
    if((nRights & Privilege::ALTER) == Privilege::ALTER)
        eRights |= adRightWriteDesign;
    if((nRights & Privilege::SELECT) == Privilege::SELECT)
        eRights |= adRightRead;
    if((nRights & Privilege::REFERENCE) == Privilege::REFERENCE)
        eRights |= adRightReference;
    if((nRights & Privilege::DROP) == Privilege::DROP)
        eRights |= adRightDrop;

    return eRights;
}

WpADOField ADOS::getField(ADORecordset* _pRecordSet,sal_Int32 _nColumnIndex)
{
    if ( !_pRecordSet )
        return WpADOField();

    ADOFields* pFields  = nullptr;
    _pRecordSet->get_Fields(&pFields);
    WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(pFields);
    if(_nColumnIndex <= 0 || _nColumnIndex > aFields.GetItemCount())
        ::dbtools::throwInvalidIndexException(nullptr);
    WpADOField aField(aFields.GetItem(_nColumnIndex-1));
    if(!aField.IsValid())
        ::dbtools::throwInvalidIndexException(nullptr);
    return aField;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
