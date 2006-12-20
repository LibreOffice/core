/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_persmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:20:55 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

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
    OSL_ENSURE( 0, rtl::OUStringToOString(
                    msg_, RTL_TEXTENCODING_UTF8 ).getStr() );
    throw RuntimeException( msg_, Reference<XInterface>() );
}

//______________________________________________________________________________
PersistentMap::~PersistentMap()
{
    try {
        m_db.close(0);
    }
    catch (DbException & exc) {
        (void) exc; // avoid warnings
        OSL_ENSURE( 0, DbEnv::strerror( exc.get_errno() ) );
    }
}

//______________________________________________________________________________
void PersistentMap::flush() const
{
    try {
        int err = m_db.sync(0);
        if (err != 0)
            throw_rtexc(err);
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
}

//______________________________________________________________________________
PersistentMap::PersistentMap( OUString const & url_, bool readOnly )
    : m_db( 0, 0 )
{
    try {
        OUString url( expandUnoRcUrl(url_) );
        if ( File::getSystemPathFromFileURL( url, m_sysPath ) != File::E_None )
        {
            OSL_ASSERT( false );
        }
        OString cstr_sysPath(
            OUStringToOString( m_sysPath, osl_getThreadTextEncoding() ) );
        char const * pcstr_sysPath = cstr_sysPath.getStr();

        u_int32_t flags = DB_CREATE;
        if (readOnly) {
            flags = DB_RDONLY;
            if (! create_ucb_content(
                    0, url,
                    Reference<com::sun::star::ucb::XCommandEnvironment>(),
                    false /* no throw */ )) {
                // ignore non-existent file in read-only mode: simulate empty db
                pcstr_sysPath = 0;
                flags = DB_CREATE;
            }
        }

        int err = m_db.open(
            // xxx todo: DB_THREAD, DB_DBT_MALLOC currently not used
            0, pcstr_sysPath, 0, DB_HASH, flags/* | DB_THREAD*/, 0664 /* fs mode */ );
        if (err != 0)
            throw_rtexc(err);
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
}

//______________________________________________________________________________
PersistentMap::PersistentMap()
    : m_db( 0, 0 )
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
bool PersistentMap::has( OUString const & key ) const
{
    return get( 0, key );
}

//______________________________________________________________________________
bool PersistentMap::get( OUString * value, OUString const & key_ ) const
{
    try {
        OString key( OUStringToOString( key_, RTL_TEXTENCODING_UTF8 ) );
        Dbt dbKey( const_cast< sal_Char * >(key.getStr()), key.getLength() );
        Dbt dbData;
        int err = m_db.get( 0, &dbKey, &dbData, 0 );
        if (err == DB_NOTFOUND)
            return false;
        if (err == 0) {
            if (value != 0) {
                *value = OUString(
                    static_cast< sal_Char const * >(dbData.get_data()),
                    dbData.get_size(), RTL_TEXTENCODING_UTF8 );
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
bool PersistentMap::put(
    OUString const & key_, OUString & value_, bool overwrite,
    bool flush_immediately )
{
    try {
        OString key( OUStringToOString( key_, RTL_TEXTENCODING_UTF8 ) );
        Dbt dbKey( const_cast< sal_Char * >(key.getStr()), key.getLength() );
        OString value( OUStringToOString( value_, RTL_TEXTENCODING_UTF8 ) );
        Dbt dbData( const_cast< sal_Char * >(
                        value.getStr()), value.getLength() );
        int err = m_db.put(
            0, &dbKey, &dbData, overwrite ? 0 : DB_NOOVERWRITE );
        if (err == 0) {
#if OSL_DEBUG_LEVEL > 0
            OUString v;
            OSL_ASSERT( get( &v, key_ ) );
            OSL_ASSERT( v.equals( value_ ) );
#endif
            if (flush_immediately) {
                err = m_db.sync(0);
                if (err != 0)
                    throw_rtexc(err);
            }
            return true;
        }
        if (err == DB_KEYEXIST) {
            OSL_ASSERT( ! overwrite );
            err = m_db.get( 0, &dbKey, &dbData, 0 );
            OSL_ASSERT( err == 0 );
            if (err == 0)
            {
                value_ = OUString(
                    static_cast< sal_Char const * >(dbData.get_data()),
                    dbData.get_size(), RTL_TEXTENCODING_UTF8 );
            }
            return false;
        }
        throw_rtexc(err);
    }
    catch (DbException & exc) {
        throw_rtexc( exc.get_errno(), exc.what() );
    }
    return false; // avoiding warning
}

//______________________________________________________________________________
bool PersistentMap::erase( OUString const & key_, bool flush_immediately )
{
    try {
        OString key( OUStringToOString( key_, RTL_TEXTENCODING_UTF8 ) );
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

            ::std::pair<t_string2string_map::iterator, bool > insertion(
                ret.insert( t_string2string_map::value_type(
                                t_string2string_map::value_type(
                                    OUString( static_cast< sal_Char const * >(
                                                  dbKey.get_data()),
                                              dbKey.get_size(),
                                              RTL_TEXTENCODING_UTF8 ),
                                    OUString( static_cast< sal_Char const * >(
                                                  dbData.get_data()),
                                              dbData.get_size(),
                                              RTL_TEXTENCODING_UTF8 ) ) ) ) );
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

}

