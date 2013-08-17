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
#include <tools/config.hxx>
#include <sfx2/app.hxx>
#include "moduledbu.hxx"
#include "dbu_dlg.hrc"
#include "dbfindex.hrc"
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

const OString aGroupIdent(RTL_CONSTASCII_STRINGPARAM("dBase III"));

DBG_NAME(ODbaseIndexDialog)

ODbaseIndexDialog::ODbaseIndexDialog( Window * pParent, String aDataSrcName )
    : ModalDialog( pParent, ModuleRes(DLG_DBASE_INDEXES) ),
    aPB_OK(             this, ModuleRes( PB_OK ) ),
    aPB_CANCEL(         this, ModuleRes( PB_CANCEL ) ),
    aPB_HELP(           this, ModuleRes( PB_HELP ) ),
    m_FT_Tables(        this, ModuleRes( FT_TABLES ) ),
    aCB_Tables(         this, ModuleRes( CB_TABLES ) ),
    m_FL_Indexes(       this, ModuleRes( FL_INDEXES ) ),
    m_FT_TableIndexes(  this, ModuleRes( FT_TABLEINDEXES ) ),
    aLB_TableIndexes(   this, ModuleRes( LB_TABLEINDEXES ) ),
    m_FT_AllIndexes(    this, ModuleRes( FT_ALLINDEXES ) ),
    aLB_FreeIndexes(    this, ModuleRes( LB_FREEINDEXES ) ),
    aIB_Add(            this, ModuleRes( IB_ADD ) ),
    aIB_Remove(         this, ModuleRes( IB_REMOVE ) ),
    aIB_AddAll(         this, ModuleRes( IB_ADDALL ) ),
    aIB_RemoveAll(      this, ModuleRes( IB_REMOVEALL ) ),
    m_aDSN(aDataSrcName),
    m_bCaseSensitiv(sal_True)
{
    DBG_CTOR(ODbaseIndexDialog,NULL);

    aCB_Tables.SetSelectHdl( LINK(this, ODbaseIndexDialog, TableSelectHdl) );
    aIB_Add.SetClickHdl( LINK(this, ODbaseIndexDialog, AddClickHdl) );
    aIB_Remove.SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveClickHdl) );
    aIB_AddAll.SetClickHdl( LINK(this, ODbaseIndexDialog, AddAllClickHdl) );
    aIB_RemoveAll.SetClickHdl( LINK(this, ODbaseIndexDialog, RemoveAllClickHdl) );
    aPB_OK.SetClickHdl( LINK(this, ODbaseIndexDialog, OKClickHdl) );

    aLB_FreeIndexes.SetSelectHdl( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );
    aLB_TableIndexes.SetSelectHdl( LINK(this, ODbaseIndexDialog, OnListEntrySelected) );

    aCB_Tables.SetDropDownLineCount(8);
    Init();
    SetCtrls();
    FreeResource();
}

ODbaseIndexDialog::~ODbaseIndexDialog()
{

    DBG_DTOR(ODbaseIndexDialog,NULL);
}

sal_Bool ODbaseIndexDialog::GetTable(const String& _rName, TableInfoListIterator& _rPosition)
{
    for (   _rPosition = m_aTableInfoList.begin();
            _rPosition != m_aTableInfoList.end();
            ++_rPosition
        )
    {
        if (m_bCaseSensitiv)
        {
            if (_rPosition->aTableName.Equals(_rName))
                return sal_True;
        }
        else
        {
            if (_rPosition->aTableName.EqualsIgnoreCaseAscii(_rName))
                return sal_True;
        }
    }
    return sal_False;
}

void ODbaseIndexDialog::checkButtons()
{
    aIB_Add.Enable(0 != aLB_FreeIndexes.GetSelectEntryCount());
    aIB_AddAll.Enable(0 != aLB_FreeIndexes.GetEntryCount());

    aIB_Remove.Enable(0 != aLB_TableIndexes.GetSelectEntryCount());
    aIB_RemoveAll.Enable(0 != aLB_TableIndexes.GetEntryCount());
}

