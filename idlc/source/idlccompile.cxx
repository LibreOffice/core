/*************************************************************************
 *
 *  $RCSfile: idlccompile.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:11:08 $
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
#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#if defined(SAL_W32) || defined(SAL_OS2)
#include <io.h>
#endif

#ifdef  SAL_UNX
#include <unistd.h>
#if defined(MACOSX) || defined(FREEBSD) || defined(NETBSD)
#include <sys/wait.h>
#else
#include <wait.h>
#endif
#endif

using namespace ::rtl;
using namespace ::osl;

extern int yyparse();
extern FILE* yyin;
extern int yydebug;

sal_Int32 lineNumber = 1;


static OUString TMP(RTL_CONSTASCII_USTRINGPARAM("TMP"));
static OUString TEMP(RTL_CONSTASCII_USTRINGPARAM("TEMP"));
static sal_Char tmpFilePattern[512];

sal_Bool isFileUrl(const OString& fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

OString convertToAbsoluteSystemPath(const OString& fileName)
{
    OUString uSysFileName;
    OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
    if ( isFileUrl(fileName) )
    {
        OSL_VERIFY(FileBase::getSystemPathFromFileURL(uFileName, uSysFileName) == FileBase::E_None);
    } else
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0)
    {
        OUString uWorkingDir, uUrlFileName;
        OSL_VERIFY( osl_getProcessWorkingDir(&uWorkingDir.pData) == osl_Process_E_None );
        OSL_VERIFY( FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uUrlFileName) == FileBase::E_None );
        OSL_VERIFY( FileBase::getSystemPathFromFileURL(uUrlFileName, uSysFileName) == FileBase::E_None );
    } else
    {
        return fileName;
    }

    return OUStringToOString(uSysFileName, osl_getThreadTextEncoding());
}

OString convertToFileUrl(const OString& fileName)
{
    if ( !isFileUrl(fileName) )
    {
        OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
        OUString uUrlFileName;
        OSL_VERIFY(FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName) == FileBase::E_None);
        return OUStringToOString(uUrlFileName, osl_getThreadTextEncoding());
    }

    return fileName;
}

// prefix must be specified, postfix could be empty string
OString makeTempName(const OString& prefix, const OString& postfix)
{
    OUString uTmpPath;
    OString tmpPath;

    if ( osl_getEnvironment(TMP.pData, &uTmpPath.pData) != osl_Process_E_None )
    {
        if ( osl_getEnvironment(TEMP.pData, &uTmpPath.pData) != osl_Process_E_None )
        {
#if defined(SAL_W32) || defined(SAL_OS2)
            tmpPath = OString("c:\\temp");
#else
            tmpPath = OString("/tmp");
#endif
        }
    }

    if ( uTmpPath.getLength() )
        tmpPath = OUStringToOString(uTmpPath, RTL_TEXTENCODING_UTF8);

#if defined(SAL_W32) || defined(SAL_UNX)

    OSL_ASSERT( sizeof(tmpFilePattern) > ( strlen(tmpPath)
                                           + RTL_CONSTASCII_LENGTH(
                                                PATH_SEPARATOR )
                                           + prefix.getLength()
                                           + RTL_CONSTASCII_LENGTH(
                                                "XXXXXX") ) );

    tmpFilePattern[ sizeof(tmpFilePattern)-1 ] = '\0';
    strncpy(tmpFilePattern, tmpPath, sizeof(tmpFilePattern)-1);
    strncat(tmpFilePattern, PATH_SEPARATOR, sizeof(tmpFilePattern)-1-strlen(tmpFilePattern));
    strncat(tmpFilePattern, prefix.getStr(), sizeof(tmpFilePattern)-1-strlen(tmpFilePattern));
    strncat(tmpFilePattern, "XXXXXX", sizeof(tmpFilePattern)-1-strlen(tmpFilePattern));

#ifdef SAL_UNX
    int nDescriptor = mkstemp(tmpFilePattern);
    if( -1 == nDescriptor )
    {
        fprintf( stderr,"idlc: couldn't create temporary file\n" );
        exit( 1 );
    }
    // the file shall later be reopened by stdio functions
    close( nDescriptor );
#else
    (void) mktemp(tmpFilePattern);
#endif
    /** DBO (08/22/2002):
        since mkstemp() creates the file, it won't be removed anywhere later appending a postfix.
        Is the postfix necessarry?
    */
//      if ( postfix.getLength() )
//          strncat(tmpFilePattern, postfix.getStr(), sizeof(tmpFilePattern)-1-strlen(tmpFilePattern));
#endif

#ifdef __OS2__
    strncpy(tmpFilePattern, tempnam(NULL, prefix.getStr()), sizeof(tmpFilePattern)-1);
#endif

    return OString(tmpFilePattern);
}

sal_Bool copyFile(const OString& sourceFile, const OString& targetFile)
{
    sal_Bool bRet = sal_True;

    FILE* pSource = fopen(sourceFile.getStr(), "rb");

    if ( !pSource )
        return sal_False;

    FILE* pTarget = fopen(targetFile.getStr(), "wb");

    if ( !pTarget )
    {
        fclose(pSource);
        return sal_False;
    }

    sal_uInt32 totalSize = 512;
    sal_uInt32 readSize = 0;
    sal_uInt32 writeSize = 0;
    sal_Char pBuffer[513];

    while ( !feof(pSource) )
    {
        if ( (readSize = fread(pBuffer, 1, totalSize, pSource)) > 0 && !ferror(pSource) )
        {
            if ( (writeSize = fwrite(pBuffer, 1, readSize, pTarget)) != readSize || ferror(pTarget) )
            {
                fclose(pSource);
                fclose(pTarget);
                return sal_False;
            }
        }
    }

    fclose(pSource);
    if ( fflush(pTarget) )
        bRet = sal_False;
    fclose(pTarget);

    return bRet;
}

