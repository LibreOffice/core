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

#define CSV_USE_CSV_ASSERTIONS
#include <cosv/csv_env.hxx>

#include <cosv/comfunc.hxx>
#include <cosv/string.hxx>
#include <cosv/streamstr.hxx>
#include <cosv/std_outp.hxx>
#include <cosv/tpl/dyn.hxx>
#include <cosv/dirchain.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/bstream.hxx>




namespace csv
{
namespace ploc
{


DirectoryChain::DirectoryChain()
{
}

DirectoryChain::~DirectoryChain()
{
}

void
DirectoryChain::Set( const char *        i_sSubPath,
                     bool                i_bPathIsAlwaysDir,
                     const char *        i_sDelimiter        )
{
    csv_assert(i_sDelimiter != 0);
    if (i_sSubPath == 0)
        return;

    const char * pRestPath = i_sSubPath;
    if (*pRestPath == *i_sDelimiter)
        ++pRestPath;

    for ( const char * pDirEnd = strchr(pRestPath,*i_sDelimiter);
          pDirEnd != 0;
          pDirEnd = strchr(pRestPath,*i_sDelimiter) )
    {
        aPath.push_back( String(pRestPath, pDirEnd) );
        pRestPath = pDirEnd + 1;
    }
    if (*pRestPath != 0 AND i_bPathIsAlwaysDir)
        aPath.push_back( String(pRestPath) );
}

void
DirectoryChain::PushBack( const String & i_sName )
{
    aPath.push_back(i_sName);
}

void
DirectoryChain::PushBack( const DirectoryChain & i_sPath )
{
    aPath.insert( aPath.end(), i_sPath.Begin(), i_sPath.End() );
}

void
DirectoryChain::PopBack( uintt i_nCount )
{
    if (i_nCount <= aPath.size())
        aPath.erase( aPath.end() - i_nCount, aPath.end() );
    else
        aPath.erase( aPath.begin(), aPath.end() );
}

void
DirectoryChain::Get( bostream &      o_rPath,
                     const char *    i_sDelimiter ) const
{
    uintt deliLen = strlen(i_sDelimiter);

    for ( std::vector<String>::const_iterator it = aPath.begin();
          it != aPath.end();
          ++it )
    {
        o_rPath.write( (*it).c_str() );
        o_rPath.write( i_sDelimiter,  deliLen);
    }
}




} // namespace ploc
} // namespace csv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
