/*************************************************************************
 *
 *  $RCSfile: tagtest.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-12-10 17:17:17 $
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
#include "tagtest.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif


#define HAS_FLAG( nFlags, nFlag )       ( ( nFlags & nFlag ) != 0 )
#define SET_FLAG( nFlags, nFlag )       ( nFlags |= nFlag )
#define RESET_FLAG( nFlags, nFlag )     ( nFlags &= ~nFlag )    // ~ = Bitweises NOT

//ByteString SimpleParser::aLastUnknownToken;


struct Tag
{
    char* aName;
    TokenId nTag;
};


static Tag __READONLY_DATA aKnownTags[] =
{
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

//  { "<#TAG_GROUP_SINGLE>", TAG_GROUP_SINGLE },
    { "<#REFINSERT>", TAG_REFINSERT },

//  { "<#GROUP_MULTI>", TAG_GROUP_MULTI },
    { "<#END>", TAG_END },
    { "<#ELSE>", TAG_ELSE },
    { "<#VERSIONEND>", TAG_VERSIONEND },
    { "<#ENDGRAPHIC>", TAG_ENDGRAPHIC },
    { "<Common Tag>", TAG_COMMONSTART },
    { "</Common Tag>", TAG_COMMONEND },
    { "", TAG_UNKNOWN_TAG },
};


SimpleParser::SimpleParser()
: nPos( 0 )
, aNextTag( TAG_NOMORETAGS )
{
}

void SimpleParser::Parse( ByteString PaSource )
{
    aSource = PaSource;
    nPos = 0;
    aLastToken = "";
    aNextTag = TokenInfo( TAG_NOMORETAGS );
    aTokenList.Clear();
};

TokenInfo SimpleParser::GetNextToken( ParserMessageList &rErrorList )
{
    TokenInfo aResult;
    if ( aNextTag.nId != TAG_NOMORETAGS )
    {
        aResult = aNextTag;
        aNextTag = TokenInfo( TAG_NOMORETAGS );
    }
    else
    {
        aLastToken = GetNextTokenString( rErrorList );
        if ( aLastToken == "" )
            return TokenInfo( TAG_NOMORETAGS );

        USHORT i = 0;
        while ( aKnownTags[i].nTag != TAG_UNKNOWN_TAG &&
            aLastToken != aKnownTags[i].aName )
            i++;
        aResult = TokenInfo( aKnownTags[i].nTag );

        // do we have a \< ... \> style tag?
        if ( aResult.nId == TAG_UNKNOWN_TAG && aLastToken.Copy(0,2).Equals( "\\<" ) )
        {
            // check for paired \" \"
            bool bEven = true;
            USHORT nPos = 0;
            USHORT nQuotedQuotesPos = aLastToken.Search( "\\\"" );
            USHORT nQuotedBackPos = aLastToken.Search( "\\\\" );    // this is only to kick out quoted backslashes
            while ( nQuotedQuotesPos != STRING_NOTFOUND )
            {
                if ( nQuotedBackPos <= nQuotedQuotesPos )
                    nPos = nQuotedBackPos+2;
                else
                {
                    nPos = nQuotedQuotesPos+2;
                    bEven = !bEven;
                }
                nQuotedQuotesPos = aLastToken.Search( "\\\"", nPos );
                nQuotedBackPos = aLastToken.Search( "\\\\", nPos );    // this is only to kick out quoted backslashes
            }
            if ( !bEven )
            {
                ByteString sTmp( "Missing quotes ( \\\" ) in Tag: " );
                sTmp += aLastToken;
                rErrorList.Insert( new ParserMessage( 24, sTmp ) );
            }

            // check if we have an end-tag or a start-tag
            if ( aLastToken.GetChar(2) == '/' )
                aResult = TokenInfo( TAG_COMMONEND, aLastToken );
            else
            {
                aResult = TokenInfo( TAG_COMMONSTART, aLastToken );
                if ( aLastToken.GetChar( aLastToken.Len() -3 ) == '/' )
                    aNextTag = TokenInfo( TAG_COMMONEND, aLastToken );
            }
        }
    }

    if ( aResult.nId == TAG_UNKNOWN_TAG )
        aResult = TokenInfo( TAG_UNKNOWN_TAG, aLastToken );
    aTokenList.Insert( aResult, LIST_APPEND );
    return aResult;
}

ByteString SimpleParser::GetTokenText()
{
    return aLastToken;
}

ByteString SimpleParser::GetNextTokenString( ParserMessageList &rErrorList )
{
    USHORT nStyle1StartPos = aSource.Search( "<#", nPos );
    USHORT nStyle2StartPos = aSource.Search( "$[", nPos );
    USHORT nStyle3StartPos = aSource.Search( "\\<", nPos );
    USHORT nStyle4StartPos = aSource.Search( "\\\\", nPos );    // this is only to kick out quoted backslashes

    // check if the tag starts with a letter to avoid things like <> <= ... >
    while ( STRING_NOTFOUND != nStyle3StartPos && !( aSource.Copy( nStyle3StartPos+2, 1 ).IsAlphaAscii() || aSource.GetChar( nStyle3StartPos+2 ) == '/' ) )
        nStyle3StartPos = aSource.Search( "\\<", nStyle3StartPos+1 );

    if ( STRING_NOTFOUND == nStyle1StartPos && STRING_NOTFOUND == nStyle2StartPos && STRING_NOTFOUND == nStyle3StartPos )
        return "";  // no more tokens

    if ( nStyle4StartPos < nStyle1StartPos && nStyle4StartPos < nStyle2StartPos && nStyle4StartPos <= nStyle3StartPos )  // <= to make sure \\ is always handled first
    {   // Skip quoted Backslash
        nPos = nStyle4StartPos +2;
        return GetNextTokenString( rErrorList );
    }

    if ( nStyle1StartPos < nStyle2StartPos && nStyle1StartPos <= nStyle3StartPos )  // <= to make sure our spechial tags are recognized before all others
    {   // test for <# ... > style tokens
        USHORT nEndPos = aSource.Search( ">", nStyle1StartPos );
        if ( nEndPos == STRING_NOTFOUND )
        {   // Token is incomplete. Skip start and search for better ones
            nPos = nStyle1StartPos +2;
            return GetNextTokenString( rErrorList );
        }
        nPos = nEndPos;
        return aSource.Copy( nStyle1StartPos, nEndPos-nStyle1StartPos +1 ).ToUpperAscii();
    }
    else if ( nStyle2StartPos < nStyle1StartPos && nStyle2StartPos < nStyle3StartPos )
    {   // test for $[ ... ] style tokens
        USHORT nEndPos = aSource.Search( "]", nStyle2StartPos);
        if ( nEndPos == STRING_NOTFOUND )
        {   // Token is incomplete. Skip start and search for better ones
            nPos = nStyle2StartPos +2;
            return GetNextTokenString( rErrorList );
        }
        nPos = nEndPos;
        return aSource.Copy( nStyle2StartPos, nEndPos-nStyle2StartPos +1 );
    }
    else
    {   // test for \< ... \> style tokens
        USHORT nEndPos = aSource.Search( "\\>", nStyle3StartPos);
        USHORT nQuotedBackPos = aSource.Search( "\\\\", nStyle3StartPos );    // this is only to kick out quoted backslashes
        while ( nQuotedBackPos <= nEndPos && nQuotedBackPos != STRING_NOTFOUND )
        {
            nEndPos = aSource.Search( "\\>", nQuotedBackPos +2);
            nQuotedBackPos = aSource.Search( "\\\\", nQuotedBackPos +2 );    // this is only to kick out quoted backslashes
        }
        if ( nEndPos == STRING_NOTFOUND )
        {   // Token is incomplete. Skip start and search for better ones
            nPos = nStyle3StartPos +2;
            ByteString sTmp( "Tag Start '\\<' without Tag End '\\>': " );
            sTmp += aSource.Copy( nStyle3StartPos-10, 20 );
            rErrorList.Insert( new ParserMessage( 24, sTmp ) );
            return GetNextTokenString( rErrorList );
        }
        // check for paired quoted "    -->   \"sometext\"

        nPos = nEndPos;
        return aSource.Copy( nStyle3StartPos, nEndPos-nStyle3StartPos +2 );
    }
}

ByteString SimpleParser::GetLexem( TokenInfo const &aToken )
{
    if ( aToken.aName.Len() )
        return aToken.aName;
    else
    {
        USHORT i = 0;
        while ( aKnownTags[i].nTag != TAG_UNKNOWN_TAG &&
            aKnownTags[i].nTag != aToken.nId )
            i++;

        return ByteString( aKnownTags[i].aName );
    }
}

TokenParser::TokenParser()
{}

void TokenParser::Parse( const ByteString &aCode )
{
    while ( aErrorList.Count() )
    {
        delete aErrorList.GetCurObject();
        aErrorList.Remove();
    }


    //Scanner initialisieren
    aParser.Parse( aCode );

    //erstes Symbol holen
    aTag = aParser.GetNextToken( aErrorList );

    nPfCaseOptions = 0;
    nAppCaseOptions = 0;
    bPfCaseActive = FALSE;
    bAppCaseActive = FALSE;

    nActiveRefTypes = 0;

    //Ausführen der Start-Produktion
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
                    ParseError( 3, "Extra Tag <#END>. Switch or <#HREF> expected." );
                }
                break;
            case TAG_BOLDOFF:
                {
                    ParseError( 4, "<#BOLD> expected before <#/BOLD>." );
                }
                break;
            case TAG_ITALICOFF:
                {
                    ParseError( 5, "<#ITALIC> expected before <#/ITALIC>." );
                }
                break;
            case TAG_UNDERLINEOFF:
                {
                    ParseError( 17, "<#UNDER> expected before <#/UNDER>." );
                }
                break;
/*          case TAG_MISSPARENTHESIS:
                {
                    ParseError( 14, "missing closing parenthesis '>'" );
                }
                break;*/
            case TAG_AEND:
                {
                    ParseError( 5, "Extra Tag <#AEND>. <#AVIS> or <#AHID> expected." );
                }
                break;
            case TAG_ELSE:
                {
                    ParseError( 16, "Application-tag or platform-tag expected before <#ELSE>." );
                }
                break;
            case TAG_UNKNOWN_TAG:
                {
                    ByteString sTmp( "unknown Tag: " );
                    sTmp += aParser.GetLexem( aTag );
                    ParseError( 6, sTmp );
                }
                break;
            default:
                {
                    ByteString sTmp( "unexpected Tag: " );
                    sTmp += aParser.GetLexem( aTag );
                    ParseError( 6, sTmp );
                }
        }
    }
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
        case TAG_OS2:
        case TAG_WIN:
        case TAG_UNIX:
        case TAG_MAC: //...
            {
                if ( ! bPfCaseActive )
                {
                    //PfCases dürfen nicht verschachtelt sein:
                    bPfCaseActive = TRUE;
                    PfCase();

                    //So jetzt kann wieder ein PfCase kommen:
                    bPfCaseActive = FALSE;
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
                    //AppCases dürfen nicht verschachtelt sein:
                    bAppCaseActive = TRUE;
                    AppCase();

                    //jetzt können wieder AppCases kommen:
                    bAppCaseActive = FALSE;
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
        case TAG_OS2:
        case TAG_WIN:
        case TAG_UNIX:
        case TAG_MAC: //First (PfBegin)
            {
                PfCase();
            }
            break;
        default:
            ParseError( 8, "<#ELSE> or <#END> or platform-tag expected." );
    }
    //Die gemerkten Tags wieder löschen für nächstes PfCase:
    nPfCaseOptions = 0;
}

