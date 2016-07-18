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
    OString lcl_NormalizeFilename(const OString& rFilename)
    {
        return rFilename.copy(
            std::max(
                rFilename.lastIndexOf( '\\' ),
                rFilename.lastIndexOf( '/' ))+1);
    };

    bool lcl_ReadPoChecked(
        PoEntry& o_rPoEntry, PoIfstream& rPoFile,
        const OString& rFileName)
    {
        try
        {
            rPoFile.readEntry( o_rPoEntry );
        }
        catch (const PoIfstream::Exception&)
        {
            SAL_WARN("l10ntools", rFileName.getStr() << " contains invalid entry\n");
            return false;
        }
        return true;
    }
}


//  class ResData


ResData::ResData( const OString &rGId )
    :
    nIdLevel( IdLevel::Null ),
    bChild( false ),
    bChildWithText( false ),
    bText( false ),
    bQuickHelpText( false ),
    bTitle( false ),
    sGId( rGId ),
    sTextTyp( "Text" )
{
    sGId = sGId.replaceAll("\r", OString());
}

ResData::ResData( const OString &rGId, const OString &rFilename)
    :
    nIdLevel( IdLevel::Null ),
    bChild( false ),
    bChildWithText( false ),
    bText( false ),
    bQuickHelpText( false ),
    bTitle( false ),
    sGId( rGId ),
    sFilename( rFilename ),
    sTextTyp( "Text" )
{
    sGId = sGId.replaceAll("\r", OString());
}


// class MergeEntrys


bool MergeEntrys::GetText( OString &rReturn,
    sal_uInt16 nTyp, const OString &nLangIndex, bool bDel )
{
    bool bReturn = true;
    switch ( nTyp ) {
        case STRING_TYP_TEXT :
            rReturn = sText[ nLangIndex ];
            if ( bDel )
                sText[ nLangIndex ] = "";
            bReturn = bTextFirst[ nLangIndex ];
            bTextFirst[ nLangIndex ] = false;
            break;
        case STRING_TYP_QUICKHELPTEXT :
            rReturn = sQuickHelpText[ nLangIndex ];
            if ( bDel )
                sQuickHelpText[ nLangIndex ] = "";
            bReturn = bQuickHelpTextFirst[ nLangIndex ];
            bQuickHelpTextFirst[ nLangIndex ] = false;
            break;
        case STRING_TYP_TITLE :
            rReturn = sTitle[ nLangIndex ];
            if ( bDel )
                sTitle[ nLangIndex ] = "";
            bReturn = bTitleFirst[ nLangIndex ];
            bTitleFirst[ nLangIndex ] = false;
            break;
    }
    return bReturn;
}

namespace
{
    OString GetDoubleBars()
    {
        //DOUBLE VERTICAL LINE instead of || because the translations make their
        //way into action_names under gtk3 where || is illegal
        return OUStringToOString(OUString(static_cast<sal_Unicode>(0x2016)), RTL_TEXTENCODING_UTF8);
    }
}

OString MergeEntrys::GetQTZText(const ResData& rResData, const OString& rOrigText)
{
    const OString sFilename = rResData.sFilename.copy(rResData.sFilename.lastIndexOf('/')+1);
    const OString sKey =
        PoEntry::genKeyId(sFilename + rResData.sGId + rResData.sId + rResData.sResTyp + rOrigText);
    return sKey + GetDoubleBars() + rOrigText;
}


// class MergeDataHashMap


std::pair<MergeDataHashMap::iterator,bool> MergeDataHashMap::insert(const OString& rKey, MergeData* pMergeData)
{
    std::pair<iterator,bool> aTemp = m_aHashMap.insert(HashMap_t::value_type( rKey, pMergeData ));
    if( m_aHashMap.size() == 1 )
    {
        // When first insert, set an iterator to the first element
        aFirstInOrder = aTemp.first;
    }
    else
    {
        // Define insertion order by setting an iterator to the next element.
        aLastInsertion->second->m_aNextData = aTemp.first;
    }
    aLastInsertion = aTemp.first;
    return aTemp;
}

