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
#include <comphelper/DirectoryHelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <unotools/localfilehelper.hxx>
#include <rtl/ustring.hxx>
#include <ucbhelper/content.hxx>
#include <vector>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace utl
{

css::uno::Sequence < OUString > LocalFileHelper::GetFolderContents( const OUString& rFolder, bool bFolder )
{
    std::vector< OUString > vFiles;
    try
    {
        ::ucbhelper::Content aCnt(
            rFolder, Reference< XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        Reference< css::sdbc::XResultSet > xResultSet;
        css::uno::Sequence< OUString > aProps { "Url" };

        try
        {
            ::ucbhelper::ResultSetInclude eInclude = bFolder ? ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS : ::ucbhelper::INCLUDE_DOCUMENTS_ONLY;
            xResultSet = aCnt.createCursor( aProps, eInclude );
        }
        catch( css::ucb::CommandAbortedException& )
        {
        }
        catch( Exception& )
        {
        }

        if ( xResultSet.is() )
        {
            Reference< XContentAccess > xContentAccess( xResultSet, UNO_QUERY );
            try
            {
                while ( xResultSet->next() )
                    vFiles.push_back( xContentAccess->queryContentIdentifierString() );
            }
            catch( css::ucb::CommandAbortedException& )
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

    return comphelper::containerToSequence(vFiles);
}

void removeTree(OUString const & url) {
    const bool bError = comphelper::DirectoryHelper::deleteDirRecursively(url);
    SAL_WARN_IF(bError, "desktop.app", "error removing directory " << url);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
