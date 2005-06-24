/*************************************************************************
 *
 *  $RCSfile: gsicheck.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-06-24 11:39:20 $
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
#include <stdio.h>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <tools/list.hxx>

// local includes
#include "tagtest.hxx"

//
// class GSILine
//
enum LineFormat { FORMAT_GSI1, FORMAT_SDF, FORMAT_UNKNOWN };

class GSILine : public ByteString
{
private:
    LineFormat aFormat;
    ULONG nLineNumber;

    ByteString aUniqId;
    ByteString aLineType;
    ByteString aLangId;
    ByteString aText;
    ByteString aQuickHelpText;
    ByteString aTitle;

    BOOL bOK;


public:
    GSILine( const ByteString &rLine, ULONG nLine );
    LineFormat  const GetLineFormat()     { return aFormat; }
    ULONG       const GetLineNumber()     { return nLineNumber; }

    ByteString  const GetUniqId()         { return aUniqId; }
    ByteString  const GetLineType()       { return aLineType; }
    ByteString  const GetLanguageId()     { return aLangId; }
    ByteString  const GetText()           { return aText; }
        String  const GetUText()          { return String( aText, RTL_TEXTENCODING_UTF8 ); }
    ByteString  const GetQuickHelpText()  { return aQuickHelpText; }
    ByteString  const GetTitle()          { return aTitle; }

    BOOL const IsOK() { return bOK; }
    void NotOK();
};

//
// class GSIBlock
//

DECLARE_LIST( GSIBlock_Impl, GSILine * );

class GSIBlock : public GSIBlock_Impl
{
private:
    GSILine *pSourceLine;
    GSILine *pReferenceLine;
    void PrintList( ParserMessageList &rList, ByteString aPrefix, GSILine *pLine );
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
    void PrintError( ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ByteString aUniqueId = ByteString() );
    void InsertLine( GSILine* pLine, const ByteString aSourceLang);
    void SetReferenceLine( GSILine* pLine );
    BOOL CheckSyntax( ULONG nLine, BOOL bRequireSourceLine );

    void WriteError( SvStream &aErrOut, BOOL bRequireSourceLine );
    void WriteCorrect( SvStream &aOkOut, BOOL bRequireSourceLine );
};



/*****************************************************************************/
void PrintError( ByteString aMsg, ByteString aPrefix,
    ByteString aContext, BOOL bPrintContext, ULONG nLine, ByteString aUniqueId = ByteString() )
