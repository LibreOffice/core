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
#include <stdio.h>
#include "connectivity/FValue.hxx"
#include "connectivity/CommonTools.hxx"
#include <connectivity/dbconversion.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>

using namespace ::dbtools;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::io;

namespace connectivity
{

namespace {
    static sal_Bool isStorageCompatible(sal_Int32 _eType1, sal_Int32 _eType2)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::isStorageCompatible" );
        sal_Bool bIsCompatible = sal_True;

        if (_eType1 != _eType2)
        {
            RTL_LOGFILE_CONTEXT_TRACE( aLogger, "ORowSetValue::isStorageCompatible _eType1 != _eType2" );
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
                    bIsCompatible = sal_False;
            }
        }
        return bIsCompatible;
    }
}

// -----------------------------------------------------------------------------
#ifdef DBG_UTIL

#include <vector>
#include <rtl/string.h>

namespace tracing
{
    struct AllocationType
    {
        const sal_Char* pName;
        sal_Int32       nAllocatedUnits;

        AllocationType( ) : pName( NULL ), nAllocatedUnits( 0 ) { }
    };

    // =============================================================================
    class AllocationTracer
    {
    public:
        typedef ::std::vector< AllocationType > AllocationState;
        static AllocationState                  s_aAllocated;
        static ::osl::Mutex                     s_aMutex;

    public:
        static void registerUnit( const sal_Char* _pName );
        static void revokeUnit( const sal_Char* _pName );

    private:
        static AllocationState::iterator    getLocation( const sal_Char* _pName );
    };

    // =============================================================================
    AllocationTracer::AllocationState::iterator AllocationTracer::getLocation( const sal_Char* _pName )
    {
        AllocationState::iterator aLookFor = s_aAllocated.begin();
        for (   ;
                aLookFor != s_aAllocated.end();
                ++aLookFor
            )
        {
            if ( 0 == rtl_str_compare( aLookFor->pName, _pName ) )
                // found
                return aLookFor;
        }
        // not found
        s_aAllocated.push_back( AllocationType() );
        aLookFor = s_aAllocated.end(); --aLookFor;
        aLookFor->pName = _pName;   // note that this assumes that _pName is a constant string ....
        return aLookFor;
    }

    // =============================================================================
    AllocationTracer::AllocationState           AllocationTracer::s_aAllocated;
    ::osl::Mutex                                AllocationTracer::s_aMutex;

    // =============================================================================
    void AllocationTracer::registerUnit( const sal_Char* _pName )
    {
        ::osl::MutexGuard aGuard( s_aMutex );

        AllocationState::iterator aPos = getLocation( _pName );
        ++aPos->nAllocatedUnits;
    }

    // =============================================================================
    void AllocationTracer::revokeUnit( const sal_Char* _pName )
    {
        ::osl::MutexGuard aGuard( s_aMutex );

        AllocationState::iterator aPos = getLocation( _pName );
        --aPos->nAllocatedUnits;
    }

#define TRACE_ALLOC( type ) tracing::AllocationTracer::registerUnit( #type );
#define TRACE_FREE( type )  tracing::AllocationTracer::revokeUnit( #type );
}
#else
#define TRACE_ALLOC( type )
#define TRACE_FREE( type )
#endif

// -----------------------------------------------------------------------------
void ORowSetValue::setTypeKind(sal_Int32 _eType)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::setTypeKind" );
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
                (*this) = getLong();
                break;

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
                (*this) = getInt32();
                break;
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
                OSL_FAIL("ORowSetValue::setTypeKind(): UNSUPPORTED TYPE!");
        }
    }

    m_eTypeKind = _eType;
}

