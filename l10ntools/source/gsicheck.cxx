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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_l10ntools.hxx"
#include <stdio.h>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>

// local includes
#include "tagtest.hxx"
#include "gsicheck.hxx"

#define MAX_GID_LID_LEN 250

/*****************************************************************************/
void PrintMessage( ByteString aType, ByteString aMsg, ByteString aPrefix,
    ByteString aContext, BOOL bPrintContext, ULONG nLine, ByteString aUniqueId = ByteString() )
/*****************************************************************************/
{
    fprintf( stdout, "%s %s, Line %lu", aType.GetBuffer(), aPrefix.GetBuffer(), nLine );
    if ( aUniqueId.Len() )
        fprintf( stdout, ", UniqueID %s", aUniqueId.GetBuffer() );
    fprintf( stdout, ": %s", aMsg.GetBuffer() );

    if ( bPrintContext )
        fprintf( stdout, "  \"%s\"", aContext.GetBuffer() );
    fprintf( stdout, "\n" );
}

/*****************************************************************************/
void PrintError( ByteString aMsg, ByteString aPrefix,
    ByteString aContext, BOOL bPrintContext, ULONG nLine, ByteString aUniqueId = ByteString() )
/*****************************************************************************/
{
    PrintMessage( "Error:", aMsg, aPrefix, aContext, bPrintContext, nLine, aUniqueId );
}

BOOL LanguageOK( ByteString aLang )
{
    if ( !aLang.Len() )
        return FALSE;

    if ( aLang.IsNumericAscii() )
        return TRUE;

    if ( aLang.GetTokenCount( '-' ) == 1 )
        return aLang.IsAlphaAscii() && aLang.IsLowerAscii();
    else if ( aLang.GetTokenCount( '-' ) == 2 )
    {
        ByteString aTok0( aLang.GetToken( 0, '-' ) );
        ByteString aTok1( aLang.GetToken( 1, '-' ) );
        return  aTok0.Len() && aTok0.IsAlphaAscii() && aTok0.IsLowerAscii()
             && aTok1.Len() && aTok1.IsAlphaAscii() && aTok1.IsUpperAscii()
             && !aTok1.EqualsIgnoreCaseAscii( aTok0 );
    }

    return FALSE;
}


//
// class LazySvFileStream
//


class LazySvFileStream : public SvFileStream
{

private:
    String aFileName;
    BOOL bOpened;
    StreamMode eOpenMode;

public:
    LazySvFileStream()
    : aFileName()
    , bOpened( FALSE )
    , eOpenMode( 0 )
    {};

    void SetOpenParams( const String& rFileName, StreamMode eOpenModeP )
    {
        aFileName = rFileName;
        eOpenMode = eOpenModeP;
    };

    void LazyOpen();
};

