/*************************************************************************
 *
 *  $RCSfile: merge.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 12:42:13 $
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
#include "export.hxx"
#include "utf8conv.hxx"

extern void ConvertHalfwitdhToFullwidth( String& rString );

//
// class PFormEntrys
//

/*****************************************************************************/
BOOL PFormEntrys::GetText( ByteString &rReturn,
    //USHORT nTyp, USHORT nLangIndex, BOOL bDel )
    USHORT nTyp, const ByteString &nLangIndex, BOOL bDel )
/*****************************************************************************/
{
    BOOL bReturn = false;
    switch ( nTyp ) {
        case STRING_TYP_TEXT :
            rReturn = sText[ nLangIndex ];
            if ( bDel )
                sText[ nLangIndex ] = "";
            bReturn = bTextFirst[ nLangIndex ];
            bTextFirst[ nLangIndex ] = FALSE;
            break;
        case STRING_TYP_HELPTEXT :
            rReturn = sHelpText;
            break;
        case STRING_TYP_QUICKHELPTEXT :
            rReturn = sQuickHelpText[ nLangIndex ];
            if ( bDel )
                sQuickHelpText[ nLangIndex ] = "";
            bReturn = bQuickHelpTextFirst[ nLangIndex ];
            bQuickHelpTextFirst[ nLangIndex ] = FALSE;
            break;
        case STRING_TYP_TITLE :
            rReturn = sTitle[ nLangIndex ];
            if ( bDel )
                sTitle[ nLangIndex ] = "";
            bReturn = bTitleFirst[ nLangIndex ];
            bTitleFirst[ nLangIndex ] = FALSE;
            break;
    }
    //printf("Returning '%s'\n",rReturn.GetBuffer());
    return bReturn;
}


//
// class MergeData
//

/*****************************************************************************/
MergeData::~MergeData()
/*****************************************************************************/
{
}

/*****************************************************************************/
PFormEntrys* MergeData::GetPFormEntrys( ResData *pResData )
/*****************************************************************************/
{
//    if( pResData->sPForm.Len() && aMap.find( pResData->sPForm ) != aMap.end() ){
    if( aMap.find( ByteString("HACK") ) != aMap.end() ){
        return aMap[ ByteString("HACK") ];
    }
    else{
        return 0;
    }
}

void MergeData::Insert( const ByteString& rPFO , PFormEntrys* pfEntrys ){
//    aMap.insert( PFormEntrysHashMap::value_type( rPFO , pfEntrys ) );
    aMap.insert( PFormEntrysHashMap::value_type( ByteString("HACK") , pfEntrys ) );

}

PFormEntrys* MergeData::GetPFObject( const ByteString& rPFO ){
    if( aMap.find( ByteString("HACK") ) != aMap.end() ){
//    if( aMap.find( rPFO ) != aMap.end() ){
        return aMap[ rPFO ];
    }
    else{
        return 0;
    }
}


/*****************************************************************************/
PFormEntrys *MergeData::InsertEntry( const ByteString &rPForm )
/*****************************************************************************/
{
    PFormEntrys* pFEntrys = new PFormEntrys( rPForm );
    aMap.insert( PFormEntrysHashMap::value_type( rPForm , pFEntrys ) );
    return pFEntrys;
}

/*****************************************************************************/
BOOL MergeData::operator==( ResData *pData )
/*****************************************************************************/
{
    return (( pData->sId == sLID ) &&
            ( pData->sGId == sGID ) &&
            ( ByteString( pData->sResTyp ).ToUpperAscii() ==
                ByteString( sTyp ).ToUpperAscii()));
}

//
// class MergeDataFile
//

#define FFORMAT_UNKNOWN 0x0000
#define FFORMAT_NEW     0x0001
#define FFORMAT_OLD     0x0002

