/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: idlc.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _IDLC_IDLC_HXX_
#define _IDLC_IDLC_HXX_

#include <idlc/idlctypes.hxx>
#include <idlc/aststack.hxx>
#include <idlc/options.hxx>

#ifdef SAL_UNX
#define SEPARATOR '/'
#define PATH_SEPARATOR "/"
#else
#define SEPARATOR '\\'
#define PATH_SEPARATOR "\\"
#endif

class AstInterface;
class AstModule;
class AstType;
class Options;
class ErrorHandler;

class Idlc
{
public:
    Idlc(Options* pOptions);
    virtual ~Idlc();

    void init();

    Options* getOptions()
        { return m_pOptions; }
    AstStack* scopes()
        { return m_pScopes; }
    AstModule* getRoot()
        { return m_pRoot; }
    ErrorHandler* error()
        { return m_pErrorHandler; }
    const ::rtl::OString& getFileName()
        { return m_fileName; }
    void setFileName(const ::rtl::OString& fileName)
        { m_fileName = fileName; }
    const ::rtl::OString& getMainFileName()
        { return m_mainFileName; }
    void setMainFileName(const ::rtl::OString& mainFileName)
        { m_mainFileName = mainFileName; }
    const ::rtl::OString& getRealFileName()
        { return m_realFileName; }
    void setRealFileName(const ::rtl::OString& realFileName)
        { m_realFileName = realFileName; }
    const ::rtl::OString& getDocumentation()
        {
            m_bIsDocValid = sal_False;
            return m_documentation;
        }
    void setDocumentation(const ::rtl::OString& documentation)
        {
            m_documentation = documentation;
            m_bIsDocValid = sal_True;
        }
    sal_Bool isDocValid();
    sal_Bool isInMainFile()
        { return m_bIsInMainfile; }
    void setInMainfile(sal_Bool bInMainfile)
        { m_bIsInMainfile = bInMainfile; }
    sal_uInt32 getErrorCount()
        { return m_errorCount; }
    void setErrorCount(sal_uInt32 errorCount)
        { m_errorCount = errorCount; }
    void incErrorCount()
        { m_errorCount++; }
    sal_uInt32 getWarningCount()
        { return m_warningCount; }
    void setWarningCount(sal_uInt32 warningCount)
        { m_warningCount = warningCount; }
    void incWarningCount()
        { m_warningCount++; }
    sal_uInt32 getLineNumber()
        { return m_lineNumber; }
    void setLineNumber(sal_uInt32 lineNumber)
        { m_lineNumber = lineNumber; }
    void incLineNumber()
        { m_lineNumber++; }
    ParseState getParseState()
        { return m_parseState; }
    void setParseState(ParseState parseState)
        { m_parseState = parseState; }

    void insertInclude(const ::rtl::OString& inc)
        { m_includes.insert(inc); }
    StringSet* getIncludes()
        { return &m_includes; }

    void setPublished(bool published) { m_published = published; }
    bool isPublished() const { return m_published; }

    void reset();
private:
    Options*            m_pOptions;
    AstStack*           m_pScopes;
    AstModule*          m_pRoot;
    ErrorHandler*       m_pErrorHandler;
    ::rtl::OString      m_fileName;
    ::rtl::OString      m_mainFileName;
    ::rtl::OString      m_realFileName;
    ::rtl::OString      m_documentation;
    sal_Bool            m_bIsDocValid;
    sal_Bool            m_bGenerateDoc;
    sal_Bool            m_bIsInMainfile;
    bool                m_published;
    sal_uInt32          m_errorCount;
    sal_uInt32          m_warningCount;
    sal_uInt32          m_lineNumber;
    ParseState          m_parseState;
    StringSet           m_includes;
};

sal_Int32 compileFile(const ::rtl::OString * pathname);
    // a null pathname means stdin
sal_Int32 produceFile(const ::rtl::OString& filenameBase);
    // filenameBase is filename without ".idl"
void removeIfExists(const ::rtl::OString& pathname);

::rtl::OString makeTempName(const ::rtl::OString& prefix, const ::rtl::OString& postfix);
sal_Bool copyFile(const ::rtl::OString* source, const ::rtl::OString& target);
    // a null source means stdin

sal_Bool isFileUrl(const ::rtl::OString& fileName);
::rtl::OString convertToAbsoluteSystemPath(const ::rtl::OString& fileName);
::rtl::OString convertToFileUrl(const ::rtl::OString& fileName);

Idlc* SAL_CALL idlc();
Idlc* SAL_CALL setIdlc(Options* pOptions);

AstDeclaration const * resolveTypedefs(AstDeclaration const * type);

AstDeclaration const * deconstructAndResolveTypedefs(
    AstDeclaration const * type, sal_Int32 * rank);

AstInterface const * resolveInterfaceTypedefs(AstType const * type);

#endif // _IDLC_IDLC_HXX_

