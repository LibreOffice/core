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

#include <cstdarg>

#include <svtools/svmedit.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/event.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/itemset.hxx>
#include <sfx2/request.hxx>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <sfx2/frame.hxx>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include "dbmgr.hxx"
#include <comphelper/uno3.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <memory>

#include <vcl/svapp.hxx>

#include "view.hxx"
#include "wrtsh.hxx"
#include "swtypes.hxx"
#include "cmdid.h"
#include "swevent.hxx"
#include "shells.hrc"
#include "textsh.hxx"
#include "swabstdlg.hxx"
#include "dbui.hrc"

#include <unomid.h>

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

void SwTextShell::ExecDB(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwNewDBMgr* pNewDBMgr = GetShell().GetNewDBMgr();
    sal_uInt16 nSlot = rReq.GetSlot();
    OUString sSourceArg, sCommandArg;
    sal_Int32 nCommandTypeArg = 0;

    const SfxPoolItem* pSourceItem = 0;
    const SfxPoolItem* pCursorItem = 0;
    const SfxPoolItem* pConnectionItem = 0;
    const SfxPoolItem* pCommandItem = 0;
    const SfxPoolItem* pCommandTypeItem = 0;
    const SfxPoolItem* pSelectionItem = 0;

    // first get the selection of rows to be inserted
    pArgs->GetItemState(FN_DB_DATA_SELECTION_ANY, sal_False, &pSelectionItem);

    Sequence<Any> aSelection;
    if(pSelectionItem)
        ((SfxUsrAnyItem*)pSelectionItem)->GetValue() >>= aSelection;

    // get the data source name
    pArgs->GetItemState(FN_DB_DATA_SOURCE_ANY, sal_False, &pSourceItem);
    if(pSourceItem)
        ((const SfxUsrAnyItem*)pSourceItem)->GetValue() >>= sSourceArg;

    // get the command
    pArgs->GetItemState(FN_DB_DATA_COMMAND_ANY, sal_False, &pCommandItem);
    if(pCommandItem)
        ((const SfxUsrAnyItem*)pCommandItem)->GetValue() >>= sCommandArg;

    // get the command type
    pArgs->GetItemState(FN_DB_DATA_COMMAND_TYPE_ANY, sal_False, &pCommandTypeItem);
    if(pCommandTypeItem)
        ((const SfxUsrAnyItem*)pCommandTypeItem)->GetValue() >>= nCommandTypeArg;

    Reference<XConnection> xConnection;
    pArgs->GetItemState(FN_DB_CONNECTION_ANY, sal_False, &pConnectionItem);
    if ( pConnectionItem )
        ((const SfxUsrAnyItem*)pConnectionItem)->GetValue() >>= xConnection;
    // may be we even get no connection
    if ( !xConnection.is() )
    {
        Reference<XDataSource> xSource;
        xConnection = pNewDBMgr->GetConnection(sSourceArg, xSource);
    }
    if(!xConnection.is())
        return ;

    // get the cursor, we use to travel, may be NULL
    Reference<XResultSet> xCursor;
    pArgs->GetItemState(FN_DB_DATA_CURSOR_ANY, sal_False, &pCursorItem);
    if ( pCursorItem )
        ((const SfxUsrAnyItem*)pCursorItem)->GetValue() >>= xCursor;

    switch (nSlot)
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

                    Application::PostUserEvent( STATIC_LINK( this, SwBaseShell,
                                            InsertDBTextHdl ), pNew );
                    // the pNew will be removed in InsertDBTextHdl !!
                }
            }
            break;

        case FN_QRY_MERGE_FIELD:
            {
                // we don't get any cursor, so we must create our own
                sal_Bool bDisposeResultSet = sal_False;
                if ( !xCursor.is() )
                {
                    xCursor = SwNewDBMgr::createCursor(sSourceArg,sCommandArg,nCommandTypeArg,xConnection);
                    bDisposeResultSet = xCursor.is();
                }

                ODataAccessDescriptor aDescriptor;
                aDescriptor.setDataSource(sSourceArg);
                aDescriptor[daCommand]      <<= sCommandArg;
                aDescriptor[daCursor]       <<= xCursor;
                aDescriptor[daSelection]    <<= aSelection;
                aDescriptor[daCommandType]  <<= nCommandTypeArg;

                SwMergeDescriptor aMergeDesc( DBMGR_MERGE, *GetShellPtr(), aDescriptor );
                pNewDBMgr->MergeNew(aMergeDesc);

                if ( bDisposeResultSet )
                    ::comphelper::disposeComponent(xCursor);
            }
            break;

        case FN_QRY_INSERT_FIELD:
            {
                const SfxPoolItem* pColumnItem = 0;
                const SfxPoolItem* pColumnNameItem = 0;

                pArgs->GetItemState(FN_DB_COLUMN_ANY, sal_False, &pColumnItem);
                pArgs->GetItemState(FN_DB_DATA_COLUMN_NAME_ANY, sal_False, &pColumnNameItem);

                OUString sColumnName;
                if(pColumnNameItem)
                    ((SfxUsrAnyItem*)pColumnNameItem)->GetValue() >>= sColumnName;
                String sDBName = sSourceArg;
                sDBName += DB_DELIM;
                sDBName += (String)sCommandArg;
                sDBName += DB_DELIM;
                sDBName += OUString::number(nCommandTypeArg);
                sDBName += DB_DELIM;
                sDBName += (String)sColumnName;

                SwFldMgr aFldMgr(GetShellPtr());
                SwInsertFld_Data aData(TYP_DBFLD, 0, sDBName, aEmptyStr, 0);
                if(pConnectionItem)
                    aData.aDBConnection = ((SfxUsrAnyItem*)pConnectionItem)->GetValue();
                if(pColumnItem)
                    aData.aDBColumn = ((SfxUsrAnyItem*)pColumnItem)->GetValue();
                aFldMgr.InsertFld(aData);
                SfxViewFrame* pViewFrame = GetView().GetViewFrame();
                uno::Reference< XDispatchRecorder > xRecorder =
                        pViewFrame->GetBindings().GetRecorder();
                if ( xRecorder.is() )
                {
                    SfxRequest aReq( pViewFrame, FN_INSERT_DBFIELD );
                    aReq.AppendItem( SfxUInt16Item(FN_PARAM_FIELD_TYPE, TYP_DBFLD));
                    aReq.AppendItem( SfxStringItem( FN_INSERT_DBFIELD, sDBName ));
                    aReq.AppendItem( SfxStringItem( FN_PARAM_1, sCommandArg ));
                    aReq.AppendItem( SfxStringItem( FN_PARAM_2, sColumnName ));
                    aReq.AppendItem( SfxInt32Item( FN_PARAM_3, nCommandTypeArg));
                    aReq.Done();
                }
            }
            break;

        default:
            OSL_ENSURE(!this, "wrong dispatcher");
            return;
    }
}

