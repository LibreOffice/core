/*************************************************************************
 *
 *  $RCSfile: textsh2.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 11:24:32 $
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

#include "dbmgr.hxx"


#include "view.hxx"
#include "wrtsh.hxx"
#include "swtypes.hxx"
#include "cmdid.h"
#include "swevent.hxx"
#include "shells.hrc"
#include "mailmrge.hxx"
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

#ifdef DEBUG
// the addressbook doesn't support the new api yet (593)
void lcl_ReplaceDataSource(String& sDBName, String& sTblName, String& sStatmnt)
{
    Reference<XNameAccess> xDBContext;
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")
    if(!xDBContext.is())
        return;
    Sequence<OUString> aDBNames = xDBContext->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    long nCount = aDBNames.getLength();
    DBG_ASSERT(nCount, "no data source available")
    if(!nCount)
        return;
    sDBName = pDBNames[0];

    Any aDBSource = xDBContext->getByName(sDBName);
    Reference<XDataSource>* pxSource = (Reference<XDataSource>*)aDBSource.getValue();

    Reference<sdbc::XConnection> xConnection;
    try
    {
        OUString sDummy;
        xConnection = (*pxSource)->getConnection(sDummy, sDummy);
    }
    catch(...) {}
    DBG_ASSERT(xConnection.is(), "no connection found")
    if (xConnection.is())
    {
        Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
        DBG_ASSERT(xTSupplier.is(), "no tables found")
        if(xTSupplier.is())
        {
            Reference<XNameAccess> xTbls = xTSupplier->getTables();
            Sequence<OUString> aTblNames = xTbls->getElementNames();
            long nCount = aTblNames.getLength();
            DBG_ASSERT(nCount, "no table found")
            const OUString* pTblNames = aTblNames.getConstArray();
            sTblName = pTblNames[2];
            sStatmnt = String::CreateFromAscii("select * from ");
            String sQuote = xConnection->getMetaData()->getIdentifierQuoteString();
            sStatmnt += sQuote;
            sStatmnt += sTblName;
            sStatmnt += sQuote;
        }
    }
}
#endif // DEBUG

inline void AddSelList( List& rLst, long nRow )
{
    rLst.Insert( (void*)nRow , LIST_APPEND );
}

void lcl_QRY_UPDATE( const SfxItemSet *pArgs, SwNewDBMgr *pNewDBMgr,
                     SwWrtShell &rSh, USHORT nSlot )
{
    DBG_ASSERT( pArgs, "arguments expected" );
    if (pArgs)
    {
        SbaSelectionListRef pSelectionList;

        const SfxStringItem &rDBNameItem = (const SfxStringItem&) pArgs->Get(SID_ATTR_SBA_DATABASE);
        const SfxStringItem &rTableNameItem = (const SfxStringItem&) pArgs->Get(SID_ATTR_SBA_DBOBJ_NAME);
        const SfxStringItem &rStatementItem = (const SfxStringItem&) pArgs->Get(SID_ATTR_SBA_STATEMENT);
        DBG_ASSERT( rStatementItem.ISA(SfxStringItem), "invalid argument type" );

        const SbaSelectionItem &rSelectionItem = (const SbaSelectionItem&) pArgs->Get(SID_ATTR_SBA_SELECTION);
        pSelectionList = rSelectionItem.GetSelectionList();


        String sDBName(rDBNameItem.GetValue());
        String sTableName(rTableNameItem.GetValue());
        String sStatement(rStatementItem.GetValue());

#ifdef DEBUG
// the addressbook doesn't support the new api yet (593)
        lcl_ReplaceDataSource(sDBName, sTableName, sStatement);
#endif // DEBUG

        pNewDBMgr->SetMergeType( DBMGR_MERGE );
        pNewDBMgr->Merge(DBMGR_MERGE, &rSh,
                        sStatement, pSelectionList, sDBName, sTableName);
    }
}

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwBaseShell::ExecDB(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwNewDBMgr* pNewDBMgr = GetShell().GetNewDBMgr();
    USHORT nSlot = rReq.GetSlot();

    switch (nSlot)
    {
        case SID_SBA_BRW_MERGE:
        {
            String sDBName;
            String sTableName;
            String sStatement;
            SbaSelectionListRef xSelectionList;
            if (pArgs)
            {
                const SfxStringItem &rDBNameItem = (const SfxStringItem&) pArgs->Get(SID_ATTR_SBA_DATABASE);
                sDBName = rDBNameItem.GetValue();
                const SfxStringItem &rTableNameItem = (const SfxStringItem&) pArgs->Get(SID_ATTR_SBA_DBOBJ_NAME);
                sTableName = rTableNameItem.GetValue();
                sStatement = ((const SfxStringItem&) pArgs->Get(SID_ATTR_SBA_STATEMENT)).GetValue();
                const SbaSelectionItem &rSelectionItem = (const SbaSelectionItem&) pArgs->Get(SID_ATTR_SBA_SELECTION);
                xSelectionList = rSelectionItem.GetSelectionList();
            }
#ifdef DEBUG
            {
                sDBName = C2S("Nordwind");
                sTableName = C2S("Artikel");
                sStatement = C2S("select * from Artikel");
            }
#endif //DEBUG
            if ( !xSelectionList.Is() )
                xSelectionList = new SbaSelectionList;

            SwMailMergeDlg* pDlg = new SwMailMergeDlg(
                    NULL, GetShellPtr(),
                    sDBName,
                    sTableName,
                    sStatement, xSelectionList );

            if (pDlg->Execute() == RET_OK)
            {
                pNewDBMgr->SetMergeType(  pDlg->GetMergeType() );

                OFF_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, GetView().GetViewFrame()->GetObjectShell()));

                pNewDBMgr->Merge(pNewDBMgr->GetMergeType(),
                                    GetShellPtr(), sStatement,
                                    xSelectionList,
                                    sDBName,
                                    sTableName);
                delete(pDlg);
            }
        }
        break;

        case SID_SBA_BRW_UPDATE:
            lcl_QRY_UPDATE( pArgs, pNewDBMgr, GetShell(), nSlot );
            break;

        case SID_SBA_BRW_INSERT:
            DBG_ASSERT( pArgs, "arguments expected" );
            if( pArgs )
            {
                String sDBName = ((SfxStringItem&)pArgs->Get(
                                        SID_ATTR_SBA_DATABASE)).GetValue();
                String sTblName = ((SfxStringItem&)pArgs->Get(
                                        SID_ATTR_SBA_DBOBJ_NAME)).GetValue();
                String sStatmnt = ((SfxStringItem&)pArgs->Get(
                                        SID_ATTR_SBA_STATEMENT)).GetValue();
                SbaSelectionListRef xSelectionList( ((SbaSelectionItem&)
                    pArgs->Get(SID_ATTR_SBA_SELECTION)).GetSelectionList());

#ifdef DEBUG
// the addressbook doesn't support the new api yet (593)
        lcl_ReplaceDataSource(sDBName, sTblName, sStatmnt);
#endif // DEBUG

                String* pDataStr = new String( sDBName );
                ((((((*pDataStr) += char(DB_DD_DELIM) )
                    += sTblName ) += char(DB_DD_DELIM) )
                    += '0' ) += char(DB_DD_DELIM) ) // Flag fuer Tabelle oder Query - unused!
                    += sStatmnt;

                if( xSelectionList.Is() )
                    for( ULONG n = 0, nEnd = xSelectionList->Count();
                                    n < nEnd; ++n )
                        ((*pDataStr) += char(DB_DD_DELIM) )
                            += String::CreateFromInt32(
                                    (long)xSelectionList->GetObject( n ));

                SwBaseShell::InsertDBTextHdl( this, pDataStr );
                // der String wird im InsertDBTextHdl geloescht !!
            }
        break;
        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
}



void SwTextShell::ExecDB(SfxRequest &rReq)
{
    const SfxItemSet *pArgs = rReq.GetArgs();
    SwNewDBMgr* pNewDBMgr = GetShell().GetNewDBMgr();
    USHORT nSlot = rReq.GetSlot();

    switch (nSlot)
    {
        case FN_QRY_INSERT:
            {
                String* pNew = new String( ((const SfxStringItem&)pArgs->
                                            Get(nSlot)).GetValue() );
                Application::PostUserEvent( STATIC_LINK( this, SwBaseShell,
                                            InsertDBTextHdl ), pNew );
                // der String wird im InsertDBTextHdl geloescht !!
            }
            break;

        case FN_QRY_MERGE_FIELD:
            {
                String sSbaData = ((const SfxStringItem&)pArgs->Get(nSlot)).GetValue();
                String sDBName = sSbaData.GetToken(0, DB_DD_DELIM);
                String sTableName(sSbaData.GetToken(1, DB_DD_DELIM));
                BOOL bTable = sSbaData.GetToken(2, DB_DD_DELIM) == C2S("1");
                String sStatement = sSbaData.GetToken(3, DB_DD_DELIM);

                SbaSelectionListRef pSelectionList;
                pSelectionList.Clear();
                pSelectionList = new SbaSelectionList;
                USHORT nCount = sSbaData.GetTokenCount(DB_DD_DELIM);

                for( USHORT i = 4; i < nCount; i++ )
                    AddSelList( *pSelectionList,
                                sSbaData.GetToken( i, DB_DD_DELIM).ToInt32() );

                pNewDBMgr->SetMergeType( DBMGR_MERGE );
                pNewDBMgr->Merge(DBMGR_MERGE,
                                    GetShellPtr(), sStatement,
                                    pSelectionList, sDBName, sTableName);
            }
            break;

        case FN_QRY_INSERT_FIELD:
            {
                String sSbaData = ((const SfxStringItem&)pArgs->Get(FN_QRY_INSERT_FIELD)).GetValue();

                String sDBName = sSbaData.GetToken(0, DB_DD_DELIM);
                sDBName += DB_DELIM;
                sDBName += sSbaData.GetToken(1, DB_DD_DELIM);
                sDBName += DB_DELIM;
                BOOL bTable = sSbaData.GetToken(2, DB_DD_DELIM) == C2S("1");
                sDBName += sSbaData.GetToken(3, DB_DD_DELIM);   // Column name

                InsertDBFld(sDBName);
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



IMPL_STATIC_LINK( SwBaseShell, InsertDBTextHdl, String*, pString )
{
    if( pString )
    {

        USHORT nTokenPos = 0;
        String sSourceName( pString->GetToken( 0, DB_DD_DELIM, nTokenPos ));
        String sTblQryName( pString->GetToken( 0, DB_DD_DELIM, nTokenPos ));
        String sStatmnt( pString->GetToken( 1, DB_DD_DELIM, nTokenPos ));

        Reference<XDataSource> xSource;
        Reference< sdbc::XConnection> xConnection = SwNewDBMgr::GetConnection(sSourceName, xSource);
        Reference< XColumnsSupplier> xColSupp;
        if(xConnection.is())
            xColSupp = SwNewDBMgr::GetColumnSupplier(xConnection,
                                    sTblQryName,
                                    SW_DB_SELECT_UNKNOWN);

        if( xColSupp.is() )
        {
            SwInsDBData aDBData;
            aDBData.sDataBaseName = sSourceName;
            aDBData.sDataTableName = sTblQryName;
            aDBData.sStatement = sStatmnt;
            SwInsertDBColAutoPilot *pDlg = new SwInsertDBColAutoPilot(
                    pThis->GetView(),
                    xSource,
                    xColSupp,
                    aDBData );
            if( RET_OK == pDlg->Execute() )
            {
                SbaSelectionList aSelectionList;
                while( nTokenPos < pString->Len() )
                    AddSelList( aSelectionList,
                                pString->GetToken( 0, DB_DD_DELIM, nTokenPos ).ToInt32() );
                pDlg->DataToDoc( &aSelectionList , xSource, xConnection);
            }
            delete pDlg;
        }
    }

    delete pString;
    return 0;
}




