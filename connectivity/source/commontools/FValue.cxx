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


#include <string.h>
#include <connectivity/FValue.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/dbconversion.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbc/XClob.hpp>
#include <com/sun/star/sdbc/XBlob.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

using namespace ::dbtools;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;

namespace connectivity
{

namespace {
    bool isStorageCompatible(sal_Int32 _eType1, sal_Int32 _eType2)
    {
        bool bIsCompatible = true;

        if (_eType1 != _eType2)
        {
            SAL_INFO( "connectivity.commontools", "ORowSetValue::isStorageCompatible _eType1 != _eType2" );
            switch (_eType1)
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                case DataType::LONGVARCHAR:
                    bIsCompatible = (DataType::CHAR         == _eType2)
                                ||  (DataType::VARCHAR      == _eType2)
                                ||  (DataType::DECIMAL      == _eType2)
                                ||  (DataType::NUMERIC      == _eType2)
                                ||  (DataType::LONGVARCHAR  == _eType2);
                    break;

                case DataType::DOUBLE:
                case DataType::REAL:
                    bIsCompatible = (DataType::DOUBLE   == _eType2)
                                ||  (DataType::REAL     == _eType2);
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                    bIsCompatible = (DataType::BINARY           == _eType2)
                                ||  (DataType::VARBINARY        == _eType2)
                                ||  (DataType::LONGVARBINARY    == _eType2);
                    break;

                case DataType::INTEGER:
                    bIsCompatible = (DataType::SMALLINT == _eType2)
                                ||  (DataType::TINYINT  == _eType2)
                                ||  (DataType::BIT      == _eType2)
                                ||  (DataType::BOOLEAN  == _eType2);
                    break;
                case DataType::SMALLINT:
                    bIsCompatible = (DataType::TINYINT  == _eType2)
                                ||  (DataType::BIT      == _eType2)
                                ||  (DataType::BOOLEAN  == _eType2);
                    break;
                case DataType::TINYINT:
                    bIsCompatible = (DataType::BIT      == _eType2)
                                ||  (DataType::BOOLEAN  == _eType2);
                    break;

                case DataType::BLOB:
                case DataType::CLOB:
                case DataType::OBJECT:
                    bIsCompatible = (DataType::BLOB     == _eType2)
                                ||  (DataType::CLOB     == _eType2)
                                ||  (DataType::OBJECT   == _eType2);
                    break;

                default:
                    bIsCompatible = false;
            }
        }
        return bIsCompatible;
    }

    bool isStorageComparable(sal_Int32 _eType1, sal_Int32 _eType2)
    {
        bool bIsComparable = true;

        if (_eType1 != _eType2)
        {
            SAL_INFO( "connectivity.commontools", "ORowSetValue::isStorageCompatible _eType1 != _eType2" );
            switch (_eType1)
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                    bIsComparable = (DataType::CHAR         == _eType2)
                                ||  (DataType::VARCHAR      == _eType2)
                                ||  (DataType::LONGVARCHAR  == _eType2);
                    break;

                case DataType::DECIMAL:
                case DataType::NUMERIC:
                    bIsComparable = (DataType::DECIMAL      == _eType2)
                                ||  (DataType::NUMERIC      == _eType2);
                    break;

            case DataType::DOUBLE:
                case DataType::REAL:
                    bIsComparable = (DataType::DOUBLE   == _eType2)
                                ||  (DataType::REAL     == _eType2);
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                    bIsComparable = (DataType::BINARY           == _eType2)
                                ||  (DataType::VARBINARY        == _eType2)
                                ||  (DataType::LONGVARBINARY    == _eType2);
                    break;

                case DataType::INTEGER:
                    bIsComparable = (DataType::SMALLINT == _eType2)
                                ||  (DataType::TINYINT  == _eType2)
                                ||  (DataType::BIT      == _eType2)
                                ||  (DataType::BOOLEAN  == _eType2);
                    break;
                case DataType::SMALLINT:
                    bIsComparable = (DataType::TINYINT  == _eType2)
                                ||  (DataType::BIT      == _eType2)
                                ||  (DataType::BOOLEAN  == _eType2);
                    break;
                case DataType::TINYINT:
                    bIsComparable = (DataType::BIT      == _eType2)
                                ||  (DataType::BOOLEAN  == _eType2);
                    break;

                case DataType::BLOB:
                case DataType::CLOB:
                case DataType::OBJECT:
                    bIsComparable = (DataType::BLOB     == _eType2)
                                ||  (DataType::CLOB     == _eType2)
                                ||  (DataType::OBJECT   == _eType2);
                    break;

                default:
                    bIsComparable = false;
            }
        }
        return bIsComparable;
    }
}

void ORowSetValue::setTypeKind(sal_Int32 _eType)
{
    if ( !m_bNull && !isStorageCompatible(_eType, m_eTypeKind) )
    {
        switch(_eType)
        {
            case DataType::VARCHAR:
            case DataType::CHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                (*this) = getString();
                break;
            case DataType::BIGINT:
            {
                sal_Int64 nVal(getLong());
                sal_uInt64 nuVal(getULong());
                if (nVal == 0 && nuVal != 0)
                    (*this) = nuVal;
                else
                    (*this) = nVal;
                break;
            }

            case DataType::FLOAT:
                (*this) = getFloat();
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                (*this) = getDouble();
                break;
            case DataType::TINYINT:
                (*this) = getInt8();
                break;
            case DataType::SMALLINT:
                (*this) = getInt16();
                break;
            case DataType::INTEGER:
            {
                sal_Int32 nVal(getInt32());
                sal_uInt32 nuVal(getUInt32());
                if (nVal == 0 && nuVal != 0)
                    (*this) = nuVal;
                else
                    (*this) = nVal;
                break;
            }
            case DataType::BIT:
            case DataType::BOOLEAN:
                (*this) = getBool();
                break;
            case DataType::DATE:
                (*this) = getDate();
                break;
            case DataType::TIME:
                (*this) = getTime();
                break;
            case DataType::TIMESTAMP:
                (*this) = getDateTime();
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                (*this) = getSequence();
                break;
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::OBJECT:
            case DataType::OTHER:
                (*this) = makeAny();
                break;
            default:
                (*this) = makeAny();
                SAL_WARN( "connectivity.commontools","ORowSetValue::setTypeKind(): UNSUPPORTED TYPE!");
        }
    }

    m_eTypeKind = _eType;
}


