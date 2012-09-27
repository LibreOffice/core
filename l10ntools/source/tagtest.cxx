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

#include "sal/config.h"

#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.hxx"

#include "tagtest.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include "gsicheck.hxx"
#include "helper.hxx"

#define HAS_FLAG( nFlags, nFlag )       ( ( nFlags & nFlag ) != 0 )
#define SET_FLAG( nFlags, nFlag )       ( nFlags |= nFlag )
#define RESET_FLAG( nFlags, nFlag )     ( nFlags &= ~nFlag )    // ~ = Bitweises NOT



TokenInfo::TokenInfo( TokenId pnId, sal_Int32 nP, rtl::OUString const & paStr, ParserMessageList &rErrorList )
: bClosed(sal_False)
, bCloseTag(sal_False)
, bIsBroken(sal_False)
, bHasBeenFixed(sal_False)
, bDone(sal_False)
, aTokenString( paStr )
, nId( pnId )
, nPos(nP)
{
    if ( nId == TAG_COMMONSTART || nId == TAG_COMMONEND )
        SplitTag( rErrorList );
}

enum tagcheck { TC_START, TC_HAS_TAG_NAME, TC_HAS_PROP_NAME_EQ, TC_HAS_PROP_NAME_EQ_SP, TC_HAS_PROP_NAME_SP, TC_INSIDE_STRING, TC_PROP_FINISHED, TC_CLOSED, TC_CLOSED_SPACE, TC_CLOSETAG, TC_CLOSETAG_HAS_TAG_NAME, TC_FINISHED, TC_ERROR };

