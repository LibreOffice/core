/*************************************************************************
 *
 *  $RCSfile: gsicheck.cxx,v $
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
#include <stdio.h>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <tools/list.hxx>

// local includes
#include "utf8conv.hxx"
#include "tagtest.hxx"

//
// class GSILine
//

class GSILine : public ByteString
{
private:
    ULONG nUniqId;
    ULONG nLineNumber;
    BOOL bOK;

public:
    GSILine( const ByteString &rLine, ULONG nLine );

    ULONG GetUniqId() { return nUniqId; }
    ULONG GetLineNumber() { return nLineNumber; }

    BOOL IsOK() { return bOK; }
    void NotOK() { bOK = FALSE; }
};

//
// class GSIBlock
//

DECLARE_LIST( GSIBlock_Impl, GSILine * );

class GSIBlock : public GSIBlock_Impl
{
private:
    GSILine *pGermanLine;
    void PrintList( ParserMessageList &rList, ByteString aPrefix, GSILine *pLine );
    BOOL bPrintContext;

public:
    GSIBlock( BOOL PbPrintContext ) : pGermanLine( NULL ), bPrintContext( PbPrintContext ) {};
    ~GSIBlock();
    void PrintError( ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ULONG nUniqueId );
    void InsertLine( const ByteString &rLine, ULONG nLine );
    BOOL CheckSyntax( ULONG nLine );

    void WriteError( SvStream &aErrOut );
    void WriteCorrect( SvStream &aOkOut );
};

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
    ByteString sTmp( rLine );
    sTmp.SearchAndReplaceAll( "($$)", "\t" );

    nUniqId = sTmp.GetToken( 0, '\t' ).ToInt32();
}

//
// class GSIBlock
//

/*****************************************************************************/
GSIBlock::~GSIBlock()
/*****************************************************************************/
{
    delete pGermanLine;

    for ( ULONG i = 0; i < Count(); i++ )
        delete ( GetObject( i ));
}

/*****************************************************************************/
void GSIBlock::InsertLine( const ByteString &rLine, ULONG nLine )
/*****************************************************************************/
{
    GSILine *pLine = new GSILine( rLine, nLine );

    ByteString sTmp( rLine );
    sTmp.SearchAndReplaceAll( "($$)", "\t" );

    if ( sTmp.GetTokenCount( '\t' ) < 5 )
    {
        PrintError( "Unable to determin languge and/or state", "Line format", rLine.Copy( 0,100 ), nLine, 0 );
        pLine->NotOK();
    }

    USHORT nLangId = sTmp.GetToken( 2, '\t' ).ToInt32();
    if ( nLangId == 49 )
        pGermanLine = pLine;
    else {
        ULONG nPos = 0;

        if ( sTmp.GetToken( 3, '\t' ).ToLowerAscii() ==  "int")
        {
            while (( nPos < Count()) &&
                    ( GetObject( nPos )->GetLineNumber() < pLine->GetLineNumber()))
                nPos++;

            Insert( pLine, nPos );
        }
    }
}

/*****************************************************************************/
void GSIBlock::PrintError( ByteString aMsg, ByteString aPrefix,
    ByteString aContext, ULONG nLine, ULONG nUniqueId )
/*****************************************************************************/
{
    fprintf( stdout, "Error: %s, Line %lu", aPrefix.GetBuffer(),
        nLine );
    if ( nUniqueId )
        fprintf( stdout, ", UniqueID %lu", nUniqueId );
    fprintf( stdout, ": %s", aMsg.GetBuffer() );

    if ( bPrintContext )
        fprintf( stdout, "  \"%s\"", aContext.GetBuffer() );
    fprintf( stdout, "\n" );
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
                aContext = pLine->Copy( 0, 300 );
            else
                aContext = pLine->Copy( pMsg->GetTagBegin()-150, 300 );
            aContext.EraseTrailingChars(' ');
            aContext.EraseLeadingChars(' ');
        }

        PrintError( pMsg->GetErrorText(), aPrefix, aContext, pLine->GetLineNumber(), pLine->GetUniqId() );
    }
}