void LazySvFileStream::LazyOpen()
{
    if ( !bOpened )
    {
        Open( aFileName, eOpenMode );
        if ( !IsOpen())
        {
            fprintf( stderr, "\nERROR: Could not open Output-File %s!\n\n", ByteString( aFileName, RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            exit ( 4 );
        }
        bOpened = TRUE;
    }
}


//
// class GSILine
//

/*****************************************************************************/
GSILine::GSILine( const ByteString &rLine, ULONG nLine )
/*****************************************************************************/
                : ByteString( rLine )
                , nLineNumber( nLine )
                , bOK( TRUE )
                , bFixed ( FALSE )
{
    if ( rLine.GetTokenCount( '\t' ) == 15 )
    {
        aFormat = FORMAT_SDF;
        aUniqId = rLine.GetToken( 0, '\t' );
        aUniqId.Append("/").Append( rLine.GetToken( 1, '\t' ) ).Append("/").Append( rLine.GetToken( 3, '\t' ) ).Append("/").Append( rLine.GetToken( 4, '\t' ) ).Append("/").Append( rLine.GetToken( 5, '\t' ) ).Append("/").Append( rLine.GetToken( 6, '\t' ) ).Append("/").Append( rLine.GetToken( 7, '\t' ) );
        aLineType = "";
        aLangId = rLine.GetToken( 9, '\t' );
        aText = rLine.GetToken( 10, '\t' );
        aQuickHelpText = rLine.GetToken( 12, '\t' );
        aTitle = rLine.GetToken( 13, '\t' );

        // do some more format checks here
        if ( !rLine.GetToken( 8, '\t' ).IsNumericAscii() )
        {
            PrintError( "The length field does not contain a number!", "Line format", rLine.GetToken( 8, '\t' ), TRUE, GetLineNumber(), GetUniqId() );
            NotOK();
        }
        if ( !LanguageOK( aLangId ) )
        {
            PrintError( "The Language is invalid!", "Line format", aLangId, TRUE, GetLineNumber(), GetUniqId() );
            NotOK();
        }
        // limit GID and LID to MAX_GID_LID_LEN chars each for database conformity, see #137575#
        if ( rLine.GetToken( 4, '\t' ).Len() > MAX_GID_LID_LEN || rLine.GetToken( 5, '\t' ).Len() > MAX_GID_LID_LEN )
        {
            PrintError( ByteString("GID and LID may only be ").Append( ByteString::CreateFromInt32(MAX_GID_LID_LEN) ).Append( " chars long each!" ), "Line format", aLangId, TRUE, GetLineNumber(), GetUniqId() );
            NotOK();
        }
    }
    else    // allow tabs in gsi files
    {
        aFormat = FORMAT_GSI;
        ByteString sTmp( rLine );
        USHORT nPos = sTmp.Search( "($$)" );
        USHORT nStart = 0;
        if ( nPos != STRING_NOTFOUND )
        {
            aUniqId = sTmp.Copy( nStart, nPos - nStart );
            nStart = nPos + 4;  // + length of the delemiter
            nPos = sTmp.Search( "($$)", nStart );
        }
        if ( nPos != STRING_NOTFOUND )
        {
            aLineType = sTmp.Copy( nStart, nPos - nStart );
            nStart = nPos + 4;  // + length of the delemiter
            nPos = sTmp.Search( "($$)", nStart );
            aUniqId.Append( "/" );
            aUniqId.Append( aLineType );
        }
        if ( nPos != STRING_NOTFOUND )
        {
            aLangId = sTmp.Copy( nStart, nPos - nStart );
            nStart = nPos + 4;  // + length of the delemiter
            nPos = sTmp.Search( "($$)", nStart );
        }
        if ( nPos != STRING_NOTFOUND )
        {
            nStart = nPos + 4;  // + length of the delemiter
        }
        if ( nPos != STRING_NOTFOUND )
            aText = sTmp.Copy( nStart );
        else
            aFormat = FORMAT_UNKNOWN;
    }

    if ( FORMAT_UNKNOWN == GetLineFormat() )
        NotOK();
}

/*****************************************************************************/
void GSILine::NotOK()
/*****************************************************************************/
{
    bOK = FALSE;
}

/*****************************************************************************/
void GSILine::ReassembleLine()
/*****************************************************************************/
{
    ByteString aReassemble;
    if ( GetLineFormat() == FORMAT_SDF )
    {
        USHORT i;
        for ( i = 0 ; i < 10 ; i++ )
        {
            aReassemble.Append( GetToken( i, '\t' ) );
            aReassemble.Append( "\t" );
        }
        aReassemble.Append( aText );
        aReassemble.Append( "\t" );
        aReassemble.Append( GetToken( 11, '\t' ) ); // should be empty but there are some places in sc. Not reflected to sources!!
        aReassemble.Append( "\t" );
        aReassemble.Append( aQuickHelpText );
        aReassemble.Append( "\t" );
        aReassemble.Append( aTitle );
        for ( i = 14 ; i < 15 ; i++ )
        {
            aReassemble.Append( "\t" );
            aReassemble.Append( GetToken( i, '\t' ) );
        }
        *(ByteString*)this = aReassemble;
    }
    else if ( GetLineFormat() == FORMAT_GSI )
    {
        USHORT nPos = Search( "($$)" );
        USHORT nStart = 0;
        if ( nPos != STRING_NOTFOUND )
        {
            nStart = nPos + 4;  // + length of the delemiter
            nPos = Search( "($$)", nStart );
        }
        if ( nPos != STRING_NOTFOUND )
        {
            nStart = nPos + 4;  // + length of the delemiter
            nPos = Search( "($$)", nStart );
        }
        if ( nPos != STRING_NOTFOUND )
        {
            nStart = nPos + 4;  // + length of the delemiter
            nPos = Search( "($$)", nStart );
        }
        if ( nPos != STRING_NOTFOUND )
        {
            nStart = nPos + 4;  // + length of the delemiter
        }
        if ( nPos != STRING_NOTFOUND )
        {
            aReassemble = Copy( 0, nStart );
            aReassemble += aText;
            *(ByteString*)this = aReassemble;
        }
        else
            PrintError( "Cannot reassemble GSI line (internal Error).", "Line format", "", FALSE, GetLineNumber(), GetUniqId() );
    }
    else
        PrintError( "Cannot reassemble line of unknown type (internal Error).", "Line format", "", FALSE, GetLineNumber(), GetUniqId() );
}

//
// class GSIBlock
//
/*****************************************************************************/
GSIBlock::GSIBlock( BOOL PbPrintContext, BOOL bSource, BOOL bTrans, BOOL bRef, BOOL bAllowKID, BOOL bAllowSusp )
/*****************************************************************************/
            : pSourceLine( NULL )
            , pReferenceLine( NULL )
            , bPrintContext( PbPrintContext )
            , bCheckSourceLang( bSource )
            , bCheckTranslationLang( bTrans )
            , bReference( bRef )
            , bAllowKeyIDs( bAllowKID )
            , bAllowSuspicious( bAllowSusp )
            , bHasBlockError( FALSE )
{
}

/*****************************************************************************/
GSIBlock::~GSIBlock()
/*****************************************************************************/
{
    delete pSourceLine;
    delete pReferenceLine;

    for ( size_t i = 0, n = maList.size(); i < n; ++i )
        delete maList[ i ];
    maList.clear();
}

/*****************************************************************************/
void GSIBlock::InsertLine( GSILine* pLine, ByteString aSourceLang)
/*****************************************************************************/
{
    if ( pLine->GetLanguageId().Equals( aSourceLang ) )
    {
        if ( pSourceLine )
        {
            PrintError( "Source Language entry double. Treating as Translation.", "File format", "", pLine->GetLineNumber(), pLine->GetUniqId() );
            bHasBlockError = TRUE;
            pSourceLine->NotOK();
            pLine->NotOK();
        }
        else
        {
            pSourceLine = pLine;
            return;
        }
    }

    if ( aSourceLang.Len() ) // only check blockstructure if source lang is given
    {
        for ( size_t nPos = 0, n = maList.size(); nPos < n; ++nPos )
        {
            if ( maList[ nPos ]->GetLanguageId().Equals( pLine->GetLanguageId() ) )
            {
                PrintError( "Translation Language entry double. Checking both.", "File format", "", pLine->GetLineNumber(), pLine->GetUniqId() );
                bHasBlockError = TRUE;
                maList[ nPos ]->NotOK();
                pLine->NotOK();
            }
            nPos++;
        }
    }
    maList.push_back( pLine );
}

/*****************************************************************************/
void GSIBlock::SetReferenceLine( GSILine* pLine )
/*****************************************************************************/
{
    pReferenceLine = pLine;
}

/*****************************************************************************/
void GSIBlock::PrintMessage( ByteString aType, ByteString aMsg, ByteString aPrefix,
    ByteString aContext, ULONG nLine, ByteString aUniqueId )
/*****************************************************************************/
{
    ::PrintMessage( aType, aMsg, aPrefix, aContext, bPrintContext, nLine, aUniqueId );
}

/*****************************************************************************/
void GSIBlock::PrintError( ByteString aMsg, ByteString aPrefix,
    ByteString aContext, ULONG nLine, ByteString aUniqueId )
/*****************************************************************************/
{
    PrintMessage( "Error:", aMsg, aPrefix, aContext, nLine, aUniqueId );
}

/*****************************************************************************/
void GSIBlock::PrintList( ParserMessageList *pList, ByteString aPrefix,
    GSILine *pLine )
/*****************************************************************************/
{
    for ( size_t i = 0 ; i < pList->size() ; i++ )
    {
        ParserMessage *pMsg = (*pList)[ i ];
        ByteString aContext;
        if ( bPrintContext )
        {
            if ( pMsg->GetTagBegin() == STRING_NOTFOUND )
                aContext = pLine->GetText().Copy( 0, 300 );
            else
                aContext = pLine->Copy( pMsg->GetTagBegin()-150, 300 );
            aContext.EraseTrailingChars(' ');
            aContext.EraseLeadingChars(' ');
        }

        PrintMessage( pMsg->Prefix(), pMsg->GetErrorText(), aPrefix, aContext, pLine->GetLineNumber(), pLine->GetUniqId() );
    }
}

/*****************************************************************************/
BOOL GSIBlock::IsUTF8( const ByteString &aTestee, BOOL bFixTags, USHORT &nErrorPos, ByteString &aErrorMsg, BOOL &bHasBeenFixed, ByteString &aFixed ) const
/*****************************************************************************/
{
    String aUTF8Tester( aTestee, RTL_TEXTENCODING_UTF8 );
    if ( STRING_MATCH != (nErrorPos = ByteString( aUTF8Tester, RTL_TEXTENCODING_UTF8 ).Match( aTestee )) )
    {
        aUTF8Tester = String( aTestee.GetBuffer(), nErrorPos, RTL_TEXTENCODING_UTF8 );
        nErrorPos = aUTF8Tester.Len();
        aErrorMsg = ByteString( "UTF8 Encoding seems to be broken" );
        return FALSE;
    }

    nErrorPos = aUTF8Tester.SearchChar( String::CreateFromAscii( "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0b\x0c\x0e\x0f"
                "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f" ).GetBuffer() );
    if ( nErrorPos != STRING_NOTFOUND )
    {
        aErrorMsg = ByteString( "String contains illegal character" );
        return FALSE;
    }

    if ( bFixTags )
    {
        bHasBeenFixed = FALSE;
        aFixed.Erase();
    }

    if ( !bAllowKeyIDs )
    {
        BOOL bIsKeyID = FALSE;
        BOOL bNewId = FALSE;
        ByteString aID( aTestee );
        USHORT nAfterID = 0;

        if ( aTestee.Equals( "{&", 0, 2 ) )
        {   // check for strings from instset_native like "{&Tahoma8}335795.Installation Wiza ..."
            USHORT nTagEnd = aTestee.Search( '}' );
            if ( nTagEnd != STRING_NOTFOUND )
            {
                if ( bFixTags )
                    aFixed = aTestee.Copy( 0, nTagEnd+1 );
                nErrorPos = nTagEnd+1;
                aID = aTestee.Copy( nTagEnd+1 );
                nAfterID = nTagEnd+1;
            }
        }

        ByteString aDelimiter( (String)String( sal_Unicode(0x2016) ), RTL_TEXTENCODING_UTF8 );

        if ( aID.Equals( aDelimiter, 6, aDelimiter.Len() ) )
        {   // New KeyId     6 Letters, digits and spechial chars followed by delimiter
            bNewId = TRUE;
            nErrorPos = 1;
            aID = aID.Copy( 0, 6 );
            nAfterID += 6;
            nAfterID = nAfterID + aDelimiter.Len();
        }
        else if ( ( aID.GetChar(6) == '*' ) && aID.Equals( aDelimiter, 7, aDelimiter.Len() ) )
        {   // New KeyId     6 Letters, digits and spechial chars followed by '*delimiter' to indicate translation in progress
            bNewId = TRUE;
            nErrorPos = 1;
            aID = aID.Copy( 0, 6 );
            nAfterID += 7;
            nAfterID = nAfterID + aDelimiter.Len();
        }
        else if ( aID.GetTokenCount( '.' ) > 1 )
        {   // test for old KeyIDs       5 to 6 digits followed by a dot   '44373.'
            bNewId = FALSE;
            nErrorPos = 1;
            aID = aID.GetToken( 0, '.' );
            nAfterID = nAfterID + aID.Len();
        }
        else
        {
            aID.Erase();
        }

        if ( bNewId )
            {
                if ( aID.Len() == 6 )
                {
                    bIsKeyID = TRUE;
                    ByteString aDigits("0123456789abcdefghijklmnopqrstuvwxyz+-<=>");
                    for ( USHORT i=0 ; i < aID.Len() ;i++ )
                    {
                        if ( aDigits.Search( aID.GetChar(i) ) == STRING_NOTFOUND )
                            bIsKeyID = FALSE;
                    }
                }
            }
        else
        {
            if ( aID.Len() > 0 && aID.GetChar(aID.Len()-1) == '*' )
                aID.Erase( aID.Len()-1 );

            if ( aID.IsNumericAscii() && aID.Len() >= 5 )
                bIsKeyID = TRUE;
        }

        if ( bIsKeyID )
        {
            aErrorMsg = ByteString( "String contains KeyID" );
            if ( bFixTags )
            {
                aFixed += aTestee.Copy( nAfterID );
                bHasBeenFixed = TRUE;
                aErrorMsg = ByteString( "FIXED String containing KeyID" );
            }
            else
                aErrorMsg = ByteString( "String contains KeyID" );
            return FALSE;
        }
    }

    return TRUE;
}

/*****************************************************************************/
BOOL GSIBlock::TestUTF8( GSILine* pTestee, BOOL bFixTags )
/*****************************************************************************/
{
    USHORT nErrorPos = 0;
    ByteString aErrorMsg;
    BOOL bError = FALSE;
    ByteString aFixed;
    BOOL bHasBeenFixed = FALSE;
    if ( !IsUTF8( pTestee->GetText(), bFixTags, nErrorPos, aErrorMsg, bHasBeenFixed, aFixed ) )
    {
        ByteString aContext( pTestee->GetText().Copy( nErrorPos, 20 ) );
        PrintError( aErrorMsg.Append(" in Text at Position " ).Append( ByteString::CreateFromInt32( nErrorPos ) ), "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId() );
        bError = TRUE;
        if ( bHasBeenFixed )
        {
            pTestee->SetText( aFixed );
            pTestee->SetFixed();
        }
    }
    if ( !IsUTF8( pTestee->GetQuickHelpText(), bFixTags, nErrorPos, aErrorMsg, bHasBeenFixed, aFixed ) )
    {
        ByteString aContext( pTestee->GetQuickHelpText().Copy( nErrorPos, 20 ) );
        PrintError( aErrorMsg.Append(" in QuickHelpText at Position " ).Append( ByteString::CreateFromInt32( nErrorPos ) ), "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId() );
        bError = TRUE;
        if ( bHasBeenFixed )
        {
            pTestee->SetQuickHelpText( aFixed );
            pTestee->SetFixed();
        }
    }
    if ( !IsUTF8( pTestee->GetTitle(), bFixTags, nErrorPos, aErrorMsg, bHasBeenFixed, aFixed ) )
    {
        ByteString aContext( pTestee->GetTitle().Copy( nErrorPos, 20 ) );
        PrintError( aErrorMsg.Append(" in Title at Position " ).Append( ByteString::CreateFromInt32( nErrorPos ) ), "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId() );
        bError = TRUE;
        if ( bHasBeenFixed )
        {
            pTestee->SetTitle( aFixed );
            pTestee->SetFixed();
        }
    }
    if ( bError )
        pTestee->NotOK();
    return !bError;
}


/*****************************************************************************/
BOOL GSIBlock::HasSuspiciousChars( GSILine* pTestee, GSILine* pSource )
/*****************************************************************************/
{
    USHORT nPos = 0;
    if ( !bAllowSuspicious && ( nPos = pTestee->GetText().Search("??")) != STRING_NOTFOUND )
        if ( pSource->GetText().Search("??") == STRING_NOTFOUND )
        {
            String aUTF8Tester = String( pTestee->GetText(), 0, nPos, RTL_TEXTENCODING_UTF8 );
            USHORT nErrorPos = aUTF8Tester.Len();
            ByteString aContext( pTestee->GetText().Copy( nPos, 20 ) );
            PrintError( ByteString("Found double questionmark in translation only. Looks like an encoding problem at Position " ).Append( ByteString::CreateFromInt32( nErrorPos ) ), "Text format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId() );
            pTestee->NotOK();
            return TRUE;
        }

    return FALSE;
}


/*****************************************************************************/
BOOL GSIBlock::CheckSyntax( ULONG nLine, BOOL bRequireSourceLine, BOOL bFixTags )
/*****************************************************************************/
{
    static LingTest aTester;
    BOOL bHasError = FALSE;

    if ( !pSourceLine )
    {
        if ( bRequireSourceLine )
        {
            PrintError( "No source language entry defined!", "File format", "", nLine );
            bHasBlockError = TRUE;
        }
    }
    else
    {
        aTester.CheckReference( pSourceLine );
        if ( pSourceLine->HasMessages() )
        {
            PrintList( pSourceLine->GetMessageList(), "ReferenceString", pSourceLine );
            pSourceLine->NotOK();
            bHasError = TRUE;
        }
    }
    if ( bReference )
    {
        if ( !pReferenceLine )
        {
            GSILine *pSource;
            if ( pSourceLine )
                pSource = pSourceLine;
            else
                pSource = maList.empty() ? NULL : maList[ 0 ];   // get some other line
            if ( pSource )
                PrintError( "No reference line found. Entry is new in source file", "File format", "", pSource->GetLineNumber(), pSource->GetUniqId() );
            else
                PrintError( "No reference line found. Entry is new in source file", "File format", "", nLine );
            bHasBlockError = TRUE;
        }
        else
        {
            if ( pSourceLine && !pSourceLine->Equals( *pReferenceLine ) )
            {
                xub_StrLen nPos = pSourceLine->Match( *pReferenceLine );
                ByteString aContext( pReferenceLine->Copy( nPos - 5, 15) );
                aContext.Append( "\" --> \"" ).Append( pSourceLine->Copy( nPos - 5, 15) );
                PrintError( "Source Language Entry has changed.", "File format", aContext, pSourceLine->GetLineNumber(), pSourceLine->GetUniqId() );
                pSourceLine->NotOK();
                bHasError = TRUE;
            }
        }
    }

    if ( pSourceLine )
        bHasError |= !TestUTF8( pSourceLine, bFixTags );

    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        aTester.CheckTestee( pItem, pSourceLine != NULL, bFixTags );
        if ( pItem->HasMessages() || aTester.HasCompareWarnings() )
        {
            if ( pItem->HasMessages() || aTester.GetCompareWarnings().HasErrors() )
                pItem->NotOK();
            bHasError = TRUE;
            PrintList( pItem->GetMessageList(), "Translation", pItem );
            PrintList( &(aTester.GetCompareWarnings()), "Translation Tag Mismatch", pItem );
        }
        bHasError |= !TestUTF8( pItem, bFixTags );
        if ( pSourceLine )
            bHasError |= HasSuspiciousChars( pItem, pSourceLine );
    }

    return bHasError || bHasBlockError;
}

void GSIBlock::WriteError( LazySvFileStream &aErrOut, BOOL bRequireSourceLine  )
{
    if ( pSourceLine && pSourceLine->IsOK() && bCheckSourceLang && !bHasBlockError )
        return;

    BOOL bHasError = FALSE;
    BOOL bCopyAll = ( !pSourceLine && bRequireSourceLine ) || ( pSourceLine && !pSourceLine->IsOK() && !bCheckTranslationLang ) || bHasBlockError;
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        if ( !pItem->IsOK() || bCopyAll )
        {
            bHasError = TRUE;
            aErrOut.LazyOpen();
            aErrOut.WriteLine( *pItem );
        }
    }

    if ( pSourceLine && ( bHasError || !pSourceLine->IsOK() ) && !( !bHasError && bCheckTranslationLang ) )
    {
        aErrOut.LazyOpen();
        aErrOut.WriteLine( *pSourceLine );
    }
}

void GSIBlock::WriteCorrect( LazySvFileStream &aOkOut, BOOL bRequireSourceLine )
{
    if ( ( !pSourceLine && bRequireSourceLine ) || ( pSourceLine && !pSourceLine->IsOK() && !bCheckTranslationLang ) )
        return;

    BOOL bHasOK = FALSE;
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        if ( ( pItem->IsOK() || bCheckSourceLang ) && !bHasBlockError )
        {
            bHasOK = TRUE;
            aOkOut.LazyOpen();
            aOkOut.WriteLine( *pItem );
        }
    }

    if ( ( pSourceLine && pSourceLine->IsOK() && ( !maList.empty() || !bCheckTranslationLang ) ) || ( bHasOK && bCheckTranslationLang ) )
    {
        aOkOut.LazyOpen();
        aOkOut.WriteLine( *pSourceLine );
    }
}

void GSIBlock::WriteFixed( LazySvFileStream &aFixOut, BOOL /*bRequireSourceLine*/ )
{
    if ( pSourceLine && !pSourceLine->IsFixed() && bCheckSourceLang )
        return;

    BOOL bHasFixes = FALSE;
    for ( size_t i = 0, n = maList.size(); i < n; ++i )
    {
        GSILine* pItem = maList[ i ];
        if ( pItem->IsFixed() )
        {
            bHasFixes = TRUE;
            aFixOut.LazyOpen();
            aFixOut.WriteLine( *pItem );
        }
    }

    if ( pSourceLine && ( bHasFixes || pSourceLine->IsFixed() ) )
    {
        aFixOut.LazyOpen();
        aFixOut.WriteLine( *pSourceLine );
    }
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck Version 1.9.0 (c)1999 - 2006 by SUN Microsystems\n" );
    fprintf( stdout, "=========================================================\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck checks the syntax of tags in GSI-Files and SDF-Files\n" );
    fprintf( stdout, "         checks for inconsistencies and malicious UTF8 encoding\n" );
    fprintf( stdout, "         checks tags in Online Help\n" );
    fprintf( stdout, "         checks for *new* KeyIDs and relax GID/LID length to %s\n", ByteString::CreateFromInt32(MAX_GID_LID_LEN).GetBuffer() );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsicheck [ -c ] [-f] [ -we ] [ -wef ErrorFilename ] [ -wc ]\n" );
    fprintf( stdout, "                 [ -wcf CorrectFilename ] [ -s | -t ] [ -l LanguageID ]\n" );
    fprintf( stdout, "                 [ -r ReferenceFile ] filename\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "-c    Add context to error message (Print the line containing the error)\n" );
    fprintf( stdout, "-f    try to fix errors. See also -wf -wff \n" );
    fprintf( stdout, "-wf   Write File containing all fixed parts\n" );
    fprintf( stdout, "-wff  Same as above but give own filename\n" );
    fprintf( stdout, "-we   Write File containing all errors\n" );
    fprintf( stdout, "-wef  Same as above but give own filename\n" );
    fprintf( stdout, "-wc   Write File containing all correct parts\n" );
    fprintf( stdout, "-wcf  Same as above but give own filename\n" );
    fprintf( stdout, "-s    Check only source language. Should be used before handing out to vendor.\n" );
    fprintf( stdout, "-t    Check only Translation language(s). Should be used before merging.\n" );
    fprintf( stdout, "-k    Allow KeyIDs to be present in strings\n" );
    fprintf( stdout, "-e    disable encoding checks. E.g.: double questionmark \'??\' which may be the\n" );
    fprintf( stdout, "      result of false conversions\n" );
    fprintf( stdout, "-l    ISO Languagecode or numerical 2 digits Identifier of the source language.\n" );
    fprintf( stdout, "      Default is en-US. Use \"\" (empty string) or 'none'\n" );
    fprintf( stdout, "      to disable source language dependent checks\n" );
    fprintf( stdout, "-r    Reference filename to check that source language entries\n" );
    fprintf( stdout, "      have not been changed\n" );
       fprintf( stdout, "\n" );
}

/*****************************************************************************/
#if defined(UNX) || defined(OS2)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{

    BOOL bError = FALSE;
    BOOL bPrintContext = FALSE;
    BOOL bCheckSourceLang = FALSE;
    BOOL bCheckTranslationLang = FALSE;
    BOOL bWriteError = FALSE;
    BOOL bWriteCorrect = FALSE;
    BOOL bWriteFixed = FALSE;
    BOOL bFixTags = FALSE;
    BOOL bAllowKID = FALSE;
    BOOL bAllowSuspicious = FALSE;
    String aErrorFilename;
    String aCorrectFilename;
    String aFixedFilename;
    BOOL bFileHasError = FALSE;
    ByteString aSourceLang( "en-US" );     // English is default
    ByteString aFilename;
    ByteString aReferenceFilename;
    BOOL bReferenceFile = FALSE;
    for ( USHORT i = 1 ; i < argc ; i++ )
    {
        if ( *argv[ i ] == '-' )
        {
            switch (*(argv[ i ]+1))
            {
                case 'c':bPrintContext = TRUE;
                    break;
                case 'w':
                    {
                        if ( (*(argv[ i ]+2)) == 'e' )
                        {
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aErrorFilename = String( argv[ i+1 ], RTL_TEXTENCODING_ASCII_US );
                                    bWriteError = TRUE;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = TRUE;
                                }
                            else
                                   bWriteError = TRUE;
                        }
                        else if ( (*(argv[ i ]+2)) == 'c' )
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aCorrectFilename = String( argv[ i+1 ], RTL_TEXTENCODING_ASCII_US );
                                    bWriteCorrect = TRUE;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = TRUE;
                                }
                            else
                                   bWriteCorrect = TRUE;
                        else if ( (*(argv[ i ]+2)) == 'f' )
                            if ( (*(argv[ i ]+3)) == 'f' )
                                if ( (i+1) < argc )
                                {
                                    aFixedFilename = String( argv[ i+1 ], RTL_TEXTENCODING_ASCII_US );
                                    bWriteFixed = TRUE;
                                    bFixTags = TRUE;
                                    i++;
                                }
                                else
                                {
                                    fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                                    bError = TRUE;
                                }
                            else
                            {
                                   bWriteFixed = TRUE;
                                bFixTags = TRUE;
                            }
                        else
                        {
                            fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                            bError = TRUE;
                        }
                    }
                    break;
                case 's':bCheckSourceLang = TRUE;
                    break;
                case 't':bCheckTranslationLang = TRUE;
                    break;
                case 'l':
                    {
                        if ( (i+1) < argc )
                        {
                            aSourceLang = ByteString( argv[ i+1 ] );
                            if ( aSourceLang.EqualsIgnoreCaseAscii( "none" ) )
                                aSourceLang.Erase();
                            i++;
                        }
                        else
                        {
                            fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                            bError = TRUE;
                        }
                    }
                    break;
                case 'r':
                    {
                        if ( (i+1) < argc )
                        {
                            aReferenceFilename = argv[ i+1 ];
                            bReferenceFile = TRUE;
                            i++;
                        }
                        else
                        {
                            fprintf( stderr, "\nERROR: Switch %s requires parameter!\n\n", argv[ i ] );
                            bError = TRUE;
                        }
                    }
                    break;
                case 'f':
                    {
                        bFixTags = TRUE;
                    }
                    break;
                case 'k':
                    {
                        bAllowKID = TRUE;
                    }
                    break;
                case 'e':
                    {
                        bAllowSuspicious = TRUE;
                    }
                    break;
                default:
                    fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                    bError = TRUE;
            }
        }
        else
        {
            if  ( !aFilename.Len())
                aFilename = ByteString( argv[ i ] );
            else
            {
                fprintf( stderr, "\nERROR: Only one filename may be specified!\n\n");
                bError = TRUE;
            }
        }
    }


    if ( !aFilename.Len() || bError )
    {
        Help();
        exit ( 0 );
    }

    if ( aSourceLang.Len() && !LanguageOK( aSourceLang ) )
    {
        fprintf( stderr, "\nERROR: The Language '%s' is invalid!\n\n", aSourceLang.GetBuffer() );
        Help();
        exit ( 1 );
    }

    if ( bCheckSourceLang && bCheckTranslationLang )
    {
        fprintf( stderr, "\nERROR: The Options -s and -t are mutually exclusive.\nUse only one of them.\n\n" );
        Help();
        exit ( 1 );
    }



    DirEntry aSource = DirEntry( String( aFilename, RTL_TEXTENCODING_ASCII_US ));
    if ( !aSource.Exists()) {
        fprintf( stderr, "\nERROR: GSI-File %s not found!\n\n", aFilename.GetBuffer() );
        exit ( 2 );
    }

    SvFileStream aGSI( String( aFilename, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
    if ( !aGSI.IsOpen()) {
        fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", aFilename.GetBuffer() );
        exit ( 3 );
    }

    SvFileStream aReferenceGSI;
    if ( bReferenceFile )
    {
        DirEntry aReferenceSource = DirEntry( String( aReferenceFilename, RTL_TEXTENCODING_ASCII_US ));
        if ( !aReferenceSource.Exists()) {
            fprintf( stderr, "\nERROR: GSI-File %s not found!\n\n", aFilename.GetBuffer() );
            exit ( 2 );
        }

        aReferenceGSI.Open( String( aReferenceFilename, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
        if ( !aReferenceGSI.IsOpen()) {
            fprintf( stderr, "\nERROR: Could not open Input-File %s!\n\n", aFilename.GetBuffer() );
            exit ( 3 );
        }
    }

    LazySvFileStream aOkOut;
    String aBaseName = aSource.GetBase();
    if ( bWriteCorrect )
    {
        if ( !aCorrectFilename.Len() )
        {
            String sTmpBase( aBaseName );
            sTmpBase += String( "_ok", RTL_TEXTENCODING_ASCII_US );
            aSource.SetBase( sTmpBase );
            aCorrectFilename = aSource.GetFull();
        }
        aOkOut.SetOpenParams( aCorrectFilename , STREAM_STD_WRITE | STREAM_TRUNC );
    }

    LazySvFileStream aErrOut;
    if ( bWriteError )
    {
        if ( !aErrorFilename.Len() )
        {
            String sTmpBase( aBaseName );
            sTmpBase += String( "_err", RTL_TEXTENCODING_ASCII_US );
            aSource.SetBase( sTmpBase );
            aErrorFilename = aSource.GetFull();
        }
        aErrOut.SetOpenParams( aErrorFilename , STREAM_STD_WRITE | STREAM_TRUNC );
    }

    LazySvFileStream aFixOut;
    if ( bWriteFixed )
    {
        if ( !aFixedFilename.Len() )
        {
            String sTmpBase( aBaseName );
            sTmpBase += String( "_fix", RTL_TEXTENCODING_ASCII_US );
            aSource.SetBase( sTmpBase );
            aFixedFilename = aSource.GetFull();
        }
        aFixOut.SetOpenParams( aFixedFilename , STREAM_STD_WRITE | STREAM_TRUNC );
    }


    ByteString sReferenceLine;
    GSILine* pReferenceLine = NULL;
    ByteString aOldReferenceId("No Valid ID");   // just set to something which can never be an ID
    ULONG nReferenceLine = 0;

    ByteString sGSILine;
    GSILine* pGSILine = NULL;
    ByteString aOldId("No Valid ID");   // just set to something which can never be an ID
    GSIBlock *pBlock = NULL;
    ULONG nLine = 0;

    while ( !aGSI.IsEof() )
    {
        aGSI.ReadLine( sGSILine );
        nLine++;
        pGSILine = new GSILine( sGSILine, nLine );
        BOOL bDelete = TRUE;


        if ( pGSILine->Len() )
        {
            if ( FORMAT_UNKNOWN == pGSILine->GetLineFormat() )
            {
                PrintError( "Format of line is unknown. Ignoring!", "Line format", pGSILine->Copy( 0,40 ), bPrintContext, pGSILine->GetLineNumber() );
                pGSILine->NotOK();
                if ( bWriteError )
                {
                    bFileHasError = TRUE;
                    aErrOut.LazyOpen();
                    aErrOut.WriteLine( *pGSILine );
                }
            }
            else if ( pGSILine->GetLineType().EqualsIgnoreCaseAscii("res-comment") )
            {   // ignore comment lines, but write them to Correct Items File
                if ( bWriteCorrect )
                {
                    aOkOut.LazyOpen();
                       aOkOut.WriteLine( *pGSILine );
                }
            }
            else
            {
                ByteString aId = pGSILine->GetUniqId();
                if ( aId != aOldId )
                {
                    if ( pBlock )
                    {
                        bFileHasError |= pBlock->CheckSyntax( nLine, aSourceLang.Len() != 0, bFixTags );

                        if ( bWriteError )
                            pBlock->WriteError( aErrOut, aSourceLang.Len() != 0 );
                        if ( bWriteCorrect )
                            pBlock->WriteCorrect( aOkOut, aSourceLang.Len() != 0 );
                        if ( bWriteFixed )
                            pBlock->WriteFixed( aFixOut, aSourceLang.Len() != 0 );

                        delete pBlock;
                    }
                    pBlock = new GSIBlock( bPrintContext, bCheckSourceLang, bCheckTranslationLang, bReferenceFile, bAllowKID, bAllowSuspicious );

                    aOldId = aId;


                    // find corresponding line in reference file
                    if ( bReferenceFile )
                    {
                        BOOL bContinueSearching = TRUE;
                        while ( ( !aReferenceGSI.IsEof() || pReferenceLine ) && bContinueSearching )
                        {
                            if ( !pReferenceLine )
                            {
                                aReferenceGSI.ReadLine( sReferenceLine );
                                nReferenceLine++;
                                pReferenceLine = new GSILine( sReferenceLine, nReferenceLine );
                            }
                            if ( pReferenceLine->GetLineFormat() != FORMAT_UNKNOWN )
                            {
                                if ( pReferenceLine->GetUniqId() == aId && pReferenceLine->GetLanguageId().Equals( aSourceLang ) )
                                {
                                    pBlock->SetReferenceLine( pReferenceLine );
                                    pReferenceLine = NULL;
                                }
                                else if ( pReferenceLine->GetUniqId() > aId )
                                {
                                    bContinueSearching = FALSE;
                                }
                                else
                                {
                                    if ( pReferenceLine->GetUniqId() < aId  && pReferenceLine->GetLanguageId().Equals( aSourceLang ) )
                                        PrintError( "No Entry in source file found. Entry has been removed from source file", "File format", "", bPrintContext, pGSILine->GetLineNumber(), pReferenceLine->GetUniqId() );
                                    delete pReferenceLine;
                                    pReferenceLine = NULL;
                                }
                            }
                            else
                            {
                                delete pReferenceLine;
                                pReferenceLine = NULL;
                            }

                        }
                    }

                }

                pBlock->InsertLine( pGSILine, aSourceLang );
                bDelete = FALSE;
            }
        }
        if ( bDelete )
            delete pGSILine;

    }
    if ( pBlock )
    {
        bFileHasError |= pBlock->CheckSyntax( nLine, aSourceLang.Len() != 0, bFixTags );

        if ( bWriteError )
            pBlock->WriteError( aErrOut, aSourceLang.Len() != 0 );
        if ( bWriteCorrect )
            pBlock->WriteCorrect( aOkOut, aSourceLang.Len() != 0 );
        if ( bWriteFixed )
            pBlock->WriteFixed( aFixOut, aSourceLang.Len() != 0 );

        delete pBlock;
    }
    aGSI.Close();

    if ( bWriteError )
        aErrOut.Close();
    if ( bWriteCorrect )
        aOkOut.Close();
    if ( bWriteFixed )
        aFixOut.Close();

    if ( bFileHasError )
        return 55;
    else
        return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