/*void MergeDataFile::Quote( ByteString& sText ){
    for( int x=0; x < sText.Len(); x++ ){
        if( sText.GetChar( x ) == '\"' ){
            if( x > 1 && sText.GetChar( x-1) != '\\' ){
                sText.Insert('\\', x);
                x++;
            }else{
                sText.Insert('\\',0);
                x++;
            }
        }
    }
}*/
/*****************************************************************************/
MergeDataFile::MergeDataFile( const ByteString &rFileName, const ByteString& sFile ,BOOL bErrLog,
                            CharSet aCharSet, BOOL bUTF8 )
/*****************************************************************************/
                : bErrorLog( bErrLog )
{
    SvFileStream aInputStream( String( rFileName, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
    aInputStream.SetStreamCharSet( aCharSet );
    ByteString sLine;

    ByteString sTYP;
    ByteString sGID;
    ByteString sLID;
    ByteString sPFO;
    ByteString nLANG;
    ByteString sTEXT;
    ByteString sQHTEXT;
    ByteString sTITLE;

    fprintf( stdout, "Scanning File %s ...\n", rFileName.GetBuffer());

    ULONG nFileFormat = FFORMAT_UNKNOWN;
    if( !aInputStream.IsOpen() ) {
        printf("ERROR : Can't open %s\n", rFileName.GetBuffer());
        exit( -1 );
    }
    while ( !aInputStream.IsEof()) {
        aInputStream.ReadLine( sLine );
        sLine = sLine.Convert( RTL_TEXTENCODING_MS_1252, aCharSet );

        if ( sLine.GetTokenCount( '\t' ) == 15  ) {
            if ( nFileFormat != FFORMAT_NEW ) {
                nFileFormat = FFORMAT_NEW;
                fprintf( stdout, "File detection: Version 2.0 detected!\n" );
            }
            // Merge While Build speedup
            if(
                 ( sLine.GetToken( 1 ,'\t').Search( sFile ) != STRING_NOTFOUND )
                 //( sLine.GetToken( 1 ,'\t').Search( sFile ) > 0 )
                ){
                sTYP = sLine.GetToken( 3, '\t' );
                sGID = sLine.GetToken( 4, '\t' );
                sLID = sLine.GetToken( 5, '\t' );
                sPFO = sLine.GetToken( 7, '\t' );
                sPFO = ByteString("HACK");
                nLANG = sLine.GetToken( 9, '\t' );

    /*          if ( bUTF8 )
                    sLine = UTF8Converter::ConvertFromUTF8( sLine, Export::GetCharSet( nLANG ));
    */
                sTEXT = sLine.GetToken( 10, '\t' );
    //            printf("%s\n",sTEXT.GetBuffer());
    //            Quote( sTEXT );
    //            printf("%s\n",sTEXT.GetBuffer());

                sQHTEXT = sLine.GetToken( 12, '\t' );
                sTITLE = sLine.GetToken( 13, '\t' );

                //if (( nLANG != 49 ) && ( LANGUAGE_ALLOWED( GetLangIndex( nLANG ))))
                nLANG.EraseLeadingAndTrailingChars();
                if ( !nLANG.EqualsIgnoreCaseAscii( "de" ) && !nLANG.EqualsIgnoreCaseAscii("en-US")  ){
                    InsertEntry( sTYP, sGID, sLID, sPFO, nLANG, sTEXT, sQHTEXT, sTITLE );
                    if( nLANG.Len() > 0 ){
                        bool bFound = false;
                        for( int x = 0; x < aLanguages.size(); x++ ){
                            if( aLanguages[ x ].Equals( nLANG ) )
                                bFound = true;
                        }
                        if( !bFound )   aLanguages.push_back( nLANG );
                    }
                }
            }
        }
        else if ( sLine.GetTokenCount( '\t' ) == 10 ){
            if ( nFileFormat != FFORMAT_OLD ) {
                nFileFormat = FFORMAT_OLD;
                fprintf( stdout, "File detection: Version 1.0 detected!\n" );
            }
            sTYP = sLine.GetToken( 1, '\t' );
                sTYP = sTYP.Copy( 1 ); sTYP.Erase( sTYP.Len() - 1 );
            sGID = sLine.GetToken( 2, '\t' );
                sGID = sGID.Copy( 1 ); sGID.Erase( sGID.Len() - 1 );
            sLID = sLine.GetToken( 3, '\t' );
                sLID = sLID.Copy( 1 ); sLID.Erase( sLID.Len() - 1 );
            sPFO = sLine.GetToken( 4, '\t' );
                sPFO = sPFO.Copy( 1 ); sPFO.Erase( sPFO.Len() - 1 );
            nLANG = sLine.GetToken( 5, '\t' );

//          if ( bUTF8 )
//              sLine = UTF8Converter::ConvertFromUTF8( sLine, Export::GetCharSet( nLANG ));

            if (( nLANG.EqualsIgnoreCaseAscii( JAPANESE_ISO ))) // == JAPANESE )
            {
                String sSLine( sLine, RTL_TEXTENCODING_UTF8 );
                ConvertHalfwitdhToFullwidth( sSLine );
                sLine = ByteString( sSLine, RTL_TEXTENCODING_UTF8 );
            }

            sTEXT = sLine.GetToken( 6, '\t' );
                sTEXT = sTEXT.Copy( 1 ); sTEXT.Erase( sTEXT.Len() - 1 );
       //     Quote( sTEXT );

            sQHTEXT = sLine.GetToken( 8, '\t' );
                sQHTEXT = sQHTEXT.Copy( 1 ); sQHTEXT.Erase( sQHTEXT.Len() - 1 );
            sTITLE = sLine.GetToken( 9, '\t' );
                sTITLE = sTITLE.Copy( 1 ); sTITLE.Erase( sTITLE.Len() - 1 );

            //if (( nLANG != 49 ) && ( LANGUAGE_ALLOWED( GetLangIndex( nLANG ))))
            nLANG.EraseLeadingAndTrailingChars();
            if ( !nLANG.EqualsIgnoreCaseAscii("de") && !nLANG.EqualsIgnoreCaseAscii("en-US") ){
                InsertEntry( sTYP, sGID, sLID, sPFO, nLANG, sTEXT, sQHTEXT, sTITLE );
                if( nLANG.Len() > 0 ){
                    bool bFound = false;
                    for( int x = 0; x < aLanguages.size(); x++ ){
                        if( aLanguages[ x ].Equals( nLANG ) )
                            bFound = true;
                     }
                     if( !bFound )   aLanguages.push_back( nLANG );

                }
            }
        }
    }
    aInputStream.Close();
    fprintf( stdout, "Merging ...\n" );
}

/*****************************************************************************/
MergeDataFile::~MergeDataFile()
/*****************************************************************************/
{
}

/*****************************************************************************/
void MergeDataFile::WriteErrorLog( const ByteString &rFileName )
/*****************************************************************************/
{
// Ivo
    /*  if ( bErrorLog ) {
        DirEntry aDirEntry( String( rFileName, RTL_TEXTENCODING_ASCII_US ));
        aDirEntry.SetExtension( String( "err", RTL_TEXTENCODING_ASCII_US ));
        sErrorLog = ByteString( aDirEntry.GetFull(), RTL_TEXTENCODING_ASCII_US );
    }
    for ( ULONG i = 0; i < Count(); i++ ) {
        MergeData *pData = GetObject( i );

        for ( ULONG j = 0; j < pData->Count(); j++ ) {
            PFormEntrys *pEntrys = pData->GetObject( j );
            for ( USHORT nLangIndex = 0; nLangIndex < LANGUAGES; nLangIndex++ ) {
                if ( pEntrys->sText[ nLangIndex ].Len() ||
                    pEntrys->sQuickHelpText[ nLangIndex ].Len() ||
                    pEntrys->sTitle[ nLangIndex ].Len())
                {
                    ByteString sLine( "0\t" );
                    sLine += pData->sTyp; sLine += "\t";
                    sLine += pData->sGID; sLine += "\t";
                    sLine += pData->sLID; sLine += "\t";
                    sLine += ByteString::CreateFromInt64( Export::LangId[ nLangIndex ] ); sLine += "\t";
                    sLine += pEntrys->sText[ nLangIndex ]; sLine += "\t\t";
                    sLine += pEntrys->sQuickHelpText[ nLangIndex ]; sLine += "\t";
                    sLine += pEntrys->sTitle[ nLangIndex ];
                    WriteError( sLine );
                }
            }
        }
    }
    if ( aErrLog.IsOpen())
        aErrLog.Close(); */
}

/*****************************************************************************/
void MergeDataFile::WriteError( const ByteString &rLine )
/*****************************************************************************/
{
    if ( bErrorLog ) {
        if ( !aErrLog.IsOpen())
            aErrLog.Open( String( sErrorLog, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        aErrLog.WriteLine( rLine );
    }
    else
        fprintf( stderr, "%s\n", rLine.GetBuffer());
}
std::vector<ByteString> MergeDataFile::GetLanguages(){
    return aLanguages;
}
/*****************************************************************************/
MergeData *MergeDataFile::GetMergeData( ResData *pResData )
/*****************************************************************************/
{
    ByteString sOldG = pResData->sGId;
    ByteString sOldL = pResData->sId;
    ByteString sGID = pResData->sGId;
    ByteString sLID;
    if ( !sGID.Len())
        sGID = pResData->sId;
    else
        sLID = pResData->sId;
    pResData->sGId = sGID;
    pResData->sId = sLID;

    ByteString sKey = CreateKey( pResData->sResTyp , pResData->sGId , pResData->sId );

    if( aMap.find( sKey ) != aMap.end() ){
        pResData->sGId = sOldG;
        pResData->sId = sOldL;
        return aMap[ sKey ];
    }
    pResData->sGId = sOldG;
    pResData->sId = sOldL;
    return NULL;
}

/*****************************************************************************/
PFormEntrys *MergeDataFile::GetPFormEntrys( ResData *pResData )
/*****************************************************************************/
{
    // search for requested PFormEntrys
    MergeData *pData = GetMergeData( pResData );
    if ( pData )
        return pData->GetPFormEntrys( pResData );
    return NULL;
}

/*****************************************************************************/
void MergeDataFile::InsertEntry(
                    const ByteString &rTYP, const ByteString &rGID,
                    const ByteString &rLID, const ByteString &rPFO,
                    const ByteString &nLANG, const ByteString &rTEXT,
                    const ByteString &rQHTEXT, const ByteString &rTITLE )
/*****************************************************************************/
{
    MergeData *pData = NULL;
    BOOL bFound = FALSE;
    long int i;

    // search for MergeData
    ByteString sKey = CreateKey( rTYP , rGID , rLID );
    if( aMap.find( sKey ) != aMap.end() ){
        pData = aMap[ sKey ];
    }else{
        pData = new MergeData( rTYP, rGID, rLID );
        aMap.insert( MergeDataHashMap::value_type( CreateKey( rTYP , rGID , rLID ) , pData ) );
    }

    bFound = FALSE;
    PFormEntrys *pFEntrys = 0;

    // search for PFormEntrys

    pFEntrys = pData->GetPFObject( rPFO );
    if( !pFEntrys ){
        // create new PFormEntrys, cause no one exists with current properties
        pFEntrys = new PFormEntrys( rPFO );
        pData->Insert( rPFO , pFEntrys );
    }

    // finaly insert the cur string
    pFEntrys->InsertEntry( nLANG , rTEXT, rQHTEXT, rTITLE );
}
ByteString MergeDataFile::CreateKey( const ByteString& rTYP , const ByteString& rGID , const ByteString& rLID ){
    ByteString sKey( rTYP );
    sKey.Append( '-' );
    sKey.Append( rGID );
    sKey.Append( '-' );
    sKey.Append( rLID  );
    return sKey.ToUpperAscii();
}