/*
                                                      \<  link  href  =  \"text\"  name  =  \"C\"  \>
START               ' ' ->  HAS_TAG_NAME
START               '/' ->  CLOSED
START               '/' ->  CLOSETAG    - no Portion (starting with /)
START               '>' ->  FINISHED
HAS_TAG_NAME        '=' ->  HAS_PROP_NAME_EQ
HAS_TAG_NAME        ' ' ->  HAS_PROP_NAME_SP
HAS_TAG_NAME        '/' ->  CLOSED
HAS_TAG_NAME        '>' ->  FINISHED
HAS_PROP_NAME_SP    '=' ->  HAS_PROP_NAME_EQ
HAS_PROP_NAME_EQ    ' ' ->  HAS_PROP_NAME_EQ_SP
HAS_PROP_NAME_EQ    '"' ->  INSIDE_STRING
HAS_PROP_NAME_EQ_SP '"' ->  INSIDE_STRING
INSIDE_STRING       ' ' ->  INSIDE_STRING
INSIDE_STRING       '=' ->  INSIDE_STRING
INSIDE_STRING       '>' ->  INSIDE_STRING
INSIDE_STRING       '"' ->  PROP_FINISHED
PROP_FINISHED       ' ' ->  HAS_TAG_NAME
PROP_FINISHED       '/' ->  CLOSED
PROP_FINISHED       '>' ->  FINISHED
CLOSED              ' ' ->  CLOSED_SPACE
CLOSED              '>' ->  FINISHED
CLOSED_SPACE        '>' ->  FINISHED

CLOSETAG            ' ' ->  CLOSETAG_HAS_TAG_NAME
CLOSETAG            '>' ->  FINISHED
CLOSETAG_HAS_TAG_NAME  '>' ->  FINISHED

*/
void TokenInfo::SplitTag( ParserMessageList &rErrorList )
{
    sal_Int32 nLastPos = 2;    // skip initial  \<
    sal_Int32 nCheckPos = nLastPos;
    static char const aDelims[] = " \\=>/";
    rtl::OUString aPortion;
    rtl::OUString aValue; // store the value of a property
    rtl::OString aName; // store the name of a property/tag
    sal_Bool bCheckName = sal_False;
    sal_Bool bCheckEmpty = sal_False;
    sal_Unicode cDelim;
    tagcheck aState = TC_START;

    // skip blanks
    while ( nLastPos < aTokenString.getLength() && aTokenString[nLastPos] == ' ')
        nLastPos++;

    nCheckPos = helper::indexOfAnyAsciiL(
        aTokenString, RTL_CONSTASCII_STRINGPARAM(aDelims), nLastPos);
    while ( nCheckPos != -1 && !( aState == TC_FINISHED || aState == TC_ERROR ) )
    {
        aPortion = aTokenString.copy( nLastPos, nCheckPos-nLastPos );

        if ( aTokenString[nCheckPos] == '\\' )
            nCheckPos++;

        cDelim = aTokenString[nCheckPos];
        nCheckPos++;

        switch ( aState )
        {
//            START           ' ' ->  HAS_TAG_NAME
//            START           '/' ->  CLOSED
//            START           '>' ->  FINISHED
            case TC_START:
                aTagName = aPortion;
                switch ( cDelim )
                {
                    case ' ':  aState = TC_HAS_TAG_NAME;
                               bCheckName = sal_True;
                               break;
                    case '/':
                        {
                            if (aPortion.isEmpty())
                            {
                                aState = TC_CLOSETAG;
                            }
                            else
                            {
                                aState = TC_CLOSED;
                                bCheckName = sal_True;
                            }
                        }
                        break;
                    case '>':  aState = TC_FINISHED;
                               bCheckName = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

//            HAS_TAG_NAME    '=' ->  HAS_PROP_NAME_EQ
//            HAS_TAG_NAME    ' ' ->  HAS_PROP_NAME_SP
//            HAS_TAG_NAME    '/' ->  CLOSED
//            HAS_TAG_NAME    '>' ->  FINISHED
            case TC_HAS_TAG_NAME:
                switch ( cDelim )
                {
                    case '=':  aState = TC_HAS_PROP_NAME_EQ;
                               bCheckName = sal_True;
                               break;
                    case ' ':  aState = TC_HAS_PROP_NAME_SP;
                               bCheckName = sal_True;
                               break;
                    case '/':  aState = TC_CLOSED;
                               bCheckEmpty = sal_True;
                               break;
                    case '>':  aState = TC_FINISHED;
                               bCheckEmpty = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

//            HAS_PROP_NAME_SP    '=' ->  HAS_PROP_NAME_EQ
            case TC_HAS_PROP_NAME_SP:
                switch ( cDelim )
                {
                    case '=':  aState = TC_HAS_PROP_NAME_EQ;
                               bCheckEmpty = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

//            HAS_PROP_NAME_EQ    ' ' ->  HAS_PROP_NAME_EQ_SP
//            HAS_PROP_NAME_EQ    '"' ->  INSIDE_STRING
            case TC_HAS_PROP_NAME_EQ:
                switch ( cDelim )
                {
                    case ' ':  aState = TC_HAS_PROP_NAME_EQ_SP;
                               bCheckEmpty = sal_True;
                               break;
                    case '\"': aState = TC_INSIDE_STRING;
                               bCheckEmpty = sal_True;
                               aValue = rtl::OUString();
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

//            HAS_PROP_NAME_EQ_SP '"' ->  INSIDE_STRING
            case TC_HAS_PROP_NAME_EQ_SP:
                switch ( cDelim )
                {
                    case '\"': aState = TC_INSIDE_STRING;
                               bCheckEmpty = sal_True;
                               aValue = rtl::OUString();
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

//            INSIDE_STRING    *  ->  INSIDE_STRING
//            INSIDE_STRING   '"' ->  PROP_FINISHED
            case TC_INSIDE_STRING:
                switch ( cDelim )
                {
                    case '\"':
                        {
                            aState = TC_PROP_FINISHED;
                            aValue += aPortion;
                            if ( aProperties.find( aName ) == aProperties.end() )
                            {
                                if ( !IsPropertyValueValid( aName, aValue ) )
                                {
                                    rErrorList.AddError( 25, rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Property '")).append(aName).append(RTL_CONSTASCII_STRINGPARAM("' has invalid value '")).append(rtl::OUStringToOString(aValue, RTL_TEXTENCODING_UTF8)).append("' ").makeStringAndClear(), *this );
                                    bIsBroken = sal_True;
                                }
                                aProperties[ aName ] = aValue;
                            }
                            else
                            {
                                rErrorList.AddError( 25, rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Property '")).append(aName).append(RTL_CONSTASCII_STRINGPARAM("' defined twice ")).makeStringAndClear(), *this );
                                bIsBroken = sal_True;
                            }
                        }
                               break;
                    default:
                        {
                            aState = TC_INSIDE_STRING;
                            aValue += aPortion;
                            aValue += rtl::OUString(cDelim);
                        }
                }
                break;

//            PROP_FINISHED   ' ' ->  HAS_TAG_NAME
//            PROP_FINISHED   '/' ->  CLOSED
//            PROP_FINISHED   '>' ->  FINISHED
            case TC_PROP_FINISHED:
                switch ( cDelim )
                {
                    case ' ': aState = TC_HAS_TAG_NAME;
                               bCheckEmpty = sal_True;
                               break;
                    case '/': aState = TC_CLOSED;
                               bCheckEmpty = sal_True;
                               break;
                    case '>': aState = TC_FINISHED;
                               bCheckEmpty = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

//            CLOSED          ' ' ->  CLOSED_SPACE
//            CLOSED          '>' ->  FINISHED
            case TC_CLOSED:
                switch ( cDelim )
                {
                    case ' ': aState = TC_CLOSED_SPACE;
                               bCheckEmpty = sal_True;
                               bClosed = sal_True;
                               break;
                    case '>': aState = TC_FINISHED;
                               bCheckEmpty = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

//            CLOSED_SPACE    '>' ->  FINISHED
            case TC_CLOSED_SPACE:
                switch ( cDelim )
                {
                    case '>': aState = TC_FINISHED;
                               bCheckEmpty = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

// CLOSETAG            ' ' ->  CLOSETAG_HAS_TAG_NAME
// CLOSETAG            '>' ->  FINISHED
            case TC_CLOSETAG:
                bCloseTag = sal_True;
                switch ( cDelim )
                {
                    case ' ': aState = TC_CLOSETAG_HAS_TAG_NAME;
                               aTagName = aPortion;
                               bCheckName = sal_True;
                               break;
                    case '>': aState = TC_FINISHED;
                               aTagName = aPortion;
                               bCheckName = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;

// CLOSETAG_HAS_TAG_NAME       '>' ->  FINISHED
            case TC_CLOSETAG_HAS_TAG_NAME:
                switch ( cDelim )
                {
                    case '>': aState = TC_FINISHED;
                               bCheckEmpty = sal_True;
                               break;
                    default:   aState = TC_ERROR;
                }
                break;


            default: rErrorList.AddError( 99, "Internal error Parsing Tag ", *this );
                     bIsBroken = sal_True;

        }

        if ( bCheckName )
        {
            if (aPortion.isEmpty())
            {
                rErrorList.AddError( 25, "Tag/Property name missing ", *this );
                bIsBroken = sal_True;
            }
            else
            {
                aName = rtl::OUStringToOString(aPortion, RTL_TEXTENCODING_UTF8);
                // "a-zA-Z_-.0-9"
                sal_Bool bBroken = sal_False;
                const sal_Char* aBuf = aName.getStr();
                for (sal_Int32 nCount = 0 ; !bBroken && nCount < aName.getLength() ; ++nCount)
                {
                    bBroken = ! (   ( aBuf[nCount] >= 'a' && aBuf[nCount] <= 'z' )
                                ||( aBuf[nCount] >= 'A' && aBuf[nCount] <= 'Z' )
                                ||( aBuf[nCount] >= '0' && aBuf[nCount] <= '9' )
                                ||( aBuf[nCount] == '_' )
                                ||( aBuf[nCount] == '-' )
                                ||( aBuf[nCount] == '.' )
                                );
                }

                if ( bBroken )
                {
                    rErrorList.AddError( 25, "Found illegal character in Tag/Property name ", *this );
                    bIsBroken = sal_True;
                }
            }

            bCheckName = sal_False;
        }

        if ( bCheckEmpty )
        {
            if (!aPortion.isEmpty())
            {
                rErrorList.AddError( 25, rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Found displaced characters '")).append(rtl::OUStringToOString(aPortion, RTL_TEXTENCODING_UTF8)).append(RTL_CONSTASCII_STRINGPARAM("' in Tag ")).makeStringAndClear(), *this );
                bIsBroken = sal_True;
            }
            bCheckEmpty = sal_False;
        }


        nLastPos = nCheckPos;

        // skip further blanks
        if ( cDelim == ' ' && aState != TC_INSIDE_STRING )
            while ( nLastPos < aTokenString.getLength() && aTokenString[nLastPos] == ' ')
                nLastPos++;

        nCheckPos = helper::indexOfAnyAsciiL(
            aTokenString, RTL_CONSTASCII_STRINGPARAM(aDelims), nLastPos);
    }
    if ( aState != TC_FINISHED )
    {
        rErrorList.AddError( 25, "Parsing error in Tag ", *this );
        bIsBroken = sal_True;
    }
}

sal_Bool TokenInfo::IsPropertyRelevant( const rtl::OString &rName, const rtl::OUString &rValue ) const
{
    if ( aTagName == "alt" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("xml-lang")) )
        return sal_False;
    if ( aTagName == "ahelp" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("visibility")) && rValue == "visible" )
        return sal_False;
    if ( aTagName == "image" && (rName.equalsL(RTL_CONSTASCII_STRINGPARAM("width")) || rName.equalsL(RTL_CONSTASCII_STRINGPARAM("height"))) )
        return sal_False;

    return sal_True;
}

sal_Bool TokenInfo::IsPropertyValueValid( const rtl::OString &rName, const rtl::OUString &rValue ) const
{
/*  removed due to i56740
    if ( aTagName.EqualsAscii( "switchinline" ) && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("select")) )
    {
        return rValue.EqualsAscii("sys") ||
               rValue.EqualsAscii("appl") ||
               rValue.EqualsAscii("distrib");
    } */
    if ( aTagName == "caseinline" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("select")) )
    {
        return !rValue.isEmpty();
    }

    // we don't know any better so we assume it to be OK
    return sal_True;
}

sal_Bool TokenInfo::IsPropertyInvariant( const rtl::OString &rName, const rtl::OUString &rValue ) const
{
    if ( aTagName == "link" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("name")) )
        return sal_False;
    if ( aTagName == "link" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("href")) )
    {   // check for external reference
        return
            !(rValue.matchIgnoreAsciiCaseAsciiL(
                  RTL_CONSTASCII_STRINGPARAM("http:"))
              || rValue.matchIgnoreAsciiCaseAsciiL(
                  RTL_CONSTASCII_STRINGPARAM("https:"))
              || rValue.matchIgnoreAsciiCaseAsciiL(
                  RTL_CONSTASCII_STRINGPARAM("ftp:")));
    }
    return sal_True;
}

sal_Bool TokenInfo::IsPropertyFixable( const rtl::OString &rName ) const
{
    // name everything that is allowed to be fixed automatically here
    if ( (aTagName == "ahelp" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("hid")))
         || (aTagName == "link" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("href")))
         || (aTagName == "alt" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("id")))
         || (aTagName == "variable" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("id")))
         || (aTagName == "image" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("src")))
         || (aTagName == "image" && rName.equalsL(RTL_CONSTASCII_STRINGPARAM("id")) ))
        return sal_True;
    return sal_False;
}

sal_Bool TokenInfo::MatchesTranslation( TokenInfo& rInfo, sal_Bool bGenErrors, ParserMessageList &rErrorList, sal_Bool bFixTags ) const
{
    // check if tags are equal
    // check if all existing properties are in the translation as well and
    // whether they have a matching content (the same in most cases)

    if ( nId != rInfo.nId )
        return sal_False;

    if ( aTagName != rInfo.aTagName )
        return sal_False;

    // If one of the tags has formating errors already it does make no sense to check here, so return right away
    if ( bGenErrors && ( bIsBroken || rInfo.bIsBroken ) )
        return sal_True;

    StringHashMap::const_iterator iProp;
    for( iProp = aProperties.begin() ; iProp != aProperties.end(); ++iProp )
    {
        if ( rInfo.aProperties.find( iProp->first ) != rInfo.aProperties.end() )
        {
            if ( IsPropertyRelevant( iProp->first, iProp->second ) || IsPropertyRelevant( iProp->first, rInfo.aProperties.find( iProp->first )->second ) )
            {
                if ( IsPropertyInvariant( iProp->first, iProp->second ) )
                {
                    if ( rInfo.aProperties.find( iProp->first )->second != iProp->second )
                    {
                        if ( bGenErrors )
                        {
                            if ( bFixTags && IsPropertyFixable( iProp->first ) )
                            {
                                rInfo.aProperties.find( iProp->first )->second = iProp->second;
                                rInfo.SetHasBeenFixed();
                                rErrorList.AddWarning( 25, rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Property '")).append(iProp->first).append(RTL_CONSTASCII_STRINGPARAM("': FIXED different value in Translation ")).makeStringAndClear(), *this );
                            }
                            else
                                rErrorList.AddError( 25, rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Property '")).append(iProp->first).append(RTL_CONSTASCII_STRINGPARAM("': value different in Translation ")).makeStringAndClear(), *this );
                        }
                        else return sal_False;
                    }
                }
            }
        }
        else
        {
            if ( IsPropertyRelevant( iProp->first, iProp->second ) )
            {
                if ( bGenErrors )
                    rErrorList.AddError( 25, rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Property '")).append(iProp->first).append(RTL_CONSTASCII_STRINGPARAM("' missing in Translation ")).makeStringAndClear(), *this );
                else return sal_False;
            }
        }
    }
    for( iProp = rInfo.aProperties.begin() ; iProp != rInfo.aProperties.end(); ++iProp )
    {
        if ( aProperties.find( iProp->first ) == aProperties.end() )
        {
            if ( IsPropertyRelevant( iProp->first, iProp->second ) )
            {
                if ( bGenErrors )
                    rErrorList.AddError( 25, rtl::OStringBuffer(RTL_CONSTASCII_STRINGPARAM("Extra Property '")).append(iProp->first).append(RTL_CONSTASCII_STRINGPARAM("' in Translation ")).makeStringAndClear(), rInfo );
                else return sal_False;
            }
        }
    }

    // if we reach here eather
    //   the tags match completely or
    //   the tags match but not the properties and we generated errors for that
    return sal_True;
}

rtl::OUString TokenInfo::GetTagName() const
{
    return aTagName;
}

rtl::OUString TokenInfo::MakeTag() const
{
    rtl::OUStringBuffer aRet;
    aRet.appendAscii("\\<");
    if ( bCloseTag )
        aRet.appendAscii("/");
    aRet.append( GetTagName() );
    StringHashMap::const_iterator iProp;

    for( iProp = aProperties.begin() ; iProp != aProperties.end(); ++iProp )
    {
        aRet.appendAscii(" ");
        aRet.append( rtl::OStringToOUString( iProp->first, RTL_TEXTENCODING_UTF8 ) );
        aRet.appendAscii("=\\\"");
        aRet.append( iProp->second );
        aRet.appendAscii("\\\"");
    }
    if ( bClosed )
        aRet.appendAscii("/");
    aRet.appendAscii("\\>");
    return aRet.makeStringAndClear();
}


void ParserMessageList::AddError( sal_Int32 nErrorNr, const rtl::OString& rErrorText, const TokenInfo &rTag )
{
    maList.push_back( new ParserError( nErrorNr, rErrorText, rTag ) );
}

void ParserMessageList::AddWarning( sal_Int32 nErrorNr, const rtl::OString& rErrorText, const TokenInfo &rTag )
{
    maList.push_back( new ParserWarning( nErrorNr, rErrorText, rTag ) );
}

sal_Bool ParserMessageList::HasErrors()
{
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
        if ( maList[ i ]->IsError() )
            return sal_True;
    return sal_False;
}

void ParserMessageList::clear()
{
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
        delete maList[ i ];
    maList.clear();
}

struct Tag
{
    rtl::OUString GetName() const { return rtl::OUString::createFromAscii( pName ); };
    const char* pName;
    TokenId nTag;
};


static const Tag aKnownTags[] =
{
/*  commenting oldstyle tags
//  { "<#GROUP_FORMAT>", TAG_GROUP_FORMAT },
    { "<#BOLD>", TAG_BOLDON },
    { "<#/BOLD>", TAG_BOLDOFF },
    { "<#ITALIC>", TAG_ITALICON },
    { "<#/ITALIC>", TAG_ITALICOFF },
    { "<#UNDER>", TAG_UNDERLINEON },
    { "<#/UNDER>", TAG_UNDERLINEOFF },

//  { "<#GROUP_NOTALLOWED>", TAG_GROUP_NOTALLOWED },
    { "<#HELPID>", TAG_HELPID },
    { "<#MODIFY>", TAG_MODIFY },
    { "<#REFNR>", TAG_REFNR },

//  { "<#GROUP_STRUCTURE>", TAG_GROUP_STRUCTURE },
    { "<#NAME>", TAG_NAME },
    { "<#HREF>", TAG_HREF },
    { "<#AVIS>", TAG_AVIS },
    { "<#AHID>", TAG_AHID },
    { "<#AEND>", TAG_AEND },

    { "<#TITEL>", TAG_TITEL },
    { "<#KEY>", TAG_KEY },
    { "<#INDEX>", TAG_INDEX },

    { "<#REFSTART>", TAG_REFSTART },

    { "<#GRAPHIC>", TAG_GRAPHIC },
    { "<#NEXTVERSION>", TAG_NEXTVERSION },

    //  { "<#GROUP_SYSSWITCH>", TAG_GROUP_SYSSWITCH },
    { "<#WIN>", TAG_WIN },
    { "<#UNIX>", TAG_UNIX },
    { "<#MAC>", TAG_MAC },
    { "<#OS2>", TAG_OS2 },

//  { "<#GROUP_PROGSWITCH>", TAG_GROUP_PROGSWITCH },
    { "<#WRITER>", TAG_WRITER },
    { "<#CALC>", TAG_CALC },
    { "<#DRAW>", TAG_DRAW },
    { "<#IMPRESS>", TAG_IMPRESS },
    { "<#SCHEDULE>", TAG_SCHEDULE },
    { "<#IMAGE>", TAG_IMAGE },
    { "<#MATH>", TAG_MATH },
    { "<#CHART>", TAG_CHART },
    { "<#OFFICE>", TAG_OFFICE },
  */
//  { "<#TAG_GROUP_META>", TAG_GROUP_META },
    { "$[officefullname]", TAG_OFFICEFULLNAME },
    { "$[officename]", TAG_OFFICENAME },
    { "$[officepath]", TAG_OFFICEPATH },
    { "$[officeversion]", TAG_OFFICEVERSION },
    { "$[portalname]", TAG_PORTALNAME },
    { "$[portalfullname]", TAG_PORTALFULLNAME },
    { "$[portalpath]", TAG_PORTALPATH },
    { "$[portalversion]", TAG_PORTALVERSION },
    { "$[portalshortname]", TAG_PORTALSHORTNAME },
/*  commenting oldstyle tags
//  { "<#TAG_GROUP_SINGLE>", TAG_GROUP_SINGLE },
    { "<#REFINSERT>", TAG_REFINSERT },

//  { "<#GROUP_MULTI>", TAG_GROUP_MULTI },
    { "<#END>", TAG_END },
    { "<#ELSE>", TAG_ELSE },
    { "<#VERSIONEND>", TAG_VERSIONEND },
    { "<#ENDGRAPHIC>", TAG_ENDGRAPHIC },*/
    { "<Common Tag>", TAG_COMMONSTART },
    { "</Common Tag>", TAG_COMMONEND },

    { "<no more tags>", TAG_NOMORETAGS },
    { "", TAG_UNKNOWN_TAG },
};


SimpleParser::SimpleParser()
: nPos( 0 )
, aNextTag( TAG_NOMORETAGS, TOK_INVALIDPOS )
{
}

void SimpleParser::Parse( rtl::OUString const & PaSource )
{
    aSource = PaSource;
    nPos = 0;
    aLastToken = rtl::OUString();
    aNextTag = TokenInfo( TAG_NOMORETAGS, TOK_INVALIDPOS );
    aTokenList.clear();
};

TokenInfo SimpleParser::GetNextToken( ParserMessageList &rErrorList )
{
    TokenInfo aResult;
    sal_Int32 nTokenStartPos = 0;
    if ( aNextTag.nId != TAG_NOMORETAGS )
    {
        aResult = aNextTag;
        aNextTag = TokenInfo( TAG_NOMORETAGS, TOK_INVALIDPOS );
    }
    else
    {
        aLastToken = GetNextTokenString( rErrorList, nTokenStartPos );
        if ( aLastToken.isEmpty() )
            return TokenInfo( TAG_NOMORETAGS, TOK_INVALIDPOS );

        // do we have a \< ... \> style tag?
        if (aLastToken.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("\\<")))
        {
            // check for paired \" \"
            bool bEven = true;
            sal_Int32 nQuotePos = 0;
            sal_Int32 nQuotedQuotesPos =
                aLastToken.indexOfAsciiL(RTL_CONSTASCII_STRINGPARAM("\\\""));
            sal_Int32 nQuotedBackPos = aLastToken.indexOfAsciiL(
                RTL_CONSTASCII_STRINGPARAM("\\\\"));
                // this is only to kick out quoted backslashes
            while (nQuotedQuotesPos != -1)
            {
                if ( nQuotedBackPos != -1 && nQuotedBackPos <= nQuotedQuotesPos )
                    nQuotePos = nQuotedBackPos+2;
                else
                {
                    nQuotePos = nQuotedQuotesPos+2;
                    bEven = !bEven;
                }
                nQuotedQuotesPos = aLastToken.indexOfAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("\\\""), nQuotePos);
                nQuotedBackPos = aLastToken.indexOfAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("\\\\"), nQuotePos);
                    // this is only to kick out quoted backslashes
            }
            if ( !bEven )
            {
                rErrorList.AddError( 24, "Missing quotes ( \\\" ) in Tag", TokenInfo( TAG_UNKNOWN_TAG, nTokenStartPos, aLastToken ) );
            }

            // check if we have an end-tag or a start-tag
            sal_Int32 nNonBlankStartPos = 2;
            while (aLastToken[nNonBlankStartPos] == ' ')
                nNonBlankStartPos++;
            if (aLastToken[nNonBlankStartPos] == '/')
                aResult = TokenInfo( TAG_COMMONEND, nTokenStartPos, aLastToken, rErrorList );
            else
            {
                aResult = TokenInfo( TAG_COMMONSTART, nTokenStartPos, aLastToken, rErrorList );
                sal_Int32 nNonBlankEndPos = aLastToken.getLength() - 3;
                while (aLastToken[nNonBlankEndPos] == ' ')
                    nNonBlankEndPos--;
                if (aLastToken[nNonBlankEndPos] == '/')
                    aNextTag = TokenInfo( TAG_COMMONEND, nTokenStartPos, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\\</")) + aResult.GetTagName() + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\\>")), rErrorList );
            }
        }
        else
        {
            sal_Int32 i = 0;
            while ( aKnownTags[i].nTag != TAG_UNKNOWN_TAG &&
                aLastToken != aKnownTags[i].GetName() )
                i++;
            aResult = TokenInfo( aKnownTags[i].nTag, nTokenStartPos );
        }
    }

    if ( aResult.nId == TAG_UNKNOWN_TAG )
        aResult = TokenInfo( TAG_UNKNOWN_TAG, nTokenStartPos, aLastToken );
    aTokenList.insert( aResult );
    return aResult;
}

