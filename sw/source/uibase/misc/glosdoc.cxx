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

#include <com/sun/star/container/XNamed.hpp>

#include <unotools/transliterationwrapper.hxx>

#include <tools/errinf.hxx>
#include <osl/diagnose.h>
#include <svl/urihelper.hxx>
#include <svl/fstathelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/tempfile.hxx>
#include <swtypes.hxx>
#include <uitool.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swunohelper.hxx>

#include <unoatxt.hxx>
#include <swerror.h>
#include <globals.hrc>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

inline OUString lcl_FullPathName(const OUString& sPath, const OUString& sName)
{
    return sPath + "/" + sName + SwGlossaries::GetExtension();
}

OUString lcl_CheckFileName( const OUString& rNewFilePath,
                          const OUString& rNewGroupName )
{
    const sal_Int32 nLen = rNewGroupName.getLength();
    OUStringBuffer aBuf(nLen);
    //group name should contain only A-Z and a-z and spaces
    for( sal_Int32 i=0; i < nLen; ++i )
    {
        const sal_Unicode cChar = rNewGroupName[i];
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

    OUString rSG = SwGlossaries::GetExtension();
    //generate generic name
    utl::TempFile aTemp("group", true, &rSG, &rNewFilePath);
    aTemp.EnableKillingFile();

    INetURLObject aTempURL( aTemp.GetURL() );
    return aTempURL.GetBase();
}

}

// supplies the default group's name
OUString SwGlossaries::GetDefName()
{
    return OUString("standard");

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
        if (rGroup==sTemp.getToken(0, GLOS_DELIM))
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
        sal_uInt16 nPath = (sal_uInt16)sTemp.getToken(1, GLOS_DELIM).toInt32();

        if (!SWUnoHelper::UCB_IsCaseSensitiveFileName( m_PathArr[nPath] )
             && rSCmp.isEqual( rGroup, sTemp.getToken( 0, GLOS_DELIM) ) )
        {
            rGroup = sTemp;
            return true;
        }
    }
    return false;
}

OUString SwGlossaries::GetGroupName(size_t nGroupId)
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
    SwTextBlocks* pGroup = GetGroupDoc(sGroup);
    if(pGroup)
    {
        sRet = pGroup->GetName();
        delete pGroup;
    }
    return sRet;
}

// supplies the group rName's text block document
SwTextBlocks* SwGlossaries::GetGroupDoc(const OUString &rName,
                                        bool bCreate)
{
    // insert to the list of text blocks if applicable
    if(bCreate && !m_GlosArr.empty())
    {
        std::vector<OUString>::const_iterator it(m_GlosArr.begin());
        for (; it != m_GlosArr.end(); ++it)
        {
            if (*it == rName)
                break;
        }
        if (it == m_GlosArr.end())
        {   // block not in the list
            m_GlosArr.push_back(rName);
        }
    }
    return GetGlosDoc( rName, bCreate );
}

// Creates a new document with the group name. temporarly also created as file
// so that groups remain there later (without access).
bool SwGlossaries::NewGroupDoc(OUString& rGroupName, const OUString& rTitle)
{
    const OUString sNewPath(rGroupName.getToken(1, GLOS_DELIM));
    sal_uInt16 nNewPath = (sal_uInt16)sNewPath.toInt32();
    if (static_cast<size_t>(nNewPath) >= m_PathArr.size())
        return false;
    const OUString sNewFilePath(m_PathArr[nNewPath]);
    const OUString sNewGroup = lcl_CheckFileName(sNewFilePath, rGroupName.getToken(0, GLOS_DELIM))
        + OUStringLiteral1(GLOS_DELIM) + sNewPath;
    SwTextBlocks *pBlock = GetGlosDoc( sNewGroup );
    if(pBlock)
    {
        GetNameList().push_back(sNewGroup);
        pBlock->SetName(rTitle);
        delete pBlock;
        rGroupName = sNewGroup;
        return true;
    }
    return false;
}

