/*************************************************************************
 *
 *  $RCSfile: dbtree.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-10-20 14:18:01 $
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

// INCLUDE ---------------------------------------------------------------

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

#ifdef REPLACE_OFADBMGR
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

#else

#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SBAOBJ_HXX //autogen
#include <offmgr/sbaobj.hxx>
#endif
#ifndef _SBA_SBADB_HXX //autogen
#include <offmgr/sbadb.hxx>
#endif
#ifndef _SBAITEMS_HRC
#include <offmgr/sbaitems.hrc>
#endif
#ifndef _SBAITEMS_HXX
#include <offmgr/sbaitems.hxx>
#endif

#endif  //REPLACE_OFADBMGR

#ifndef _DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif

#include "helpid.h"
#include "dbmgr.hxx"
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"

#include "dbui.hrc"
#include "dbtree.hxx"

#ifdef REPLACE_OFADBMGR
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;
#endif

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
// STATIC DATA -----------------------------------------------------------


/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


SwDBTreeList::SwDBTreeList(Window *pParent, const ResId& rResId, const String& rDefDBName, const BOOL bShowCol):

    SvTreeListBox   (pParent, rResId),

    aRootClosed     (SW_RES(BMP_ROOT_CLOSED)),
    aRootOpened     (SW_RES(BMP_ROOT_OPENED)),
    aDBBMP          (SW_RES(BMP_DB)),
    aTableBMP       (SW_RES(BMP_TABLE)),
    aQueryBMP       (SW_RES(BMP_QUERY)),

    sDefDBName      (rDefDBName),
    bShowColumns    (bShowCol),
    bInitialized    (FALSE)
{
#ifdef REPLACE_OFADBMGR
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")
#endif

    SetHelpId(HID_DB_SELECTION_TLB);

    if (IsVisible())
        InitTreeList();
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


 SwDBTreeList::~SwDBTreeList()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


void SwDBTreeList::InitTreeList()
{
#ifdef REPLACE_OFADBMGR
    if(!xDBContext.is())
        return;
#endif
    SetSelectionMode(SINGLE_SELECTION);
    SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    // Font nicht setzen, damit der Font des Controls uebernommen wird!
    SetSpaceBetweenEntries(0);
    SetNodeBitmaps( aRootClosed, aRootOpened );

    SetDragDropMode(SV_DRAGDROP_APP_COPY);


    GetModel()->SetCompareHdl(LINK(this, SwDBTreeList, DBCompare));

#ifdef REPLACE_OFADBMGR
    Sequence<OUString> aDBNames = xDBContext->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    long nCount = aDBNames.getLength();

    for(long i = 0; i < nCount; i++)
    {
        String sDBName(pDBNames[i]);
        InsertEntry(sDBName, aDBBMP, aDBBMP, NULL, TRUE);
    }
    String sDBName(sDefDBName.GetToken(0, DB_DELIM));
    String sTableName(sDefDBName.GetToken(1, DB_DELIM));
    String sColumnName(sDefDBName.GetToken(2, DB_DELIM));
    Select(sDBName, sTableName, sColumnName);

#else
    OfficeApplication* pOffApp = OFF_APP();
    SbaObject* pSbaObject = pOffApp->GetSbaObject();
    String sDBNames = pSbaObject->GetDatabaseNames();
    USHORT nCount = sDBNames.GetTokenCount();

    for (USHORT i = 0; i < nCount; i++)
    {
        String sDBName(sDBNames.GetToken(i));
        InsertEntry(sDBName, aDBBMP, aDBBMP, NULL, TRUE);
    }
    Select(sDefDBName);
#endif

    bInitialized = TRUE;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwDBTreeList::ShowColumns(BOOL bShowCol)
{
    if (bShowCol != bShowColumns)
    {
        bShowColumns = bShowCol;
#ifdef REPLACE_OFADBMGR
        String sTableName, sColumnName;
        String  sDBName(GetDBName(sTableName, sColumnName));
#else
        String sDBName(GetDBName());
#endif

        SetUpdateMode(FALSE);

        SvLBoxEntry* pEntry = First();

        while (pEntry)
        {
            pEntry = (SvLBoxEntry*)GetRootLevelParent( pEntry );
            Collapse(pEntry);       // zuklappen

            SvLBoxEntry* pChild;
            while ((pChild = FirstChild(pEntry)) != 0L)
                GetModel()->Remove(pChild);

            pEntry = Next(pEntry);
        }

        if (sDBName.Len())
        {
#ifdef REPLACE_OFADBMGR
            Select(sDBName, sTableName, sColumnName);   // force RequestingChilds
#else
            Select(sDBName);    // RequestingChilds erzwingen
#endif
        }
        SetUpdateMode(TRUE);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void  SwDBTreeList::RequestingChilds(SvLBoxEntry* pParent)
{
    if (!pParent->HasChilds())
    {
#ifdef REPLACE_OFADBMGR

        if (GetParent(pParent)) // column names
        {

            String sSourceName = GetEntryText(GetParent(pParent));
            String sTableName = GetEntryText(pParent);

            if(!xDBContext->hasByName(sSourceName))
                return;
            Any aDBSource = xDBContext->getByName(sSourceName);
            Reference<XDataSource>* pxSource = (Reference<XDataSource>*)aDBSource.getValue();
            BOOL bTable = pParent->GetUserData != 0;
            Reference<XConnection> xConnection;
            try
            {
                OUString sDummy;
                xConnection = (*pxSource)->getConnection(sDummy, sDummy);
            }
            catch(...) {}
            Reference<XColumnsSupplier> xColsSupplier;
            if(bTable)
            {
                Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
                if(xTSupplier.is())
                {
                    Reference<XNameAccess> xTbls = xTSupplier->getTables();
                    DBG_ASSERT(xTbls->hasByName(sTableName), "table not available anymore?")
                    try
                    {
                        Any aTable = xTbls->getByName(sTableName);
                        Reference<XPropertySet> xPropSet = *(Reference<XPropertySet>*)aTable.getValue();
                        xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                    }
                    catch(...){}
                }
            }
            else
            {
                Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
                if(xQSupplier.is())
                {
                    Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                    DBG_ASSERT(xQueries->hasByName(sTableName), "table not available anymore?")
                    try
                    {
                        Any aQuery = xQueries->getByName(sTableName);
                        Reference<XPropertySet> xPropSet = *(Reference<XPropertySet>*)aQuery.getValue();
                        xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                    }
                    catch(...){}
                }
            }

            if(xColsSupplier.is())
            {
                Reference <XNameAccess> xCols = xColsSupplier->getColumns();
                Sequence<OUString> aColNames = xCols->getElementNames();
                const OUString* pColNames = aColNames.getConstArray();
                long nCount = aColNames.getLength();
                for (long i = 0; i < nCount; i++)
                {
                    String sName = pColNames[i];
                    if(bTable)
                        InsertEntry(sName, pParent);
                    else
                        InsertEntry(sName, pParent);
                }
            }
        }
        else    // Tabellennamen
        {
            String sSourceName = GetEntryText(pParent);
            if(!xDBContext->hasByName(sSourceName))
                return;
            Any aDBSource = xDBContext->getByName(sSourceName);
            Reference<XDataSource>* pxSource = (Reference<XDataSource>*)aDBSource.getValue();

            Reference<XConnection> xConnection;
            try
            {
                OUString sDummy;
                xConnection = (*pxSource)->getConnection(sDummy, sDummy);
            }
            catch(...) {}
//          SbaDatabaseRef pConnection = pSbaObject->GetDatabase(sDBName, TRUE);
            if (xConnection.is())
            {
                Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
                if(xTSupplier.is())
                {
                    Reference<XNameAccess> xTbls = xTSupplier->getTables();
                    Sequence<OUString> aTblNames = xTbls->getElementNames();
                    String sTableName;
                    long nCount = aTblNames.getLength();
                    const OUString* pTblNames = aTblNames.getConstArray();
                    for (long i = 0; i < nCount; i++)
                    {
                        sTableName = pTblNames[i];
                        SvLBoxEntry* pTableEntry = InsertEntry(sTableName, aTableBMP, aTableBMP, pParent, bShowColumns);
                        //to discriminate between queries and tables the user data of table entries is set
                        pTableEntry->SetUserData((void*)1);
                    }
                }

                Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
                if(xQSupplier.is())
                {
                    Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                    Sequence<OUString> aQueryNames = xQueries->getElementNames();
                    String sQueryName;
                    long nCount = aQueryNames.getLength();
                    const OUString* pQueryNames = aQueryNames.getConstArray();
                    for (long i = 0; i < nCount; i++)
                    {
                        sQueryName = pQueryNames[i];
                        InsertEntry(sQueryName, aQueryBMP, aQueryBMP, pParent, bShowColumns);
                    }
                }
            }
        }
#else
        SbaObject* pSbaObject = OFF_APP()->GetSbaObject();
        if (GetParent(pParent)) // Spaltennamen
        {
            SwWrtShell *pSh = ::GetActiveView()->GetWrtShellPtr();
            SwNewDBMgr *pMgr = pSh->GetNewDBMgr();

            String sDBName = GetEntryText(GetParent(pParent));
            sDBName += DB_DELIM;
            sDBName += GetEntryText(pParent);

            if (!sDBName.Len() || (!pMgr->IsDBOpen(DBMGR_STD, sDBName) && !pMgr->OpenDB(DBMGR_STD, sDBName, TRUE)))
                return;

            SbaDBDataDefRef aDBDef = pMgr->OpenColumnNames(DBMGR_STD);

            if (aDBDef.Is())
            {
                const SbaColumnList& rCols = aDBDef->GetOriginalColumns();

                for (USHORT i = 1; i <= rCols.Count(); i++)
                {
                    const SbaNameItem* pNameItem = (const SbaNameItem*)&rCols.GetObject(i-1)->Get(SBA_DEF_FLTNAME);
                    InsertEntry(pNameItem->GetValue(), pParent);
                }
            }
            pMgr->CloseAll(DBMGR_STD);
        }
        else    // Tabellennamen
        {
            String sDBName = GetEntryText(pParent);

            SbaDatabaseRef pConnection = pSbaObject->GetDatabase(sDBName, TRUE);
            if (pConnection.Is())
            {
                String sTableName;

                USHORT nCount = pConnection->GetObjectCount(dbTable);

                for (USHORT i = 0; i < nCount; i++)
                {
                    sTableName = pConnection->GetObjectName(dbTable, i);
                    InsertEntry(sTableName, aTableBMP, aTableBMP, pParent, bShowColumns);
                }

                nCount = pConnection->GetObjectCount(dbQuery);

                for (i = 0; i < nCount; i++)
                {
                    sTableName = pConnection->GetObjectName(dbQuery, i);
                    InsertEntry(sTableName, aQueryBMP, aQueryBMP, pParent, bShowColumns);
                }
            }
        }
#endif
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwDBTreeList, DBCompare, SvSortData*, pData )
{
    SvLBoxEntry* pRight = (SvLBoxEntry*)(pData->pRight );

    if (GetParent(pRight) && GetParent(GetParent(pRight)))
        return COMPARE_GREATER; // Spaltennamen nicht sortieren

    return DefaultCompare(pData);   // Sonst Basisklasse rufen
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

#ifdef REPLACE_OFADBMGR
String  SwDBTreeList::GetDBName(String& rTableName, String& rColumnName, BOOL* pbIsTable)
#else
String SwDBTreeList::GetDBName() const
#endif
{
    String sDBName;
#ifdef REPLACE_OFADBMGR
#else
    String sColumnName;
#endif
    SvLBoxEntry* pEntry = FirstSelected();

    if (pEntry && GetParent(pEntry))
    {
        if (GetParent(GetParent(pEntry)))
        {
#ifdef REPLACE_OFADBMGR
            rColumnName = GetEntryText(pEntry);
#else
            sColumnName = GetEntryText(pEntry);
#endif
            pEntry = GetParent(pEntry); // Spaltenname war selektiert
        }
        sDBName = GetEntryText(GetParent(pEntry));
#ifdef REPLACE_OFADBMGR
        if(pbIsTable)
        {
            *pbIsTable = pEntry->GetUserData() != 0;
        }
        rTableName = GetEntryText(pEntry);
#else
        sDBName += DB_DELIM;
        sDBName += GetEntryText(pEntry);

        if (sColumnName.Len())
        {
            sDBName += DB_DELIM;
            sDBName += sColumnName;
        }
#endif
    }
    return sDBName;
}

/*------------------------------------------------------------------------
 Beschreibung:  Format: Datenbank.Tabelle
------------------------------------------------------------------------*/