void ORowSetValue::free()
{
    if(!m_bNull)
    {
        switch(m_eTypeKind)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                OSL_ENSURE(m_aValue.m_pString,"String pointer is null!");
                rtl_uString_release(m_aValue.m_pString);
                m_aValue.m_pString = nullptr;
                break;
            case DataType::DATE:
                delete static_cast<css::util::Date*>(m_aValue.m_pValue);
                m_aValue.m_pValue = nullptr;
                break;
            case DataType::TIME:
                delete static_cast<css::util::Time*>(m_aValue.m_pValue);
                m_aValue.m_pValue = nullptr;
                break;
            case DataType::TIMESTAMP:
                delete static_cast<css::util::DateTime*>(m_aValue.m_pValue);
                m_aValue.m_pValue = nullptr;
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                delete static_cast<Sequence<sal_Int8>*>(m_aValue.m_pValue);
                m_aValue.m_pValue = nullptr;
                break;
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::OBJECT:
                delete static_cast<Any*>(m_aValue.m_pValue);
                m_aValue.m_pValue = nullptr;
                break;
            case DataType::BIT:
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::BIGINT:
            case DataType::BOOLEAN:
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::REAL:
                break;
            default:
                if ( m_aValue.m_pValue )
                {
                    delete static_cast<Any*>(m_aValue.m_pValue);
                    m_aValue.m_pValue = nullptr;
                }
                break;

        }
        m_bNull = true;
    }
}

ORowSetValue& ORowSetValue::operator=(const ORowSetValue& _rRH)
{
    if(&_rRH == this)
        return *this;

    if ( m_eTypeKind != _rRH.m_eTypeKind || (_rRH.m_bNull && !m_bNull) || m_bSigned != _rRH.m_bSigned)
        free();

    m_bBound    = _rRH.m_bBound;
    m_eTypeKind = _rRH.m_eTypeKind;
    m_bSigned   = _rRH.m_bSigned;

    if(m_bNull && !_rRH.m_bNull)
    {
        switch(_rRH.m_eTypeKind)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                rtl_uString_acquire(_rRH.m_aValue.m_pString);
                m_aValue.m_pString = _rRH.m_aValue.m_pString;
                break;
            case DataType::DATE:
                m_aValue.m_pValue   = new Date(*static_cast<Date*>(_rRH.m_aValue.m_pValue));
                break;
            case DataType::TIME:
                m_aValue.m_pValue   = new Time(*static_cast<Time*>(_rRH.m_aValue.m_pValue));
                break;
            case DataType::TIMESTAMP:
                m_aValue.m_pValue   = new DateTime(*static_cast<DateTime*>(_rRH.m_aValue.m_pValue));
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                m_aValue.m_pValue   = new Sequence<sal_Int8>(*static_cast<Sequence<sal_Int8>*>(_rRH.m_aValue.m_pValue));
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                m_aValue.m_bBool    = _rRH.m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt8    = _rRH.m_aValue.m_nInt8;
                else
                    m_aValue.m_uInt8    = _rRH.m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt16   = _rRH.m_aValue.m_nInt16;
                else
                    m_aValue.m_uInt16   = _rRH.m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt32   = _rRH.m_aValue.m_nInt32;
                else
                    m_aValue.m_uInt32   = _rRH.m_aValue.m_uInt32;
                break;
            case DataType::BIGINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt64   = _rRH.m_aValue.m_nInt64;
                else
                    m_aValue.m_uInt64   = _rRH.m_aValue.m_uInt64;
                break;
            case DataType::FLOAT:
                m_aValue.m_nFloat  = _rRH.m_aValue.m_nFloat;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                m_aValue.m_nDouble  = _rRH.m_aValue.m_nDouble;
                break;
            default:
                m_aValue.m_pValue   = new Any(*static_cast<Any*>(_rRH.m_aValue.m_pValue));
        }
    }
    else if(!_rRH.m_bNull)
    {
        switch(_rRH.m_eTypeKind)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                (*this) = OUString(_rRH.m_aValue.m_pString);
                break;
            case DataType::DATE:
                (*this) = *static_cast<Date*>(_rRH.m_aValue.m_pValue);
                break;
            case DataType::TIME:
                (*this) = *static_cast<Time*>(_rRH.m_aValue.m_pValue);
                break;
            case DataType::TIMESTAMP:
                (*this) = *static_cast<DateTime*>(_rRH.m_aValue.m_pValue);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                (*this) = *static_cast<Sequence<sal_Int8>*>(_rRH.m_aValue.m_pValue);
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                m_aValue.m_bBool    = _rRH.m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt8    = _rRH.m_aValue.m_nInt8;
                else
                    m_aValue.m_uInt8    = _rRH.m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt16   = _rRH.m_aValue.m_nInt16;
                else
                    m_aValue.m_uInt16   = _rRH.m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt32   = _rRH.m_aValue.m_nInt32;
                else
                    m_aValue.m_uInt32   = _rRH.m_aValue.m_uInt32;
                break;
            case DataType::BIGINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt64   = _rRH.m_aValue.m_nInt64;
                else
                    m_aValue.m_uInt64   = _rRH.m_aValue.m_uInt64;
                break;
            case DataType::FLOAT:
                m_aValue.m_nFloat  = _rRH.m_aValue.m_nFloat;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                m_aValue.m_nDouble  = _rRH.m_aValue.m_nDouble;
                break;
            default:
                *static_cast<Any*>(m_aValue.m_pValue) = *static_cast<Any*>(_rRH.m_aValue.m_pValue);
        }
    }

    m_bNull     = _rRH.m_bNull;
    // OJ: BUGID: 96277
    m_eTypeKind = _rRH.m_eTypeKind;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(ORowSetValue&& _rRH)
{
    if ( m_eTypeKind != _rRH.m_eTypeKind || !m_bNull)
        free();
    if(!_rRH.m_bNull)
    {
        m_aValue    = _rRH.m_aValue;
        memset(&_rRH.m_aValue, 0, sizeof(_rRH.m_aValue));
    }
    m_bBound    = _rRH.m_bBound;
    m_eTypeKind = _rRH.m_eTypeKind;
    m_bSigned   = _rRH.m_bSigned;
    m_bNull     = _rRH.m_bNull;
    _rRH.m_bNull = true;
    return *this;
}


