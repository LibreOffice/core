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

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>

#include <comphelper/processfactory.hxx>
#include <unotools/intlwrapper.hxx>

#include <comphelper/storagehelper.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/doctempl.hxx>
#include "docvor.hxx"
#include "orgmgr.hxx"
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"
#include "view.hrc"
#include <sfx2/docfilt.hxx>
#include "fltfnc.hxx"

using namespace ::com::sun::star;

//=========================================================================

/*  [Description]

    Implementation class, single entry in the file view.
*/

struct _FileListEntry
{
    String aFileName;           // File Name with complete path
    String aBaseName;           // File Name
    const CollatorWrapper* pCollator;
    SfxObjectShellLock aDocShell; // ObjectShell as reference class

    sal_Bool bFile;                 // As File on disk
                                // (!= not processed, not loaded as document
                                // these are not saved!)
    sal_Bool bOwner;                // self-generated
    sal_Bool bNoName;
    sal_Bool bOwnFormat;

    _FileListEntry( const String& rFileName,
                    const CollatorWrapper* pColl, const String* pTitle = NULL );
    ~_FileListEntry();

    int operator==( const _FileListEntry &rCmp) const;
    int operator< ( const _FileListEntry &rCmp) const;
    sal_Bool DeleteObjectShell();
};

//-------------------------------------------------------------------------

inline int _FileListEntry::operator==(const _FileListEntry &rCmp) const
{
    DBG_ASSERT( pCollator, "invalid CollatorWrapper" );
    return COMPARE_EQUAL == pCollator->compareString(aBaseName, rCmp.aBaseName);
}

//-------------------------------------------------------------------------

inline int _FileListEntry::operator< (const _FileListEntry &rCmp) const
{
    DBG_ASSERT( pCollator, "invalid CollatorWrapper" );
    return COMPARE_LESS == pCollator->compareString(aBaseName, rCmp.aBaseName);
}

//-------------------------------------------------------------------------

_FileListEntry::_FileListEntry( const String& rFileName,
                                const CollatorWrapper* pColl, const String* pTitle ) :

    aFileName   ( rFileName ),
    pCollator   ( pColl ),
    bFile       ( sal_False ),
    bOwner      ( sal_False ),
    bNoName     ( sal_True ),
    bOwnFormat  ( sal_True )
{
    if ( pTitle )
        aBaseName = *pTitle;
    else
    {
        INetURLObject aObj( rFileName, INET_PROT_FILE );
        aBaseName = aObj.getName( INetURLObject::LAST_SEGMENT, true,
                                  INetURLObject::DECODE_WITH_CHARSET );
    }
}

//-------------------------------------------------------------------------

_FileListEntry::~_FileListEntry()
{
    DeleteObjectShell();
}

//=========================================================================

sal_Bool _FileListEntry::DeleteObjectShell()

/*  [Description]

    Release of ther DocumentShell

    [Return value]          TRUE: Everything is ok
                            FALSE: An error occured
                            (the document could not be saved)
*/

{
    sal_Bool bRet = sal_True;

    if(bOwner && aDocShell.Is() && aDocShell->IsModified())
    {
        // Converted?
        if( bOwnFormat )
        {
            if(!aDocShell->Save() )
                bRet = sal_False;
            else
            {
                try {
                    uno::Reference< embed::XTransactedObject > xTransact( aDocShell->GetStorage(), uno::UNO_QUERY );
                    OSL_ENSURE( xTransact.is(), "Storage must implement XTransactedObject!\n" );
                    if ( !xTransact.is() )
                        throw uno::RuntimeException();

                    xTransact->commit();
                }
                catch( uno::Exception& )
                {
                }

            }
        }
        else
        {
            // If converted save in native format
            INetURLObject aObj( aFileName );
            String aTitle = aObj.getName( INetURLObject::LAST_SEGMENT, true,
                                          INetURLObject::DECODE_WITH_CHARSET );
            bRet = aDocShell->PreDoSaveAs_Impl(
                        aTitle, aDocShell->GetFactory().GetFilterContainer()->GetAnyFilter( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT )->GetFilterName(), 0 );
        }
    }

    if( bOwner)
    {
        aDocShell.Clear();
    }

    return bRet;
}

//-------------------------------------------------------------------------

