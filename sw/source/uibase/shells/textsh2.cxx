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

#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/request.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <comphelper/types.hxx>
#include <sfx2/frame.hxx>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <dbmgr.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <osl/diagnose.h>

#include <vcl/svapp.hxx>

#include <view.hxx>
#include <wrtsh.hxx>
#include <swtypes.hxx>
#include <cmdid.h>
#include <textsh.hxx>
#include <swabstdlg.hxx>

using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

struct DBTextStruct_Impl
{
    SwDBData aDBData;
    Sequence<Any> aSelection;
    Reference<XResultSet>   xCursor;
    Reference<XConnection>  xConnection;
};

void SwTextShell::ExecDB(SfxRequest const &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwDBManager* pDBManager = GetShell().GetDBManager();
    OUString sSourceArg, sCommandArg;
    sal_Int32 nCommandTypeArg = 0;

    const SfxPoolItem* pSourceItem = nullptr;
    const SfxPoolItem* pCursorItem = nullptr;
    const SfxPoolItem* pConnectionItem = nullptr;
    const SfxPoolItem* pCommandItem = nullptr;
    const SfxPoolItem* pCommandTypeItem = nullptr;
    const SfxPoolItem* pSelectionItem = nullptr;

    // first get the selection of rows to be inserted
    pArgs->GetItemState(FN_DB_DATA_SELECTION_ANY, false, &pSelectionItem);

    Sequence<Any> aSelection;
    if(pSelectionItem)
        static_cast<const SfxUnoAnyItem*>(pSelectionItem)->GetValue() >>= aSelection;

    // get the data source name
    pArgs->GetItemState(FN_DB_DATA_SOURCE_ANY, false, &pSourceItem);
    if(pSourceItem)
        static_cast<const SfxUnoAnyItem*>(pSourceItem)->GetValue() >>= sSourceArg;

    // get the command
    pArgs->GetItemState(FN_DB_DATA_COMMAND_ANY, false, &pCommandItem);
    if(pCommandItem)
        static_cast<const SfxUnoAnyItem*>(pCommandItem)->GetValue() >>= sCommandArg;

    // get the command type
    pArgs->GetItemState(FN_DB_DATA_COMMAND_TYPE_ANY, false, &pCommandTypeItem);
    if(pCommandTypeItem)
        static_cast<const SfxUnoAnyItem*>(pCommandTypeItem)->GetValue() >>= nCommandTypeArg;

    Reference<XConnection> xConnection;
    pArgs->GetItemState(FN_DB_CONNECTION_ANY, false, &pConnectionItem);
    if ( pConnectionItem )
        static_cast<const SfxUnoAnyItem*>(pConnectionItem)->GetValue() >>= xConnection;
    // may be we even get no connection
    if ( !xConnection.is() )
    {
        Reference<XDataSource> xSource;
        SwView &rSwView = GetView();
        xConnection = SwDBManager::GetConnection(sSourceArg, xSource, &rSwView);
    }
    if(!xConnection.is())
        return ;

    // get the cursor, we use to travel, may be NULL
    Reference<XResultSet> xCursor;
    pArgs->GetItemState(FN_DB_DATA_CURSOR_ANY, false, &pCursorItem);
    if ( pCursorItem )
        static_cast<const SfxUnoAnyItem*>(pCursorItem)->GetValue() >>= xCursor;

    switch (rReq.GetSlot())
    {
        case FN_QRY_INSERT:
            {
                if(pSourceItem && pCommandItem && pCommandTypeItem)
                {
                    DBTextStruct_Impl* pNew     = new DBTextStruct_Impl;
                    pNew->aDBData.sDataSource   = sSourceArg;
                    pNew->aDBData.sCommand      = sCommandArg;
                    pNew->aDBData.nCommandType  = nCommandTypeArg;
                    pNew->aSelection            = aSelection;
                    //if the cursor is NULL, it must be created inside InsertDBTextHdl
                    // because it called via a PostUserEvent
                    pNew->xCursor               = xCursor;
                    pNew->xConnection           = xConnection;

                    Application::PostUserEvent( LINK( this, SwBaseShell, InsertDBTextHdl ), pNew );
                    // the pNew will be removed in InsertDBTextHdl !!
                }
            }
            break;

        case FN_QRY_MERGE_FIELD:
            {
                // we don't get any cursor, so we must create our own
                bool bDisposeResultSet = false;
                if ( !xCursor.is() )
                {
                    SwView &rSwView = GetView();
                    xCursor = SwDBManager::createCursor(sSourceArg,sCommandArg,nCommandTypeArg,xConnection,&rSwView);
                    bDisposeResultSet = xCursor.is();
                }

                ODataAccessDescriptor aDescriptor;
                aDescriptor.setDataSource(sSourceArg);
                aDescriptor[DataAccessDescriptorProperty::Command]      <<= sCommandArg;
                aDescriptor[DataAccessDescriptorProperty::Cursor]       <<= xCursor;
                aDescriptor[DataAccessDescriptorProperty::Selection]    <<= aSelection;
                aDescriptor[DataAccessDescriptorProperty::CommandType]  <<= nCommandTypeArg;

                SwMergeDescriptor aMergeDesc( DBMGR_MERGE, *GetShellPtr(), aDescriptor );
                pDBManager->Merge(aMergeDesc);

                if ( bDisposeResultSet )
                    ::comphelper::disposeComponent(xCursor);
            }
            break;

        case FN_QRY_INSERT_FIELD:
            {
                const SfxPoolItem* pColumnItem = nullptr;
                const SfxPoolItem* pColumnNameItem = nullptr;

                pArgs->GetItemState(FN_DB_COLUMN_ANY, false, &pColumnItem);
                pArgs->GetItemState(FN_DB_DATA_COLUMN_NAME_ANY, false, &pColumnNameItem);

                OUString sColumnName;
                if(pColumnNameItem)
                    static_cast<const SfxUnoAnyItem*>(pColumnNameItem)->GetValue() >>= sColumnName;
                OUString sDBName = sSourceArg + OUStringChar(DB_DELIM)
                    + sCommandArg + OUStringChar(DB_DELIM)
                    + OUString::number(nCommandTypeArg)
                    + OUStringChar(DB_DELIM) + sColumnName;

                SwFieldMgr aFieldMgr(GetShellPtr());
                SwInsertField_Data aData(SwFieldTypesEnum::Database, 0, sDBName, OUString(), 0);
                if(pConnectionItem)
                    aData.m_aDBConnection = static_cast<const SfxUnoAnyItem*>(pConnectionItem)->GetValue();
                if(pColumnItem)
                    aData.m_aDBColumn = static_cast<const SfxUnoAnyItem*>(pColumnItem)->GetValue();
                aFieldMgr.InsertField(aData);
                SfxViewFrame* pViewFrame = GetView().GetViewFrame();
                uno::Reference< XDispatchRecorder > xRecorder =
                        pViewFrame->GetBindings().GetRecorder();
                if ( xRecorder.is() )
                {
                    SfxRequest aReq( pViewFrame, FN_INSERT_DBFIELD );
                    aReq.AppendItem( SfxUInt16Item(FN_PARAM_FIELD_TYPE, static_cast<sal_uInt16>(SwFieldTypesEnum::Database)));
                    aReq.AppendItem( SfxStringItem( FN_INSERT_DBFIELD, sDBName ));
                    aReq.AppendItem( SfxStringItem( FN_PARAM_1, sCommandArg ));
                    aReq.AppendItem( SfxStringItem( FN_PARAM_2, sColumnName ));
                    aReq.AppendItem( SfxInt32Item( FN_PARAM_3, nCommandTypeArg));
                    aReq.Done();
                }
            }
            break;

        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
}

IMPL_LINK( SwBaseShell, InsertDBTextHdl, void*, p, void )
{
    DBTextStruct_Impl* pDBStruct = static_cast<DBTextStruct_Impl*>(p);
    if( pDBStruct )
    {
        bool bDispose = false;
        Reference< sdbc::XConnection> xConnection = pDBStruct->xConnection;
        Reference<XDataSource> xSource = SwDBManager::getDataSourceAsParent(xConnection,pDBStruct->aDBData.sDataSource);
        // #111987# the connection is disposed and so no parent has been found
        if(xConnection.is() && !xSource.is())
            return;

        if ( !xConnection.is()  )
        {
            SwView &rSwView = GetView();
            xConnection = SwDBManager::GetConnection(pDBStruct->aDBData.sDataSource, xSource, &rSwView);
            bDispose = true;
        }

        Reference< XColumnsSupplier> xColSupp;
        if(xConnection.is())
            xColSupp = SwDBManager::GetColumnSupplier(xConnection,
                                    pDBStruct->aDBData.sCommand,
                                    pDBStruct->aDBData.nCommandType == CommandType::QUERY ?
                                        SwDBSelect::QUERY : SwDBSelect::TABLE);

        if( xColSupp.is() )
        {
            SwDBData aDBData = pDBStruct->aDBData;
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            ScopedVclPtr<AbstractSwInsertDBColAutoPilot>pDlg (pFact->CreateSwInsertDBColAutoPilot(GetView(),
                                                                                                xSource,
                                                                                                xColSupp,
                                                                                                aDBData));
            if( RET_OK == pDlg->Execute() )
            {
                Reference <XResultSet> xResSet = pDBStruct->xCursor;
                pDlg->DataToDoc( pDBStruct->aSelection, xSource, xConnection, xResSet);
            }
        }
        if ( bDispose )
            ::comphelper::disposeComponent(xConnection);
    }

    delete pDBStruct;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
