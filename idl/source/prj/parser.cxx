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

#include <sal/config.h>

#include <parser.hxx>
#include <database.hxx>
#include <globals.hxx>
#include <osl/file.hxx>

bool SvIdlParser::ReadSvIdl( bool bImported, const OUString & rPath )
{
    rBase.SetPath(rPath); // only valid for this iteration
    bool bOk = true;
    SvToken& rTok = rInStm.GetToken();
    // only one import at the very beginning
    if( rTok.Is( SvHash_import() ) )
    {
        rTok = rInStm.GetToken_Next();
        if( rTok.IsString() )
        {
            OUString aFullName;
            if( osl::FileBase::E_None == osl::File::searchFileURL(
                OStringToOUString(rTok.GetString(), RTL_TEXTENCODING_ASCII_US),
                rPath,
                aFullName) )
            {
                osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );
                rBase.AddDepFile(aFullName);
                SvTokenStream aTokStm( aFullName );
                SvIdlParser aInputParser(rBase, aTokStm);
                bOk = aInputParser.ReadSvIdl( true, rPath );
            }
            else
                bOk = false;
        }
        else
            bOk = false;
    }

    while( bOk )
    {
        rTok = rInStm.GetToken();
        if( rTok.IsEof() )
            return true;

        if( rTok.Is( SvHash_module() ) )
        {
            tools::SvRef<SvMetaModule> aModule = new SvMetaModule( bImported );
            if( aModule->ReadSvIdl( rBase, rInStm ) )
                rBase.GetModuleList().push_back( aModule );
            else
                bOk = false;
        }
        else
            bOk = false;
    }
    if( !bOk || !rTok.IsEof() )
    {
         // error treatment
         rBase.WriteError( rInStm );
         return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
