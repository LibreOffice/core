/*************************************************************************
 *
 *  $RCSfile: tagtest.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:10:43 $
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

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

typedef USHORT Token;

#define TAG_GROUPMASK               0xF000
#define TAG_GROUPSHIFT              12

#define TAG_GROUP( nTag )           (( nTag & TAG_GROUPMASK ) >> TAG_GROUPSHIFT )
#define TAG_NOGROUP( nTag )         ( nTag & ~TAG_GROUPMASK )   // ~ = Bitweises NOT

#define TAG_NOMORETAGS              0x0

#define TAG_GROUP_FORMAT            0x1
#define TAG_ON                      0x100
#define TAG_BOLDON                  ( TAG_GROUP_FORMAT << TAG_GROUPSHIFT | TAG_ON | 0x001 )
#define TAG_BOLDOFF                 ( TAG_GROUP_FORMAT << TAG_GROUPSHIFT |          0x001 )
#define TAG_ITALICON                ( TAG_GROUP_FORMAT << TAG_GROUPSHIFT | TAG_ON | 0x002 )
#define TAG_ITALICOFF               ( TAG_GROUP_FORMAT << TAG_GROUPSHIFT |          0x002 )
#define TAG_UNDERLINEON             ( TAG_GROUP_FORMAT << TAG_GROUPSHIFT | TAG_ON | 0x004 )
#define TAG_UNDERLINEOFF            ( TAG_GROUP_FORMAT << TAG_GROUPSHIFT |          0x004 )

#define TAG_GROUP_NOTALLOWED        0x2
#define TAG_HELPID                  ( TAG_GROUP_NOTALLOWED << TAG_GROUPSHIFT | 0x001 )
#define TAG_MODIFY                  ( TAG_GROUP_NOTALLOWED << TAG_GROUPSHIFT | 0x002 )
#define TAG_REFNR                   ( TAG_GROUP_NOTALLOWED << TAG_GROUPSHIFT | 0x004 )

#define TAG_GROUP_STRUCTURE         0x3
#define TAG_NAME                    ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x001 )
#define TAG_HREF                    ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x002 )
#define TAG_AVIS                    ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x004 )
#define TAG_AHID                    ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x008 )

#define TAG_TITEL                   ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x020 )
#define TAG_KEY                     ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x040 )
#define TAG_INDEX                   ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x080 )

#define TAG_REFSTART                ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x100 )

#define TAG_GRAPHIC                 ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x200 )
#define TAG_NEXTVERSION             ( TAG_GROUP_STRUCTURE << TAG_GROUPSHIFT | 0x400 )

#define TAG_GROUP_SYSSWITCH         0x4
#define TAG_WIN                     ( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x001 )
#define TAG_UNIX                    ( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x002 )
#define TAG_MAC                     ( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x004 )
#define TAG_OS2                     ( TAG_GROUP_SYSSWITCH << TAG_GROUPSHIFT | 0x008 )

#define TAG_GROUP_PROGSWITCH        0x5
#define TAG_WRITER                  ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x001 )
#define TAG_CALC                    ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x002 )
#define TAG_DRAW                    ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x004 )
#define TAG_IMPRESS                 ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x008 )
#define TAG_SCHEDULE                ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x010 )
#define TAG_IMAGE                   ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x020 )
#define TAG_MATH                    ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x040 )
#define TAG_CHART                   ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x080 )
#define TAG_OFFICE                  ( TAG_GROUP_PROGSWITCH << TAG_GROUPSHIFT | 0x100 )


#define TAG_GROUP_META              0x6
#define TAG_OFFICEFULLNAME          ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x001 )
#define TAG_OFFICENAME              ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x002 )
#define TAG_OFFICEPATH              ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x004 )
#define TAG_OFFICEVERSION           ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x008 )
#define TAG_PORTALNAME              ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x010 )
#define TAG_PORTALFULLNAME          ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x020 )
#define TAG_PORTALPATH              ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x040 )
#define TAG_PORTALVERSION           ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x080 )
#define TAG_PORTALSHORTNAME         ( TAG_GROUP_META << TAG_GROUPSHIFT | 0x100 )


#define TAG_GROUP_SINGLE            0x7
#define TAG_REFINSERT               ( TAG_GROUP_SINGLE << TAG_GROUPSHIFT | 0x001 )


#define TAG_GROUP_MULTI             0x8
#define TAG_END                     ( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x010 )
#define TAG_ELSE                    ( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x020 )
#define TAG_AEND                    ( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x040 )
#define TAG_VERSIONEND              ( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x080 )
#define TAG_ENDGRAPHIC              ( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x100 )

#define TAG_UNKNOWN_TAG             ( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x800 )

DECLARE_LIST( TokenListImpl, Token* );

class TokenList : private TokenListImpl
{
private:

    TokenList&   operator =( const TokenList& rList )
                { TokenListImpl::operator =( rList ); return *this; }


public:
    TokenListImpl::Count;


    TokenList() : TokenListImpl(){};

    void        Clear()
        {
            for ( ULONG i = 0 ; i < Count() ; i++ )
                delete TokenListImpl::GetObject( i );
            TokenListImpl::Clear();
        }
    void        Insert( Token p, ULONG nIndex )
        { TokenListImpl::Insert( new Token(p), nIndex ); }
    Token       Remove( ULONG nIndex )
        {
            Token aT = GetObject( nIndex );
            delete TokenListImpl::GetObject( nIndex );
            TokenListImpl::Remove( nIndex );
            return aT;
        }
    Token       Remove( Token p ){ return Remove( GetPos( p ) ); }
//    Token     GetCurObject() const { return *TokenListImpl::GetCurObject(); }
    Token       GetObject( ULONG nIndex ) const
        {
            if ( TokenListImpl::GetObject(nIndex) )
                return *TokenListImpl::GetObject(nIndex);
            else
                return 0;
        }
    ULONG       GetPos( const Token p ) const
        {
            for ( ULONG i = 0 ; i < Count() ; i++ )
                if ( p == GetObject( i ) )
                    return i;
            return LIST_ENTRY_NOTFOUND;
        }

    TokenList( const TokenList& rList )
        {
            for ( ULONG i = 0 ; i < rList.Count() ; i++ )
            {
                Insert( rList.GetObject( i ), LIST_APPEND );
            }
        }
};

class SimpleParser
{
private:
    USHORT nPos;
    ByteString aSource;
    ByteString aLastToken;
    static ByteString aLastUnknownToken;
    TokenList aTokenList;

    ByteString GetNextTokenString();

public:
    SimpleParser();
    void Parse( ByteString PaSource );
    Token GetNextToken();
    ByteString GetTokenText();
    static ByteString GetLexem( Token aToken );
    USHORT GetScanningPosition(){ return nPos; }
    TokenList GetTokenList(){ return aTokenList; }
};

class ParserMessage
{
    USHORT nErrorNr;
    ByteString aErrorText;
    USHORT nTagBegin,nTagLength;
public:
    ParserMessage( USHORT PnErrorNr, const ByteString &PaErrorText, USHORT PnTagBegin = STRING_NOTFOUND, USHORT PnTagLength = 0)
        : nErrorNr( PnErrorNr )
        , aErrorText( PaErrorText )
        , nTagBegin( PnTagBegin )
        , nTagLength( PnTagLength )
    {}


    USHORT GetErrorNr() { return nErrorNr; }
    ByteString GetErrorText() { return aErrorText; }

    USHORT GetTagBegin() { return nTagBegin; }
    USHORT GetTagLength() { return nTagLength; }
};

DECLARE_LIST( ParserMessageList, ParserMessage* );

class TokenParser
{
    BOOL match( const Token &aCurrentToken, const Token &aExpectedToken );
    void ParseError( USHORT nErrNr, const ByteString &aErrMsg );
    void Paragraph();
    void PfCase();
    void PfCaseBegin();
    void AppCase();
    void AppCaseBegin();
    void CaseEnd();
    void SimpleTag();
    void TagPair();
    void TagRef();
    void Error( const ByteString &aMsg );

    SimpleParser aParser;
    Token nTag;

    Token nPfCaseOptions;
    Token nAppCaseOptions;
    BOOL bPfCaseActive ,bAppCaseActive;

    Token nActiveRefTypes;

    ParserMessageList aErrorList;

public:
    TokenParser();
    void Parse( const ByteString &aCode );
    ParserMessageList& GetErrors(){ return aErrorList; }
    BOOL HasErrors(){ return ( aErrorList.Count() ); }
    TokenList GetTokenList(){ return aParser.GetTokenList(); }
};

class LingTest
{
private:
    TokenParser aReferenceParser;
    TokenParser aTesteeParser;
    ParserMessageList aCompareWarningList;
    void CheckMandatoryTag( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList, Token aToken );
    void CheckTags( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList );
public:
    BOOL ReferenceOK( const ByteString &aReference );
    BOOL TesteeOK( const ByteString &aTestee );

    ParserMessageList& GetReferenceErrors(){ return aReferenceParser.GetErrors(); }
    BOOL HasReferenceErrors(){ return aReferenceParser.HasErrors(); }

    ParserMessageList& GetTesteeErrors(){ return aTesteeParser.GetErrors(); }
    BOOL HasTesteeErrors(){ return aTesteeParser.HasErrors(); }

    ParserMessageList& GetCompareWarnings(){ return aCompareWarningList; }
    BOOL HasCompareWarnings(){ return ( aCompareWarningList.Count() ); }
};

