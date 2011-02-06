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

#ifndef _TAGTEST_HXX_
#define _TAGTEST_HXX_

#include <tools/string.hxx>
#include <tools/list.hxx>
#include <boost/unordered_map.hpp>
#include <vector>

class GSILine;

typedef USHORT TokenId;

#define TOK_INVALIDPOS  USHORT( 0xFFFF )

class ParserMessage;
typedef ::std::vector< ParserMessage* > Impl_ParserMessageList;

class ParserMessageList;

struct equalByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_EQUAL;
    }
};
struct lessByteString{
        bool operator()( const ByteString& rKey1, const ByteString& rKey2 ) const {
            return rKey1.CompareTo( rKey2 )==COMPARE_LESS;
    }
};

struct hashByteString{
    size_t operator()( const ByteString& rName ) const{
                boost::hash< const char* > myHash;
                return myHash( rName.GetBuffer() );
    }
};



typedef boost::unordered_map<ByteString , String , hashByteString,equalByteString>
                                StringHashMap;

class TokenInfo
{
private:
    void SplitTag( ParserMessageList &rErrorList );

    String aTagName;
    StringHashMap aProperties;
    BOOL bClosed;    // tag is closed  <sdnf/>
    BOOL bCloseTag;  // tag is close Tag  </sdnf>


    BOOL bIsBroken;
    BOOL bHasBeenFixed;
    BOOL bDone;

public:

    String aTokenString;
    TokenId nId;
    USHORT nPos;            // Position in String

    TokenInfo():bClosed(FALSE),bCloseTag(FALSE),bIsBroken(FALSE),bHasBeenFixed(FALSE),bDone(FALSE),nId( 0 ){;}
explicit    TokenInfo( TokenId pnId, USHORT nP ):bClosed(FALSE),bCloseTag(FALSE),bIsBroken(FALSE),bHasBeenFixed(FALSE),bDone(FALSE),nId( pnId ),nPos(nP){;}
explicit    TokenInfo( TokenId pnId, USHORT nP, String paStr ):bClosed(FALSE),bCloseTag(FALSE),bIsBroken(FALSE),bHasBeenFixed(FALSE),bDone(FALSE),aTokenString( paStr ),nId( pnId ),nPos(nP) {;}
explicit    TokenInfo( TokenId pnId, USHORT nP, String paStr, ParserMessageList &rErrorList );

    String GetTagName() const;

    String MakeTag() const;

    /**
        Is the property to be ignored or does it have the default value anyways
    **/
    BOOL IsPropertyRelevant( const ByteString &aName, const String &aValue ) const;
    BOOL IsPropertyValueValid( const ByteString &aName, const String &aValue ) const;
    /**
        Does the property contain the same value for all languages
        e.g.: the href in a link tag
    **/
    BOOL IsPropertyInvariant( const ByteString &aName, const String &aValue ) const;
    /**
        a subset of IsPropertyInvariant but containing only those that are fixable
        we dont wat to fix e.g.: ahelp :: visibility
    **/
    BOOL IsPropertyFixable( const ByteString &aName ) const;
    BOOL MatchesTranslation( TokenInfo& rInfo, BOOL bGenErrors, ParserMessageList &rErrorList, BOOL bFixTags = FALSE ) const;

    BOOL IsDone() const { return bDone; }
    void SetDone( BOOL bNew = TRUE ) { bDone = bNew; }

    BOOL HasBeenFixed() const { return bHasBeenFixed; }
    void SetHasBeenFixed( BOOL bNew = TRUE ) { bHasBeenFixed = bNew; }
};


class ParserMessageList
{
private:
    Impl_ParserMessageList maList;

public:
    ~ParserMessageList() { clear(); }
    void AddError( USHORT nErrorNr, ByteString aErrorText, const TokenInfo &rTag );
    void AddWarning( USHORT nErrorNr, ByteString aErrorText, const TokenInfo &rTag );

    BOOL HasErrors();
    bool empty() const { return maList.empty(); }
    size_t size() const { return maList.size(); }
    ParserMessage* operator [] ( size_t i ) { return ( i < maList.size() ) ? maList[ i ] : NULL; }
    void clear();
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

typedef ::std::vector< TokenInfo* > TokenListImpl;

class TokenList
{
private:
    TokenListImpl maList;
    TokenList&   operator =( const TokenList& rList );

public:
    TokenList() {};
    ~TokenList(){ clear(); };