SfxObjectList::SfxObjectList()
{
}

//-------------------------------------------------------------------------

SfxObjectList::~SfxObjectList()
{
    for( const_iterator it = begin(); it != end(); ++it )
        delete *it;
    clear();
}

//-------------------------------------------------------------------------

const String &SfxObjectList::GetBaseName(const _FileListEntry* p) const
{
    return p->aBaseName;
}

//-------------------------------------------------------------------------

SfxOrganizeMgr::SfxOrganizeMgr( SfxOrganizeListBox_Impl *pLeft,
                                SfxOrganizeListBox_Impl *pRight,
                                SfxDocumentTemplates *pTempl) :
    pImpl(new SfxOrganizeMgr_Impl),
    pTemplates(pTempl? pTempl: new SfxDocumentTemplates),
    pLeftBox(pLeft),
    pRightBox(pRight),
    bDeleteTemplates(pTempl == 0),
    bModified(0)

/*  [Description]

    Constructor. The current document is added to the list of documents.
*/
{
    pImpl->pDocList = new SfxObjectList;
    pImpl->pIntlWrapper = new IntlWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    const CollatorWrapper* pCollator = pImpl->pIntlWrapper->getCaseCollator();
    for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst(); pTmp; pTmp = SfxObjectShell::GetNext(*pTmp) )
    {
        if ( pTmp->GetCreateMode() != SFX_CREATE_MODE_STANDARD ||
             !( pTmp->GetFlags() & SFXOBJECTSHELL_HASOPENDOC ) || !pTmp->GetStyleSheetPool() )
            continue;
        _FileListEntry* pNewEntry = NULL;
        String aTitle = pTmp->GetTitle( SFX_TITLE_TITLE );
        pNewEntry = new _FileListEntry( pTmp->GetMedium()->GetName(), pCollator, &aTitle );
        pNewEntry->aDocShell = pTmp;
        pImpl->pDocList->insert( pNewEntry );
    }
}

//-------------------------------------------------------------------------

