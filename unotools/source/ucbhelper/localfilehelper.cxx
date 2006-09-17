/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localfilehelper.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:29:27 $
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
#include "precompiled_unotools.hxx"

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif

#include <unotools/localfilehelper.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <tools/debug.hxx>
#include <tools/list.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace utl
{

sal_Bool LocalFileHelper::ConvertSystemPathToURL( const String& rName, const String& rBaseURL, String& rReturn )
{
    rReturn = ::rtl::OUString();

    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
        rtl::OUString aRet;
        if ( FileBase::getFileURLFromSystemPath( rName, aRet ) == FileBase::E_None )
            rReturn = aRet;
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
            rReturn = ::ucb::getFileURLFromSystemPath( xManager, rBaseURL, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    return ( rReturn.Len() != 0 );
}

sal_Bool LocalFileHelper::ConvertURLToSystemPath( const String& rName, String& rReturn )
{
    rReturn = ::rtl::OUString();
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
        rtl::OUString aRet;
        if( FileBase::getSystemPathFromFileURL( rName, aRet ) == FileBase::E_None )
            rReturn = aRet;
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
            rReturn = ::ucb::getSystemPathFromFileURL( xManager, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
        }
    }

    return ( rReturn.Len() != 0 );
}

sal_Bool LocalFileHelper::ConvertPhysicalNameToURL( const String& rName, String& rReturn )
{
    rReturn = ::rtl::OUString();
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
        rtl::OUString aRet;
        if ( FileBase::getFileURLFromSystemPath( rName, aRet ) == FileBase::E_None )
            rReturn = aRet;
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();

        try
        {
            rtl::OUString aBase( ::ucb::getLocalFileURL( xManager ) );
            rReturn = ::ucb::getFileURLFromSystemPath( xManager, aBase, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
        }
    }

    return ( rReturn.Len() != 0 );
}

sal_Bool LocalFileHelper::ConvertURLToPhysicalName( const String& rName, String& rReturn )
{
    rReturn = ::rtl::OUString();
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
        ::rtl::OUString aRet;
        if ( FileBase::getSystemPathFromFileURL( rName, aRet ) == FileBase::E_None )
            rReturn = aRet;
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
            INetURLObject aObj( rName );
            INetURLObject aLocal( ::ucb::getLocalFileURL( xManager ) );
            if ( aObj.GetProtocol() == aLocal.GetProtocol() )
                rReturn = ::ucb::getSystemPathFromFileURL( xManager, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
        }
    }

    return ( rReturn.Len() != 0 );
}

sal_Bool LocalFileHelper::IsLocalFile( const String& rName )
{
    String aTmp;
    return ConvertURLToPhysicalName( rName, aTmp );
}

sal_Bool LocalFileHelper::IsFileContent( const String& rName )
{
    String aTmp;
    return ConvertURLToSystemPath( rName, aTmp );
}

DECLARE_LIST( StringList_Impl, ::rtl::OUString* )

::com::sun::star::uno::Sequence < ::rtl::OUString > LocalFileHelper::GetFolderContents( const ::rtl::OUString& rFolder, sal_Bool bFolder )
{
    StringList_Impl* pFiles = NULL;
    try
    {
        ::ucb::Content aCnt( rFolder, Reference< XCommandEnvironment > () );
        Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aProps(1);
        ::rtl::OUString* pProps = aProps.getArray();
        pProps[0] = ::rtl::OUString::createFromAscii( "Url" );

        try
        {
            ::ucb::ResultSetInclude eInclude = bFolder ? ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS : ::ucb::INCLUDE_DOCUMENTS_ONLY;
            xResultSet = aCnt.createCursor( aProps, eInclude );
        }
        catch( ::com::sun::star::ucb::CommandAbortedException& )
        {
        }
        catch( Exception& )
        {
        }

        if ( xResultSet.is() )
        {
            pFiles = new StringList_Impl;
            Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                {
                    ::rtl::OUString aId = xContentAccess->queryContentIdentifierString();
                    ::rtl::OUString* pFile = new ::rtl::OUString( aId );
                    pFiles->Insert( pFile, LIST_APPEND );
                }
            }
            catch( ::com::sun::star::ucb::CommandAbortedException& )
            {
            }
            catch( Exception& )
            {
            }
        }
    }
    catch( Exception& )
    {
    }

    if ( pFiles )
    {
        ULONG nCount = pFiles->Count();
        Sequence < ::rtl::OUString > aRet( nCount );
        ::rtl::OUString* pRet = aRet.getArray();
        for ( USHORT i = 0; i < nCount; ++i )
        {
            ::rtl::OUString* pFile = pFiles->GetObject(i);
            pRet[i] = *( pFile );
            delete pFile;
        }
        delete pFiles;
        return aRet;
    }
    else
        return Sequence < ::rtl::OUString > ();
}

}
