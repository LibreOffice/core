/*************************************************************************
 *
 *  $RCSfile: gsicheck.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2002-09-11 10:02:15 $
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

class GSILine : public ByteString
{
private:
    ByteString aUniqId;
    ULONG nLineNumber;
    BOOL bOK;

public:
    GSILine( const ByteString &rLine, ULONG nLine );

    ByteString GetUniqId() { return aUniqId; }
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
    GSILine *pSourceLine;
    void PrintList( ParserMessageList &rList, ByteString aPrefix, GSILine *pLine );
    BOOL bPrintContext;
    BOOL bInternal;

public:
    GSIBlock( BOOL PbPrintContext, BOOL bInt ) : pSourceLine( NULL ), bPrintContext( PbPrintContext ), bInternal( bInt ) {};
    ~GSIBlock();
    void PrintError( ByteString aMsg, ByteString aPrefix, ByteString aContext, ULONG nLine, ByteString aUniqueId = ByteString() );
    void InsertLine( const ByteString &rLine, ULONG nLine , const int sourceLang);
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

    aUniqId = sTmp.GetToken( 0, '\t' );
}

//
// class GSIBlock
//
/*****************************************************************************/
GSIBlock::~GSIBlock()
/*****************************************************************************/
{
    delete pSourceLine;

    for ( ULONG i = 0; i < Count(); i++ )
        delete ( GetObject( i ));
}

/*****************************************************************************/
void GSIBlock::InsertLine( const ByteString &rLine, ULONG nLine , const int sourceLang)
/*****************************************************************************/
{
    GSILine *pLine = new GSILine( rLine, nLine );

    ByteString sTmp( rLine );
    sTmp.SearchAndReplaceAll( "($$)", "\t" );

    if ( sTmp.GetTokenCount( '\t' ) < 5 )
    {
        PrintError( "Unable to determin language and/or state", "Line format", rLine.Copy( 0,100 ), nLine );
        pLine->NotOK();
    }

    USHORT nLangId = sTmp.GetToken( 2, '\t' ).ToInt32();
    if ( nLangId == sourceLang )
        pSourceLine = pLine;
    else {
        ULONG nPos = 0;

        BOOL bLineIsInternal = sTmp.GetToken( 3, '\t' ).EqualsIgnoreCaseAscii( "int" );
        if ( (  bLineIsInternal &&  bInternal )
           ||( !bLineIsInternal && !bInternal ) )
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
    ByteString aContext, ULONG nLine, ByteString aUniqueId )
/*****************************************************************************/
{
    fprintf( stdout, "Error: %s, Line %lu", aPrefix.GetBuffer(),
        nLine );
    if ( aUniqueId.Len() )
        fprintf( stdout, ", UniqueID %s", aUniqueId.GetBuffer() );
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

    if ( !pSourceLine )
    {
        PrintError( "No source languages reference defined!", "File format", "", nLine );
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
    fprintf( stdout, "gsicheck Version 1.5.1 (c)1999 - 2001 by SUN Microsystems\n" );
    fprintf( stdout, "================================================\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsicheck checks the syntax of tags in GSI-Files (Gutschmitt-Interface)\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsicheck [ -c ] [ -we ] [ -wc ] [ -i ] [ -l ISO-code ] filename\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "-c    Add context to error message (Print the line containing the error)\n" );
    fprintf( stdout, "-we   Write GSI-File containing all errors\n" );
    fprintf( stdout, "-wc   Write GSI-File containing all correct parts\n" );
    fprintf( stdout, "-i    Check records marked 'int' rather than marked 'ext' or similar\n" );
    fprintf( stdout, "-l    Numerical 2 digits ISO-code of the source language. Default = 49 \n" );
       fprintf( stdout, "\n" );
}

/*****************************************************************************/
ByteString GetLineType( const ByteString &rLine )
/*****************************************************************************/
{
    ByteString sTmp( rLine );
    sTmp.SearchAndReplaceAll( "($$)", "\t" );

    return sTmp.GetToken( 1, '\t' );
}

/*****************************************************************************/
ByteString GetUniqId( const ByteString &rLine )
/*****************************************************************************/
{
    ByteString sTmp( rLine );
    sTmp.SearchAndReplaceAll( "($$)", "\t" );

    return sTmp.GetToken( 0, '\t' );
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
    int  sourceLang=49;
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
                case 'i':bInternal = TRUE;
                    break;
                case 'l': sourceLang=( *(argv[ i ]+3)-48 )*10 + ( *(argv[ i ]+4)-48 );i++;break;
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

    ByteString aOldId;
    GSIBlock *pBlock = NULL;
    ULONG nLine = 0;

    while ( !aGSI.IsEof())
    {
        aGSI.ReadLine( sGSILine );
        nLine++;

        if ( sGSILine.Len())
        {
            if ( GetLineType( sGSILine ).CompareIgnoreCaseToAscii("res-comment") == COMPARE_EQUAL )
            {   // ignore comment lines, but write them to Correct Items File
                if ( bWriteCorrect )
                       aOkOut.WriteLine( sGSILine );
            }
            else
            {
                ByteString aId = GetUniqId( sGSILine );
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
                    pBlock = new GSIBlock( bPrintContext, bInternal );

                    aOldId = aId;
                }

                pBlock->InsertLine( sGSILine, nLine , sourceLang);
            }
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
