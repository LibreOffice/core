/*************************************************************************
 *
 *  $RCSfile: lngmerge.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:52:35 $
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
#include "utf8conv.hxx"

//
// class LngParser
//

/*****************************************************************************/
LngParser::LngParser( const ByteString &rLngFile, BOOL bUTF8, BOOL bULFFormat , bool bQuiet_in )
/*****************************************************************************/
                : sSource( rLngFile ),
                nError( LNG_OK ),
                pLines( NULL ),
                bDBIsUTF8( bUTF8 ),
                bULF( bULFFormat ),
                bQuiet( bQuiet_in )
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
void LngParser::FillInFallbacks( ByteStringHashMap Text )
/*****************************************************************************/
{
    ByteString sCur;
    for( long int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];

        if( !sCur.EqualsIgnoreCaseAscii("de") && !sCur.EqualsIgnoreCaseAscii("en-US") ){
        //if( !sCur.Equals(ByteString("de")) && !sCur.Equals(ByteString("en-US"))){
            ByteString sFallbackLang = Export::GetFallbackLanguage( sCur );
            if( sFallbackLang.Len() ){
                Text[ sCur ] = Text[ sFallbackLang ];
            }
        }
    }
}

/*****************************************************************************/
BOOL LngParser::CreateSDF(
    const ByteString &rSDFFile, const ByteString &rPrj,
    const ByteString &rRoot )
/*****************************************************************************/
{

    Export::InitLanguages( false );
    aLanguages = Export::GetLanguages();
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
    sActFileName.SearchAndReplaceAll( "/", "\\" );

    ULONG nPos  = 0;
    BOOL bStart = true;
    ByteString sGroup;
      ByteStringHashMap Text;
    ByteString sID;
    ByteString sLine;

    while( nPos < pLines->Count() ){
        sLine = *pLines->GetObject( nPos++ );
        while( nPos < pLines->Count() && !isNextGroup( sGroup , sLine ) ){
            ReadLine( sLine , Text );
            sID = sGroup;
            sLine = *pLines->GetObject( nPos++ );
        };
        if( bStart ){
            bStart = false;
            sID = sGroup;
        }
        else {

            WriteSDF( aSDFStream , Text , rSDFFile , rPrj , rRoot , sActFileName , sID );
        }
    }
    aSDFStream.Close();
    return true;
}

 void LngParser::WriteSDF( SvFileStream &aSDFStream , ByteStringHashMap &rText_inout ,
     const ByteString &rSDFFile, const ByteString &rPrj , const ByteString &rRoot ,
     const ByteString &sActFileName , const ByteString &sID )
 {
    //BOOL bExport = (rText_inout[ ByteString("de") ].Len() > 0) && (rText_inout[ ByteString("en-US") ].Len() > 0);
    BOOL bExport = true;
    if ( bExport ) {
           ByteString sTimeStamp( Export::GetTimeStamp());
        ByteString sCur;
        FillInFallbacks( rText_inout );
        for( long int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];
            ByteString sAct = rText_inout[ sCur ];
            if ( !sAct.Len() && sCur.Len() )
                sAct = rText_inout[ ByteString("de") ];
            ByteString sOutput( rPrj ); sOutput += "\t";
            if ( rRoot.Len())
                sOutput += sActFileName;
            sOutput += "\t0\t";
            sOutput += "LngText\t";
            sOutput += sID; sOutput += "\t\t\t\t0\t";
            sOutput += sCur; sOutput += "\t";
            sOutput += sAct; sOutput += "\t\t\t\t";
            sOutput += sTimeStamp;
            aSDFStream.WriteLine( sOutput );
        }
    }
 }
 bool LngParser::isNextGroup( ByteString &sGroup_out , ByteString &sLine_in ){
    sLine_in.EraseLeadingChars( ' ' );
    sLine_in.EraseTrailingChars( ' ' );
    if (( sLine_in.GetChar( 0 ) == '[' ) &&
            ( sLine_in.GetChar( sLine_in.Len() - 1 ) == ']' )){
        sGroup_out = sLine_in.GetToken( 1, '[' ).GetToken( 0, ']' );
        sGroup_out.EraseLeadingChars( ' ' );
        sGroup_out.EraseTrailingChars( ' ' );
        return true;
    }
    return false;
 }
 void LngParser::ReadLine( const ByteString &sLine_in , ByteStringHashMap &rText_inout){
    //printf("sLine -> '%s'\n",sLine_in.GetBuffer());
    ByteString sLang = sLine_in.GetToken( 0, '=' );
    sLang.EraseLeadingChars( ' ' );
    sLang.EraseTrailingChars( ' ' );
    ByteString sText = sLine_in.GetToken( 1, '\"' ).GetToken( 0, '\"' );
    if( sLang.Len() )
        rText_inout[ sLang ] = sText;
 }