OTableIndex ODbaseIndexDialog::implRemoveIndex(const String& _rName, TableIndexList& _rList, ListBox& _rDisplay, sal_Bool _bMustExist)
{
    OTableIndex aReturn;

    sal_Int32 nPos = 0;

    TableIndexListIterator aSearch;
    for (   aSearch = _rList.begin();
            aSearch != _rList.end();
            ++aSearch, ++nPos
        )
    {
        if ( m_bCaseSensitiv ? aSearch->GetIndexFileName().Equals(_rName) : aSearch->GetIndexFileName().EqualsIgnoreCaseAscii(_rName) )
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

OTableIndex ODbaseIndexDialog::RemoveTableIndex( const String& _rTableName, const String& _rIndexName, sal_Bool _bMustExist )
{
    OTableIndex aReturn;

    // does the table exist ?
    TableInfoListIterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return aReturn;

    return implRemoveIndex(_rIndexName, aTablePos->aIndexList, aLB_TableIndexes, _bMustExist);
}

void ODbaseIndexDialog::InsertTableIndex( const String& _rTableName, const OTableIndex& _rIndex)
{
    TableInfoListIterator aTablePos;
    if (!GetTable(_rTableName, aTablePos))
        return;

    implInsertIndex(_rIndex, aTablePos->aIndexList, aLB_TableIndexes);
}

IMPL_LINK( ODbaseIndexDialog, OKClickHdl, PushButton*, /*pButton*/ )
{
    // let all tables write their INF file

    for (   ConstTableInfoListIterator aLoop = m_aTableInfoList.begin();
            aLoop != m_aTableInfoList.end();
            ++aLoop
        )
        aLoop->WriteInfFile(m_aDSN);

    EndDialog();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, AddClickHdl, PushButton*, /*pButton*/ )
{
    String aSelection = aLB_FreeIndexes.GetSelectEntry();
    String aTableName = aCB_Tables.GetText();
    OTableIndex aIndex = RemoveFreeIndex( aSelection, sal_True );
    InsertTableIndex( aTableName, aIndex );

    checkButtons();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, RemoveClickHdl, PushButton*, /*pButton*/ )
{
    String aSelection = aLB_TableIndexes.GetSelectEntry();
    String aTableName = aCB_Tables.GetText();
    OTableIndex aIndex = RemoveTableIndex( aTableName, aSelection, sal_True );
    InsertFreeIndex( aIndex );

    checkButtons();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, AddAllClickHdl, PushButton*, /*pButton*/ )
{
    sal_uInt16 nCnt = aLB_FreeIndexes.GetEntryCount();
    String aTableName = aCB_Tables.GetText();

    for( sal_uInt16 nPos = 0; nPos < nCnt; ++nPos )
        InsertTableIndex( aTableName, RemoveFreeIndex( aLB_FreeIndexes.GetEntry(0), sal_True ) );

    checkButtons();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, RemoveAllClickHdl, PushButton*, /*pButton*/ )
{
    sal_uInt16 nCnt = aLB_TableIndexes.GetEntryCount();
    String aTableName = aCB_Tables.GetText();

    for( sal_uInt16 nPos = 0; nPos < nCnt; ++nPos )
        InsertFreeIndex( RemoveTableIndex( aTableName, aLB_TableIndexes.GetEntry(0), sal_True ) );

    checkButtons();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, OnListEntrySelected, ListBox*, /*NOTINTERESTEDIN*/ )
{
    checkButtons();
    return 0;
}

IMPL_LINK( ODbaseIndexDialog, TableSelectHdl, ComboBox*, pComboBox )
{
    // search the table
    TableInfoListIterator aTablePos;
    if (!GetTable(pComboBox->GetText(), aTablePos))
        return 0L;

    // fill the listbox for the indexes
    aLB_TableIndexes.Clear();
    for (   ConstTableIndexListIterator aLoop = aTablePos->aIndexList.begin();
            aLoop != aTablePos->aIndexList.end();
            ++aLoop
        )
        aLB_TableIndexes.InsertEntry( aLoop->GetIndexFileName() );

    if ( aTablePos->aIndexList.size() )
        aLB_TableIndexes.SelectEntryPos(0);

    checkButtons();
    return 0;
}

void ODbaseIndexDialog::Init()
{
    aPB_OK.Disable();
    m_FL_Indexes.Disable();
    m_FT_TableIndexes.Disable();
    aLB_TableIndexes.Disable();
    m_FT_AllIndexes.Disable();
    aLB_FreeIndexes.Disable();
    aIB_Add.Disable();
    aIB_Remove.Disable();
    aIB_AddAll.Disable();
    aIB_RemoveAll.Disable();

    // All indizes are first added to a list of free indizes.
    // Afterwards, check the index of each table in the Inf-file.
    // These indizes are removed from the list of free indizes and
    // entered in the indexlist of the table.

    // if the string does not contain a path, cut the string
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    {
        SvtPathOptions aPathOptions;
        m_aDSN = aPathOptions.SubstituteVariable(m_aDSN);
    }
    aURL.SetSmartURL(m_aDSN);

    //  String aFileName = aURL.PathToFileName();
    m_aDSN = aURL.GetMainURL(INetURLObject::NO_DECODE);
    ::ucbhelper::Content aFile;
    sal_Bool bFolder=sal_True;
    try
    {
        aFile = ::ucbhelper::Content(m_aDSN,Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext());
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

    ::std::vector< String > aUsedIndexes;

    const OUString *pBegin = aFolderContent.getConstArray();
    const OUString *pEnd   = pBegin + aFolderContent.getLength();
    aURL.SetSmartProtocol(INET_PROT_FILE);
    for(;pBegin != pEnd;++pBegin)
    {
        OUString aName;
        ::utl::LocalFileHelper::ConvertURLToPhysicalName(pBegin->getStr(),aName);
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
            String aEntry;

            for( sal_uInt16 nKey = 0; nKey < nKeyCnt; nKey++ )
            {
                // does the key point to an index file ?
                aKeyName = aInfFile.GetKeyName( nKey );
                aNDX = aKeyName.copy(0,3);

                // yes -> add to the tables index list
                if (aNDX.equalsL(RTL_CONSTASCII_STRINGPARAM("NDX")))
                {
                    aEntry = OStringToOUString(aInfFile.ReadKey(aKeyName), osl_getThreadTextEncoding());
                    rTabInfo.aIndexList.push_back( OTableIndex( aEntry ) );

                    // and remove it from the free index list
                    aUsedIndexes.push_back(aEntry);
                        // do this later below. We may not have encountered the index file, yet, thus we may not
                        // know the index as beeing free, yet
                }
            }
        }
    }

    for (   ::std::vector< String >::const_iterator aUsedIndex = aUsedIndexes.begin();
            aUsedIndex != aUsedIndexes.end();
            ++aUsedIndex
        )
        RemoveFreeIndex( *aUsedIndex, sal_False );

    if (m_aTableInfoList.size())
    {
        aPB_OK.Enable();
        m_FL_Indexes.Enable();
        m_FT_TableIndexes.Enable();
        aLB_TableIndexes.Enable();
        m_FT_AllIndexes.Enable();
        aLB_FreeIndexes.Enable();
    }

    checkButtons();
}

void ODbaseIndexDialog::SetCtrls()
{
    // ComboBox tables
    for (   ConstTableInfoListIterator aLoop = m_aTableInfoList.begin();
            aLoop != m_aTableInfoList.end();
            ++aLoop
        )
        aCB_Tables.InsertEntry( aLoop->aTableName );

    // put the first dataset into Edit
    if( m_aTableInfoList.size() )
    {
        const OTableInfo& rTabInfo = m_aTableInfoList.front();
        aCB_Tables.SetText( rTabInfo.aTableName );

        // build ListBox of the table indizes
        for (   ConstTableIndexListIterator aIndex = rTabInfo.aIndexList.begin();
                aIndex != rTabInfo.aIndexList.end();
                ++aIndex
            )
            aLB_TableIndexes.InsertEntry( aIndex->GetIndexFileName() );

        if( rTabInfo.aIndexList.size() )
            aLB_TableIndexes.SelectEntryPos( 0 );
    }

    // ListBox of the free indizes
    for (   ConstTableIndexListIterator aFree = m_aFreeIndexList.begin();
            aFree != m_aFreeIndexList.end();
            ++aFree
        )
        aLB_FreeIndexes.InsertEntry( aFree->GetIndexFileName() );

    if( m_aFreeIndexList.size() )
        aLB_FreeIndexes.SelectEntryPos( 0 );

    TableSelectHdl(&aCB_Tables);
    checkButtons();
}

void OTableInfo::WriteInfFile( const String& rDSN ) const
{
    // open INF file
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    String aDsn = rDSN;
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

    // first, delete all table indizes
    OString aNDX;
    sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
    sal_uInt16 nKey = 0;

    while( nKey < nKeyCnt )
    {
        // Does the key point to an index file?...
        OString aKeyName = aInfFile.GetKeyName( nKey );
        aNDX = aKeyName.copy(0,3);

        //...if yes, delete index file, nKey is at subsequent key
        if (aNDX.equalsL(RTL_CONSTASCII_STRINGPARAM("NDX")))
        {
            aInfFile.DeleteKey(aKeyName);
            nKeyCnt--;
        }
        else
            nKey++;

    }

    // now add all saved indizes
    sal_uInt16 nPos = 0;
    for (   ConstTableIndexListIterator aIndex = aIndexList.begin();
            aIndex != aIndexList.end();
            ++aIndex, ++nPos
        )
    {
        OStringBuffer aKeyName(RTL_CONSTASCII_STRINGPARAM("NDX"));
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
            aContent.executeCommand( OUString("delete"),makeAny( sal_Bool( sal_True ) ) );
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
