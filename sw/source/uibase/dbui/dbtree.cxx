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
#include <vcl/treelistentry.hxx>

#include <bitmaps.hlst>

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

void SwDBTreeList_Impl::elementRemoved( const ContainerEvent& )
{
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
    SetNodeBitmaps(Image(StockImage::Yes, RID_BMP_COLLAPSE),
                   Image(StockImage::Yes, RID_BMP_EXPAND));

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

    Image aImg(StockImage::Yes, RID_BMP_DB);
    for(long i = 0; i < nCount; i++)
    {
        OUString sDBName(pDBNames[i]);
        Reference<XConnection> xConnection = pImpl->GetConnection(sDBName);
        if (xConnection.is())
        {
            InsertEntry(sDBName, aImg, aImg, nullptr, true);
        }
    }
    Select(OUString(), OUString(), OUString());

    bInitialized = true;
}

void SwDBTreeList::AddDataSource(const OUString& rSource)
{
    Image aImg(StockImage::Yes, RID_BMP_DB);
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
                        Image aImg(StockImage::Yes, RID_BMP_DBTABLE);
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
                        Image aImg(StockImage::Yes, RID_BMP_DBQUERY);
                        for (long i = 0; i < nCount; i++)
                        {
                            sQueryName = pQueryNames[i];
                            SvTreeListEntry* pQueryEntry = InsertEntry(sQueryName, aImg, aImg, pParent, bShowColumns);
                            pQueryEntry->SetUserData(reinterpret_cast<void*>(1));
                        }
                    }
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

DBTreeList::DBTreeList(std::unique_ptr<weld::TreeView> xTreeView)
    : bInitialized(false)
    , bShowColumns(false)
    , pImpl(new SwDBTreeList_Impl)
    , m_xTreeView(std::move(xTreeView))
{
    m_xTreeView->connect_expanding(LINK(this, DBTreeList, RequestingChildrenHdl));
#if 0
    if (m_xTreeView->get_visible())
        InitTreeList();
#endif
}

DBTreeList::~DBTreeList()
{
}

void DBTreeList::InitTreeList()
{
    if (!pImpl->HasContext() && pImpl->GetWrtShell())
        return;

//TODO    SetDragDropMode(DragDropMode::APP_COPY);

//TODO    GetModel()->SetCompareHdl(LINK(this, DBTreeList, DBCompare));

    Sequence< OUString > aDBNames = pImpl->GetContext()->getElementNames();
    auto const sort = comphelper::string::NaturalStringSorter(
        comphelper::getProcessComponentContext(),
        Application::GetSettings().GetUILanguageTag().getLocale());
    std::sort(
        aDBNames.begin(), aDBNames.end(),
        [&sort](OUString const & x, OUString const & y)
        { return sort.compare(x, y) < 0; });
    const OUString* pDBNames = aDBNames.getConstArray();
    sal_Int32 nCount = aDBNames.getLength();

    OUString aImg(RID_BMP_DB);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        OUString sDBName(pDBNames[i]);
        Reference<XConnection> xConnection = pImpl->GetConnection(sDBName);
        if (xConnection.is())
        {
            m_xTreeView->insert(nullptr, -1, &sDBName, nullptr, nullptr, nullptr, &aImg, true, nullptr);
        }
    }
    Select(OUString(), OUString(), OUString());

    bInitialized = true;
}

void DBTreeList::AddDataSource(const OUString& rSource)
{
    OUString aImg(RID_BMP_DB);
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    m_xTreeView->insert(nullptr, -1, &rSource, nullptr, nullptr, nullptr, &aImg, true, xIter.get());
    m_xTreeView->select(*xIter);
}

