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

#include <sal/config.h>
#include <sal/log.hxx>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <export.hxx>
#include <po.hxx>

namespace
{
    OString lcl_NormalizeFilename(std::string_view rFilename)
    {
        size_t idx1 = rFilename.rfind( '\\' );
        size_t idx2 = rFilename.rfind( '/' );
        if (idx1 == std::string_view::npos && idx2 == std::string_view::npos)
            return OString(rFilename);
        if (idx1 == std::string_view::npos)
            idx1 = 0;
        if (idx2 == std::string_view::npos)
            idx2 = 0;
        return OString(rFilename.substr(std::max(idx1, idx2)+1));
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
            SAL_WARN("l10ntools", rFileName << " contains invalid entry");
            return false;
        }
        return true;
    }
}




ResData::ResData( OString _sGId )
    :
    sGId(std::move( _sGId ))
{
    sGId = sGId.replaceAll("\r", OString());
}

ResData::ResData( OString _sGId, OString _sFilename)
    :
    sGId(std::move( _sGId )),
    sFilename(std::move( _sFilename ))
{
    sGId = sGId.replaceAll("\r", OString());
}




bool MergeEntrys::GetText( OString &rReturn,
    const OString &nLangIndex, bool bDel )
{
    bool bReturn = true;
    rReturn = sText[ nLangIndex ];
    if ( bDel )
        sText[ nLangIndex ] = "";
    bReturn = bTextFirst[ nLangIndex ];
    bTextFirst[ nLangIndex ] = false;
    return bReturn;
}

namespace
{
    OString GetDoubleBars()
    {
        //DOUBLE VERTICAL LINE instead of || because the translations make their
        //way into action_names under gtk3 where || is illegal
        return u8"\u2016";
    }
}

OString MergeEntrys::GetQTZText(const ResData& rResData, std::string_view rOrigText)
{
    const OString sFilename = rResData.sFilename.copy(rResData.sFilename.lastIndexOf('/')+1);
    const OString sKey =
        PoEntry::genKeyId(sFilename + rResData.sGId + rResData.sId + rResData.sResTyp + rOrigText);
    return sKey + GetDoubleBars() + rOrigText;
}



MergeDataFile::MergeDataFile(
    const OString &rFileName, std::string_view rFile,
    bool bCaseSensitive, bool bWithQtz )
{
    auto const env = getenv("ENABLE_RELEASE_BUILD");
    OString sEnableReleaseBuild(env == nullptr ? "" : env);

    std::ifstream aInputStream( rFileName.getStr() );
    if ( !aInputStream.is_open() )
    {
        SAL_WARN("l10ntools", "Can't open po path container file for " << rFileName);
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
        const OString sPoFileName(sPoFile.data(), static_cast<sal_Int32>(sPoFile.length()));
        PoIfstream aPoInput;
        aPoInput.open( sPoFileName );
        if ( !aPoInput.isOpen() )
        {
            SAL_WARN("l10ntools", "Can't open file: " << sPoFileName);
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
                if (aPoInput.eof())
                    break;
                bInSameComp = PoEntry::IsInSameComp(aActPo, aNextPo);
            } while( bInSameComp );

            InsertEntry(
                aActPo.getResourceType(), aActPo.getGroupId(),
                aActPo.getLocalId(), sLang, sText,
                sQHText, sTitle, aActPo.getSourceFile(),
                bFirstLang, bCaseSensitive );

            if( bFirstLang && bWithQtz &&
                sEnableReleaseBuild != "TRUE" )
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
}

std::vector<OString> MergeDataFile::GetLanguages() const
{
    return std::vector<OString>(aLanguageSet.begin(),aLanguageSet.end());
}

MergeEntrys *MergeDataFile::GetMergeData( ResData *pResData , bool bCaseSensitive )
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

    auto mit = aMap.find( sKey );
    if(mit != aMap.end())
    {
        pResData->sGId = sOldG;
        pResData->sId = sOldL;
        return mit->second.get();
    }
    pResData->sGId = sOldG;
    pResData->sId = sOldL;
    return nullptr;
}

MergeEntrys *MergeDataFile::GetMergeEntrys( ResData *pResData )
{
    // search for requested MergeEntrys
    return GetMergeData( pResData );
}

MergeEntrys *MergeDataFile::GetMergeEntrysCaseSensitive( ResData *pResData )
{
    // search for requested MergeEntrys
    return GetMergeData( pResData , true );
}

void MergeDataFile::InsertEntry(
    std::string_view rTYP, std::string_view rGID,
    std::string_view rLID, const OString &nLANG,
    const OString &rTEXT, const OString &rQHTEXT,
    const OString &rTITLE, std::string_view rInFilename,
    bool bFirstLang, bool bCaseSensitive )
{
    MergeEntrys *pMergeEntrys = nullptr;

    // search for MergeData
    OString sKey = CreateKey(rTYP , rGID , rLID , rInFilename , bCaseSensitive);

    if( !bFirstLang )
    {
        auto mit = aMap.find( sKey );
        if(mit != aMap.end())
            pMergeEntrys = mit->second.get();

    }

    if( !pMergeEntrys )
    {
        pMergeEntrys = new MergeEntrys;
        if (!aMap.emplace( sKey, std::unique_ptr<MergeEntrys>(pMergeEntrys) ).second)
        {
            std::cerr << "Duplicate entry " << sKey << "\n";
            std::exit(EXIT_FAILURE);
        }
    }


    // insert the cur string
    if( nLANG =="qtz" )
    {
        const OString sTemp = OString::Concat(rInFilename) + rGID + rLID + rTYP;
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

OString MergeDataFile::CreateKey(std::string_view rTYP, std::string_view rGID,
    std::string_view rLID, std::string_view rFilename, bool bCaseSensitive)
{
    static const char sStroke[] = "-";
    OString sKey = OString::Concat(rTYP) + sStroke + rGID + sStroke + rLID + sStroke +
        lcl_NormalizeFilename(rFilename);
    if(bCaseSensitive)
        return sKey;         // officecfg case sensitive identifier
    return sKey.toAsciiUpperCase();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
