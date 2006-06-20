/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: directory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:27:14 $
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
/*
 * This file pinched from webdavdatasupplier (etc.)
 * cut & paste + new getData impl. & collate ResultSet code.
 */
#include <vector>
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _UCBHELPER_PROVIDERHELPER_HXX
#include <ucbhelper/providerhelper.hxx>
#endif

#include "directory.hxx"

#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-directory.h>

using namespace com::sun::star;
using namespace gvfs;

// DynamicResultSet Implementation.

DynamicResultSet::DynamicResultSet(
    const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
    const rtl::Reference< Content >& rxContent,
    const com::sun::star::ucb::OpenCommandArgument2& rCommand,
    const uno::Reference<com::sun::star::ucb::XCommandEnvironment >& rxEnv )
    : ResultSetImplHelper( rxSMgr, rCommand ),
      m_xContent( rxContent ),
      m_xEnv( rxEnv )
{
}
void DynamicResultSet::initStatic()
{
    m_xResultSet1
        = new ::ucb::ResultSet( m_xSMgr,
                    m_aCommand.Properties,
                    new DataSupplier( m_xSMgr,
                              m_xContent,
                              m_aCommand.Mode ),
                    m_xEnv );
}
void DynamicResultSet::initDynamic()
{
    initStatic();
    m_xResultSet2 = m_xResultSet1;
}

//=========================================================================


// DataSupplier Implementation.



struct ResultListEntry
{
    rtl::OUString                                             aId;
    uno::Reference< com::sun::star::ucb::XContentIdentifier > xId;
    uno::Reference< com::sun::star::ucb::XContent >           xContent;
    uno::Reference< sdbc::XRow >                              xRow;
    GnomeVFSFileInfo                                          aInfo;

    ResultListEntry( const GnomeVFSFileInfo *fileInfo)
    {
        gnome_vfs_file_info_copy (&aInfo, fileInfo);
    }

     ~ResultListEntry()
    {
        gnome_vfs_file_info_clear (&aInfo);
    }
};

//=========================================================================
//
// ResultList.
//
//=========================================================================

typedef std::vector< ResultListEntry* > ResultList;

//=========================================================================
//
// struct DataSupplier_Impl.
//
//=========================================================================

struct gvfs::DataSupplier_Impl
{
    osl::Mutex                                   m_aMutex;
    ResultList                                   m_aResults;
    rtl::Reference< Content >                    m_xContent;
    uno::Reference< lang::XMultiServiceFactory > m_xSMgr;
    sal_Int32                                    m_nOpenMode;
    sal_Bool                                     m_bCountFinal;

    DataSupplier_Impl(
              const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
              const rtl::Reference< Content >& rContent,
              sal_Int32 nOpenMode )
        : m_xContent( rContent ), m_xSMgr( rxSMgr ),
          m_nOpenMode( nOpenMode ), m_bCountFinal( sal_False ) {}
    ~DataSupplier_Impl()
    {
        ResultList::const_iterator it  = m_aResults.begin();
        ResultList::const_iterator end = m_aResults.end();

        while ( it != end )
            {
                delete (*it);
                it++;
            }
    }
};

DataSupplier::DataSupplier(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            const rtl::Reference< Content >& rContent,
            sal_Int32 nOpenMode )
: m_pImpl( new DataSupplier_Impl( rxSMgr, rContent, nOpenMode ) )
{
}

//=========================================================================
// virtual
DataSupplier::~DataSupplier()
{
    delete m_pImpl;
}

// virtual
rtl::OUString DataSupplier::queryContentIdentifierString( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) ) {
        rtl::OUString aId = m_pImpl->m_aResults[ nIndex ]->aId;
        if ( aId.getLength() ) // cached
            return aId;
    }

    if ( getResult( nIndex ) ) {
        rtl::OUString aId = m_pImpl->m_xContent->getOUURI();

        char *escaped_name;
        escaped_name = gnome_vfs_escape_string( m_pImpl->m_aResults[ nIndex ]->aInfo.name );

        if ( ( aId.lastIndexOf( '/' ) + 1 ) != aId.getLength() )
            aId += rtl::OUString::createFromAscii( "/" );

        aId += rtl::OUString::createFromAscii( escaped_name );

        g_free( escaped_name );

        m_pImpl->m_aResults[ nIndex ]->aId = aId;
        return aId;
    }

    return rtl::OUString();
}

// virtual
uno::Reference< com::sun::star::ucb::XContentIdentifier >
DataSupplier::queryContentIdentifier( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) ) {
        uno::Reference< com::sun::star::ucb::XContentIdentifier > xId
            = m_pImpl->m_aResults[ nIndex ]->xId;
        if ( xId.is() ) // Already cached.
            return xId;
    }

    rtl::OUString aId = queryContentIdentifierString( nIndex );
    if ( aId.getLength() ) {
        uno::Reference< com::sun::star::ucb::XContentIdentifier > xId
            = new ::ucb::ContentIdentifier( aId );
        m_pImpl->m_aResults[ nIndex ]->xId = xId;
        return xId;
    }

    return uno::Reference< com::sun::star::ucb::XContentIdentifier >();
}

