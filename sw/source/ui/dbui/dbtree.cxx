/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// INCLUDE ---------------------------------------------------------------


#include <sot/formats.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svx/dbaexchange.hxx>

#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#include <swmodule.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <wrtsh.hxx>
#include <dbtree.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif

#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;

/* -----------------------------17.07.01 13:10--------------------------------

 ---------------------------------------------------------------------------*/
struct SwConnectionData
{
    ::rtl::OUString                sSourceName;
    Reference<XConnection>  xConnection;
};

typedef SwConnectionData* SwConnectionDataPtr;
SV_DECL_PTRARR_DEL( SwConnectionArr, SwConnectionDataPtr, 32, 32 )
SV_IMPL_PTRARR( SwConnectionArr, SwConnectionDataPtr )
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
class SwDBTreeList_Impl : public cppu::WeakImplHelper1 < XContainerListener >
{
    Reference< XNameAccess > xDBContext;
    SwConnectionArr aConnections;
    SwWrtShell* pWrtSh;

    public:
        SwDBTreeList_Impl(SwWrtShell* pShell) :
            pWrtSh(pShell) {}
        ~SwDBTreeList_Impl();

    virtual void SAL_CALL elementInserted( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL elementRemoved( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL elementReplaced( const ContainerEvent& Event ) throw (RuntimeException);
    virtual void SAL_CALL disposing( const EventObject& Source ) throw (RuntimeException);

    sal_Bool                        HasContext();
    SwWrtShell*                 GetWrtShell() { return pWrtSh;}
    void                        SetWrtShell(SwWrtShell& rSh) { pWrtSh = &rSh;}
    Reference< XNameAccess >    GetContext() {return xDBContext;}
    Reference<XConnection>      GetConnection(const rtl::OUString& rSourceName);
};
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
SwDBTreeList_Impl::~SwDBTreeList_Impl()
{
    Reference<XContainer> xContainer(xDBContext, UNO_QUERY);
    if(xContainer.is())
    {
        m_refCount++;
        //block necessary due to solaris' compiler behaviour to
        //remove temporaries at the block's end
        {
            xContainer->removeContainerListener( this );
        }
        m_refCount--;
    }
}
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
void SwDBTreeList_Impl::elementInserted( const ContainerEvent&  ) throw (RuntimeException)
{
    // information not needed
}
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
void SwDBTreeList_Impl::elementRemoved( const ContainerEvent& rEvent ) throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    ::rtl::OUString sSource;
    rEvent.Accessor >>= sSource;
    for(sal_uInt16 i = 0; i < aConnections.Count(); i++)
    {
        SwConnectionDataPtr pPtr = aConnections[i];
        if(pPtr->sSourceName == sSource)
        {
//            SwConnectionDataPtr pPtr = aConnections[i];
//            Reference<XComponent> xComp(pPtr->xConnection, UNO_QUERY);
//            if(xComp.is())
//                xComp->dispose();
            aConnections.DeleteAndDestroy(i);
            break;
        }
    }
}
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
void SwDBTreeList_Impl::disposing( const EventObject&  ) throw (RuntimeException)
{
    xDBContext = 0;
}
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
void SwDBTreeList_Impl::elementReplaced( const ContainerEvent& rEvent ) throw (RuntimeException)
{
    elementRemoved(rEvent);
}
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwDBTreeList_Impl::HasContext()
{
    if(!xDBContext.is())
    {
        Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if( xMgr.is() )
        {
            Reference<XInterface> xInstance = xMgr->createInstance(
                        C2U( "com.sun.star.sdb.DatabaseContext" ));
            xDBContext = Reference<XNameAccess>(xInstance, UNO_QUERY) ;
            Reference<XContainer> xContainer(xDBContext, UNO_QUERY);
            if(xContainer.is())
                xContainer->addContainerListener( this );
        }
        DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available");
    }
    return xDBContext.is();
}
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XConnection>  SwDBTreeList_Impl::GetConnection(const rtl::OUString& rSourceName)
{
    Reference<XConnection>  xRet;
    for(sal_uInt16 i = 0; i < aConnections.Count(); i++)
    {
        SwConnectionDataPtr pPtr = aConnections[i];
        if(pPtr->sSourceName == rSourceName)
        {
            xRet = pPtr->xConnection;
            break;
        }
    }
    if(!xRet.is() && xDBContext.is() && pWrtSh)
    {
        SwConnectionDataPtr pPtr = new SwConnectionData();
        pPtr->sSourceName = rSourceName;
        xRet = pWrtSh->GetNewDBMgr()->RegisterConnection(pPtr->sSourceName);
        aConnections.Insert(pPtr, aConnections.Count());
    }
    return xRet;
}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
SwDBTreeList::SwDBTreeList(Window *pParent, const ResId& rResId,
                        SwWrtShell* pSh,
                        const String& rDefDBName, const sal_Bool bShowCol):

