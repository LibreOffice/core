/*************************************************************************
 *
 *  $RCSfile: idlc.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:23:01 $
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
#define _IDLC_IDLC_HXX_

#ifndef _IDLC_IDLCTYPES_HXX_
#include <idlc/idlctypes.hxx>
#endif
#ifndef _IDLC_ASTSTACK_HXX_
#include <idlc/aststack.hxx>
#endif
#ifndef _IDLC_OPTIONS_HXX_
#include <idlc/options.hxx>
#endif

class AstModule;
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
    sal_uInt32          m_errorCount;
    sal_uInt32          m_lineNumber;
    ParseState          m_parseState;
    StringSet           m_includes;
};

sal_Int32 SAL_CALL compileFile(const ::rtl::OString& fileName);
sal_Int32 SAL_CALL produceFile(const ::rtl::OString& fileName);
void SAL_CALL removeIfExists(const ::rtl::OString& fileName);
sal_Bool SAL_CALL canBeRedefined(AstDeclaration *pDecl);

class AstType;
AstType* SAL_CALL resolveTypeDef(AstType* pType);

Idlc* SAL_CALL idlc();
Idlc* SAL_CALL setIdlc(Options* pOptions);


#endif // _IDLC_IDLC_HXX_