rtl::OUString SimpleParser::GetNextTokenString( ParserMessageList &rErrorList, sal_Int32 &rTagStartPos )
{
    sal_Int32 nStyle2StartPos = aSource.indexOfAsciiL(
        RTL_CONSTASCII_STRINGPARAM("$["), nPos );
    sal_Int32 nStyle3StartPos = aSource.indexOfAsciiL(
        RTL_CONSTASCII_STRINGPARAM("\\<"), nPos);
    sal_Int32 nStyle4StartPos = aSource.indexOfAsciiL(
        RTL_CONSTASCII_STRINGPARAM("\\\\"), nPos);
        // this is only to kick out quoted backslashes

    rTagStartPos = 0;

    if (nStyle2StartPos == -1 && nStyle3StartPos == -1)
        return rtl::OUString();  // no more tokens

    if ( nStyle4StartPos != -1
         && (nStyle2StartPos == -1 || nStyle4StartPos < nStyle2StartPos)
         && (nStyle3StartPos == -1 || nStyle4StartPos < nStyle3StartPos ) )
        // to make sure \\ is always handled first
    {   // Skip quoted Backslash
        nPos = nStyle4StartPos +2;
        return GetNextTokenString( rErrorList, rTagStartPos );
    }

    if ( nStyle2StartPos != -1 && ( nStyle3StartPos == -1 || nStyle2StartPos < nStyle3StartPos ) )
    {   // test for $[ ... ] style tokens
        sal_Int32 nEndPos = aSource.indexOf(']', nStyle2StartPos);
        if (nEndPos == -1)
        {   // Token is incomplete. Skip start and search for better ones
            nPos = nStyle2StartPos +2;
            return GetNextTokenString( rErrorList, rTagStartPos );
        }
        nPos = nEndPos;
        rTagStartPos = nStyle2StartPos;
        return aSource.copy(nStyle2StartPos, nEndPos - nStyle2StartPos + 1);
    }
    else
    {   // test for \< ... \> style tokens
        sal_Int32 nEndPos = aSource.indexOfAsciiL(
            RTL_CONSTASCII_STRINGPARAM("\\>"), nStyle3StartPos);
        sal_Int32 nQuotedBackPos = aSource.indexOfAsciiL(
            RTL_CONSTASCII_STRINGPARAM("\\\\"), nStyle3StartPos);
            // this is only to kick out quoted backslashes
        while (nQuotedBackPos <= nEndPos && nQuotedBackPos != -1)
        {
            nEndPos = aSource.indexOfAsciiL(
                RTL_CONSTASCII_STRINGPARAM("\\>"), nQuotedBackPos + 2);
            nQuotedBackPos = aSource.indexOfAsciiL(
                RTL_CONSTASCII_STRINGPARAM("\\\\"), nQuotedBackPos + 2);
                // this is only to kick out quoted backslashes
        }
        if (nEndPos == -1)
        {   // Token is incomplete. Skip start and search for better ones
            nPos = nStyle3StartPos +2;
            rErrorList.AddError( 24, "Tag Start '\\<' without Tag End '\\>'", TokenInfo( TAG_UNKNOWN_TAG, nStyle3StartPos, helper::abbreviate(aSource, nStyle3StartPos - 10, 20) ) );
            return GetNextTokenString( rErrorList, rTagStartPos );
        }
        // check for paired quoted "    -->   \"sometext\"

        nPos = nEndPos;
        rTagStartPos = nStyle3StartPos;
        return aSource.copy(nStyle3StartPos, nEndPos-nStyle3StartPos + 2);
    }
}