    size_t size() const { return maList.size(); }
    void clear()
        {
            for ( size_t i = 0 ; i < maList.size() ; i++ )
                delete maList[ i ];
            maList.clear();
        }

    void insert( TokenInfo p, size_t nIndex = LIST_APPEND )
        {
            if ( nIndex < maList.size() ) {
                TokenListImpl::iterator it = maList.begin();
                ::std::advance( it, nIndex );
                maList.insert( it, new TokenInfo(p) );
            } else {
                maList.push_back( new TokenInfo(p) );
            }
        }
    TokenInfo& operator [] ( size_t nIndex ) const
        {
            return *maList[ nIndex ];
        }

    TokenList( const TokenList& rList );
};

class ParserMessage
{
    USHORT nErrorNr;
    ByteString aErrorText;
    USHORT nTagBegin,nTagLength;

protected:
    ParserMessage( USHORT PnErrorNr, ByteString PaErrorText, const TokenInfo &rTag );
public:

    USHORT GetErrorNr() { return nErrorNr; }
    ByteString GetErrorText() { return aErrorText; }

    USHORT GetTagBegin() { return nTagBegin; }
    USHORT GetTagLength() { return nTagLength; }

    virtual ~ParserMessage() {}
    virtual BOOL IsError() =0;
    virtual ByteString Prefix() =0;
};

class ParserError : public ParserMessage
{
public:
    ParserError( USHORT PnErrorNr, ByteString PaErrorText, const TokenInfo &rTag );

    virtual BOOL IsError() {return TRUE;};
    virtual ByteString Prefix() {return "Error:"; };
};

class ParserWarning : public ParserMessage
{
public:
    ParserWarning( USHORT PnErrorNr, ByteString PaErrorText, const TokenInfo &rTag );

    virtual BOOL IsError() {return FALSE;};
    virtual ByteString Prefix() {return "Warning:"; };
};

class SimpleParser
{
private:
    USHORT nPos;
    String aSource;
    String aLastToken;
    TokenList aTokenList;

    TokenInfo aNextTag;     // to store closetag in case of combined tags like <br/>

    String GetNextTokenString( ParserMessageList &rErrorList, USHORT &rTokeStartPos );

public:
    SimpleParser();
    void Parse( String PaSource );
    TokenInfo GetNextToken( ParserMessageList &rErrorList );
    static String GetLexem( TokenInfo const &aToken );
    TokenList& GetTokenList(){ return aTokenList; }
};

class TokenParser
{
    BOOL match( const TokenInfo &aCurrentToken, const TokenId &aExpectedToken );
    BOOL match( const TokenInfo &aCurrentToken, const TokenInfo &aExpectedToken );
    void ParseError( USHORT nErrNr, ByteString aErrMsg, const TokenInfo &rTag );
    void Paragraph();
    void PfCase();
    void PfCaseBegin();
    void AppCase();
    void AppCaseBegin();
    void CaseEnd();
    void SimpleTag();
    void TagPair();
    void TagRef();

    SimpleParser aParser;
    TokenInfo aTag;

    TokenId nPfCaseOptions;
    TokenId nAppCaseOptions;
    BOOL bPfCaseActive ,bAppCaseActive;

    TokenId nActiveRefTypes;

    ParserMessageList *pErrorList;

public:
    TokenParser();
    void Parse( const String &aCode, ParserMessageList* pList );
    TokenList& GetTokenList(){ return aParser.GetTokenList(); }
};

class LingTest
{
private:
    TokenParser aReferenceParser;
    TokenParser aTesteeParser;
    ParserMessageList aCompareWarningList;
    void CheckTags( TokenList &aReference, TokenList &aTestee, BOOL bFixTags );
    BOOL IsTagMandatory( TokenInfo const &aToken, TokenId &aMetaTokens );
    String aFixedTestee;
public:
    void CheckReference( GSILine *aReference );
    void CheckTestee( GSILine *aTestee, BOOL bHasSourceLine, BOOL bFixTags );

    ParserMessageList& GetCompareWarnings(){ return aCompareWarningList; }
    BOOL HasCompareWarnings(){ return ( !aCompareWarningList.empty() ); }

    String GetFixedTestee(){ return aFixedTestee; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