void TokenParser::PfCaseBegin()
{
    switch ( aTag.nId )
    {
        case TAG_OS2:
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
                    ByteString sTmp( aParser.GetLexem( aTag ));
                    sTmp += " defined twice in the same platform-case.";
                    ParseError( 9, sTmp );
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
            ParseError( 1, "<#ELSE> or <#END> or application-case-tag expected." );
        }

    //Die gemerkten Tags wieder löschen für nächstes AppCase:
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
                    ByteString sTmp( aParser.GetLexem( aTag ));
                    sTmp += " defined twice in the same application-case.";
                    ParseError( 13, sTmp );
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
            ParseError( 2, "<#ELSE> or <#END> expected." );
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
            ParseError( 15, "[<#SimpleTag>] expected." );
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
                TokenInfo aEndTag( TAG_COMMONEND, ByteString( "Close tag for " ).Append(aTag.aName) );
                match( aTag, TAG_COMMONSTART );
                Paragraph();
                match( aTag, aEndTag );
            }
            break;
        default:
            ParseError( 10, "<#BOLD>, <#ITALIC>, <#UNDER> expected." );
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
                    ParseError( 11, "Tags <#GRAPHIC>,<#NEXTVERSION> allowed only once per paragraph." );
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
                    ParseError( 11, "Nested <#AHID>,<#AVIS> not allowed." );
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
                    ParseError( 11, "Nested <#HREF>,<#NAME> or <#KEY> not allowed." );
                }
            }
            break;
        default:
            ParseError( 12, "<#HREF>,<#NAME> or <#KEY> expected." );
    }
}

