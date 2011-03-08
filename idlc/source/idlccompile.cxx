/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_idlc.hxx"
#include <idlc/idlc.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.hxx>

#if defined(SAL_W32) || defined(SAL_OS2)
#include <io.h>
#endif

#ifdef  SAL_UNX
#include <errno.h>
#include <unistd.h>
#if defined(MACOSX) || defined(FREEBSD) || defined(NETBSD) || \
    defined(AIX) || defined(OPENBSD) || defined(DRAGONFLY)
#include <sys/wait.h>
#else
#include <wait.h>
#endif
#endif

#include <string.h>

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
        if (FileBase::getSystemPathFromFileURL(uFileName, uSysFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    } else
    {
        OUString uWorkingDir, uUrlFileName, uTmp;
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None)
        {
            OSL_ASSERT(false);
        }
        if (FileBase::getFileURLFromSystemPath(uFileName, uTmp)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
        if (FileBase::getAbsoluteFileURL(uWorkingDir, uTmp, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
        if (FileBase::getSystemPathFromFileURL(uUrlFileName, uSysFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    }

    return OUStringToOString(uSysFileName, osl_getThreadTextEncoding());
}

OString convertToFileUrl(const OString& fileName)
{
    if ( !isFileUrl(fileName) )
    {
        OString tmp = convertToAbsoluteSystemPath(fileName);
        OUString uFileName(tmp.getStr(), tmp.getLength(), osl_getThreadTextEncoding());
        OUString uUrlFileName;
        if (FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
        return OUStringToOString(uUrlFileName, osl_getThreadTextEncoding());
    }

    return fileName;
}

OString makeTempName(const OString& prefix)
{
    OUString uTmpPath;
    OString tmpPath;

    if ( osl_getEnvironment(TMP.pData, &uTmpPath.pData) != osl_Process_E_None )
    {
        if ( osl_getEnvironment(TEMP.pData, &uTmpPath.pData) != osl_Process_E_None )
        {
#if defined(SAL_W32)
            tmpPath = OString("c:\\temp");
#else
            tmpPath = OString("/tmp");
#endif
        }
    }

    if ( uTmpPath.getLength() )
        tmpPath = OUStringToOString(uTmpPath, RTL_TEXTENCODING_UTF8);

#if defined(SAL_W32) || defined(SAL_UNX) || defined(SAL_OS2)

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
        fprintf(stderr, "idlc: mkstemp(\"%s\") failed: %s\n", tmpFilePattern, strerror(errno));
        exit( 1 );
    }
    // the file shall later be reopened by stdio functions
    close( nDescriptor );
#else
    (void) mktemp(tmpFilePattern);
#endif
#endif

    return OString(tmpFilePattern);
}

sal_Bool copyFile(const OString* source, const OString& target)
{
    sal_Bool bRet = sal_True;

    FILE* pSource = source == 0 ? stdin : fopen(source->getStr(), "rb");

    if ( !pSource )
        return sal_False;

    FILE* pTarget = fopen(target.getStr(), "wb");

    if ( !pTarget )
    {
        fclose(pSource);
        return sal_False;
    }

    size_t totalSize = 512;
    size_t readSize  = 0;
    size_t writeSize = 0;
    char   pBuffer[513];

    while ( !feof(pSource) )
    {
        if ( (readSize = fread(pBuffer, 1, totalSize, pSource)) > 0 && !ferror(pSource) )
        {
            if ( (writeSize = fwrite(pBuffer, 1, readSize, pTarget)) != readSize || ferror(pTarget) )
            {
                if (source != 0) {
                    fclose(pSource);
                }
                fclose(pTarget);
                return sal_False;
            }
        }
    }

    if (source != 0) {
        fclose(pSource);
    }
    if ( fflush(pTarget) )
        bRet = sal_False;
    fclose(pTarget);

    return bRet;
}

sal_Int32 compileFile(const OString * pathname)
{
    // preprocess input file
    OString tmpFile = makeTempName(OString("idli_"));
    OString preprocFile = makeTempName(OString("idlf_"));

    OString fileName;
    if (pathname == 0) {
        fileName = "stdin";
    } else {
        fileName = *pathname;
    }

    if ( !copyFile(pathname, tmpFile) )
    {
          fprintf(stderr, "%s: could not copy %s%s to %s\n",
                idlc()->getOptions()->getProgramName().getStr(),
                pathname == 0 ? "" : "file ", fileName.getStr(),
                tmpFile.getStr());
          exit(99);
    }

    idlc()->setFileName(fileName);
    idlc()->setMainFileName(fileName);
    idlc()->setRealFileName(tmpFile);

    OStringBuffer cppArgs(512);
    cppArgs.append("-DIDL -Xi -Xc -+ -I.");
    Options* pOptions = idlc()->getOptions();

    OString filePath;
    sal_Int32 index = fileName.lastIndexOf(SEPARATOR);

    if ( index > 0)
    {
        filePath = fileName.copy(0, index);

        if ( filePath.getLength() )
        {
            cppArgs.append(" -I\"");
            cppArgs.append(filePath);
            cppArgs.append("\"");
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

    cppArgs.append(" \"");
    cppArgs.append(tmpFile);
    cppArgs.append("\" \"");
    cppArgs.append(preprocFile);
    cppArgs.append("\"");

    OString cmdFileName = makeTempName(OString("idlc_"));
    FILE* pCmdFile = fopen(cmdFileName, "w");

    if ( !pCmdFile )
    {
          fprintf(stderr, "%s: couldn't open temporary file for preprocessor commands: %s\n",
            idlc()->getOptions()->getProgramName().getStr(), cmdFileName.getStr());
          exit(99);
    }
#ifdef SAL_OS2_00
      char* tok = strtok( (char*)cppArgs.getStr(), " \t\n\r");
      while( tok) {
         if (tok[strlen(tok)-1] == '\"')
            tok[strlen(tok)-1] = '\0';
         if (*tok == '\"')
            memcpy( tok, tok+1, strlen(tok));
         if (strlen(tok)>0) {
            fputs(tok, pCmdFile);
            fputc('\n', pCmdFile);
         }
         tok = strtok( NULL, " \t\n\r");
      }
#else
    fprintf(pCmdFile, "%s", cppArgs.getStr());
#endif
    fclose(pCmdFile);

    OUString cmdArg(RTL_CONSTASCII_USTRINGPARAM("@"));
    cmdArg += OStringToOUString(cmdFileName, RTL_TEXTENCODING_UTF8);

    OUString cpp;
    OUString startDir;
    if (osl_getExecutableFile(&cpp.pData) != osl_Process_E_None) {
        OSL_ASSERT(false);
    }

    sal_Int32 idx= cpp.lastIndexOf(OUString( RTL_CONSTASCII_USTRINGPARAM("idlc")) );
     cpp = cpp.copy(0, idx);

#if defined(SAL_W32) || defined(SAL_OS2)
     cpp += OUString( RTL_CONSTASCII_USTRINGPARAM("idlcpp.exe"));
#else
    cpp += OUString( RTL_CONSTASCII_USTRINGPARAM("idlcpp"));
#endif

    oslProcess      hProcess = NULL;
    oslProcessError procError = osl_Process_E_None;

    procError = osl_executeProcess(cpp.pData, &cmdArg.pData, 1, osl_Process_WAIT,
                                   0, startDir.pData, 0, 0, &hProcess);

    oslProcessInfo hInfo;
    hInfo.Size = (sal_uInt32)(sizeof(oslProcessInfo));
    if (osl_getProcessInfo(hProcess, osl_Process_EXITCODE, &hInfo)
        != osl_Process_E_None)
    {
        OSL_ASSERT(false);
    }

    if ( procError || (hInfo.Code != 0) )
    {
        if ( procError != osl_Process_E_None )
            fprintf(stderr, "%s: starting preprocessor failed\n", pOptions->getProgramName().getStr());
        else
            fprintf(stderr, "%s: preprocessing %s%s failed\n",
                    pOptions->getProgramName().getStr(),
                    pathname == 0 ? "" : "file ", fileName.getStr());

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