// -----------------------------------------------------------------------------
void ORowSetValue::free()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::free" );
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
                m_aValue.m_pString = NULL;
                break;
            case DataType::INTEGER:
                if ( !m_bSigned )
                {
                    delete (sal_Int64*)m_aValue.m_pValue;
                    TRACE_FREE( sal_Int64 )
                    m_aValue.m_pValue = NULL;
                }
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                {
                    delete (sal_Int64*)m_aValue.m_pValue;
                    TRACE_FREE( sal_Int64 )
                    m_aValue.m_pValue = NULL;
                }
                else
                {
                    OSL_ENSURE(m_aValue.m_pString,"String pointer is null!");
                    rtl_uString_release(m_aValue.m_pString);
                    m_aValue.m_pString = NULL;
                }
                break;
            case DataType::FLOAT:
                delete (float*)m_aValue.m_pValue;
                TRACE_FREE( float )
                m_aValue.m_pValue = NULL;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                delete (double*)m_aValue.m_pValue;
                TRACE_FREE( double )
                m_aValue.m_pValue = NULL;
                break;
            case DataType::DATE:
                delete (::com::sun::star::util::Date*)m_aValue.m_pValue;
                TRACE_FREE( Date )
                m_aValue.m_pValue = NULL;
                break;
            case DataType::TIME:
                delete (::com::sun::star::util::Time*)m_aValue.m_pValue;
                TRACE_FREE( Time )
                m_aValue.m_pValue = NULL;
                break;
            case DataType::TIMESTAMP:
                delete (::com::sun::star::util::DateTime*)m_aValue.m_pValue;
                TRACE_FREE( DateTime )
                m_aValue.m_pValue = NULL;
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                delete (Sequence<sal_Int8>*)m_aValue.m_pValue;
                TRACE_FREE( Sequence_sal_Int8 )
                m_aValue.m_pValue = NULL;
                break;
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::OBJECT:
                delete (Any*)m_aValue.m_pValue;
                TRACE_FREE( Any )
                m_aValue.m_pValue = NULL;
                break;
            case DataType::BIT:
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::BOOLEAN:
                break;
            default:
                if ( m_aValue.m_pValue )
                {
                    delete (Any*)m_aValue.m_pValue;
                    TRACE_FREE( Any )
                    m_aValue.m_pValue = NULL;
                }
                break;

        }
        m_bNull = sal_True;
    }
}
// -----------------------------------------------------------------------------
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
            case DataType::BIGINT:
                if ( _rRH.m_bSigned )
                {
                    m_aValue.m_pValue   = new sal_Int64(*(sal_Int64*)_rRH.m_aValue.m_pValue);
                    TRACE_ALLOC( sal_Int64 )
                }
                else
                {
                    rtl_uString_acquire(_rRH.m_aValue.m_pString);
                    m_aValue.m_pString = _rRH.m_aValue.m_pString;
                }
                break;
            case DataType::FLOAT:
                m_aValue.m_pValue   = new float(*(float*)_rRH.m_aValue.m_pValue);
                TRACE_ALLOC( float )
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                m_aValue.m_pValue   = new double(*(double*)_rRH.m_aValue.m_pValue);
                TRACE_ALLOC( double )
                break;
            case DataType::DATE:
                m_aValue.m_pValue   = new Date(*(Date*)_rRH.m_aValue.m_pValue);
                TRACE_ALLOC( Date )
                break;
            case DataType::TIME:
                m_aValue.m_pValue   = new Time(*(Time*)_rRH.m_aValue.m_pValue);
                TRACE_ALLOC( Time )
                break;
            case DataType::TIMESTAMP:
                m_aValue.m_pValue   = new DateTime(*(DateTime*)_rRH.m_aValue.m_pValue);
                TRACE_ALLOC( DateTime )
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                m_aValue.m_pValue   = new Sequence<sal_Int8>(*(Sequence<sal_Int8>*)_rRH.m_aValue.m_pValue);
                TRACE_ALLOC( Sequence_sal_Int8 )
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                m_aValue.m_bBool    = _rRH.m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt8    = _rRH.m_aValue.m_nInt8;
                else
                    m_aValue.m_nInt16   = _rRH.m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt16   = _rRH.m_aValue.m_nInt16;
                else
                    m_aValue.m_nInt32   = _rRH.m_aValue.m_nInt32;
                break;
            case DataType::INTEGER:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt32   = _rRH.m_aValue.m_nInt32;
                else
                {
                    m_aValue.m_pValue   = new sal_Int64(*(sal_Int64*)_rRH.m_aValue.m_pValue);
                    TRACE_ALLOC( sal_Int64 )
                }
                break;
            default:
                m_aValue.m_pValue   = new Any(*(Any*)_rRH.m_aValue.m_pValue);
                TRACE_ALLOC( Any )
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
                (*this) = ::rtl::OUString(_rRH.m_aValue.m_pString);
                break;
            case DataType::BIGINT:
                if ( _rRH.m_bSigned )
                    (*this) = *(sal_Int64*)_rRH.m_aValue.m_pValue;
                else
                    (*this) = ::rtl::OUString(_rRH.m_aValue.m_pString);
                break;
            case DataType::FLOAT:
                (*this) = *(float*)_rRH.m_aValue.m_pValue;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                (*this) = *(double*)_rRH.m_aValue.m_pValue;
                break;
            case DataType::DATE:
                (*this) = *(Date*)_rRH.m_aValue.m_pValue;
                break;
            case DataType::TIME:
                (*this) = *(Time*)_rRH.m_aValue.m_pValue;
                break;
            case DataType::TIMESTAMP:
                (*this) = *(DateTime*)_rRH.m_aValue.m_pValue;
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                (*this) = *(Sequence<sal_Int8>*)_rRH.m_aValue.m_pValue;
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                m_aValue.m_bBool    = _rRH.m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt8    = _rRH.m_aValue.m_nInt8;
                else
                    m_aValue.m_nInt16   = _rRH.m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt16   = _rRH.m_aValue.m_nInt16;
                else
                    m_aValue.m_nInt32   = _rRH.m_aValue.m_nInt32;
                break;
            case DataType::INTEGER:
                if ( _rRH.m_bSigned )
                    m_aValue.m_nInt32   = _rRH.m_aValue.m_nInt32;
                else
                    *static_cast<sal_Int64*>(m_aValue.m_pValue) = *(sal_Int64*)_rRH.m_aValue.m_pValue;
                break;
            default:
                (*(Any*)m_aValue.m_pValue)  = (*(Any*)_rRH.m_aValue.m_pValue);
        }
    }

    m_bNull     = _rRH.m_bNull;
    // OJ: BUGID: 96277
    m_eTypeKind = _rRH.m_eTypeKind;

    return *this;
}
// -------------------------------------------------------------------------

ORowSetValue& ORowSetValue::operator=(const Date& _rRH)
{
    if(m_eTypeKind != DataType::DATE)
        free();

    if(m_bNull)
    {
        m_aValue.m_pValue = new Date(_rRH);
        TRACE_ALLOC( Date )
        m_eTypeKind = DataType::DATE;
        m_bNull = sal_False;
    }
    else
        *(Date*)m_aValue.m_pValue = _rRH;

    return *this;
}
// -------------------------------------------------------------------------
ORowSetValue& ORowSetValue::operator=(const Time& _rRH)
{
    if(m_eTypeKind != DataType::TIME)
        free();

    if(m_bNull)
    {
        m_aValue.m_pValue = new Time(_rRH);
        TRACE_ALLOC( Time )
        m_eTypeKind = DataType::TIME;
        m_bNull = sal_False;
    }
    else
        *(Time*)m_aValue.m_pValue = _rRH;

    return *this;
}
// -------------------------------------------------------------------------
ORowSetValue& ORowSetValue::operator=(const DateTime& _rRH)
{
    if(m_eTypeKind != DataType::TIMESTAMP)
        free();
    if(m_bNull)
    {
        m_aValue.m_pValue = new DateTime(_rRH);
        TRACE_ALLOC( DateTime )
        m_eTypeKind = DataType::TIMESTAMP;
        m_bNull = sal_False;
    }
    else
        *(DateTime*)m_aValue.m_pValue = _rRH;

    return *this;
}
// -------------------------------------------------------------------------

