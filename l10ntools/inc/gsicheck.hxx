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

#ifndef _GSICHECK_HXX_
#define _GSICHECK_HXX_

#include "sal/config.h"

#include <cstddef>
#include <vector>

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
