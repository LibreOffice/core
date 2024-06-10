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

#include <algorithm>
#include <string_view>

#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/servicehelper.hxx>

#include <unotools/transliterationwrapper.hxx>

#include <vcl/errinf.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <svl/urihelper.hxx>
#include <svl/fstathelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/tempfile.hxx>
#include <o3tl/string_view.hxx>
#include <swtypes.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swunohelper.hxx>

#include <unoatxt.hxx>
#include <swerror.h>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

OUString lcl_FullPathName(std::u16string_view sPath, std::u16string_view sName)
{
    return OUString::Concat(sPath) + "/" + sName + SwGlossaries::GetExtension();
}

OUString lcl_CheckFileName( const OUString& rNewFilePath,
                          std::u16string_view aNewGroupName )
{
    const sal_Int32 nLen = aNewGroupName.size();
    OUStringBuffer aBuf(nLen);
    //group name should contain only A-Z and a-z and spaces
    for( sal_Int32 i=0; i < nLen; ++i )
    {
        const sal_Unicode cChar = aNewGroupName[i];
        if (rtl::isAsciiAlphanumeric(cChar) ||
            cChar == '_' || cChar == 0x20)
        {
            aBuf.append(cChar);
        }
    }

    const OUString sRet = aBuf.makeStringAndClear().trim();
    if ( !sRet.isEmpty() )
    {
        if (!FStatHelper::IsDocument( lcl_FullPathName(rNewFilePath, sRet) ))
            return sRet;
    }

    //generate generic name
    utl::TempFileNamed aTemp(u"group", true, SwGlossaries::GetExtension(), &rNewFilePath);
    aTemp.EnableKillingFile();

    INetURLObject aTempURL( aTemp.GetURL() );
    return aTempURL.GetBase();
}

}

// supplies the default group's name
OUString SwGlossaries::GetDefName()
{
    return u"standard"_ustr;

}

// supplies the number of text block groups
size_t SwGlossaries::GetGroupCnt()
{
    return GetNameList().size();
}

// supplies the group's name
bool SwGlossaries::FindGroupName(OUString& rGroup)
{
    // if the group name doesn't contain a path, a suitable group entry
    // can the searched for here;
    const size_t nCount = GetGroupCnt();
    for(size_t i = 0; i < nCount; ++i)
    {
        const OUString sTemp(GetGroupName(i));
        if (rGroup == o3tl::getToken(sTemp, 0, GLOS_DELIM))
        {
            rGroup = sTemp;
            return true;
        }
    }
    // you can search two times because for more directories the case sensitive
    // name could occur multiple times
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for(size_t i = 0; i < nCount; ++i)
    {
        const OUString sTemp( GetGroupName( i ));
        sal_uInt16 nPath = o3tl::toUInt32(o3tl::getToken(sTemp, 1, GLOS_DELIM));

        if (!SWUnoHelper::UCB_IsCaseSensitiveFileName( m_PathArr[nPath] )
             && rSCmp.isEqual( rGroup, sTemp.getToken( 0, GLOS_DELIM) ) )
        {
            rGroup = sTemp;
            return true;
        }
    }
    return false;
}

OUString const & SwGlossaries::GetGroupName(size_t nGroupId)
{
    OSL_ENSURE(nGroupId < m_GlosArr.size(),
            "SwGlossaries::GetGroupName: index out of bounds");
    return m_GlosArr[nGroupId];
}

OUString SwGlossaries::GetGroupTitle( const OUString& rGroupName )
{
    OUString sRet;
    OUString sGroup(rGroupName);
    if (sGroup.indexOf(GLOS_DELIM)<0)
        FindGroupName(sGroup);
    std::unique_ptr<SwTextBlocks> pGroup = GetGroupDoc(sGroup);
    if(pGroup)
    {
        sRet = pGroup->GetName();
    }
    return sRet;
}

