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

#include <sot/formats.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <cppuhelper/implbase.hxx>
#include <svx/dbaexchange.hxx>

#include <dbmgr.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <dbtree.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>
#include <svtools/treelistentry.hxx>

#include <helpids.h>
#include <strings.hrc>
#include "bitmaps.hlst"

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

class SwDBTreeList_Impl : public cppu::WeakImplHelper < XContainerListener >
{
    Reference< XDatabaseContext > m_xDatabaseContext;
    SwWrtShell* m_pWrtShell;

    public:
        explicit SwDBTreeList_Impl()
            : m_pWrtShell(nullptr)
        {
        }
        virtual ~SwDBTreeList_Impl() override;

    virtual void SAL_CALL elementInserted( const ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const ContainerEvent& Event ) override;
    virtual void SAL_CALL disposing( const EventObject& Source ) override;

    bool                        HasContext();
    SwWrtShell*                 GetWrtShell() { return m_pWrtShell;}
    void                        SetWrtShell(SwWrtShell& rSh) { m_pWrtShell = &rSh;}
    const Reference<XDatabaseContext>& GetContext() const {return m_xDatabaseContext;}
    Reference<XConnection>      GetConnection(const OUString& rSourceName);
};

SwDBTreeList_Impl::~SwDBTreeList_Impl()
{
    if(m_xDatabaseContext.is())
    {
        m_refCount++;
        //block necessary due to solaris' compiler behaviour to
        //remove temporaries at the block's end
        {
            m_xDatabaseContext->removeContainerListener( this );
        }
        m_refCount--;
    }
}

void SwDBTreeList_Impl::elementInserted( const ContainerEvent&  )
{
    // information not needed
}

void SwDBTreeList_Impl::elementRemoved( const ContainerEvent& rEvent )
{
    SolarMutexGuard aGuard;
    OUString sSource;
    rEvent.Accessor >>= sSource;
}

void SwDBTreeList_Impl::disposing( const EventObject&  )
{
    m_xDatabaseContext = nullptr;
}

void SwDBTreeList_Impl::elementReplaced( const ContainerEvent& rEvent )
{
    elementRemoved(rEvent);
}

bool SwDBTreeList_Impl::HasContext()
{
    if(!m_xDatabaseContext.is())
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        m_xDatabaseContext = DatabaseContext::create(xContext);
        m_xDatabaseContext->addContainerListener( this );
    }
    return m_xDatabaseContext.is();
}

Reference<XConnection>  SwDBTreeList_Impl::GetConnection(const OUString& rSourceName)
{
    Reference<XConnection> xRet;
    if (m_xDatabaseContext.is() && m_pWrtShell)
    {
        xRet = m_pWrtShell->GetDBManager()->RegisterConnection(rSourceName);
    }
    return xRet;
}