ORowSetValue& ORowSetValue::operator=(const ::rtl::OUString& _rRH)
{
    if(m_eTypeKind != DataType::VARCHAR || m_aValue.m_pString != _rRH.pData)
    {
        free();
        m_bNull = sal_False;

        m_aValue.m_pString = _rRH.pData;
        rtl_uString_acquire(m_aValue.m_pString);
        m_eTypeKind = DataType::VARCHAR;
    }

    return *this;
}
// -------------------------------------------------------------------------

ORowSetValue& ORowSetValue::operator=(const double& _rRH)
{
    if( !isStorageCompatible(m_eTypeKind,DataType::DOUBLE) )
        free();

    if(m_bNull)
    {
        m_aValue.m_pValue = new double(_rRH);
        TRACE_ALLOC( double )
        m_eTypeKind = DataType::DOUBLE;
        m_bNull = sal_False;
    }
    else
        *(double*)m_aValue.m_pValue = _rRH;

    return *this;
}
// -----------------------------------------------------------------------------
ORowSetValue& ORowSetValue::operator=(const float& _rRH)
{
    if(m_eTypeKind != DataType::FLOAT)
        free();

    if(m_bNull)
    {
        m_aValue.m_pValue = new float(_rRH);
        TRACE_ALLOC( float )
        m_eTypeKind = DataType::FLOAT;
        m_bNull = sal_False;
    }
    else
        *(float*)m_aValue.m_pValue = _rRH;

    return *this;
}
// -------------------------------------------------------------------------

ORowSetValue& ORowSetValue::operator=(const sal_Int8& _rRH)
{
    if(m_eTypeKind != DataType::TINYINT )
        free();

    m_aValue.m_nInt8 = _rRH;
    m_eTypeKind = DataType::TINYINT;
    m_bNull = sal_False;
    return *this;
}
// -------------------------------------------------------------------------

ORowSetValue& ORowSetValue::operator=(const sal_Int16& _rRH)
{
    if(m_eTypeKind != DataType::SMALLINT )
        free();

    m_aValue.m_nInt16 = _rRH;
    m_eTypeKind = DataType::SMALLINT;
    m_bNull = sal_False;

    return *this;
}
// -------------------------------------------------------------------------

ORowSetValue& ORowSetValue::operator=(const sal_Int32& _rRH)
{
    if(m_eTypeKind != DataType::INTEGER )
        free();

    if ( m_bSigned )
        m_aValue.m_nInt32 = _rRH;
    else
    {
        if ( m_bNull )
        {
            m_aValue.m_pValue = new sal_Int64(_rRH);
            TRACE_ALLOC( sal_Int64 )
        }
        else
            *static_cast<sal_Int64*>(m_aValue.m_pValue) = static_cast<sal_Int64>(_rRH);
    }

    m_eTypeKind = DataType::INTEGER;
    m_bNull = sal_False;

    return *this;
}
// -------------------------------------------------------------------------

ORowSetValue& ORowSetValue::operator=(const sal_Bool _rRH)
{
    if(m_eTypeKind != DataType::BIT && DataType::BOOLEAN != m_eTypeKind )
        free();

    m_aValue.m_bBool = _rRH;
    m_eTypeKind = DataType::BIT;
    m_bNull = sal_False;

    return *this;
}
// -------------------------------------------------------------------------
ORowSetValue& ORowSetValue::operator=(const sal_Int64& _rRH)
{
    if ( DataType::BIGINT != m_eTypeKind || !m_bSigned )
        free();

    if ( m_bSigned )
    {
        if(m_bNull)
        {
            m_aValue.m_pValue = new sal_Int64(_rRH);
            TRACE_ALLOC( sal_Int64 )
        }
        else
            *static_cast<sal_Int64*>(m_aValue.m_pValue) = _rRH;
    }
    else
    {
        ::rtl::OUString aVal = ::rtl::OUString::valueOf(_rRH);
        m_aValue.m_pString = aVal.pData;
        rtl_uString_acquire(m_aValue.m_pString);
    }

    m_eTypeKind = DataType::BIGINT;
    m_bNull = sal_False;

    return *this;
}
// -------------------------------------------------------------------------
ORowSetValue& ORowSetValue::operator=(const Sequence<sal_Int8>& _rRH)
{
    if (!isStorageCompatible(DataType::LONGVARBINARY,m_eTypeKind))
        free();

    if (m_bNull)
    {
        m_aValue.m_pValue = new Sequence<sal_Int8>(_rRH);
        TRACE_ALLOC( Sequence_sal_Int8 )
    }
    else
        *static_cast< Sequence< sal_Int8 >* >(m_aValue.m_pValue) = _rRH;

    m_eTypeKind = DataType::LONGVARBINARY;
    m_bNull = sal_False;

    return *this;
}
// -------------------------------------------------------------------------
ORowSetValue& ORowSetValue::operator=(const Any& _rAny)
{
    if (!isStorageCompatible(DataType::OBJECT,m_eTypeKind))
        free();

    if ( m_bNull )
    {
        m_aValue.m_pValue = new Any(_rAny);
        TRACE_ALLOC( Any )
    }
    else
        *static_cast<Any*>(m_aValue.m_pValue) = _rAny;

    m_eTypeKind = DataType::OBJECT;
    m_bNull = sal_False;

    return *this;
}
// -------------------------------------------------------------------------

sal_Bool operator==(const Date& _rLH,const Date& _rRH)
{
    return _rLH.Day == _rRH.Day && _rLH.Month == _rRH.Month && _rLH.Year == _rRH.Year;
}
// -------------------------------------------------------------------------

sal_Bool operator==(const Time& _rLH,const Time& _rRH)
{
    return _rLH.Minutes == _rRH.Minutes && _rLH.Hours == _rRH.Hours && _rLH.Seconds == _rRH.Seconds && _rLH.HundredthSeconds == _rRH.HundredthSeconds;
}
// -------------------------------------------------------------------------

