/*************************************************************************
 *
 *  $RCSfile: tagtest.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-12-10 17:16:43 $
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

typedef USHORT TokenId;

struct TokenInfo
{
    ByteString aName;
    TokenId nId;

    TokenInfo():nId( 0 ){;}
explicit    TokenInfo( TokenId pnId ):nId( pnId ){;}
explicit    TokenInfo( TokenId pnId, ByteString paStr ):nId( pnId ), aName( paStr ){;}
    BOOL operator == ( const TokenInfo& rInfo ) const
    { return nId == rInfo.nId  && aName.Equals( rInfo.aName ); }
};


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

#define TAG_GROUP_MISC              0x9
#define TAG_COMMONSTART             ( TAG_GROUP_MISC << TAG_GROUPSHIFT | 0x001 )
#define TAG_COMMONEND               ( TAG_GROUP_MISC << TAG_GROUPSHIFT | 0x002 )

#define TAG_UNKNOWN_TAG             ( TAG_GROUP_MULTI << TAG_GROUPSHIFT | 0x800 )

DECLARE_LIST( TokenListImpl, TokenInfo* );

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
    void        Insert( TokenInfo p, ULONG nIndex )
        { TokenListImpl::Insert( new TokenInfo(p), nIndex ); }
    TokenInfo       Remove( ULONG nIndex )
        {
            TokenInfo aT = GetObject( nIndex );
            delete TokenListImpl::GetObject( nIndex );
            TokenListImpl::Remove( nIndex );
            return aT;
        }
    TokenInfo       Remove( TokenInfo p ){ return Remove( GetPos( p ) ); }
//    TokenInfo     GetCurObject() const { return *TokenListImpl::GetCurObject(); }
    TokenInfo       GetObject( ULONG nIndex ) const
        {
            if ( TokenListImpl::GetObject(nIndex) )
                return *TokenListImpl::GetObject(nIndex);
            else
                return TokenInfo();
        }
    ULONG       GetPos( const TokenInfo p ) const
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

class SimpleParser
{
private:
    USHORT nPos;
    ByteString aSource;
    ByteString aLastToken;
//  static ByteString aLastUnknownToken;
    TokenList aTokenList;

    TokenInfo aNextTag;     // to store closetag in case of combined tags like <br/>

    ByteString GetNextTokenString( ParserMessageList &rErrorList );

public:
    SimpleParser();
    void Parse( ByteString PaSource );
    TokenInfo GetNextToken( ParserMessageList &rErrorList );
    ByteString GetTokenText();
    static ByteString GetLexem( TokenInfo const &aToken );
    USHORT GetScanningPosition(){ return nPos; }
    TokenList GetTokenList(){ return aTokenList; }
};

class TokenParser
{
    BOOL match( const TokenInfo &aCurrentToken, const TokenId &aExpectedToken );
    BOOL match( const TokenInfo &aCurrentToken, const TokenInfo &aExpectedToken );
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
    TokenInfo aTag;

    TokenId nPfCaseOptions;
    TokenId nAppCaseOptions;
    BOOL bPfCaseActive ,bAppCaseActive;

    TokenId nActiveRefTypes;

    ParserMessageList aErrorList;

public:
    TokenParser();
    void Parse( const ByteString &aCode );
    ParserMessageList& GetErrors(){ return aErrorList; }
    BOOL HasErrors(){ return ( aErrorList.Count() > 0 ); }
    TokenList GetTokenList(){ return aParser.GetTokenList(); }
};

class LingTest
{
private:
    TokenParser aReferenceParser;
    TokenParser aTesteeParser;
    ParserMessageList aCompareWarningList;
    void CheckMandatoryTag( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList, TokenInfo aToken );
    void CheckTags( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList );
    BOOL IsTagMandatory( TokenInfo const &aToken, TokenId &aMetaTokens );
public:
    BOOL ReferenceOK( const ByteString &aReference );
    BOOL TesteeOK( const ByteString &aTestee, BOOL bHasSourceLine );

    ParserMessageList& GetReferenceErrors(){ return aReferenceParser.GetErrors(); }
    BOOL HasReferenceErrors(){ return aReferenceParser.HasErrors(); }

    ParserMessageList& GetTesteeErrors(){ return aTesteeParser.GetErrors(); }
    BOOL HasTesteeErrors(){ return aTesteeParser.HasErrors(); }

    ParserMessageList& GetCompareWarnings(){ return aCompareWarningList; }
    BOOL HasCompareWarnings(){ return ( aCompareWarningList.Count() > 0 ); }
};