ORowSetValue& ORowSetValue::operator=(const Date& _rRH)
{
    if(m_eTypeKind != DataType::DATE)
        free();

    if(m_bNull)
    {
        m_aValue.m_pValue = new Date(_rRH);
        m_eTypeKind = DataType::DATE;
        m_bNull = false;
    }
    else
        *static_cast<Date*>(m_aValue.m_pValue) = _rRH;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(const css::util::Time& _rRH)
{
    if(m_eTypeKind != DataType::TIME)
        free();

    if(m_bNull)
    {
        m_aValue.m_pValue = new Time(_rRH);
        m_eTypeKind = DataType::TIME;
        m_bNull = false;
    }
    else
        *static_cast<Time*>(m_aValue.m_pValue) = _rRH;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(const DateTime& _rRH)
{
    if(m_eTypeKind != DataType::TIMESTAMP)
        free();
    if(m_bNull)
    {
        m_aValue.m_pValue = new DateTime(_rRH);
        m_eTypeKind = DataType::TIMESTAMP;
        m_bNull = false;
    }
    else
        *static_cast<DateTime*>(m_aValue.m_pValue) = _rRH;

    return *this;
}


ORowSetValue& ORowSetValue::operator=(const OUString& _rRH)
{
    if(m_eTypeKind != DataType::VARCHAR || m_aValue.m_pString != _rRH.pData)
    {
        free();
        m_bNull = false;

        m_aValue.m_pString = _rRH.pData;
        rtl_uString_acquire(m_aValue.m_pString);
        m_eTypeKind = DataType::VARCHAR;
    }

    return *this;
}


ORowSetValue& ORowSetValue::operator=(double _rRH)
{
    if(m_eTypeKind != DataType::DOUBLE)
        free();

    m_aValue.m_nDouble = _rRH;
    m_eTypeKind = DataType::DOUBLE;
    m_bNull = false;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(float _rRH)
{
    if(m_eTypeKind != DataType::FLOAT)
        free();

    m_aValue.m_nFloat = _rRH;
    m_eTypeKind = DataType::FLOAT;
    m_bNull = false;

    return *this;
}


ORowSetValue& ORowSetValue::operator=(sal_Int8 _rRH)
{
    if(m_eTypeKind != DataType::TINYINT )
        free();

    m_aValue.m_nInt8 = _rRH;
    m_eTypeKind = DataType::TINYINT;
    m_bNull = false;
    m_bSigned = true;
    return *this;
}

ORowSetValue& ORowSetValue::operator=(sal_Int16 _rRH)
{
    if(m_eTypeKind != DataType::SMALLINT )
        free();

    m_aValue.m_nInt16 = _rRH;
    m_eTypeKind = DataType::SMALLINT;
    m_bNull = false;
    m_bSigned = true;

    return *this;
}


ORowSetValue& ORowSetValue::operator=(sal_uInt16 _rRH)
{
    if(m_eTypeKind != DataType::SMALLINT )
        free();

    m_aValue.m_uInt16 = _rRH;
    m_eTypeKind = DataType::SMALLINT;
    m_bNull = false;
    m_bSigned = false;

    return *this;
}


ORowSetValue& ORowSetValue::operator=(sal_Int32 _rRH)
{
    if(m_eTypeKind != DataType::INTEGER )
        free();

    m_aValue.m_nInt32 = _rRH;

    m_eTypeKind = DataType::INTEGER;
    m_bNull = false;
    m_bSigned = true;

    return *this;
}


ORowSetValue& ORowSetValue::operator=(sal_uInt32 _rRH)
{
    if(m_eTypeKind != DataType::INTEGER )
        free();

    m_aValue.m_uInt32 = _rRH;

    m_eTypeKind = DataType::INTEGER;
    m_bNull = false;
    m_bSigned = false;

    return *this;
}


ORowSetValue& ORowSetValue::operator=(const bool _rRH)
{
    if(m_eTypeKind != DataType::BIT && DataType::BOOLEAN != m_eTypeKind )
        free();

    m_aValue.m_bBool = _rRH;
    m_eTypeKind = DataType::BOOLEAN;
    m_bNull = false;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(sal_Int64 _rRH)
{
    if ( DataType::BIGINT != m_eTypeKind)
        free();

    m_aValue.m_nInt64 = _rRH;
    m_eTypeKind = DataType::BIGINT;
    m_bNull = false;
    m_bSigned = true;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(sal_uInt64 _rRH)
{
    if ( DataType::BIGINT != m_eTypeKind)
        free();

    m_aValue.m_uInt64 = _rRH;
    m_eTypeKind = DataType::BIGINT;
    m_bNull = false;
    m_bSigned = false;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(const Sequence<sal_Int8>& _rRH)
{
    if (!isStorageCompatible(DataType::LONGVARBINARY,m_eTypeKind))
        free();

    if (m_bNull)
    {
        m_aValue.m_pValue = new Sequence<sal_Int8>(_rRH);
    }
    else
        *static_cast< Sequence< sal_Int8 >* >(m_aValue.m_pValue) = _rRH;

    m_eTypeKind = DataType::LONGVARBINARY;
    m_bNull = false;

    return *this;
}

ORowSetValue& ORowSetValue::operator=(const Any& _rAny)
{
    if (!isStorageCompatible(DataType::OBJECT,m_eTypeKind))
        free();

    if ( m_bNull )
    {
        m_aValue.m_pValue = new Any(_rAny);
    }
    else
        *static_cast<Any*>(m_aValue.m_pValue) = _rAny;

    m_eTypeKind = DataType::OBJECT;
    m_bNull = false;

    return *this;
}


bool ORowSetValue::operator==(const ORowSetValue& _rRH) const
{
    if ( m_bNull != _rRH.isNull() )
        return false;

    if(m_bNull && _rRH.isNull())
        return true;

    if ( !isStorageComparable(m_eTypeKind, _rRH.m_eTypeKind ))
    {
        switch(m_eTypeKind)
        {
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::REAL:
                return getDouble() == _rRH.getDouble();
            default:
                switch(_rRH.m_eTypeKind)
                {
                    case DataType::FLOAT:
                    case DataType::DOUBLE:
                    case DataType::REAL:
                            return getDouble() == _rRH.getDouble();
                    default:
                            break;
                }
                break;
        }
        return false;
    }

    bool bRet = false;
    OSL_ENSURE(!m_bNull,"Should not be null!");
    switch(m_eTypeKind)
    {
        case DataType::VARCHAR:
        case DataType::CHAR:
        case DataType::LONGVARCHAR:
        {
            OUString aVal1(m_aValue.m_pString);
            OUString aVal2(_rRH.m_aValue.m_pString);
            return aVal1 == aVal2;
        }
        default:
            if ( m_bSigned != _rRH.m_bSigned )
                return false;
            break;
    }

    switch(m_eTypeKind)
    {
        case DataType::DECIMAL:
        case DataType::NUMERIC:
            {
                OUString aVal1(m_aValue.m_pString);
                OUString aVal2(_rRH.m_aValue.m_pString);
                bRet = aVal1 == aVal2;
            }
            break;
        case DataType::FLOAT:
            bRet = m_aValue.m_nFloat == _rRH.m_aValue.m_nFloat;
            break;
        case DataType::DOUBLE:
        case DataType::REAL:
            bRet = m_aValue.m_nDouble == _rRH.m_aValue.m_nDouble;
            break;
        case DataType::TINYINT:
            bRet = m_bSigned ? ( m_aValue.m_nInt8 == _rRH.m_aValue.m_nInt8 ) : (m_aValue.m_uInt8 == _rRH.m_aValue.m_uInt8);
            break;
        case DataType::SMALLINT:
            bRet = m_bSigned ? ( m_aValue.m_nInt16 == _rRH.m_aValue.m_nInt16 ) : (m_aValue.m_uInt16 == _rRH.m_aValue.m_uInt16);
            break;
        case DataType::INTEGER:
            bRet = m_bSigned ? ( m_aValue.m_nInt32 == _rRH.m_aValue.m_nInt32 ) : (m_aValue.m_uInt32 == _rRH.m_aValue.m_uInt32);
            break;
        case DataType::BIGINT:
            bRet = m_bSigned ? ( m_aValue.m_nInt64 == _rRH.m_aValue.m_nInt64 ) : (m_aValue.m_uInt64 == _rRH.m_aValue.m_uInt64);
            break;
        case DataType::BIT:
        case DataType::BOOLEAN:
            bRet = m_aValue.m_bBool == _rRH.m_aValue.m_bBool;
            break;
        case DataType::DATE:
            bRet = *static_cast<Date*>(m_aValue.m_pValue) == *static_cast<Date*>(_rRH.m_aValue.m_pValue);
            break;
        case DataType::TIME:
            bRet = *static_cast<Time*>(m_aValue.m_pValue) == *static_cast<Time*>(_rRH.m_aValue.m_pValue);
            break;
        case DataType::TIMESTAMP:
            bRet = *static_cast<DateTime*>(m_aValue.m_pValue) == *static_cast<DateTime*>(_rRH.m_aValue.m_pValue);
            break;
        case DataType::BINARY:
        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
            bRet = false;
            break;
        case DataType::BLOB:
        case DataType::CLOB:
        case DataType::OBJECT:
        case DataType::OTHER:
            bRet = false;
            break;
        default:
            bRet = false;
            SAL_WARN( "connectivity.commontools","ORowSetValue::operator==(): UNSUPPORTED TYPE!");
            break;
    }
    return bRet;
}

Any ORowSetValue::makeAny() const
{
    Any rValue;
    if(isBound() && !isNull())
    {
        switch(getTypeKind())
        {
            case DataType::SQLNULL:
                assert(rValue == Any());
                break;
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                OSL_ENSURE(m_aValue.m_pString,"Value is null!");
                rValue <<= OUString(m_aValue.m_pString);
                break;
            case DataType::FLOAT:
                rValue <<= m_aValue.m_nFloat;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                rValue <<= m_aValue.m_nDouble;
                break;
            case DataType::DATE:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *static_cast<Date*>(m_aValue.m_pValue);
                break;
            case DataType::TIME:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *static_cast<Time*>(m_aValue.m_pValue);
                break;
            case DataType::TIMESTAMP:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *static_cast<DateTime*>(m_aValue.m_pValue);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *static_cast<Sequence<sal_Int8>*>(m_aValue.m_pValue);
                break;
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::OBJECT:
            case DataType::OTHER:
                rValue = getAny();
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                rValue <<= m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    // TypeClass_BYTE
                    rValue <<= m_aValue.m_nInt8;
                else
                    // There is no TypeClass_UNSIGNED_BYTE,
                    // so silently promote it to a 16-bit integer,
                    // that is TypeClass_UNSIGNED_SHORT
                    rValue <<= static_cast<sal_uInt16>(m_aValue.m_uInt8);
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    // TypeClass_SHORT
                    rValue <<= m_aValue.m_nInt16;
                else
                    // TypeClass_UNSIGNED_SHORT
                    rValue <<= m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    // TypeClass_LONG
                    rValue <<= m_aValue.m_nInt32;
                else
                    // TypeClass_UNSIGNED_LONG
                    rValue <<= m_aValue.m_uInt32;
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    // TypeClass_HYPER
                    rValue <<= m_aValue.m_nInt64;
                else
                    // TypeClass_UNSIGNED_HYPER
                    rValue <<= m_aValue.m_uInt64;
                break;
            default:
                SAL_WARN( "connectivity.commontools","ORowSetValue::makeAny(): UNSUPPORTED TYPE!");
                rValue = getAny();
                break;
        }
    }
    return rValue;
}

OUString ORowSetValue::getString( ) const
{
    OUString aRet;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                aRet = m_aValue.m_pString;
                break;
            case DataType::FLOAT:
                aRet = OUString::number(static_cast<float>(*this));
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                aRet = OUString::number(static_cast<double>(*this));
                break;
            case DataType::DATE:
                aRet = DBTypeConversion::toDateString(*this);
                break;
            case DataType::TIME:
                aRet = DBTypeConversion::toTimeString(*this);
                break;
            case DataType::TIMESTAMP:
                aRet = DBTypeConversion::toDateTimeString(*this);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                {
                    OUStringBuffer sVal("0x");
                    Sequence<sal_Int8> aSeq(getSequence());
                    const sal_Int8* pBegin  = aSeq.getConstArray();
                    const sal_Int8* pEnd    = pBegin + aSeq.getLength();
                    for(;pBegin != pEnd;++pBegin)
                        sVal.append(static_cast<sal_Int32>(*pBegin),16);
                    aRet = sVal.makeStringAndClear();
                }
                break;
            case DataType::BIT:
                aRet = OUString::number(int(static_cast<bool>(*this)));
                break;
            case DataType::BOOLEAN:
                aRet = OUString::boolean(static_cast<bool>(*this));
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
                if ( m_bSigned )
                    aRet = OUString::number(static_cast<sal_Int32>(*this));
                else
                    aRet = OUString::number(static_cast<sal_uInt32>(*this));
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    aRet = OUString::number(static_cast<sal_Int64>(*this));
                else
                    aRet = OUString::number(static_cast<sal_uInt64>(*this));
                break;
            case DataType::CLOB:
                {
                    Any aValue( getAny() );
                    Reference< XClob > xClob;
                    if ( (aValue >>= xClob) && xClob.is() )
                    {
                        aRet = xClob->getSubString(1,static_cast<sal_Int32>(xClob->length()) );
                    }
                }
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= aRet;
                    break;
                }
        }
    }
    return aRet;
}

bool ORowSetValue::getBool()    const
{
    bool bRet = false;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                {
                    const OUString sValue(m_aValue.m_pString);
                    if ( sValue.equalsIgnoreAsciiCase("true") || (sValue == "1") )
                    {
                        bRet = true;
                        break;
                    }
                    else if ( sValue.equalsIgnoreAsciiCase("false") || (sValue == "0") )
                    {
                        bRet = false;
                        break;
                    }
                }
                [[fallthrough]];
            case DataType::DECIMAL:
            case DataType::NUMERIC:

                bRet = OUString(m_aValue.m_pString).toInt32() != 0;
                break;
            case DataType::FLOAT:
                bRet = m_aValue.m_nFloat != 0.0;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                bRet = m_aValue.m_nDouble != 0.0;
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                OSL_FAIL("getBool() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                bRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                bRet = m_bSigned ? (m_aValue.m_nInt8  != 0) : (m_aValue.m_uInt8 != 0);
                break;
            case DataType::SMALLINT:
                bRet = m_bSigned ? (m_aValue.m_nInt16  != 0) : (m_aValue.m_uInt16 != 0);
                break;
            case DataType::INTEGER:
                bRet = m_bSigned ? (m_aValue.m_nInt32 != 0) : (m_aValue.m_uInt32 != 0);
                break;
            case DataType::BIGINT:
                bRet = m_bSigned ? (m_aValue.m_nInt64 != 0) : (m_aValue.m_uInt64 != 0);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= bRet;
                    break;
                }
        }
    }
    return bRet;
}


sal_Int8 ORowSetValue::getInt8()    const
{
    sal_Int8 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = sal_Int8(OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::FLOAT:
                nRet = sal_Int8(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int8(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getInt8() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = sal_Int8(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = static_cast<sal_Int8>(m_aValue.m_uInt8);
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int8>(m_aValue.m_nInt16);
                else
                    nRet = static_cast<sal_Int8>(m_aValue.m_uInt16);
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int8>(m_aValue.m_nInt32);
                else
                    nRet = static_cast<sal_Int8>(m_aValue.m_uInt32);
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int8>(m_aValue.m_nInt64);
                else
                    nRet = static_cast<sal_Int8>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}


sal_uInt8 ORowSetValue::getUInt8()    const
{
    sal_uInt8 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = sal_uInt8(OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::FLOAT:
                nRet = sal_uInt8(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_uInt8(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getuInt8() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = int(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_uInt8>(m_aValue.m_nInt16);
                else
                    nRet = static_cast<sal_uInt8>(m_aValue.m_uInt16);
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = static_cast<sal_uInt8>(m_aValue.m_nInt32);
                else
                    nRet = static_cast<sal_uInt8>(m_aValue.m_uInt32);
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_uInt8>(m_aValue.m_nInt64);
                else
                    nRet = static_cast<sal_uInt8>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    // Cf. "There is no TypeClass_UNSIGNED_BYTE" in makeAny:
                    sal_uInt16 n;
                    if (aValue >>= n) {
                        nRet = static_cast<sal_uInt8>(n);
                    }
                    break;
                }
        }
    }
    return nRet;
}


sal_Int16 ORowSetValue::getInt16()  const
{
    sal_Int16 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = sal_Int16(OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::FLOAT:
                nRet = sal_Int16(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int16(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getInt16() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = sal_Int16(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = static_cast<sal_Int16>(m_aValue.m_uInt16);
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int16>(m_aValue.m_nInt32);
                else
                    nRet = static_cast<sal_Int16>(m_aValue.m_uInt32);
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int16>(m_aValue.m_nInt64);
                else
                    nRet = static_cast<sal_Int16>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}


sal_uInt16 ORowSetValue::getUInt16()  const
{
    sal_uInt16 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = sal_uInt16(OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::FLOAT:
                nRet = sal_uInt16(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_uInt16(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getuInt16() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = sal_uInt16(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = static_cast<sal_uInt16>(m_aValue.m_nInt32);
                else
                    nRet = static_cast<sal_uInt16>(m_aValue.m_uInt32);
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_uInt16>(m_aValue.m_nInt64);
                else
                    nRet = static_cast<sal_uInt16>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}


sal_Int32 ORowSetValue::getInt32()  const
{
    sal_Int32 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = OUString(m_aValue.m_pString).toInt32();
                break;
            case DataType::FLOAT:
                nRet = sal_Int32(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int32(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDays(*static_cast<css::util::Date*>(m_aValue.m_pValue));
                break;
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getInt32() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = sal_Int32(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = static_cast<sal_Int32>(m_aValue.m_uInt32);
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int32>(m_aValue.m_nInt64);
                else
                    nRet = static_cast<sal_Int32>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}


sal_uInt32 ORowSetValue::getUInt32()  const
{
    sal_uInt32 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = OUString(m_aValue.m_pString).toUInt32();
                break;
            case DataType::FLOAT:
                nRet = sal_uInt32(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_uInt32(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDays(*static_cast<css::util::Date*>(m_aValue.m_pValue));
                break;
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getuInt32() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = sal_uInt32(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = m_aValue.m_uInt32;
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_uInt32>(m_aValue.m_nInt64);
                else
                    nRet = static_cast<sal_uInt32>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}


sal_Int64 ORowSetValue::getLong()   const
{
    sal_Int64 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = OUString(m_aValue.m_pString).toInt64();
                break;
            case DataType::FLOAT:
                nRet = sal_Int64(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int64(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDays(*static_cast<css::util::Date*>(m_aValue.m_pValue));
                break;
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getLong() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = sal_Int64(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = m_aValue.m_uInt32;
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt64;
                else
                    nRet = static_cast<sal_Int64>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}


sal_uInt64 ORowSetValue::getULong()   const
{
    sal_uInt64 nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = OUString(m_aValue.m_pString).toUInt64();
                break;
            case DataType::FLOAT:
                nRet = sal_uInt64(m_aValue.m_nFloat);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_uInt64(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDays(*static_cast<css::util::Date*>(m_aValue.m_pValue));
                break;
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getULong() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = sal_uInt64(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = m_aValue.m_uInt32;
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt64;
                else
                    nRet = m_aValue.m_uInt64;
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}


float ORowSetValue::getFloat()  const
{
    float nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = OUString(m_aValue.m_pString).toFloat();
                break;
            case DataType::FLOAT:
                nRet = m_aValue.m_nFloat;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = static_cast<float>(m_aValue.m_nDouble);
                break;
            case DataType::DATE:
                nRet = static_cast<float>(dbtools::DBTypeConversion::toDouble(*static_cast<css::util::Date*>(m_aValue.m_pValue)));
                break;
            case DataType::TIME:
                nRet = static_cast<float>(dbtools::DBTypeConversion::toDouble(*static_cast<css::util::Time*>(m_aValue.m_pValue)));
                break;
            case DataType::TIMESTAMP:
                nRet = static_cast<float>(dbtools::DBTypeConversion::toDouble(*static_cast<css::util::DateTime*>(m_aValue.m_pValue)));
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getDouble() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = float(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = static_cast<float>(m_aValue.m_uInt16);
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = static_cast<float>(m_aValue.m_nInt32);
                else
                    nRet = static_cast<float>(m_aValue.m_uInt32);
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = static_cast<float>(m_aValue.m_nInt64);
                else
                    nRet = static_cast<float>(m_aValue.m_uInt64);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}

double ORowSetValue::getDouble()    const
{
    double nRet = 0;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                nRet = OUString(m_aValue.m_pString).toDouble();
                break;
            case DataType::FLOAT:
                nRet = m_aValue.m_nFloat;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = m_aValue.m_nDouble;
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDouble(*static_cast<css::util::Date*>(m_aValue.m_pValue));
                break;
            case DataType::TIME:
                nRet = dbtools::DBTypeConversion::toDouble(*static_cast<css::util::Time*>(m_aValue.m_pValue));
                break;
            case DataType::TIMESTAMP:
                nRet = dbtools::DBTypeConversion::toDouble(*static_cast<css::util::DateTime*>(m_aValue.m_pValue));
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_FAIL("getDouble() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = double(m_aValue.m_bBool);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_uInt8;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_uInt16;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = m_aValue.m_uInt32;
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt64;
                else
                    nRet = m_aValue.m_uInt64;
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}

Sequence<sal_Int8>  ORowSetValue::getSequence() const
{
    Sequence<sal_Int8> aSeq;
    if (!m_bNull)
    {
        switch(m_eTypeKind)
        {
            case DataType::OBJECT:
            case DataType::CLOB:
            case DataType::BLOB:
            {
                Reference<XInputStream> xStream;
                const Any aValue = makeAny();
                if(aValue.hasValue())
                {
                    Reference<XBlob> xBlob(aValue,UNO_QUERY);
                    if ( xBlob.is() )
                        xStream = xBlob->getBinaryStream();
                    else
                    {
                        Reference<XClob> xClob(aValue,UNO_QUERY);
                        if ( xClob.is() )
                            xStream = xClob->getCharacterStream();
                    }
                    if(xStream.is())
                    {
                        const sal_uInt32    nBytesToRead = 65535;
                        sal_uInt32          nRead;

                        do
                        {
                            css::uno::Sequence< sal_Int8 > aReadSeq;

                            nRead = xStream->readSomeBytes( aReadSeq, nBytesToRead );

                            if( nRead )
                            {
                                const sal_uInt32 nOldLength = aSeq.getLength();
                                aSeq.realloc( nOldLength + nRead );
                                memcpy( aSeq.getArray() + nOldLength, aReadSeq.getConstArray(), aReadSeq.getLength() );
                            }
                        }
                        while( nBytesToRead == nRead );
                        xStream->closeInput();
                    }
                }
            }
            break;
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                {
                    aSeq = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(m_aValue.m_pString->buffer),
                                              sizeof(sal_Unicode) * m_aValue.m_pString->length);
                }
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                aSeq = *static_cast< Sequence<sal_Int8>*>(m_aValue.m_pValue);
                break;
            default:
                {
                    Any aValue = makeAny();
                    aValue >>= aSeq;
                    break;
                }
        }
    }
    return aSeq;

}

css::util::Date ORowSetValue::getDate() const
{
    css::util::Date aValue;
    if(!m_bNull)
    {
        switch(m_eTypeKind)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                aValue = DBTypeConversion::toDate(getString());
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::REAL:
                aValue = DBTypeConversion::toDate(static_cast<double>(*this));
                break;

            case DataType::DATE:
                aValue = *static_cast< css::util::Date*>(m_aValue.m_pValue);
                break;
            case DataType::TIMESTAMP:
                {
                    css::util::DateTime* pDateTime = static_cast< css::util::DateTime*>(m_aValue.m_pValue);
                    aValue.Day      = pDateTime->Day;
                    aValue.Month    = pDateTime->Month;
                    aValue.Year     = pDateTime->Year;
                }
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::BIGINT:
                aValue = DBTypeConversion::toDate( double( sal_Int64( *this ) ) );
                break;

            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::OBJECT:
            default:
                OSL_ENSURE( false, "ORowSetValue::getDate: cannot retrieve the data!" );
                [[fallthrough]];

            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::TIME:
                aValue = DBTypeConversion::toDate( double(0) );
                break;
        }
    }
    return aValue;
}

css::util::Time ORowSetValue::getTime()        const
{
    css::util::Time aValue;
    if(!m_bNull)
    {
        switch(m_eTypeKind)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                aValue = DBTypeConversion::toTime(getString());
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
                aValue = DBTypeConversion::toTime(static_cast<double>(*this));
                break;
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::REAL:
                aValue = DBTypeConversion::toTime(static_cast<double>(*this));
                break;
            case DataType::TIMESTAMP:
                {
                    css::util::DateTime* pDateTime = static_cast< css::util::DateTime*>(m_aValue.m_pValue);
                    aValue.NanoSeconds      = pDateTime->NanoSeconds;
                    aValue.Seconds          = pDateTime->Seconds;
                    aValue.Minutes          = pDateTime->Minutes;
                    aValue.Hours            = pDateTime->Hours;
                }
                break;
            case DataType::TIME:
                aValue = *static_cast< css::util::Time*>(m_aValue.m_pValue);
                break;
            default:
                {
                    Any aAnyValue = makeAny();
                    aAnyValue >>= aValue;
                    break;
                }
        }
    }
    return aValue;
}

css::util::DateTime ORowSetValue::getDateTime()    const
{
    css::util::DateTime aValue;
    if(!m_bNull)
    {
        switch(m_eTypeKind)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                aValue = DBTypeConversion::toDateTime(getString());
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
                aValue = DBTypeConversion::toDateTime(static_cast<double>(*this));
                break;
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::REAL:
                aValue = DBTypeConversion::toDateTime(static_cast<double>(*this));
                break;
            case DataType::DATE:
                {
                    css::util::Date* pDate = static_cast< css::util::Date*>(m_aValue.m_pValue);
                    aValue.Day      = pDate->Day;
                    aValue.Month    = pDate->Month;
                    aValue.Year     = pDate->Year;
                }
                break;
            case DataType::TIME:
                {
                    css::util::Time* pTime = static_cast< css::util::Time*>(m_aValue.m_pValue);
                    aValue.NanoSeconds      = pTime->NanoSeconds;
                    aValue.Seconds          = pTime->Seconds;
                    aValue.Minutes          = pTime->Minutes;
                    aValue.Hours            = pTime->Hours;
                }
                break;
            case DataType::TIMESTAMP:
                aValue = *static_cast< css::util::DateTime*>(m_aValue.m_pValue);
                break;
            default:
                {
                    Any aAnyValue = makeAny();
                    aAnyValue >>= aValue;
                    break;
                }
        }
    }
    return aValue;
}

void ORowSetValue::setSigned(bool _bMod)
{
    if ( m_bSigned != _bMod )
    {
        m_bSigned = _bMod;
        if ( !m_bNull )
        {
            sal_Int32 nType = m_eTypeKind;
            switch(m_eTypeKind)
            {
                case DataType::TINYINT:
                    if ( m_bSigned )
                        (*this) = getInt8();
                    else
                    {
                        m_bSigned = !m_bSigned;
                        (*this) = getInt16();
                        m_bSigned = !m_bSigned;
                    }
                    break;
                case DataType::SMALLINT:
                    if ( m_bSigned )
                        (*this) = getInt16();
                    else
                    {
                        m_bSigned = !m_bSigned;
                        (*this) = getInt32();
                        m_bSigned = !m_bSigned;
                    }
                    break;
                case DataType::INTEGER:
                    if ( m_bSigned )
                        (*this) = getInt32();
                    else
                    {
                        m_bSigned = !m_bSigned;
                        (*this) = getLong();
                        m_bSigned = !m_bSigned;
                    }
                    break;
                case DataType::BIGINT:
                    if ( m_bSigned )
                        m_aValue.m_nInt64 = static_cast<sal_Int64>(m_aValue.m_uInt64);
                    else
                        m_aValue.m_uInt64 = static_cast<sal_uInt64>(m_aValue.m_nInt64);
                    break;
            }
            m_eTypeKind = nType;
        }
    }
}


namespace detail
{
    class SAL_NO_VTABLE IValueSource
    {
    public:
        virtual OUString             getString() const = 0;
        virtual bool                        getBoolean() const = 0;
        virtual sal_Int8                    getByte() const = 0;
        virtual sal_Int16                   getShort() const = 0;
        virtual sal_Int32                   getInt() const = 0;
        virtual sal_Int64                   getLong() const = 0;
        virtual float                       getFloat() const = 0;
        virtual double                      getDouble() const = 0;
        virtual Date                        getDate() const = 0;
        virtual css::util::Time             getTime() const = 0;
        virtual DateTime                    getTimestamp() const = 0;
        virtual Sequence< sal_Int8 >        getBytes() const = 0;
        virtual Reference< XBlob >          getBlob() const = 0;
        virtual Reference< XClob >          getClob() const = 0;
        virtual Any                         getObject() const = 0;
        virtual bool                        wasNull() const = 0;

        virtual ~IValueSource() { }
    };

    class RowValue : public IValueSource
    {
    public:
        RowValue( const Reference< XRow >& _xRow, const sal_Int32 _nPos )
            :m_xRow( _xRow )
            ,m_nPos( _nPos )
        {
        }

        // IValueSource
        virtual OUString             getString() const override           { return m_xRow->getString( m_nPos ); };
        virtual bool                        getBoolean() const override          { return m_xRow->getBoolean( m_nPos ); };
        virtual sal_Int8                    getByte() const override             { return m_xRow->getByte( m_nPos ); };
        virtual sal_Int16                   getShort() const override            { return m_xRow->getShort( m_nPos ); }
        virtual sal_Int32                   getInt() const override              { return m_xRow->getInt( m_nPos ); }
        virtual sal_Int64                   getLong() const override             { return m_xRow->getLong( m_nPos ); }
        virtual float                       getFloat() const override            { return m_xRow->getFloat( m_nPos ); };
        virtual double                      getDouble() const override           { return m_xRow->getDouble( m_nPos ); };
        virtual Date                        getDate() const override             { return m_xRow->getDate( m_nPos ); };
        virtual css::util::Time             getTime() const override             { return m_xRow->getTime( m_nPos ); };
        virtual DateTime                    getTimestamp() const override        { return m_xRow->getTimestamp( m_nPos ); };
        virtual Sequence< sal_Int8 >        getBytes() const override            { return m_xRow->getBytes( m_nPos ); };
        virtual Reference< XBlob >          getBlob() const override             { return m_xRow->getBlob( m_nPos ); };
        virtual Reference< XClob >          getClob() const override             { return m_xRow->getClob( m_nPos ); };
        virtual Any                         getObject() const override           { return m_xRow->getObject( m_nPos ,nullptr); };
        virtual bool                        wasNull() const override             { return m_xRow->wasNull( ); };

    private:
        const Reference< XRow > m_xRow;
        const sal_Int32         m_nPos;
    };

    class ColumnValue : public IValueSource
    {
    public:
        explicit ColumnValue( const Reference< XColumn >& _rxColumn )
            :m_xColumn( _rxColumn )
        {
        }

        // IValueSource
        virtual OUString             getString() const override           { return m_xColumn->getString(); };
        virtual bool                        getBoolean() const override          { return m_xColumn->getBoolean(); };
        virtual sal_Int8                    getByte() const override             { return m_xColumn->getByte(); };
        virtual sal_Int16                   getShort() const override            { return m_xColumn->getShort(); }
        virtual sal_Int32                   getInt() const override              { return m_xColumn->getInt(); }
        virtual sal_Int64                   getLong() const override             { return m_xColumn->getLong(); }
        virtual float                       getFloat() const override            { return m_xColumn->getFloat(); };
        virtual double                      getDouble() const override           { return m_xColumn->getDouble(); };
        virtual Date                        getDate() const override             { return m_xColumn->getDate(); };
        virtual css::util::Time             getTime() const override             { return m_xColumn->getTime(); };
        virtual DateTime                    getTimestamp() const override        { return m_xColumn->getTimestamp(); };
        virtual Sequence< sal_Int8 >        getBytes() const override            { return m_xColumn->getBytes(); };
        virtual Reference< XBlob >          getBlob() const override             { return m_xColumn->getBlob(); };
        virtual Reference< XClob >          getClob() const override             { return m_xColumn->getClob(); };
        virtual Any                         getObject() const override           { return m_xColumn->getObject( nullptr ); };
        virtual bool                        wasNull() const override             { return m_xColumn->wasNull( ); };

    private:
        const Reference< XColumn >  m_xColumn;
    };
}


void ORowSetValue::fill( const sal_Int32 _nType, const Reference< XColumn >& _rxColumn )
{
    detail::ColumnValue aColumnValue( _rxColumn );
    impl_fill( _nType, true, aColumnValue );
}


void ORowSetValue::fill( sal_Int32 _nPos, sal_Int32 _nType, bool  _bNullable, const Reference< XRow>& _xRow )
{
    detail::RowValue aRowValue( _xRow, _nPos );
    impl_fill( _nType, _bNullable, aRowValue );
}


void ORowSetValue::fill(sal_Int32 _nPos,
                     sal_Int32 _nType,
                     const css::uno::Reference< css::sdbc::XRow>& _xRow)
{
    fill(_nPos,_nType,true,_xRow);
}


void ORowSetValue::impl_fill( const sal_Int32 _nType, bool _bNullable, const detail::IValueSource& _rValueSource )
{
    switch(_nType)
    {
    case DataType::CHAR:
    case DataType::VARCHAR:
    case DataType::DECIMAL:
    case DataType::NUMERIC:
    case DataType::LONGVARCHAR:
        (*this) = _rValueSource.getString();
        break;
    case DataType::BIGINT:
        if ( isSigned() )
            (*this) = _rValueSource.getLong();
        else
            // TODO: this is rather horrible performance-wise
            //       but fixing it needs extending the css::sdbc::XRow API
            //       to have a getULong(), and needs updating all drivers :-|
            //       When doing that, add getUByte, getUShort, getUInt for symmetry/completeness
            (*this) = _rValueSource.getString().toUInt64();
        break;
    case DataType::FLOAT:
        (*this) = _rValueSource.getFloat();
        break;
    case DataType::DOUBLE:
    case DataType::REAL:
        (*this) = _rValueSource.getDouble();
        break;
    case DataType::DATE:
        (*this) = _rValueSource.getDate();
        break;
    case DataType::TIME:
        (*this) = _rValueSource.getTime();
        break;
    case DataType::TIMESTAMP:
        (*this) = _rValueSource.getTimestamp();
        break;
    case DataType::BINARY:
    case DataType::VARBINARY:
    case DataType::LONGVARBINARY:
        (*this) = _rValueSource.getBytes();
        break;
    case DataType::BIT:
    case DataType::BOOLEAN:
        (*this) = _rValueSource.getBoolean();
        break;
    case DataType::TINYINT:
        if ( isSigned() )
            (*this) = _rValueSource.getByte();
        else
            (*this) = _rValueSource.getShort();
        break;
    case DataType::SMALLINT:
        if ( isSigned() )
            (*this) = _rValueSource.getShort();
        else
            (*this) = _rValueSource.getInt();
        break;
    case DataType::INTEGER:
        if ( isSigned() )
            (*this) = _rValueSource.getInt();
        else
            (*this) = _rValueSource.getLong();
        break;
    case DataType::CLOB:
        (*this) = css::uno::makeAny(_rValueSource.getClob());
        setTypeKind(DataType::CLOB);
        break;
    case DataType::BLOB:
        (*this) = css::uno::makeAny(_rValueSource.getBlob());
        setTypeKind(DataType::BLOB);
        break;
    case DataType::OTHER:
        (*this) = _rValueSource.getObject();
        setTypeKind(DataType::OTHER);
        break;
    default:
        SAL_WARN( "connectivity.commontools", "ORowSetValue::fill: unsupported type!" );
        (*this) = _rValueSource.getObject();
        break;
    }
    if ( _bNullable && _rValueSource.wasNull() )
        setNull();
    setTypeKind(_nType);
}

void ORowSetValue::fill(const Any& _rValue)
{
    switch (_rValue.getValueType().getTypeClass())
    {
        case TypeClass_VOID:
            setNull();            break;
        case TypeClass_BOOLEAN:
        {
            bool bValue( false );
            _rValue >>= bValue;
            (*this) = bValue;
            break;
        }
        case TypeClass_CHAR:
        {
            sal_Unicode aDummy(0);
            _rValue >>= aDummy;
            (*this) = OUString(aDummy);
            break;
        }
        case TypeClass_STRING:
        {
            OUString sDummy;
            _rValue >>= sDummy;
            (*this) = sDummy;
            break;
        }
        case TypeClass_FLOAT:
        {
            float aDummy(0.0);
            _rValue >>= aDummy;
            (*this) = aDummy;
            break;
        }
        case TypeClass_DOUBLE:
        {
            double aDummy(0.0);
            _rValue >>= aDummy;
            (*this) = aDummy;
            break;
        }
        case TypeClass_BYTE:
        {
            sal_Int8 aDummy(0);
            _rValue >>= aDummy;
            (*this) = aDummy;
            break;
        }
        case TypeClass_SHORT:
        {
            sal_Int16 aDummy(0);
            _rValue >>= aDummy;
            (*this) = aDummy;
            break;
        }
        case TypeClass_UNSIGNED_SHORT:
        {
            sal_uInt16 nValue(0);
            _rValue >>= nValue;
            (*this) = nValue;
            break;
        }
        case TypeClass_LONG:
        {
            sal_Int32 aDummy(0);
            _rValue >>= aDummy;
            (*this) = aDummy;
            break;
        }
        case TypeClass_UNSIGNED_LONG:
        {
            sal_uInt32 nValue(0);
            _rValue >>= nValue;
            (*this) = static_cast<sal_Int64>(nValue);
            setSigned(false);
            break;
        }
        case TypeClass_HYPER:
        {
            sal_Int64 nValue(0);
            _rValue >>= nValue;
            (*this) = nValue;
            break;
        }
        case TypeClass_UNSIGNED_HYPER:
        {
            sal_uInt64 nValue(0);
            _rValue >>= nValue;
            (*this) = nValue;
            setSigned(false);
            break;
        }
        case TypeClass_ENUM:
        {
            sal_Int32 enumValue( 0 );
            ::cppu::enum2int( enumValue, _rValue );
            (*this) = enumValue;
        }
        break;

        case TypeClass_SEQUENCE:
        {
            Sequence<sal_Int8> aDummy;
            if ( _rValue >>= aDummy )
                (*this) = aDummy;
            else
                SAL_WARN( "connectivity.commontools", "ORowSetValue::fill: unsupported sequence type!" );
            break;
        }

        case TypeClass_STRUCT:
        {
            css::util::Date aDate;
            css::util::Time aTime;
            css::util::DateTime aDateTime;
            if ( _rValue >>= aDate )
            {
                (*this) = aDate;
            }
            else if ( _rValue >>= aTime )
            {
                (*this) = aTime;
            }
            else if ( _rValue >>= aDateTime )
            {
                (*this) = aDateTime;
            }
            else
                SAL_WARN( "connectivity.commontools", "ORowSetValue::fill: unsupported structure!" );

            break;
        }
        case TypeClass_INTERFACE:
            {
                Reference< XClob > xClob;
                if ( _rValue >>= xClob )
                {
                    (*this) = _rValue;
                    setTypeKind(DataType::CLOB);
                }
                else
                {
                    Reference< XBlob > xBlob;
                    if ( _rValue >>= xBlob )
                    {
                        (*this) = _rValue;
                        setTypeKind(DataType::BLOB);
                    }
                    else
                    {
                        (*this) = _rValue;
                    }
                }
            }
            break;

        default:
            SAL_WARN( "connectivity.commontools","Unknown type");
            break;
    }
}

}   // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
