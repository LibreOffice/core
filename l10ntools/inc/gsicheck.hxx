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

#ifndef _GSICHECK_HXX_
#define _GSICHECK_HXX_

#include "sal/config.h"

#include <cstddef>
#include <vector>
#include <fstream>

#include "tagtest.hxx"

//
// class GSILine
//
enum LineFormat { FORMAT_SDF, FORMAT_UNKNOWN };

class GSILine
{
private:
    ParserMessageList aMessages;
    LineFormat aFormat;
    std::size_t nLineNumber;

    rtl::OString aUniqId;
    rtl::OString aLineType;
    rtl::OString aLangId;
    rtl::OString aText;
    rtl::OString aQuickHelpText;
    rtl::OString aTitle;

    sal_Bool bOK;
    sal_Bool bFixed;

    void              ReassembleLine();

public:
    rtl::OString data_;

    GSILine( const rtl::OString &rLine, std::size_t nLine );
    LineFormat  GetLineFormat() const    { return aFormat; }
    std::size_t GetLineNumber() const    { return nLineNumber; }

    rtl::OString  const GetUniqId()     const    { return aUniqId; }
    rtl::OString  const GetLineType()   const    { return aLineType; }
    rtl::OString  const GetLanguageId() const    { return aLangId; }
    rtl::OString  const GetText()       const    { return aText; }
    rtl::OUString  const GetUText()      const    { return rtl::OStringToOUString( aText, RTL_TEXTENCODING_UTF8 ); }
    rtl::OString  const GetQuickHelpText() const { return aQuickHelpText; }
    rtl::OString  const GetTitle()      const    { return aTitle; }

    void SetUText( rtl::OUString const &aNew ) { aText = rtl::OUStringToOString(aNew, RTL_TEXTENCODING_UTF8); ReassembleLine(); }
    void        SetText( rtl::OString const &aNew ) { aText = aNew; ReassembleLine(); }
    void        SetQuickHelpText( rtl::OString const &aNew ) { aQuickHelpText = aNew; ReassembleLine(); }
    void        SetTitle( rtl::OString const &aNew ) { aTitle = aNew; ReassembleLine(); }

    ParserMessageList* GetMessageList() { return &aMessages; };
    sal_Bool HasMessages(){ return ( aMessages.size() > 0 ); };

    sal_Bool IsOK() const { return bOK; }
    void NotOK();

    sal_Bool IsFixed() const { return bFixed; }
    void SetFixed() { bFixed = sal_True; };
};

//
// class GSIBlock
//

typedef std::vector< GSILine* > GSIBlock_Impl;

class LazyStream;

class GSIBlock
{
private:
    GSIBlock_Impl maList;
    GSILine *pSourceLine;
    GSILine *pReferenceLine;
    void PrintList( ParserMessageList *pList, rtl::OString const & aPrefix, GSILine *pLine );
    sal_Bool bPrintContext;
    sal_Bool bCheckSourceLang;
    sal_Bool bCheckTranslationLang;
    sal_Bool bReference;
    sal_Bool bAllowSuspicious;

    sal_Bool bHasBlockError;

    sal_Bool IsUTF8( const rtl::OString &aTestee, sal_Bool bFixTags, sal_Int32 &nErrorPos, rtl::OString &aErrorMsg, sal_Bool &bHasBeenFixed, rtl::OString &aFixed ) const;
    sal_Bool TestUTF8( GSILine* pTestee, sal_Bool bFixTags );
    sal_Bool HasSuspiciousChars( GSILine* pTestee, GSILine* pSource );

public:
    GSIBlock( sal_Bool PbPrintContext, sal_Bool bSource, sal_Bool bTrans, sal_Bool bRef, sal_Bool bAllowSusp );
    ~GSIBlock();
    void PrintMessage( rtl::OString const & aType, rtl::OString const & aMsg, rtl::OString const & aPrefix, rtl::OString const & aContext, std::size_t nLine, rtl::OString const & aUniqueId = rtl::OString() );
    void PrintError( rtl::OString const & aMsg, rtl::OString const & aPrefix, rtl::OString const & aContext, std::size_t nLine, rtl::OString const & aUniqueId = rtl::OString() );
    void InsertLine( GSILine* pLine, const rtl::OString &rSourceLang);
    void SetReferenceLine( GSILine* pLine );
    sal_Bool CheckSyntax( std::size_t nLine, sal_Bool bRequireSourceLine, sal_Bool bFixTags );

    void WriteError( LazyStream &aErrOut, sal_Bool bRequireSourceLine );
    void WriteCorrect( LazyStream &aOkOut, sal_Bool bRequireSourceLine );
    void WriteFixed( LazyStream &aFixOut );
};

sal_Bool check(rtl::OString s,std::size_t nLine);

void PrintMessage( rtl::OString const & aType, rtl::OString const & aMsg, rtl::OString const & aPrefix,
                   rtl::OString const & aContext, sal_Bool bPrintContext, std::size_t nLine, rtl::OString aUniqueId = rtl::OString() );

void PrintError( rtl::OString const & aMsg, rtl::OString const & aPrefix,
                 rtl::OString const & aContext, sal_Bool bPrintContext, std::size_t nLine, rtl::OString const & aUniqueId = rtl::OString() );

bool LanguageOK( rtl::OString const & aLang );

class LazyStream: public std::ofstream
{

private:
    rtl::OString aFileName;
    bool bOpened;

public:
    LazyStream()
    : aFileName()
    , bOpened(false)
    {};

    void SetFileName( const rtl::OString& rFileName )
    {
        aFileName = rFileName;
    };

    void LazyOpen();
};

namespace {

sal_Int32 const MAX_GID_LID_LEN = 250;

rtl::OString copyUpTo(
    rtl::OString const & text, sal_Int32 start, sal_Int32 maximumLength)
{
    assert(start >= 0 && start <= text.getLength());
    return text.copy(start, std::min(text.getLength() - start, maximumLength));
}

rtl::OString addSuffix(
    rtl::OString const & pathname, rtl::OString const & suffix)
{
    sal_Int32 n = pathname.lastIndexOf('.');
    if (n == -1) {
        fprintf(
            stderr,
            ("Error: pathname \"%s\" does not contain dot to add suffix in"
             " front of\n"),
            pathname.getStr());
        exit(EXIT_FAILURE);
    }
    return pathname.replaceAt(n, 0, suffix);
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