BOOL TokenParser::match( const TokenInfo &aCurrentToken, const TokenId &aExpectedToken )
{
    return match( aCurrentToken, TokenInfo( aExpectedToken ) );
}

BOOL TokenParser::match( const TokenInfo &aCurrentToken, const TokenInfo &aExpectedToken )
{
    if ( aCurrentToken.nId == aExpectedToken.nId )
    {
        aTag = aParser.GetNextToken( aErrorList );
        return TRUE;
    }
    else
    {
        ByteString sTmp( "Expected Symbol: " );
        sTmp += aParser.GetLexem( aExpectedToken );
        sTmp += ".";
        ParseError( 7, sTmp );
        return FALSE;
    }
}


void TokenParser::ParseError( USHORT nErrNr, const ByteString &aErrMsg )
{
    USHORT nTokenLength = aParser.GetTokenText().Len() + 2;
    aErrorList.Insert( new ParserMessage( nErrNr, aErrMsg, aParser.GetScanningPosition()-nTokenLength, nTokenLength), LIST_APPEND );

    // Das Fehlerhafte Tag überspringen
    aTag = aParser.GetNextToken( aErrorList );
}

void LingTest::CheckMandatoryTag( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList, TokenInfo aToken )
{
    while ( (aReference.GetPos( aToken ) != LIST_ENTRY_NOTFOUND) && (aTestee.GetPos( aToken ) != LIST_ENTRY_NOTFOUND) )
    {
        aReference.Remove( aToken );
        aTestee.Remove( aToken );
    }
    if ( aReference.GetPos( aToken ) != LIST_ENTRY_NOTFOUND )
    {
        ByteString sTmp( "Missing Tag in Translation: " );
        sTmp += SimpleParser::GetLexem( aToken );
        rErrorList.Insert( new ParserMessage( 18, sTmp ) );
        aReference.Remove( aToken );
    }
    if ( aTestee.GetPos( aToken ) != LIST_ENTRY_NOTFOUND )
    {
        ByteString sTmp( "Extra Tag in Translation: " );
        sTmp += SimpleParser::GetLexem( aToken );
        rErrorList.Insert( new ParserMessage( 19, sTmp ));
        aTestee.Remove( aToken );
    }
}