    SvTreeListBox   (pParent, rResId),
    aImageList      (SW_RES(ILIST_DB_DLG    )),
    aImageListHC    (SW_RES(ILIST_DB_DLG_HC )),
    sDefDBName      (rDefDBName),
    bInitialized    (sal_False),
    bShowColumns    (bShowCol),
    pImpl(new SwDBTreeList_Impl(pSh))
{
    SetHelpId(HID_DB_SELECTION_TLB);

    if (IsVisible())
        InitTreeList();
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
SwDBTreeList::~SwDBTreeList()
{
    delete pImpl;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/


void SwDBTreeList::InitTreeList()
{
    if(!pImpl->HasContext() && pImpl->GetWrtShell())
        return;
    SetSelectionMode(SINGLE_SELECTION);
    SetStyle(GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    // Font nicht setzen, damit der Font des Controls uebernommen wird!
    SetSpaceBetweenEntries(0);
    SetNodeBitmaps( aImageList.GetImage(IMG_COLLAPSE),
                    aImageList.GetImage(IMG_EXPAND  ), BMP_COLOR_NORMAL );
    SetNodeBitmaps( aImageListHC.GetImage(IMG_COLLAPSE),
                    aImageListHC.GetImage(IMG_EXPAND  ), BMP_COLOR_HIGHCONTRAST );

    SetDragDropMode(SV_DRAGDROP_APP_COPY);

    GetModel()->SetCompareHdl(LINK(this, SwDBTreeList, DBCompare));

    Sequence< ::rtl::OUString > aDBNames = pImpl->GetContext()->getElementNames();
    const ::rtl::OUString* pDBNames = aDBNames.getConstArray();
    long nCount = aDBNames.getLength();

    Image aImg = aImageList.GetImage(IMG_DB);
    Image aHCImg = aImageListHC.GetImage(IMG_DB);
    for(long i = 0; i < nCount; i++)
    {
        String sDBName(pDBNames[i]);
        SvLBoxEntry* pEntry = InsertEntry(sDBName, aImg, aImg, NULL, sal_True);
        SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
        SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
    }
    String sDBName(sDefDBName.GetToken(0, DB_DELIM));
    String sTableName(sDefDBName.GetToken(1, DB_DELIM));
    String sColumnName(sDefDBName.GetToken(2, DB_DELIM));
    Select(sDBName, sTableName, sColumnName);


    bInitialized = sal_True;
}
/*-- 27.05.2004 09:19:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwDBTreeList::AddDataSource(const String& rSource)
{
    Image aImg = aImageList.GetImage(IMG_DB);
    Image aHCImg = aImageListHC.GetImage(IMG_DB);
    SvLBoxEntry* pEntry = InsertEntry(rSource, aImg, aImg, NULL, sal_True);
    SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
    SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
    SvTreeListBox::Select(pEntry);
}
/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

void SwDBTreeList::ShowColumns(sal_Bool bShowCol)
{
    if (bShowCol != bShowColumns)
    {
        bShowColumns = bShowCol;
        String sTableName, sColumnName;
        String  sDBName(GetDBName(sTableName, sColumnName));

        SetUpdateMode(sal_False);

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
        SetUpdateMode(sal_True);
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
            try
            {

                String sSourceName = GetEntryText(GetParent(pParent));
                String sTableName = GetEntryText(pParent);

                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                sal_Bool bTable = pParent->GetUserData() == 0;
                Reference<XColumnsSupplier> xColsSupplier;
                if(bTable)
                {
                    Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTbls = xTSupplier->getTables();
                        DBG_ASSERT(xTbls->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aTable = xTbls->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aTable >>= xPropSet;
                            xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                        }
                        catch(Exception&)
                        {}
                    }
                }
                else
                {
                    Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        DBG_ASSERT(xQueries->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aQuery = xQueries->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aQuery >>= xPropSet;
                            xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
                        }
                        catch(Exception&)
                        {}
                    }
                }

                if(xColsSupplier.is())
                {
                    Reference <XNameAccess> xCols = xColsSupplier->getColumns();
                    Sequence< ::rtl::OUString> aColNames = xCols->getElementNames();
                    const ::rtl::OUString* pColNames = aColNames.getConstArray();
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
            catch(const Exception&)
            {
            }
        }
        else    // Tabellennamen
        {
            try
            {
                String sSourceName = GetEntryText(pParent);
                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                if (xConnection.is())
                {
                    Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTbls = xTSupplier->getTables();
                        Sequence< ::rtl::OUString> aTblNames = xTbls->getElementNames();
                        String sTableName;
                        long nCount = aTblNames.getLength();
                        const ::rtl::OUString* pTblNames = aTblNames.getConstArray();
                        Image aImg = aImageList.GetImage(IMG_DBTABLE);
                        Image aHCImg = aImageListHC.GetImage(IMG_DBTABLE);
                        for (long i = 0; i < nCount; i++)
                        {
                            sTableName = pTblNames[i];
                            SvLBoxEntry* pTableEntry = InsertEntry(sTableName, aImg, aImg, pParent, bShowColumns);
                            //to discriminate between queries and tables the user data of table entries is set
                            pTableEntry->SetUserData((void*)0);
                            SetExpandedEntryBmp(pTableEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                            SetCollapsedEntryBmp(pTableEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                        }
                    }

                    Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        Sequence< ::rtl::OUString> aQueryNames = xQueries->getElementNames();
                        String sQueryName;
                        long nCount = aQueryNames.getLength();
                        const ::rtl::OUString* pQueryNames = aQueryNames.getConstArray();
                        Image aImg = aImageList.GetImage(IMG_DBQUERY);
                        Image aHCImg = aImageListHC.GetImage(IMG_DBQUERY);
                        for (long i = 0; i < nCount; i++)
                        {
                            sQueryName = pQueryNames[i];
                            SvLBoxEntry* pQueryEntry = InsertEntry(sQueryName, aImg, aImg, pParent, bShowColumns);
                            pQueryEntry->SetUserData((void*)1);
                            SetExpandedEntryBmp(pQueryEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                            SetCollapsedEntryBmp( pQueryEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
                        }
                    }
                }
            }
            catch(const Exception&)
            {
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

String  SwDBTreeList::GetDBName(String& rTableName, String& rColumnName, sal_Bool* pbIsTable)
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
            *pbIsTable = pEntry->GetUserData() == 0;
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
    sal_uInt16 nParent = 0;
    sal_uInt16 nChild = 0;

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
 Beschreibung:
------------------------------------------------------------------------*/

void SwDBTreeList::StartDrag( sal_Int8 /*nAction*/, const Point& /*rPosPixel*/ )
{
    String sTableName, sColumnName;
    String  sDBName( GetDBName( sTableName, sColumnName ));
    if( sDBName.Len() )
    {
        TransferDataContainer* pContainer = new TransferDataContainer;
        STAR_REFERENCE( datatransfer::XTransferable ) xRef( pContainer );
        if( sColumnName.Len() )
        {
            // Datenbankfeld draggen
            svx::OColumnTransferable aColTransfer(
                            sDBName
                            ,::rtl::OUString()
                            , sdb::CommandType::TABLE
                            ,sTableName
                            , sColumnName
                            ,(CTF_FIELD_DESCRIPTOR |CTF_COLUMN_DESCRIPTOR ));
            aColTransfer.addDataToContainer( pContainer );
        }

        sDBName += '.';
        sDBName += sTableName;
        if( sColumnName.Len() )
        {
            sDBName += '.';
            sDBName += sColumnName;
        }

        pContainer->CopyString( FORMAT_STRING, sDBName );
        pContainer->StartDrag( this, DND_ACTION_COPY | DND_ACTION_LINK,
                                Link() );
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/
sal_Int8 SwDBTreeList::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}
/*-- 07.10.2003 13:28:22---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwDBTreeList::SetWrtShell(SwWrtShell& rSh)
{
    pImpl->SetWrtShell(rSh);
    if (IsVisible() && !bInitialized)
        InitTreeList();
}
