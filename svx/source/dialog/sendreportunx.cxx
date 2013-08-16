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

#include <config_folders.h>

#include "docrecovery.hxx"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/bootstrap.hxx"
#include "rtl/strbuf.hxx"
#include "tools/appendunixshellword.hxx"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

#define RCFILE ".crash_reportrc"

using namespace ::std;

static const char *get_home_dir()
{
    struct passwd *ppwd = getpwuid( getuid() );

    return ppwd ? (ppwd->pw_dir ? ppwd->pw_dir : "/") : "/";
}

static bool read_line( FILE *fp, string& rLine )
{
    char szBuffer[1024];
    bool bSuccess = false;
    bool bEOL = false;
    string  line;


    while ( !bEOL && fgets( szBuffer, sizeof(szBuffer), fp ) )
    {
        int len = strlen(szBuffer);

        bSuccess = true;

        while ( len && szBuffer[len - 1] == '\n' )
        {
            szBuffer[--len] = 0;
            bEOL = true;
        }

        line.append( szBuffer );
    }

    rLine = line;
    return bSuccess;
}

static string trim_string( const string& rString )
{
    string temp = rString;

    while ( temp.length() && (temp[0] == ' ' || temp[0] == '\t') )
        temp.erase( 0, 1 );

    string::size_type   len = temp.length();

    while ( len && (temp[len-1] == ' ' || temp[len-1] == '\t') )
    {
        temp.erase( len - 1, 1 );
        len = temp.length();
    }

    return temp;
}

static string get_profile_string( const char *pFileName, const char *pSectionName, const char *pKeyName, const char *pDefault = NULL )
{
    FILE    *fp = fopen( pFileName, "r" );
    string  retValue = pDefault ? pDefault : "";

    if ( fp )
    {
        string line;
        string section;

        while ( read_line( fp, line ) )
        {
            line = trim_string( line );

            if ( line.length() && line[0] == '[' )
            {
                line.erase( 0, 1 );
                string::size_type end = line.find( ']', 0 );

                if ( string::npos != end )
                    section = trim_string( line.substr( 0, end ) );
            }
            else
            {

                string::size_type iEqualSign = line.find( '=', 0 );

                if ( iEqualSign != string::npos )
                {
                    string  keyname = line.substr( 0, iEqualSign );
                    keyname = trim_string( keyname );

                    string  value = line.substr( iEqualSign + 1, string::npos );
                    value = trim_string( value );

                    if (
                        0 == strcasecmp( section.c_str(), pSectionName ) &&
                        0 == strcasecmp( keyname.c_str(), pKeyName )
                         )
                    {
                        retValue = value;
                        break;
                    }
                }
            }
        }

        fclose( fp );
    }

    return retValue;
}

static bool get_profile_bool( const char *pFileName, const char *pSectionName, const char *pKeyName )
{
    string  str = get_profile_string( pFileName, pSectionName, pKeyName );

    if ( !strcasecmp( str.c_str(), "true" ) )
        return true;
    return false;
}

static String get_profile_String( const char *pFileName, const char *pSectionName, const char *pKeyName, const char * = NULL )
{
    string  str = get_profile_string( pFileName, pSectionName, pKeyName );
    String  result( str.c_str(), RTL_TEXTENCODING_UTF8 );

    return result;
}

namespace svx{
    namespace DocRecovery{

        bool ErrorRepSendDialog::ReadParams()
        {
            string  sRCFile = get_home_dir();

            sRCFile += "/";
            sRCFile += string(RCFILE);

            maEMailAddrED.SetText( get_profile_String( sRCFile.c_str(), "Options", "ReturnAddress" ) );
            maParams.maHTTPProxyServer = get_profile_String( sRCFile.c_str(), "Options", "ProxyServer" );
            maParams.maHTTPProxyPort = get_profile_String( sRCFile.c_str(), "Options", "ProxyPort" );
            maParams.miHTTPConnectionType = get_profile_bool( sRCFile.c_str(), "Options", "UseProxy" ) ? 2 : 1;
            maContactCB.Check( get_profile_bool( sRCFile.c_str(), "Options", "AllowContact" ) );

            return true;
        }

        bool ErrorRepSendDialog::SaveParams()
        {
            bool success = false;
            string  sRCFile = get_home_dir();

            sRCFile += "/";
            sRCFile += string(RCFILE);

            FILE *fp = fopen( sRCFile.c_str(), "w" );

            if ( fp )
            {
                fprintf( fp, "[Options]\n" );
                fprintf( fp, "UseProxy=%s\n", 2 == maParams.miHTTPConnectionType ? "true" : "false" );
                fprintf( fp, "ProxyServer=%s\n", OUStringToOString( maParams.maHTTPProxyServer, RTL_TEXTENCODING_UTF8 ).getStr() );
                fprintf( fp, "ProxyPort=%s\n", OUStringToOString( maParams.maHTTPProxyPort, RTL_TEXTENCODING_UTF8 ).getStr() );
                fprintf( fp, "ReturnAddress=%s\n", OUStringToOString( GetEMailAddress(), RTL_TEXTENCODING_UTF8 ).getStr() );
                fprintf( fp, "AllowContact=%s\n", IsContactAllowed() ? "true" : "false" );
                fclose( fp );
            }

            return success;
        }

        bool ErrorRepSendDialog::SendReport()
        {
            OUString sSubEnvVar("ERRORREPORT_SUBJECT");
            OUString strSubject(GetDocType());
            osl_setEnvironment(sSubEnvVar.pData, strSubject.pData);

            char szBodyFile[L_tmpnam] = "";
            FILE *fp = fopen( tmpnam( szBodyFile ), "w" );

            if ( fp )
            {
                OString strUTF8(OUStringToOString(GetUsing(), RTL_TEXTENCODING_UTF8));

                size_t nWritten = fwrite(strUTF8.getStr(), 1, strUTF8.getLength(), fp);
                OSL_VERIFY(nWritten == static_cast<size_t>(strUTF8.getLength()));
                fclose( fp );

                OUString sBodyEnvVar("ERRORREPORT_BODYFILE");
                OUString strBodyFile(OStringToOUString(OString(szBodyFile),
                    osl_getThreadTextEncoding()));
                osl_setEnvironment(sBodyEnvVar.pData, strBodyFile.pData);
            }

            int ret = -1;
            OUString path1("$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/crashrep");
            rtl::Bootstrap::expandMacros(path1);
            OString path2;
            if ((osl::FileBase::getSystemPathFromFileURL(path1, path1) ==
                 osl::FileBase::E_None) &&
                path1.convertToString(
                    &path2, osl_getThreadTextEncoding(),
                    (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
                     RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
            {
                OStringBuffer cmd;
                tools::appendUnixShellWord(&cmd, path2);
                cmd.append(" -debug -load -send -noui");
                ret = system(cmd.getStr());
            }

            if ( szBodyFile[0] )
            {
                unlink( szBodyFile );
            }

            return -1 != ret;
        }


    }   // namespace DocRecovery
}   // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