rtl::OUString SimpleParser::GetLexem( TokenInfo const &aToken )
{
    if ( !aToken.aTokenString.isEmpty() )
        return aToken.aTokenString;
    else
    {
        sal_Int32 i = 0;
        while ( aKnownTags[i].nTag != TAG_UNKNOWN_TAG &&
            aKnownTags[i].nTag != aToken.nId )
            i++;

        return aKnownTags[i].GetName();
    }
}

TokenParser::TokenParser()
: pErrorList( NULL )
{}

void TokenParser::Parse( const rtl::OUString &aCode, ParserMessageList* pList )
{
    pErrorList = pList;

    //Scanner initialisieren
    aParser.Parse( aCode );

    //erstes Symbol holen
    aTag = aParser.GetNextToken( *pErrorList );

    nPfCaseOptions = 0;
    nAppCaseOptions = 0;
    bPfCaseActive = sal_False;
    bAppCaseActive = sal_False;

    nActiveRefTypes = 0;

    //Ausfuehren der Start-Produktion
    Paragraph();

    //Es wurde nicht die ganze Kette abgearbeitet, bisher ist aber
    //kein Fehler aufgetreten
    //=> es wurde ein einleitendes Tag vergessen
    if ( aTag.nId != TAG_NOMORETAGS )
    {
        switch ( aTag.nId )
        {
            case TAG_END:
                {
                    ParseError( 3, "Extra Tag <#END>. Switch or <#HREF> expected.", aTag );
                }
                break;
            case TAG_BOLDOFF:
                {
                    ParseError( 4, "<#BOLD> expected before <#/BOLD>.", aTag );
                }
                break;
            case TAG_ITALICOFF:
                {
                    ParseError( 5, "<#ITALIC> expected before <#/ITALIC>.", aTag );
                }
                break;
            case TAG_UNDERLINEOFF:
                {
                    ParseError( 17, "<#UNDER> expected before <#/UNDER>.", aTag );
                }
                break;
            case TAG_AEND:
                {
                    ParseError( 5, "Extra Tag <#AEND>. <#AVIS> or <#AHID> expected.", aTag );
                }
                break;
            case TAG_ELSE:
                {
                    ParseError( 16, "Application-tag or platform-tag expected before <#ELSE>.", aTag );
                }
                break;
            case TAG_UNKNOWN_TAG:
                {
                    ParseError( 6, "unknown Tag", aTag );
                }
                break;
            default:
                {
                    ParseError( 6, "unexpected Tag", aTag );
                }
        }
    }
    pErrorList = NULL;
}