ByteString GetTagName( ByteString const &aTag )
{
    ByteString aRet( aTag.Copy(2) );    // cut off  \<
    aRet.EraseLeadingChars( ' ' );
    USHORT nPos;
    if ( ( nPos = aRet.Search(' ') )  != STRING_NOTFOUND )
        return aRet.Copy( 0, nPos );
    aRet.Erase( aRet.Len() -2 );
    if ( aRet.GetChar( aRet.Len()-1 ) == '/' )
        aRet.Erase( aRet.Len() -1 );
    else if ( aRet.GetChar( 0 ) == '/' )
        aRet.Erase( 0, 1 );
    return aRet;
}

BOOL LingTest::IsTagMandatory( TokenInfo const &aToken, TokenId &aMetaTokens )
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
        return TRUE;
    }
    else if (   TAG_COMMONSTART == aTokenId
             || TAG_COMMONEND == aTokenId )
    {
        ByteString aTagName = GetTagName( aToken.aName );
        return !(aTagName.EqualsIgnoreCaseAscii( "comment" )
              || aTagName.EqualsIgnoreCaseAscii( "bookmark_value" )
              || aTagName.EqualsIgnoreCaseAscii( "emph" )
              || aTagName.EqualsIgnoreCaseAscii( "item" )
              || aTagName.EqualsIgnoreCaseAscii( "br" ) );
    }
    return FALSE;
}

