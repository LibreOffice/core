/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    sal_Bool HasMessages(){ return ( aMessages.Count() > 0 ); };

    sal_Bool IsOK() const { return bOK; }
    void NotOK();

    sal_Bool IsFixed() const { return bFixed; }
    void SetFixed() { bFixed = sal_True; };
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

