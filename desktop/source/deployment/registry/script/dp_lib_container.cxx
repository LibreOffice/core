/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_lib_container.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_desktop.hxx"

#include "dp_script.hrc"
#include "dp_misc.h"
#include "dp_resource.h"
#include "dp_ucb.h"
#include "dp_interact.h"
#include "dp_xml.h"
#include "dp_lib_container.h"
#include "rtl/ustrbuf.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/ucb/XSimpleFileAccess.hpp"
#include "com/sun/star/io/XActiveDataSource.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/deployment/DeploymentException.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace dp_registry {
namespace backend {
namespace script {

namespace {
struct StrCannotDetermineLibName : public StaticResourceString<
    StrCannotDetermineLibName, RID_STR_CANNOT_DETERMINE_LIBNAME> {};
struct StrLibNameAlreadyExists : public StaticResourceString<
    StrLibNameAlreadyExists, RID_STR_LIBNAME_ALREADY_EXISTS> {};
}

//______________________________________________________________________________
OUString LibraryContainer::get_libname(
    OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    Reference<XComponentContext> const & xContext )
{
    ::xmlscript::LibDescriptor import;
    ::ucbhelper::Content ucb_content( url, xCmdEnv );
    xml_parse( ::xmlscript::importLibrary( import ), ucb_content, xContext );

    if (import.aName.getLength() == 0) {
        throw Exception( StrCannotDetermineLibName::get(),
                         Reference<XInterface>() );
    }
    return import.aName;
}

//______________________________________________________________________________
void LibraryContainer::flush(
    Reference<XCommandEnvironment> const & xCmdEnv ) const
{
    ::osl::MutexGuard guard( m_mutex );
    if (!m_inited || !m_modified || m_container_url.getLength() == 0)
        return;

    ::xmlscript::LibDescriptorArray export_array( m_map.size() );
    t_libs_map::iterator iPos( m_map.begin() );
    t_libs_map::iterator const iEnd( m_map.end() );
    ::std::size_t nPos = 0;
    for ( ; iPos != iEnd; ++iPos ) {
        export_array.mpLibs[ nPos ] = iPos->second;
        ++nPos;
    }

    // raise sax writer:
    Reference< xml::sax::XExtendedDocumentHandler > xHandler(
        m_xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.xml.sax.Writer"), m_xContext ),
        UNO_QUERY_THROW );

    // try to erase .xlc file:
    erase_path( m_container_url, xCmdEnv );
    // create new one:
    Reference<io::XActiveDataSource> xSource( xHandler, UNO_QUERY_THROW );

    Reference<XSimpleFileAccess> xSimpleFileAccess(
        m_xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.ucb.SimpleFileAccess"), m_xContext ),
        UNO_QUERY_THROW );
    xSource->setOutputStream(
        xSimpleFileAccess->openFileWrite( m_container_url ) );

    ::xmlscript::exportLibraryContainer( xHandler, &export_array );
    m_modified = false;
}

//______________________________________________________________________________
void LibraryContainer::init(
    Reference<XCommandEnvironment> const & xCmdEnv ) const
{
    ::osl::MutexGuard guard( m_mutex );
    if (! m_inited)
    {
        m_modified = false;
        ::ucbhelper::Content ucb_content;
        if (create_ucb_content( &ucb_content, m_container_url,
                                xCmdEnv, false /* no throw */ ))
        {
            ::xmlscript::LibDescriptorArray import_array;
            xml_parse( ::xmlscript::importLibraryContainer( &import_array ),
                       ucb_content, m_xContext );
            for ( sal_Int32 nPos = import_array.mnLibCount; nPos--; )
            {
                ::xmlscript::LibDescriptor const & descr =
                      import_array.mpLibs[ nPos ];
                // filter dead package entries in xlc file:
                if (descr.aStorageURL.matchAsciiL(
                        RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.expand:") ) &&
                    !create_ucb_content( 0, descr.aStorageURL, xCmdEnv,
                                         false /* no throw */ ))
                {
                    OSL_ENSURE( 0, ::rtl::OUStringToOString(
                                    descr.aStorageURL,
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                    m_modified = true; // force write back
                    continue;
                }
                ::std::pair<t_libs_map::iterator, bool> insertion(
                    m_map.insert(
                        t_libs_map::value_type( descr.aName, descr ) ) );
                OSL_ASSERT( insertion.second );
                (void) insertion;
            }
        }
        m_inited = true;
    }
}

//______________________________________________________________________________
bool LibraryContainer::insert(
    OUString const & libname, OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (libname.getLength() == 0)
        return false;

    // opt: expand here only once
    OUString const & insert_url = url;
    //( path_normalize( m_env.expand_reg_url( url ) ) );

    ::xmlscript::LibDescriptor descr;
    descr.aName = libname;
    descr.aStorageURL = url;
    descr.bLink = true;
    descr.bReadOnly = false;
    descr.bPasswordProtected = false;
    descr.bPreload = false;
    // all other fields are ignored by xmlscript lib export
    init( xCmdEnv );

    ::osl::MutexGuard guard( m_mutex );
    t_libs_map::iterator const iFind( m_map.find( descr.aName ) );
    if (m_map.end() == iFind)
    {
        ::std::pair< t_libs_map::iterator, bool > insertion(
            m_map.insert( t_libs_map::value_type( descr.aName, descr ) ) );
        OSL_ASSERT( insertion.second );
        m_modified = insertion.second;
        if (m_immediateFlush && m_modified)
            flush( xCmdEnv );
        return insertion.second;
    }
    else
    {
        // found one:
        OUString const & storage_url = iFind->second.aStorageURL;
        if (! insert_url.equals( storage_url )) {
            throw container::ElementExistException(
                StrLibNameAlreadyExists::get() + descr.aName,
                Reference<XInterface>() );
        }
        return true;
    }
}

//______________________________________________________________________________
bool LibraryContainer::remove(
    OUString const & libname, OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    bool exact )
{
    bool succ = false;
    ::osl::MutexGuard guard( m_mutex );
    init( xCmdEnv );
    if (exact)
    {
        if (libname.getLength() != 0)
        {
            ::std::size_t erased = m_map.erase( libname );
            if (0 < erased) {
                m_modified = true;
                succ = true;
            }
        }
    }
    else
    {
        // erase some matching url:
        t_libs_map::iterator iPos( m_map.begin() );
        t_libs_map::iterator const iEnd( m_map.end() );
        while (iPos != iEnd)
        {
            if (iPos->second.aStorageURL.getLength() > 0)
            {
                OUString const & storage_url = iPos->second.aStorageURL;
                if (storage_url.match( url )) {
                    t_libs_map::iterator iErase( iPos );
                    ++iPos;
                    m_map.erase( iErase );
                    m_modified = true;
                    continue;
                }
            }
            ++iPos;
        }
        succ = true; // always successful
    }

    if (m_immediateFlush && m_modified)
        flush( xCmdEnv );
    return succ;
}

//______________________________________________________________________________
bool LibraryContainer::has(
    OUString const & libname,
    Reference< XCommandEnvironment > const & xCmdEnv ) const
{
    ::osl::MutexGuard guard( m_mutex );
    init( xCmdEnv );
    return m_map.find( libname ) != m_map.end();
}

//______________________________________________________________________________
t_descr_list LibraryContainer::getLibs(
    Reference<XCommandEnvironment> const & xCmdEnv ) const
{
    ::osl::MutexGuard guard( m_mutex );
    init( xCmdEnv );
    t_descr_list ret;
    t_libs_map::iterator iPos( m_map.begin() );
    t_libs_map::iterator const iEnd( m_map.end() );
    for ( ; iPos != iEnd; ++iPos )
        ret.push_back( iPos->second );
    return ret;
}

}
}
}