IMPL_STATIC_LINK( SwBaseShell, InsertDBTextHdl, DBTextStruct_Impl*, pDBStruct )
{
    if( pDBStruct )
    {
        bool bDispose = false;
        Reference< sdbc::XConnection> xConnection = pDBStruct->xConnection;
        Reference<XDataSource> xSource = SwNewDBMgr::getDataSourceAsParent(xConnection,pDBStruct->aDBData.sDataSource);
        // #111987# the connection is disposed an so no parent has been found
        if(xConnection.is() && !xSource.is())
            return 0;

        if ( !xConnection.is()  )
        {
            xConnection = SwNewDBMgr::GetConnection(pDBStruct->aDBData.sDataSource, xSource);
            bDispose = true;
        }

        Reference< XColumnsSupplier> xColSupp;
        if(xConnection.is())
            xColSupp = SwNewDBMgr::GetColumnSupplier(xConnection,
                                    pDBStruct->aDBData.sCommand,
                                    pDBStruct->aDBData.nCommandType == CommandType::QUERY ?
                                        SW_DB_SELECT_QUERY : SW_DB_SELECT_TABLE);

        if( xColSupp.is() )
        {
            SwDBData aDBData = pDBStruct->aDBData;
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
            ::std::auto_ptr<AbstractSwInsertDBColAutoPilot>pDlg (pFact->CreateSwInsertDBColAutoPilot(pThis->GetView(),
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
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
