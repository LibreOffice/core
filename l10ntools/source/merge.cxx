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

namespace
{
    static ::rtl::OString lcl_NormalizeFilename(const ::rtl::OString& rFilename)
    {
        return rFilename.copy(
            ::std::max(
                rFilename.lastIndexOf( "\\" ),
                rFilename.lastIndexOf( "/" ))+1);
    };
}

extern void ConvertHalfwitdhToFullwidth( String& rString );

//
// class PFormEntrys
//

ByteString PFormEntrys::Dump()
{
    ByteString sRet( "PFormEntrys\n" );
    ByteString a("sText");
    if(sText.size())
        Export::DumpMap(a , sText);
    return sRet;
}

sal_Bool PFormEntrys::GetTransex3Text( ByteString &rReturn,
    sal_uInt16 nTyp, const ByteString &nLangIndex, sal_Bool bDel )
{
    sal_Bool rc = GetText( rReturn , nTyp , nLangIndex , bDel );
    ByteString test( rReturn );
    for( sal_uInt16 idx = 0; idx < rReturn.Len(); idx++ )
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
sal_Bool PFormEntrys::GetText( ByteString &rReturn,
    sal_uInt16 nTyp, const ByteString &nLangIndex, sal_Bool bDel )
{

    sal_Bool bReturn=sal_True;
    switch ( nTyp ) {
        case STRING_TYP_TEXT :
            rReturn = sText[ nLangIndex ];
            if ( bDel )
                sText[ nLangIndex ] = "";
            bReturn = bTextFirst[ nLangIndex ];
            bTextFirst[ nLangIndex ] = sal_False;
            break;
        case STRING_TYP_HELPTEXT :
            rReturn = sHelpText;
            break;
        case STRING_TYP_QUICKHELPTEXT :
            rReturn = sQuickHelpText[ nLangIndex ];
            if ( bDel )
                sQuickHelpText[ nLangIndex ] = "";
            bReturn = bQuickHelpTextFirst[ nLangIndex ];
            bQuickHelpTextFirst[ nLangIndex ] = sal_False;
            break;
        case STRING_TYP_TITLE :
            rReturn = sTitle[ nLangIndex ];
            if ( bDel )
                sTitle[ nLangIndex ] = "";
            bReturn = bTitleFirst[ nLangIndex ];
            bTitleFirst[ nLangIndex ] = sal_False;
            break;
    }
    return bReturn;
}


//
// class MergeData
//

MergeData::~MergeData()
{
}

PFormEntrys* MergeData::GetPFormEntrys(ResData*)
{
    if( aMap.find( ByteString("HACK") ) != aMap.end() )
        return aMap[ ByteString("HACK") ];
    return NULL;
}

void MergeData::Insert(const ByteString&, PFormEntrys* pfEntrys )
{
    aMap.insert( PFormEntrysHashMap::value_type( ByteString("HACK") , pfEntrys ) );
}

ByteString MergeData::Dump(){
    ByteString sRet( "MergeData\n" );

    printf("MergeData sTyp = %s , sGid = %s , sLid =%s , sFilename = %s\n",sTyp.GetBuffer(),sGID.GetBuffer(),sLID.GetBuffer(), sFilename.GetBuffer() );

    PFormEntrysHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg )
    {
        printf("aMap[ %s ] = " ,idbg->first.GetBuffer());
        ( (PFormEntrys*)(idbg->second) )->Dump();
        printf("\n");
    }
    printf("\n");
    return sRet;
}

PFormEntrys* MergeData::GetPFObject( const ByteString& rPFO ){
    if( aMap.find( ByteString("HACK") ) != aMap.end() )
        return aMap[ rPFO ];
    return NULL;
}


PFormEntrys *MergeData::InsertEntry( const ByteString &rPForm )
{
    PFormEntrys* pFEntrys = new PFormEntrys( rPForm );
    aMap.insert( PFormEntrysHashMap::value_type( rPForm , pFEntrys ) );
    return pFEntrys;
}

sal_Bool MergeData::operator==( ResData *pData )
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