void TokenParser::Paragraph()
{
    switch ( aTag.nId )
    {
        case TAG_GRAPHIC:
        case TAG_NEXTVERSION:
            {
                TagRef();
                Paragraph();
            }
            break;
        case TAG_AVIS:
        case TAG_AHID:
            {
                TagRef();
                Paragraph();
            }
            break;
        case TAG_HELPID:
            {
                SimpleTag();
                Paragraph();
            }
            break;
        case TAG_OFFICEFULLNAME:
        case TAG_OFFICENAME:
        case TAG_OFFICEPATH:
        case TAG_OFFICEVERSION:
        case TAG_PORTALNAME:
        case TAG_PORTALFULLNAME:
        case TAG_PORTALPATH:
        case TAG_PORTALVERSION:
        case TAG_PORTALSHORTNAME:
            {
                SimpleTag();
                Paragraph();
            }
            break;
        case TAG_REFINSERT:
            {
                SimpleTag();
                Paragraph();
            }
            break;
        case TAG_BOLDON:
        case TAG_ITALICON:
        case TAG_UNDERLINEON:
        case TAG_COMMONSTART:
            {
                TagPair();
                Paragraph();
            }
            break;
        case TAG_HREF:
        case TAG_NAME:
        case TAG_KEY:
        case TAG_INDEX:
        case TAG_TITEL:
        case TAG_REFSTART:
            {
                TagRef();
                Paragraph();
            }
            break;
        case TAG_WIN:
        case TAG_UNIX:
        case TAG_MAC: //...
            {
                if ( ! bPfCaseActive )
                {
                    //PfCases duerfen nicht verschachtelt sein:
                    bPfCaseActive = sal_True;
                    PfCase();

                    //So jetzt kann wieder ein PfCase kommen:
                    bPfCaseActive = sal_False;
                    Paragraph();
                }
            }
            break;
        case TAG_WRITER:
        case TAG_CALC:
        case TAG_DRAW:
        case TAG_IMPRESS:
        case TAG_SCHEDULE:
        case TAG_IMAGE:
        case TAG_MATH:
        case TAG_CHART:
        case TAG_OFFICE:
            {
                if ( !bAppCaseActive )
                {
                    //AppCases duerfen nicht verschachtelt sein:
                    bAppCaseActive = sal_True;
                    AppCase();

                    //jetzt koennen wieder AppCases kommen:
                    bAppCaseActive = sal_False;
                    Paragraph();
                }
            }
            break;

        //Case TAG_BOLDOFF, TAG_ITALICOFF, TAG_BUNDERLINE, TAG_END
            //nichts tun wg. epsilon-Prod.
    }
}