sal_Bool operator==(const DateTime& _rLH,const DateTime& _rRH)
{
    return _rLH.Day == _rRH.Day && _rLH.Month == _rRH.Month && _rLH.Year == _rRH.Year &&
        _rLH.Minutes == _rRH.Minutes && _rLH.Hours == _rRH.Hours && _rLH.Seconds == _rRH.Seconds && _rLH.HundredthSeconds == _rRH.HundredthSeconds;
}
// -------------------------------------------------------------------------

bool ORowSetValue::operator==(const ORowSetValue& _rRH) const
{
    if ( m_bNull != _rRH.isNull() )
        return false;

    if(m_bNull && _rRH.isNull())
        return true;

    if ( m_eTypeKind != _rRH.m_eTypeKind )
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
    OSL_ENSURE(!m_bNull,"SHould not be null!");
    switch(m_eTypeKind)
    {
        case DataType::VARCHAR:
        case DataType::CHAR:
        case DataType::LONGVARCHAR:
        {
            ::rtl::OUString aVal1(m_aValue.m_pString);
            ::rtl::OUString aVal2(_rRH.m_aValue.m_pString);
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
                ::rtl::OUString aVal1(m_aValue.m_pString);
                ::rtl::OUString aVal2(_rRH.m_aValue.m_pString);
                bRet = aVal1 == aVal2;
            }
            break;
        case DataType::FLOAT:
            bRet = *(float*)m_aValue.m_pValue == *(float*)_rRH.m_aValue.m_pValue;
            break;
        case DataType::DOUBLE:
        case DataType::REAL:
            bRet = *(double*)m_aValue.m_pValue == *(double*)_rRH.m_aValue.m_pValue;
            break;
        case DataType::TINYINT:
            bRet = m_bSigned ? ( m_aValue.m_nInt8 == _rRH.m_aValue.m_nInt8 ) : (m_aValue.m_nInt16 == _rRH.m_aValue.m_nInt16);
            break;
        case DataType::SMALLINT:
            bRet = m_bSigned ? ( m_aValue.m_nInt16 == _rRH.m_aValue.m_nInt16 ) : (m_aValue.m_nInt32 == _rRH.m_aValue.m_nInt32);
            break;
        case DataType::INTEGER:
            bRet = m_bSigned ? ( m_aValue.m_nInt32 == _rRH.m_aValue.m_nInt32 ) : (*(sal_Int64*)m_aValue.m_pValue == *(sal_Int64*)_rRH.m_aValue.m_pValue);
            break;
        case DataType::BIGINT:
            if ( m_bSigned )
                bRet = *(sal_Int64*)m_aValue.m_pValue == *(sal_Int64*)_rRH.m_aValue.m_pValue;
            else
            {
                ::rtl::OUString aVal1(m_aValue.m_pString);
                ::rtl::OUString aVal2(_rRH.m_aValue.m_pString);
                bRet = aVal1 == aVal2;
            }
            break;
        case DataType::BIT:
        case DataType::BOOLEAN:
            bRet = m_aValue.m_bBool == _rRH.m_aValue.m_bBool;
            break;
        case DataType::DATE:
            bRet = *(Date*)m_aValue.m_pValue == *(Date*)_rRH.m_aValue.m_pValue;
            break;
        case DataType::TIME:
            bRet = *(Time*)m_aValue.m_pValue == *(Time*)_rRH.m_aValue.m_pValue;
            break;
        case DataType::TIMESTAMP:
            bRet = *(DateTime*)m_aValue.m_pValue == *(DateTime*)_rRH.m_aValue.m_pValue;
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
            OSL_FAIL("ORowSetValue::operator==(): UNSUPPORTED TYPE!");
            break;
    }
    return bRet;
}
// -------------------------------------------------------------------------
Any ORowSetValue::makeAny() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::makeAny" );
    Any rValue;
    if(isBound() && !isNull())
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::LONGVARCHAR:
                OSL_ENSURE(m_aValue.m_pString,"Value is null!");
                rValue <<= (::rtl::OUString)m_aValue.m_pString;
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                {
                    OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                    rValue <<= *(sal_Int64*)m_aValue.m_pValue;
                }
                else
                {
                    OSL_ENSURE(m_aValue.m_pString,"Value is null!");
                    rValue <<= (::rtl::OUString)m_aValue.m_pString;
                }
                break;
            case DataType::FLOAT:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *(float*)m_aValue.m_pValue;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *(double*)m_aValue.m_pValue;
                break;
            case DataType::DATE:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *(Date*)m_aValue.m_pValue;
                break;
            case DataType::TIME:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *(Time*)m_aValue.m_pValue;
                break;
            case DataType::TIMESTAMP:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *(DateTime*)m_aValue.m_pValue;
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                rValue <<= *(Sequence<sal_Int8>*)m_aValue.m_pValue;
                break;
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::OBJECT:
            case DataType::OTHER:
                rValue = getAny();
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                rValue.setValue( &m_aValue.m_bBool, ::getCppuBooleanType() );
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    rValue <<= m_aValue.m_nInt8;
                else
                    rValue <<= m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    rValue <<= m_aValue.m_nInt16;
                else
                    rValue <<= m_aValue.m_nInt32;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    rValue <<= m_aValue.m_nInt32;
                else
                {
                    OSL_ENSURE(m_aValue.m_pValue,"Value is null!");
                    rValue <<= *(sal_Int64*)m_aValue.m_pValue;
                }
                break;
            default:
                OSL_FAIL("ORowSetValue::makeAny(): UNSPUPPORTED TYPE!");
                rValue = getAny();
                break;
        }
    }
    return rValue;
}
// -------------------------------------------------------------------------
::rtl::OUString ORowSetValue::getString( ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getString" );
    ::rtl::OUString aRet;
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
            case DataType::BIGINT:
                if ( m_bSigned )
                    aRet = ::rtl::OUString::valueOf((sal_Int64)*this);
                else
                    aRet = m_aValue.m_pString;
                break;
            case DataType::FLOAT:
                aRet = ::rtl::OUString::valueOf((float)*this);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                aRet = ::rtl::OUString::valueOf((double)*this);
                break;
            case DataType::DATE:
                aRet = connectivity::toDateString(*this);
                break;
            case DataType::TIME:
                aRet = connectivity::toTimeString(*this);
                break;
            case DataType::TIMESTAMP:
                aRet = connectivity::toDateTimeString(*this);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                {
                    ::rtl::OUStringBuffer sVal(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0x")));
                    Sequence<sal_Int8> aSeq(getSequence());
                    const sal_Int8* pBegin  = aSeq.getConstArray();
                    const sal_Int8* pEnd    = pBegin + aSeq.getLength();
                    for(;pBegin != pEnd;++pBegin)
                        sVal.append((sal_Int32)*pBegin,16);
                    aRet = sVal.makeStringAndClear();
                }
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                aRet = ::rtl::OUString::valueOf((sal_Int32)(sal_Bool)*this);
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    aRet = ::rtl::OUString::valueOf((sal_Int32)(sal_Int8)*this);
                else
                    aRet = ::rtl::OUString::valueOf((sal_Int32)(sal_Int16)*this);
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    aRet = ::rtl::OUString::valueOf((sal_Int32)(sal_Int16)*this);
                else
                    aRet = ::rtl::OUString::valueOf((sal_Int32)*this);
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    aRet = ::rtl::OUString::valueOf((sal_Int32)*this);
                else
                    aRet = ::rtl::OUString::valueOf((sal_Int64)*this);
                break;
            case DataType::CLOB:
                {
                    Any aValue( getAny() );
                    Reference< XClob > xClob;
                    if ( aValue >>= xClob )
                    {
                        if ( xClob.is() )
                        {
                            aRet = xClob->getSubString(1,(sal_Int32)xClob->length() );
                        }
                    }
                }
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= aRet;
                    break;
                }
        }
    }
    return aRet;
}
// -------------------------------------------------------------------------
sal_Bool ORowSetValue::getBool()    const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getBool" );
    sal_Bool bRet = sal_False;
    if(!m_bNull)
    {
        switch(getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                {
                    const ::rtl::OUString sValue(m_aValue.m_pString);
                    const static ::rtl::OUString s_sTrue(RTL_CONSTASCII_USTRINGPARAM("true"));
                    const static ::rtl::OUString s_sFalse(RTL_CONSTASCII_USTRINGPARAM("false"));
                    if ( sValue.equalsIgnoreAsciiCase(s_sTrue) )
                    {
                        bRet = sal_True;
                        break;
                    }
                    else if ( sValue.equalsIgnoreAsciiCase(s_sFalse) )
                    {
                        bRet = sal_False;
                        break;
                    }
                }
                // run through
            case DataType::DECIMAL:
            case DataType::NUMERIC:

                bRet = ::rtl::OUString(m_aValue.m_pString).toInt32() != 0;
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    bRet = *(sal_Int64*)m_aValue.m_pValue != 0;
                else
                    bRet = ::rtl::OUString(m_aValue.m_pString).toInt64() != 0;
                break;
            case DataType::FLOAT:
                bRet = *(float*)m_aValue.m_pValue != 0.0;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                bRet = *(double*)m_aValue.m_pValue != 0.0;
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                OSL_ASSERT(!"getBool() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                bRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                bRet = m_bSigned ? (m_aValue.m_nInt8  != 0) : (m_aValue.m_nInt16 != 0);
                break;
            case DataType::SMALLINT:
                bRet = m_bSigned ? (m_aValue.m_nInt16  != 0) : (m_aValue.m_nInt32 != 0);
                break;
            case DataType::INTEGER:
                bRet = m_bSigned ? (m_aValue.m_nInt32 != 0) : (*static_cast<sal_Int64*>(m_aValue.m_pValue) != sal_Int64(0));
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= bRet;
                    break;
                }
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------
sal_Int8 ORowSetValue::getInt8()    const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getInt8" );


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
                nRet = sal_Int8(::rtl::OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = sal_Int8(*(sal_Int64*)m_aValue.m_pValue);
                else
                    nRet = sal_Int8(::rtl::OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::FLOAT:
                nRet = sal_Int8(*(float*)m_aValue.m_pValue);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int8(*(double*)m_aValue.m_pValue);
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_ASSERT(!"getInt8() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = static_cast<sal_Int8>(m_aValue.m_nInt16);
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int8>(m_aValue.m_nInt16);
                else
                    nRet = static_cast<sal_Int8>(m_aValue.m_nInt32);
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int8>(m_aValue.m_nInt32);
                else
                    nRet = static_cast<sal_Int8>(*static_cast<sal_Int64*>(m_aValue.m_pValue));
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int16 ORowSetValue::getInt16()  const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getInt16" );


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
                nRet = sal_Int16(::rtl::OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = sal_Int16(*(sal_Int64*)m_aValue.m_pValue);
                else
                    nRet = sal_Int16(::rtl::OUString(m_aValue.m_pString).toInt32());
                break;
            case DataType::FLOAT:
                nRet = sal_Int16(*(float*)m_aValue.m_pValue);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int16(*(double*)m_aValue.m_pValue);
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_ASSERT(!"getInt16() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = static_cast<sal_Int16>(m_aValue.m_nInt32);
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = static_cast<sal_Int16>(m_aValue.m_nInt32);
                else
                    nRet = static_cast<sal_Int16>(*static_cast<sal_Int64*>(m_aValue.m_pValue));
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int32 ORowSetValue::getInt32()  const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getInt32" );
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
                nRet = ::rtl::OUString(m_aValue.m_pString).toInt32();
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = sal_Int32(*(sal_Int64*)m_aValue.m_pValue);
                else
                    nRet = ::rtl::OUString(m_aValue.m_pString).toInt32();
                break;
            case DataType::FLOAT:
                nRet = sal_Int32(*(float*)m_aValue.m_pValue);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int32(*(double*)m_aValue.m_pValue);
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDays(*(::com::sun::star::util::Date*)m_aValue.m_pValue);
                break;
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_ASSERT(!"getInt32() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_nInt32;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = static_cast<sal_Int32>(*static_cast<sal_Int64*>(m_aValue.m_pValue));
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}
// -------------------------------------------------------------------------
sal_Int64 ORowSetValue::getLong()   const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getLong" );
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
                nRet = ::rtl::OUString(m_aValue.m_pString).toInt64();
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = *(sal_Int64*)m_aValue.m_pValue;
                else
                    nRet = ::rtl::OUString(m_aValue.m_pString).toInt64();
                break;
            case DataType::FLOAT:
                nRet = sal_Int64(*(float*)m_aValue.m_pValue);
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = sal_Int64(*(double*)m_aValue.m_pValue);
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDays(*(::com::sun::star::util::Date*)m_aValue.m_pValue);
                break;
            case DataType::TIME:
            case DataType::TIMESTAMP:
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_ASSERT(!"getInt32() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_nInt32;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = *(sal_Int64*)m_aValue.m_pValue;
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}
// -------------------------------------------------------------------------
float ORowSetValue::getFloat()  const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getFloat" );
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
                nRet = ::rtl::OUString(m_aValue.m_pString).toFloat();
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = float(*(sal_Int64*)m_aValue.m_pValue);
                else
                    nRet = ::rtl::OUString(m_aValue.m_pString).toFloat();
                break;
            case DataType::FLOAT:
                nRet = *(float*)m_aValue.m_pValue;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = (float)*(double*)m_aValue.m_pValue;
                break;
            case DataType::DATE:
                nRet = (float)dbtools::DBTypeConversion::toDouble(*(::com::sun::star::util::Date*)m_aValue.m_pValue);
                break;
            case DataType::TIME:
                nRet = (float)dbtools::DBTypeConversion::toDouble(*(::com::sun::star::util::Time*)m_aValue.m_pValue);
                break;
            case DataType::TIMESTAMP:
                nRet = (float)dbtools::DBTypeConversion::toDouble(*(::com::sun::star::util::DateTime*)m_aValue.m_pValue);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_ASSERT(!"getDouble() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = (float)m_aValue.m_nInt32;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = (float)m_aValue.m_nInt32;
                else
                    nRet = float(*(sal_Int64*)m_aValue.m_pValue);
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}
// -------------------------------------------------------------------------
double ORowSetValue::getDouble()    const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getDouble" );


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
                nRet = ::rtl::OUString(m_aValue.m_pString).toDouble();
                break;
            case DataType::BIGINT:
                if ( m_bSigned )
                    nRet = double(*(sal_Int64*)m_aValue.m_pValue);
                else
                    nRet = ::rtl::OUString(m_aValue.m_pString).toDouble();
                break;
            case DataType::FLOAT:
                nRet = *(float*)m_aValue.m_pValue;
                break;
            case DataType::DOUBLE:
            case DataType::REAL:
                nRet = *(double*)m_aValue.m_pValue;
                break;
            case DataType::DATE:
                nRet = dbtools::DBTypeConversion::toDouble(*(::com::sun::star::util::Date*)m_aValue.m_pValue);
                break;
            case DataType::TIME:
                nRet = dbtools::DBTypeConversion::toDouble(*(::com::sun::star::util::Time*)m_aValue.m_pValue);
                break;
            case DataType::TIMESTAMP:
                nRet = dbtools::DBTypeConversion::toDouble(*(::com::sun::star::util::DateTime*)m_aValue.m_pValue);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::BLOB:
            case DataType::CLOB:
                OSL_ASSERT(!"getDouble() for this type is not allowed!");
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                nRet = m_aValue.m_bBool;
                break;
            case DataType::TINYINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt8;
                else
                    nRet = m_aValue.m_nInt16;
                break;
            case DataType::SMALLINT:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt16;
                else
                    nRet = m_aValue.m_nInt32;
                break;
            case DataType::INTEGER:
                if ( m_bSigned )
                    nRet = m_aValue.m_nInt32;
                else
                    nRet = double(*(sal_Int64*)m_aValue.m_pValue);
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= nRet;
                    break;
                }
        }
    }
    return nRet;
}
// -----------------------------------------------------------------------------
Sequence<sal_Int8>  ORowSetValue::getSequence() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getSequence" );
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
                            ::com::sun::star::uno::Sequence< sal_Int8 > aReadSeq;

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
                    ::rtl::OUString sVal(m_aValue.m_pString);
                    aSeq = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(sVal.getStr()),sizeof(sal_Unicode)*sVal.getLength());
                }
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                aSeq = *static_cast< Sequence<sal_Int8>*>(m_aValue.m_pValue);
                break;
            default:
                {
                    Any aValue = getAny();
                    aValue >>= aSeq;
                    break;
                }
        }
    }
    return aSeq;

}
// -----------------------------------------------------------------------------
::com::sun::star::util::Date ORowSetValue::getDate() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getDate" );
    ::com::sun::star::util::Date aValue;
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
                aValue = DBTypeConversion::toDate((double)*this);
                break;

            case DataType::DATE:
                aValue = *static_cast< ::com::sun::star::util::Date*>(m_aValue.m_pValue);
                break;
            case DataType::TIMESTAMP:
                {
                    ::com::sun::star::util::DateTime* pDateTime = static_cast< ::com::sun::star::util::DateTime*>(m_aValue.m_pValue);
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
                // NO break!

            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::TIME:
                aValue = DBTypeConversion::toDate( (double)0 );
                break;
        }
    }
    return aValue;
}
// -----------------------------------------------------------------------------
::com::sun::star::util::Time ORowSetValue::getTime()        const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getTime" );
    ::com::sun::star::util::Time aValue;
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
                aValue = DBTypeConversion::toTime((double)*this);
                break;
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::REAL:
                aValue = DBTypeConversion::toTime((double)*this);
                break;
            case DataType::TIMESTAMP:
                {
                    ::com::sun::star::util::DateTime* pDateTime = static_cast< ::com::sun::star::util::DateTime*>(m_aValue.m_pValue);
                    aValue.HundredthSeconds = pDateTime->HundredthSeconds;
                    aValue.Seconds          = pDateTime->Seconds;
                    aValue.Minutes          = pDateTime->Minutes;
                    aValue.Hours            = pDateTime->Hours;
                }
                break;
            case DataType::TIME:
                aValue = *static_cast< ::com::sun::star::util::Time*>(m_aValue.m_pValue);
                break;
            default:
                {
                    Any aAnyValue = getAny();
                    aAnyValue >>= aValue;
                    break;
                }
        }
    }
    return aValue;
}
// -----------------------------------------------------------------------------
::com::sun::star::util::DateTime ORowSetValue::getDateTime()    const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::getDateTime" );
    ::com::sun::star::util::DateTime aValue;
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
                aValue = DBTypeConversion::toDateTime((double)*this);
                break;
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::REAL:
                aValue = DBTypeConversion::toDateTime((double)*this);
                break;
            case DataType::DATE:
                {
                    ::com::sun::star::util::Date* pDate = static_cast< ::com::sun::star::util::Date*>(m_aValue.m_pValue);
                    aValue.Day      = pDate->Day;
                    aValue.Month    = pDate->Month;
                    aValue.Year     = pDate->Year;
                }
                break;
            case DataType::TIME:
                {
                    ::com::sun::star::util::Time* pTime = static_cast< ::com::sun::star::util::Time*>(m_aValue.m_pValue);
                    aValue.HundredthSeconds = pTime->HundredthSeconds;
                    aValue.Seconds          = pTime->Seconds;
                    aValue.Minutes          = pTime->Minutes;
                    aValue.Hours            = pTime->Hours;
                }
                break;
            case DataType::TIMESTAMP:
                aValue = *static_cast< ::com::sun::star::util::DateTime*>(m_aValue.m_pValue);
                break;
            default:
                {
                    Any aAnyValue = getAny();
                    aAnyValue >>= aValue;
                    break;
                }
        }
    }
    return aValue;
}
// -----------------------------------------------------------------------------
void ORowSetValue::setSigned(sal_Bool _bMod)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::setSigned" );
    if ( m_bSigned != _bMod )
    {
        m_bSigned = _bMod;
        if ( !m_bNull )
        {
            sal_Int32 nType = m_eTypeKind;
            switch(m_eTypeKind)
            {
                case DataType::BIGINT:
                    if ( m_bSigned ) // now we are signed, so we were unsigned and need to call getString()
                    {
                        m_bSigned = !m_bSigned;
                        const ::rtl::OUString sValue = getString();
                        free();
                        m_bSigned = !m_bSigned;
                        (*this) = sValue;
                    }
                    else
                    {
                        m_bSigned = !m_bSigned;
                        const sal_Int64 nValue = getLong();
                        free();
                        m_bSigned = !m_bSigned;
                        (*this) = nValue;
                    }
                    break;
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
            }
            m_eTypeKind = nType;
        }
    }
}