/*****************************************************************************/
BOOL LngParser::Merge(
    const ByteString &rSDFFile, const ByteString &rDestinationFile )
/*****************************************************************************/
{
    Export::InitLanguages( true );
    SvFileStream aDestination(
        String( rDestinationFile, RTL_TEXTENCODING_ASCII_US ),
        STREAM_STD_WRITE | STREAM_TRUNC );
    if ( !aDestination.IsOpen()) {
        nError = LNG_COULD_NOT_OPEN;
    }
    nError = LNG_OK;
//    MergeDataFile( const ByteString &rFileName, const ByteString& rFile , BOOL bErrLog, CharSet aCharSet, BOOL bUTF8 );

    MergeDataFile aMergeDataFile( rSDFFile, sSource , FALSE, RTL_TEXTENCODING_MS_1252, bDBIsUTF8 );
    ByteString sTmp( Export::sLanguages );
    if( sTmp.ToUpperAscii().Equals("ALL") )
        Export::SetLanguages( aMergeDataFile.GetLanguages() );
    aLanguages = Export::GetLanguages();

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
        ByteStringHashMap Text;
        ByteString sID( sGroup );
        ULONG nLastLangPos = 0;

        ResData  *pResData = new ResData( "", sID );
        pResData->sResTyp = "LngText";
        PFormEntrys *pEntrys = aMergeDataFile.GetPFormEntrys( pResData );

        // read languages
        bGroup = FALSE;

        ByteString sLanguagesDone;

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
                nPos ++;
                sLanguagesDone = "";
            }
            else if ( sLine.GetTokenCount( '=' ) > 1 ) {
                ByteString sLang = sLine.GetToken( 0, '=' );
                sLang.EraseLeadingChars( ' ' );
                sLang.EraseTrailingChars( ' ' );

                ByteString sSearch( ";" );
                sSearch += sLang;
                sSearch += ";";

                if (( sLanguagesDone.Search( sSearch ) != STRING_NOTFOUND )) {
                    pLines->Remove( nPos );
                }
                if( bULF && pEntrys )
                {
                    // this is a valid text line
                    ByteString sText = sLine.GetToken( 1, '\"' ).GetToken( 0, '\"' );
                    USHORT nIndex;
                    if( sLang.Len() ){
                        ByteString sNewText;
                        pEntrys->GetText( sNewText, STRING_TYP_TEXT, sLang, TRUE );
                        //if ( bULF )
                        //  sNewText = UTF8Converter::ConvertToUTF8( sNewText, Export::GetCharSet( Export::LangId[ nIndex ] ));

                        if ( sNewText.Len()) {
                            ByteString *pLine = pLines->GetObject( nPos );

                            if( !sLang.EqualsIgnoreCaseAscii("de") && !sLang.EqualsIgnoreCaseAscii("en-US") ){
                                ByteString sText( sLang );
                                sText += " = \"";
                                sText += sNewText;
                                sText += "\"";
                                *pLine = sText;
                            }
                            Text[ sLang ] = sNewText;
                        }
                    }
                    nLastLangPos = nPos;
                    nPos ++;
                    sLanguagesDone += sSearch;
                }
                else {
                    nLastLangPos = nPos;
                    nPos ++;
                    sLanguagesDone += sSearch;
                }
            }
            else
                nPos++;
        }
        ByteString sCur;
        if ( nLastLangPos ) {
            for( long int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];

                if( ( !sCur.EqualsIgnoreCaseAscii("de") && !sCur.EqualsIgnoreCaseAscii("en-US")) && !Text[ sCur ].Len() && pEntrys ){
                ByteString sNewText;
                    pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur, TRUE );
                    if (( sNewText.Len()) &&
                        !(( sCur.Equals("x-comment") ) && ( sNewText == "-" )))
                    {
                        ByteString sLine;
                        sLine += sCur;
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
