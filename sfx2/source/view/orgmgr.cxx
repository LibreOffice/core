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
#include "precompiled_sfx2.hxx"
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
#include "sfxresid.hxx"
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

    BOOL bFile;                 // As File on disk
                                // (!= not processed, not loaded as document
                                // these are not saved!)
    BOOL bOwner;                // self-generated
    BOOL bNoName;
    BOOL bOwnFormat;

    _FileListEntry( const String& rFileName,
                    const CollatorWrapper* pColl, const String* pTitle = NULL );
    ~_FileListEntry();

    int operator==( const _FileListEntry &rCmp) const;
    int operator< ( const _FileListEntry &rCmp) const;
    BOOL DeleteObjectShell();
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
    bFile       ( FALSE ),
    bOwner      ( FALSE ),
    bNoName     ( TRUE ),
    bOwnFormat  ( TRUE )
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

//-------------------------------------------------------------------------

SV_IMPL_OP_PTRARR_SORT(_SfxObjectList, _FileListEntry*)

//=========================================================================

BOOL _FileListEntry::DeleteObjectShell()

/*  [Description]

    Release of ther DocumentShell

    [Return value]          TRUE: Everything is ok
                            FALSE: An error occured
                            (the document could not be saved)
*/

{
    BOOL bRet = TRUE;

    if(bOwner && aDocShell.Is() && aDocShell->IsModified())
    {
        // Converted?
        if( bOwnFormat )
        {
            if(!aDocShell->Save() )
                bRet = FALSE;
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

//              aDocShell->SfxObjectShell::DoSaveCompleted();
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
    DeleteAndDestroy(0, Count());
}

//-------------------------------------------------------------------------

const String &SfxObjectList::GetBaseName(USHORT i) const
{
    return (*this)[i]->aBaseName;
}

//-------------------------------------------------------------------------

const String& SfxObjectList::GetFileName( USHORT i ) const
{
    return (*this)[i]->aFileName;
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
        pImpl->pDocList->C40_PTR_INSERT( _FileListEntry, pNewEntry );
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

SfxObjectShellRef SfxOrganizeMgr::CreateObjectShell( USHORT nIdx )

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
        BOOL bDum = FALSE;
        SfxApplication* pSfxApp = SFX_APP();
        String aFilePath = aFileObj.GetMainURL( INetURLObject::NO_DECODE );
        pEntry->aDocShell = pSfxApp->DocAlreadyLoaded( aFilePath, FALSE, bDum );
        if ( !pEntry->aDocShell.Is() )
        {
            pEntry->bOwner = TRUE;
            SfxMedium* pMed = new SfxMedium(
                aFilePath, ( STREAM_READ | STREAM_SHARE_DENYWRITE ), FALSE, 0 );
            const SfxFilter* pFilter = NULL;
            pMed->UseInteractionHandler(TRUE);
            if (
                pSfxApp->GetFilterMatcher().GuessFilter(*pMed, &pFilter, SFX_FILTER_TEMPLATE, 0) ||
                (pFilter && !pFilter->IsOwnFormat()) ||
                (pFilter && !pFilter->UsesStorage())
               )
            {
                pSfxApp->LoadTemplate( pEntry->aDocShell, aFilePath );
                pEntry->bOwnFormat = FALSE;
                delete pMed;
                if ( pEntry->aDocShell.Is() )
                    return (SfxObjectShellRef)(SfxObjectShell*)(pEntry->aDocShell);
            }
            else
            {
                if ( pFilter )
                {
                    pEntry->bOwnFormat = TRUE;
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

BOOL SfxOrganizeMgr::DeleteObjectShell(USHORT nIdx)

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

SfxObjectShellRef SfxOrganizeMgr::CreateObjectShell(USHORT nRegion,
                                                        USHORT nIdx)
/*  [Description]

    Access to the  DocumentShell at Position nIdx in Region
    nRegion (Document template)

    [Return value]                      Reference to the DocumentShell

*/
{
    return pTemplates->CreateObjectShell(nRegion, nIdx);
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::DeleteObjectShell(USHORT nRegion, USHORT nIdx)

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

BOOL    SfxOrganizeMgr::Copy(USHORT nTargetRegion,
                            USHORT nTargetIdx,
                            USHORT nSourceRegion,
                            USHORT nSourceIdx)

/*  [Description]

    Copy of a Document Template

    [Parameter]

    USHORT nTargetRegion                Index of the Target Region
    USHORT nTargetIdx                   Index of the Target Position
    USHORT nSourceRegion                Index of the Source Region
    USHORT nSourceIdx                   Index of the template to be
                                        copied/moved.

    [Return value]                      Success (TRUE) or Failure (FALSE)


    [Cross-reference]

    <SfxDocumentTemplates::Copy(USHORT nTargetRegion,
                                USHORT nTargetIdx,
                                USHORT nSourceRegion,
                                USHORT nSourceIdx)>

*/

{
    if(nSourceIdx == USHRT_MAX) // No directories copied
        return FALSE ;
    const BOOL bOk = pTemplates->Copy(nTargetRegion, nTargetIdx,
                                        nSourceRegion, nSourceIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

BOOL    SfxOrganizeMgr::Move(USHORT nTargetRegion,
                            USHORT nTargetIdx,
                            USHORT nSourceRegion,
                            USHORT nSourceIdx)

/*  [Description]

    Moving a template

    [Parameter]

    USHORT nTargetRegion                Index of the Target Region
    USHORT nTargetIdx                   Index of the Target Position
    USHORT nSourceRegion                Index of the Source Region
    USHORT nSourceIdx                   Index of the template to be
                                        copied/moved.

    [Return value]                      Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::Move(USHORT nTargetRegion,
                                USHORT nTargetIdx,
                                USHORT nSourceRegion,
                                USHORT nSourceIdx)>
*/

{
    if(nSourceIdx == USHRT_MAX) // No directory moved
        return FALSE ;
    const BOOL bOk = pTemplates->Move(nTargetRegion, nTargetIdx,
                                        nSourceRegion, nSourceIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

BOOL    SfxOrganizeMgr::Delete(SfxOrganizeListBox_Impl *pCaller,
                                USHORT nRegion, USHORT nIdx)

/*  [Description]

    Delete  a Document Template

    [Parameter]

    SfxOrganizeListBox *pCaller     calling ListBox, since this event
                                    is triggered by the menu or the
                                    keyboard, the ListBox must be updated.
    USHORT nRegion                  Index for Region
    USHORT nIdx                     Index of Document template

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::Delete(USHORT nRegion, USHORT nIdx)>

*/

{
    BOOL bOk = FALSE;

    if ( USHRT_MAX == nIdx )
    {
        // deleting of a group

        SvLBoxEntry *pGroupToDelete = pCaller->SvLBox::GetEntry(nRegion);
        if ( pGroupToDelete )
        {
            USHORT nItemNum = (USHORT)( pCaller->GetModel()->GetChildCount( pGroupToDelete ) );
            USHORT nToDeleteNum = 0;
            SvLBoxEntry **pEntriesToDelete = new SvLBoxEntry*[nItemNum];

            USHORT nInd = 0;
            for ( nInd = 0; nInd < nItemNum; nInd++ )
                pEntriesToDelete[nInd] = NULL;

            for ( nInd = 0; nInd < nItemNum; nInd++ )
            {
                // TODO/LATER: check that nInd is the same index that is used in pTemplates
                if ( pTemplates->Delete( nRegion, nInd ) )
                {
                    bModified = 1;
                    pEntriesToDelete[nToDeleteNum++] = pCaller->SvLBox::GetEntry( pGroupToDelete, nInd );
                }
            }

            for ( nInd = 0; nInd < nToDeleteNum; nInd++ )
                if ( pEntriesToDelete[nInd] )
                    pCaller->GetModel()->Remove( pEntriesToDelete[nInd] );

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

BOOL    SfxOrganizeMgr::InsertDir
(
    SfxOrganizeListBox_Impl* pCaller, /* calling ListBox, since this event
                                         is triggered by the menu or the
                                         keyboard, the ListBox must be updated. */
    const String&            rText,   // logical Name of Region
    USHORT                   nRegion  // Index of Region
)

/*  [Description]

    Insert Region.

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::InsertDir(const String &, USHORT nRegion)>
*/

{
    const BOOL bOk = pTemplates->InsertDir(rText, nRegion);
    if(bOk)
    {
        bModified = 1;
        SvLBoxEntry *pEntry = pCaller->InsertEntry(rText,
                                                   pCaller->GetOpenedBmp(0),
                                                   pCaller->GetClosedBmp(0),
                                                   0, TRUE, nRegion);
        pCaller->Update();
        pCaller->EditEntry(pEntry);
    }
    return bOk;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::SetName(const String &rName,
                             USHORT nRegion, USHORT nIdx)

/*  [Description]

    Set (logical) Name

    [Parameter]

    const String &rName             The new Name
    USHORT nRegion                  Index of Region
    USHORT nIdx                     Index of Document template

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::SetName(const String &, USHORT nRegion, USHORT nIdx)>
*/

{
    const BOOL bOk = pTemplates->SetName(rName, nRegion, nIdx);
    if(bOk)
        bModified = 1;
    return bOk;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::CopyTo(USHORT nRegion, USHORT nIdx, const String &rName) const

/*  [Description]

    Export of a Template

    [Parameter]

    USHORT nRegion                  Index of Region
    USHORT nIdx                     Index of Document Template
    const String &rName             File name

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::CopyTo( USHORT nRegion, USHORT nIdx, const String &)>
*/

{
    return pTemplates->CopyTo(nRegion, nIdx, rName);
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::CopyFrom(SfxOrganizeListBox_Impl *pCaller,
                              USHORT nRegion, USHORT nIdx, String &rName)

/*  [Description]

    Import of Document Template

    [Parameter]

    SfxOrganizeListBox *pCaller     calling ListBox, since this event
                                    is triggered by the menu or the
                                    keyboard, the ListBox must be updated.

    USHORT nRegion                  Index of Region
    USHORT nIdx                     Index of Document Template
    String &rName                   File name

    [Return value]                  Success (TRUE) or Failure (FALSE)

    [Cross-reference]

    <SfxDocumentTemplates::CopyFrom( USHORT nRegion, USHORT nIdx, const String &)>
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
                              TRUE,
                              nIdx);
        pCaller->Update();
        // pCaller->EditEntry( pEntry );
        pCaller->Expand( pParent );
        bModified = TRUE;
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::InsertFile( SfxOrganizeListBox_Impl* pCaller, const String& rFileName )

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
    if ( pImpl->pDocList->C40_PTR_INSERT( _FileListEntry, pEntry ) )
    {
        USHORT nPos = 0;
        pImpl->pDocList->Seek_Entry( pEntry, &nPos );
        pCaller->InsertEntry( pEntry->aBaseName, pCaller->GetOpenedBmp(1),
                              pCaller->GetClosedBmp(1), 0, TRUE, nPos );
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

BOOL SfxOrganizeMgr::Rescan()

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
        bModified = TRUE;
        return TRUE;
    }
    return FALSE;
}

//-------------------------------------------------------------------------

void SfxOrganizeMgr::SaveAll(Window *pParent)

/*  [Description]

    Save all Documents that have been modified

    [Parameter]

    Window *pParent            Parent of the Error message Box
*/

{
    USHORT nRangeCount = pTemplates->GetRegionCount();
    USHORT i;
    for(i = 0; i < nRangeCount; ++i)
    {
        if( pTemplates->IsRegionLoaded( i ))
        {
            const USHORT nCount = pTemplates->GetCount(i);
            for(USHORT j = 0; j < nCount; ++j)
            {
                if(!pTemplates->DeleteObjectShell(i, j))
                {
                    String aText = String(SfxResId(STR_ERROR_SAVE_TEMPLATE));
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
    nRangeCount = pImpl->pDocList->Count();
    for(i = 0; i < nRangeCount; ++i)
    {
        _FileListEntry *pEntry = (*pImpl->pDocList)[i];
        if(!pEntry->DeleteObjectShell())
        {
            String aText(SfxResId(STR_ERROR_SAVE_TEMPLATE));
            aText += pEntry->aBaseName;
            ErrorBox aBox(pParent, WinBits(WB_OK_CANCEL | WB_DEF_CANCEL), aText);
            if(RET_CANCEL == aBox.Execute())
                break;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
