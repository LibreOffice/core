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
    if ( _suSysPath.indexOf(rtl::OUString::createFromAscii("..")) != -1 )
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


