/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gsicheck.hxx,v $
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

#ifndef _GSICHECK_HXX_
#define _GSICHECK_HXX_

#include "tagtest.hxx"

//
// class GSILine
//
enum LineFormat { FORMAT_GSI, FORMAT_SDF, FORMAT_UNKNOWN };

class GSILine : public ByteString
{
private:

    ParserMessageList aMessages;
    LineFormat aFormat;
    ULONG nLineNumber;

    ByteString aUniqId;
    ByteString aLineType;
    ByteString aLangId;
    ByteString aText;
    ByteString aQuickHelpText;
    ByteString aTitle;

    BOOL bOK;
    BOOL bFixed;

    void              ReassembleLine();

public:
    GSILine( const ByteString &rLine, ULONG nLine );
    LineFormat  const GetLineFormat() const    { return aFormat; }
    ULONG       const GetLineNumber() const    { return nLineNumber; }

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
    BOOL HasMessages(){ return ( aMessages.Count() > 0 ); };

    BOOL const IsOK() { return bOK; }
    void NotOK();

    BOOL const IsFixed() { return bFixed; }
    void SetFixed() { bFixed = TRUE; };
};

//
// class GSIBlock
//

DECLARE_LIST( GSIBlock_Impl, GSILine * )

class LazySvFileStream;

class GSIBlock : public GSIBlock_Impl
{
private:
    GSILine *pSourceLine;
    GSILine *pReferenceLine;
    void PrintList( ParserMessageList *pList, ByteString aPrefix, GSILine *pLine );
    BOOL bPrintContext;
    BOOL bCheckSourceLang;
    BOOL bCheckTranslationLang;
    BOOL bReference;
    BOOL bAllowKeyIDs;
    BOOL bAllowSuspicious;

    BOOL bHasBlockError;

    BOOL IsUTF8( const ByteString &aTestee, BOOL bFixTags, USHORT &nErrorPos, ByteString &aErrorMsg, BOOL &bHasBeenFixed, ByteString &aFixed ) const;
    BOOL TestUTF8( GSILine* pTestee, BOOL bFixTags );
    BOOL HasSuspiciousChars( GSILine* pTestee, GSILine* pSource );

public:
    GSIBlock( BOOL PbPrintContext, BOOL bSource, BOOL bTrans, BOOL bRef, BOOL bAllowKID, BOOL bAllowSusp );
    ~GSIBlock();
    void PrintMessage( ByteString aType, ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ByteString aUniqueId = ByteString() );
    void PrintError( ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ByteString aUniqueId = ByteString() );
    void InsertLine( GSILine* pLine, const ByteString aSourceLang);
    void SetReferenceLine( GSILine* pLine );
    BOOL CheckSyntax( ULONG nLine, BOOL bRequireSourceLine, BOOL bFixTags );

    void WriteError( LazySvFileStream &aErrOut, BOOL bRequireSourceLine );
    void WriteCorrect( LazySvFileStream &aOkOut, BOOL bRequireSourceLine );
    void WriteFixed( LazySvFileStream &aFixOut, BOOL bRequireSourceLine );
};

#endif

