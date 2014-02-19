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

bool LocalFileHelper::ConvertSystemPathToURL( const OUString& rName, const OUString& rBaseURL, OUString& rReturn )
{
    rReturn = "";

    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext() ) );
    try
    {
        rReturn = ::ucbhelper::getFileURLFromSystemPath( pBroker, rBaseURL, rName );
    }
    catch ( ::com::sun::star::uno::RuntimeException& )
    {
        return false;
    }

    return !rReturn.isEmpty();
}

bool LocalFileHelper::ConvertURLToSystemPath( const OUString& rName, OUString& rReturn )
{
    rReturn = "";
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

    return !rReturn.isEmpty();
}

bool LocalFileHelper::ConvertPhysicalNameToURL(const OUString& rName, OUString& rReturn)
{
    rReturn = OUString();
    Reference< XUniversalContentBroker > pBroker(
        UniversalContentBroker::create(
            comphelper::getProcessComponentContext() ) );
    try
    {
        OUString aBase( ::ucbhelper::getLocalFileURL() );
        rReturn = ::ucbhelper::getFileURLFromSystemPath( pBroker, aBase, rName );
    }
    catch (const ::com::sun::star::uno::RuntimeException&)
    {
    }

    return !rReturn.isEmpty();
}

bool LocalFileHelper::ConvertURLToPhysicalName(const OUString& rName, OUString& rReturn)
{
    rReturn = "";
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

bool LocalFileHelper::IsLocalFile(const OUString& rName)
{
    OUString aTmp;
    return ConvertURLToPhysicalName(rName, aTmp);
}

bool LocalFileHelper::IsFileContent(const OUString& rName)
{
    OUString aTmp;
    return ConvertURLToSystemPath(rName, aTmp);
}

typedef ::std::vector< OUString* > StringList_Impl;

::com::sun::star::uno::Sequence < OUString > LocalFileHelper::GetFolderContents( const OUString& rFolder, bool bFolder )
{
    StringList_Impl* pFiles = NULL;
    try
    {
        ::ucbhelper::Content aCnt(
            rFolder, Reference< XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;
        ::com::sun::star::uno::Sequence< OUString > aProps(1);
        OUString* pProps = aProps.getArray();
        pProps[0] = "Url";

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
                    OUString aId = xContentAccess->queryContentIdentifierString();
                    OUString* pFile = new OUString( aId );
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
        Sequence < OUString > aRet( nCount );
        OUString* pRet = aRet.getArray();
        for ( size_t i = 0; i < nCount; ++i )
        {
            OUString* pFile = (*pFiles)[ i ];
            pRet[i] = *( pFile );
            delete pFile;
        }
        delete pFiles;
        return aRet;
    }
    else
        return Sequence < OUString > ();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