/*****************************************************************************/
BOOL GSIBlock::CheckSyntax( ULONG nLine )
/*****************************************************************************/
{
    static LingTest aTester;

    if ( !pGermanLine )
    {
        PrintError( "No German reference defined!", "File format", "", nLine, 0 );
        aTester.ReferenceOK( "" );
    }
    else
    {
        if ( !aTester.ReferenceOK( *pGermanLine ) )
        {
            PrintList( aTester.GetReferenceErrors(), "ReferenceString", pGermanLine );
            pGermanLine->NotOK();
        }
    }

    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( !aTester.TesteeOK( *GetObject( i ) ) )
        {
            GetObject( i )->NotOK();
            if ( aTester.HasTesteeErrors() )
                PrintList( aTester.GetTesteeErrors(), "Translation", GetObject( i ) );
            if ( aTester.HasCompareWarnings() )
                PrintList( aTester.GetCompareWarnings(), "Translation Tag Missmatch", GetObject( i ) );
        }
    }

    return TRUE;
}

void GSIBlock::WriteError( SvStream &aErrOut )
{
    BOOL bHasError = FALSE;
    BOOL bCopyAll = !pGermanLine || !pGermanLine->IsOK();
    ULONG i;
    for ( i = 0; i < Count(); i++ )
    {
        if ( !GetObject( i )->IsOK() || bCopyAll )
        {
            bHasError = TRUE;
            aErrOut.WriteLine( *GetObject( i ) );
        }
    }

    if ( pGermanLine && ( bHasError || !pGermanLine->IsOK() ) )
        aErrOut.WriteLine( *pGermanLine );
}

void GSIBlock::WriteCorrect( SvStream &aOkOut )
{
    if ( !pGermanLine )
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

    if ( pGermanLine && bHasOK )
        aOkOut.WriteLine( *pGermanLine );
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
    fprintf( stdout, "gsicheck (c)1999 by StarOffice Entwicklungs GmbH\n" );
    fprintf( stdout, "================================================\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck checks the syntax of tags in GSI-Files (Gutschmitt-Interface)\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsicheck [ -c ] [ -we ] [ -wc ] filename\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "-c    Add context to error message\n" );
    fprintf( stdout, "-we   Write GSI-File containing all errors\n" );
    fprintf( stdout, "-wc   Write GSI-File containing all correct parts\n" );
       fprintf( stdout, "\n" );
}

/*****************************************************************************/
ULONG GetUniqId( const ByteString &rLine )
/*****************************************************************************/
{
    ByteString sTmp( rLine );
    sTmp.SearchAndReplaceAll( "($$)", "\t" );

    return sTmp.GetToken( 0, '\t' ).ToInt64();
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
    BOOL bWriteError = FALSE;
    BOOL bWriteCorrect = FALSE;
    ByteString aFilename;
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
                default:
                    fprintf( stderr, "\nERROR: Unknown Switch %s!\n\n", argv[ i ] );
                    bError = TRUE;
            }
        }
        else
        {
            if ( !aFilename.Len() )
                aFilename = ByteString( argv[ i ] );
            else
            {
                fprintf( stderr, "\nERROR: Only one filename may be specified!\n\n", argv[ i ] );
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


    ByteString sGSILine;

    ULONG nOldId = 0;
    GSIBlock *pBlock = NULL;
    ULONG nLine = 0;

    while ( !aGSI.IsEof()) {

        aGSI.ReadLine( sGSILine );
        nLine++;

        if ( sGSILine.Len()) {
            ULONG nId = GetUniqId( sGSILine );
            if ( nId != nOldId ) {
                if ( pBlock )
                {
                    pBlock->CheckSyntax( nLine );

                    if ( bWriteError )
                        pBlock->WriteError( aErrOut );
                    if ( bWriteCorrect )
                        pBlock->WriteCorrect( aOkOut );

                    delete pBlock;
                }
                pBlock = new GSIBlock( bPrintContext );

                nOldId = nId;
            }

            pBlock->InsertLine( sGSILine, nLine );
        }
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
