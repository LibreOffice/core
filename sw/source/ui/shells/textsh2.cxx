/*************************************************************************
 *
 *  $RCSfile: textsh2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:47 $
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
#ifdef REPLACE_OFADBMGR
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
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#else

#endif  //REPLACE_OFADBMGR

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

#ifdef REPLACE_OFADBMGR
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
#endif
#define C2S(cChar) UniString::CreateFromAscii(cChar)
#define DB_DD_DELIM 0x0b

#ifdef DEBUG
// the addressbook doesn't support the new api yet (593)
#ifdef REPLACE_OFADBMGR
void lcl_ReplaceDataSource(String& sDBName, String& sTblName, String& sStatmnt)
{
    Reference<XNameAccess> xDBContext;
    Reference< XMultiServiceFactory > xMgr( ::utl::getProcessServiceFactory() );
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
#endif // REPLACE_OFADBMGR
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
#ifdef REPLACE_OFADBMGR
        lcl_ReplaceDataSource(sDBName, sTableName, sStatement);
#endif
#endif // DEBUG

#ifdef REPLACE_OFADBMGR
#else
        if(sTableName.Len())
        {
            sDBName += DB_DELIM;
            sDBName += rTableNameItem.GetValue();
        }
#endif
        pNewDBMgr->SetMergeType( DBMGR_MERGE );
#ifdef REPLACE_OFADBMGR
        pNewDBMgr->Merge(DBMGR_MERGE, &rSh,
                        sStatement, pSelectionList, sDBName, sTableName);
#else
        pNewDBMgr->Merge(DBMGR_MERGE, &rSh,
                        sStatement, pSelectionList, sDBName);
#endif
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
#ifdef REPLACE_OFADBMGR
#ifdef DEBUG
            {
                sDBName = C2S("Nordwind");
                sTableName = C2S("Artikel");
                sStatement = C2S("select * from Artikel");
            }
#endif //DEBUG
#endif //REPLACE_OFADBMGR
#ifdef REPLACE_OFADBMGR
#else
            else
                break;
#endif
            if ( !xSelectionList.Is() )
                xSelectionList = new SbaSelectionList;

#ifdef REPLACE_OFADBMGR
#else
            if (sTableName.Len())
            {
                sDBName += DB_DELIM;
                sDBName += sTableName;
            }
#endif

            SwMailMergeDlg* pDlg = new SwMailMergeDlg(
                    NULL, GetShellPtr(),
                    sDBName,
#ifdef REPLACE_OFADBMGR
                    sTableName,
#endif
                    sStatement, xSelectionList );

            if (pDlg->Execute() == RET_OK)
            {
                pNewDBMgr->SetMergeType(  pDlg->GetMergeType() );

                OFF_APP()->NotifyEvent(SfxEventHint(SW_EVENT_MAIL_MERGE, GetView().GetViewFrame()->GetObjectShell()));

#ifdef REPLACE_OFADBMGR
                pNewDBMgr->Merge(pNewDBMgr->GetMergeType(),
                                    GetShellPtr(), sStatement,
                                    xSelectionList,
                                    sDBName,
                                    sTableName);
#else
                pNewDBMgr->Merge(pNewDBMgr->GetMergeType(),
                                    GetShellPtr(), sStatement,
                                    xSelectionList, sDBName);
#endif
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
#ifdef REPLACE_OFADBMGR
        lcl_ReplaceDataSource(sDBName, sTblName, sStatmnt);
#endif
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
#ifdef REPLACE_OFADBMGR
                String sTableName(sSbaData.GetToken(1, DB_DD_DELIM));
#else
                sDBName += DB_DELIM;
                sDBName += sSbaData.GetToken(1, DB_DD_DELIM);
#endif
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
#ifdef REPLACE_OFADBMGR
                pNewDBMgr->Merge(DBMGR_MERGE,
                                    GetShellPtr(), sStatement,
                                    pSelectionList, sDBName, sTableName);
#else
                pNewDBMgr->Merge(DBMGR_MERGE,
                                    GetShellPtr(), sStatement,
                                    pSelectionList, sDBName);
#endif
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

#ifdef REPLACE_OFADBMGR
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
//          else
//              rSh.ChgDBName( sOldDBName );
            delete pDlg;
        }
//      else
//          rSh.ChgDBName( sOldDBName );
#else
        USHORT nTokenPos = 0;
        String sDBName( pString->GetToken( 0, DB_DD_DELIM, nTokenPos ));
        ( sDBName += DB_DELIM ) +=
                pString->GetToken( 0, DB_DD_DELIM, nTokenPos );
        String sStatmnt( pString->GetToken( 1, DB_DD_DELIM, nTokenPos ));

        SbaSelectionList aSelectionList;
        while( nTokenPos < pString->Len() )
            AddSelList( aSelectionList,
                        pString->GetToken( 0, DB_DD_DELIM, nTokenPos ).ToInt32() );

        SwWrtShell& rSh = pThis->GetShell();
        SwNewDBMgr* pNewDBMgr = rSh.GetNewDBMgr();
        String sOldDBName( rSh.GetDBName() );
        ( sDBName += ';' ) += sStatmnt;
        rSh.ChgDBName( sDBName );

        if( pNewDBMgr->OpenDB( DBMGR_STD, rSh.GetDBDesc() ) )
        {
            pNewDBMgr->ChangeStatement( DBMGR_STD, sStatmnt );

            SwInsertDBColAutoPilot *pDlg = new SwInsertDBColAutoPilot(
                                                        pThis->GetView() );
            if( pDlg->HasValidDB() && RET_OK == pDlg->Execute() )
            {
                // dann jetzt ueber den DBManager die Selektierten
                // Daten ins Dokument einfuegen.
                pDlg->DataToDoc( &aSelectionList );
            }
            else
                rSh.ChgDBName( sOldDBName );
            delete pDlg;
            pNewDBMgr->CloseAll();
        }
        else
            rSh.ChgDBName( sOldDBName );
#endif
    }

    delete pString;
    return 0;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.115  2000/09/18 16:06:06  willem.vandorp
    OpenOffice header added.

    Revision 1.114  2000/07/18 12:50:09  os
    replace ofadbmgr

    Revision 1.113  2000/07/07 15:25:43  os
    replace ofadbmgr

    Revision 1.112  2000/07/06 07:59:11  os
    replace ofadbmgr

    Revision 1.111  2000/07/05 08:22:09  os
    Replace ofadbmgr

    Revision 1.110  2000/06/07 13:18:20  os
    using UCB

    Revision 1.109  2000/05/26 07:21:33  os
    old SW Basic API Slots removed

    Revision 1.108  2000/05/23 19:29:09  jp
    Bugfixes for Unicode

    Revision 1.107  2000/05/10 11:53:02  os
    Basic API removed

    Revision 1.106  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.105  2000/02/11 14:58:04  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.104  2000/01/06 07:32:58  os
    #71436# mail merge dialog: execute via status method disposed

    Revision 1.103  1999/11/11 15:00:55  hr
    #65293#: STLPORT 3.2.1

    Revision 1.102  1999/09/29 07:00:54  mh
    chg: header

    Revision 1.101  1999/03/01 15:22:38  MA
    #62490# Altlast entfernt (Drucken und Briefumschlaege/Etiketten und Datenbank)


      Rev 1.100   01 Mar 1999 16:22:38   MA
   #62490# Altlast entfernt (Drucken und Briefumschlaege/Etiketten und Datenbank)

      Rev 1.99   26 Feb 1999 16:08:42   MA
   #62490# Rest vom Drucken Etiketten entfernt

      Rev 1.98   12 Oct 1998 10:16:12   OM
   #57790# Dialog nicht mehr AppModal

      Rev 1.97   28 May 1998 17:09:54   JP
   Auch beim Droppen von Datensaetzen als Text den neuen DBInsertDialog starten

      Rev 1.96   20 May 1998 21:32:32   JP
   InsertDBCols als Text: rufe den neuen Autopiloten

      Rev 1.95   09 Dec 1997 12:31:08   OM
   #45200# Serienbrief: Speichern-Monitor

      Rev 1.94   02 Dec 1997 19:38:24   MA
   #45900#, SelectionList muss fuer Dialog existieren

      Rev 1.93   24 Nov 1997 09:46:52   MA
   includes

      Rev 1.92   18 Nov 1997 14:45:22   OM
   Sba-Umstellung 372

      Rev 1.91   03 Nov 1997 13:55:42   MA
   precomp entfernt

      Rev 1.90   08 Sep 1997 11:12:38   OM
   #43152# SingleJob-Parameter fuer DataUpdatePrint

      Rev 1.89   02 Sep 1997 14:58:36   OM
   Neue Parameter fuer DataUpdatePrint

      Rev 1.88   02 Sep 1997 09:58:00   OM
   SDB-Headeranpassung

      Rev 1.87   01 Sep 1997 13:23:56   OS
   DLL-Umstellung

      Rev 1.86   06 Aug 1997 14:37:02   TRI
   VCL: includes

      Rev 1.85   11 Jul 1997 14:46:36   OM
   #41525# Nur durch SQL-Statement eingeschraenkte Datensaetze verwenden

      Rev 1.84   09 Jun 1997 17:23:16   OM
   Serienbriefe als Dateien speichern

      Rev 1.83   29 Apr 1997 16:19:48   OM
   Druckdialog bei Aufruf aus Basic unterdruecken

      Rev 1.82   07 Apr 1997 17:45:56   MH
   chg: header

      Rev 1.81   09 Mar 1997 15:51:36   OM
   Basic abgesichert

      Rev 1.80   24 Feb 1997 16:54:02   OS
   Item mit Tabellenname auswerten

      Rev 1.79   13 Feb 1997 22:59:16   MA
   Datenbankfunktionalitaet wieder hergestellt

      Rev 1.78   12 Feb 1997 16:57:48   OM
   Datenbankslots ueber Baseshell rufen

      Rev 1.77   12 Feb 1997 10:40:46   OM
   Endlosschleife behoben

      Rev 1.76   12 Feb 1997 09:22:44   OM
   DD-Trenner korrigiert

      Rev 1.75   09 Feb 1997 15:09:34   OM
   Anderer Trenner bei DB Drag u. Drop

      Rev 1.74   07 Feb 1997 17:20:36   OM
   Datenbankumstellung Drag u. Drop

      Rev 1.73   16 Dec 1996 10:59:02   OM
   Drucken aus DB-Browser angefangen

      Rev 1.72   11 Dec 1996 10:50:56   MA
   Warnings

      Rev 1.71   01 Dec 1996 19:30:28   sdo
   GCC-Parser

      Rev 1.70   12 Nov 1996 16:50:58   MA
   richtige Versionen gemerged

      Rev 1.68   08 Nov 1996 17:40:14   OM
   DB-Mode fuer Serienbriefe und Etiketten zum Teil wiederbelebt

      Rev 1.67   24 Oct 1996 13:36:40   JP
   String Umstellung: [] -> GetChar()

      Rev 1.66   25 Sep 1996 14:12:10   OM
   Neue Datenbanktrenner

      Rev 1.65   19 Sep 1996 14:32:52   OM
   Datenbank Basicanbindung

      Rev 1.64   18 Sep 1996 10:44:58   OM
   Serienbriefe wieder angebunden

      Rev 1.63   13 Sep 1996 15:47:16   OM
   Serienbrief

      Rev 1.62   06 Sep 1996 12:31:42   OM
   Statusmethode fuer DB-Slot

      Rev 1.61   28 Aug 1996 15:54:58   OS
   includes

      Rev 1.60   23 Aug 1996 15:51:56   OM
   Datenbank: Drag&Drop

      Rev 1.59   23 Aug 1996 14:48:00   OM
   Datenbank: Drag&Drop

      Rev 1.58   21 Aug 1996 14:51:52   OM
   Datenbank Drag&Drop

      Rev 1.57   07 Aug 1996 16:24:24   OM
   Datenbankumstellung

      Rev 1.56   19 Jul 1996 16:10:14   OM
   SBA-Umstellung

      Rev 1.55   17 Jul 1996 13:47:40   OM
   Datenbankumstellung 327

      Rev 1.54   02 Jul 1996 16:36:20   OM
   #28615# Makroausfuehrung auch bei Serienbriefdruck

      Rev 1.53   04 Jun 1996 14:11:22   OM
   Serienbrief Mailing

      Rev 1.52   06 May 1996 17:39:34   OM
   Mehrere Datenbanken pro Dokument

      Rev 1.51   30 Apr 1996 09:24:34   OM
   Mehrere Datenbanken pro Dok

------------------------------------------------------------------------*/




