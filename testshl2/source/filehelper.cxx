/*************************************************************************
 *
 *  $RCSfile: filehelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:07:31 $
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "filehelper.hxx"
#include "getopt.hxx"

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

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

    char* pTmp = getenv( "TEMP" );
    if (pTmp == NULL) pTmp = getenv("temp");
    if (pTmp == NULL) pTmp = getenv("TMP");
    if (pTmp == NULL) pTmp = getenv("tmp");

    if( pTmp && strlen(pTmp) >= 2 )
    {
        sTempDir = std::string( pTmp );
    }
    else
    {
#ifdef UNX
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
    rtl::OUString fURL;
    bool bRelativ = false;
#ifdef WNT
    sal_Char cFileSep[] = "\\";
#endif
#ifdef UNX
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
        osl::FileBase::getAbsoluteFileURL( curDirPth, _suSysPath, fURL );
    }
    else
    {
        osl::FileBase::getFileURLFromSystemPath( _suSysPath, fURL );
    }
    return fURL;
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