// virtual
uno::Reference< com::sun::star::ucb::XContent >
DataSupplier::queryContent( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) ) {
        uno::Reference< com::sun::star::ucb::XContent > xContent
            = m_pImpl->m_aResults[ nIndex ]->xContent;
        if ( xContent.is() ) // Already cached.
            return xContent;
    }

    uno::Reference< com::sun::star::ucb::XContentIdentifier > xId
        = queryContentIdentifier( nIndex );
    if ( xId.is() ) {
        try
        {
            // FIXME:
            // It would be really nice to propagate this information
            // to the Content, but we can't then register it with the
            // ContentProvider, and the ucbhelper hinders here.
            uno::Reference< com::sun::star::ucb::XContent > xContent
                = m_pImpl->m_xContent->getProvider()->queryContent( xId );
            m_pImpl->m_aResults[ nIndex ]->xContent = xContent;
            return xContent;

        }
        catch ( com::sun::star::ucb::IllegalIdentifierException& ) {
        }
    }
    return uno::Reference< com::sun::star::ucb::XContent >();
}

// virtual
sal_Bool DataSupplier::getResult( sal_Int32 nIndex )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) > nIndex ) // Result already present.
        return sal_True;

    if ( getData() && sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) > nIndex )
        return sal_True;

    return sal_False;
}

// virtual
sal_Int32 DataSupplier::totalCount()
{
    getData();

    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    return m_pImpl->m_aResults.size();
}

// virtual
sal_Int32 DataSupplier::currentCount()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
    return m_pImpl->m_aResults.size();
}

// virtual
sal_Bool DataSupplier::isCountFinal()
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );
    return m_pImpl->m_bCountFinal;
}

// virtual
uno::Reference< sdbc::XRow > DataSupplier::queryPropertyValues( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) ) {
        uno::Reference< sdbc::XRow > xRow = m_pImpl->m_aResults[ nIndex ]->xRow;
        if ( xRow.is() ) // Already cached.
            return xRow;
    }

    if ( getResult( nIndex ) ) {
        // Inefficient - but we can't create xContent's sensibly
        // nor can we do the property code sensibly cleanly staticaly.
        Content *pContent = static_cast< ::gvfs::Content * >(queryContent( nIndex ).get());

        uno::Reference< sdbc::XRow > xRow =
            pContent->getPropertyValues( getResultSet()->getProperties(),
                             getResultSet()->getEnvironment() );

        m_pImpl->m_aResults[ nIndex ]->xRow = xRow;

        return xRow;
    }

    return uno::Reference< sdbc::XRow >();
}

// virtual
void DataSupplier::releasePropertyValues( sal_Int32 nIndex )
{
    osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( nIndex < sal::static_int_cast<sal_Int32>(m_pImpl->m_aResults.size()) )
        m_pImpl->m_aResults[ nIndex ]->xRow = uno::Reference< sdbc::XRow >();
}

// virtual
void DataSupplier::close()
{
}

// virtual
void DataSupplier::validate()
    throw( com::sun::star::ucb::ResultSetException )
{
}

sal_Bool DataSupplier::getData()
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

    if ( !m_pImpl->m_bCountFinal ) {
        GnomeVFSResult result;
        GnomeVFSDirectoryHandle *dirHandle = NULL;

        {
            Authentication aAuth( getResultSet()->getEnvironment() );
            char *uri = m_pImpl->m_xContent->getURI();
            result = gnome_vfs_directory_open
                ( &dirHandle, uri, GNOME_VFS_FILE_INFO_DEFAULT );

            if (result != GNOME_VFS_OK) {
#ifdef DEBUG
                g_warning ("Failed open of '%s' with '%s'",
                       uri, gnome_vfs_result_to_string( result ));
#endif
                g_free( uri );
                return sal_False;
            }

            g_free( uri );
        }


        GnomeVFSFileInfo fileInfo;
        fileInfo.name = 0;
        while ((result = gnome_vfs_directory_read_next (dirHandle, &fileInfo)) == GNOME_VFS_OK) {
            if( fileInfo.name && fileInfo.name[0] == '.' &&
                ( fileInfo.name[1] == '\0' ||
                  ( fileInfo.name[1] == '.' && fileInfo.name[2] == '\0' ) ) )
                continue;

            switch ( m_pImpl->m_nOpenMode ) {
            case com::sun::star::ucb::OpenMode::FOLDERS:
                if ( !(fileInfo.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE) ||
                     fileInfo.type != GNOME_VFS_FILE_TYPE_DIRECTORY )
                    continue;
                break;

            case com::sun::star::ucb::OpenMode::DOCUMENTS:
                if ( !(fileInfo.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE) ||
                     fileInfo.type != GNOME_VFS_FILE_TYPE_REGULAR )
                    continue;
                break;

            case com::sun::star::ucb::OpenMode::ALL:
            default:
                break;
            }

            m_pImpl->m_aResults.push_back( new ResultListEntry( &fileInfo ) );
        }
#ifdef DEBUG
        g_warning ("Got %d directory entries", result);
#endif

        m_pImpl->m_bCountFinal = sal_True;

        // Callback possible, because listeners may be informed!
        aGuard.clear();
        getResultSet()->rowCountFinal();

        if (result != GNOME_VFS_ERROR_EOF) {
#ifdef DEBUG
            g_warning( "Failed read_next '%s'",
                   gnome_vfs_result_to_string( result ) );
#endif
            return sal_False;
        }

        result = gnome_vfs_directory_close (dirHandle);
        if (result != GNOME_VFS_OK) {
#ifdef DEBUG
            g_warning( "Failed close '%s'",
                   gnome_vfs_result_to_string( result ) );
#endif
            return sal_False;
        }
    }

    return sal_True;
}



