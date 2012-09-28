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

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <vector>

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace utl
{

sal_Bool LocalFileHelper::ConvertSystemPathToURL( const String& rName, const String& rBaseURL, String& rReturn )
{
    rReturn = ::rtl::OUString();

    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext() ) );
    try
    {
        rReturn = ::ucbhelper::getFileURLFromSystemPath( pBroker, rBaseURL, rName );
    }
    catch ( ::com::sun::star::uno::RuntimeException& )
    {
        return sal_False;
    }

    return ( rReturn.Len() != 0 );
}

sal_Bool LocalFileHelper::ConvertURLToSystemPath( const String& rName, String& rReturn )
{
    rReturn = ::rtl::OUString();
    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext() ) );
    try
    {
        rReturn = ::ucbhelper::getSystemPathFromFileURL( pBroker, rName );
    }
    catch ( ::com::sun::star::uno::RuntimeException& )
    {
    }

    return ( rReturn.Len() != 0 );
}

bool LocalFileHelper::ConvertPhysicalNameToURL(const rtl::OUString& rName, rtl::OUString& rReturn)
{
    rReturn = ::rtl::OUString();
    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext() ) );
    try
    {
        rtl::OUString aBase( ::ucbhelper::getLocalFileURL() );
        rReturn = ::ucbhelper::getFileURLFromSystemPath( pBroker, aBase, rName );
    }
    catch (const ::com::sun::star::uno::RuntimeException&)
    {
    }

    return !rReturn.isEmpty();
}

bool LocalFileHelper::ConvertURLToPhysicalName(const rtl::OUString& rName, rtl::OUString& rReturn)
{
    rReturn = ::rtl::OUString();
    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext() ) );
    try
    {
        INetURLObject aObj( rName );
        INetURLObject aLocal( ::ucbhelper::getLocalFileURL() );
        if ( aObj.GetProtocol() == aLocal.GetProtocol() )
            rReturn = ::ucbhelper::getSystemPathFromFileURL( pBroker, rName );
    }
    catch (const ::com::sun::star::uno::RuntimeException&)
    {
    }

    return !rReturn.isEmpty();
}

sal_Bool LocalFileHelper::IsLocalFile(const rtl::OUString& rName)
{
    rtl::OUString aTmp;
    return ConvertURLToPhysicalName(rName, aTmp);
}

sal_Bool LocalFileHelper::IsFileContent(const rtl::OUString& rName)
{
    String aTmp;
    return ConvertURLToSystemPath(rName, aTmp);
}

typedef ::std::vector< ::rtl::OUString* > StringList_Impl;

::com::sun::star::uno::Sequence < ::rtl::OUString > LocalFileHelper::GetFolderContents( const ::rtl::OUString& rFolder, sal_Bool bFolder )
{
    StringList_Impl* pFiles = NULL;
    try
    {
        ::ucbhelper::Content aCnt(
            rFolder, Reference< XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aProps(1);
        ::rtl::OUString* pProps = aProps.getArray();
        pProps[0] = ::rtl::OUString("Url");

        try
        {
            ::ucbhelper::ResultSetInclude eInclude = bFolder ? ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS : ::ucbhelper::INCLUDE_DOCUMENTS_ONLY;
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
                    pFiles->push_back( pFile );
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
        size_t nCount = pFiles->size();
        Sequence < ::rtl::OUString > aRet( nCount );
        ::rtl::OUString* pRet = aRet.getArray();
        for ( size_t i = 0; i < nCount; ++i )
        {
            ::rtl::OUString* pFile = (*pFiles)[ i ];
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
