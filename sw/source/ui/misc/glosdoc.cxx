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
#include <memory>

#include <com/sun/star/container/XNamed.hpp>

#include <unotools/transliterationwrapper.hxx>


#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#include <osl/diagnose.h>
#include <svl/urihelper.hxx>
#include <svl/fstathelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/string.hxx>
#include <swtypes.hxx>
#include <uitool.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <swunohelper.hxx>

#include <unoatxt.hxx>
#include <swerror.h>
#include <globals.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{

inline OUString lcl_FullPathName(const OUString sPath, const OUString sName)
{
    return sPath + OUString(INET_PATH_TOKEN) + sName + SwGlossaries::GetExtension();
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
        if (comphelper::string::isalnumAscii(cChar) ||
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
    utl::TempFile aTemp(OUString("group"), &rSG, &rNewFilePath );
    aTemp.EnableKillingFile();

    INetURLObject aTempURL( aTemp.GetURL() );
    return aTempURL.GetBase();
}

}

/*------------------------------------------------------------------------
    Description: supplies the default group's name
------------------------------------------------------------------------*/
String  SwGlossaries::GetDefName()
{
    return OUString("standard");

}

/*------------------------------------------------------------------------
    Description: supplies the number of text block groups
------------------------------------------------------------------------*/
sal_uInt16 SwGlossaries::GetGroupCnt()
{
    return static_cast<sal_uInt16>(GetNameList().size());
}

/*------------------------------------------------------------------------
    Description: supplies the group's name
------------------------------------------------------------------------*/
sal_Bool SwGlossaries::FindGroupName(OUString& rGroup)
{
    // if the group name doesn't contain a path, a suitable group entry
    // can the searched for here;
    sal_uInt16 nCount = GetGroupCnt();
    sal_uInt16 i;
    for(i= 0; i < nCount; i++)
    {
        const OUString sTemp(GetGroupName(i));
        if (rGroup==sTemp.getToken(0, GLOS_DELIM))
        {
            rGroup = sTemp;
            return sal_True;
        }
    }
    // you can search two times because for more directories the case sensitive
    // name could occur multiple times
    const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
    for(i = 0; i < nCount; i++)
    {
        const OUString sTemp( GetGroupName( i ));
        sal_uInt16 nPath = (sal_uInt16)sTemp.getToken(1, GLOS_DELIM).toInt32();

        if (!SWUnoHelper::UCB_IsCaseSensitiveFileName( m_PathArr[nPath] )
             && rSCmp.isEqual( rGroup, sTemp.getToken( 0, GLOS_DELIM) ) )
        {
            rGroup = sTemp;
            return sal_True;
        }
    }
    return sal_False;
}

String SwGlossaries::GetGroupName(sal_uInt16 nGroupId)
{
    OSL_ENSURE(static_cast<size_t>(nGroupId) < m_GlosArr.size(),
            "SwGlossaries::GetGroupName: index out of bounds");
    return m_GlosArr[nGroupId];
}

String  SwGlossaries::GetGroupTitle( const String& rGroupName )
{
    String  sRet;
    OUString sGroup(rGroupName);
    if (sGroup.indexOf(GLOS_DELIM)<0)
        FindGroupName(sGroup);
    SwTextBlocks* pGroup = GetGroupDoc(sGroup, sal_False);
    if(pGroup)
    {
        sRet = pGroup->GetName();
         PutGroupDoc( pGroup );
    }
    return sRet;
}