/*****************************************************************************/
{
    fprintf( stdout, "Error: %s, Line %lu", aPrefix.GetBuffer(), nLine );
    if ( aUniqueId.Len() )
        fprintf( stdout, ", UniqueID %s", aUniqueId.GetBuffer() );
    fprintf( stdout, ": %s", aMsg.GetBuffer() );

    if ( bPrintContext )
        fprintf( stdout, "  \"%s\"", aContext.GetBuffer() );
    fprintf( stdout, "\n" );
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
// class GSILine
//

/*****************************************************************************/
GSILine::GSILine( const ByteString &rLine, ULONG nLine )
/*****************************************************************************/
                : ByteString( rLine )
                , nLineNumber( nLine )
                , bOK( TRUE )
{
    if ( rLine.GetTokenCount( '\t' ) == 15 )
    {
        aFormat = FORMAT_SDF;
        aUniqId = rLine.GetToken( 0, '\t' ).Append("/").Append( rLine.GetToken( 1, '\t' ) ).Append("/").Append( rLine.GetToken( 3, '\t' ) ).Append("/").Append( rLine.GetToken( 4, '\t' ) ).Append("/").Append( rLine.GetToken( 5, '\t' ) ).Append("/").Append( rLine.GetToken( 6, '\t' ) ).Append("/").Append( rLine.GetToken( 7, '\t' ) );
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
    }
    else    // allow tabs in gsi files
    {
        aFormat = FORMAT_GSI1;
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
//          ByteString aStatus = sTmp.Copy( nStart, nPos - nStart );     // ext int ...
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

//
// class GSIBlock
//
/*****************************************************************************/
GSIBlock::GSIBlock( BOOL PbPrintContext, BOOL bSource, BOOL bTrans, BOOL bRef )
/*****************************************************************************/
            : pSourceLine( NULL )
            , pReferenceLine( NULL )
            , bPrintContext( PbPrintContext )
            , bCheckSourceLang( bSource )
            , bCheckTranslationLang( bTrans )
            , bReference( bRef )
            , bHasBlockError( FALSE )
{
}

/*****************************************************************************/
GSIBlock::~GSIBlock()
/*****************************************************************************/
{
    delete pSourceLine;
    delete pReferenceLine;

    for ( ULONG i = 0; i < Count(); i++ )
        delete ( GetObject( i ));
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
    ULONG nPos = 0;

    if ( aSourceLang.Len() ) // only check blockstructure if source lang is given
    {
        while ( nPos < Count() )
        {
            if ( GetObject( nPos )->GetLanguageId().Equals( pLine->GetLanguageId() ) )
            {
                PrintError( "Translation Language entry double. Checking both.", "File format", "", pLine->GetLineNumber(), pLine->GetUniqId() );
                bHasBlockError = TRUE;
                GetObject( nPos )->NotOK();
                pLine->NotOK();
            }
            nPos++;
        }
    }
    Insert( pLine, LIST_APPEND );
}

/*****************************************************************************/
void GSIBlock::SetReferenceLine( GSILine* pLine )
/*****************************************************************************/
{
    pReferenceLine = pLine;
}

/*****************************************************************************/
void GSIBlock::PrintError( ByteString aMsg, ByteString aPrefix,
    ByteString aContext, ULONG nLine, ByteString aUniqueId )
/*****************************************************************************/
{
    ::PrintError( aMsg, aPrefix, aContext, bPrintContext, nLine, aUniqueId );
}

/*****************************************************************************/
void GSIBlock::PrintList( ParserMessageList &rList, ByteString aPrefix,
    GSILine *pLine )
/*****************************************************************************/
{
    ULONG i;
    for ( i = 0 ; i < rList.Count() ; i++ )
    {
        ParserMessage *pMsg = rList.GetObject( i );
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

        PrintError( pMsg->GetErrorText(), aPrefix, aContext, pLine->GetLineNumber(), pLine->GetUniqId() );
    }
}

/*****************************************************************************/
BOOL GSIBlock::IsUTF8( const ByteString &aTestee, USHORT &nErrorPos, ByteString &aErrorMsg ) const
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

    return TRUE;
}

/*****************************************************************************/
BOOL GSIBlock::TestUTF8( GSILine* pTestee )
/*****************************************************************************/
{
    USHORT nErrorPos = 0;
    ByteString aErrorMsg;
    BOOL bError = FALSE;
    if ( !IsUTF8( pTestee->GetText(), nErrorPos, aErrorMsg ) )
    {
        ByteString aContext( pTestee->GetText().Copy( nErrorPos, 20 ) );
        PrintError( aErrorMsg.Append(" in Text at Position " ).Append( ByteString::CreateFromInt32( nErrorPos ) ), "File format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId() );
        bError = TRUE;
    }
    if ( !IsUTF8( pTestee->GetQuickHelpText(), nErrorPos, aErrorMsg ) )
    {
        ByteString aContext( pTestee->GetQuickHelpText().Copy( nErrorPos, 20 ) );
        PrintError( aErrorMsg.Append(" in QuickHelpText at Position " ).Append( ByteString::CreateFromInt32( nErrorPos ) ), "File format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId() );
        bError = TRUE;
    }
    if ( !IsUTF8( pTestee->GetTitle(), nErrorPos, aErrorMsg ) )
    {
        ByteString aContext( pTestee->GetTitle().Copy( nErrorPos, 20 ) );
        PrintError( aErrorMsg.Append(" in Title at Position " ).Append( ByteString::CreateFromInt32( nErrorPos ) ), "File format", aContext, pTestee->GetLineNumber(), pTestee->GetUniqId() );
        bError = TRUE;
    }
    if ( bError )
        pTestee->NotOK();
    return !bError;
}

/*****************************************************************************/
BOOL GSIBlock::CheckSyntax( ULONG nLine, BOOL bRequireSourceLine )
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
//      aTester.ReferenceOK( String() );
    }
    else
    {
        if ( !aTester.ReferenceOK( pSourceLine->GetUText() ) )
        {
            PrintList( aTester.GetReferenceErrors(), "ReferenceString", pSourceLine );
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
                pSource = GetObject( 0 );   // get some other line
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
                PrintError( "Source Language Entry has changed.", "File format", pReferenceLine->Copy( nPos - 5, 15).Append( "\" --> \"" ). Append( pSourceLine->Copy( nPos - 5, 15) ), pSourceLine->GetLineNumber(), pSourceLine->GetUniqId() );
                pSourceLine->NotOK();
                bHasError = TRUE;
            }
        }
    }

    if ( pSourceLine )
        bHasError |= !TestUTF8( pSourceLine );

    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( !aTester.TesteeOK( GetObject( i )->GetUText(), pSourceLine != NULL ) )
        {
            GetObject( i )->NotOK();
            bHasError = TRUE;
            if ( aTester.HasTesteeErrors() )
                PrintList( aTester.GetTesteeErrors(), "Translation", GetObject( i ) );
            if ( pSourceLine && aTester.HasCompareWarnings() )
                PrintList( aTester.GetCompareWarnings(), "Translation Tag Missmatch", GetObject( i ) );
        }
        bHasError |= !TestUTF8( GetObject( i ) );
    }

    return bHasError || bHasBlockError;
}