// supplies the group rName's text block document
std::unique_ptr<SwTextBlocks> SwGlossaries::GetGroupDoc(const OUString &rName,
                                        bool bCreate)
{
    // insert to the list of text blocks if applicable
    if(bCreate && !m_GlosArr.empty())
    {
        if (std::none_of(m_GlosArr.begin(), m_GlosArr.end(),
                [&rName](const OUString& rEntry) { return rEntry == rName; }))
        {   // block not in the list
            m_GlosArr.push_back(rName);
        }
    }
    return GetGlosDoc( rName, bCreate );
}

// Creates a new document with the group name. temporarily also created as file
// so that groups remain there later (without access).
bool SwGlossaries::NewGroupDoc(OUString& rGroupName, const OUString& rTitle)
{
    const std::u16string_view sNewPath(o3tl::getToken(rGroupName, 1, GLOS_DELIM));
    sal_uInt16 nNewPath = o3tl::narrowing<sal_uInt16>(o3tl::toInt32(sNewPath));
    if (static_cast<size_t>(nNewPath) >= m_PathArr.size())
        return false;
    const OUString sNewFilePath(m_PathArr[nNewPath]);
    const OUString sNewGroup = lcl_CheckFileName(sNewFilePath, o3tl::getToken(rGroupName, 0, GLOS_DELIM))
        + OUStringChar(GLOS_DELIM) + sNewPath;
    std::unique_ptr<SwTextBlocks> pBlock = GetGlosDoc( sNewGroup );
    if(pBlock)
    {
        GetNameList().push_back(sNewGroup);
        pBlock->SetName(rTitle);
        rGroupName = sNewGroup;
        return true;
    }
    return false;
}

bool    SwGlossaries::RenameGroupDoc(
    const OUString& rOldGroup, OUString& rNewGroup, const OUString& rNewTitle )
{
    sal_uInt16 nOldPath = o3tl::narrowing<sal_uInt16>(o3tl::toInt32(o3tl::getToken(rOldGroup, 1, GLOS_DELIM)));
    if (static_cast<size_t>(nOldPath) >= m_PathArr.size())
        return false;

    const OUString sOldFileURL =
        lcl_FullPathName(m_PathArr[nOldPath], o3tl::getToken(rOldGroup, 0, GLOS_DELIM));

    if (!FStatHelper::IsDocument( sOldFileURL ))
    {
        OSL_FAIL("group doesn't exist!");
        return false;
    }

    sal_uInt16 nNewPath = o3tl::narrowing<sal_uInt16>(o3tl::toInt32(o3tl::getToken(rNewGroup, 1, GLOS_DELIM)));
    if (static_cast<size_t>(nNewPath) >= m_PathArr.size())
        return false;

    const OUString sNewFileName = lcl_CheckFileName(m_PathArr[nNewPath],
                                                    o3tl::getToken(rNewGroup, 0, GLOS_DELIM));
    const OUString sNewFileURL = lcl_FullPathName(m_PathArr[nNewPath], sNewFileName);

    if (FStatHelper::IsDocument( sNewFileURL ))
    {
        OSL_FAIL("group already exists!");
        return false;
    }

    if (!SWUnoHelper::UCB_MoveFile(sOldFileURL, sNewFileURL ))
        return false;

    RemoveFileFromList( rOldGroup );

    rNewGroup = sNewFileName + OUStringChar(GLOS_DELIM) + OUString::number(nNewPath);
    if (m_GlosArr.empty())
    {
        GetNameList();
    }
    else
    {
        m_GlosArr.push_back(rNewGroup);
    }

    SwTextBlocks aNewBlock( sNewFileURL );
    aNewBlock.SetName(rNewTitle);

    return true;
}

// Deletes a text block group
bool SwGlossaries::DelGroupDoc(std::u16string_view rName)
{
    sal_uInt16 nPath = o3tl::narrowing<sal_uInt16>(o3tl::toInt32(o3tl::getToken(rName, 1, GLOS_DELIM)));
    if (static_cast<size_t>(nPath) >= m_PathArr.size())
        return false;
    const std::u16string_view sBaseName(o3tl::getToken(rName, 0, GLOS_DELIM));
    const OUString sFileURL = lcl_FullPathName(m_PathArr[nPath], sBaseName);
    const OUString aName = sBaseName + OUStringChar(GLOS_DELIM) + OUString::number(nPath);
    // Even if the file doesn't exist it has to be deleted from
    // the list of text block regions
    // no && because of CFfront
    bool bRemoved = SWUnoHelper::UCB_DeleteFile( sFileURL );
    OSL_ENSURE(bRemoved, "file has not been removed");
    RemoveFileFromList( aName );
    return bRemoved;
}

