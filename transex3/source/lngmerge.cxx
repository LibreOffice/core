/*************************************************************************
 *
 *  $RCSfile: lngmerge.cxx,v $
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
#include <tools/fsys.hxx>

// local includes
#include "lngmerge.hxx"

//
// class LngParser
//

/*****************************************************************************/
LngParser::LngParser( const ByteString &rLngFile )
/*****************************************************************************/
                : sSource( rLngFile ),
                nError( LNG_OK ),
                pLines( NULL )
{
    pLines = new LngLineList( 100, 100 );

    DirEntry aEntry( String( sSource, RTL_TEXTENCODING_ASCII_US ));
    if ( aEntry.Exists()) {
        SvFileStream aStream( String( sSource, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
        if ( aStream.IsOpen()) {
            ByteString sLine;
            while ( !aStream.IsEof()) {
                aStream.ReadLine( sLine );
                pLines->Insert( new ByteString( sLine ), LIST_APPEND );
            }
        }
        else
            nError = LNG_COULD_NOT_OPEN;
    }
    else
        nError = LNG_FILE_NOTFOUND;
}

/*****************************************************************************/
LngParser::~LngParser()
/*****************************************************************************/
{
    for ( ULONG i = 0; i < pLines->Count(); i++ )
        delete pLines->GetObject( i );
    delete pLines;
}

/*****************************************************************************/
BOOL LngParser::CreateSDF(
    const ByteString &rSDFFile, const ByteString &rPrj,
    const ByteString &rRoot )
/*****************************************************************************/
{
    SvFileStream aSDFStream( String( rSDFFile, RTL_TEXTENCODING_ASCII_US ),
        STREAM_STD_WRITE | STREAM_TRUNC );
    if ( !aSDFStream.IsOpen()) {
        nError = SDF_COULD_NOT_OPEN;
    }
    nError = SDF_OK;

    DirEntry aEntry( String( sSource, RTL_TEXTENCODING_ASCII_US ));
    aEntry.ToAbs();
    String sFullEntry = aEntry.GetFull();
    aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
    aEntry += DirEntry( rRoot );
    ByteString sPrjEntry( aEntry.GetFull(), gsl_getSystemTextEncoding());
    ByteString sActFileName(
        sFullEntry.Copy( sPrjEntry.Len() + 1 ), gsl_getSystemTextEncoding());
    sActFileName.ToLowerAscii();

    ULONG nPos = 0;
    BOOL bGroup = FALSE;
    ByteString sGroup;

    // seek to next group
    while ( nPos < pLines->Count() && !bGroup ) {
        ByteString sLine( *pLines->GetObject( nPos ));
        sLine.EraseLeadingChars( ' ' );
        sLine.EraseTrailingChars( ' ' );
        if (( sLine.GetChar( 0 ) == '[' ) &&
            ( sLine.GetChar( sLine.Len() - 1 ) == ']' ))
        {
            sGroup = sLine.GetToken( 1, '[' ).GetToken( 0, ']' );
            sGroup.EraseLeadingChars( ' ' );
            sGroup.EraseTrailingChars( ' ' );
            bGroup = TRUE;
        }
        nPos ++;
    }

    while ( nPos < pLines->Count()) {

        ByteString Text[ LANGUAGES ];
        ByteString sID( sGroup );

        // read languages
        bGroup = FALSE;

        while ( nPos < pLines->Count() && !bGroup ) {
            ByteString sLine( *pLines->GetObject( nPos ));
            sLine.EraseLeadingChars( ' ' );
            sLine.EraseTrailingChars( ' ' );
            if (( sLine.GetChar( 0 ) == '[' ) &&
                ( sLine.GetChar( sLine.Len() - 1 ) == ']' ))
            {
                sGroup = sLine.GetToken( 1, '[' ).GetToken( 0, ']' );
                sGroup.EraseLeadingChars( ' ' );
                sGroup.EraseTrailingChars( ' ' );
                bGroup = TRUE;
            }
            else if ( sLine.GetTokenCount( '=' ) > 1 ) {
                ByteString sLang = sLine.GetToken( 0, '=' );
                sLang.EraseLeadingChars( ' ' );
                sLang.EraseTrailingChars( ' ' );
                if (( sLang.IsNumericAscii()) &&
                    ( MergeDataFile::GetLangIndex( sLang.ToInt32()) < LANGUAGES ))
                {
                    // this is a valid text line
                    ByteString sText = sLine.GetToken( 1, '\"' ).GetToken( 0, '\"' );
                    USHORT nIndex = MergeDataFile::GetLangIndex( sLang.ToInt32());
                    Text[ nIndex ] = sText;
                }
            }
            nPos ++;
        }
        BOOL bExport = Text[ GERMAN_INDEX ].Len() &&
            ( Text[ ENGLISH_INDEX ].Len() || Text[ ENGLISH_US_INDEX ].Len());
        if ( bExport ) {

            Time aTime;
            ByteString sTimeStamp( ByteString::CreateFromInt64( Date().GetDate()));
            sTimeStamp += " ";
            sTimeStamp += ByteString::CreateFromInt32( aTime.GetHour());
            sTimeStamp += ":";
            sTimeStamp += ByteString::CreateFromInt32( aTime.GetMin());
            sTimeStamp += ":";
            sTimeStamp += ByteString::CreateFromInt32( aTime.GetSec());

            for ( ULONG i = 0; i < LANGUAGES; i++ ) {
                if ( LANGUAGE_ALLOWED( i )) {
                    ByteString sAct = Text[ i ];
                    if ( !sAct.Len() && i )
                        sAct = Text[ GERMAN_INDEX ];

                    ByteString sOutput( rPrj ); sOutput += "\t";
                    if ( rRoot.Len())
                        sOutput += sActFileName;
                    sOutput += "\t0\t";
                    sOutput += "LngText\t";
                    sOutput += sID; sOutput += "\t\t\t\t0\t";
                    sOutput += ByteString::CreateFromInt64( Export::LangId[ i ] ); sOutput += "\t";
                    sOutput += sAct; sOutput += "\t\t\t\t";
                    sOutput += sTimeStamp;

                    aSDFStream.WriteLine( sOutput );
                }
            }
        }
    }
    aSDFStream.Close();
    return TRUE;
}

/*****************************************************************************/
BOOL LngParser::Merge(
    const ByteString &rSDFFile, const ByteString &rDestinationFile )
/*****************************************************************************/
{
    SvFileStream aDestination(
        String( rDestinationFile, RTL_TEXTENCODING_ASCII_US ),
        STREAM_STD_WRITE | STREAM_TRUNC );
    if ( !aDestination.IsOpen()) {
        nError = LNG_COULD_NOT_OPEN;
    }
    nError = LNG_OK;

    MergeDataFile aMergeDataFile( rSDFFile, FALSE, RTL_TEXTENCODING_MS_1252 );

    ULONG nPos = 0;
    BOOL bGroup = FALSE;
    ByteString sGroup;

    // seek to next group
    while ( nPos < pLines->Count() && !bGroup ) {
        ByteString sLine( *pLines->GetObject( nPos ));
        sLine.EraseLeadingChars( ' ' );
        sLine.EraseTrailingChars( ' ' );
        if (( sLine.GetChar( 0 ) == '[' ) &&
            ( sLine.GetChar( sLine.Len() - 1 ) == ']' ))
        {
            sGroup = sLine.GetToken( 1, '[' ).GetToken( 0, ']' );
            sGroup.EraseLeadingChars( ' ' );
            sGroup.EraseTrailingChars( ' ' );
            bGroup = TRUE;
        }
        nPos ++;
    }

    while ( nPos < pLines->Count()) {

        ByteString Text[ LANGUAGES ];
        ByteString sID( sGroup );
        ULONG nLastLangPos = 0;

        ResData  *pResData = new ResData( "", sID );
        pResData->sResTyp = "lngtext";
        PFormEntrys *pEntrys = aMergeDataFile.GetPFormEntrys( pResData );

        // read languages
        bGroup = FALSE;

        while ( nPos < pLines->Count() && !bGroup ) {
            ByteString sLine( *pLines->GetObject( nPos ));
            sLine.EraseLeadingChars( ' ' );
            sLine.EraseTrailingChars( ' ' );
            if (( sLine.GetChar( 0 ) == '[' ) &&
                ( sLine.GetChar( sLine.Len() - 1 ) == ']' ))
            {
                sGroup = sLine.GetToken( 1, '[' ).GetToken( 0, ']' );
                sGroup.EraseLeadingChars( ' ' );
                sGroup.EraseTrailingChars( ' ' );
                bGroup = TRUE;
            }
            else if ( sLine.GetTokenCount( '=' ) > 1 ) {
                ByteString sLang = sLine.GetToken( 0, '=' );
                sLang.EraseLeadingChars( ' ' );
                sLang.EraseTrailingChars( ' ' );
                if (( sLang.IsNumericAscii()) &&
                    ( MergeDataFile::GetLangIndex( sLang.ToInt32()) < LANGUAGES ) &&
                    ( pEntrys ))
                {
                    // this is a valid text line
                    USHORT nIndex = MergeDataFile::GetLangIndex( sLang.ToInt32());
                    ByteString sNewText;
                    pEntrys->GetText( sNewText, STRING_TYP_TEXT, nIndex, TRUE );
                    if ( sNewText.Len()) {
                        ByteString *pLine = pLines->GetObject( nPos );

                        if ( sLang.ToInt32() != GERMAN ) {

                            ByteString sText( sLang );
                            sText += " = \"";
                            sText += sNewText;
                            sText += "\"";
                            *pLine = sText;
                        }
                        Text[ nIndex ] = sNewText;
                    }
                    nLastLangPos = nPos;
                }
            }
            nPos ++;
        }
        if ( nLastLangPos ) {
            for ( USHORT i = 0; i < LANGUAGES; i++ ) {
                if (( i != GERMAN ) && ( !Text[ i ].Len())) {
                    ByteString sNewText;
                    pEntrys->GetText( sNewText, STRING_TYP_TEXT, i, TRUE );
                    if (( sNewText.Len()) &&
                        !(( i == COMMENT ) && ( sNewText == "-" )))
                    {
                        ByteString sLine;
                        if ( Export::LangId[ i ] < 10 )
                            sLine += "0";
                        sLine += Export::LangId[ i ];
                        sLine += " = \"";
                        sLine += sNewText;
                        sLine += "\"";

                        nLastLangPos++;
                        nPos++;

                        pLines->Insert( new ByteString( sLine ), nLastLangPos );
                    }
                }
            }
        }

        delete pResData;
    }

    for ( ULONG i = 0; i < pLines->Count(); i++ )
        aDestination.WriteLine( *pLines->GetObject( i ));

    aDestination.Close();
    return TRUE;
}