MergeDataHashMap::iterator const & MergeDataHashMap::find(const OString& rKey)
{
    iterator aHint = m_aHashMap.end();

    // Add a hint
    if( bFirstSearch && !m_aHashMap.empty() )
    {
        aHint = aFirstInOrder;
    }
    else if( aLastFound == aLastInsertion )
    {
        // Next to the last element is the first element
        aHint = aFirstInOrder;
    }
    else if( aLastFound != m_aHashMap.end() && aLastFound != aLastInsertion )
    {
        aHint = aLastFound->second->m_aNextData;
    }

    // If hint works than no need for search
    if( aHint != m_aHashMap.end() && aHint->first == rKey )
    {
        aLastFound = aHint;
    }
    else
    {
        aLastFound = m_aHashMap.find(rKey);
    }

    bFirstSearch = false;
    return aLastFound;
}


// class MergeData


MergeData::MergeData(const OString &rGID,
    const OString &rLID )
    : sGID( rGID ),
    sLID( rLID ) ,
    pMergeEntrys( new MergeEntrys() )
{
}

MergeData::~MergeData()
{
    delete pMergeEntrys;
}


// class MergeDataFile


MergeDataFile::MergeDataFile(
    const OString &rFileName, const OString &rFile,
    bool bCaseSensitive, bool bWithQtz )
{
    OString sEnableReleaseBuild(getenv("ENABLE_RELEASE_BUILD"));

    std::ifstream aInputStream( rFileName.getStr() );
    if ( !aInputStream.is_open() )
    {
        SAL_WARN("l10ntools", "Can't open po path container file for " << rFileName.getStr());
        return;
    }
    std::string sPoFile;
    aInputStream >> sPoFile;
    bool bFirstLang = true;
    while( !aInputStream.eof() )
    {
        bool bSkipCurrentPOFile = false;
        const OString sFileName( lcl_NormalizeFilename(rFile) );
        const bool bReadAll = sFileName.isEmpty();
        // coverity[tainted_data] - this is a build time tool
        const OString sPoFileName(sPoFile.data(), (sal_Int32)sPoFile.length());
        PoIfstream aPoInput;
        aPoInput.open( sPoFileName );
        if ( !aPoInput.isOpen() )
        {
            SAL_WARN("l10ntools", "Can't open file: " << sPoFileName.getStr());
            return;
        }

        OString sLang;
        //Get language id from path
        {
            const OString sTransSource("translations/source/");
            const sal_Int32 nStart =
                sPoFileName.indexOf(sTransSource)+sTransSource.getLength();
            const sal_Int32 nCount =
                sPoFileName.indexOf('/',nStart) - nStart;
            sLang = sPoFileName.copy(nStart,nCount);
        }
        aLanguageSet.insert( sLang );
        PoEntry aNextPo;
        do
        {
            if( !lcl_ReadPoChecked(aNextPo, aPoInput, sPoFileName) )
            {
                bSkipCurrentPOFile = true;
                break;
            }
        } while( !aPoInput.eof() && aNextPo.getSourceFile() != sFileName && !bReadAll );
        while( !aPoInput.eof() && (aNextPo.getSourceFile() == sFileName || bReadAll ) && !bSkipCurrentPOFile )
        {
            PoEntry aActPo( aNextPo );

            bool bInSameComp = false;
            OString sText;
            OString sQHText;
            OString sTitle;
            OString sExText;
            OString sExQHText;
            OString sExTitle;
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
                        break;
                    case PoEntry::TQUICKHELPTEXT:
                        sQHText = sTemp;
                        sExQHText = aActPo.getMsgId();
                        break;
                    case PoEntry::TTITLE:
                        sTitle = sTemp;
                        sExTitle = aActPo.getMsgId();
                        break;
                }
                if( !lcl_ReadPoChecked(aNextPo, aPoInput, sPoFileName) )
                {
                    bSkipCurrentPOFile = true;
                    break;
                }
            } while( !aPoInput.eof() &&
                ( bInSameComp = PoEntry::IsInSameComp(aActPo, aNextPo) ) );

            InsertEntry(
                aActPo.getResourceType(), aActPo.getGroupId(),
                aActPo.getLocalId(), sLang, sText,
                sQHText, sTitle, aActPo.getSourceFile(),
                bFirstLang, bCaseSensitive );

            if( bFirstLang && bWithQtz &&
                !sEnableReleaseBuild.equals("TRUE") )
            {
                aLanguageSet.insert("qtz");
                InsertEntry(
                    aActPo.getResourceType(), aActPo.getGroupId(),
                    aActPo.getLocalId(), "qtz",
                    sExText, sExQHText,
                    sExTitle, aActPo.getSourceFile(),
                    false, bCaseSensitive );
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

std::vector<OString> MergeDataFile::GetLanguages() const
{
    return std::vector<OString>(aLanguageSet.begin(),aLanguageSet.end());
}

MergeData *MergeDataFile::GetMergeData( ResData *pResData , bool bCaseSensitive )
{
    OString sOldG = pResData->sGId;
    OString sOldL = pResData->sId;
    OString sGID = pResData->sGId;
    OString sLID;
    if (sGID.isEmpty())
        sGID = pResData->sId;
    else
        sLID = pResData->sId;
    pResData->sGId = sGID;
    pResData->sId = sLID;

    OString sKey = CreateKey( pResData->sResTyp , pResData->sGId , pResData->sId , pResData->sFilename , bCaseSensitive );

    MergeDataHashMap::const_iterator mit = aMap.find( sKey );
    if(mit != aMap.end())
    {
        pResData->sGId = sOldG;
        pResData->sId = sOldL;
        return mit->second;
    }
    pResData->sGId = sOldG;
    pResData->sId = sOldL;
    return nullptr;
}

MergeEntrys *MergeDataFile::GetMergeEntrys( ResData *pResData )
{
    // search for requested MergeEntrys
    MergeData *pData = GetMergeData( pResData );
    if ( pData )
        return pData->GetMergeEntries();
    return nullptr;
}

MergeEntrys *MergeDataFile::GetMergeEntrysCaseSensitive( ResData *pResData )
{
    // search for requested MergeEntrys
    MergeData *pData = GetMergeData( pResData , true );
    if ( pData )
        return pData->GetMergeEntries();
    return nullptr;
}

void MergeDataFile::InsertEntry(
    const OString &rTYP, const OString &rGID,
    const OString &rLID, const OString &nLANG,
    const OString &rTEXT, const OString &rQHTEXT,
    const OString &rTITLE, const OString &rInFilename,
    bool bFirstLang, bool bCaseSensitive )
{
    MergeData *pData = nullptr;

    // search for MergeData
    OString sKey = CreateKey(rTYP , rGID , rLID , rInFilename , bCaseSensitive);

    if( !bFirstLang )
    {
        MergeDataHashMap::const_iterator mit = aMap.find( sKey );
        if(mit != aMap.end())
            pData = mit->second;

    }

    if( !pData )
    {
        pData = new MergeData( rGID, rLID );
        aMap.insert( sKey, pData );
    }


    // insert the cur string
    MergeEntrys *pMergeEntrys = pData->GetMergeEntries();
    if( nLANG =="qtz" )
    {
        const OString sTemp = rInFilename + rGID + rLID + rTYP;
        pMergeEntrys->InsertEntry(
            nLANG,
            rTEXT.isEmpty()? rTEXT : PoEntry::genKeyId(sTemp + rTEXT) + GetDoubleBars() + rTEXT,
            rQHTEXT.isEmpty()? rQHTEXT : PoEntry::genKeyId(sTemp + rQHTEXT) + GetDoubleBars() + rQHTEXT,
            rTITLE.isEmpty()? rTITLE : PoEntry::genKeyId(sTemp + rTITLE) + GetDoubleBars() + rTITLE );
    }
    else
    {
        pMergeEntrys->InsertEntry( nLANG , rTEXT, rQHTEXT, rTITLE );
    }
}

OString MergeDataFile::CreateKey(const OString& rTYP, const OString& rGID,
    const OString& rLID, const OString& rFilename, bool bCaseSensitive)
{
    static const char sStroke[] = "-";
    OString sKey( rTYP );
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
