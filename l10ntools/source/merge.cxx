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

#include <stdio.h>
#include <tools/fsys.hxx>
#include <comphelper/string.hxx>
#include "export.hxx"
#include <iostream>

using namespace std;
using comphelper::string::getToken;
using comphelper::string::getTokenCount;

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

//
// class PFormEntrys
//

rtl::OString PFormEntrys::Dump()
{
    if (sText.size())
    {
        rtl::OString a(RTL_CONSTASCII_STRINGPARAM("sText"));
        Export::DumpMap(a , sText);
    }
    return rtl::OString(RTL_CONSTASCII_STRINGPARAM("PFormEntrys\n"));
}

sal_Bool PFormEntrys::GetTransex3Text( rtl::OString &rReturn,
    sal_uInt16 nTyp, const rtl::OString &nLangIndex, sal_Bool bDel )
{
    sal_Bool rc = GetText( rReturn , nTyp , nLangIndex , bDel );
    for( sal_Int32 idx = 0; idx < rReturn.getLength(); idx++ )
    {
        if( rReturn[idx] == '\"' && ( idx >= 1 )  &&  rReturn[idx-1] == '\\' )
        {
            rReturn = rReturn.replaceAt( idx-1, 1, rtl::OString() );
        }
    }
    return rc;
}
/*****************************************************************************/
sal_Bool PFormEntrys::GetText( rtl::OString &rReturn,
    sal_uInt16 nTyp, const rtl::OString &nLangIndex, sal_Bool bDel )
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

PFormEntrys* MergeData::GetPFormEntries()
{
    if( aMap.find( rtl::OString(RTL_CONSTASCII_STRINGPARAM("HACK")) ) != aMap.end() )
        return aMap[rtl::OString(RTL_CONSTASCII_STRINGPARAM("HACK"))];
    return NULL;
}

void MergeData::Insert(PFormEntrys* pfEntrys )
{
    aMap.insert( PFormEntrysHashMap::value_type( rtl::OString(RTL_CONSTASCII_STRINGPARAM("HACK")) , pfEntrys ) );
}

rtl::OString MergeData::Dump()
{
    printf("MergeData sTyp = %s , sGid = %s , sLid =%s , sFilename = %s\n",
        sTyp.getStr(), sGID.getStr(), sLID.getStr(), sFilename.getStr());

    PFormEntrysHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg )
    {
        printf("aMap[ %s ] = " ,idbg->first.getStr());
        ( (PFormEntrys*)(idbg->second) )->Dump();
        printf("\n");
    }
    printf("\n");
    return rtl::OString(RTL_CONSTASCII_STRINGPARAM("MergeData\n"));
}

PFormEntrys* MergeData::GetPFObject( const rtl::OString& rPFO )
{
    if( aMap.find( rtl::OString(RTL_CONSTASCII_STRINGPARAM("HACK")) ) != aMap.end() )
        return aMap[ rPFO ];
    return NULL;
}

sal_Bool MergeData::operator==( ResData *pData )
{
    return pData->sId == sLID && pData->sGId == sGID
        && pData->sResTyp.equalsIgnoreAsciiCase(sTyp);
}

//
// class MergeDataFile
//

#define FFORMAT_UNKNOWN 0x0000
#define FFORMAT_NEW     0x0001
#define FFORMAT_OLD     0x0002


