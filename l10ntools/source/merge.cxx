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
#include "export.hxx"
#include "utf8conv.hxx"
#include <iostream>

using namespace std;

extern void ConvertHalfwitdhToFullwidth( String& rString );

//
// class PFormEntrys
//

ByteString PFormEntrys::Dump(){
    ByteString sRet( "PFormEntrys\n" );
    //sRet.Append( Export::DumpMap( ByteString("sText") , sText ) );
    //sRet.Append("\n");
    ByteString a("sText");
    if ( sText.size() ) Export::DumpMap( a , sText );
    return sRet;
}

BOOL PFormEntrys::GetTransex3Text( ByteString &rReturn,
    USHORT nTyp, const ByteString &nLangIndex, BOOL bDel )
{
    BOOL rc = GetText( rReturn , nTyp , nLangIndex , bDel );
    ByteString test( rReturn );
    for( USHORT idx = 0; idx < rReturn.Len(); idx++ )
    {
        if( rReturn.GetChar( idx ) == '\"' && ( idx >= 1 )  &&  rReturn.GetChar( idx-1 ) == '\\' )
        {
            rReturn.Erase( idx-1 , 1 );
        }
    }
    //if( !rReturn.Equals( test ) )
    //    printf("*CHANGED******************\n%s\n%s\n",test.GetBuffer(),rReturn.GetBuffer());
    return rc;
}
/*****************************************************************************/
BOOL PFormEntrys::GetText( ByteString &rReturn,
    USHORT nTyp, const ByteString &nLangIndex, BOOL bDel )
/*****************************************************************************/
{

    /*printf("DBG: PFormEntrys::GetText(nId=%s)\n",nLangIndex.GetBuffer() );

            // DEBUG******************
            ByteStringHashMap::const_iterator idbg;
            std::cout << "HASHKEYS : \n";
            for( idbg = sText.begin() ; idbg != sText.end(); ++idbg )
                std::cout << (idbg->first).GetBuffer() << "\n";
            std::cout << "\n\n";
            std::cout << "String sText[ nLangIndex ] = " << sText[ nLangIndex ].GetBuffer() << "\n";
            // DEBUG******************
*/

    BOOL bReturn=TRUE;
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

    (void) pResData;    // FIXME
        if( aMap.find( ByteString("HACK") ) != aMap.end() ){
            return aMap[ ByteString("HACK") ];
        }
        else{
            return 0;
        }
}

void MergeData::Insert( const ByteString& rPFO , PFormEntrys* pfEntrys ){
    (void) rPFO;    // FIXME
    aMap.insert( PFormEntrysHashMap::value_type( ByteString("HACK") , pfEntrys ) );

}
ByteString MergeData::Dump(){
    ByteString sRet( "MergeData\n" );

    printf("MergeData sTyp = %s , sGid = %s , sLid =%s , sFilename = %s\n",sTyp.GetBuffer(),sGID.GetBuffer(),sLID.GetBuffer(), sFilename.GetBuffer() );

    PFormEntrysHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg ){
        printf("aMap[ %s ] = " ,idbg->first.GetBuffer());
        ( (PFormEntrys*)(idbg->second) )->Dump();
        printf("\n") ;
    }
    printf("\n") ;
    return sRet;
}

