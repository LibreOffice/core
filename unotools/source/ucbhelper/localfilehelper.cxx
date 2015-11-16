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
#include <comphelper/processfactory.hxx>
#include <sal/log.hxx>
#include <unotools/localfilehelper.hxx>
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

typedef ::std::vector< OUString* > StringList_Impl;

::com::sun::star::uno::Sequence < OUString > LocalFileHelper::GetFolderContents( const OUString& rFolder, bool bFolder )
{
    StringList_Impl* pFiles = nullptr;
    try
    {
        ::ucbhelper::Content aCnt(
            rFolder, Reference< XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        Reference< ::com::sun::star::sdbc::XResultSet > xResultSet;
        ::com::sun::star::uno::Sequence< OUString > aProps { "Url" };

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

void removeTree(OUString const & url) {
    osl::Directory dir(url);
    osl::FileBase::RC rc = dir.open();
    switch (rc) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_NOENT:
        return; //TODO: SAL_WARN if recursive
    default:
        SAL_WARN("desktop.app", "cannot open directory " << dir.getURL() << ": " << +rc);
        return;
    }
    for (;;) {
        osl::DirectoryItem i;
        rc = dir.getNextItem(i, SAL_MAX_UINT32);
        if (rc == osl::FileBase::E_NOENT) {
            break;
        }
        if (rc != osl::FileBase::E_None) {
            SAL_WARN( "desktop.app", "cannot iterate directory " << dir.getURL() << ": " << +rc);
            break;
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        rc = i.getFileStatus(stat);
        if (rc != osl::FileBase::E_None) {
            SAL_WARN( "desktop.app", "cannot stat in directory " << dir.getURL() << ": " << +rc);
            continue;
        }
        if (stat.getFileType() == osl::FileStatus::Directory) { //TODO: symlinks
            removeTree(stat.getFileURL());
        } else {
            rc = osl::File::remove(stat.getFileURL());
            SAL_WARN_IF(
                rc != osl::FileBase::E_None, "desktop.app",
                "cannot remove file " << stat.getFileURL() << ": " << +rc);
        }
    }
    if (dir.isOpen()) {
        rc = dir.close();
        SAL_WARN_IF(
            rc != osl::FileBase::E_None, "desktop.app",
            "cannot close directory " << dir.getURL() << ": " << +rc);
    }
    rc = osl::Directory::remove(url);
    SAL_WARN_IF(
        rc != osl::FileBase::E_None, "desktop.app",
        "cannot remove directory " << url << ": " << +rc);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