SwGlossaries::~SwGlossaries()
{
    InvalidateUNOOjects();
}

// read a block document
std::unique_ptr<SwTextBlocks> SwGlossaries::GetGlosDoc( const OUString &rName, bool bCreate ) const
{
    sal_uInt16 nPath = o3tl::narrowing<sal_uInt16>(o3tl::toInt32(o3tl::getToken(rName, 1, GLOS_DELIM)));
    std::unique_ptr<SwTextBlocks> pTmp;
    if (static_cast<size_t>(nPath) < m_PathArr.size())
    {
        const OUString sFileURL =
            lcl_FullPathName(m_PathArr[nPath], o3tl::getToken(rName, 0, GLOS_DELIM));

        bool bExist = false;
        if(!bCreate)
            bExist = FStatHelper::IsDocument( sFileURL );

        if (bCreate || bExist)
        {
            pTmp.reset(new SwTextBlocks( sFileURL ));
            bool bOk = true;
            if( pTmp->GetError() )
            {
                ErrorHandler::HandleError( pTmp->GetError() );
                bOk = ! pTmp->GetError().IsError();
            }

            if( bOk && pTmp->GetName().isEmpty() )
                pTmp->SetName( rName );
        }
    }

    return pTmp;
}

// access to the list of names; read in if applicable
std::vector<OUString> & SwGlossaries::GetNameList()
{
    if (m_GlosArr.empty())
    {
        const OUString sExt( SwGlossaries::GetExtension() );
        for (size_t i = 0; i < m_PathArr.size(); ++i)
        {
            std::vector<OUString> aFiles;

            SWUnoHelper::UCB_GetFileListOfFolder(m_PathArr[i], aFiles, &sExt);
            for (const OUString& aTitle : aFiles)
            {
                const OUString sName( aTitle.subView( 0, aTitle.getLength() - sExt.getLength() )
                    + OUStringChar(GLOS_DELIM) + OUString::number( static_cast<sal_Int16>(i) ));
                m_GlosArr.push_back(sName);
            }
        }
        if (m_GlosArr.empty())
        {
            // the standard block is inside of the path's first part
            m_GlosArr.emplace_back(SwGlossaries::GetDefName() + OUStringChar(GLOS_DELIM) + "0" );
        }
    }
    return m_GlosArr;
}

SwGlossaries::SwGlossaries()
{
    UpdateGlosPath(true);
}

// set new path and recreate internal array
static OUString lcl_makePath(const std::vector<OUString>& rPaths)
{
    std::vector<OUString>::const_iterator aIt(rPaths.begin());
    const std::vector<OUString>::const_iterator aEnd(rPaths.end());
    OUStringBuffer aPath(*aIt);
    for (++aIt; aIt != aEnd; ++aIt)
    {
        aPath.append(SVT_SEARCHPATH_DELIMITER);
        const INetURLObject aTemp(*aIt);
        aPath.append(aTemp.GetFull());
    }
    return aPath.makeStringAndClear();
}

