/*************************************************************************
 *
 *  $RCSfile: gsicheck.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:10:46 $
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
    USHORT nLangId;
    ByteString aStatus;
    ByteString aText;

    BOOL bOK;


public:
    GSILine( const ByteString &rLine, ULONG nLine );

    LineFormat  const GetLineFormat()     { return aFormat; }
    ULONG       const GetLineNumber()     { return nLineNumber; }

    ByteString  const GetUniqId()         { return aUniqId; }
    ByteString  const GetLineType()       { return aLineType; }
    USHORT      const GetLanguageId()     { return nLangId; }
    ByteString  const GetLineStatus()     { return aStatus; }
    ByteString  const GetText()           { return aText; }

    BOOL const IsOK() { return bOK; }
    void NotOK() { bOK = FALSE; }
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
    BOOL bInternal;
    BOOL bReference;

    BOOL TestUTF8( GSILine* pTestee );

public:
    GSIBlock( BOOL PbPrintContext, BOOL bInt, BOOL bRef ) : pSourceLine( NULL ), pReferenceLine( NULL ), bPrintContext( PbPrintContext ), bInternal( bInt ), bReference( bRef ) {};
    ~GSIBlock();
    void PrintError( ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ByteString aUniqueId = ByteString() );
    void InsertLine( GSILine* pLine, const USHORT nSourceLang);
    void SetReferenceLine( GSILine* pLine );
    BOOL CheckSyntax( ULONG nLine );

    void WriteError( SvStream &aErrOut );
    void WriteCorrect( SvStream &aOkOut );
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
       if ( rLine.GetTokenCount( '\t' ) == 1 )
    {
        ByteString sTmp( rLine );
        sTmp.SearchAndReplaceAll( "($$)", "\t" );
        if ( sTmp.GetTokenCount( '\t' ) == 5 )
        {
            aFormat = FORMAT_GSI1;
            aUniqId = sTmp.GetToken( 0, '\t' );
            aLineType = sTmp.GetToken( 1, '\t' );
            nLangId = sTmp.GetToken( 2, '\t' ).ToInt32();
            aStatus = sTmp.GetToken( 3, '\t' );     // ext int ...
            aText = sTmp.GetToken( 4, '\t' );
        }
        else
            aFormat = FORMAT_UNKNOWN;
    }
    else if ( rLine.GetTokenCount( '\t' ) == 15 )
    {
        aFormat = FORMAT_SDF;
        aUniqId = rLine.GetToken( 4, '\t' ).Append("/").Append( rLine.GetToken( 3, '\t' ) ).Append("/").Append( rLine.GetToken( 5, '\t' ) );
        aLineType = "";
        nLangId = rLine.GetToken( 9, '\t' ).ToInt32();
        aStatus = "";
        aText = rLine.GetToken( 10, '\t' );
    }
    else
        aFormat = FORMAT_UNKNOWN;

    if ( FORMAT_UNKNOWN == GetLineFormat() )
        NotOK();
}

//
// class GSIBlock
//
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
void GSIBlock::InsertLine( GSILine* pLine, const USHORT nSourceLang)
/*****************************************************************************/
{
    if ( pLine->GetLanguageId() == nSourceLang )
    {
        if ( pSourceLine && bInternal )
            PrintError( "Source Language entry double. Ignoring.", "File format", "", pLine->GetLineNumber(), pLine->GetUniqId() );
        pSourceLine = pLine;
    }
    else {
        ULONG nPos = 0;

        BOOL bLineIsInternal = pLine->GetLineStatus().EqualsIgnoreCaseAscii( "int" );
        if ( (  bLineIsInternal &&  bInternal )
           ||( !bLineIsInternal && !bInternal )
           ||( pLine->GetLineFormat() == FORMAT_SDF ))  // in sdf files check it all
        {
            while ( nPos < Count() )
            {
                if ( GetObject( nPos )->GetLanguageId() == pLine->GetLanguageId() )
                    PrintError( "Translation Language entry double. Checking both.", "File format", "", pLine->GetLineNumber(), pLine->GetUniqId() );
                nPos++;
            }

            Insert( pLine, LIST_APPEND );
        }
    }
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
BOOL GSIBlock::TestUTF8( GSILine* pTestee )
/*****************************************************************************/
{
    String aUTF8Tester( pTestee->GetText(), RTL_TEXTENCODING_UTF8 );
    if ( !ByteString( aUTF8Tester, RTL_TEXTENCODING_UTF8 ).Equals( pTestee->GetText() ) )
    {
        PrintError( "UTF8 Encoding seems to be broken", "File format", "", pTestee->GetLineNumber(), pTestee->GetUniqId() );
        pTestee->NotOK();
        return FALSE;
    }
    return TRUE;
}

/*****************************************************************************/
BOOL GSIBlock::CheckSyntax( ULONG nLine )
/*****************************************************************************/
{
    static LingTest aTester;

    if ( !pSourceLine )
    {
        PrintError( "No source language entry defined!", "File format", "", nLine );
        aTester.ReferenceOK( "" );
    }
    else
    {
        if ( !aTester.ReferenceOK( *pSourceLine ) )
        {
            if ( bInternal )
                PrintList( aTester.GetReferenceErrors(), "ReferenceString", pSourceLine );
            pSourceLine->NotOK();
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
        }
        else
        {
            if ( pSourceLine && !pSourceLine->Equals( *pReferenceLine ) )
            {
                xub_StrLen nPos = pSourceLine->Match( *pReferenceLine );
                PrintError( "Source Language has changed.", "File format", pReferenceLine->Copy( nPos - 5, 15).Append( "\" --> \"" ). Append( pSourceLine->Copy( nPos - 5, 15) ), pSourceLine->GetLineNumber(), pSourceLine->GetUniqId() );
                pSourceLine->NotOK();
            }
        }
    }

    if ( bInternal && pSourceLine )
        TestUTF8( pSourceLine );

    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( !aTester.TesteeOK( *GetObject( i ) ) )
        {
            GetObject( i )->NotOK();
            if ( aTester.HasTesteeErrors() )
                PrintList( aTester.GetTesteeErrors(), "Translation", GetObject( i ) );
            if ( pSourceLine && aTester.HasCompareWarnings() )
                PrintList( aTester.GetCompareWarnings(), "Translation Tag Missmatch", GetObject( i ) );
        }
        TestUTF8( GetObject( i ) );
    }

    return TRUE;
}