bool    SwGlossaries::RenameGroupDoc(
    const OUString& rOldGroup, OUString& rNewGroup, const OUString& rNewTitle )
{
    sal_uInt16 nOldPath = (sal_uInt16)rOldGroup.getToken(1, GLOS_DELIM).toInt32();
    if (static_cast<size_t>(nOldPath) >= m_PathArr.size())
        return false;

    const OUString sOldFileURL =
        lcl_FullPathName(m_PathArr[nOldPath], rOldGroup.getToken(0, GLOS_DELIM));

    if (!FStatHelper::IsDocument( sOldFileURL ))
    {
        OSL_FAIL("group doesn't exist!");
        return false;
    }

    sal_uInt16 nNewPath = (sal_uInt16)rNewGroup.getToken(1, GLOS_DELIM).toInt32();
    if (static_cast<size_t>(nNewPath) >= m_PathArr.size())
        return false;

    const OUString sNewFileName = lcl_CheckFileName(m_PathArr[nNewPath],
                                                    rNewGroup.getToken(0, GLOS_DELIM));
    const OUString sNewFileURL = lcl_FullPathName(m_PathArr[nNewPath], sNewFileName);

    if (FStatHelper::IsDocument( sNewFileURL ))
    {
        OSL_FAIL("group already exists!");
        return false;
    }

    if (!SWUnoHelper::UCB_CopyFile(sOldFileURL, sNewFileURL, true ))
        return false;

    RemoveFileFromList( rOldGroup );

    rNewGroup = sNewFileName + OUStringLiteral1(GLOS_DELIM) + OUString::number(nNewPath);
    if (m_GlosArr.empty())
    {
        GetNameList();
    }
    else
    {
        m_GlosArr.push_back(rNewGroup);
    }

    std::unique_ptr<SwTextBlocks> pNewBlock(new SwTextBlocks( sNewFileURL ));
    pNewBlock->SetName(rNewTitle);

    return true;
}