sal_Int32 SAL_CALL compileFile(const OString& fileName)
{
    // preporcess input file
    OString tmpFile = makeTempName(OString("idli_"), OString(".idl"));
    OString preprocFile = makeTempName(OString("idlf_"), OString(".idl"));

    if ( !copyFile(fileName, tmpFile) )
    {
          fprintf(stderr, "%s: couldn't copy file '%s' to '%s'\n",
            idlc()->getOptions()->getProgramName().getStr(), fileName.getStr(), tmpFile.getStr());
          exit(99);
    }

    idlc()->setFileName(fileName);
    idlc()->setMainFileName(fileName);
    idlc()->setRealFileName(tmpFile);

    OStringBuffer cppArgs(512);
    cppArgs.append("-DIDL -Xi -Xc -+ -I.");
    Options* pOptions = idlc()->getOptions();

    OString filePath;
    sal_uInt32 index = fileName.lastIndexOf(SEPARATOR);

    if ( index > 0)
    {
        filePath = fileName.copy(0, index);

        if ( filePath.getLength() )
        {
            cppArgs.append(" -I");
            cppArgs.append(filePath);
        }
    }

    if ( pOptions->isValid("-D") )
    {
        cppArgs.append(" ");
        cppArgs.append(pOptions->getOption("-D"));
    }
    if ( pOptions->isValid("-I") )
    {
        cppArgs.append(" ");
        cppArgs.append(pOptions->getOption("-I"));
    }

    cppArgs.append(" ");
    cppArgs.append(tmpFile);
    cppArgs.append(" ");
    cppArgs.append(preprocFile);

    OString cmdFileName = makeTempName(OString("idlc_"), OString());
    FILE* pCmdFile = fopen(cmdFileName, "w");

    if ( !pCmdFile )
    {
          fprintf(stderr, "%s: couldn't open temporary file for preprocessor commands: %s\n",
            idlc()->getOptions()->getProgramName().getStr(), cmdFileName.getStr());
          exit(99);
    }
    fprintf(pCmdFile, "%s", cppArgs.getStr());
    fclose(pCmdFile);

    OUString cmdArg(RTL_CONSTASCII_USTRINGPARAM("@"));
    cmdArg += OStringToOUString(cmdFileName, RTL_TEXTENCODING_UTF8);

    OUString cpp;
    OUString startDir;
    OSL_VERIFY(osl_getExecutableFile(&cpp.pData) == osl_Process_E_None);

#if defined(SAL_W32) || defined(SAL_OS2)
    cpp = cpp.copy(0, cpp.getLength() - 8);
    cpp += OUString( RTL_CONSTASCII_USTRINGPARAM("idlcpp.exe"));
#else
    cpp = cpp.copy(0, cpp.getLength() - 4);
    cpp += OUString( RTL_CONSTASCII_USTRINGPARAM("idlcpp"));
#endif

    oslProcess      hProcess = NULL;
    oslProcessError procError = osl_Process_E_None;

    procError = osl_executeProcess(cpp.pData, &cmdArg.pData, 1, osl_Process_WAIT,
                                   0, startDir.pData, 0, 0, &hProcess);

    oslProcessInfo hInfo;
    hInfo.Size = sizeof(oslProcessInfo);
    OSL_VERIFY( osl_getProcessInfo(hProcess, osl_Process_EXITCODE, &hInfo) == osl_Process_E_None );

    if ( procError || (hInfo.Code != 0) )
    {
        if ( procError != osl_Process_E_None )
            fprintf(stderr, "%s: starting preprocessor failed\n", pOptions->getProgramName().getStr());
        else
            fprintf(stderr, "%s: preprocessing \"%s\" failed\n", pOptions->getProgramName().getStr(), fileName.getStr());

        unlink(tmpFile.getStr());
        unlink(preprocFile.getStr());
        unlink(cmdFileName.getStr());
        osl_freeProcessHandle(hProcess);
        exit(hInfo.Code ? hInfo.Code : 99);
    }
    osl_freeProcessHandle(hProcess);

    if (unlink(tmpFile.getStr()) != 0)
    {
        fprintf(stderr, "%s: Could not remove cpp input file %s\n",
                 pOptions->getProgramName().getStr(), tmpFile.getStr());
        exit(99);
    }

    if (unlink(cmdFileName.getStr()) != 0)
    {
        fprintf(stderr, "%s: Could not remove unocpp command file %s\n",
                   pOptions->getProgramName().getStr(), cmdFileName.getStr());

        exit(99);
    }

    if ( pOptions->isValid("-E") )
    {
        if (unlink(preprocFile) != 0)
        {
            fprintf(stderr, "%s: Could not remove parser input file %s\n",
                       pOptions->getProgramName().getStr(), preprocFile.getStr());
            exit(99);
        }
        exit(0);
    }

    // parse file
    yyin = fopen(preprocFile.getStr(), "r");
    if (yyin == NULL)
    {
        fprintf(stderr, "%s: Could not open cpp output file %s\n",
                   pOptions->getProgramName().getStr(), preprocFile.getStr());
        exit(99);
    }

    //yydebug = 0 no trace information
    //yydebug = 1 parser produce trace information
    yydebug = 0;

    sal_Int32 nErrors = yyparse();
    nErrors = idlc()->getErrorCount();

    fclose(yyin);
    if (unlink(preprocFile.getStr()) != 0)
    {
        fprintf(stderr, "%s: Could not remove parser input file %s\n",
                pOptions->getProgramName().getStr(), preprocFile.getStr());
        exit(99);
    }

    return nErrors;
}