void TokenParser::PfCase()
{

    //Produktion:
    //PfCase -> PfCaseBegin Paragraph (PfCase | PfCaseEnd)

    PfCaseBegin();

    //Jetzt ist eine PfCase-Produktion aktiv:
    Paragraph();
    switch ( aTag.nId )
    {
        case TAG_ELSE:
        case TAG_END:
            {
                CaseEnd();
            }
            break;
        case TAG_WIN:
        case TAG_UNIX:
        case TAG_MAC: //First (PfBegin)
            {
                PfCase();
            }
            break;
        default:
            ParseError( 8, "<#ELSE> or <#END> or platform-tag expected.", aTag );
    }
    //Die gemerkten Tags wieder loeschen fuer naechstes PfCase:
    nPfCaseOptions = 0;
}

void TokenParser::PfCaseBegin()
{
    switch ( aTag.nId )
    {
        case TAG_WIN:
        case TAG_UNIX:
        case TAG_MAC:
            {
                //Token darf noch nicht vorgekommen sein im
                //aktuellen Plattform-Case:
                if ( !HAS_FLAG( nPfCaseOptions, TAG_NOGROUP( aTag.nId ) ) )
                {
                    SET_FLAG( nPfCaseOptions, TAG_NOGROUP( aTag.nId ) );
                    match( aTag, aTag );
                }
                else {
                    ParseError( 9, "Tag defined twice in the same platform-case", aTag );
                }
            }
    }
}

void TokenParser::AppCase()
{

    //Produktion:
    //AppCase -> AppCaseBegin Paragraph (AppCase | AppCaseEnd)


    AppCaseBegin();

    Paragraph();

    switch ( aTag.nId )
    {
        case TAG_ELSE:
        case TAG_END:
            {
                CaseEnd();
            }
            break;
        case TAG_WRITER:
        case TAG_DRAW:
        case TAG_CALC:
        case TAG_IMAGE:
        case TAG_MATH:
        case TAG_CHART:
        case TAG_OFFICE:
        case TAG_IMPRESS:
        case TAG_SCHEDULE: //First (AppBegin)
            {
                AppCase();
            }
            break;
        default:
            ParseError( 1, "<#ELSE> or <#END> or application-case-tag expected.", aTag );
        }

    //Die gemerkten Tags wieder loeschen fuer naechstes AppCase:
    nAppCaseOptions = 0;
}

void TokenParser::AppCaseBegin()
{
    switch ( aTag.nId )
    {
        case TAG_WRITER:
        case TAG_DRAW:
        case TAG_CALC:
        case TAG_IMAGE:
        case TAG_MATH:
        case TAG_CHART:
        case TAG_OFFICE:
        case TAG_IMPRESS:
        case TAG_SCHEDULE:
            {
                //Token darf noch nicht vorgekommen sein im
                //aktuellen Plattform-Case:
                if ( !HAS_FLAG( nAppCaseOptions, TAG_NOGROUP( aTag.nId ) ) )
                {
                    SET_FLAG( nAppCaseOptions, TAG_NOGROUP( aTag.nId ) );
                    match( aTag, aTag );
                }
                else {
                    ParseError( 13, "Tag defined twice in the same application-case.", aTag );
                }
            }
    }
}

void TokenParser::CaseEnd()
{
    //Produktion:
    //CaseEnd -> <#ELSE> Paragraph <#END> | <#END>

    switch ( aTag.nId )
    {
        case TAG_ELSE:
        {
            match( aTag, TAG_ELSE );
            Paragraph();
            match( aTag, TAG_END );
        }
        break;
        case TAG_END:
        {
            match( aTag, TAG_END );
        }
        break;
        default:
            ParseError( 2, "<#ELSE> or <#END> expected.", aTag );
    }
}

