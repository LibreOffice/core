/*************************************************************************
 *
 *  $RCSfile: tagtest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:26 $
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

#ifdef DEBUG
#include <stdio.h>
#endif


#define HAS_FLAG( nFlags, nFlag )       ( ( nFlags & nFlag ) != 0 )
#define SET_FLAG( nFlags, nFlag )       ( nFlags |= nFlag )
#define RESET_FLAG( nFlags, nFlag )     ( nFlags &= ~nFlag )    // ~ = Bitweises NOT


struct Tag
{
    char* aName;
    Token nTag;
};


static Tag __READONLY_DATA aKnownTags[] =
{
//  { "GROUP_FORMAT", TAG_GROUP_FORMAT },
    { "BOLD", TAG_BOLDON },
    { "/BOLD", TAG_BOLDOFF },
    { "ITALIC", TAG_ITALICON },
    { "/ITALIC", TAG_ITALICOFF },
    { "UNDER", TAG_UNDERLINEON },
    { "/UNDER", TAG_UNDERLINEOFF },

//  { "GROUP_NOTALLOWED", TAG_GROUP_NOTALLOWED },
    { "HELPID", TAG_HELPID },
    { "MODIFY", TAG_MODIFY },
    { "REFNR", TAG_REFNR },

//  { "GROUP_STRUCTURE", TAG_GROUP_STRUCTURE },
    { "NAME", TAG_NAME },
    { "HREF", TAG_HREF },
    { "AVIS", TAG_AVIS },
    { "AHID", TAG_AHID },
    { "AEND", TAG_AEND },

    { "TITEL", TAG_TITEL },
    { "KEY", TAG_KEY },
    { "INDEX", TAG_INDEX },

//  { "GROUP_SYSSWITCH", TAG_GROUP_SYSSWITCH },
    { "WIN", TAG_WIN },
    { "UNIX", TAG_UNIX },
    { "MAC", TAG_MAC },
    { "OS2", TAG_OS2 },

//  { "GROUP_PROGSWITCH", TAG_GROUP_PROGSWITCH },
    { "WRITER", TAG_WRITER },
    { "CALC", TAG_CALC },
    { "DRAW", TAG_DRAW },
    { "IMPRESS", TAG_IMPRESS },
    { "SCHEDULE", TAG_SCHEDULE },
    { "IMAGE", TAG_IMAGE },
    { "MATH", TAG_MATH },
    { "CHART", TAG_CHART },
    { "OFFICE", TAG_OFFICE },

//  { "GROUP_MULTI", TAG_GROUP_MULTI },
    { "END", TAG_END },
    { "ELSE", TAG_ELSE },
    { "", TAG_UNKNOWN_TAG },
};


SimpleParser::SimpleParser()
: nPos( 0 )
{
}

void SimpleParser::Parse( ByteString PaSource )
{
    aSource = PaSource;
    nPos = 0;
    aLastToken = "";
    aTokenList.Clear();
};

Token SimpleParser::GetNextToken()
{
    aLastToken = GetNextTokenString();
    if ( aLastToken == "" )
        return TAG_NOMORETAGS;

    USHORT i = 0;
    while ( aKnownTags[i].nTag != TAG_UNKNOWN_TAG &&
        aLastToken != aKnownTags[i].aName )
        i++;

    aTokenList.Insert( aKnownTags[i].nTag, LIST_APPEND );
    return aKnownTags[i].nTag;
}

ByteString SimpleParser::GetTokenText()
{
    return aLastToken;
}

ByteString SimpleParser::GetNextTokenString()
{
    USHORT nStartPos = aSource.Search( "<#", nPos );
    if ( STRING_NOTFOUND == nStartPos )
        return "";
    USHORT nEndPos = aSource.Search( ">", nStartPos );
    nPos = nEndPos;
    return aSource.Copy( nStartPos+2, nEndPos-nStartPos -2 ).ToUpperAscii();
}

ByteString SimpleParser::GetLexem( Token aToken )
{
    USHORT i = 0;
    while ( aKnownTags[i].nTag != TAG_UNKNOWN_TAG &&
        aKnownTags[i].nTag != aToken )
        i++;
    ByteString sReturn( "<#" );
    sReturn += aKnownTags[i].aName;
    sReturn += ">";
    return sReturn;
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
    nTag = aParser.GetNextToken();

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
    if ( nTag != TAG_NOMORETAGS )
    {
        switch ( nTag )
        {
            case TAG_END:
                {
                    ParseError( 3, "Switch or <#HREF> or <#NAME> expected." );
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
                    ParseError( 5, "Switch or <#HREF> or <#NAME> expected." );
                }
                break;
            case TAG_ELSE:
                {
                    ParseError( 16, "Application-tag or platform-tag expected before <#ELSE>." );
                }
                break;
            default:
                {
                    ByteString sTmp( "unknown tag: " );
                    sTmp += aParser.GetLexem( nTag );
                    ParseError( 6, sTmp );
                }
        }
    }
}

void TokenParser::Paragraph()
{
    switch ( nTag )
    {
        case TAG_AVIS:
        case TAG_AHID:
            {
            //  TagPair();
            //  Paragraph();
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
        case TAG_BOLDON:
        case TAG_ITALICON:
        case TAG_UNDERLINEON:
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
    switch ( nTag )
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
    switch ( nTag )
    {
        case TAG_OS2:
        case TAG_WIN:
        case TAG_UNIX:
        case TAG_MAC:
            {
                //Token darf noch nicht vorgekommen sein im
                //aktuellen Plattform-Case:
                if ( !HAS_FLAG( nPfCaseOptions, TAG_NOGROUP( nTag ) ) )
                {
                    SET_FLAG( nPfCaseOptions, TAG_NOGROUP( nTag ) );
                    match( nTag, nTag );
                }
                else {
                    ByteString sTmp( aParser.GetLexem( nTag ));
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

    switch ( nTag )
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
    switch ( nTag )
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
                if ( !HAS_FLAG( nAppCaseOptions, TAG_NOGROUP( nTag ) ) )
                {
                    SET_FLAG( nAppCaseOptions, TAG_NOGROUP( nTag ) );
                    match( nTag, nTag );
                }
                else {
                    ByteString sTmp( aParser.GetLexem( nTag ));
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

    switch ( nTag )
    {
        case TAG_ELSE:
        {
            match( nTag, TAG_ELSE );
            Paragraph();
            match( nTag, TAG_END );
        }
        break;
        case TAG_END:
        {
            match( nTag, TAG_END );
        }
        break;
        default:
            ParseError( 2, "<#ELSE> or <#END> expected." );
    }
}

void TokenParser::SimpleTag()
{

    switch ( nTag )
    {
        case TAG_HELPID:
            {
                match( nTag, TAG_HELPID );
            }
            break;
        default:
            ParseError( 15, "[<#SimpleTag>] expected." );
    }
}

void TokenParser::TagPair()
{
    switch ( nTag )
    {
        case TAG_BOLDON:
            {
                match( nTag, TAG_BOLDON );
                Paragraph();
                match( nTag, TAG_BOLDOFF );
            }
            break;
        case TAG_ITALICON:
            {
                match( nTag, TAG_ITALICON );
                Paragraph();
                match( nTag, TAG_ITALICOFF );
            }
            break;
        case TAG_UNDERLINEON:
            {
                match( nTag, TAG_UNDERLINEON );
                Paragraph();
                match( nTag, TAG_UNDERLINEOFF );
            }
            break;
        default:
            ParseError( 10, "<#BOLD>, <#ITALIC>, <#UNDER> expected." );
    }
}


void TokenParser::TagRef()
{
    switch ( nTag )
    {
        case TAG_AVIS:
        case TAG_AHID:
            {
                if ( !HAS_FLAG( nActiveRefTypes, TAG_NOGROUP( nTag ) ) )
                {
                    Token aThisToken = nTag;
                    SET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                    match( nTag, nTag );
                    Paragraph();
                    match( nTag, TAG_AEND );
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
            {
                if ( !HAS_FLAG( nActiveRefTypes, TAG_NOGROUP( nTag ) ) )
                {
                    Token aThisToken = nTag;
                    SET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
                    match( nTag, nTag );
                    Paragraph();
                    match( nTag, TAG_END );
                    RESET_FLAG( nActiveRefTypes, TAG_NOGROUP( aThisToken ) );
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

BOOL TokenParser::match( const Token &aCurrentToken, const Token &aExpectedToken )
{
    if ( aCurrentToken == aExpectedToken )
    {
        nTag = aParser.GetNextToken();
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
    nTag = aParser.GetNextToken();
}

void LingTest::CheckMandatoryTag( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList, Token aToken )
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

void LingTest::CheckTags( TokenList aReference, TokenList aTestee, ParserMessageList &rErrorList )
{
    ULONG i=0,j=0;
    // Clean old Warnings
    while ( aCompareWarningList.Count() )
    {
        delete aCompareWarningList.GetCurObject();
        aCompareWarningList.Remove();
    }

    // First check the mandatory Tags
/*  Check below is much stricter allready
    CheckMandatoryTag( aReference, aTestee, rErrorList, TAG_AVIS );
    CheckMandatoryTag( aReference, aTestee, rErrorList, TAG_AHID );
    CheckMandatoryTag( aReference, aTestee, rErrorList, TAG_HREF );
    CheckMandatoryTag( aReference, aTestee, rErrorList, TAG_NAME );
*/
    i=0;
    while ( i < aReference.Count() )
    {
        Token aToken = aReference.GetObject( i );
        Token aTokenGroup = TAG_GROUP( aToken );
        if ( TAG_GROUP_PROGSWITCH == aTokenGroup
            || TAG_NAME == aToken
            || TAG_HREF == aToken
            || TAG_AVIS == aToken
            || TAG_AHID == aToken )
            i++;
        else
            aReference.Remove( i );
    }

    i=0;
    while ( i < aTestee.Count() )
    {
        Token aToken = aTestee.GetObject( i );
        Token aTokenGroup = TAG_GROUP( aToken );
        if ( TAG_GROUP_PROGSWITCH == aTokenGroup
            || TAG_NAME == aToken
            || TAG_HREF == aToken
            || TAG_AVIS == aToken
            || TAG_AHID == aToken )
            i++;
        else
            aTestee.Remove( i );
    }

    BOOL bFatal = FALSE;
    i=0,j=0;
    while ( ( i < aReference.Count() || j < aTestee.Count() ) && !bFatal )
    {
        if ( aReference.GetObject( i ) != aTestee.GetObject( j ) )
        {
            ParserMessage *pNewWarning = NULL;
            if ( j >= aTestee.Count() || aReference.GetObject( i+1 ) == aTestee.GetObject( j )
                                      || aReference.GetObject( i+2 ) == aTestee.GetObject( j ) )
            {
                ByteString sTmp( "Missing Tag in Translation: " );
                sTmp += SimpleParser::GetLexem( aReference.GetObject( i ));
                pNewWarning =  new ParserMessage( 20, sTmp );
                aReference.Remove( i );
            }
            else if ( i >= aReference.Count() || aReference.GetObject( i ) == aTestee.GetObject( j+1 )
                                              || aReference.GetObject( i ) == aTestee.GetObject( j+2 ) )
            {
                ByteString sTmp( "Extra Tag in Translation: " );
                sTmp += SimpleParser::GetLexem( aTestee.GetObject( j ));
                pNewWarning =  new ParserMessage( 21, sTmp );
                aTestee.Remove( j );
            }
            else
            {
                pNewWarning = new ParserMessage( 22, "Tags Differ. Too many differences to be specified" );
                bFatal = TRUE;
            }

            if ( pNewWarning )
                aCompareWarningList.Insert( pNewWarning, LIST_APPEND );
        }
        else
            i++,j++;
    }
}

BOOL LingTest::ReferenceOK( const ByteString &aReference )
{
    aReferenceParser.Parse( aReference );
    return !aReferenceParser.HasErrors();
}

BOOL LingTest::TesteeOK( const ByteString &aTestee )
{
    aTesteeParser.Parse( aTestee );

    CheckTags( aReferenceParser.GetTokenList(), aTesteeParser.GetTokenList(), aTesteeParser.GetErrors() );

    return !aTesteeParser.HasErrors() && !HasCompareWarnings();
}

