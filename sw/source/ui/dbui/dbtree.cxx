/*************************************************************************
 *
 *  $RCSfile: dbtree.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-10-27 11:24:06 $
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

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::beans;

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
    Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
    if( xMgr.is() )
    {
        Reference<XInterface> xInstance = xMgr->createInstance( C2U( "com.sun.star.sdb.DatabaseContext" ));
        xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
    }
    DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")

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
    if(!xDBContext.is())
        return;
    SetSelectionMode(SINGLE_SELECTION);
    SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    // Font nicht setzen, damit der Font des Controls uebernommen wird!
    SetSpaceBetweenEntries(0);
    SetNodeBitmaps( aRootClosed, aRootOpened );

    SetDragDropMode(SV_DRAGDROP_APP_COPY);


    GetModel()->SetCompareHdl(LINK(this, SwDBTreeList, DBCompare));

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
        String sTableName, sColumnName;
        String  sDBName(GetDBName(sTableName, sColumnName));

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
            Select(sDBName, sTableName, sColumnName);   // force RequestingChilds
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

String  SwDBTreeList::GetDBName(String& rTableName, String& rColumnName, BOOL* pbIsTable)
{
    String sDBName;
    SvLBoxEntry* pEntry = FirstSelected();

    if (pEntry && GetParent(pEntry))
    {
        if (GetParent(GetParent(pEntry)))
        {
            rColumnName = GetEntryText(pEntry);
            pEntry = GetParent(pEntry); // Spaltenname war selektiert
        }
        sDBName = GetEntryText(GetParent(pEntry));
        if(pbIsTable)
        {
            *pbIsTable = pEntry->GetUserData() != 0;
        }
        rTableName = GetEntryText(pEntry);
    }
    return sDBName;
}

/*------------------------------------------------------------------------
 Beschreibung:  Format: Datenbank.Tabelle
------------------------------------------------------------------------*/


void SwDBTreeList::Select(const String& rDBName, const String& rTableName, const String& rColumnName)
{
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
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

BOOL  SwDBTreeList::QueryDrop( DropEvent& rEvt)
{
    return FALSE;
}