IMPL_LINK(DBTreeList, RequestingChildrenHdl, weld::TreeIter&, rParent, bool)
{
    if (!m_xTreeView->iter_has_child(rParent))
    {
        if (m_xTreeView->get_iter_depth(rParent)) // column names
        {
            try
            {
                std::unique_ptr<weld::TreeIter> xGrandParent(m_xTreeView->make_iterator(&rParent));
                m_xTreeView->iter_parent(*xGrandParent);
                OUString sSourceName = m_xTreeView->get_text(*xGrandParent);
                OUString sTableName = m_xTreeView->get_text(rParent);

                if(!pImpl->GetContext()->hasByName(sSourceName))
                    return true;
                Reference<XConnection> xConnection = pImpl->GetConnection(sSourceName);
                bool bTable = m_xTreeView->get_id(rParent).isEmpty();
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
                        m_xTreeView->append(&rParent, sName);
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
                OUString sSourceName = m_xTreeView->get_text(rParent);
                if (!pImpl->GetContext()->hasByName(sSourceName))
                    return true;
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
                        OUString aImg(RID_BMP_DBTABLE);
                        for (long i = 0; i < nCount; i++)
                        {
                            sTableName = pTableNames[i];
                            m_xTreeView->insert(&rParent, -1, &sTableName, nullptr,
                                                nullptr, nullptr, &aImg, bShowColumns, nullptr);
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
                        OUString aImg(RID_BMP_DBQUERY);
                        for (long i = 0; i < nCount; i++)
                        {
                            sQueryName = pQueryNames[i];
                            //to discriminate between queries and tables the user data of query entries is set
                            OUString sId(OUString::number(1));
                            m_xTreeView->insert(&rParent, -1, &sQueryName, &sId,
                                                nullptr, nullptr, &aImg, bShowColumns, nullptr);
                        }
                    }
                }
            }
            catch (const Exception&)
            {
            }
        }
    }
    return true;
}

#if 0
IMPL_LINK( DBTreeList, DBCompare, const SvSortData&, rData, sal_Int32 )
{
    SvTreeListEntry* pRight = const_cast<SvTreeListEntry*>(rData.pRight);

    if (GetParent(pRight) && GetParent(GetParent(pRight)))
        return 1; // don't sort column names

    return DefaultCompare(rData);   // otherwise call base class
}
#endif

OUString DBTreeList::GetDBName(OUString& rTableName, OUString& rColumnName, sal_Bool* pbIsTable)
{
    OUString sDBName;
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    if (m_xTreeView->get_selected(xIter.get()) && m_xTreeView->get_iter_depth(*xIter))
    {
        if (m_xTreeView->get_iter_depth(*xIter) > 2)
        {
            rColumnName = m_xTreeView->get_text(*xIter);
            m_xTreeView->iter_parent(*xIter); // column name was selected
        }
        if (pbIsTable)
        {
            *pbIsTable = m_xTreeView->get_id(*xIter).isEmpty();
        }
        rTableName = m_xTreeView->get_text(*xIter);
        m_xTreeView->iter_parent(*xIter);
        sDBName = m_xTreeView->get_text(*xIter);
    }
    return sDBName;
}

// Format: database.table
void DBTreeList::Select(const OUString& rDBName, const OUString& rTableName, const OUString& rColumnName)
{
    std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_iter_first(*xParent))
        return;

    do
    {
        if (rDBName == m_xTreeView->get_text(*xParent))
        {
            if (!m_xTreeView->iter_has_child(*xParent))
                m_xTreeView->expand_row(*xParent);
            std::unique_ptr<weld::TreeIter> xChild(m_xTreeView->make_iterator(xParent.get()));
            if (!m_xTreeView->iter_children(*xChild))
                continue;
            do
            {
                if (rTableName == m_xTreeView->get_text(*xChild))
                {
                    m_xTreeView->copy_iterator(*xChild, *xParent);

                    bool bNoChild = false;
                    if (bShowColumns && !rColumnName.isEmpty())
                    {
                        if (!m_xTreeView->iter_has_child(*xParent))
                            m_xTreeView->expand_row(*xParent);

                        bNoChild = true;
                        if (m_xTreeView->iter_children(*xChild))
                        {
                            do
                            {
                                if (rColumnName == m_xTreeView->get_text(*xChild))
                                {
                                    bNoChild = false;
                                    break;
                                }
                            }
                            while (m_xTreeView->iter_next_sibling(*xChild));
                        }
                    }

                    if (bNoChild)
                        m_xTreeView->copy_iterator(*xParent, *xChild);

                    m_xTreeView->scroll_to_row(*xChild);
                    m_xTreeView->select(*xChild);
                    return;
                }
            }
            while (m_xTreeView->iter_next_sibling(*xChild));
        }
    } while (m_xTreeView->iter_next_sibling(*xParent));
}

void DBTreeList::SetWrtShell(SwWrtShell& rSh)
{
    pImpl->SetWrtShell(rSh);
    if (m_xTreeView->get_visible() && !bInitialized)
        InitTreeList();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
