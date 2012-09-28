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

#include <idlc/idlc.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.hxx>

#if defined(SAL_W32)
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

    if ( !uTmpPath.isEmpty() )
        tmpPath = OUStringToOString(uTmpPath, RTL_TEXTENCODING_UTF8);

#if defined(SAL_W32) || defined(SAL_UNX)

    OSL_ASSERT( sizeof(tmpFilePattern) >
                (size_t) ( tmpPath.getLength()
                           + RTL_CONSTASCII_LENGTH( PATH_SEPARATOR )
                           + prefix.getLength()
                           + RTL_CONSTASCII_LENGTH( "XXXXXX") ) );

    tmpFilePattern[ sizeof(tmpFilePattern)-1 ] = '\0';
    strncpy(tmpFilePattern, tmpPath.getStr(), sizeof(tmpFilePattern)-1);
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
    char   pBuffer[513];

    while ( !feof(pSource) )
    {
        if ( (readSize = fread(pBuffer, 1, totalSize, pSource)) > 0 && !ferror(pSource) )
        {
            if ( (fwrite(pBuffer, 1, readSize, pTarget)) != readSize || ferror(pTarget) )
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

    ::std::vector< ::rtl::OUString> lCppArgs;
    lCppArgs.push_back("-DIDL");
    lCppArgs.push_back("-C");
    lCppArgs.push_back("-zI");

    OStringBuffer cppArgs(256);
    Options* pOptions = idlc()->getOptions();

    OString filePath;
    sal_Int32 index = fileName.lastIndexOf(SEPARATOR);

    if ( index > 0)
    {
        filePath = fileName.copy(0, index);

        if ( !filePath.isEmpty() )
        {
            cppArgs.append("-I");
            cppArgs.append(filePath);
            lCppArgs.push_back(rtl::OStringToOUString(
                cppArgs.makeStringAndClear().replace('\\', '/'),
                RTL_TEXTENCODING_UTF8));
        }
    }

    if ( pOptions->isValid("-D") )
    {
        OString token, dOpt = pOptions->getOption("-D");
        sal_Int32 nIndex = 0;
        do
        {
            token = dOpt.getToken( 0, ' ', nIndex );
            if (token.getLength())
                lCppArgs.push_back(rtl::OStringToOUString("-D" + token, RTL_TEXTENCODING_UTF8));
        } while( nIndex != -1 );
    }

	if ( pOptions->isValid("-I") )
    {
        OString token, incOpt = pOptions->getOption("-I");
        sal_Int32 nIndex = 0;
        do
        {
            token = incOpt.getToken( 0, ' ', nIndex );
            if (token.getLength())
                lCppArgs.push_back(rtl::OStringToOUString("-I" + token, RTL_TEXTENCODING_UTF8));
        } while( nIndex != -1 );
    }

    lCppArgs.push_back(OUString(RTL_CONSTASCII_USTRINGPARAM("-o")));

	cppArgs.append(preprocFile);
    lCppArgs.push_back(OStringToOUString(cppArgs.makeStringAndClear(), RTL_TEXTENCODING_UTF8));

    cppArgs.append(tmpFile);
    lCppArgs.push_back(OStringToOUString(cppArgs.makeStringAndClear(), RTL_TEXTENCODING_UTF8));

    OUString cpp;
    OUString startDir;
#ifndef SYSTEM_UCPP
    if (osl_getExecutableFile(&cpp.pData) != osl_Process_E_None) {
        OSL_ASSERT(false);
    }

    sal_Int32 idx= cpp.lastIndexOf("idlc");
    cpp = cpp.copy(0, idx);

#if defined(SAL_W32)
    cpp += OUString( RTL_CONSTASCII_USTRINGPARAM("ucpp.exe"));
#else
    cpp += OUString( RTL_CONSTASCII_USTRINGPARAM("ucpp"));
#endif
#else // SYSTEM_UCPP
    cpp = OUString( RTL_CONSTASCII_USTRINGPARAM(UCPP));
#endif
    oslProcess      hProcess = NULL;
    oslProcessError procError = osl_Process_E_None;

    const int nCmdArgs = lCppArgs.size();
    rtl_uString** pCmdArgs = 0;
    pCmdArgs = (rtl_uString**)rtl_allocateZeroMemory(nCmdArgs * sizeof(rtl_uString*));

    ::std::vector< ::rtl::OUString >::iterator iter = lCppArgs.begin();
    ::std::vector< ::rtl::OUString >::iterator end = lCppArgs.end();
    int i = 0;
	while ( iter != end ) {
        pCmdArgs[i++] = (*iter).pData;
        ++iter;
    }

	procError = osl_executeProcess( cpp.pData, pCmdArgs, nCmdArgs, osl_Process_WAIT,
                                    0, startDir.pData, 0, 0, &hProcess );

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

        osl_freeProcessHandle(hProcess);
        rtl_freeMemory(pCmdArgs);
        exit(hInfo.Code ? hInfo.Code : 99);
    }
    osl_freeProcessHandle(hProcess);
    rtl_freeMemory(pCmdArgs);

    if (unlink(tmpFile.getStr()) != 0)
    {
        fprintf(stderr, "%s: Could not remove cpp input file %s\n",
                 pOptions->getProgramName().getStr(), tmpFile.getStr());
        exit(99);
    }

    if ( pOptions->isValid("-E") )
    {
        if (unlink(preprocFile.getStr()) != 0)
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
