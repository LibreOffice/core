/*************************************************************************
 *
 *  $RCSfile: gsicheck.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-07 13:24:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

DECLARE_LIST( GSIBlock_Impl, GSILine * );

class SvStream;

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
    BOOL bHasBlockError;

    BOOL IsUTF8( const ByteString &aTestee, USHORT &nErrorPos, ByteString &aErrorMsg ) const;
    BOOL TestUTF8( GSILine* pTestee );

public:
    GSIBlock( BOOL PbPrintContext, BOOL bSource, BOOL bTrans, BOOL bRef );
    ~GSIBlock();
    void PrintMessage( ByteString aType, ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ByteString aUniqueId = ByteString() );
    void PrintError( ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ByteString aUniqueId = ByteString() );
    void InsertLine( GSILine* pLine, const ByteString aSourceLang);
    void SetReferenceLine( GSILine* pLine );
    BOOL CheckSyntax( ULONG nLine, BOOL bRequireSourceLine, BOOL bFixTags );

    void WriteError( SvStream &aErrOut, BOOL bRequireSourceLine );
    void WriteCorrect( SvStream &aOkOut, BOOL bRequireSourceLine );
    void WriteFixed( SvStream &aFixOut, BOOL bRequireSourceLine );
};

#endif