void GSIBlock::WriteError( SvStream &aErrOut, BOOL bRequireSourceLine  )
{
    if ( pSourceLine && pSourceLine->IsOK() && bCheckSourceLang && !bHasBlockError )
        return;

    BOOL bHasError = FALSE;
    BOOL bCopyAll = ( !pSourceLine && bRequireSourceLine ) || ( pSourceLine && !pSourceLine->IsOK() && !bCheckTranslationLang ) || bHasBlockError;
    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( !GetObject( i )->IsOK() || bCopyAll )
        {
            bHasError = TRUE;
            aErrOut.WriteLine( *GetObject( i ) );
        }
    }

    if ( pSourceLine && ( bHasError || !pSourceLine->IsOK() ) && !( !bHasError && bCheckTranslationLang ) )
        aErrOut.WriteLine( *pSourceLine );
}

void GSIBlock::WriteCorrect( SvStream &aOkOut, BOOL bRequireSourceLine )
{
    if ( ( !pSourceLine && bRequireSourceLine ) || ( pSourceLine && !pSourceLine->IsOK() && !bCheckTranslationLang ) )
        return;

    BOOL bHasOK = FALSE;
    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( ( GetObject( i )->IsOK() || bCheckSourceLang ) && !bHasBlockError )
        {
            bHasOK = TRUE;
            aOkOut.WriteLine( *GetObject( i ) );
        }
    }

    if ( ( pSourceLine && pSourceLine->IsOK() && ( Count() || !bCheckTranslationLang ) ) || ( bHasOK && bCheckTranslationLang ) )
        aOkOut.WriteLine( *pSourceLine );
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
    fprintf( stdout, "gsicheck Version 1.7.8 (c)1999 - 2001 by SUN Microsystems\n" );
    fprintf( stdout, "=========================================================\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck checks the syntax of tags in GSI-Files and SDF-Files\n" );
    fprintf( stdout, "         checks for inconsistencies and malicious UTF8 encoding\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsicheck [ -c ] [ -we ] [ -wef ErrorFilename ] [ -wc ]\n" );
    fprintf( stdout, "                 [ -wcf CorrectFilename ] [ -s | -t ] [ -l LanguageID ]\n" );
    fprintf( stdout, "                 [ -r ReferenceFile ] filename\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "-c    Add context to error message (Print the line containing the error)\n" );
    fprintf( stdout, "-we   Write GSI-File containing all errors\n" );
    fprintf( stdout, "-wef  Same as above but give own filename\n" );
    fprintf( stdout, "-wc   Write GSI-File containing all correct parts\n" );
    fprintf( stdout, "-wcf  Same as above but give own filename\n" );
    fprintf( stdout, "-s    Check only source language. Should be used before handing out to vendor.\n" );
    fprintf( stdout, "-t    Check only Translation language(s). Should be used before merging.\n" );
    fprintf( stdout, "-l    ISO Languagecode or numerical 2 digits Identifier of the source language.\n" );
    fprintf( stdout, "      Default is en-US. Use \"\" (empty string) or 'none'\n" );
    fprintf( stdout, "      to disable source language dependent checks\n" );
    fprintf( stdout, "-r    Reference filename to check that source language entries\n" );
    fprintf( stdout, "      have not been changed\n" );
       fprintf( stdout, "\n" );
}