#ifdef REPLACE_OFADBMGR
void SwDBTreeList::Select(const String& rDBName, const String& rTableName, const String& rColumnName)
#else
void  SwDBTreeList::Select(const String& rDataBaseName)
#endif
{
#ifdef REPLACE_OFADBMGR
#else
    String rDBName(rDataBaseName.GetToken(0, DB_DELIM));
    String rTableName(rDataBaseName.GetToken(1, DB_DELIM));
    String rColumnName(rDataBaseName.GetToken(2, DB_DELIM));
#endif
    SvLBoxEntry* pParent;
    SvLBoxEntry* pChild;
    USHORT nParent = 0;
    USHORT nChild = 0;

    while ((pParent = GetEntry(nParent++)) != NULL)
    {
        if (rDBName == GetEntryText(pParent))
        {
            if (!pParent->HasChilds())
                RequestingChilds(pParent);
            while ((pChild = GetEntry(pParent, nChild++)) != NULL)
            {
                if (rTableName == GetEntryText(pChild))
                {
                    pParent = pChild;

                    if (bShowColumns && rColumnName.Len())
                    {
                        nChild = 0;

                        if (!pParent->HasChilds())
                            RequestingChilds(pParent);

                        while ((pChild = GetEntry(pParent, nChild++)) != NULL)
                            if (rColumnName == GetEntryText(pChild))
                                break;
                    }
                    if (!pChild)
                        pChild = pParent;

                    MakeVisible(pChild);
                    SvTreeListBox::Select(pChild);
                    return;
                }
            }
        }
    }
}

