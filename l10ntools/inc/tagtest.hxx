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

#ifndef _TAGTEST_HXX_
#define _TAGTEST_HXX_

#include <boost/unordered_map.hpp>
#include <vector>

class GSILine;

typedef sal_Int32 TokenId;

#define TOK_INVALIDPOS (-1)

class ParserMessage;
typedef ::std::vector< ParserMessage* > Impl_ParserMessageList;

class ParserMessageList;

typedef boost::unordered_map<rtl::OString, rtl::OUString, rtl::OStringHash> StringHashMap;

class TokenInfo
{
private:
    void SplitTag( ParserMessageList &rErrorList );

    rtl::OUString aTagName;
    StringHashMap aProperties;
    sal_Bool bClosed;    // tag is closed  <sdnf/>
    sal_Bool bCloseTag;  // tag is close Tag  </sdnf>


    sal_Bool bIsBroken;
    sal_Bool bHasBeenFixed;
    sal_Bool bDone;

public:

    rtl::OUString aTokenString;
    TokenId nId;
    sal_Int32 nPos;            // Position in String

    TokenInfo():bClosed(sal_False),bCloseTag(sal_False),bIsBroken(sal_False),bHasBeenFixed(sal_False),bDone(sal_False),nId( 0 ){;}
explicit    TokenInfo( TokenId pnId, sal_Int32 nP ):bClosed(sal_False),bCloseTag(sal_False),bIsBroken(sal_False),bHasBeenFixed(sal_False),bDone(sal_False),nId( pnId ),nPos(nP){;}
    explicit    TokenInfo( TokenId pnId, sal_Int32 nP, rtl::OUString const & paStr ):bClosed(sal_False),bCloseTag(sal_False),bIsBroken(sal_False),bHasBeenFixed(sal_False),bDone(sal_False),aTokenString( paStr ),nId( pnId ),nPos(nP) {;}
    explicit    TokenInfo( TokenId pnId, sal_Int32 nP, rtl::OUString const & paStr, ParserMessageList &rErrorList );

    rtl::OUString GetTagName() const;

    rtl::OUString MakeTag() const;

    /**
        Is the property to be ignored or does it have the default value anyways
    **/
    sal_Bool IsPropertyRelevant( const rtl::OString &rName, const rtl::OUString &rValue ) const;
    sal_Bool IsPropertyValueValid( const rtl::OString &rName, const rtl::OUString &rValue ) const;
    /**
        Does the property contain the same value for all languages
        e.g.: the href in a link tag
    **/
    sal_Bool IsPropertyInvariant( const rtl::OString &rName, const rtl::OUString &rValue ) const;
    /**
        a subset of IsPropertyInvariant but containing only those that are fixable
        we dont wat to fix e.g.: ahelp :: visibility
    **/
    sal_Bool IsPropertyFixable( const rtl::OString &rName ) const;
    sal_Bool MatchesTranslation( TokenInfo& rInfo, sal_Bool bGenErrors, ParserMessageList &rErrorList, sal_Bool bFixTags = sal_False ) const;

    sal_Bool IsDone() const { return bDone; }
    void SetDone( sal_Bool bNew = sal_True ) { bDone = bNew; }

    sal_Bool HasBeenFixed() const { return bHasBeenFixed; }
    void SetHasBeenFixed( sal_Bool bNew = sal_True ) { bHasBeenFixed = bNew; }
};


class ParserMessageList
{
private:
    Impl_ParserMessageList maList;

public:
    ~ParserMessageList() { clear(); }
    void AddError( sal_Int32 nErrorNr, const rtl::OString& rErrorText, const TokenInfo &rTag );
    void AddWarning( sal_Int32 nErrorNr, const rtl::OString& rErrorText, const TokenInfo &rTag );

    sal_Bool HasErrors();
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
    TokenList() {}
    ~TokenList(){ clear(); }

    size_t size() const { return maList.size(); }
    void clear()
        {
            for ( size_t i = 0 ; i < maList.size() ; i++ )
                delete maList[ i ];
            maList.clear();
        }

    void insert( TokenInfo p, size_t nIndex = size_t(-1) )
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
    sal_Int32 nErrorNr;
    rtl::OString aErrorText;
    sal_Int32 nTagBegin,nTagLength;

protected:
    ParserMessage( sal_Int32 PnErrorNr, const rtl::OString &rPaErrorText, const TokenInfo &rTag );
public:

    sal_Int32 GetErrorNr() { return nErrorNr; }
    rtl::OString GetErrorText() { return aErrorText; }

    sal_Int32 GetTagBegin() { return nTagBegin; }
    sal_Int32 GetTagLength() { return nTagLength; }

    virtual ~ParserMessage() {}
    virtual sal_Bool IsError() =0;
    virtual rtl::OString Prefix() =0;
};

class ParserError : public ParserMessage
{
public:
    ParserError( sal_Int32 PnErrorNr, const rtl::OString &rPaErrorText, const TokenInfo &rTag );

    virtual sal_Bool IsError() {return sal_True;}
    virtual rtl::OString Prefix() {return rtl::OString(RTL_CONSTASCII_STRINGPARAM("Error:")); }
};

class ParserWarning : public ParserMessage
{
public:
    ParserWarning( sal_Int32 PnErrorNr, const rtl::OString &rPaErrorText, const TokenInfo &rTag );

    virtual sal_Bool IsError() {return sal_False;}
    virtual rtl::OString Prefix() {return rtl::OString(RTL_CONSTASCII_STRINGPARAM("Warning:")); }
};

class SimpleParser
{
private:
    sal_Int32 nPos;
    rtl::OUString aSource;
    rtl::OUString aLastToken;
    TokenList aTokenList;

    TokenInfo aNextTag;     // to store closetag in case of combined tags like <br/>

    rtl::OUString GetNextTokenString( ParserMessageList &rErrorList, sal_Int32 &rTokeStartPos );

public:
    SimpleParser();
    void Parse( rtl::OUString const & PaSource );
    TokenInfo GetNextToken( ParserMessageList &rErrorList );
    static rtl::OUString GetLexem( TokenInfo const &aToken );
    TokenList& GetTokenList(){ return aTokenList; }
};

class TokenParser
{
    sal_Bool match( const TokenInfo &aCurrentToken, const TokenId &aExpectedToken );
    sal_Bool match( const TokenInfo &aCurrentToken, const TokenInfo &aExpectedToken );
    void ParseError( sal_Int32 nErrNr, const rtl::OString &rErrMsg, const TokenInfo &rTag );
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
    sal_Bool bPfCaseActive ,bAppCaseActive;

    TokenId nActiveRefTypes;

    ParserMessageList *pErrorList;

public:
    TokenParser();
    void Parse( const rtl::OUString &aCode, ParserMessageList* pList );
    TokenList& GetTokenList(){ return aParser.GetTokenList(); }
};

class LingTest
{
private:
    TokenParser aReferenceParser;
    TokenParser aTesteeParser;
    ParserMessageList aCompareWarningList;
    void CheckTags( TokenList &aReference, TokenList &aTestee, sal_Bool bFixTags );
    sal_Bool IsTagMandatory( TokenInfo const &aToken, TokenId &aMetaTokens );
    rtl::OUString aFixedTestee;
public:
    void CheckReference( GSILine *aReference );
    void CheckTestee( GSILine *aTestee, sal_Bool bHasSourceLine, sal_Bool bFixTags );

    ParserMessageList& GetCompareWarnings(){ return aCompareWarningList; }
    sal_Bool HasCompareWarnings(){ return ( !aCompareWarningList.empty() ); }

    rtl::OUString GetFixedTestee(){ return aFixedTestee; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