SfxOrganizeMgr::~SfxOrganizeMgr()
{
    if ( bDeleteTemplates )
        delete pTemplates;
    delete pImpl->pDocList;
    delete pImpl->pIntlWrapper;
    delete pImpl;
    pLeftBox = pRightBox = NULL;
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeMgr::CreateObjectShell( sal_uInt16 nIdx )

/*  [Description]

    Access to the DocumentShell at the position nIdx.

    [Return value]                      Reference to the DocumentShell

*/

{
    _FileListEntry* pEntry = (*pImpl->pDocList)[nIdx];
    // otherwise create Doc-Shell
    if ( !pEntry->aDocShell.Is() )
    {
        INetURLObject aFileObj( pEntry->aFileName );
        sal_Bool bDum = sal_False;
        SfxApplication* pSfxApp = SFX_APP();
        String aFilePath = aFileObj.GetMainURL( INetURLObject::NO_DECODE );
        pEntry->aDocShell = pSfxApp->DocAlreadyLoaded( aFilePath, sal_False, bDum );
        if ( !pEntry->aDocShell.Is() )
        {
            pEntry->bOwner = sal_True;
            SfxMedium* pMed = new SfxMedium(
                aFilePath, ( STREAM_READ | STREAM_SHARE_DENYWRITE ) );
            const SfxFilter* pFilter = NULL;
            pMed->UseInteractionHandler(sal_True);
            if (
                pSfxApp->GetFilterMatcher().GuessFilter(*pMed, &pFilter, SFX_FILTER_TEMPLATE, 0) ||
                (pFilter && !pFilter->IsOwnFormat()) ||
                (pFilter && !pFilter->UsesStorage())
               )
            {
                pSfxApp->LoadTemplate( pEntry->aDocShell, aFilePath );
                pEntry->bOwnFormat = sal_False;
                delete pMed;
                if ( pEntry->aDocShell.Is() )
                    return (SfxObjectShellRef)(SfxObjectShell*)(pEntry->aDocShell);
            }
            else
            {
                if ( pFilter )
                {
                    pEntry->bOwnFormat = sal_True;
                    pEntry->aDocShell = SfxObjectShell::CreateObject( pFilter->GetServiceName(), SFX_CREATE_MODE_ORGANIZER );
                    if ( pEntry->aDocShell.Is() )
                    {
                        pEntry->aDocShell->DoInitNew(0);
                        pEntry->aDocShell->LoadFrom( *pMed );
                        // Medium is now owned by DocShell
                        pEntry->aDocShell->DoSaveCompleted( pMed );
                    }
                }
            }
        }
    }
    return ( SfxObjectShellRef )(SfxObjectShell*)(pEntry->aDocShell);
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeMgr::DeleteObjectShell(sal_uInt16 nIdx)

/*  [Description]

    Release DocumentShell at position nIdx

    [Return value]          TRUE: Everything is ok
                            FALSE: An error occured
                            (the document could not be saved)
*/
{
    return (*pImpl->pDocList)[nIdx]->DeleteObjectShell();
}

//-------------------------------------------------------------------------

SfxObjectShellRef SfxOrganizeMgr::CreateObjectShell(sal_uInt16 nRegion,
                                                        sal_uInt16 nIdx)
/*  [Description]

    Access to the  DocumentShell at Position nIdx in Region
    nRegion (Document template)

    [Return value]                      Reference to the DocumentShell

*/
{
    return pTemplates->CreateObjectShell(nRegion, nIdx);
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeMgr::DeleteObjectShell(sal_uInt16 nRegion, sal_uInt16 nIdx)

/*  [Description]

    Release of the DocumentShell at Position nIdx in Region
    nRegion (Document template)

    [Return value]          TRUE: Everything is ok
                            FALSE: An error occured
                            (the document could not be saved)
*/

{
    return pTemplates->DeleteObjectShell(nRegion, nIdx);
}

//-------------------------------------------------------------------------

sal_Bool    SfxOrganizeMgr::Copy(sal_uInt16 nTargetRegion,
                            sal_uInt16 nTargetIdx,
                            sal_uInt16 nSourceRegion,
                            sal_uInt16 nSourceIdx)

/*  [Description]

    Copy of a Document Template

    [Parameter]

    sal_uInt16 nTargetRegion                Index of the Target Region
    sal_uInt16 nTargetIdx                   Index of the Target Position
    sal_uInt16 nSourceRegion                Index of the Source Region
    sal_uInt16 nSourceIdx                   Index of the template to be
                                        copied/moved.

    [Return value]                      Success (TRUE) or Failure (FALSE)


    [Cross-reference]

    <SfxDocumentTemplates::Copy(sal_uInt16 nTargetRegion,
                                sal_uInt16 nTargetIdx,
                                sal_uInt16 nSourceRegion,
                                sal_uInt16 nSourceIdx)>

*/

{
    if(nSourceIdx == USHRT_MAX) // No directories copied
        return sal_False ;
    const sal_Bool bOk = pTemplates->Copy(nTargetRegion, nTargetIdx,
                                        nSourceRegion, nSourceIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool    SfxOrganizeMgr::Move(sal_uInt16 nTargetRegion,
                            sal_uInt16 nTargetIdx,
                            sal_uInt16 nSourceRegion,
                            sal_uInt16 nSourceIdx)

/*  [Description]

    Moving a template

    [Parameter]

    sal_uInt16 nTargetRegion                Index of the Target Region
    sal_uInt16 nTargetIdx                   Index of the Target Position
    sal_uInt16 nSourceRegion                Index of the Source Region
    sal_uInt16 nSourceIdx                   Index of the template to be
                                        copied/moved.

    [Return value]                      Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::Move(sal_uInt16 nTargetRegion,
                                sal_uInt16 nTargetIdx,
                                sal_uInt16 nSourceRegion,
                                sal_uInt16 nSourceIdx)>
*/

{
    if(nSourceIdx == USHRT_MAX) // No directory moved
        return sal_False ;
    const sal_Bool bOk = pTemplates->Move(nTargetRegion, nTargetIdx,
                                        nSourceRegion, nSourceIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool    SfxOrganizeMgr::Delete(SfxOrganizeListBox_Impl *pCaller,
                                sal_uInt16 nRegion, sal_uInt16 nIdx)

/*  [Description]

    Delete  a Document Template

    [Parameter]

    SfxOrganizeListBox *pCaller     calling ListBox, since this event
                                    is triggered by the menu or the
                                    keyboard, the ListBox must be updated.
    sal_uInt16 nRegion                  Index for Region
    sal_uInt16 nIdx                     Index of Document template

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::Delete(sal_uInt16 nRegion, sal_uInt16 nIdx)>

*/

{
    sal_Bool bOk = sal_False;

    if ( USHRT_MAX == nIdx )
    {
        // deleting of a group

        SvLBoxEntry *pGroupToDelete = pCaller->SvLBox::GetEntry(nRegion);
        if ( pGroupToDelete )
        {
            sal_uInt16 nItemNum = (sal_uInt16)( pCaller->GetModel()->GetChildCount( pGroupToDelete ) );
            typedef std::deque<SvLBoxEntry*> BoxEntries;
            BoxEntries pEntriesToDelete;

            sal_uInt16 nInd = 0;
            for ( nInd = 0; nInd < nItemNum; nInd++ )
            {
                // TODO/LATER: check that nInd is the same index that is used in pTemplates
                if ( pTemplates->Delete( nRegion, nInd ) )
                {
                    bModified = 1;
                    pEntriesToDelete.push_back( pCaller->SvLBox::GetEntry( pGroupToDelete, nInd ) );
                }
            }

            for ( BoxEntries::const_iterator aIt( pEntriesToDelete.begin() ), aEnd( pEntriesToDelete.end() ); aIt != aEnd; ++aIt )
                pCaller->GetModel()->Remove( *aIt );

            if ( !pCaller->GetModel()->GetChildCount( pGroupToDelete ) )
            {
                bOk = pTemplates->Delete( nRegion, nIdx );
                if ( bOk )
                    pCaller->GetModel()->Remove( pGroupToDelete );
            }
        }
    }
    else
    {
        // deleting of a template
        bOk = pTemplates->Delete(nRegion, nIdx);
        if(bOk)
        {
            bModified = 1;
                // Entry to be deleted.
            SvLBoxEntry *pEntryToDelete = pCaller->SvLBox::GetEntry(pCaller->SvLBox::GetEntry(nRegion), nIdx);

            pCaller->GetModel()->Remove(pEntryToDelete);
        }
    }

    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool    SfxOrganizeMgr::InsertDir
(
    SfxOrganizeListBox_Impl* pCaller, /* calling ListBox, since this event
                                         is triggered by the menu or the
                                         keyboard, the ListBox must be updated. */
    const String&            rText,   // logical Name of Region
    sal_uInt16                   nRegion  // Index of Region
)

/*  [Description]

    Insert Region.

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::InsertDir(const String &, sal_uInt16 nRegion)>
*/

{
    const sal_Bool bOk = pTemplates->InsertDir(rText, nRegion);
    if(bOk)
    {
        bModified = 1;
        SvLBoxEntry *pEntry = pCaller->InsertEntry(rText,
                                                   pCaller->GetOpenedBmp(0),
                                                   pCaller->GetClosedBmp(0),
                                                   0, sal_True, nRegion);
        pCaller->Update();
        pCaller->EditEntry(pEntry);
    }
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeMgr::SetName(const String &rName,
                             sal_uInt16 nRegion, sal_uInt16 nIdx)

/*  [Description]

    Set (logical) Name

    [Parameter]

    const String &rName             The new Name
    sal_uInt16 nRegion                  Index of Region
    sal_uInt16 nIdx                     Index of Document template

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::SetName(const String &, sal_uInt16 nRegion, sal_uInt16 nIdx)>
*/

{
    const sal_Bool bOk = pTemplates->SetName(rName, nRegion, nIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeMgr::CopyTo(sal_uInt16 nRegion, sal_uInt16 nIdx, const String &rName) const

/*  [Description]

    Export of a Template

    [Parameter]

    sal_uInt16 nRegion                  Index of Region
    sal_uInt16 nIdx                     Index of Document Template
    const String &rName             File name

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::CopyTo( sal_uInt16 nRegion, sal_uInt16 nIdx, const String &)>
*/

{
    return pTemplates->CopyTo(nRegion, nIdx, rName);
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeMgr::CopyFrom(SfxOrganizeListBox_Impl *pCaller,
                              sal_uInt16 nRegion, sal_uInt16 nIdx, String &rName)

/*  [Description]

    Import of Document Template

    [Parameter]

    SfxOrganizeListBox *pCaller     calling ListBox, since this event
                                    is triggered by the menu or the
                                    keyboard, the ListBox must be updated.

    sal_uInt16 nRegion                  Index of Region
    sal_uInt16 nIdx                     Index of Document Template
    String &rName                   File name

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::CopyFrom( sal_uInt16 nRegion, sal_uInt16 nIdx, const String &)>
*/

{
    SvLBoxEntry *pParent = pCaller->FirstSelected();
    if( nIdx!=USHRT_MAX )
        pParent = pCaller->GetParent(pParent);
    if( pTemplates->CopyFrom( nRegion, nIdx, rName ) )
    {
        // Update pCaller
        if( nIdx == USHRT_MAX )
            nIdx = 0;
        else nIdx++;

        pCaller->InsertEntry( rName,
                              pCaller->GetOpenedBmp(1),
                              pCaller->GetClosedBmp(1),
                              pParent,
                              sal_True,
                              nIdx);
        pCaller->Update();
        pCaller->Expand( pParent );
        bModified = sal_True;
        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeMgr::InsertFile( SfxOrganizeListBox_Impl* pCaller, const String& rFileName )

/*  [Description]

    Insert a file in the file view.

    [Parameter]

    SfxOrganizeListBox *pCaller     calling ListBox, since this event
                                    is triggered by the menu or the
                                    keyboard, the ListBox must be updated.

    const String &rFileName         Name of inserted File.

    [Return value]                  Success (TRUE) or Failure (FALSE)
*/

{
    const CollatorWrapper* pCollator = pImpl->pIntlWrapper->getCaseCollator();
    _FileListEntry* pEntry = new _FileListEntry( rFileName, pCollator );
    std::pair<SfxObjectList::const_iterator, bool> aRes = pImpl->pDocList->insert( pEntry );
    if ( aRes.second )
    {
        sal_uInt16 nPos = aRes.first - pImpl->pDocList->begin();
        pCaller->InsertEntry( pEntry->aBaseName, pCaller->GetOpenedBmp(1),
                              pCaller->GetClosedBmp(1), 0, sal_True, nPos );
        return sal_True;
    }
    else
        delete pEntry;
    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxOrganizeMgr::Rescan()

/*  [Description]

    Updating the database.

    [Return value]

    TRUE                                        Changes were made
    FALSE                                       No changes

    [Cross-reference]

    <SfxDocumentTemplates::Rescan()>
*/

{
    if(pTemplates->Rescan())
    {
        bModified = sal_True;
        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------

void SfxOrganizeMgr::SaveAll(Window *pParent)

/*  [Description]

    Save all Documents that have been modified

    [Parameter]

    Window *pParent            Parent of the Error message Box
*/

{
    sal_uInt16 nRangeCount = pTemplates->GetRegionCount();
    sal_uInt16 i;
    for(i = 0; i < nRangeCount; ++i)
    {
        if( pTemplates->IsRegionLoaded( i ))
        {
            const sal_uInt16 nCount = pTemplates->GetCount(i);
            for(sal_uInt16 j = 0; j < nCount; ++j)
            {
                if(!pTemplates->DeleteObjectShell(i, j))
                {
                    String aText = SfxResId(STR_ERROR_SAVE_TEMPLATE).toString();
                    aText += pTemplates->GetName(i, j);
                    ErrorBox aBox(pParent,
                                  WinBits(WB_OK_CANCEL | WB_DEF_CANCEL),
                                  aText);
                    if(RET_CANCEL == aBox.Execute())
                        break;
                }
            }
        }
    }
    for( SfxObjectList::const_iterator it = pImpl->pDocList->begin(); it != pImpl->pDocList->end(); ++it )
    {
        _FileListEntry *pEntry = *it;
        if(!pEntry->DeleteObjectShell())
        {
            String aText(SfxResId(STR_ERROR_SAVE_TEMPLATE).toString());
            aText += pEntry->aBaseName;
            ErrorBox aBox(pParent, WinBits(WB_OK_CANCEL | WB_DEF_CANCEL), aText);
            if(RET_CANCEL == aBox.Execute())
                break;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
