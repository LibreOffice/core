/*************************************************************************
 *
 *  $RCSfile: dbtree.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:20:41 $
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
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif

#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DBTREE_HXX
#include <dbtree.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdb;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::task;
using namespace com::sun::star::beans;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
/* -----------------------------17.07.01 13:10--------------------------------

 ---------------------------------------------------------------------------*/
struct SwConnectionData
{
    OUString                sSourceName;
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

    BOOL                        HasContext();
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
    OUString sSource;
    rEvent.Accessor >>= sSource;
    for(USHORT i = 0; i < aConnections.Count(); i++)
    {
        SwConnectionDataPtr pPtr = aConnections[i];
        if(pPtr->sSourceName == sSource)
        {
            SwConnectionDataPtr pPtr = aConnections[i];
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
BOOL SwDBTreeList_Impl::HasContext()
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
        DBG_ASSERT(xDBContext.is(), "com.sun.star.sdb.DataBaseContext: service not available")
    }
    return xDBContext.is();
}
/* -----------------------------17.07.01 13:24--------------------------------

 ---------------------------------------------------------------------------*/
Reference<XConnection>  SwDBTreeList_Impl::GetConnection(const rtl::OUString& rSourceName)
{
    Reference<XConnection>  xRet;
    for(USHORT i = 0; i < aConnections.Count(); i++)
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
                        const String& rDefDBName, const BOOL bShowCol):

    SvTreeListBox   (pParent, rResId),
    aImageList      (SW_RES(ILIST_DB_DLG    )),
    aImageListHC    (SW_RES(ILIST_DB_DLG_HC )),
    sDefDBName      (rDefDBName),
    bShowColumns    (bShowCol),
    pImpl(new SwDBTreeList_Impl(pSh)),
    bInitialized    (FALSE)
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
    SetWindowBits(WB_HASLINES|WB_CLIPCHILDREN|WB_SORT|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    // Font nicht setzen, damit der Font des Controls uebernommen wird!
    SetSpaceBetweenEntries(0);
    SetNodeBitmaps( aImageList.GetImage(IMG_COLLAPSE),
                    aImageList.GetImage(IMG_EXPAND  ), BMP_COLOR_NORMAL );
    SetNodeBitmaps( aImageListHC.GetImage(IMG_COLLAPSE),
                    aImageListHC.GetImage(IMG_EXPAND  ), BMP_COLOR_HIGHCONTRAST );

    SetDragDropMode(SV_DRAGDROP_APP_COPY);

    GetModel()->SetCompareHdl(LINK(this, SwDBTreeList, DBCompare));

    Sequence<OUString> aDBNames = pImpl->GetContext()->getElementNames();
    const OUString* pDBNames = aDBNames.getConstArray();
    long nCount = aDBNames.getLength();

    Image aImg = aImageList.GetImage(IMG_DB);
    Image aHCImg = aImageListHC.GetImage(IMG_DB);
    for(long i = 0; i < nCount; i++)
    {
        String sDBName(pDBNames[i]);
        SvLBoxEntry* pEntry = InsertEntry(sDBName, aImg, aImg, NULL, TRUE);
        SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
        SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
    }
    String sDBName(sDefDBName.GetToken(0, DB_DELIM));
    String sTableName(sDefDBName.GetToken(1, DB_DELIM));
    String sColumnName(sDefDBName.GetToken(2, DB_DELIM));
    Select(sDBName, sTableName, sColumnName);


    bInitialized = TRUE;
}
/*-- 27.05.2004 09:19:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwDBTreeList::AddDataSource(const String& rSource)
{
    Image aImg = aImageList.GetImage(IMG_DB);
    Image aHCImg = aImageListHC.GetImage(IMG_DB);
    SvLBoxEntry* pEntry = InsertEntry(rSource, aImg, aImg, NULL, TRUE);
    SetExpandedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
    SetCollapsedEntryBmp(pEntry, aHCImg, BMP_COLOR_HIGHCONTRAST);
    SvTreeListBox::Select(pEntry);
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
            try
            {

                String sSourceName = GetEntryText(GetParent(pParent));
                String sTableName = GetEntryText(pParent);

                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                BOOL bTable = pParent->GetUserData() == 0;
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
                        DBG_ASSERT(xQueries->hasByName(sTableName), "table not available anymore?")
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
            catch(const Exception& rEx)
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
                        Sequence<OUString> aTblNames = xTbls->getElementNames();
                        String sTableName;
                        long nCount = aTblNames.getLength();
                        const OUString* pTblNames = aTblNames.getConstArray();
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
                        Sequence<OUString> aQueryNames = xQueries->getElementNames();
                        String sQueryName;
                        long nCount = aQueryNames.getLength();
                        const OUString* pQueryNames = aQueryNames.getConstArray();
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
            catch(const Exception& rEx)
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

void SwDBTreeList::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
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
                            , com::sun::star::sdb::CommandType::TABLE
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
sal_Int8 SwDBTreeList::AcceptDrop( const AcceptDropEvent& rEvt )
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