// Deletes a text block group
bool SwGlossaries::DelGroupDoc(const OUString &rName)
{
    sal_uInt16 nPath = (sal_uInt16)rName.getToken(1, GLOS_DELIM).toInt32();
    if (static_cast<size_t>(nPath) >= m_PathArr.size())
        return false;
    const OUString sBaseName(rName.getToken(0, GLOS_DELIM));
    const OUString sFileURL = lcl_FullPathName(m_PathArr[nPath], sBaseName);
    const OUString aName = sBaseName + OUStringLiteral1(GLOS_DELIM) + OUString::number(nPath);
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
SwTextBlocks* SwGlossaries::GetGlosDoc( const OUString &rName, bool bCreate ) const
{
    sal_uInt16 nPath = (sal_uInt16)rName.getToken(1, GLOS_DELIM).toInt32();
    SwTextBlocks *pTmp = nullptr;
    if (static_cast<size_t>(nPath) < m_PathArr.size())
    {
        const OUString sFileURL =
            lcl_FullPathName(m_PathArr[nPath], rName.getToken(0, GLOS_DELIM));

        bool bExist = false;
        if(!bCreate)
            bExist = FStatHelper::IsDocument( sFileURL );

        if (bCreate || bExist)
        {
            pTmp = new SwTextBlocks( sFileURL );
            bool bOk = true;
            if( pTmp->GetError() )
            {
                ErrorHandler::HandleError( pTmp->GetError() );
                bOk = !IsError( pTmp->GetError() );
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
            for( std::vector<OUString>::const_iterator filesIt(aFiles.begin());
                 filesIt != aFiles.end(); ++filesIt)
            {
                const OUString aTitle = *filesIt;
                const OUString sName( aTitle.copy( 0, aTitle.getLength() - sExt.getLength() )
                    + OUStringLiteral1(GLOS_DELIM) + OUString::number( static_cast<sal_Int16>(i) ));
                m_GlosArr.push_back(sName);
            }
        }
        if (m_GlosArr.empty())
        {
            // the standard block is inside of the path's first part
            m_GlosArr.push_back( SwGlossaries::GetDefName() + OUStringLiteral1(GLOS_DELIM) + "0" );
        }
    }
    return m_GlosArr;
}

SwGlossaries::SwGlossaries()
{
    UpdateGlosPath(true);
}

// set new path and recreate internal array
OUString lcl_makePath(const std::vector<OUString>& rPaths)
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
    return aPath.getStr();
}

void SwGlossaries::UpdateGlosPath(bool bFull)
{
    SvtPathOptions aPathOpt;
    OUString aNewPath( aPathOpt.GetAutoTextPath() );
    bool bPathChanged = m_aPath != aNewPath;
    if (bFull || bPathChanged)
    {
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
                if (aDirArr.size() &&
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

                ErrorHandler::HandleError( (new StringErrorInfo(
                                        ERR_AUTOPATH_ERROR, lcl_makePath(m_aInvalidPaths),
                                        ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR ))->GetErrorCode());
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
}

void SwGlossaries::ShowError()
{
    sal_uInt32 nPathError = (new StringErrorInfo(ERR_AUTOPATH_ERROR,
                                            lcl_makePath(m_aInvalidPaths), ERRCODE_BUTTON_OK ))->GetErrorCode();
    ErrorHandler::HandleError( nPathError );
}

OUString SwGlossaries::GetExtension()
{
    return OUString(".bau");
}

void SwGlossaries::RemoveFileFromList( const OUString& rGroup )
{
    if (!m_GlosArr.empty())
    {
        for (std::vector<OUString>::iterator it(m_GlosArr.begin());
                it != m_GlosArr.end(); ++it)
        {
            if (*it == rGroup)
            {
                {
                    // tell the UNO AutoTextGroup object that it's not valid anymore
                    for (   UnoAutoTextGroups::iterator aLoop = m_aGlossaryGroups.begin();
                            aLoop != m_aGlossaryGroups.end();
                        )
                    {
                        Reference< container::XNamed > xNamed( aLoop->get(), UNO_QUERY );
                        if ( !xNamed.is() )
                        {
                            aLoop = m_aGlossaryGroups.erase(aLoop);
                        }
                        else if ( xNamed->getName() == rGroup )
                        {
                            static_cast< SwXAutoTextGroup* >( xNamed.get() )->Invalidate();
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
                        Reference< lang::XUnoTunnel > xEntryTunnel( aLoop->get(), UNO_QUERY );

                        SwXAutoTextEntry* pEntry = nullptr;
                        if ( xEntryTunnel.is() )
                            pEntry = reinterpret_cast< SwXAutoTextEntry* >(
                                xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );

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
                break;
            }
        }
    }
}

OUString SwGlossaries::GetCompleteGroupName( const OUString& rGroupName )
{
    const size_t nCount = GetGroupCnt();
    // when the group name was created internally the path is here as well
    sal_Int32 nIndex = 0;
    const OUString sGroupName(rGroupName.getToken(0, GLOS_DELIM, nIndex));
    const bool bPathLen = !rGroupName.getToken(0, GLOS_DELIM, nIndex).isEmpty();
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
            if (sGroupName == sGrpName.getToken(0, GLOS_DELIM))
                return sGrpName;
        }
    }
    return OUString();
}

void SwGlossaries::InvalidateUNOOjects()
{
    // invalidate all the AutoTextGroup-objects
    for (   UnoAutoTextGroups::iterator aGroupLoop = m_aGlossaryGroups.begin();
            aGroupLoop != m_aGlossaryGroups.end();
            ++aGroupLoop
        )
    {
        Reference< text::XAutoTextGroup > xGroup( aGroupLoop->get(), UNO_QUERY );
        if ( xGroup.is() )
            static_cast< SwXAutoTextGroup* >( xGroup.get() )->Invalidate();
    }
    UnoAutoTextGroups aTmpg = UnoAutoTextGroups();
    m_aGlossaryGroups.swap( aTmpg );

    // invalidate all the AutoTextEntry-objects
    for (   UnoAutoTextEntries::const_iterator aEntryLoop = m_aGlossaryEntries.begin();
            aEntryLoop != m_aGlossaryEntries.end();
            ++aEntryLoop
        )
    {
        Reference< lang::XUnoTunnel > xEntryTunnel( aEntryLoop->get(), UNO_QUERY );
        SwXAutoTextEntry* pEntry = nullptr;
        if ( xEntryTunnel.is() )
            pEntry = reinterpret_cast< SwXAutoTextEntry* >(
                xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );

        if ( pEntry )
            pEntry->Invalidate();
    }
    UnoAutoTextEntries aTmpe = UnoAutoTextEntries();
    m_aGlossaryEntries.swap( aTmpe );
}

Reference< text::XAutoTextGroup > SwGlossaries::GetAutoTextGroup( const OUString& _rGroupName )
{
    bool _bCreate = true;
    // first, find the name with path-extension
    const OUString sCompleteGroupName = GetCompleteGroupName( _rGroupName );

    Reference< text::XAutoTextGroup >  xGroup;

    // look up the group in the cache
    UnoAutoTextGroups::iterator aSearch = m_aGlossaryGroups.begin();
    for ( ; aSearch != m_aGlossaryGroups.end(); )
    {
        Reference< lang::XUnoTunnel > xGroupTunnel( aSearch->get(), UNO_QUERY );

        SwXAutoTextGroup* pSwGroup = nullptr;
        if ( xGroupTunnel.is() )
            pSwGroup = reinterpret_cast< SwXAutoTextGroup* >( xGroupTunnel->getSomething( SwXAutoTextGroup::getUnoTunnelId() ) );

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
        m_aGlossaryGroups.push_back( css::uno::WeakReference< css::text::XAutoTextGroup >( xGroup ) );
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

    if ( pGlosGroup.get() && !pGlosGroup->GetError() )
    {
        sal_uInt16 nIdx = pGlosGroup->GetIndex( rEntryName );
        if ( USHRT_MAX == nIdx )
            throw container::NoSuchElementException();
    }
    else
        throw lang::WrappedTargetException();

    Reference< text::XAutoTextEntry > xReturn;

    UnoAutoTextEntries::iterator aSearch( m_aGlossaryEntries.begin() );
    for ( ; aSearch != m_aGlossaryEntries.end(); )
    {
        Reference< lang::XUnoTunnel > xEntryTunnel( aSearch->get(), UNO_QUERY );

        SwXAutoTextEntry* pEntry = nullptr;
        if ( xEntryTunnel.is() )
            pEntry = reinterpret_cast< SwXAutoTextEntry* >( xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );
        else
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
        m_aGlossaryEntries.push_back( css::uno::WeakReference< css::text::XAutoTextEntry >( xReturn ) );
    }

    return xReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