void SwGlossaries::UpdateGlosPath(bool bFull)
{
    SvtPathOptions aPathOpt;
    const OUString& aNewPath( aPathOpt.GetAutoTextPath() );
    bool bPathChanged = m_aPath != aNewPath;
    if (!(bFull || bPathChanged))
        return;

    m_aPath = aNewPath;

    m_PathArr.clear();

    std::vector<OUString> aDirArr;
    std::vector<OUString> aInvalidPaths;
    if (!m_aPath.isEmpty())
    {
        sal_Int32 nIndex = 0;
        do
        {
            const OUString sPth = URIHelper::SmartRel2Abs(
                INetURLObject(),
                m_aPath.getToken(0, SVT_SEARCHPATH_DELIMITER, nIndex),
                URIHelper::GetMaybeFileHdl());
            if (!aDirArr.empty() &&
                std::find(aDirArr.begin(), aDirArr.end(), sPth) != aDirArr.end())
            {
                continue;
            }
            aDirArr.push_back(sPth);
            if( !FStatHelper::IsFolder( sPth ) )
                aInvalidPaths.push_back(sPth);
            else
                m_PathArr.push_back(sPth);
        }
        while (nIndex>=0);
    }

    if (m_aPath.isEmpty() || !aInvalidPaths.empty())
    {
        std::sort(aInvalidPaths.begin(), aInvalidPaths.end());
        aInvalidPaths.erase(std::unique(aInvalidPaths.begin(), aInvalidPaths.end()), aInvalidPaths.end());
        if (bPathChanged || (m_aInvalidPaths != aInvalidPaths))
        {
            m_aInvalidPaths = aInvalidPaths;
            // wrong path, that means AutoText directory doesn't exist

            ErrorHandler::HandleError( ErrCodeMsg(
                                    ERR_AUTOPATH_ERROR, lcl_makePath(m_aInvalidPaths),
                                    DialogMask::ButtonsOk | DialogMask::MessageError ) );
            m_bError = true;
        }
        else
            m_bError = false;
    }
    else
        m_bError = false;

    if (!m_GlosArr.empty())
    {
        m_GlosArr.clear();
        GetNameList();
    }
}

void SwGlossaries::ShowError()
{
    ErrCodeMsg nPathError(ERR_AUTOPATH_ERROR, lcl_makePath(m_aInvalidPaths), DialogMask::ButtonsOk );
    ErrorHandler::HandleError( nPathError );
}

OUString SwGlossaries::GetExtension()
{
    return u".bau"_ustr;
}

void SwGlossaries::RemoveFileFromList( const OUString& rGroup )
{
    if (m_GlosArr.empty())
        return;

    auto it = std::find(m_GlosArr.begin(), m_GlosArr.end(), rGroup);
    if (it == m_GlosArr.end())
        return;

    {
        // tell the UNO AutoTextGroup object that it's not valid anymore
        for (   UnoAutoTextGroups::iterator aLoop = m_aGlossaryGroups.begin();
                aLoop != m_aGlossaryGroups.end();
            )
        {
            rtl::Reference< SwXAutoTextGroup > xNamed( aLoop->get() );
            if ( !xNamed.is() )
            {
                aLoop = m_aGlossaryGroups.erase(aLoop);
            }
            else if ( xNamed->getName() == rGroup )
            {
                xNamed->Invalidate();
                    // note that this static_cast works because we know that the array only
                    // contains SwXAutoTextGroup implementation
                m_aGlossaryGroups.erase( aLoop );
                break;
            } else
                ++aLoop;
        }
    }

    {
        // tell all our UNO AutoTextEntry objects that they're not valid anymore
        for (   UnoAutoTextEntries::iterator aLoop = m_aGlossaryEntries.begin();
                aLoop != m_aGlossaryEntries.end();
            )
        {
            rtl::Reference<SwXAutoTextEntry> pEntry = aLoop->get();
            if ( pEntry && ( pEntry->GetGroupName() == rGroup ) )
            {
                pEntry->Invalidate();
                aLoop = m_aGlossaryEntries.erase( aLoop );
            }
            else
                ++aLoop;
        }
    }

    m_GlosArr.erase(it);
}

OUString SwGlossaries::GetCompleteGroupName( std::u16string_view rGroupName )
{
    const size_t nCount = GetGroupCnt();
    // when the group name was created internally the path is here as well
    sal_Int32 nIndex = 0;
    const std::u16string_view sGroupName(o3tl::getToken(rGroupName, 0, GLOS_DELIM, nIndex));
    const bool bPathLen = !o3tl::getToken(rGroupName, 0, GLOS_DELIM, nIndex).empty();
    for ( size_t i = 0; i < nCount; i++ )
    {
        const OUString sGrpName = GetGroupName(i);
        if (bPathLen)
        {
            if (rGroupName == sGrpName)
                return sGrpName;
        }
        else
        {
            if (sGroupName == o3tl::getToken(sGrpName, 0, GLOS_DELIM))
                return sGrpName;
        }
    }
    return OUString();
}

