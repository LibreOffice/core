/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "export.hxx"
#include "po.hxx"

namespace
{
    static sal_Int32 lcl_BasenameIndex(const OString& rFilename)
    {
        sal_Int32 index;
        for(index = rFilename.getLength() - 1; index >= 0 ; --index)
        {
            if(rFilename[index] == '/' || rFilename[index] == '\\')
            {
                break;
            }
        }
        return index + 1;
    }

    static bool lcl_ReadPoChecked(
        PoEntry& o_rPoEntry, PoIfstream& rPoFile,
        const OString& rFileName)
    {
        try
        {
            rPoFile.readEntry( o_rPoEntry );
        }
        catch( PoIfstream::Exception& aException )
        {
            if( aException == PoIfstream::INVALIDENTRY )
            {
                printf(
                    "Warning : %s contains invalid entry\n",
                    rFileName.getStr() );
                return false;
            }
        }
        return true;
    }
}

//
// class PFormEntrys
//

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

MergeDataFile::MergeDataFile(
    const rtl::OString &rFileName, const rtl::OString &rFile,
    bool bCaseSensitive, bool bWithQtz )
{
    std::ifstream aInputStream( rFileName.getStr() );
    if ( !aInputStream.is_open() )
    {
        printf("Warning : Can't open po path container file\n");
        return;
    }
    std::string sPoFile;
    aInputStream >> sPoFile;
    bool bFirstLang = true;
    while( !aInputStream.eof() )
    {
        const OString sHack("HACK");
        const OString sFileName( rFile.getStr() + lcl_BasenameIndex(rFile) );
        const bool bReadAll = sFileName.isEmpty();
        const OString sPoFileName(sPoFile.data(), sPoFile.length());
        PoIfstream aPoInput;
        aPoInput.open( sPoFileName );
        if ( !aPoInput.isOpen() )
        {
            printf( "Warning : Can't open %s\n", sPoFileName.getStr() );
            return;
        }
        PoHeader aPoHeader;
        try
        {
            aPoInput.readHeader( aPoHeader );
        }
        catch( PoIfstream::Exception& aException )
        {
            if( aException == PoIfstream::INVALIDHEADER )
            {
                printf(
                    "Warning : %s has invalid header\n",
                    sPoFileName.getStr() );
                return;
            }
        }

        OString sLang;
        //Get language id from path
        {
            const OString sTransSource("translations/source/");
            const sal_Int32 nStart =
                sPoFileName.indexOf(sTransSource)+sTransSource.getLength();
            const sal_Int32 nCount =
                sPoFileName.indexOf("/",nStart) - nStart;
            sLang = sPoFileName.copy(nStart,nCount);
        }
        aLanguageSet.insert( sLang );
        PoEntry aNextPo;
        do
        {
            if( !lcl_ReadPoChecked(aNextPo, aPoInput, sPoFileName) )
            {
                return;
            }
        } while( !aPoInput.eof() && aNextPo.getSourceFile() != sFileName && !bReadAll );
        while( !aPoInput.eof() && (aNextPo.getSourceFile() == sFileName || bReadAll ))
        {
            PoEntry aActPo( aNextPo );

            bool bInSameComp = false;
            OString sText;
            OString sQHText;
            OString sTitle;
            OString sExText;
            OString sExQHText;
            OString sExTitle;
            OString sQTZText;
            OString sQTZQHText;
            OString sQTZTitle;
            do
            {
                if( bInSameComp )
                    aActPo = aNextPo;
                OString sTemp = aActPo.getMsgStr();
                if( aActPo.isFuzzy() || sTemp.isEmpty() )
                    sTemp = aActPo.getMsgId();
                switch( aActPo.getType() )
                {
                    case PoEntry::TTEXT:
                        sText = sTemp;
                        sExText = aActPo.getMsgId();
                        sQTZText = aActPo.getKeyId();
                        break;
                    case PoEntry::TQUICKHELPTEXT:
                        sQHText = sTemp;
                        sExQHText = aActPo.getMsgId();
                        sQTZQHText = aActPo.getKeyId();
                        break;
                    case PoEntry::TTITLE:
                        sTitle = sTemp;
                        sExTitle = aActPo.getMsgId();
                        sQTZTitle = aActPo.getKeyId();
                        break;
                }
                if( !lcl_ReadPoChecked(aNextPo, aPoInput, sPoFileName) )
                {
                    return;
                }
            } while( !aPoInput.eof() &&
                ( bInSameComp = PoEntry::IsInSameComp(aActPo, aNextPo) ) );

            InsertEntry(
                aActPo.getResourceType(), aActPo.getGroupId(),
                aActPo.getLocalId(), sHack, sLang, sText,
                sQHText, sTitle, aActPo.getSourceFile(), bCaseSensitive );

            if( bFirstLang && bWithQtz &&
                ( strcmp(getenv("ENABLE_RELEASE_BUILD"),"TRUE") ) )
            {
                aLanguageSet.insert("qtz");
                InsertEntry(
                    aActPo.getResourceType(), aActPo.getGroupId(),
                    aActPo.getLocalId(), sHack, "qtz",
                    sQTZText + "||" + sExText, sQTZQHText + "||" + sExQHText,
                    sQTZTitle + "||" + sExTitle, aActPo.getSourceFile(),
                    bCaseSensitive );
            }
        }
        aPoInput.close();
        aInputStream >> sPoFile;
        bFirstLang = false;
    }
    aInputStream.close();
}

MergeDataFile::~MergeDataFile()
{
    for (MergeDataHashMap::iterator aI = aMap.begin(), aEnd = aMap.end(); aI != aEnd; ++aI)
        delete aI->second;
}

std::vector<rtl::OString> MergeDataFile::GetLanguages() const
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
    ::rtl::OString sKey = rTYP + "-" + rGID + "-" + rLID + "-" + (rFilename.getStr() + lcl_BasenameIndex(rFilename) );
    OSL_TRACE("created key: %s", sKey.getStr());
    if(bCaseSensitive)
        return sKey;         // officecfg case sensitive identifier
    return sKey.toAsciiUpperCase();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
