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
#include "precompiled_xmlhelp.hxx"

#include "db.hxx"

#include <rtl/alloc.h>
#include <cstring>

#include "com/sun/star/io/XSeekable.hpp"

#include "osl/file.hxx"
#include "osl/thread.hxx"
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;

namespace berkeleydbproxy {

//----------------------------------------------------------------------------
namespace db_internal
{
    static inline int check_error(int dberr, const char * where)
    {
        (void)where;
        return dberr;
    }
}

void DBData::copyToBuffer( const char* pSrcData, int nSize )
{
    m_nSize = nSize;
    delete [] m_pBuffer;
    m_pBuffer = new char[m_nSize+1];
    memcpy( m_pBuffer, pSrcData, m_nSize );
    m_pBuffer[m_nSize] = 0;
}


// DBHelp

bool DBHelp::implReadLenAndData( const char* pData, int& riPos, DBData& rValue )
{
    bool bSuccess = false;

    // Read key len
    const char* pStartPtr = pData + riPos;
    char* pEndPtr;
    sal_Int32 nKeyLen = strtol( pStartPtr, &pEndPtr, 16 );
    if( pEndPtr == pStartPtr )
        return bSuccess;
    riPos += (pEndPtr - pStartPtr) + 1;

    const char* pKeySrc = pData + riPos;
    rValue.copyToBuffer( pKeySrc, nKeyLen );
    riPos += nKeyLen + 1;

    bSuccess = true;
    return bSuccess;
}

void DBHelp::createHashMap( bool bOptimizeForPerformance )
{
    releaseHashMap();
    if( bOptimizeForPerformance )
    {
        if( m_pStringToDataMap != NULL )
            return;
        m_pStringToDataMap = new StringToDataMap();
    }
    else
    {
        if( m_pStringToValPosMap != NULL )
            return;
        m_pStringToValPosMap = new StringToValPosMap();
    }

    Reference< XInputStream > xIn = m_xSFA->openFileRead( m_aFileURL );
    if( xIn.is() )
    {
        Sequence< sal_Int8 > aData;
        sal_Int32 nSize = m_xSFA->getSize( m_aFileURL );
        sal_Int32 nRead = xIn->readBytes( aData, nSize );

        const char* pData = (const char*)aData.getConstArray();
        int iPos = 0;
        while( iPos < nRead )
        {
            DBData aDBKey;
            if( !implReadLenAndData( pData, iPos, aDBKey ) )
                break;

            rtl::OString aOKeyStr = aDBKey.getData();

            // Read val len
            const char* pStartPtr = pData + iPos;
            char* pEndPtr;
            sal_Int32 nValLen = strtol( pStartPtr, &pEndPtr, 16 );
            if( pEndPtr == pStartPtr )
                break;

            iPos += (pEndPtr - pStartPtr) + 1;

            if( bOptimizeForPerformance )
            {
                const char* pValSrc = pData + iPos;
                rtl::OString aValStr( pValSrc, nValLen );
                (*m_pStringToDataMap)[aOKeyStr] = aValStr;
            }
            else
            {
                // store value start position
                (*m_pStringToValPosMap)[aOKeyStr] = std::pair<int,int>( iPos, nValLen );
            }
            iPos += nValLen + 1;
        }

        xIn->closeInput();
    }
}

void DBHelp::releaseHashMap( void )
{
    if( m_pStringToDataMap != NULL )
    {
        delete m_pStringToDataMap;
        m_pStringToDataMap = NULL;
    }
    if( m_pStringToValPosMap != NULL )
    {
        delete m_pStringToValPosMap;
        m_pStringToValPosMap = NULL;
    }
}


bool DBHelp::getValueForKey( const rtl::OString& rKey, DBData& rValue )
{
    bool bSuccess = false;
    if( !m_xSFA.is() )
        return bSuccess;

    try
    {

    if( m_pStringToDataMap == NULL && m_pStringToValPosMap == NULL )
    {
        bool bOptimizeForPerformance = false;
        createHashMap( bOptimizeForPerformance );
    }

    if( m_pStringToValPosMap != NULL )
    {
        StringToValPosMap::const_iterator it = m_pStringToValPosMap->find( rKey );
        if( it != m_pStringToValPosMap->end() )
        {
            const std::pair<int,int>& rValPair = it->second;
            int iValuePos = rValPair.first;
            int nValueLen = rValPair.second;

            Reference< XInputStream > xIn = m_xSFA->openFileRead( m_aFileURL );
            if( xIn.is() )
            {
                Reference< XSeekable > xXSeekable( xIn, UNO_QUERY );
                if( xXSeekable.is() )
                {
                    xXSeekable->seek( iValuePos );

                    Sequence< sal_Int8 > aData;
                    sal_Int32 nRead = xIn->readBytes( aData, nValueLen );
                    if( nRead == nValueLen )
                    {
                        const char* pData = (const sal_Char*)aData.getConstArray();
                        rValue.copyToBuffer( pData, nValueLen );
                        bSuccess = true;
                    }
                }
                xIn->closeInput();
            }
        }
    }

    else if( m_pStringToDataMap != NULL )
    {
        StringToDataMap::const_iterator it = m_pStringToDataMap->find( rKey );
        if( it != m_pStringToDataMap->end() )
        {
            const rtl::OString& rValueStr = it->second;
            int nValueLen = rValueStr.getLength();
            const char* pData = rValueStr.getStr();
            rValue.copyToBuffer( pData, nValueLen );
            bSuccess = true;
        }
    }

    }
    catch( Exception & )
    {
        bSuccess = false;
    }

    return bSuccess;
}

bool DBHelp::startIteration( void )
{
    bool bSuccess = false;

    sal_Int32 nSize = m_xSFA->getSize( m_aFileURL );

    Reference< XInputStream > xIn = m_xSFA->openFileRead( m_aFileURL );
    if( xIn.is() )
    {
        m_nItRead = xIn->readBytes( m_aItData, nSize );
        if( m_nItRead == nSize )
        {
            bSuccess = true;
            m_pItData = (const char*)m_aItData.getConstArray();
            m_iItPos = 0;
        }
        else
        {
            stopIteration();
        }
    }

    return bSuccess;
}

bool DBHelp::getNextKeyAndValue( DBData& rKey, DBData& rValue )
{
    bool bSuccess = false;

    if( m_iItPos < m_nItRead )
    {
        if( implReadLenAndData( m_pItData, m_iItPos, rKey ) )
        {
            if( implReadLenAndData( m_pItData, m_iItPos, rValue ) )
                bSuccess = true;
        }
    }

    return bSuccess;
}

void DBHelp::stopIteration( void )
{
    m_aItData = Sequence<sal_Int8>();
    m_pItData = NULL;
    m_nItRead = -1;
    m_iItPos = -1;
}


Db::Db()
{
    db_internal::check_error( db_create(&m_pDBP,0,0),"Db::Db" );
    m_pDBHelp = NULL;
}


Db::~Db()
{
    if (m_pDBP)
    {
        // should not happen
        // TODO: add assert
    }

    delete m_pDBHelp;
}


int Db::close(u_int32_t flags)
{
    int error = m_pDBP->close(m_pDBP,flags);
    m_pDBP = 0;
    return db_internal::check_error(error,"Db::close");
}

int Db::open(DB_TXN *txnid,
             const char *file,
             const char *database,
             DBTYPE type,
             u_int32_t flags,
             int mode)
{
    int err = m_pDBP->open(m_pDBP,txnid,file,database,type,flags,mode);
    return db_internal::check_error( err,"Db::open" );
}

int Db::open(DB_TXN *txnid,
             ::rtl::OUString const & fileURL,
             DBTYPE type,
             u_int32_t flags,
             int mode)
{
    ::rtl::OUString ouPath;
    ::osl::FileBase::getSystemPathFromFileURL(fileURL, ouPath);
    const ::rtl::OString sPath = ::rtl::OUStringToOString(ouPath, osl_getThreadTextEncoding());
    return open(txnid, sPath.getStr(), 0, type, flags, mode);
}



int Db::get(DB_TXN *txnid, Dbt *key, Dbt *data, u_int32_t flags)
{
    int err = m_pDBP->get(m_pDBP,txnid,key,data,flags);

    // these are non-exceptional outcomes
    if (err != DB_NOTFOUND && err != DB_KEYEMPTY)
        db_internal::check_error( err,"Db::get" );

    return err;
}

int Db::cursor(DB_TXN *txnid, Dbc **cursorp, u_int32_t flags)
{
    DBC * dbc = 0;
    int error = m_pDBP->cursor(m_pDBP,txnid,&dbc,flags);

    if (!db_internal::check_error(error,"Db::cursor"))
        *cursorp = new Dbc(dbc);

    return error;
}

//----------------------------------------------------------------------------

Dbc::Dbc(DBC * dbc)
: m_pDBC(dbc)
{
}

Dbc::~Dbc()
{
}

int Dbc::close()
{
    int err = m_pDBC->c_close(m_pDBC);
    delete this;
    return db_internal::check_error( err,"Dbcursor::close" );
}

int Dbc::get(Dbt *key, Dbt *data, u_int32_t flags)
{
    int err = m_pDBC->c_get(m_pDBC,key,data,flags);

    // these are non-exceptional outcomes
    if (err != DB_NOTFOUND && err != DB_KEYEMPTY)
        db_internal::check_error( err, "Dbcursor::get" );

    return err;
}

//----------------------------------------------------------------------------


Dbt::Dbt()
{
    using namespace std;
    DBT * thispod = this;
    memset(thispod, 0, sizeof *thispod);
}


Dbt::Dbt(void *data_arg, u_int32_t size_arg)
{
    using namespace std;
    DBT * thispod = this;
    memset(thispod, 0, sizeof *thispod);
    this->set_data(data_arg);
    this->set_size(size_arg);
}

Dbt::~Dbt()
{
}

void * Dbt::get_data() const
{
    return this->data;
}

void Dbt::set_data(void *value)
{
    this->data = value;
}

u_int32_t Dbt::get_size() const
{
    return this->size;
}

void Dbt::set_size(u_int32_t value)
{
    this->size = value;
}

void Dbt::set_flags(u_int32_t value)
{
    this->flags = value;
}

} // namespace ecomp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