PFormEntrys* MergeData::GetPFObject( const ByteString& rPFO ){
    if( aMap.find( ByteString("HACK") ) != aMap.end() ){
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
    ByteString sResTyp_upper( pData->sResTyp );
    sResTyp_upper.ToUpperAscii();
    ByteString sTyp_upper( sTyp );
    sTyp_upper.ToUpperAscii();

    return (( pData->sId == sLID ) &&
            ( pData->sGId == sGID ) &&
            ( sResTyp_upper  ==  sTyp_upper )
            );
}

//
// class MergeDataFile
//

#define FFORMAT_UNKNOWN 0x0000
#define FFORMAT_NEW     0x0001
#define FFORMAT_OLD     0x0002

/*****************************************************************************/
MergeDataFile::MergeDataFile( const ByteString &rFileName, const ByteString& sFile ,BOOL bErrLog,
                            CharSet aCharSet, bool bCaseSensitive )

/*****************************************************************************/
                : bErrorLog( bErrLog )
{

    SvFileStream aInputStream( String( rFileName, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
    aInputStream.SetStreamCharSet( aCharSet );
    ByteString sLine;
//    printf("\nReading localize.sdf ...\n");
    ByteString sTYP;
    ByteString sGID;
    ByteString sLID;
    ByteString sPFO;
    ByteString nLANG;
    ByteString sTEXT;
    ByteString sQHTEXT;
    ByteString sTITLE;
    ByteString sHACK("HACK");

    const ByteString sEmpty("");

    if( !aInputStream.IsOpen() ) {
        printf("Warning : Can't open %s\n", rFileName.GetBuffer());
        return;
    }
    while ( !aInputStream.IsEof()) {
        xub_StrLen nToks;
        aInputStream.ReadLine( sLine );
        sLine = sLine.Convert( RTL_TEXTENCODING_MS_1252, aCharSet );

        nToks = sLine.GetTokenCount( '\t' );
        if ( nToks == 15 ) {
            // Skip all wrong filenames
            ByteString filename = sLine.GetToken( 1 , '\t' );
            filename = filename.Copy( filename.SearchCharBackward( "\\" )+1 , filename.Len() );

            if( sFile.Equals( sEmpty ) || ( !sFile.Equals( sEmpty ) && filename.Equals( sFile )  ) )
            {
              xub_StrLen rIdx = 0;
              sTYP = sLine.GetToken( 3, '\t', rIdx );
              sGID = sLine.GetToken( 0, '\t', rIdx ); // 4
              sLID = sLine.GetToken( 0, '\t', rIdx ); // 5
              sPFO = sLine.GetToken( 1, '\t', rIdx ); // 7
              sPFO = sHACK;
              nLANG = sLine.GetToken( 1, '\t', rIdx ); // 9
              sTEXT = sLine.GetToken( 0, '\t', rIdx ); // 10

              sQHTEXT = sLine.GetToken( 1, '\t', rIdx ); // 12
              sTITLE = sLine.GetToken( 0, '\t', rIdx );  // 13

                nLANG.EraseLeadingAndTrailingChars();

#ifdef MERGE_SOURCE_LANGUAGES
                if( true ){
#else
                if (  !nLANG.EqualsIgnoreCaseAscii("en-US")  ){
#endif
                  ByteStringHashMap::const_iterator lit;
                  lit = aLanguageMap.find (nLANG);
                  ByteString aLANG;
                  if (lit == aLanguageMap.end()) {
                    aLANG = nLANG;
                    aLanguageMap.insert( ByteStringHashMap::value_type( aLANG, aLANG ) );
                        // Remember read languages for -l all switch
                    aLanguageList.push_back( nLANG );
                  } else
                    aLANG = lit->first;

                  InsertEntry( sTYP, sGID, sLID, sPFO, aLANG, sTEXT, sQHTEXT, sTITLE , filename , bCaseSensitive );
                }
            }
        }
        else if ( nToks == 10 ) {
            printf("ERROR: File format is obsolete and no longer supported!\n");
        }
    }
    aInputStream.Close();
}
/*****************************************************************************/
MergeDataFile::~MergeDataFile()
/*****************************************************************************/
{
}

ByteString MergeDataFile::Dump(){
    ByteString sRet( "MergeDataFile\n" );

      //sRet.Append( Export::DumpMap( "aLanguageSet" , aLanguageSet ) );
    //sRet.Append( Export::DumpMap( "aLanguageList" , aLanguageList ) );
    printf("MergeDataFile\n");
    MergeDataHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg ){
        /*sRet.Append( "aMap[" );
        sRet.Append( idbg->first );
           sRet.Append( "]= " );
        sRet.Append( ((MergeData*) (idbg->second))->Dump() );
        sRet.Append("\n");*/

        printf("aMap[ %s ] = ",idbg->first.GetBuffer());
        ((MergeData*) (idbg->second))->Dump();
        printf("\n");
    }
    printf("\n");
    //sRet.Append("\n");
    return sRet;
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
    return aLanguageList;
}

/*****************************************************************************/
MergeData *MergeDataFile::GetMergeData( ResData *pResData , bool bCaseSensitive )
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
    //printf("MergeData:: Search gid=%s lid=%s filename=%s \n", pResData->sGId.GetBuffer(),pResData->sId.GetBuffer(),pResData->sFilename.GetBuffer()  );
    ByteString sKey = CreateKey( pResData->sResTyp , pResData->sGId , pResData->sId , pResData->sFilename , bCaseSensitive );

    //printf("DBG: Searching [%s]\n",sKey.GetBuffer());
    if( aMap.find( sKey ) != aMap.end() ){
        pResData->sGId = sOldG;
        pResData->sId = sOldL;
        //printf("DBG: Found[%s]\n",sKey.GetBuffer());
        return aMap[ sKey ];
    }
    pResData->sGId = sOldG;
    pResData->sId = sOldL;
    //printf("DBG: Found[%s]\n",sKey.GetBuffer());
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
PFormEntrys *MergeDataFile::GetPFormEntrysCaseSensitive( ResData *pResData )
/*****************************************************************************/
{
    // search for requested PFormEntrys
    MergeData *pData = GetMergeData( pResData , true );
    if ( pData )
        return pData->GetPFormEntrys( pResData );
    return NULL;
}
/*****************************************************************************/
void MergeDataFile::InsertEntry(
                    const ByteString &rTYP, const ByteString &rGID,
                    const ByteString &rLID, const ByteString &rPFO,
                    const ByteString &nLANG, const ByteString &rTEXT,
                    const ByteString &rQHTEXT, const ByteString &rTITLE ,
                    const ByteString &rInFilename , bool bCaseSensitive
                    )
/*****************************************************************************/
{
    MergeData *pData;

    // uniquify the filename to save memory.
    ByteStringHashMap::const_iterator fit = aFilenames.find (rInFilename);
    ByteString aFilename;
    if (fit == aFilenames.end()) {
        aFilename = rInFilename;
        aFilenames.insert (ByteStringHashMap::value_type (aFilename, aFilename));
    } else
        aFilename = fit->first;

    // search for MergeData

    ByteString sKey = CreateKey( rTYP , rGID , rLID , aFilename , bCaseSensitive );
    MergeDataHashMap::const_iterator mit;
    mit = aMap.find( sKey );
    if( mit != aMap.end() ){
        pData = mit->second;
    }else{
        pData = new MergeData( rTYP, rGID, rLID, aFilename );
        aMap.insert( MergeDataHashMap::value_type( sKey, pData ) );
    }

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

    //printf("DBG: MergeDataFile::Insert[]=( sKey=%s,nLang=%s,rTEXT=%s)\n",sKey2.GetBuffer(),nLANG.GetBuffer(),rTEXT.GetBuffer());
}
ByteString MergeDataFile::CreateKey( const ByteString& rTYP , const ByteString& rGID , const ByteString& rLID , const ByteString& rFilename , bool bCaseSensitive ){

    ByteString sKey( rTYP );
    sKey.Append( '-'        );
    sKey.Append( rGID       );
    sKey.Append( '-'        );
    sKey.Append( rLID       );
    sKey.Append( '-'        );
    sKey.Append( rFilename  );

    if( bCaseSensitive ) return sKey;         // officecfg case sensitive identifier
    else return sKey.ToUpperAscii();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
