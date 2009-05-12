/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filehelper.cxx,v $
 * $Revision: 1.12 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "testshl/filehelper.hxx"
#include "testshl/getopt.hxx"

#include <osl/process.h>
#include <osl/file.hxx>

#ifndef _SAL_TRES_H_
#include <rtl/tres.h>
#endif

namespace FileHelper
{
// -----------------------------------------------------------------------------
// taken from setup2
std::string getTempPath()
{
    std::string sTempDir;

    char const * pTmp = getenv( "TEMP" );
    if (pTmp == NULL) pTmp = getenv("temp");
    if (pTmp == NULL) pTmp = getenv("TMP");
    if (pTmp == NULL) pTmp = getenv("tmp");

    if( pTmp && strlen(pTmp) >= 2 )
    {
        sTempDir = std::string( pTmp );
    }
    else
    {
#if (defined UNX) || (defined OS2)
        int nLen;
        pTmp = P_tmpdir;
        nLen = strlen(pTmp);
        if (pTmp[ nLen - 1] == '/')
        {
            char cBuf[256];
            char* pBuf = cBuf;
            strncpy( pBuf, pTmp, nLen - 1 );
            pBuf[nLen - 1] = '\0';
            sTempDir = std::string( pBuf );
        }
        else
        {
            sTempDir = std::string( pTmp );
        }
#else
        fprintf(stderr, "error: No temp dir found.\n");
#endif
    }
    return sTempDir;
}

// -----------------------------------------------------------------------------
rtl::OUString convertPath( rtl::OUString const& _suSysPath )
{
    // PRE: String should contain a filename, relativ or absolut
    rtl::OUString suURL;
    bool bRelativ = false;
#ifdef WNT
    sal_Char cFileSep[] = "\\";
#endif
#if (defined UNX) || (defined OS2)
    sal_Char cFileSep[] = "/";
#endif

    if ( _suSysPath.indexOf(rtl::OUString::createFromAscii("..")) == 0 )
    {
        bRelativ = true;
    }
    else if ( _suSysPath.indexOf(rtl::OUString::createFromAscii(cFileSep)) != 0 )
    {
        // no fileseparator found at first position found, must be relative
        bRelativ = true;
    }

    if (bRelativ)
    {
        // filepath contains '..' so it's a relative path make it absolut.
        rtl::OUString curDirPth;
        osl_getProcessWorkingDir( &curDirPth.pData );

        // rtl::OString sCurDirPath = rtl::OUStringToOString(curDirPth, RTL_TEXTENCODING_ASCII_US);
        // fprintf(stderr, "Current Dir '%s'.\n", sCurDirPath.getStr());

        osl::FileBase::getAbsoluteFileURL( curDirPth, _suSysPath, suURL );

        // rtl::OString sURL = rtl::OUStringToOString(suURL, RTL_TEXTENCODING_ASCII_US);
        // fprintf(stderr, "  File URL: '%s'.\n", sURL.getStr());
    }
    else
    {
        osl::FileBase::getFileURLFromSystemPath( _suSysPath, suURL );
    }
    return suURL;
}
// -----------------------------------------------------------------------------
rtl::OUString convertPath( rtl::OString const& sysPth )
{
    rtl::OUString sysPath( rtl::OUString::createFromAscii( sysPth.getStr() ) );
    return convertPath(sysPath);
}

/**
 * create bitmap of comandline parameters
 */

//# CmdLineBits createFlags( vector< sal_Char* > const& cmdln )
//# {
//#     CmdLineBits retflags = rtl_tres_Flag_OK;
//#
//#     vector< sal_Char* >::const_iterator iter = cmdln.begin();
//#     while( iter != cmdln.end() )
//#     {
//#         fprintf( stderr, "%s\n", *iter );
//#         if ( *iter[0] == '-' )
//#         {
//#             rtl::OString item( *iter );
//#             if ( item == "-boom" )               // stop near error position, exception only
//#                 retflags |= rtl_tres_Flag_BOOM;
//#
//#             if ( item == "-verbose" )
//#                 retflags |= rtl_tres_Flag_VERBOSE;
//#
//#             if ( item == "-skip" )
//#                 retflags |= rtl_tres_Flag_SKIP;
//#
//#             if ( item == "-log" )
//#                 retflags |= rtl_tres_Flag_LOG;
//#
//#             if ( item == "-his" )
//#                 retflags |= rtl_tres_Flag_HIS;
//#
//#             if ( item == "-time" )
//#                 retflags |= rtl_tres_Flag_TIME;
//#
//#             if ( item == "-msg" )
//#                 retflags |= rtl_tres_Flag_MSG;
//#         }
//#         iter++;
//#     }
//#
//#     return retflags;
//# }
//#
//# CmdLineBits createFlags(int argc, char* argv[])
//# {
//#     vector< sal_Char* > cmdln;
//#     sal_Int32 i;
//#
//#      /* collect comandline */
//#     for ( i = 1; i < argc; i++ )
//#          cmdln.push_back( argv[i] );
//#
//#     return createFlags(cmdln);
//# }

CmdLineBits createFlags( GetOpt & _aOptions )
{
    CmdLineBits retflags = rtl_tres_Flag_OK;

    if (_aOptions.hasOpt("-boom")) // stop near error position, exception only
    {
        retflags |= rtl_tres_Flag_BOOM;
    }

    if (_aOptions.hasOpt("-verbose"))
    {
        retflags |= rtl_tres_Flag_VERBOSE;
    }

    if (_aOptions.hasOpt("-quiet"))
    {
        retflags |= rtl_tres_Flag_QUIET;
    }
    return retflags;
}
// -----------------------------------------------------------------------------

/**
 * display usage screen
 */

//# void usage()
//# {
//#     fprintf( stdout,
//#             "USAGE: testshl shlname [-boom][-verbose][-log][-his][-msg]\n" );
//#     exit(0);
//# }


} // namespace FileHelper