void GSIBlock::WriteError( SvStream &aErrOut )
{
    BOOL bHasError = FALSE;
    BOOL bCopyAll = ( !pSourceLine || !pSourceLine->IsOK() ) && bInternal;
    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( !GetObject( i )->IsOK() || bCopyAll )
        {
            bHasError = TRUE;
            aErrOut.WriteLine( *GetObject( i ) );
        }
    }

    if ( pSourceLine && ( bHasError || ( !pSourceLine->IsOK() && bInternal ) ) )
        aErrOut.WriteLine( *pSourceLine );
}

void GSIBlock::WriteCorrect( SvStream &aOkOut )
{
    if ( !pSourceLine )
        return;

    BOOL bHasOK = FALSE;
    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( GetObject( i )->IsOK() )
        {
            bHasOK = TRUE;
            aOkOut.WriteLine( *GetObject( i ) );
        }
    }

    if ( pSourceLine && bHasOK )
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
    fprintf( stdout, "gsicheck Version 1.6.0 (c)1999 - 2001 by SUN Microsystems\n" );
    fprintf( stdout, "=========================================================\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck checks the syntax of tags in GSI-Files and SDF-Files\n" );
    fprintf( stdout, "         checks for inconsistencies and malicious UTF8 encoding\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsicheck [ -c ] [ -we ] [ -wc ] [ -i ] [ -l LanguageID ]\n" );
    fprintf( stdout, "                 [ -r ReferenceFile ] filename\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "-c    Add context to error message (Print the line containing the error)\n" );
    fprintf( stdout, "-we   Write GSI-File containing all errors\n" );
    fprintf( stdout, "-wc   Write GSI-File containing all correct parts\n" );
    fprintf( stdout, "-i    Check records marked 'int' rather than marked 'ext' or similar\n" );
    fprintf( stdout, "-l    Numerical 2 digits Identifier of the source language. Default = 49\n" );
    fprintf( stdout, "-r    Reference filename to check that source language has not been changed\n" );
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
    BOOL bInternal = FALSE;
    BOOL bWriteError = FALSE;
    BOOL bWriteCorrect = FALSE;
    USHORT nSourceLang = 49;     // German is default
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
                            bWriteError = TRUE;
                        else if ( (*(argv[ i ]+2)) == 'c' )
                            bWriteCorrect = TRUE;
                        else
                        {
                            fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                            bError = TRUE;
                        }
                    }
                    break;
                case 'i':bInternal = TRUE;
                    break;
                case 'l':
                    {
                        if ( (i+1) < argc )
                        {
                            nSourceLang = ByteString( argv[ i+1 ] ).ToInt32();
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
        String sTmpBase( aBaseName );
        sTmpBase += String( "_ok", RTL_TEXTENCODING_ASCII_US );
        aSource.SetBase( sTmpBase );
        aOkOut.Open( aSource.GetFull() , STREAM_STD_WRITE | STREAM_TRUNC );
        if ( !aOkOut.IsOpen())
        {
            fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", ByteString( aSource.GetFull(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
            exit ( 4 );
        }
    }

    SvFileStream aErrOut;
    if ( bWriteError )
    {
        String sTmpBase( aBaseName );
        sTmpBase += String( "_err", RTL_TEXTENCODING_ASCII_US );
        aSource.SetBase( sTmpBase );
        aErrOut.Open( aSource.GetFull() , STREAM_STD_WRITE | STREAM_TRUNC );
        if ( !aErrOut.IsOpen())
        {
            fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", ByteString( aSource.GetFull(), RTL_TEXTENCODING_ASCII_US ).GetBuffer() );
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
                        pBlock->CheckSyntax( nLine );

                        if ( bWriteError )
                            pBlock->WriteError( aErrOut );
                        if ( bWriteCorrect )
                            pBlock->WriteCorrect( aOkOut );

                        delete pBlock;
                    }
                    pBlock = new GSIBlock( bPrintContext, bInternal, bReferenceFile );

                    aOldId = aId;


                    // find corrosponding line in reference file
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
                                if ( pReferenceLine->GetUniqId() == aId && pReferenceLine->GetLanguageId() == nSourceLang )
                                {
                                    pBlock->SetReferenceLine( pReferenceLine );
                                    pReferenceLine = NULL;
                                }
                                else if ( pReferenceLine->GetUniqId() > aId )
                                {
//                                    if ( pGSILine->GetLanguageId() == nSourceLang )
//                                      PrintError( "No reference line found. Entry is new in source file", "File format", "", bPrintContext, pGSILine->GetLineNumber(), aId );
                                    bContinueSearching = FALSE;
                                }
                                else
                                {
                                    if ( pReferenceLine->GetUniqId() < aId  && pReferenceLine->GetLanguageId() == nSourceLang )
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

                pBlock->InsertLine( pGSILine, nSourceLang );
                bDelete = FALSE;
            }
        }
        if ( bDelete )
            delete pGSILine;

    }
    if ( pBlock )
    {
        pBlock->CheckSyntax( nLine );

        if ( bWriteError )
            pBlock->WriteError( aErrOut );
        if ( bWriteCorrect )
            pBlock->WriteCorrect( aOkOut );

        delete pBlock;
    }
    aGSI.Close();
    return 0;
}