/*------------------------------------------------------------------------
 Beschreibung: Initialisierung verz”gern
------------------------------------------------------------------------*/

void  SwDBTreeList::Show()
{
    if (!bInitialized)
        InitTreeList();

    SvTreeListBox::Show();
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void  SwDBTreeList::Command( const CommandEvent& rCEvt )
{
    switch( rCEvt.GetCommand() )
    {
        case COMMAND_STARTDRAG:
            StartExecuteDrag();
            ExecuteDrag(Pointer(POINTER_MOVEDATA), Pointer(POINTER_COPYDATA),
                        Pointer(POINTER_LINKDATA), DRAG_COPYABLE | DRAG_LINKABLE);
        break;
        default:
            SvTreeListBox::Command(rCEvt);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwDBTreeList::StartExecuteDrag()
{
    DragServer::Clear();
#ifdef REPLACE_OFADBMGR
    String sTableName, sColumnName;
    String  sDBName(GetDBName(sTableName, sColumnName));
    if(sDBName.Len())
    {
        if (sColumnName.Len())
        {
            String aCopyData = sDBName;
            aCopyData   += char(11);
            aCopyData   += sTableName;
            aCopyData   += char(11);
            aCopyData   += String(String::CreateFromAscii("0"));
            aCopyData   += char(11);
            aCopyData   += sColumnName;

            // Datenbankfeld draggen
            DragServer::CopyData(aCopyData.GetBuffer(), aCopyData.Len() + 1,
                                    SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE);
        }

        sDBName += '.';
        sDBName += sTableName;
        if(sColumnName.Len())
        {
            sDBName += '.';
            sDBName += sColumnName;
        }
        DragServer::CopyString(sDBName);
    }
#else
    String sDBName(GetDBName());
    if(sDBName.Len())
    {
        if (sDBName.GetTokenCount(DB_DELIM) > 2)    // Spaltenname ist enthalten
        {
            String aCopyData = sDBName.GetToken(0, DB_DELIM);
            aCopyData   += char(11);
            aCopyData   += sDBName.GetToken(1, DB_DELIM);
            aCopyData   += char(11);
            aCopyData   += String(String::CreateFromAscii("0"));
            aCopyData   += char(11);
            aCopyData   += sDBName.GetToken(2, DB_DELIM);

            // Datenbankfeld draggen
            DragServer::CopyData(aCopyData.GetBuffer(), aCopyData.Len() + 1,
                                    SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE);
        }

        USHORT nPos;
        while ((nPos = sDBName.SearchAndReplace(DB_DELIM, '.')) != STRING_NOTFOUND);
        DragServer::CopyString(sDBName);
    }
#endif
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

BOOL  SwDBTreeList::QueryDrop( DropEvent& rEvt)
{
    return FALSE;
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:34  hr
    initial import

    Revision 1.41  2000/09/18 16:05:19  willem.vandorp
    OpenOffice header added.

    Revision 1.40  2000/06/26 13:31:03  os
    new DataBase API

    Revision 1.39  2000/04/11 08:03:52  os
    UNICODE

    Revision 1.38  2000/02/11 14:44:29  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.37  1999/11/09 16:54:18  os
    #69090# default in ::Command

    Revision 1.36  1999/01/20 16:00:04  AWO
    #59398#FormatIds


      Rev 1.35   20 Jan 1999 17:00:04   AWO
   #59398#FormatIds

      Rev 1.34   21 Aug 1998 12:59:04   OM
   #55373# D&D von DB-Feldern

      Rev 1.33   07 Mar 1998 13:30:14   OM
   Alles bei Column-Umschaltung einklappen

      Rev 1.32   27 Feb 1998 18:26:26   OM
   Aufgeraeumt

      Rev 1.31   03 Feb 1998 12:21:12   OM
   #46621# HelpIDs fuer Feldbefehl-Dlg

      Rev 1.30   06 Jan 1998 18:12:02   OM
   Felbefehl-Dlg

      Rev 1.29   05 Jan 1998 17:44:54   OM
   DB-Feldbefehl bearbeiten

      Rev 1.28   15 Dec 1997 11:32:08   OM
   GetObjectName gefixt

      Rev 1.27   12 Dec 1997 10:37:44   OM
   GetObjectName angepasst

      Rev 1.26   12 Dec 1997 10:11:02   OM
   Spaltennamen nicht sortieren

      Rev 1.25   11 Dec 1997 17:01:10   OM
   Feldumstellung

      Rev 1.24   19 Nov 1997 16:33:06   OM
   Datenbank-TP Drag&Drop

      Rev 1.23   18 Nov 1997 14:34:02   OM
   Sba-Umstellung 372

      Rev 1.22   18 Nov 1997 10:35:42   OM
   Neuer Feldbefehldialog

      Rev 1.21   17 Nov 1997 14:36:32   OM
   Column-Darstellung an/abschaltbar

      Rev 1.20   10 Nov 1997 10:48:56   TJ
   GetTableName und GetQueryName durch GetObjectName ersetzt

      Rev 1.19   06 Nov 1997 17:01:50   OM
   #45188# Schriftgroesse in TreeListBox anders initialisieren

      Rev 1.18   06 Nov 1997 13:01:26   OM
   Geaenderte Datenbankauswahl

      Rev 1.17   05 Nov 1997 17:02:12   OM
   Spaltennamen anzeigen

      Rev 1.16   14 Oct 1997 14:10:34   OM
   Feldumstellung

      Rev 1.15   02 Sep 1997 09:57:10   OM
   SDB-Headeranpassung

      Rev 1.14   01 Sep 1997 13:16:04   OS
   DLL-Umstellung

      Rev 1.13   25 Aug 1997 11:53:54   OS
   368-Changes SBA

      Rev 1.12   06 Aug 1997 11:36:00   TRI
   GetpApp statt pApp

      Rev 1.11   14 Mar 1997 14:09:26   OM
   Sortierte DB-Listboxen

      Rev 1.10   11 Nov 1996 09:25:38   MA
   ResMgr

      Rev 1.9   14 Oct 1996 16:06:44   OM
   Datenbankumstellung 341c

      Rev 1.8   25 Sep 1996 14:10:44   OM
   Neue Datenbanktrenner

      Rev 1.7   03 Sep 1996 12:01:58   OM
   #30967# Fehlendes SEXPORT

      Rev 1.6   28 Aug 1996 09:18:04   OS
   includes

      Rev 1.5   14 Aug 1996 11:39:36   OM
   RequestingChilds nur bei leerem Parent

      Rev 1.4   08 Aug 1996 16:15:54   OM
   Neue Segs

      Rev 1.3   08 Aug 1996 16:15:14   OM
   Tabellen nur on demand anfordern

      Rev 1.2   05 Aug 1996 15:48:38   OM
   Neue Segs

      Rev 1.1   05 Aug 1996 15:47:52   OM
   Datenbankumstellung

      Rev 1.0   25 Jul 1996 16:35:50   OM
   Initial revision.

------------------------------------------------------------------------*/


