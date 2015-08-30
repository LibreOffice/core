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

#include "dbfindex.hxx"
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>
#include <tools/config.hxx>
#include <sfx2/app.hxx>
#include "moduledbu.hxx"
#include "dbu_dlg.hrc"
#include <osl/diagnose.h>
#include <unotools/localfilehelper.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <ucbhelper/content.hxx>
#include <svl/filenotation.hxx>
#include <rtl/strbuf.hxx>

namespace dbaui
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::svt;

const OString aGroupIdent("dBase III");


ODbaseIndexDialog::ODbaseIndexDialog(vcl::Window * pParent, const OUString& aDataSrcName)
    : ModalDialog(pParent, "DBaseIndexDialog", "dbaccess/ui/dbaseindexdialog.ui")
    , m_aDSN(aDataSrcName)
    , m_bCaseSensitiv(true)
{
    get(m_pPB_OK, "ok");
    get(m_pCB_Tables, "table");
    get(m_pIndexes, "frame");
    get(m_pLB_TableIndexes, "tableindex");
    get(m_pLB_FreeIndexes, "freeindex");
    Size aSize(LogicToPixel(Size(76, 98), MAP_APPFONT));
    m_pLB_TableIndexes->set_height_request(aSize.Height());
    m_pLB_TableIndexes->set_width_request(aSize.Width());
    m_pLB_FreeIndexes->set_height_request(aSize.Height());
    m_pLB_FreeIndexes->set_width_request(aSize.Width());
    get(m_pAdd, "add");
    get(m_pAddAll, "addall");
    get(m_pRemove, "remove");
    get(m_pRemoveAll, "removeall");


    m_pCB_Tables->SetSelectHdl( LINK(this, ODbaseIndexDialog, TableSelectHdl) );
    m_pAdd->SetClickHdl( LINK(this, ODbaseIndexDialog, AddClickHdl) );
    m_pRemove->SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveClickHdl) );
    m_pAddAll->SetClickHdl( LINK(this, ODbaseIndexDialog, AddAllClickHdl) );
    m_pRemoveAll->SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveAllClickHdl) );
    m_pPB_OK->SetClickHdl( LINK(this, ODbaseIndexDialog, OKClickHdl) );

    m_pLB_FreeIndexes->SetSelectHdl( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );
    m_pLB_TableIndexes->SetSelectHdl( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );

    m_pCB_Tables->SetDropDownLineCount(8);
    Init();
    SetCtrls();
}

ODbaseIndexDialog::~ODbaseIndexDialog()
{
    disposeOnce();
}

void ODbaseIndexDialog::dispose()
{
    m_pPB_OK.clear();
    m_pCB_Tables.clear();
    m_pIndexes.clear();
    m_pLB_TableIndexes.clear();
    m_pLB_FreeIndexes.clear();
    m_pAdd.clear();
    m_pRemove.clear();
    m_pAddAll.clear();
    m_pRemoveAll.clear();
    ModalDialog::dispose();
}

bool ODbaseIndexDialog::GetTable(const OUString& _rName, TableInfoList::iterator& _rPosition)
{
    for (   _rPosition = m_aTableInfoList.begin();
            _rPosition != m_aTableInfoList.end();
            ++_rPosition
        )
    {
        if (m_bCaseSensitiv)
        {
            if (_rPosition->aTableName == _rName)
                return true;
        }
        else
        {
            if (_rPosition->aTableName.equalsIgnoreAsciiCase(_rName))
                return true;
        }
    }
    return false;
}

void ODbaseIndexDialog::checkButtons()
{
    m_pAdd->Enable(0 != m_pLB_FreeIndexes->GetSelectEntryCount());
    m_pAddAll->Enable(0 != m_pLB_FreeIndexes->GetEntryCount());

    m_pRemove->Enable(0 != m_pLB_TableIndexes->GetSelectEntryCount());
    m_pRemoveAll->Enable(0 != m_pLB_TableIndexes->GetEntryCount());
}

OTableIndex ODbaseIndexDialog::implRemoveIndex(const OUString& _rName, TableIndexList& _rList, ListBox& _rDisplay, bool _bMustExist)
{
    OTableIndex aReturn;

    sal_Int32 nPos = 0;

    TableIndexList::iterator aSearch;
    for (   aSearch = _rList.begin();
            aSearch != _rList.end();
            ++aSearch, ++nPos
        )
    {
        if ( m_bCaseSensitiv ? aSearch->GetIndexFileName() == _rName : aSearch->GetIndexFileName().equalsIgnoreAsciiCase(_rName) )
        {
            aReturn = *aSearch;

            _rList.erase(aSearch);
            _rDisplay.RemoveEntry( _rName );

            // adjust selection if necessary
            if ((sal_uInt32)nPos == _rList.size())
                _rDisplay.SelectEntryPos((sal_uInt16)nPos-1);
            else
                _rDisplay.SelectEntryPos((sal_uInt16)nPos);

            break;
        }
    }

    (void)_bMustExist;
    OSL_ENSURE(!_bMustExist || (aSearch != _rList.end()), "ODbaseIndexDialog::implRemoveIndex : did not find the index!");
    return aReturn;
}

void ODbaseIndexDialog::implInsertIndex(const OTableIndex& _rIndex, TableIndexList& _rList, ListBox& _rDisplay)
{
    _rList.push_front( _rIndex );
    _rDisplay.InsertEntry( _rIndex.GetIndexFileName() );
    _rDisplay.SelectEntryPos(0);
}

OTableIndex ODbaseIndexDialog::RemoveTableIndex( const OUString& _rTableName, const OUString& _rIndexName, bool _bMustExist )
{
    OTableIndex aReturn;

    // does the table exist ?
    TableInfoList::iterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return aReturn;

    return implRemoveIndex(_rIndexName, aTablePos->aIndexList, *m_pLB_TableIndexes, _bMustExist);
}

void ODbaseIndexDialog::InsertTableIndex( const OUString& _rTableName, const OTableIndex& _rIndex)
{
    TableInfoList::iterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return;

    implInsertIndex(_rIndex, aTablePos->aIndexList, *m_pLB_TableIndexes);
}

IMPL_LINK_NOARG_TYPED( ODbaseIndexDialog, OKClickHdl, Button*, void )
{
    // let all tables write their INF file

    for (   TableInfoList::const_iterator aLoop = m_aTableInfoList.begin();
            aLoop != m_aTableInfoList.end();
            ++aLoop
        )
        aLoop->WriteInfFile(m_aDSN);

    EndDialog();
}

IMPL_LINK_NOARG_TYPED( ODbaseIndexDialog, AddClickHdl, Button*, void )
{
    OUString aSelection = m_pLB_FreeIndexes->GetSelectEntry();
    OUString aTableName = m_pCB_Tables->GetText();
    OTableIndex aIndex = RemoveFreeIndex( aSelection, true );
    InsertTableIndex( aTableName, aIndex );

    checkButtons();
}

IMPL_LINK_NOARG_TYPED( ODbaseIndexDialog, RemoveClickHdl, Button*, void )
{
    OUString aSelection = m_pLB_TableIndexes->GetSelectEntry();
    OUString aTableName = m_pCB_Tables->GetText();
    OTableIndex aIndex = RemoveTableIndex( aTableName, aSelection, true );
    InsertFreeIndex( aIndex );

    checkButtons();
}

IMPL_LINK_NOARG_TYPED( ODbaseIndexDialog, AddAllClickHdl, Button*, void )
{
    const sal_Int32 nCnt = m_pLB_FreeIndexes->GetEntryCount();
    OUString aTableName = m_pCB_Tables->GetText();

    for( sal_Int32 nPos = 0; nPos < nCnt; ++nPos )
        InsertTableIndex( aTableName, RemoveFreeIndex( m_pLB_FreeIndexes->GetEntry(0), true ) );

    checkButtons();
}

IMPL_LINK_NOARG_TYPED( ODbaseIndexDialog, RemoveAllClickHdl, Button*, void )
{
    const sal_Int32 nCnt = m_pLB_TableIndexes->GetEntryCount();
    OUString aTableName = m_pCB_Tables->GetText();

    for( sal_Int32 nPos = 0; nPos < nCnt; ++nPos )
        InsertFreeIndex( RemoveTableIndex( aTableName, m_pLB_TableIndexes->GetEntry(0), true ) );

    checkButtons();
}

IMPL_LINK( ODbaseIndexDialog, OnListEntrySelected, ListBox*, /*NOTINTERESTEDIN*/ )
{
    checkButtons();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, TableSelectHdl, ComboBox*, pComboBox )
{
    // search the table
    TableInfoList::iterator aTablePos;
    if (!GetTable(pComboBox->GetText(), aTablePos))
        return 0L;

    // fill the listbox for the indexes
    m_pLB_TableIndexes->Clear();
    for (   TableIndexList::const_iterator aLoop = aTablePos->aIndexList.begin();
            aLoop != aTablePos->aIndexList.end();
            ++aLoop
        )
        m_pLB_TableIndexes->InsertEntry( aLoop->GetIndexFileName() );

    if ( aTablePos->aIndexList.size() )
        m_pLB_TableIndexes->SelectEntryPos(0);

    checkButtons();
    return 0;
}

void ODbaseIndexDialog::Init()
{
    m_pPB_OK->Disable();
    m_pIndexes->Disable();

    // All indices are first added to a list of free indices.
    // Afterwards, check the index of each table in the Inf-file.
    // These indices are removed from the list of free indices and
    // entered in the indexlist of the table.

    // if the string does not contain a path, cut the string
    INetURLObject aURL;
    aURL.SetSmartProtocol(INetProtocol::File);
    {
        SvtPathOptions aPathOptions;
        m_aDSN = aPathOptions.SubstituteVariable(m_aDSN);
    }
    aURL.SetSmartURL(m_aDSN);

    //  String aFileName = aURL.PathToFileName();
    m_aDSN = aURL.GetMainURL(INetURLObject::NO_DECODE);
    ::ucbhelper::Content aFile;
    bool bFolder=true;
    try
    {
        aFile = ::ucbhelper::Content(m_aDSN,Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext());
        bFolder = aFile.isFolder();
    }
    catch(Exception&)
    {
        return;
    }

    // first assume for all indexes they're free

    Sequence< OUString> aFolderContent( ::utl::LocalFileHelper::GetFolderContents(m_aDSN,bFolder));

    OUString aIndexExt("ndx");
    OUString aTableExt("dbf");

    ::std::vector< OUString > aUsedIndexes;

    const OUString *pBegin = aFolderContent.getConstArray();
    const OUString *pEnd   = pBegin + aFolderContent.getLength();
    aURL.SetSmartProtocol(INetProtocol::File);
    for(;pBegin != pEnd;++pBegin)
    {
        OUString aName;
        osl::FileBase::getSystemPathFromFileURL(pBegin->getStr(),aName);
        aURL.SetSmartURL(aName);
        OUString aExt = aURL.getExtension();
        if (aExt == aIndexExt)
        {
            m_aFreeIndexList.push_back( OTableIndex(aURL.getName()) );
        }
        else if (aExt == aTableExt)
        {
            m_aTableInfoList.push_back( OTableInfo(aURL.getName()) );
            OTableInfo& rTabInfo = m_aTableInfoList.back();

            // open the INF file
            aURL.setExtension("inf");
            OFileNotation aTransformer(aURL.GetURLNoPass(), OFileNotation::N_URL);
            Config aInfFile( aTransformer.get(OFileNotation::N_SYSTEM) );
            aInfFile.SetGroup( aGroupIdent );

            // fill the indexes list
            OString aNDX;
            sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
            OString aKeyName;
            OUString aEntry;

            for( sal_uInt16 nKey = 0; nKey < nKeyCnt; nKey++ )
            {
                // does the key point to an index file ?
                aKeyName = aInfFile.GetKeyName( nKey );
                aNDX = aKeyName.copy(0,3);

                // yes -> add to the tables index list
                if (aNDX == "NDX")
                {
                    aEntry = OStringToOUString(aInfFile.ReadKey(aKeyName), osl_getThreadTextEncoding());
                    rTabInfo.aIndexList.push_back( OTableIndex( aEntry ) );

                    // and remove it from the free index list
                    aUsedIndexes.push_back(aEntry);
                        // do this later below. We may not have encountered the index file, yet, thus we may not
                        // know the index as being free, yet
                }
            }
        }
    }

    for (   ::std::vector< OUString >::const_iterator aUsedIndex = aUsedIndexes.begin();
            aUsedIndex != aUsedIndexes.end();
            ++aUsedIndex
        )
        RemoveFreeIndex( *aUsedIndex, false );

    if (m_aTableInfoList.size())
    {
        m_pPB_OK->Enable();
        m_pIndexes->Enable();
    }

    checkButtons();
}

void ODbaseIndexDialog::SetCtrls()
{
    // ComboBox tables
    for (   TableInfoList::const_iterator aLoop = m_aTableInfoList.begin();
            aLoop != m_aTableInfoList.end();
            ++aLoop
        )
        m_pCB_Tables->InsertEntry( aLoop->aTableName );

    // put the first dataset into Edit
    if( m_aTableInfoList.size() )
    {
        const OTableInfo& rTabInfo = m_aTableInfoList.front();
        m_pCB_Tables->SetText( rTabInfo.aTableName );

        // build ListBox of the table indices
        for (   TableIndexList::const_iterator aIndex = rTabInfo.aIndexList.begin();
                aIndex != rTabInfo.aIndexList.end();
                ++aIndex
            )
            m_pLB_TableIndexes->InsertEntry( aIndex->GetIndexFileName() );

        if( rTabInfo.aIndexList.size() )
            m_pLB_TableIndexes->SelectEntryPos( 0 );
    }

    // ListBox of the free indices
    for (   TableIndexList::const_iterator aFree = m_aFreeIndexList.begin();
            aFree != m_aFreeIndexList.end();
            ++aFree
        )
        m_pLB_FreeIndexes->InsertEntry( aFree->GetIndexFileName() );

    if( m_aFreeIndexList.size() )
        m_pLB_FreeIndexes->SelectEntryPos( 0 );

    TableSelectHdl(m_pCB_Tables);
    checkButtons();
}

void OTableInfo::WriteInfFile( const OUString& rDSN ) const
{
    // open INF file
    INetURLObject aURL;
    aURL.SetSmartProtocol(INetProtocol::File);
    OUString aDsn = rDSN;
    {
        SvtPathOptions aPathOptions;
        aDsn = aPathOptions.SubstituteVariable(aDsn);
    }
    aURL.SetSmartURL(aDsn);
    aURL.Append(aTableName);
    aURL.setExtension("inf");

    OFileNotation aTransformer(aURL.GetURLNoPass(), OFileNotation::N_URL);
    Config aInfFile( aTransformer.get(OFileNotation::N_SYSTEM) );
    aInfFile.SetGroup( aGroupIdent );

    // first, delete all table indices
    OString aNDX;
    sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
    sal_uInt16 nKey = 0;

    while( nKey < nKeyCnt )
    {
        // Does the key point to an index file?...
        OString aKeyName = aInfFile.GetKeyName( nKey );
        aNDX = aKeyName.copy(0,3);

        //...if yes, delete index file, nKey is at subsequent key
        if (aNDX == "NDX")
        {
            aInfFile.DeleteKey(aKeyName);
            nKeyCnt--;
        }
        else
            nKey++;

    }

    // now add all saved indices
    sal_uInt16 nPos = 0;
    for (   TableIndexList::const_iterator aIndex = aIndexList.begin();
            aIndex != aIndexList.end();
            ++aIndex, ++nPos
        )
    {
        OStringBuffer aKeyName("NDX");
        if( nPos > 0 )  // first index contains no number
            aKeyName.append(static_cast<sal_Int32>(nPos));
        aInfFile.WriteKey(
            aKeyName.makeStringAndClear(),
            OUStringToOString(aIndex->GetIndexFileName(),
                osl_getThreadTextEncoding()));
    }

    aInfFile.Flush();

    // if only [dbase] is left in INF-file, delete file
    if(!nPos)
    {
        try
        {
            ::ucbhelper::Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
            aContent.executeCommand( OUString("delete"), makeAny( true ) );
        }
        catch (const Exception& e )
        {
            (void)e;  // make compiler happy
            // simply silent this. The strange algorithm here does a lot of
            // things even if no files at all were created or accessed, so it's
            // possible that the file we're trying to delete does not even
            // exist, and this is a valid condition.
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