/*------------------------------------------------------------------------
    Description: supplies the group rName's text block document
------------------------------------------------------------------------*/
SwTextBlocks* SwGlossaries::GetGroupDoc(const String &rName,
                                        sal_Bool bCreate)
{
    // insert to the list of text blocks if applicable
    if(bCreate && !m_GlosArr.empty())
    {
        std::vector<String>::const_iterator it(m_GlosArr.begin());
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

/*------------------------------------------------------------------------
 Description:   delete a text block
------------------------------------------------------------------------*/
void SwGlossaries::PutGroupDoc(SwTextBlocks *pBlock) {
    delete pBlock;
}

/*------------------------------------------------------------------------
    Description:   Creates a new document with the group name. temporarly
                   also created as file so that groups remain there later
                   (without access).
------------------------------------------------------------------------*/
sal_Bool SwGlossaries::NewGroupDoc(OUString& rGroupName, const OUString& rTitle)
{
    sal_uInt16 nNewPath = (sal_uInt16)rGroupName.getToken(1, GLOS_DELIM).toInt32();
    if (static_cast<size_t>(nNewPath) >= m_PathArr.size())
        return sal_False;
    const OUString sNewFilePath(m_PathArr[nNewPath]);
    String sNewGroup = lcl_CheckFileName(sNewFilePath, rGroupName.getToken(0, GLOS_DELIM));
    sNewGroup += GLOS_DELIM;
    sNewGroup += rGroupName.getToken(1, GLOS_DELIM);
    SwTextBlocks *pBlock = GetGlosDoc( sNewGroup );
    if(pBlock)
    {
        GetNameList().push_back(sNewGroup);
        pBlock->SetName(rTitle);
        PutGroupDoc(pBlock);
        rGroupName = sNewGroup;
        return sal_True;
    }
    return sal_False;
}

sal_Bool    SwGlossaries::RenameGroupDoc(
    const OUString& rOldGroup, OUString& rNewGroup, const OUString& rNewTitle )
{
    sal_uInt16 nOldPath = (sal_uInt16)rOldGroup.getToken(1, GLOS_DELIM).toInt32();
    if (static_cast<size_t>(nOldPath) >= m_PathArr.size())
        return sal_False;

    const OUString sOldFileURL =
        lcl_FullPathName(m_PathArr[nOldPath], rOldGroup.getToken(0, GLOS_DELIM));

    if (!FStatHelper::IsDocument( sOldFileURL ))
    {
        OSL_FAIL("group doesn't exist!");
        return sal_False;
    }

    sal_uInt16 nNewPath = (sal_uInt16)rNewGroup.getToken(1, GLOS_DELIM).toInt32();
    if (static_cast<size_t>(nNewPath) >= m_PathArr.size())
        return sal_False;

    const OUString sNewFileName = lcl_CheckFileName(m_PathArr[nNewPath],
                                                    rNewGroup.getToken(0, GLOS_DELIM));
    const OUString sNewFileURL = lcl_FullPathName(m_PathArr[nNewPath], sNewFileName);

    if (FStatHelper::IsDocument( sNewFileURL ))
    {
        OSL_FAIL("group already exists!");
        return sal_False;
    }

    if (!SWUnoHelper::UCB_CopyFile(sOldFileURL, sNewFileURL, sal_True ))
        return sal_False;

    RemoveFileFromList( rOldGroup );

    rNewGroup = sNewFileName + OUString(GLOS_DELIM) + OUString::number(nNewPath);
    if (m_GlosArr.empty())
    {
        GetNameList();
    }
    else
    {
        m_GlosArr.push_back(rNewGroup);
    }

    SwTextBlocks* pNewBlock = new SwTextBlocks( sNewFileURL );
    pNewBlock->SetName(rNewTitle);
    delete pNewBlock;

    return sal_True;
}

/*------------------------------------------------------------------------
    Description: Deletes a text block group
------------------------------------------------------------------------*/
sal_Bool SwGlossaries::DelGroupDoc(const String &rName)
{
    sal_uInt16 nPath = (sal_uInt16)rName.GetToken(1, GLOS_DELIM).ToInt32();
    if (static_cast<size_t>(nPath) >= m_PathArr.size())
        return sal_False;
    const OUString sBaseName(rName.GetToken(0, GLOS_DELIM));
    const OUString sFileURL = lcl_FullPathName(m_PathArr[nPath], sBaseName);
    const OUString aName = sBaseName + OUString(GLOS_DELIM) + OUString::number(nPath);
    // Even if the file doesn't exist it has to be deleted from
    // the list of text block regions
    // no && because of CFfront
    sal_Bool bRemoved = SWUnoHelper::UCB_DeleteFile( sFileURL );
    OSL_ENSURE(bRemoved, "file has not been removed");
    RemoveFileFromList( aName );
    return bRemoved;
}

SwGlossaries::~SwGlossaries()
{
    InvalidateUNOOjects();
}

/*------------------------------------------------------------------------
    Description: read a block document
------------------------------------------------------------------------*/
SwTextBlocks* SwGlossaries::GetGlosDoc( const String &rName, sal_Bool bCreate ) const
{
    sal_uInt16 nPath = (sal_uInt16)rName.GetToken(1, GLOS_DELIM).ToInt32();
    SwTextBlocks *pTmp = 0;
    if (static_cast<size_t>(nPath) < m_PathArr.size())
    {
        const OUString sFileURL =
            lcl_FullPathName(m_PathArr[nPath], rName.GetToken(0, GLOS_DELIM));

        sal_Bool bExist = sal_False;
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

/*------------------------------------------------------------------------
    Description: access to the list of names; read in if applicable
------------------------------------------------------------------------*/
std::vector<String> & SwGlossaries::GetNameList()
{
    if (m_GlosArr.empty())
    {
        String sExt( SwGlossaries::GetExtension() );
        for (size_t i = 0; i < m_PathArr.size(); ++i)
        {
            std::vector<String*> aFiles;

            SWUnoHelper::UCB_GetFileListOfFolder(m_PathArr[i], aFiles, &sExt);
            for( std::vector<String*>::const_iterator filesIt(aFiles.begin());
                 filesIt != aFiles.end(); ++filesIt)
            {
                String *pTitle = *filesIt;
                String sName( pTitle->Copy( 0, pTitle->Len() - sExt.Len() ));
                sName += GLOS_DELIM;
                sName += OUString::number( static_cast<sal_Int16>(i) );
                m_GlosArr.push_back(sName);

                // don't need any more these pointers
                delete pTitle;
            }
        }
        if (m_GlosArr.empty())
        {
            // the standard block is inside of the path's first part
            String tmp( SwGlossaries::GetDefName() );
            tmp += GLOS_DELIM;
            tmp += '0';
            m_GlosArr.push_back( tmp );
        }
    }
    return m_GlosArr;
}

SwGlossaries::SwGlossaries()
{
    UpdateGlosPath(sal_True);
}

/*------------------------------------------------------------------------
    Description: set new path and recreate internal array
------------------------------------------------------------------------*/

rtl::OUString lcl_makePath(const std::vector<rtl::OUString>& rPaths)
{
    std::vector<rtl::OUString>::const_iterator aIt(rPaths.begin());
    const std::vector<rtl::OUString>::const_iterator aEnd(rPaths.end());
    rtl::OUStringBuffer aPath(*aIt);
    for (++aIt; aIt != aEnd; ++aIt)
    {
        aPath.append(SVT_SEARCHPATH_DELIMITER);
        const INetURLObject aTemp(*aIt);
        aPath.append(aTemp.GetFull());
    }
    return aPath.getStr();
}

void SwGlossaries::UpdateGlosPath(sal_Bool bFull)
{
    SvtPathOptions aPathOpt;
    String aNewPath( aPathOpt.GetAutoTextPath() );
    bool bPathChanged = m_aPath != aNewPath;
    if (bFull || bPathChanged)
    {
        m_aPath = aNewPath;

        m_PathArr.clear();

        sal_uInt16 nTokenCount = comphelper::string::getTokenCount(m_aPath, SVT_SEARCHPATH_DELIMITER);
        std::vector<String> aDirArr;
        std::vector<rtl::OUString> aInvalidPaths;
        for( sal_uInt16 i = 0; i < nTokenCount; i++ )
        {
            String sPth(m_aPath.GetToken(i, SVT_SEARCHPATH_DELIMITER));
            sPth = URIHelper::SmartRel2Abs(
                INetURLObject(), sPth, URIHelper::GetMaybeFileHdl());

            if(i && std::find(aDirArr.begin(), aDirArr.end(), sPth) != aDirArr.end())
            {
                continue;
            }
            aDirArr.push_back(sPth);
            if( !FStatHelper::IsFolder( sPth ) )
                aInvalidPaths.push_back(sPth);
            else
                m_PathArr.push_back(sPth);
        }

        if(!nTokenCount || !aInvalidPaths.empty())
        {
            std::sort(aInvalidPaths.begin(), aInvalidPaths.end());
            aInvalidPaths.erase(std::unique(aInvalidPaths.begin(), aInvalidPaths.end()), aInvalidPaths.end());
            if (bPathChanged || (m_aInvalidPaths != aInvalidPaths))
            {
                m_aInvalidPaths = aInvalidPaths;
                // wrong path, that means AutoText directory doesn't exist

                ErrorHandler::HandleError( *new StringErrorInfo(
                                        ERR_AUTOPATH_ERROR, lcl_makePath(m_aInvalidPaths),
                                        ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR ));
                m_bError = sal_True;
            }
            else
                m_bError = sal_False;
        }
        else
            m_bError = sal_False;

        if (!m_GlosArr.empty())
        {
            m_GlosArr.clear();
            GetNameList();
        }
    }
}

void SwGlossaries::ShowError()
{
    sal_uInt32 nPathError = *new StringErrorInfo(ERR_AUTOPATH_ERROR,
                                            lcl_makePath(m_aInvalidPaths), ERRCODE_BUTTON_OK );
    ErrorHandler::HandleError( nPathError );
}

String SwGlossaries::GetExtension()
{
    return OUString(".bau");
}

void SwGlossaries::RemoveFileFromList( const String& rGroup )
{
    if (!m_GlosArr.empty())
    {
        for (std::vector<String>::iterator it(m_GlosArr.begin());
                it != m_GlosArr.end(); ++it)
        {
            if (*it == rGroup)
            {
                OUString aUName = rGroup;
                {
                    // tell the UNO AutoTextGroup object that it's not valid anymore
                    for (   UnoAutoTextGroups::iterator aLoop = m_aGlossaryGroups.begin();
                            aLoop != m_aGlossaryGroups.end();
                            ++aLoop
                        )
                    {
                        Reference< container::XNamed > xNamed( aLoop->get(), UNO_QUERY );
                        if ( xNamed.is() && ( xNamed->getName() == aUName ) )
                        {
                            static_cast< SwXAutoTextGroup* >( xNamed.get() )->Invalidate();
                                // note that this static_cast works because we know that the array only
                                // contains SwXAutoTextGroup implementation
                            m_aGlossaryGroups.erase( aLoop );
                            break;
                        }
                    }
                }

                {
                    // tell all our UNO AutoTextEntry objects that they're not valid anymore
                    for (   UnoAutoTextEntries::iterator aLoop = m_aGlossaryEntries.begin();
                            aLoop != m_aGlossaryEntries.end();
                        )
                    {
                        Reference< lang::XUnoTunnel > xEntryTunnel( aLoop->get(), UNO_QUERY );

                        SwXAutoTextEntry* pEntry = NULL;
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

String SwGlossaries::GetCompleteGroupName( const OUString& GroupName )
{
    sal_uInt16 nCount = GetGroupCnt();
    // when the group name was created internally the path is here as well
    String sGroup(GroupName);
    String sGroupName(sGroup.GetToken(0, GLOS_DELIM));
    String sPath = sGroup.GetToken(1, GLOS_DELIM);
    bool bPathLen = sPath.Len() > 0;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        String sGrpName = GetGroupName(i);
        if(bPathLen ? sGroup == sGrpName : sGroupName == sGrpName.GetToken(0, GLOS_DELIM))
        {
            return sGrpName;
        }
    }
    return aEmptyStr;
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
        SwXAutoTextEntry* pEntry = NULL;
        if ( xEntryTunnel.is() )
            pEntry = reinterpret_cast< SwXAutoTextEntry* >(
                xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );

        if ( pEntry )
            pEntry->Invalidate();
    }
    UnoAutoTextEntries aTmpe = UnoAutoTextEntries();
    m_aGlossaryEntries.swap( aTmpe );
}

Reference< text::XAutoTextGroup > SwGlossaries::GetAutoTextGroup( const OUString& _rGroupName, bool _bCreate )
{
    // first, find the name with path-extension
    String sCompleteGroupName = GetCompleteGroupName( _rGroupName );

    Reference< text::XAutoTextGroup >  xGroup;

    // look up the group in the cache
    UnoAutoTextGroups::iterator aSearch = m_aGlossaryGroups.begin();
    for ( ; aSearch != m_aGlossaryGroups.end(); )
    {
        Reference< lang::XUnoTunnel > xGroupTunnel( aSearch->get(), UNO_QUERY );

        SwXAutoTextGroup* pSwGroup = 0;
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
            if ( sCompleteGroupName.Len() )
            {   // the group still exists -> return it
                xGroup = pSwGroup;
                break;
            }
            else
            {
                // this group does not exist (anymore) -> release the cached UNO object for it
                aSearch = m_aGlossaryGroups.erase( aSearch );
                // so it won't be created below
                _bCreate = sal_False;
                break;
            }
        }

        ++aSearch;
    }

    if ( !xGroup.is() && _bCreate )
    {
        xGroup = new SwXAutoTextGroup( sCompleteGroupName, this );
        // cache it
        m_aGlossaryGroups.push_back( AutoTextGroupRef( xGroup ) );
    }

    return xGroup;
}

Reference< text::XAutoTextEntry > SwGlossaries::GetAutoTextEntry( const String& _rCompleteGroupName, const OUString& _rGroupName, const OUString& _rEntryName,
    bool _bCreate )
{
    //standard must be created
    sal_Bool bCreate = ( _rCompleteGroupName == GetDefName() );
    ::std::auto_ptr< SwTextBlocks > pGlosGroup( GetGroupDoc( _rCompleteGroupName, bCreate ) );

    if ( pGlosGroup.get() && !pGlosGroup->GetError() )
    {
        sal_uInt16 nIdx = pGlosGroup->GetIndex( _rEntryName );
        if ( USHRT_MAX == nIdx )
            throw container::NoSuchElementException();
    }
    else
        throw lang::WrappedTargetException();

    Reference< text::XAutoTextEntry > xReturn;
    String sGroupName( _rGroupName );
    String sEntryName( _rEntryName );

    UnoAutoTextEntries::iterator aSearch( m_aGlossaryEntries.begin() );
    for ( ; aSearch != m_aGlossaryEntries.end(); )
    {
        Reference< lang::XUnoTunnel > xEntryTunnel( aSearch->get(), UNO_QUERY );

        SwXAutoTextEntry* pEntry = NULL;
        if ( xEntryTunnel.is() )
            pEntry = reinterpret_cast< SwXAutoTextEntry* >( xEntryTunnel->getSomething( SwXAutoTextEntry::getUnoTunnelId() ) );
        else
        {
            // the object is dead in the meantime -> remove from cache
            aSearch = m_aGlossaryEntries.erase( aSearch );
            continue;
        }

        if  (   pEntry
            &&  ( COMPARE_EQUAL == pEntry->GetGroupName().CompareTo( sGroupName ) )
            &&  ( COMPARE_EQUAL == pEntry->GetEntryName().CompareTo( sEntryName ) )
            )
        {
            xReturn = pEntry;
            break;
        }

        ++aSearch;
    }

    if ( !xReturn.is() && _bCreate )
    {
        xReturn = new SwXAutoTextEntry( this, sGroupName, sEntryName );
        // cache it
        m_aGlossaryEntries.push_back( AutoTextEntryRef( xReturn ) );
    }

    return xReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