void LingTest::CheckTags( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList )
{
    ULONG i=0,j=0;
    // Clean old Warnings
    while ( aCompareWarningList.Count() )
    {
        delete aCompareWarningList.GetCurObject();
        aCompareWarningList.Remove();
    }

    /* in xml tags, do not require the following tags
        comment
        bookmark_value
        emph
        item
        br
    */

    // filter uninteresting Tags
    TokenId aMetaTokens = 0;
    i=0;
    while ( i < aReference.Count() )
    {
        if ( IsTagMandatory( aReference.GetObject( i ), aMetaTokens ) )
        {
            i++;
        }
        else
            aReference.Remove( i );
    }

    aMetaTokens = 0;
    i=0;
    while ( i < aTestee.Count() )
    {
        if ( IsTagMandatory( aTestee.GetObject( i ), aMetaTokens ) )
        {
            i++;
        }
        else
            aTestee.Remove( i );
    }

    // remove all found tags
    i = 0;
    while ( i < aReference.Count() )
    {
        BOOL bTagFound = FALSE;
        j = 0;
        while ( j < aTestee.Count() && !bTagFound )
        {
            if ( aReference.GetObject( i ).nId == aTestee.GetObject( j ).nId )
                bTagFound = TRUE;
            else
                j++;
        }
        if ( bTagFound )
        {
            aReference.Remove( i );
            aTestee.Remove( j );
        }
        else
            i++;
    }

    // list remaining tags as errors
    ParserMessage *pNewWarning = NULL;
    i = 0;
    while ( i < aReference.Count() )
    {
        ByteString sTmp( "Missing Tag in Translation: " );
        sTmp += SimpleParser::GetLexem( aReference.GetObject( i ));
        pNewWarning =  new ParserMessage( 20, sTmp );
        aCompareWarningList.Insert( pNewWarning, LIST_APPEND );
        i++;
    }
    i = 0;
    while ( i < aTestee.Count() )
    {
        ByteString sTmp( "Extra Tag in Translation: " );
        sTmp += SimpleParser::GetLexem( aTestee.GetObject( i ));
        pNewWarning =  new ParserMessage( 21, sTmp );
        aCompareWarningList.Insert( pNewWarning, LIST_APPEND );
        i++;
    }

//  This message is obselete now
//    pNewWarning = new ParserMessage( 22, "Tags Differ. Too many differences to be specified" );
}

BOOL LingTest::ReferenceOK( const ByteString &aReference )
{
    aReferenceParser.Parse( aReference );
    return !aReferenceParser.HasErrors();
}

BOOL LingTest::TesteeOK( const ByteString &aTestee, BOOL bHasSourceLine )
{
    aTesteeParser.Parse( aTestee );

    if ( bHasSourceLine )
        CheckTags( aReferenceParser.GetTokenList(), aTesteeParser.GetTokenList(), aTesteeParser.GetErrors() );

    return !aTesteeParser.HasErrors() && !HasCompareWarnings();
}

