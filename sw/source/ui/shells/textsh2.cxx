/*************************************************************************
 *
 *  $RCSfile: textsh2.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: os $ $Date: 2001-10-16 11:11:34 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#if STLPORT_VERSION>=321
#include <cstdarg>
#endif

#include <svtools/svmedit.hxx>
#ifndef _SBASLTID_HRC //autogen
#include <offmgr/sbasltid.hrc>
#endif
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
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
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


#include "view.hxx"
#include "wrtsh.hxx"
#include "swtypes.hxx"
#include "cmdid.h"
#include "swevent.hxx"
#include "shells.hrc"
#include "textsh.hxx"
#include "dbinsdlg.hxx"

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
#define C2S(cChar) UniString::CreateFromAscii(cChar)
#define DB_DD_DELIM 0x0b

struct DBTextStruct_Impl
{
    SwDBData aDBData;
    Sequence<Any> aSelection;
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
    const SfxPoolItem* pCommandItem = 0;
    const SfxPoolItem* pCommandTypeItem = 0;
    const SfxPoolItem* pSelectionItem = 0;
    pArgs->GetItemState(FN_DB_DATA_SELECTION_ANY, FALSE, &pSelectionItem);
    Sequence<Any> aSelection;
    if(pSelectionItem)
    {
        ((SfxUsrAnyItem*)pSelectionItem)->GetValue() >>= aSelection;
    }
    pArgs->GetItemState(FN_DB_DATA_SOURCE_ANY, FALSE, &pSourceItem);
    if(pSourceItem)
        ((const SfxUsrAnyItem*)pSourceItem)->GetValue() >>= sSourceArg;
    pArgs->GetItemState(FN_DB_DATA_COMMAND_ANY, FALSE, &pCommandItem);
    if(pCommandItem)
        ((const SfxUsrAnyItem*)pCommandItem)->GetValue() >>= sCommandArg;
    pArgs->GetItemState(FN_DB_DATA_COMMAND_TYPE_ANY, FALSE, &pCommandTypeItem);
    if(pCommandTypeItem)
        ((const SfxUsrAnyItem*)pCommandTypeItem)->GetValue() >>= nCommandTypeArg;

    switch (nSlot)
    {
        case FN_QRY_INSERT:
            {
                if(pSourceItem && pCommandItem && pCommandTypeItem)
                {
                    DBTextStruct_Impl* pNew = new DBTextStruct_Impl;
                    pNew->aDBData.sDataSource = sSourceArg;
                    pNew->aDBData.sCommand = sCommandArg;
                    pNew->aDBData.nCommandType = nCommandTypeArg;
                    pNew->aSelection = aSelection;
                    Application::PostUserEvent( STATIC_LINK( this, SwBaseShell,
                                            InsertDBTextHdl ), pNew );
                    // the pNew will be removed in InsertDBTextHdl !!
                }
            }
            break;

        case FN_QRY_MERGE_FIELD:
            {
                Sequence<sal_Int32> aIntSelection(aSelection.getLength());
                sal_Int32 * pIntSelection = aIntSelection.getArray();
                const Any* pSelection = aSelection.getConstArray();
                sal_Int32 nIdx = 0;
                for( sal_Int32 i = 0; i < aSelection.getLength(); i++ , nIdx++)
                    pSelection[nIdx] >>= pIntSelection[nIdx];

                Reference<XResultSet>  xResultSet;
                Reference<XDataSource> xSource;
                SwDBData aData;
                aData.sDataSource = sSourceArg;
                aData.sCommand = sCommandArg;
                aData.nCommandType = nCommandTypeArg;
                Reference< XConnection> xConnection = pNewDBMgr->GetConnection(aData.sDataSource, xSource);
                if(!xConnection.is())
                    return ;
                String sStatement;
                if(nCommandTypeArg == CommandType::COMMAND)
                    sStatement = sCommandArg;
                BOOL bDisposeResultSet = FALSE;
                if(!sStatement.Len())
                {
                    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
                    if( xMgr.is() )
                    {
                        Reference<XInterface> xInstance = xMgr->createInstance(
                            C2U( "com.sun.star.sdb.RowSet" ));
                        Reference <XPropertySet> xRowSetPropSet(xInstance, UNO_QUERY);
                        if(xRowSetPropSet.is())
                        {
                            Any aConnection;
                            aConnection <<= xConnection;
                            xRowSetPropSet->setPropertyValue(C2U("ActiveConnection"), aConnection);
                            Any aString;
                            aString <<= aData.sDataSource;
                            xRowSetPropSet->setPropertyValue(C2U("DataSourceName"), aString);
                            aString <<= aData.sCommand;
                            xRowSetPropSet->setPropertyValue(C2U("Command"), aString);
                            Any aInt;
                            aInt <<= aData.nCommandType;
                            xRowSetPropSet->setPropertyValue(C2U("CommandType"), aInt);
                            Reference< XRowSet > xRowSet(xInstance, UNO_QUERY);
                            xRowSet->execute();
                            xResultSet = Reference<XResultSet>(xRowSet, UNO_QUERY);
                            bDisposeResultSet = TRUE;
                        }
                    }
                }
                else
                {
                    try
                    {
                        Reference<XStatement> xStatement = xConnection->createStatement();
                        xResultSet = xStatement->executeQuery( sStatement );
                    }
                    catch(Exception& rExcept)
                    {
                    }
                }
                Sequence<PropertyValue> aProperties(5);
                PropertyValue* pProperties = aProperties.getArray();
                pProperties[0].Name = C2U("DataSourceName");
                pProperties[0].Value <<= (OUString)aData.sDataSource;
                pProperties[1].Name = C2U("Command");
                pProperties[1].Value <<= (OUString)aData.sCommand;
                pProperties[2].Name = C2U("Cursor");
                pProperties[2].Value <<= xResultSet;
                pProperties[3].Name = C2U("Selection");
                pProperties[3].Value <<= aIntSelection;
                pProperties[4].Name = C2U("CommandType");
                pProperties[4].Value <<= aData.nCommandType;

                pNewDBMgr->MergeNew(DBMGR_MERGE, *GetShellPtr(), aProperties);
                if(bDisposeResultSet)
                {
                    Reference<XComponent> xComp(xResultSet, UNO_QUERY);
                    if(xComp.is())
                        xComp->dispose();
                }
            }
            break;

        case FN_QRY_INSERT_FIELD:
            {
                const SfxPoolItem* pConnectionItem = 0;
                const SfxPoolItem* pColumnItem = 0;
                const SfxPoolItem* pColumnNameItem = 0;

                pArgs->GetItemState(FN_DB_CONNECTION_ANY, FALSE, &pConnectionItem);
                pArgs->GetItemState(FN_DB_COLUMN_ANY, FALSE, &pColumnItem);
                pArgs->GetItemState(FN_DB_DATA_COLUMN_NAME_ANY, FALSE, &pColumnNameItem);

                OUString sSource, sCommand;
                sal_Int32 nCommandType = 0;
                if(pSourceItem)
                    ((SfxUsrAnyItem*)pSourceItem)->GetValue() >>= sSource;
                if(pCommandItem)
                    ((SfxUsrAnyItem*)pCommandItem)->GetValue() >>= sCommand;
                if(pCommandTypeItem)
                    ((SfxUsrAnyItem*)pCommandTypeItem)->GetValue() >>= nCommandType;
                OUString sColumnName;
                if(pColumnNameItem)
                    ((SfxUsrAnyItem*)pColumnNameItem)->GetValue() >>= sColumnName;
                String sDBName = sSource;
                sDBName += DB_DELIM;
                sDBName += (String)sCommand;
                sDBName += DB_DELIM;
                sDBName += String::CreateFromInt32(nCommandType);
                sDBName += DB_DELIM;
                sDBName += (String)sColumnName;

                SwFldMgr aFldMgr(GetShellPtr());
                SwInsertFld_Data aData(TYP_DBFLD, 0, sDBName, aEmptyStr, 0, FALSE, TRUE);
                if(pConnectionItem)
                    aData.aDBConnection = ((SfxUsrAnyItem*)pConnectionItem)->GetValue();
                if(pColumnItem)
                    aData.aDBColumn = ((SfxUsrAnyItem*)pColumnItem)->GetValue();
                aFldMgr.InsertFld(aData);
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
        Reference<XDataSource> xSource;
        Reference< sdbc::XConnection> xConnection =
            SwNewDBMgr::GetConnection(pDBStruct->aDBData.sDataSource, xSource);
        Reference< XColumnsSupplier> xColSupp;
        if(xConnection.is())
            xColSupp = SwNewDBMgr::GetColumnSupplier(xConnection,
                                    pDBStruct->aDBData.sCommand,
                                    pDBStruct->aDBData.nCommandType == CommandType::QUERY ?
                                        SW_DB_SELECT_QUERY : SW_DB_SELECT_TABLE);

        if( xColSupp.is() )
        {
            SwDBData aDBData = pDBStruct->aDBData;
            SwInsertDBColAutoPilot *pDlg = new SwInsertDBColAutoPilot(
                    pThis->GetView(),
                    xSource,
                    xColSupp,
                    aDBData );
            if( RET_OK == pDlg->Execute() )
            {
                Reference <XResultSet> xResSet;
                pDlg->DataToDoc( pDBStruct->aSelection, xSource, xConnection, xResSet);
            }
            delete pDlg;
        }
        Reference <XComponent> xComp(xConnection, UNO_QUERY);
        if(xComp.is())
            xComp->dispose();
    }

    delete pDBStruct;
    return 0;
}