MergeDataFile::MergeDataFile(
    const rtl::OString &rFileName,
    const rtl::OString &rFile,
    bool bErrLog,
    bool bCaseSensitive)
    : bErrorLog( bErrLog )
{
    SvFileStream aInputStream(rtl::OStringToOUString(rFileName, RTL_TEXTENCODING_ASCII_US), STREAM_STD_READ);
    aInputStream.SetStreamCharSet( RTL_TEXTENCODING_MS_1252 );
    rtl::OString sLine;
    const ::rtl::OString sHACK(RTL_CONSTASCII_STRINGPARAM("HACK"));
    const ::rtl::OString sFileNormalized(lcl_NormalizeFilename(rFile));
    const bool isFileEmpty = !sFileNormalized.isEmpty();

    if( !aInputStream.IsOpen() )
    {
        printf("Warning : Can't open %s\n", rFileName.getStr());
        return;
    }
    while ( !aInputStream.IsEof())
    {
        xub_StrLen nToks;
        aInputStream.ReadLine( sLine );
        nToks = getTokenCount(sLine, '\t');
        if ( nToks == 15 )
        {
            // Skip all wrong filenames
            const ::rtl::OString filename = lcl_NormalizeFilename(getToken(sLine, 1 , '\t'));
            if(isFileEmpty || sFileNormalized.equals("") || (!isFileEmpty && filename.equals(sFileNormalized)) )
            {
                sal_Int32 rIdx = 0;
                const rtl::OString sTYP = sLine.getToken( 3, '\t', rIdx );
                const rtl::OString sGID = sLine.getToken( 0, '\t', rIdx ); // 4
                const rtl::OString sLID = sLine.getToken( 0, '\t', rIdx ); // 5
                rtl::OString sPFO = sLine.getToken( 1, '\t', rIdx ); // 7
                sPFO = sHACK;
                rtl::OString nLANG = sLine.getToken( 1, '\t', rIdx ); // 9
                nLANG = comphelper::string::strip(nLANG, ' ');
                const rtl::OString sTEXT = sLine.getToken( 0, '\t', rIdx ); // 10
                const rtl::OString sQHTEXT = sLine.getToken( 1, '\t', rIdx ); // 12
                const rtl::OString sTITLE = sLine.getToken( 0, '\t', rIdx );  // 13


                if (!nLANG.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")))
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
    for (MergeDataHashMap::iterator aI = aMap.begin(), aEnd = aMap.end(); aI != aEnd; ++aI)
        delete aI->second;
}

rtl::OString MergeDataFile::Dump()
{
    rtl::OString sRet(RTL_CONSTASCII_STRINGPARAM("MergeDataFile\n"));

    printf("MergeDataFile\n");
    MergeDataHashMap::const_iterator idbg;
    for( idbg = aMap.begin() ; idbg != aMap.end(); ++idbg )
    {
        printf("aMap[ %s ] = ",idbg->first.getStr());
        idbg->second->Dump();
        printf("\n");
    }
    printf("\n");
    return sRet;
}

std::vector<rtl::OString> MergeDataFile::GetLanguages()
{
    return std::vector<rtl::OString>(aLanguageSet.begin(),aLanguageSet.end());
}

MergeData *MergeDataFile::GetMergeData( ResData *pResData , bool bCaseSensitive )
{
    rtl::OString sOldG = pResData->sGId;
    rtl::OString sOldL = pResData->sId;
    rtl::OString sGID = pResData->sGId;
    rtl::OString sLID;
    if (sGID.isEmpty())
        sGID = pResData->sId;
    else
        sLID = pResData->sId;
    pResData->sGId = sGID;
    pResData->sId = sLID;

    rtl::OString sKey = CreateKey( pResData->sResTyp , pResData->sGId , pResData->sId , pResData->sFilename , bCaseSensitive );

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
        return pData->GetPFormEntries();
    return NULL;
}

PFormEntrys *MergeDataFile::GetPFormEntrysCaseSensitive( ResData *pResData )
{
    // search for requested PFormEntrys
    MergeData *pData = GetMergeData( pResData , true );
    if ( pData )
        return pData->GetPFormEntries();
    return NULL;
}

void MergeDataFile::InsertEntry(
    const rtl::OString &rTYP, const rtl::OString &rGID,
    const rtl::OString &rLID, const rtl::OString &rPFO,
    const rtl::OString &nLANG, const rtl::OString &rTEXT,
    const rtl::OString &rQHTEXT, const rtl::OString &rTITLE ,
    const rtl::OString &rInFilename , bool bCaseSensitive
    )
{
    MergeData *pData;

    // search for MergeData
    rtl::OString sKey = CreateKey(rTYP , rGID , rLID , rInFilename , bCaseSensitive);
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
        pData->Insert( pFEntrys );
    }

    // finaly insert the cur string
    pFEntrys->InsertEntry( nLANG , rTEXT, rQHTEXT, rTITLE );
}

rtl::OString MergeDataFile::CreateKey(const rtl::OString& rTYP, const rtl::OString& rGID,
    const rtl::OString& rLID, const rtl::OString& rFilename, bool bCaseSensitive)
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