void TokenParser::SimpleTag()
{

    switch ( aTag.nId )
    {
        case TAG_HELPID:
            {
                match( aTag, TAG_HELPID );
            }
            break;
        case TAG_OFFICEFULLNAME:
        case TAG_OFFICENAME:
        case TAG_OFFICEPATH:
        case TAG_OFFICEVERSION:
        case TAG_PORTALNAME:
        case TAG_PORTALFULLNAME:
        case TAG_PORTALPATH:
        case TAG_PORTALVERSION:
        case TAG_PORTALSHORTNAME:

        case TAG_REFINSERT:
            {
                match( aTag, aTag );
            }
            break;
        default:
            ParseError( 15, "[<#SimpleTag>] expected.", aTag );
    }
}

void TokenParser::TagPair()
{
    switch ( aTag.nId )
    {
        case TAG_BOLDON:
            {
                match( aTag, TAG_BOLDON );
                Paragraph();
                match( aTag, TAG_BOLDOFF );
            }
            break;
        case TAG_ITALICON:
            {
                match( aTag, TAG_ITALICON );
                Paragraph();
                match( aTag, TAG_ITALICOFF );
            }
            break;
        case TAG_UNDERLINEON:
            {
                match( aTag, TAG_UNDERLINEON );
                Paragraph();
                match( aTag, TAG_UNDERLINEOFF );
            }
            break;
        case TAG_COMMONSTART:
            {
                //remember tag so we can give the original tag in case of an error
                TokenInfo aEndTag( aTag );
                aEndTag.nId = TAG_COMMONEND;
                match( aTag, TAG_COMMONSTART );
                Paragraph();
                match( aTag, aEndTag );
            }
            break;
        default:
            ParseError( 10, "<#BOLD>, <#ITALIC>, <#UNDER> expected.", aTag );
    }
}


void TokenParser::TagRef()
{
    switch ( aTag.nId )
    {
        case TAG_GRAPHIC:
        case TAG_NEXTVERSION:
            {
                if ( !HAS_FLAG( nActiveRefTypes, TAG_NOGROUP( aTag.nId ) ) )
                {
                    TokenId aThisToken = aTag.nId;
                    SET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                    match( aTag, aTag );
                    Paragraph();
                    if ( aThisToken == TAG_GRAPHIC )
                        match( aTag, TAG_ENDGRAPHIC );
                    else
                        match( aTag, TAG_VERSIONEND );
                    // don't reset since alowed only once per paragraph
                    // RESET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                }
                else
                {
                    ParseError( 11, "Tags <#GRAPHIC>,<#NEXTVERSION> allowed only once per paragraph at", aTag );
                }
            }
            break;
        case TAG_AVIS:
        case TAG_AHID:
            {
                if ( !HAS_FLAG( nActiveRefTypes, TAG_NOGROUP( aTag.nId ) ) )
                {
                    TokenId aThisToken = aTag.nId;
                    SET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                    match( aTag, aTag );
                    Paragraph();
                    match( aTag, TAG_AEND );
                    RESET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                }
                else
                {
                    ParseError( 11, "Nested <#AHID>,<#AVIS> not allowed.", aTag );
                }
            }
            break;
        case TAG_HREF:
        case TAG_NAME:
            {

            }
            // NOBREAK
        case TAG_KEY:
        case TAG_INDEX:
        case TAG_TITEL:
        case TAG_REFSTART:
            {
                if ( !HAS_FLAG( nActiveRefTypes, TAG_NOGROUP( aTag.nId ) ) )
                {
                    TokenId aThisToken = aTag.nId;
                    match( aTag, aTag );
                    if ( aThisToken != TAG_NAME )
                    {   // TAG_NAME has no TAG_END
                        SET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                        Paragraph();
                        match( aTag, TAG_END );
                        RESET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                    }
                }
                else
                {
                    ParseError( 11, "Nested <#HREF>,<#NAME> or <#KEY> not allowed.", aTag );
                }
            }
            break;
        default:
            ParseError( 12, "<#HREF>,<#NAME> or <#KEY> expected.", aTag );
    }
}

sal_Bool TokenParser::match( const TokenInfo &aCurrentToken, const TokenId &aExpectedToken )
{
    return match( aCurrentToken, TokenInfo( aExpectedToken, TOK_INVALIDPOS ) );
}

sal_Bool TokenParser::match( const TokenInfo &aCurrentToken, const TokenInfo &rExpectedToken )
{
    TokenInfo aExpectedToken( rExpectedToken );
    if ( aCurrentToken.nId == aExpectedToken.nId )
    {
        if ( ( aCurrentToken.nId == TAG_COMMONEND
               && aCurrentToken.GetTagName() == aExpectedToken.GetTagName() )
             || aCurrentToken.nId != TAG_COMMONEND )
        {
            aTag = aParser.GetNextToken( *pErrorList );
            return sal_True;
        }
    }

    if ( aExpectedToken.nId == TAG_COMMONEND )
    {
        aExpectedToken.aTokenString =
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Close tag for "))
            + aExpectedToken.aTokenString;
    }

    rtl::OString sTmp(RTL_CONSTASCII_STRINGPARAM("Expected Symbol"));
    if ( aCurrentToken.nId == TAG_NOMORETAGS )
    {
        ParseError( 7, sTmp, aExpectedToken );
    }
    else
    {
        rtl::OStringBuffer aBuf(sTmp);
        aBuf.append(": ").
            append(rtl::OUStringToOString(aParser.GetLexem( aExpectedToken ), RTL_TEXTENCODING_UTF8)).
            append(RTL_CONSTASCII_STRINGPARAM(" near "));
        ParseError( 7, aBuf.makeStringAndClear(), aCurrentToken );
    }
    return sal_False;
}

void TokenParser::ParseError( sal_Int32 nErrNr, const rtl::OString &rErrMsg, const TokenInfo &rTag )
{
    pErrorList->AddError( nErrNr, rErrMsg, rTag);

    // Das Fehlerhafte Tag ueberspringen
    aTag = aParser.GetNextToken( *pErrorList );
}


ParserMessage::ParserMessage( sal_Int32 PnErrorNr, const rtl::OString &rPaErrorText, const TokenInfo &rTag )
        : nErrorNr( PnErrorNr )
        , nTagBegin( 0 )
        , nTagLength( 0 )
{
    rtl::OUString aLexem( SimpleParser::GetLexem( rTag ) );
    rtl::OStringBuffer aErrorBuffer(rPaErrorText);
    aErrorBuffer.append(RTL_CONSTASCII_STRINGPARAM(": "));
    aErrorBuffer.append(rtl::OUStringToOString(aLexem, RTL_TEXTENCODING_UTF8));
    if ( rTag.nId == TAG_NOMORETAGS )
        aErrorBuffer.append(RTL_CONSTASCII_STRINGPARAM(" at end of line "));
    else if ( rTag.nPos != TOK_INVALIDPOS )
    {
        aErrorBuffer.append(RTL_CONSTASCII_STRINGPARAM(" at Position "));
        aErrorBuffer.append(static_cast<sal_Int32>(rTag.nPos));
    }
    aErrorText = aErrorBuffer.makeStringAndClear();
    nTagBegin = rTag.nPos;
    nTagLength = aLexem.getLength();
}

ParserError::ParserError( sal_Int32 ErrorNr, const rtl::OString &rErrorText, const TokenInfo &rTag )
: ParserMessage( ErrorNr, rErrorText, rTag )
{}

ParserWarning::ParserWarning( sal_Int32 ErrorNr, const rtl::OString &rErrorText, const TokenInfo &rTag )
: ParserMessage( ErrorNr, rErrorText, rTag )
{}

sal_Bool LingTest::IsTagMandatory( TokenInfo const &aToken, TokenId &aMetaTokens )
{
    TokenId aTokenId = aToken.nId;
    TokenId aTokenGroup = TAG_GROUP( aTokenId );
    if ( TAG_GROUP_PROGSWITCH == aTokenGroup
        || TAG_REFINSERT == aTokenId
        || TAG_REFSTART == aTokenId
        || TAG_NAME == aTokenId
        || TAG_HREF == aTokenId
        || TAG_AVIS == aTokenId
        || TAG_AHID == aTokenId
        || TAG_GRAPHIC == aTokenId
        || TAG_NEXTVERSION == aTokenId
        || ( TAG_GROUP_META == aTokenGroup && (aMetaTokens & aTokenId) == aTokenId ) )
    {
        if ( TAG_GROUP_META == aTokenGroup )
            aMetaTokens |= aTokenId;
        return sal_True;
    }
    else if (   TAG_COMMONSTART == aTokenId
             || TAG_COMMONEND == aTokenId )
    {
        rtl::OUString aTagName = aToken.GetTagName();
        return !(aTagName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("comment"))
              || aTagName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("bookmark_value"))
              || aTagName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("emph"))
              || aTagName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("item"))
              || aTagName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("br")) );
    }
    return sal_False;
}