// -----------------------------------------------------------------------------
namespace detail
{
    class SAL_NO_VTABLE IValueSource
    {
    public:
        virtual ::rtl::OUString             getString() const = 0;
        virtual sal_Bool                    getBoolean() const = 0;
        virtual sal_Int8                    getByte() const = 0;
        virtual sal_Int16                   getShort() const = 0;
        virtual sal_Int32                   getInt() const = 0;
        virtual sal_Int64                   getLong() const = 0;
        virtual float                       getFloat() const = 0;
        virtual double                      getDouble() const = 0;
        virtual Date                        getDate() const = 0;
        virtual Time                        getTime() const = 0;
        virtual DateTime                    getTimestamp() const = 0;
        virtual Sequence< sal_Int8 >        getBytes() const = 0;
        virtual Reference< XInputStream >   getBinaryStream() const = 0;
        virtual Reference< XInputStream >   getCharacterStream() const = 0;
        virtual Reference< XBlob >          getBlob() const = 0;
        virtual Reference< XClob >          getClob() const = 0;
        virtual Any                         getObject() const = 0;
        virtual sal_Bool                    wasNull() const = 0;

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
        virtual ::rtl::OUString             getString() const           { return m_xRow->getString( m_nPos ); };
        virtual sal_Bool                    getBoolean() const          { return m_xRow->getBoolean( m_nPos ); };
        virtual sal_Int8                    getByte() const             { return m_xRow->getByte( m_nPos ); };
        virtual sal_Int16                   getShort() const            { return m_xRow->getShort( m_nPos ); }
        virtual sal_Int32                   getInt() const              { return m_xRow->getInt( m_nPos ); }
        virtual sal_Int64                   getLong() const             { return m_xRow->getLong( m_nPos ); }
        virtual float                       getFloat() const            { return m_xRow->getFloat( m_nPos ); };
        virtual double                      getDouble() const           { return m_xRow->getDouble( m_nPos ); };
        virtual Date                        getDate() const             { return m_xRow->getDate( m_nPos ); };
        virtual Time                        getTime() const             { return m_xRow->getTime( m_nPos ); };
        virtual DateTime                    getTimestamp() const        { return m_xRow->getTimestamp( m_nPos ); };
        virtual Sequence< sal_Int8 >        getBytes() const            { return m_xRow->getBytes( m_nPos ); };
        virtual Reference< XInputStream >   getBinaryStream() const     { return m_xRow->getBinaryStream( m_nPos ); };
        virtual Reference< XInputStream >   getCharacterStream() const  { return m_xRow->getCharacterStream( m_nPos ); };
        virtual Reference< XBlob >          getBlob() const             { return m_xRow->getBlob( m_nPos ); };
        virtual Reference< XClob >          getClob() const             { return m_xRow->getClob( m_nPos ); };
        virtual Any                         getObject() const           { return m_xRow->getObject( m_nPos ,NULL); };
        virtual sal_Bool                    wasNull() const             { return m_xRow->wasNull( ); };

