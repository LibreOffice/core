/*************************************************************************
 *
 *  $RCSfile: textsh2.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:23:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <svtools/svmedit.hxx>
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDATABASEACCESS_HPP_
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#include "dbmgr.hxx"
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#include <memory>

#include "view.hxx"
#include "wrtsh.hxx"
#include "swtypes.hxx"
#include "cmdid.h"
#include "swevent.hxx"
#include "shells.hrc"
#include "textsh.hxx"
//CHINA001 #include "dbinsdlg.hxx"
#include "swabstdlg.hxx" //CHINA001
#include "dbui.hrc" //CHINA001

using namespace rtl;
using namespace svx;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;

#define C2U(cChar) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(cChar))
#define C2S(cChar) UniString::CreateFromAscii(cChar)
#define DB_DD_DELIM 0x0b

struct DBTextStruct_Impl
{
    SwDBData aDBData;
    Sequence<Any> aSelection;
    Reference<XResultSet>   xCursor;
    Reference<XConnection>  xConnection;
};
inline void AddSelList( List& rLst, long nRow )
{
    rLst.Insert( (void*)nRow , LIST_APPEND );
}
void SwTextShell::ExecDB(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwNewDBMgr* pNewDBMgr = GetShell().GetNewDBMgr();
    USHORT nSlot = rReq.GetSlot();
    OUString sSourceArg, sCommandArg;
    sal_Int32 nCommandTypeArg = 0;

    const SfxPoolItem* pSourceItem = 0;
    const SfxPoolItem* pCursorItem = 0;
    const SfxPoolItem* pConnectionItem = 0;
    const SfxPoolItem* pCommandItem = 0;
    const SfxPoolItem* pCommandTypeItem = 0;
    const SfxPoolItem* pSelectionItem = 0;

    // first get the selection of rows to be inserted
    pArgs->GetItemState(FN_DB_DATA_SELECTION_ANY, FALSE, &pSelectionItem);

    Sequence<Any> aSelection;
    if(pSelectionItem)
        ((SfxUsrAnyItem*)pSelectionItem)->GetValue() >>= aSelection;

    // get the data source name
    pArgs->GetItemState(FN_DB_DATA_SOURCE_ANY, FALSE, &pSourceItem);
    if(pSourceItem)
        ((const SfxUsrAnyItem*)pSourceItem)->GetValue() >>= sSourceArg;

    // get the command
    pArgs->GetItemState(FN_DB_DATA_COMMAND_ANY, FALSE, &pCommandItem);
    if(pCommandItem)
        ((const SfxUsrAnyItem*)pCommandItem)->GetValue() >>= sCommandArg;

    // get the command type
    pArgs->GetItemState(FN_DB_DATA_COMMAND_TYPE_ANY, FALSE, &pCommandTypeItem);
    if(pCommandTypeItem)
        ((const SfxUsrAnyItem*)pCommandTypeItem)->GetValue() >>= nCommandTypeArg;

    Reference<XConnection> xConnection;
    pArgs->GetItemState(FN_DB_CONNECTION_ANY, FALSE, &pConnectionItem);
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
    pArgs->GetItemState(FN_DB_DATA_CURSOR_ANY, FALSE, &pCursorItem);
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
                BOOL bDisposeResultSet = FALSE;
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

                pNewDBMgr->MergeNew(DBMGR_MERGE, *GetShellPtr(), aDescriptor);

                if ( bDisposeResultSet )
                    ::comphelper::disposeComponent(xCursor);
            }
            break;

        case FN_QRY_INSERT_FIELD:
            {
                const SfxPoolItem* pColumnItem = 0;
                const SfxPoolItem* pColumnNameItem = 0;

                pArgs->GetItemState(FN_DB_COLUMN_ANY, FALSE, &pColumnItem);
                pArgs->GetItemState(FN_DB_DATA_COLUMN_NAME_ANY, FALSE, &pColumnNameItem);

                OUString sColumnName;
                if(pColumnNameItem)
                    ((SfxUsrAnyItem*)pColumnNameItem)->GetValue() >>= sColumnName;
                String sDBName = sSourceArg;
                sDBName += DB_DELIM;
                sDBName += (String)sCommandArg;
                sDBName += DB_DELIM;
                sDBName += String::CreateFromInt32(nCommandTypeArg);
                sDBName += DB_DELIM;
                sDBName += (String)sColumnName;

                SwFldMgr aFldMgr(GetShellPtr());
                SwInsertFld_Data aData(TYP_DBFLD, 0, sDBName, aEmptyStr, 0, FALSE, TRUE);
                if(pConnectionItem)
                    aData.aDBConnection = ((SfxUsrAnyItem*)pConnectionItem)->GetValue();
                if(pColumnItem)
                    aData.aDBColumn = ((SfxUsrAnyItem*)pColumnItem)->GetValue();
                aFldMgr.InsertFld(aData);
                SfxViewFrame* pViewFrame = GetView().GetViewFrame();
                com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder =
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
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

IMPL_STATIC_LINK( SwBaseShell, InsertDBTextHdl, DBTextStruct_Impl*, pDBStruct )
{
    if( pDBStruct )
    {
        sal_Bool bDispose = sal_False;
        Reference< sdbc::XConnection> xConnection = pDBStruct->xConnection;
        Reference<XDataSource> xSource = SwNewDBMgr::getDataSourceAsParent(xConnection,pDBStruct->aDBData.sDataSource);
        // #111987# the connection is disposed an so no parent has been found
        if(xConnection.is() && !xSource.is())
            return 0;

        if ( !xConnection.is()  )
        {
            xConnection = SwNewDBMgr::GetConnection(pDBStruct->aDBData.sDataSource, xSource);
            bDispose = sal_True;
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
//CHINA001          ::std::auto_ptr<SwInsertDBColAutoPilot> pDlg( new SwInsertDBColAutoPilot(
//CHINA001          pThis->GetView(),
//CHINA001          xSource,
//CHINA001          xColSupp,
//CHINA001          aDBData ) );
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();//CHINA001
            DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");//CHINA001
            ::std::auto_ptr<AbstractSwInsertDBColAutoPilot>pDlg (pFact->CreateSwInsertDBColAutoPilot( pThis->GetView(),
                                                                                                xSource,
                                                                                                xColSupp,
                                                                                                aDBData,
                                                                                                ResId( DLG_AP_INSERT_DB_SEL ))); //CHINA001
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