void LingTest::CheckTags( TokenList &aReference, TokenList &aTestee, sal_Bool bFixTags )
{
    size_t i=0,j=0;
    // Clean old Warnings
    aCompareWarningList.clear();

    /* in xml tags, do not require the following tags
        comment
        bookmark_value
        emph
        item
        br
    */

    // filter uninteresting Tags
    TokenId aMetaTokens = 0;
    for ( i=0 ; i < aReference.size() ; i++ )
    {
        if ( !IsTagMandatory( aReference[ i ], aMetaTokens ) )
            aReference[ i ].SetDone();
    }

    aMetaTokens = 0;
    for ( i=0 ; i < aTestee.size() ; i++ )
    {
        if ( !IsTagMandatory( aTestee[ i ], aMetaTokens ) )
            aTestee[ i ].SetDone();
    }

    // remove all matching tags
    for ( i=0 ; i < aReference.size() ; i++ )
    {
        if ( aReference[ i ].IsDone() )
            continue;

        sal_Bool bTagFound = sal_False;
        for ( j=0 ; j < aTestee.size() && !bTagFound ; j++ )
        {
            if ( aTestee[ j ].IsDone() )
                continue;

            if ( aReference[ i ].MatchesTranslation( aTestee[ j ], sal_False, aCompareWarningList ) )
            {
                aReference[ i ].SetDone();
                aTestee[ j ].SetDone();
                bTagFound = sal_True;
            }
        }
    }

    sal_Bool bCanFix = sal_True;

    if ( bFixTags )
    {
        // we fix only if its a really simple case
        sal_Int32 nTagCount = 0;
        for ( i=0 ; i < aReference.size() ; i++ )
            if ( !aReference[ i ].IsDone() )
                nTagCount++;
        if ( nTagCount > 1 )
            bCanFix = sal_False;

        nTagCount = 0;
        for ( i=0 ; i < aTestee.size() ; i++ )
            if ( !aTestee[ i ].IsDone() )
                nTagCount++;
        if ( nTagCount > 1 )
            bCanFix = sal_False;
    }

    // generate errors for tags that have differing attributes
    for ( i=0 ; i < aReference.size() ; i++ )
    {
        if ( aReference[ i ].IsDone() )
            continue;

        sal_Bool bTagFound = sal_False;
        for ( j=0 ; j < aTestee.size() && !bTagFound ; j++ )
        {
            if ( aTestee[ j ].IsDone() )
                continue;

            if ( aReference[ i ].MatchesTranslation( aTestee[ j ], sal_True, aCompareWarningList, bCanFix && bFixTags ) )
            {
                aReference[ i ].SetDone();
                aTestee[ j ].SetDone();
                bTagFound = sal_True;
            }
        }
    }

    // list remaining tags as errors
    for ( i=0 ; i < aReference.size() ; i++ )
    {
        if ( aReference[ i ].IsDone() )
            continue;

        aCompareWarningList.AddError( 20, "Missing Tag in Translation", aReference[ i ] );
    }
    for ( i=0 ; i < aTestee.size() ; i++ )
    {
        if ( aTestee[ i ].IsDone() )
            continue;

        aCompareWarningList.AddError( 21, "Extra Tag in Translation", aTestee[ i ] );
    }

    for ( i=0 ; i < aReference.size() ; i++ )
        aReference[ i ].SetDone( sal_False );

    for ( i=0 ; i < aTestee.size() ; i++ )
        aTestee[ i ].SetDone( sal_False );
}

void LingTest::CheckReference( GSILine *aReference )
{
    aReferenceParser.Parse( aReference->GetUText(), aReference->GetMessageList() );
}

void LingTest::CheckTestee( GSILine *aTestee, sal_Bool bHasSourceLine, sal_Bool bFixTags )
{
    aFixedTestee = aTestee->GetUText();
    aTesteeParser.Parse( aFixedTestee, aTestee->GetMessageList() );

    if ( bHasSourceLine )
        CheckTags( aReferenceParser.GetTokenList(), aTesteeParser.GetTokenList(), bFixTags );

    if ( bFixTags )
    {
        TokenList& aTesteeTokens = aTesteeParser.GetTokenList();
        sal_Bool bFixesDone = sal_False;
        // count backwards to allow replacing from right to left
        int i;
        for ( i = aTesteeTokens.size() ; i > 0 ; )
        {
            if ( aTesteeTokens[ --i ].HasBeenFixed() )
            {
                bFixesDone = sal_True;
                aFixedTestee = aFixedTestee.replaceAt( aTesteeTokens[ i ].nPos, aTesteeTokens[ i ].aTokenString.getLength(), aTesteeTokens[ i ].MakeTag() );
            }
        }
        if ( bFixesDone )
        {
            aTestee->SetUText( aFixedTestee );
            aTestee->SetFixed();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