    private:
        const Reference< XRow > m_xRow;
        const sal_Int32         m_nPos;
    };

    class ColumnValue : public IValueSource
    {
    public:
        ColumnValue( const Reference< XColumn >& _rxColumn )
            :m_xColumn( _rxColumn )
        {
        }

        // IValueSource
        virtual ::rtl::OUString             getString() const           { return m_xColumn->getString(); };
        virtual sal_Bool                    getBoolean() const          { return m_xColumn->getBoolean(); };
        virtual sal_Int8                    getByte() const             { return m_xColumn->getByte(); };
        virtual sal_Int16                   getShort() const            { return m_xColumn->getShort(); }
        virtual sal_Int32                   getInt() const              { return m_xColumn->getInt(); }
        virtual sal_Int64                   getLong() const             { return m_xColumn->getLong(); }
        virtual float                       getFloat() const            { return m_xColumn->getFloat(); };
        virtual double                      getDouble() const           { return m_xColumn->getDouble(); };
        virtual Date                        getDate() const             { return m_xColumn->getDate(); };
        virtual Time                        getTime() const             { return m_xColumn->getTime(); };
        virtual DateTime                    getTimestamp() const        { return m_xColumn->getTimestamp(); };
        virtual Sequence< sal_Int8 >        getBytes() const            { return m_xColumn->getBytes(); };
        virtual Reference< XInputStream >   getBinaryStream() const     { return m_xColumn->getBinaryStream(); };
        virtual Reference< XInputStream >   getCharacterStream() const  { return m_xColumn->getCharacterStream(); };
        virtual Reference< XBlob >          getBlob() const             { return m_xColumn->getBlob(); };
        virtual Reference< XClob >          getClob() const             { return m_xColumn->getClob(); };
        virtual Any                         getObject() const           { return m_xColumn->getObject( NULL ); };
        virtual sal_Bool                    wasNull() const             { return m_xColumn->wasNull( ); };

