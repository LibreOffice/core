/*************************************************************************
 *
 *  $RCSfile: localfilehelper.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: mba $ $Date: 2001-07-18 11:11:27 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#include <ucbhelper/content.hxx>

using namespace osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace utl
{

sal_Bool LocalFileHelper::ConvertSystemPathToURL( const String& rName, const String& rBaseURL, String& rReturn )
{
    rtl::OUString aRet;
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
#ifdef TF_FILEURL
        FileBase::getFileURLFromSystemPath( rName, aRet );
#else
        ::rtl::OUString aTmp;

        FileBase::normalizePath( rName, aTmp );
        FileBase::getFileURLFromNormalizedPath( aTmp, aRet );
#endif
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
            aRet = ::ucb::getFileURLFromSystemPath( xManager, rBaseURL, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    rReturn = aRet;
    return ( aRet.getLength() != 0 );
}

sal_Bool LocalFileHelper::ConvertURLToSystemPath( const String& rName, String& rReturn )
{
    rtl::OUString aRet;
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
#ifdef TF_FILEURL
        FileBase::getSystemPathFromFileURL( rName, aRet );
#else
        ::rtl::OUString aTmp;
        FileBase::getNormalizedPathFromFileURL( rName, aTmp );
        FileBase::getSystemPathFromNormalizedPath( aTmp, aRet );
#endif
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
            aRet = ::ucb::getSystemPathFromFileURL( xManager, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    rReturn = aRet;
    return ( aRet.getLength() != 0 );
}

sal_Bool LocalFileHelper::ConvertPhysicalNameToURL( const String& rName, String& rReturn )
{
    rtl::OUString aRet;

    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
#ifdef TF_FILEURL
        FileBase::getFileURLFromSystemPath( rName, aRet );
#else
        ::rtl::OUString aTmp;
        FileBase::normalizePath( rName, aTmp );
        FileBase::getFileURLFromNormalizedPath( aTmp, aRet );
#endif
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();

        try
        {
            rtl::OUString aBase( ::ucb::getLocalFileURL( xManager ) );
            aRet = ::ucb::getFileURLFromSystemPath( xManager, aBase, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    rReturn = aRet;
    return ( aRet.getLength() != 0 );
}

sal_Bool LocalFileHelper::ConvertURLToPhysicalName( const String& rName, String& rReturn )
{
    rtl::OUString aRet;
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
#ifdef TF_FILEURL
        FileBase::getSystemPathFromFileURL( rName, aRet );
#else
        ::rtl::OUString aTmp;
        FileBase::getNormalizedPathFromFileURL( rName, aTmp );
        FileBase::getSystemPathFromNormalizedPath( aTmp, aRet );

        /* HR0: Why this ??? */
        aRet = aTmp;
#endif
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
            aRet = ::ucb::getSystemPathFromFileURL( xManager, rName );
        }
        catch ( ::com::sun::star::uno::RuntimeException& )
        {
            return sal_False;
        }
    }

    rReturn = aRet;
    return ( aRet.getLength() != 0 );
}

sal_Bool LocalFileHelper::IsLocalFile( const String& rName )
{
    String aTmp;
    return ConvertURLToPhysicalName( rName, aTmp );
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
        pProps[0] == ::rtl::OUString::createFromAscii( "Url" );

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

};