/*****************************************************************************/
#if defined( UNX ) || defined( MAC )
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
    String aErrorFilename;
    String aCorrectFilename;
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
                fprintf( stderr, "\nERROR: Only one filename may be specified!\n\n", argv[ i ]);
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
            fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", aFilename.GetBuffer() );
            exit ( 3 );
        }
    }

    SvFileStream aOkOut;
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
        aOkOut.Open( aCorrectFilename , STREAM_STD_WRITE | STREAM_TRUNC );
        if ( !aOkOut.IsOpen())
        {
            fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", ByteString( aCorrectFilename, RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            exit ( 4 );
        }
    }

    SvFileStream aErrOut;
    if ( bWriteError )
    {
        if ( !aErrorFilename.Len() )
        {
            String sTmpBase( aBaseName );
            sTmpBase += String( "_err", RTL_TEXTENCODING_ASCII_US );
            aSource.SetBase( sTmpBase );
            aErrorFilename = aSource.GetFull();
        }
        aErrOut.Open( aErrorFilename , STREAM_STD_WRITE | STREAM_TRUNC );
        if ( !aErrOut.IsOpen())
        {
            fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", ByteString( aErrorFilename, RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            exit ( 4 );
        }
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
                    aErrOut.WriteLine( *pGSILine );
                }
            }
            else if ( pGSILine->GetLineType().EqualsIgnoreCaseAscii("res-comment") )
            {   // ignore comment lines, but write them to Correct Items File
                if ( bWriteCorrect )
                       aOkOut.WriteLine( *pGSILine );
            }
            else
            {
                ByteString aId = pGSILine->GetUniqId();
                if ( aId != aOldId )
                {
                    if ( pBlock )
                    {
                        bFileHasError |= pBlock->CheckSyntax( nLine, aSourceLang.Len() != 0 );

                        if ( bWriteError )
                            pBlock->WriteError( aErrOut, aSourceLang.Len() != 0 );
                        if ( bWriteCorrect )
                            pBlock->WriteCorrect( aOkOut, aSourceLang.Len() != 0 );

                        delete pBlock;
                    }
                    pBlock = new GSIBlock( bPrintContext, bCheckSourceLang, bCheckTranslationLang, bReferenceFile );

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
//                                    if ( pGSILine->GetLanguageId() == aSourceLang )
//                                      PrintError( "No reference line found. Entry is new in source file", "File format", "", bPrintContext, pGSILine->GetLineNumber(), aId );
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
        bFileHasError |= pBlock->CheckSyntax( nLine, aSourceLang.Len() != 0 );

        if ( bWriteError )
            pBlock->WriteError( aErrOut, aSourceLang.Len() != 0 );
        if ( bWriteCorrect )
            pBlock->WriteCorrect( aOkOut, aSourceLang.Len() != 0 );

        delete pBlock;
    }
    aGSI.Close();

    if ( bWriteError )
        aErrOut.Close();
    if ( bWriteCorrect )
        aOkOut.Close();

    if ( bFileHasError )
        return 55;
    else
        return 0;
}
