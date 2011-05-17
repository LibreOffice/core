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

#include "tagtest.hxx"
#include <vector>

//
// class GSILine
//
enum LineFormat { FORMAT_GSI, FORMAT_SDF, FORMAT_UNKNOWN };

class GSILine : public ByteString
{
private:

    ParserMessageList aMessages;
    LineFormat aFormat;
    sal_uLong nLineNumber;

    ByteString aUniqId;
    ByteString aLineType;
    ByteString aLangId;
    ByteString aText;
    ByteString aQuickHelpText;
    ByteString aTitle;

    sal_Bool bOK;
    sal_Bool bFixed;

    void              ReassembleLine();

public:
    GSILine( const ByteString &rLine, sal_uLong nLine );
    LineFormat  GetLineFormat() const    { return aFormat; }
    sal_uLong       GetLineNumber() const    { return nLineNumber; }

    ByteString  const GetUniqId()     const    { return aUniqId; }
    ByteString  const GetLineType()   const    { return aLineType; }
    ByteString  const GetLanguageId() const    { return aLangId; }
    ByteString  const GetText()       const    { return aText; }
        String  const GetUText()      const    { return String( aText, RTL_TEXTENCODING_UTF8 ); }
    ByteString  const GetQuickHelpText() const { return aQuickHelpText; }
    ByteString  const GetTitle()      const    { return aTitle; }

          void        SetUText( String &aNew ) { aText = ByteString( aNew, RTL_TEXTENCODING_UTF8 ); ReassembleLine(); }
          void        SetText( ByteString &aNew ) { aText = aNew; ReassembleLine(); }
          void        SetQuickHelpText( ByteString &aNew ) { aQuickHelpText = aNew; ReassembleLine(); }
          void        SetTitle( ByteString &aNew ) { aTitle = aNew; ReassembleLine(); }

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

typedef ::std::vector< GSILine* > GSIBlock_Impl;

class LazySvFileStream;

class GSIBlock
{
private:
    GSIBlock_Impl maList;
    GSILine *pSourceLine;
    GSILine *pReferenceLine;
    void PrintList( ParserMessageList *pList, ByteString aPrefix, GSILine *pLine );
    sal_Bool bPrintContext;
    sal_Bool bCheckSourceLang;
    sal_Bool bCheckTranslationLang;
    sal_Bool bReference;
    sal_Bool bAllowKeyIDs;
    sal_Bool bAllowSuspicious;

    sal_Bool bHasBlockError;

    sal_Bool IsUTF8( const ByteString &aTestee, sal_Bool bFixTags, sal_uInt16 &nErrorPos, ByteString &aErrorMsg, sal_Bool &bHasBeenFixed, ByteString &aFixed ) const;
    sal_Bool TestUTF8( GSILine* pTestee, sal_Bool bFixTags );
    sal_Bool HasSuspiciousChars( GSILine* pTestee, GSILine* pSource );

public:
    GSIBlock( sal_Bool PbPrintContext, sal_Bool bSource, sal_Bool bTrans, sal_Bool bRef, sal_Bool bAllowKID, sal_Bool bAllowSusp );
    ~GSIBlock();
    void PrintMessage( ByteString aType, ByteString aMsg, ByteString aPrefix, ByteString aContext, sal_uLong nLine, ByteString aUniqueId = ByteString() );
    void PrintError( ByteString aMsg, ByteString aPrefix, ByteString aContext, sal_uLong nLine, ByteString aUniqueId = ByteString() );
    void InsertLine( GSILine* pLine, const ByteString aSourceLang);
    void SetReferenceLine( GSILine* pLine );
    sal_Bool CheckSyntax( sal_uLong nLine, sal_Bool bRequireSourceLine, sal_Bool bFixTags );

    void WriteError( LazySvFileStream &aErrOut, sal_Bool bRequireSourceLine );
    void WriteCorrect( LazySvFileStream &aOkOut, sal_Bool bRequireSourceLine );
    void WriteFixed( LazySvFileStream &aFixOut, sal_Bool bRequireSourceLine );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