void SwGlossaries::InvalidateUNOOjects()
{
    // invalidate all the AutoTextGroup-objects
    for (const auto& rGroup : m_aGlossaryGroups)
    {
        rtl::Reference< SwXAutoTextGroup > xGroup( rGroup.get() );
        if ( xGroup.is() )
            xGroup->Invalidate();
    }
    UnoAutoTextGroups().swap(m_aGlossaryGroups);

    // invalidate all the AutoTextEntry-objects
    for (const auto& rEntry : m_aGlossaryEntries)
    {
        rtl::Reference<SwXAutoTextEntry> pEntry = rEntry.get();
        if ( pEntry )
            pEntry->Invalidate();
    }
    UnoAutoTextEntries().swap(m_aGlossaryEntries);
}

Reference< text::XAutoTextGroup > SwGlossaries::GetAutoTextGroup( std::u16string_view _rGroupName )
{
    bool _bCreate = true;
    // first, find the name with path-extension
    const OUString sCompleteGroupName = GetCompleteGroupName( _rGroupName );

    rtl::Reference< SwXAutoTextGroup >  xGroup;

    // look up the group in the cache
    UnoAutoTextGroups::iterator aSearch = m_aGlossaryGroups.begin();
    for ( ; aSearch != m_aGlossaryGroups.end(); )
    {
        rtl::Reference<SwXAutoTextGroup> pSwGroup = aSearch->get();
        if ( !pSwGroup )
        {
            // the object is dead in the meantime -> remove from cache
            aSearch = m_aGlossaryGroups.erase( aSearch );
            continue;
        }

        if ( _rGroupName == pSwGroup->getName() )
        {                               // the group is already cached
            if ( !sCompleteGroupName.isEmpty() )
            {   // the group still exists -> return it
                xGroup = pSwGroup;
                break;
            }
            else
            {
                // this group does not exist (anymore) -> release the cached UNO object for it
                aSearch = m_aGlossaryGroups.erase( aSearch );
                // so it won't be created below
                _bCreate = false;
                break;
            }
        }

        ++aSearch;
    }

    if ( !xGroup.is() && _bCreate )
    {
        xGroup = new SwXAutoTextGroup( sCompleteGroupName, this );
        // cache it
        m_aGlossaryGroups.emplace_back( xGroup );
    }

    return xGroup;
}

Reference< text::XAutoTextEntry > SwGlossaries::GetAutoTextEntry(
    const OUString& rCompleteGroupName,
    const OUString& rGroupName,
    const OUString& rEntryName )
{
    //standard must be created
    bool bCreate = ( rCompleteGroupName == GetDefName() );
    std::unique_ptr< SwTextBlocks > pGlosGroup( GetGroupDoc( rCompleteGroupName, bCreate ) );

    if (!pGlosGroup || pGlosGroup->GetError())
        throw lang::WrappedTargetException();

    sal_uInt16 nIdx = pGlosGroup->GetIndex( rEntryName );
    if ( USHRT_MAX == nIdx )
        throw container::NoSuchElementException();

    rtl::Reference< SwXAutoTextEntry > xReturn;

    UnoAutoTextEntries::iterator aSearch( m_aGlossaryEntries.begin() );
    for ( ; aSearch != m_aGlossaryEntries.end(); )
    {
        rtl::Reference< SwXAutoTextEntry > pEntry( aSearch->get() );

        if ( !pEntry )
        {
            // the object is dead in the meantime -> remove from cache
            aSearch = m_aGlossaryEntries.erase( aSearch );
            continue;
        }

        if  (   pEntry
            &&  pEntry->GetGroupName() == rGroupName
            &&  pEntry->GetEntryName() == rEntryName
            )
        {
            xReturn = pEntry;
            break;
        }

        ++aSearch;
    }

    if ( !xReturn.is() )
    {
        xReturn = new SwXAutoTextEntry( this, rGroupName, rEntryName );
        // cache it
        m_aGlossaryEntries.emplace_back( xReturn );
    }

    return xReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
