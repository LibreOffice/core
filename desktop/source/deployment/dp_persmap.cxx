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


#include "dp_misc.h"
#include "dp_ucb.h"
#include "dp_persmap.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/file.hxx"
#include "osl/thread.h"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using ::osl::File;

namespace dp_misc
{

//______________________________________________________________________________
void PersistentMap::throw_rtexc( int err, char const * pmsg ) const
{
    OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[") );
    buf.append( m_sysPath );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] Berkeley Db error (") );
    buf.append( static_cast<sal_Int32>(err) );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("): ") );
    if (pmsg == 0)
        pmsg = DbEnv::strerror(err);
    const OString msg(pmsg);
    buf.append( OUString( msg.getStr(), msg.getLength(),
                          osl_getThreadTextEncoding() ) );
    const OUString msg_(buf.makeStringAndClear());
    OSL_FAIL( rtl::OUStringToOString(
                    msg_, RTL_TEXTENCODING_UTF8 ).getStr() );
    throw RuntimeException( msg_, uno::Reference<XInterface>() );
}

//______________________________________________________________________________
PersistentMap::~PersistentMap()
{
    try {
        m_db.close(0);
    }
    catch (DbException & exc) {
        (void) exc; // avoid warnings
        OSL_FAIL( DbEnv::strerror( exc.get_errno() ) );
    }
}

//______________________________________________________________________________
PersistentMap::PersistentMap( OUString const & url )
    : m_db( 0 )
{
    try
    {
        rtl::OUString fileURL = expandUnoRcUrl(url);
        if ( File::getSystemPathFromFileURL( fileURL, m_sysPath ) != File::E_None )
            OSL_ASSERT( false );

        OString cstr_sysPath(
            OUStringToOString( m_sysPath, RTL_TEXTENCODING_UTF8 ) );
        int err = m_db.open(
            // xxx todo: DB_THREAD, DB_DBT_MALLOC currently not used
            0, cstr_sysPath.getStr(), 0, DB_HASH,
            DB_CREATE/* | DB_THREAD*/, 0664 /* fs mode */ );
        if (err != 0)
            throw_rtexc(err);
    }
    catch (const DbException & exc)
    {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
}

//______________________________________________________________________________
PersistentMap::PersistentMap()
    : m_db( 0 )
{
    try {
        // xxx todo: DB_THREAD, DB_DBT_MALLOC currently not used
        int err = m_db.open( 0, 0, 0, DB_HASH, DB_CREATE/* | DB_THREAD*/, 0 );
        if (err != 0)
            throw_rtexc(err);
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
}

//______________________________________________________________________________
bool PersistentMap::has( OString const & key ) const
{
    return get( 0, key );
}

// for 3 functions here MSVC gives C4702 "unreachable code" if optimization
// is enabled and return is there and C4715 "not all control paths return
// a value" if optimization disabled and no return...
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4702 )
#endif

//______________________________________________________________________________
bool PersistentMap::get( OString * value, OString const & key ) const
{
    try {
        Dbt dbKey( const_cast< sal_Char * >(key.getStr()), key.getLength() );
        Dbt dbData;
        int err = m_db.get( 0, &dbKey, &dbData, 0 );
        if (err == DB_NOTFOUND)
            return false;
        if (err == 0) {
            if (value != 0) {
                *value = OString(
                    static_cast< sal_Char const * >(dbData.get_data()),
                    dbData.get_size() );
            }
            return true;
        }
        throw_rtexc(err);
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
    return false; // avoiding warning
}

//______________________________________________________________________________
void PersistentMap::put( OString const & key, OString const & value )
{
    try {
        Dbt dbKey( const_cast< sal_Char * >(key.getStr()), key.getLength() );
        Dbt dbData( const_cast< sal_Char * >(
                        value.getStr()), value.getLength() );
        int err = m_db.put( 0, &dbKey, &dbData, 0 );
        if (err == 0) {
#if OSL_DEBUG_LEVEL > 0
            OString v;
            OSL_ASSERT( get( &v, key ) );
            OSL_ASSERT( v.equals( value ) );
#endif
            err = m_db.sync(0);
        }
        if (err != 0)
            throw_rtexc(err);
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
}

//______________________________________________________________________________
bool PersistentMap::erase( OString const & key, bool flush_immediately )
{
    try {
        Dbt dbKey( const_cast< sal_Char * >(key.getStr()), key.getLength() );
        int err = m_db.del( &dbKey, 0 );
        if (err == 0) {
            if (flush_immediately) {
                err = m_db.sync(0);
                if (err != 0)
                    throw_rtexc(err);
            }
            return true;
        }
        if (err == DB_NOTFOUND)
            return false;
        throw_rtexc(err);
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
    return false; // avoiding warning
}

//______________________________________________________________________________
t_string2string_map PersistentMap::getEntries() const
{
    try {
        Dbc * pcurs = 0;
        int err = m_db.cursor( 0, &pcurs, 0 );
        if (err != 0)
            throw_rtexc(err);

        t_string2string_map ret;
        for (;;) {
            Dbt dbKey, dbData;
            err = pcurs->get( &dbKey, &dbData, DB_NEXT );
            if (err == DB_NOTFOUND)
                break;
            if (err != 0)
                throw_rtexc(err);

#if OSL_DEBUG_LEVEL > 0
            ::std::pair<t_string2string_map::iterator, bool> insertion =
#endif
            ret.insert(
                t_string2string_map::value_type(
                    OString( static_cast<sal_Char const*>(dbKey.get_data()), dbKey.get_size() ),
                    OString( static_cast<sal_Char const*>(dbData.get_data()), dbData.get_size() )
                ) );
            OSL_ASSERT( insertion.second );
        }
        err = pcurs->close();
        if (err != 0)
            throw_rtexc(err);
        return ret;
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
    return t_string2string_map(); // avoiding warning
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