MergeDataFile::MergeDataFile(
    const ByteString &rFileName,
    const ByteString& sFile,
    sal_Bool bErrLog,
    CharSet aCharSet,
    bool bCaseSensitive)
    : bErrorLog( bErrLog )
{
    SvFileStream aInputStream( String( rFileName, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
    aInputStream.SetStreamCharSet( aCharSet );
    ByteString sLine;
    const ByteString sHACK("HACK");
    const ::rtl::OString sFileNormalized(lcl_NormalizeFilename(sFile));
    const bool isFileEmpty = sFileNormalized.getLength();

    if( !aInputStream.IsOpen() )
    {
        printf("Warning : Can't open %s\n", rFileName.GetBuffer());
        return;
    }
    while ( !aInputStream.IsEof())
    {
        xub_StrLen nToks;
        aInputStream.ReadLine( sLine );
        sLine = sLine.Convert( RTL_TEXTENCODING_MS_1252, aCharSet );

        nToks = sLine.GetTokenCount( '\t' );
        if ( nToks == 15 )
        {
            // Skip all wrong filenames
            const ::rtl::OString filename = lcl_NormalizeFilename(sLine.GetToken( 1 , '\t' ));
            if(isFileEmpty || sFileNormalized.equals("") || (!isFileEmpty && filename.equals(sFileNormalized)) )
            {
                xub_StrLen rIdx = 0;
                const ByteString sTYP = sLine.GetToken( 3, '\t', rIdx );
                const ByteString sGID = sLine.GetToken( 0, '\t', rIdx ); // 4
                const ByteString sLID = sLine.GetToken( 0, '\t', rIdx ); // 5
                ByteString sPFO = sLine.GetToken( 1, '\t', rIdx ); // 7
                sPFO = sHACK;
                ByteString nLANG = sLine.GetToken( 1, '\t', rIdx ); // 9
                nLANG.EraseLeadingAndTrailingChars();
                const ByteString sTEXT = sLine.GetToken( 0, '\t', rIdx ); // 10
                const ByteString sQHTEXT = sLine.GetToken( 1, '\t', rIdx ); // 12
                const ByteString sTITLE = sLine.GetToken( 0, '\t', rIdx );  // 13


#ifdef MERGE_SOURCE_LANGUAGES
                if( true )
#else
                if( !nLANG.EqualsIgnoreCaseAscii("en-US") )
#endif
                {
                    aLanguageSet.insert(nLANG);
                    InsertEntry( sTYP, sGID, sLID, sPFO, nLANG, sTEXT, sQHTEXT, sTITLE, filename, bCaseSensitive );
                }
            }
        }
        else if ( nToks == 10 )
        {
            printf("ERROR: File format is obsolete and no longer supported!\n");
        }
    }
    aInputStream.Close();
}

MergeDataFile::~MergeDataFile()
{
}

ByteString MergeDataFile::Dump(){
    ByteString sRet( "MergeDataFile\n" );

    printf("MergeDataFile\n");
    MergeDataHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg )
    {
        printf("aMap[ %s ] = ",idbg->first.GetBuffer());
        ((MergeData*) (idbg->second))->Dump();
        printf("\n");
    }
    printf("\n");
    return sRet;
}

void MergeDataFile::WriteError( const ByteString &rLine )
{
    if ( bErrorLog )
    {
        if ( !aErrLog.IsOpen())
            aErrLog.Open( String( sErrorLog, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
        aErrLog.WriteLine( rLine );
    }
    else
        fprintf( stderr, "%s\n", rLine.GetBuffer());
}

std::vector<ByteString> MergeDataFile::GetLanguages(){
    return std::vector<ByteString>(aLanguageSet.begin(),aLanguageSet.end());
}

MergeData *MergeDataFile::GetMergeData( ResData *pResData , bool bCaseSensitive )
{
    ByteString sOldG = pResData->sGId;
    ByteString sOldL = pResData->sId;
    ByteString sGID = pResData->sGId;
    ByteString sLID;
    if(!sGID.Len())
        sGID = pResData->sId;
    else
        sLID = pResData->sId;
    pResData->sGId = sGID;
    pResData->sId = sLID;

    ByteString sKey = CreateKey( pResData->sResTyp , pResData->sGId , pResData->sId , pResData->sFilename , bCaseSensitive );

    if(aMap.find( sKey ) != aMap.end())
    {
        pResData->sGId = sOldG;
        pResData->sId = sOldL;
        return aMap[ sKey ];
    }
    pResData->sGId = sOldG;
    pResData->sId = sOldL;
    return NULL;
}


PFormEntrys *MergeDataFile::GetPFormEntrys( ResData *pResData )
{
    // search for requested PFormEntrys
    MergeData *pData = GetMergeData( pResData );
    if ( pData )
        return pData->GetPFormEntrys( pResData );
    return NULL;
}

PFormEntrys *MergeDataFile::GetPFormEntrysCaseSensitive( ResData *pResData )
{
    // search for requested PFormEntrys
    MergeData *pData = GetMergeData( pResData , true );
    if ( pData )
        return pData->GetPFormEntrys( pResData );
    return NULL;
}

void MergeDataFile::InsertEntry(
    const ByteString &rTYP, const ByteString &rGID,
    const ByteString &rLID, const ByteString &rPFO,
    const ByteString &nLANG, const ByteString &rTEXT,
    const ByteString &rQHTEXT, const ByteString &rTITLE ,
    const ByteString &rInFilename , bool bCaseSensitive
    )
{
    MergeData *pData;

    // search for MergeData
    ByteString sKey = CreateKey( rTYP , rGID , rLID , rInFilename , bCaseSensitive );
    MergeDataHashMap::const_iterator mit;
    mit = aMap.find( sKey );
    if( mit != aMap.end() )
    {
        pData = mit->second;
    }
    else
    {
        pData = new MergeData( rTYP, rGID, rLID, rInFilename );
        aMap.insert( MergeDataHashMap::value_type( sKey, pData ) );
    }

    PFormEntrys *pFEntrys = 0;

    // search for PFormEntrys
    pFEntrys = pData->GetPFObject( rPFO );
    if( !pFEntrys )
    {
        // create new PFormEntrys, cause no one exists with current properties
        pFEntrys = new PFormEntrys( rPFO );
        pData->Insert( rPFO , pFEntrys );
    }

    // finaly insert the cur string
    pFEntrys->InsertEntry( nLANG , rTEXT, rQHTEXT, rTITLE );
}

ByteString MergeDataFile::CreateKey( const ByteString& rTYP , const ByteString& rGID , const ByteString& rLID , const ByteString& rFilename , bool bCaseSensitive )
{
    static const ::rtl::OString sStroke('-');
    ::rtl::OString sKey( rTYP );
    sKey += sStroke;
    sKey += rGID;
    sKey += sStroke;
    sKey += rLID;
    sKey += sStroke;
    sKey += lcl_NormalizeFilename(rFilename);
    OSL_TRACE("created key: %s", sKey.getStr());
    if(bCaseSensitive)
        return sKey;         // officecfg case sensitive identifier
    return sKey.toAsciiUpperCase();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */