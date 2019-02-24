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
#include <dbu_dlg.hxx>
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


ODbaseIndexDialog::ODbaseIndexDialog(weld::Window * pParent, const OUString& aDataSrcName)
    : GenericDialogController(pParent, "dbaccess/ui/dbaseindexdialog.ui", "DBaseIndexDialog")
    , m_aDSN(aDataSrcName)
    , m_xPB_OK(m_xBuilder->weld_button("ok"))
    , m_xCB_Tables(m_xBuilder->weld_combo_box("table"))
    , m_xIndexes(m_xBuilder->weld_widget("frame"))
    , m_xLB_TableIndexes(m_xBuilder->weld_tree_view("tableindex"))
    , m_xLB_FreeIndexes(m_xBuilder->weld_tree_view("freeindex"))
    , m_xAdd(m_xBuilder->weld_button("add"))
    , m_xRemove(m_xBuilder->weld_button("remove"))
    , m_xAddAll(m_xBuilder->weld_button("addall"))
    , m_xRemoveAll(m_xBuilder->weld_button("removeall"))
{
    int nWidth = m_xLB_TableIndexes->get_approximate_digit_width() * 18;
    int nHeight = m_xLB_TableIndexes->get_height_rows(10);
    m_xLB_TableIndexes->set_size_request(nWidth, nHeight);
    m_xLB_FreeIndexes->set_size_request(nWidth, nHeight);

    m_xCB_Tables->connect_changed( LINK(this, ODbaseIndexDialog, TableSelectHdl) );
    m_xAdd->connect_clicked( LINK(this, ODbaseIndexDialog, AddClickHdl) );
    m_xRemove->connect_clicked( LINK(this, ODbaseIndexDialog, RemoveClickHdl) );
    m_xAddAll->connect_clicked( LINK(this, ODbaseIndexDialog, AddAllClickHdl) );
    m_xRemoveAll->connect_clicked( LINK(this, ODbaseIndexDialog, RemoveAllClickHdl) );
    m_xPB_OK->connect_clicked( LINK(this, ODbaseIndexDialog, OKClickHdl) );

    m_xLB_FreeIndexes->connect_changed( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );
    m_xLB_TableIndexes->connect_changed( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );

    Init();
    SetCtrls();
}

ODbaseIndexDialog::~ODbaseIndexDialog()
{
}

void ODbaseIndexDialog::checkButtons()
{
    m_xAdd->set_sensitive(0 != m_xLB_FreeIndexes->count_selected_rows());
    m_xAddAll->set_sensitive(0 != m_xLB_FreeIndexes->n_children());

    m_xRemove->set_sensitive(0 != m_xLB_TableIndexes->count_selected_rows());
    m_xRemoveAll->set_sensitive(0 != m_xLB_TableIndexes->n_children());
}

OTableIndex ODbaseIndexDialog::implRemoveIndex(const OUString& _rName, TableIndexList& _rList, weld::TreeView& _rDisplay, bool _bMustExist)
{
    OTableIndex aReturn;

    TableIndexList::iterator aSearch = std::find_if(_rList.begin(), _rList.end(),
        [&_rName](const OTableIndex& rIndex) { return rIndex.GetIndexFileName() == _rName; });
    if (aSearch != _rList.end())
    {
        sal_Int32 nPos = static_cast<sal_Int32>(std::distance(_rList.begin(), aSearch));

        aReturn = *aSearch;

        _rList.erase(aSearch);
        _rDisplay.remove_text(_rName);

        // adjust selection if necessary
        if (static_cast<sal_uInt32>(nPos) == _rList.size())
            _rDisplay.select(static_cast<sal_uInt16>(nPos)-1);
        else
            _rDisplay.select(static_cast<sal_uInt16>(nPos));
    }

    OSL_ENSURE(!_bMustExist || (aSearch != _rList.end()), "ODbaseIndexDialog::implRemoveIndex : did not find the index!");
    return aReturn;
}

void ODbaseIndexDialog::implInsertIndex(const OTableIndex& _rIndex, TableIndexList& _rList, weld::TreeView& _rDisplay)
{
    _rList.push_front(_rIndex);
    _rDisplay.append_text(_rIndex.GetIndexFileName());
    _rDisplay.select(0);
}

OTableIndex ODbaseIndexDialog::RemoveTableIndex( const OUString& _rTableName, const OUString& _rIndexName )
{
    OTableIndex aReturn;

    // does the table exist ?
    TableInfoList::iterator aTablePos = std::find_if(m_aTableInfoList.begin(), m_aTableInfoList.end(),
                                           [&] (const OTableInfo& arg) { return arg.aTableName == _rTableName; });

    if (aTablePos == m_aTableInfoList.end())
        return aReturn;

    return implRemoveIndex(_rIndexName, aTablePos->aIndexList, *m_xLB_TableIndexes, true/*_bMustExist*/);
}

void ODbaseIndexDialog::InsertTableIndex( const OUString& _rTableName, const OTableIndex& _rIndex)
{
    TableInfoList::iterator aTablePos = std::find_if(m_aTableInfoList.begin(), m_aTableInfoList.end(),
                                           [&] (const OTableInfo& arg) { return arg.aTableName == _rTableName; });

    if (aTablePos == m_aTableInfoList.end())
        return;

    implInsertIndex(_rIndex, aTablePos->aIndexList, *m_xLB_TableIndexes);
}

IMPL_LINK_NOARG(ODbaseIndexDialog, OKClickHdl, weld::Button&, void)
{
    // let all tables write their INF file

    for (auto const& tableInfo : m_aTableInfoList)
        tableInfo.WriteInfFile(m_aDSN);

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ODbaseIndexDialog, AddClickHdl, weld::Button&, void)
{
    OUString aSelection = m_xLB_FreeIndexes->get_selected_text();
    OUString aTableName = m_xCB_Tables->get_active_text();
    OTableIndex aIndex = RemoveFreeIndex( aSelection, true );
    InsertTableIndex( aTableName, aIndex );

    checkButtons();
}

IMPL_LINK_NOARG(ODbaseIndexDialog, RemoveClickHdl, weld::Button&, void)
{
    OUString aSelection = m_xLB_TableIndexes->get_selected_text();
    OUString aTableName = m_xCB_Tables->get_active_text();
    OTableIndex aIndex = RemoveTableIndex( aTableName, aSelection );
    InsertFreeIndex( aIndex );

    checkButtons();
}

IMPL_LINK_NOARG(ODbaseIndexDialog, AddAllClickHdl, weld::Button&, void)
{
    const sal_Int32 nCnt = m_xLB_FreeIndexes->n_children();
    OUString aTableName = m_xCB_Tables->get_active_text();

    for (sal_Int32 nPos = 0; nPos < nCnt; ++nPos)
        InsertTableIndex(aTableName, RemoveFreeIndex(m_xLB_FreeIndexes->get_text(0), true));

    checkButtons();
}

IMPL_LINK_NOARG(ODbaseIndexDialog, RemoveAllClickHdl, weld::Button&, void)
{
    const sal_Int32 nCnt = m_xLB_TableIndexes->n_children();
    OUString aTableName = m_xCB_Tables->get_active_text();

    for (sal_Int32 nPos = 0; nPos < nCnt; ++nPos)
        InsertFreeIndex(RemoveTableIndex(aTableName, m_xLB_TableIndexes->get_text(0)));

    checkButtons();
}

IMPL_LINK_NOARG(ODbaseIndexDialog, OnListEntrySelected, weld::TreeView&, void)
{
    checkButtons();
}

IMPL_LINK(ODbaseIndexDialog, TableSelectHdl, weld::ComboBox&, rComboBox, void)
{
    // search the table
    TableInfoList::iterator aTablePos = std::find_if(m_aTableInfoList.begin(), m_aTableInfoList.end(),
                                           [&] (const OTableInfo& arg) { return arg.aTableName == rComboBox.get_active_text() ; });

    if (aTablePos == m_aTableInfoList.end())
        return;

    // fill the listbox for the indexes
    m_xLB_TableIndexes->clear();
    for (auto const& index : aTablePos->aIndexList)
        m_xLB_TableIndexes->append_text(index.GetIndexFileName());

    if (!aTablePos->aIndexList.empty())
        m_xLB_TableIndexes->select(0);

    checkButtons();
}

void ODbaseIndexDialog::Init()
{
    m_xPB_OK->set_sensitive(false);
    m_xIndexes->set_sensitive(false);

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
    m_aDSN = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
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

    OUString const aIndexExt("ndx");
    OUString const aTableExt("dbf");

    std::vector< OUString > aUsedIndexes;

    aURL.SetSmartProtocol(INetProtocol::File);
    for(const OUString& rURL : ::utl::LocalFileHelper::GetFolderContents(m_aDSN, bFolder))
    {
        OUString aName;
        osl::FileBase::getSystemPathFromFileURL(rURL,aName);
        aURL.SetSmartURL(aName);
        OUString aExt = aURL.getExtension();
        if (aExt == aIndexExt)
        {
            m_aFreeIndexList.emplace_back(aURL.getName() );
        }
        else if (aExt == aTableExt)
        {
            m_aTableInfoList.emplace_back(aURL.getName() );
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
                    rTabInfo.aIndexList.emplace_back( aEntry );

                    // and remove it from the free index list
                    aUsedIndexes.push_back(aEntry);
                        // do this later below. We may not have encountered the index file, yet, thus we may not
                        // know the index as being free, yet
                }
            }
        }
    }

    for (auto const& usedIndex : aUsedIndexes)
        RemoveFreeIndex( usedIndex, false );

    if (!m_aTableInfoList.empty())
    {
        m_xPB_OK->set_sensitive(true);
        m_xIndexes->set_sensitive(true);
    }

    checkButtons();
}

void ODbaseIndexDialog::SetCtrls()
{
    // ComboBox tables
    for (auto const& tableInfo : m_aTableInfoList)
        m_xCB_Tables->append_text(tableInfo.aTableName);

    // put the first dataset into Edit
    if (!m_aTableInfoList.empty())
    {
        const OTableInfo& rTabInfo = m_aTableInfoList.front();
        m_xCB_Tables->set_entry_text(rTabInfo.aTableName);

        // build ListBox of the table indices
        for (auto const& index : rTabInfo.aIndexList)
            m_xLB_TableIndexes->append_text(index.GetIndexFileName());

        if (!rTabInfo.aIndexList.empty())
            m_xLB_TableIndexes->select(0);
    }

    // ListBox of the free indices
    for (auto const& freeIndex : m_aFreeIndexList)
        m_xLB_FreeIndexes->append_text(freeIndex.GetIndexFileName());

    if (!m_aFreeIndexList.empty())
        m_xLB_FreeIndexes->select(0);

    TableSelectHdl(*m_xCB_Tables);
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
    for (auto const& index : aIndexList)
    {
        OStringBuffer aKeyName("NDX");
        if( nPos > 0 )  // first index contains no number
            aKeyName.append(static_cast<sal_Int32>(nPos));
        aInfFile.WriteKey(
            aKeyName.makeStringAndClear(),
            OUStringToOString(index.GetIndexFileName(),
                osl_getThreadTextEncoding()));
        ++nPos;
    }

    aInfFile.Flush();

    // if only [dbase] is left in INF-file, delete file
    if(!nPos)
    {
        try
        {
            ::ucbhelper::Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>(), comphelper::getProcessComponentContext());
            aContent.executeCommand( "delete", makeAny( true ) );
        }
        catch (const Exception& )
        {
            // simply silent this. The strange algorithm here does a lot of
            // things even if no files at all were created or accessed, so it's
            // possible that the file we're trying to delete does not even
            // exist, and this is a valid condition.
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
