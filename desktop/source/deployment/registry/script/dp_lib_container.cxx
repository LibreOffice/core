/*************************************************************************
 *
 *  $RCSfile: dp_lib_container.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:11:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

//______________________________________________________________________________
OUString LibraryContainer::get_libname(
    OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    Reference<XComponentContext> const & xContext )
{
    ::xmlscript::LibDescriptor import;
    ::ucb::Content ucb_content( url, xCmdEnv );
    xml_parse( ::xmlscript::importLibrary( import ), ucb_content, xContext );

    if (import.aName.getLength() == 0)
        throw Exception( getResourceString(RID_STR_CANNOT_DETERMINE_LIBNAME),
                         Reference<XInterface>() );
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
void LibraryContainer::verify_init(
    Reference<XCommandEnvironment> const & xCmdEnv ) const
{
    ::osl::MutexGuard guard( m_mutex );
    if (! m_inited)
    {
        ::ucb::Content ucb_content;
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
                ::std::pair< t_libs_map::iterator, bool > insertion(
                    m_map.insert(
                        t_libs_map::value_type( descr.aName, descr ) ) );
                OSL_ASSERT( insertion.second );
            }
        }
        m_modified = false;
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
    verify_init( xCmdEnv );

    ::osl::MutexGuard guard( m_mutex );
    t_libs_map::const_iterator const iFind( m_map.find( descr.aName ) );
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
        if (! insert_url.equals( storage_url ))
            throw container::ElementExistException(
                getResourceString(RID_STR_LIBNAME_ALREADY_EXISTS) + descr.aName,
                Reference<XInterface>() );
        return true;
    }
}

//______________________________________________________________________________
bool LibraryContainer::remove(
    OUString const & libname, OUString const & url,
    Reference< XCommandEnvironment > const & xCmdEnv,
    bool exact )
{
    bool succ = false;
    ::osl::MutexGuard guard( m_mutex );
    verify_init( xCmdEnv );
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
            bool remove = false;
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
    verify_init( xCmdEnv );
    return m_map.find( libname ) != m_map.end();
}

//______________________________________________________________________________
t_descr_list LibraryContainer::getLibs(
    Reference< XCommandEnvironment > const & xCmdEnv ) const
{
    ::osl::MutexGuard guard( m_mutex );
    verify_init( xCmdEnv );
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