SwDBTreeList::SwDBTreeList(vcl::Window *pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , bInitialized(false)
    , bShowColumns(false)
    , pImpl(new SwDBTreeList_Impl)
{
    if (IsVisible())
        InitTreeList();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(SwDBTreeList, WB_TABSTOP)

Size SwDBTreeList::GetOptimalSize() const
{
    return LogicToPixel(Size(100, 62), MapMode(MapUnit::MapAppFont));
}

SwDBTreeList::~SwDBTreeList()
{
    disposeOnce();
}

void SwDBTreeList::dispose()
{
    pImpl = nullptr;
    SvTreeListBox::dispose();
}

void SwDBTreeList::InitTreeList()
{
    if(!pImpl->HasContext() && pImpl->GetWrtShell())
        return;
    SetSelectionMode(SelectionMode::Single);
    SetStyle(GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|WB_HASBUTTONS|WB_HASBUTTONSATROOT|WB_HSCROLL);
    // don't set font, so that the Control's font is being applied!
    SetSpaceBetweenEntries(0);
    SetNodeBitmaps(Image(BitmapEx(RID_BMP_COLLAPSE)),
                   Image(BitmapEx(RID_BMP_EXPAND)));

    SetDragDropMode(DragDropMode::APP_COPY);

    GetModel()->SetCompareHdl(LINK(this, SwDBTreeList, DBCompare));

    Sequence< OUString > aDBNames = pImpl->GetContext()->getElementNames();
    auto const sort = comphelper::string::NaturalStringSorter(
        comphelper::getProcessComponentContext(),
        Application::GetSettings().GetUILanguageTag().getLocale());
    std::sort(
        aDBNames.begin(), aDBNames.end(),
        [&sort](OUString const & x, OUString const & y)
        { return sort.compare(x, y) < 0; });
    const OUString* pDBNames = aDBNames.getConstArray();
    long nCount = aDBNames.getLength();

    Image aImg(BitmapEx(RID_BMP_DB));
    for(long i = 0; i < nCount; i++)
    {
        OUString sDBName(pDBNames[i]);
        InsertEntry(sDBName, aImg, aImg, nullptr, true);
    }
    Select(OUString(), OUString(), OUString());

    bInitialized = true;
}

void SwDBTreeList::AddDataSource(const OUString& rSource)
{
    Image aImg(BitmapEx(RID_BMP_DB));
    SvTreeListEntry* pEntry = InsertEntry(rSource, aImg, aImg, nullptr, true);
    SvTreeListBox::Select(pEntry);
}

void SwDBTreeList::ShowColumns(bool bShowCol)
{
    if (bShowCol != bShowColumns)
    {
        bShowColumns = bShowCol;
        OUString sTableName;
        OUString sColumnName;
        const OUString sDBName(GetDBName(sTableName, sColumnName));

        SetUpdateMode(false);

        SvTreeListEntry* pEntry = First();

        while (pEntry)
        {
            pEntry = GetRootLevelParent( pEntry );
            Collapse(pEntry);       // zuklappen

            SvTreeListEntry* pChild;
            while ((pChild = FirstChild(pEntry)) != nullptr)
                GetModel()->Remove(pChild);

            pEntry = Next(pEntry);
        }

        if (!sDBName.isEmpty())
        {
            Select(sDBName, sTableName, sColumnName);   // force RequestingChildren
        }
        SetUpdateMode(true);
    }
}

void  SwDBTreeList::RequestingChildren(SvTreeListEntry* pParent)
{
    if (!pParent->HasChildren())
    {
        if (GetParent(pParent)) // column names
        {
            try
            {

                OUString sSourceName = GetEntryText(GetParent(pParent));
                OUString sTableName = GetEntryText(pParent);

                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                bool bTable = pParent->GetUserData() == nullptr;
                Reference<XColumnsSupplier> xColsSupplier;
                if(bTable)
                {
                    Reference<XTablesSupplier> xTSupplier(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTables = xTSupplier->getTables();
                        OSL_ENSURE(xTables->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aTable = xTables->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aTable >>= xPropSet;
                            xColsSupplier.set(xPropSet, UNO_QUERY);
                        }
                        catch (const Exception&)
                        {
                        }
                    }
                }
                else
                {
                    Reference<XQueriesSupplier> xQSupplier(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        OSL_ENSURE(xQueries->hasByName(sTableName), "table not available anymore?");
                        try
                        {
                            Any aQuery = xQueries->getByName(sTableName);
                            Reference<XPropertySet> xPropSet;
                            aQuery >>= xPropSet;
                            xColsSupplier.set(xPropSet, UNO_QUERY);
                        }
                        catch (const Exception&)
                        {
                        }
                    }
                }

                if(xColsSupplier.is())
                {
                    Reference <XNameAccess> xCols = xColsSupplier->getColumns();
                    Sequence< OUString> aColNames = xCols->getElementNames();
                    const OUString* pColNames = aColNames.getConstArray();
                    long nCount = aColNames.getLength();
                    for (long i = 0; i < nCount; i++)
                    {
                        OUString sName = pColNames[i];
                        InsertEntry(sName, pParent);
                    }
                }
            }
            catch (const Exception&)
            {
            }
        }
        else    // table names
        {
            try
            {
                OUString sSourceName = GetEntryText(pParent);
                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                if (xConnection.is())
                {
                    Reference<XTablesSupplier> xTSupplier(xConnection, UNO_QUERY);
                    if(xTSupplier.is())
                    {
                        Reference<XNameAccess> xTables = xTSupplier->getTables();
                        Sequence< OUString> aTableNames = xTables->getElementNames();
                        OUString sTableName;
                        long nCount = aTableNames.getLength();
                        const OUString* pTableNames = aTableNames.getConstArray();
                        Image aImg(BitmapEx(RID_BMP_DBTABLE));
                        for (long i = 0; i < nCount; i++)
                        {
                            sTableName = pTableNames[i];
                            SvTreeListEntry* pTableEntry = InsertEntry(sTableName, aImg, aImg, pParent, bShowColumns);
                            //to discriminate between queries and tables the user data of table entries is set
                            pTableEntry->SetUserData(nullptr);
                        }
                    }

                    Reference<XQueriesSupplier> xQSupplier(xConnection, UNO_QUERY);
                    if(xQSupplier.is())
                    {
                        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
                        Sequence< OUString> aQueryNames = xQueries->getElementNames();
                        OUString sQueryName;
                        long nCount = aQueryNames.getLength();
                        const OUString* pQueryNames = aQueryNames.getConstArray();
                        Image aImg(BitmapEx(RID_BMP_DBQUERY));
                        for (long i = 0; i < nCount; i++)
                        {
                            sQueryName = pQueryNames[i];
                            SvTreeListEntry* pQueryEntry = InsertEntry(sQueryName, aImg, aImg, pParent, bShowColumns);
                            pQueryEntry->SetUserData(reinterpret_cast<void*>(1));
                        }
                    }
                }
                else
                {
                    // Defunct connection entry
                    RemoveEntry(pParent);
                }
            }
            catch (const Exception&)
            {
            }
        }
    }
}

IMPL_LINK( SwDBTreeList, DBCompare, const SvSortData&, rData, sal_Int32 )
{
    SvTreeListEntry* pRight = const_cast<SvTreeListEntry*>(rData.pRight);

    if (GetParent(pRight) && GetParent(GetParent(pRight)))
        return 1; // don't sort column names

    return DefaultCompare(rData);   // otherwise call base class
}

OUString SwDBTreeList::GetDBName(OUString& rTableName, OUString& rColumnName, sal_Bool* pbIsTable)
{
    OUString sDBName;
    SvTreeListEntry* pEntry = FirstSelected();

    if (pEntry && GetParent(pEntry))
    {
        if (GetParent(GetParent(pEntry)))
        {
            rColumnName = GetEntryText(pEntry);
            pEntry = GetParent(pEntry); // column name was selected
        }
        sDBName = GetEntryText(GetParent(pEntry));
        if(pbIsTable)
        {
            *pbIsTable = pEntry->GetUserData() == nullptr;
        }
        rTableName = GetEntryText(pEntry);
    }
    return sDBName;
}

// Format: database.table
void SwDBTreeList::Select(const OUString& rDBName, const OUString& rTableName, const OUString& rColumnName)
{
    SvTreeListEntry* pParent;
    SvTreeListEntry* pChild;
    sal_uInt16 nParent = 0;
    sal_uInt16 nChild = 0;

    while ((pParent = GetEntry(nParent++)) != nullptr)
    {
        if (rDBName == GetEntryText(pParent))
        {
            if (!pParent->HasChildren())
                RequestingChildren(pParent);
            while ((pChild = GetEntry(pParent, nChild++)) != nullptr)
            {
                if (rTableName == GetEntryText(pChild))
                {
                    pParent = pChild;

                    if (bShowColumns && !rColumnName.isEmpty())
                    {
                        nChild = 0;

                        if (!pParent->HasChildren())
                            RequestingChildren(pParent);

                        while ((pChild = GetEntry(pParent, nChild++)) != nullptr)
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

void SwDBTreeList::StartDrag( sal_Int8 /*nAction*/, const Point& /*rPosPixel*/ )
{
    OUString sTableName;
    OUString sColumnName;
    OUString sDBName( GetDBName( sTableName, sColumnName ));
    if (!sDBName.isEmpty())
    {
        rtl::Reference<TransferDataContainer> pContainer = new TransferDataContainer;
        if( !sColumnName.isEmpty() )
        {
            // drag database field
            rtl::Reference< svx::OColumnTransferable > xColTransfer( new svx::OColumnTransferable(
                            sDBName,
                            sTableName,
                            sColumnName,
                            (ColumnTransferFormatFlags::FIELD_DESCRIPTOR|ColumnTransferFormatFlags::COLUMN_DESCRIPTOR) ) );
            xColTransfer->addDataToContainer( pContainer.get() );
        }

        sDBName += "." + sTableName;
        if (!sColumnName.isEmpty())
        {
            sDBName += "." + sColumnName;
        }

        pContainer->CopyString( SotClipboardFormatId::STRING, sDBName );
        pContainer->StartDrag( this, DND_ACTION_COPY | DND_ACTION_LINK,
                                Link<sal_Int8,void>() );
    }
}

sal_Int8 SwDBTreeList::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

void SwDBTreeList::SetWrtShell(SwWrtShell& rSh)
{
    pImpl->SetWrtShell(rSh);
    if (IsVisible() && !bInitialized)
        InitTreeList();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