    private:
        const Reference< XColumn >  m_xColumn;
    };
}

// -----------------------------------------------------------------------------
void ORowSetValue::fill( const sal_Int32 _nType, const Reference< XColumn >& _rxColumn )
{
    detail::ColumnValue aColumnValue( _rxColumn );
    impl_fill( _nType, sal_True, aColumnValue );
}

// -----------------------------------------------------------------------------
void ORowSetValue::fill( sal_Int32 _nPos, sal_Int32 _nType, sal_Bool  _bNullable, const Reference< XRow>& _xRow )
{
    detail::RowValue aRowValue( _xRow, _nPos );
    impl_fill( _nType, _bNullable, aRowValue );
}

// -----------------------------------------------------------------------------
void ORowSetValue::fill(sal_Int32 _nPos,
                     sal_Int32 _nType,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow>& _xRow)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::fill (1)" );
    fill(_nPos,_nType,sal_True,_xRow);
}

// -----------------------------------------------------------------------------
void ORowSetValue::impl_fill( const sal_Int32 _nType, sal_Bool _bNullable, const detail::IValueSource& _rValueSource )

{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::fill (2)" );
    sal_Bool bReadData = sal_True;
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
            (*this) = _rValueSource.getString();
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
        (*this) = ::com::sun::star::uno::makeAny(_rValueSource.getClob());
        setTypeKind(DataType::CLOB);
        break;
    case DataType::BLOB:
        (*this) = ::com::sun::star::uno::makeAny(_rValueSource.getBlob());
        setTypeKind(DataType::BLOB);
        break;
    case DataType::OTHER:
        (*this) = _rValueSource.getObject();
        setTypeKind(DataType::OTHER);
        break;
    default:
        OSL_FAIL( "ORowSetValue::fill: unsupported type!" );
        (*this) = _rValueSource.getObject();
        break;
    }
    if ( bReadData && _bNullable && _rValueSource.wasNull() )
        setNull();
    setTypeKind(_nType);
}
// -----------------------------------------------------------------------------
void ORowSetValue::fill(const Any& _rValue)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbtools", "Ocke.Janssen@sun.com", "ORowSetValue::fill (3)" );
    switch (_rValue.getValueType().getTypeClass())
    {
        case TypeClass_VOID:
            setNull();
            break;
        case TypeClass_BOOLEAN:
        {
            sal_Bool bValue( sal_False );
            _rValue >>= bValue;
            (*this) = bValue;
            break;
        }
        case TypeClass_CHAR:
        {
            sal_Unicode aDummy(0);
            _rValue >>= aDummy;
            (*this) = ::rtl::OUString(aDummy);
            break;
        }
        case TypeClass_STRING:
        {
            ::rtl::OUString sDummy;
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
        case TypeClass_LONG:
        {
            sal_Int32 aDummy(0);
            _rValue >>= aDummy;
            (*this) = aDummy;
            break;
        }
        case TypeClass_UNSIGNED_SHORT:
        {
            sal_uInt16 nValue(0);
            _rValue >>= nValue;
            (*this) = static_cast<sal_Int32>(nValue);
            setSigned(sal_False);
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
            (*this) = static_cast<sal_Int64>(nValue);
            setSigned(sal_False);
            break;
        }
        case TypeClass_UNSIGNED_LONG:
        {
            sal_uInt32 nValue(0);
            _rValue >>= nValue;
            (*this) = static_cast<sal_Int64>(nValue);
            setSigned(sal_False);
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
                OSL_FAIL( "ORowSetValue::fill: unsupported sequence type!" );
            break;
        }

        case TypeClass_STRUCT:
        {
            ::com::sun::star::util::Date aDate;
            ::com::sun::star::util::Time aTime;
            ::com::sun::star::util::DateTime aDateTime;
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
                OSL_FAIL( "ORowSetValue::fill: unsupported structure!" );

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
            OSL_FAIL("Unknown type");
            break;
    }
}

}   // namespace connectivity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
