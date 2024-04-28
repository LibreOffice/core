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

#include <adtabdlg.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <imageprovider.hxx>
#include <comphelper/containermultiplexer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <algorithm>

// slot ids
using namespace dbaui;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace dbtools;

TableObjectListFacade::~TableObjectListFacade()
{
}

namespace {

class TableListFacade : public ::cppu::BaseMutex
                    ,   public TableObjectListFacade
                    ,   public ::comphelper::OContainerListener
{
    OTableTreeListBox&           m_rTableList;
    Reference< XConnection >    m_xConnection;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                m_pContainerListener;
    bool                        m_bAllowViews;

public:
    TableListFacade(OTableTreeListBox& _rTableList, const Reference< XConnection >& _rxConnection)
        : ::comphelper::OContainerListener(m_aMutex)
        ,m_rTableList( _rTableList )
        ,m_xConnection( _rxConnection )
        ,m_bAllowViews(true)
    {
    }
    virtual ~TableListFacade() override;

private:
    virtual void    updateTableObjectList( bool _bAllowViews ) override;
    virtual OUString  getSelectedName( OUString& _out_rAliasName ) const override;
    virtual bool    isLeafSelected() const override;
    // OContainerListener
    virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) override;
    virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) override;
    virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) override;
};

}

TableListFacade::~TableListFacade()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

OUString TableListFacade::getSelectedName( OUString& _out_rAliasName ) const
{
    weld::TreeView& rTableList = m_rTableList.GetWidget();
    std::unique_ptr<weld::TreeIter> xEntry(rTableList.make_iterator());

    if (!rTableList.get_selected(xEntry.get()))
        return OUString();

    OUString aCatalog, aSchema, aTableName;
    std::unique_ptr<weld::TreeIter> xSchema(rTableList.make_iterator(xEntry.get()));
    if (rTableList.iter_parent(*xSchema))
    {
        auto xAll = m_rTableList.getAllObjectsEntry();
        if (!xAll || !xSchema->equal(*xAll))
        {
            std::unique_ptr<weld::TreeIter> xCatalog(rTableList.make_iterator(xSchema.get()));
            if (rTableList.iter_parent(*xCatalog))
            {
                if (!xAll || !xCatalog->equal(*xAll))
                    aCatalog = rTableList.get_text(*xCatalog, 0);
            }
            aSchema = rTableList.get_text(*xSchema, 0);
        }
    }
    aTableName = rTableList.get_text(*xEntry, 0);

    OUString aComposedName;
    try
    {
        Reference< XDatabaseMetaData > xMeta( m_xConnection->getMetaData(), UNO_SET_THROW );
        if (  aCatalog.isEmpty()
            && !aSchema.isEmpty()
            && xMeta->supportsCatalogsInDataManipulation()
            && !xMeta->supportsSchemasInDataManipulation() )
        {
            aCatalog = aSchema;
            aSchema.clear();
        }

        aComposedName = ::dbtools::composeTableName(
            xMeta, aCatalog, aSchema, aTableName, false, ::dbtools::EComposeRule::InDataManipulation );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    _out_rAliasName = aTableName;
    return aComposedName;
}

void TableListFacade::_elementInserted( const container::ContainerEvent& /*_rEvent*/ )
{
    updateTableObjectList(m_bAllowViews);
}

void TableListFacade::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ )
{
    updateTableObjectList(m_bAllowViews);
}

void TableListFacade::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ )
{
}

void TableListFacade::updateTableObjectList( bool _bAllowViews )
{
    m_bAllowViews = _bAllowViews;
    weld::TreeView& rTableList = m_rTableList.GetWidget();
    rTableList.clear();
    try
    {
        Reference< XTablesSupplier > xTableSupp( m_xConnection, UNO_QUERY_THROW );

        Reference< XViewsSupplier > xViewSupp;
        Reference< XNameAccess > xTables, xViews;
        Sequence< OUString > sTables, sViews;

        xTables = xTableSupp->getTables();
        if ( xTables.is() )
        {
            if ( !m_pContainerListener.is() )
            {
                Reference< XContainer> xContainer(xTables,uno::UNO_QUERY);
                if ( xContainer.is() )
                    m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
            }
            sTables = xTables->getElementNames();
        }

        xViewSupp.set( xTableSupp, UNO_QUERY );
        if ( xViewSupp.is() )
        {
            xViews = xViewSupp->getViews();
            if ( xViews.is() )
                sViews = xViews->getElementNames();
        }

        // if no views are allowed remove the views also out the table name filter
        if ( !_bAllowViews )
        {
            std::vector<OUString> aTables(sTables.begin(), sTables.end());

            for (auto& view : sViews)
                std::erase_if(aTables, [Equal = comphelper::UStringMixEqual(), &view](auto& s)
                              { return Equal(s, view); });
            sTables = Sequence< OUString>(aTables.data(), aTables.size());
            sViews = Sequence< OUString>();
        }

        m_rTableList.UpdateTableList( m_xConnection, sTables, sViews );

        std::unique_ptr<weld::TreeIter> xEntry(rTableList.make_iterator());
        bool bEntry = rTableList.get_iter_first(*xEntry);
        while (bEntry && rTableList.iter_has_child(*xEntry))
        {
            rTableList.expand_row(*xEntry);
            bEntry = rTableList.iter_next(*xEntry);
        }
        if (bEntry)
            rTableList.select(*xEntry);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

bool TableListFacade::isLeafSelected() const
{
    weld::TreeView& rTableList = m_rTableList.GetWidget();
    std::unique_ptr<weld::TreeIter> xEntry(rTableList.make_iterator());
    const bool bEntry = rTableList.get_selected(xEntry.get());
    return bEntry && !rTableList.iter_has_child(*xEntry);
}

namespace {

class QueryListFacade : public ::cppu::BaseMutex
                    ,   public TableObjectListFacade
                    ,   public ::comphelper::OContainerListener
{
    weld::TreeView&             m_rQueryList;
    Reference< XConnection >    m_xConnection;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                m_pContainerListener;

public:
    QueryListFacade( weld::TreeView& _rQueryList, const Reference< XConnection >& _rxConnection )
        : ::comphelper::OContainerListener(m_aMutex)
        ,m_rQueryList( _rQueryList )
        ,m_xConnection( _rxConnection )
    {
    }
    virtual ~QueryListFacade() override;

private:
    virtual void    updateTableObjectList( bool _bAllowViews ) override;
    virtual OUString  getSelectedName( OUString& _out_rAliasName ) const override;
    virtual bool    isLeafSelected() const override;
    // OContainerListener
    virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) override;
    virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) override;
    virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) override;
};

}

QueryListFacade::~QueryListFacade()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

void QueryListFacade::_elementInserted( const container::ContainerEvent& _rEvent )
{
    OUString sName;
    if ( _rEvent.Accessor >>= sName )
    {
        OUString aQueryImage(ImageProvider::getDefaultImageResourceID(css::sdb::application::DatabaseObject::QUERY));
        m_rQueryList.append("", sName, aQueryImage);
    }
}

void QueryListFacade::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ )
{
    updateTableObjectList(true);
}

void QueryListFacade::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ )
{
}

void QueryListFacade::updateTableObjectList( bool /*_bAllowViews*/ )
{
    m_rQueryList.clear();
    try
    {
        OUString aQueryImage(ImageProvider::getDefaultImageResourceID(css::sdb::application::DatabaseObject::QUERY));

        Reference< XQueriesSupplier > xSuppQueries( m_xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xQueries( xSuppQueries->getQueries(), UNO_SET_THROW );
        if ( !m_pContainerListener.is() )
        {
            Reference< XContainer> xContainer(xQueries,UNO_QUERY_THROW);
            m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
        }
        const Sequence< OUString > aQueryNames = xQueries->getElementNames();

        for ( auto const & name : aQueryNames )
            m_rQueryList.append("", name, aQueryImage);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

OUString QueryListFacade::getSelectedName( OUString& _out_rAliasName ) const
{
    OUString sSelected;
    std::unique_ptr<weld::TreeIter> xEntry(m_rQueryList.make_iterator());
    const bool bEntry = m_rQueryList.get_selected(xEntry.get());
    if (bEntry)
        sSelected = _out_rAliasName = m_rQueryList.get_text(*xEntry, 0);
    return sSelected;
}

bool QueryListFacade::isLeafSelected() const
{
    std::unique_ptr<weld::TreeIter> xEntry(m_rQueryList.make_iterator());
    const bool bEntry = m_rQueryList.get_selected(xEntry.get());
    return bEntry && !m_rQueryList.iter_has_child(*xEntry);

}

OAddTableDlg::OAddTableDlg(weld::Window* pParent, IAddTableDialogContext& _rContext)
   : GenericDialogController(pParent, "dbaccess/ui/tablesjoindialog.ui", "TablesJoinDialog")
   , m_rContext(_rContext)
   , m_xCaseTables(m_xBuilder->weld_radio_button("tables"))
   , m_xCaseQueries(m_xBuilder->weld_radio_button("queries"))
   // false means: do not show any buttons
   , m_xTableList(new OTableTreeListBox(m_xBuilder->weld_tree_view("tablelist"), false))
   , m_xQueryList(m_xBuilder->weld_tree_view("querylist"))
   , m_xAddButton(m_xBuilder->weld_button("add"))
   , m_xCloseButton(m_xBuilder->weld_button("close"))
{
    weld::TreeView& rTableList = m_xTableList->GetWidget();
    Size aSize(rTableList.get_approximate_digit_width() * 23,
               rTableList.get_height_rows(15));
    rTableList.set_size_request(aSize.Width(), aSize.Height());
    m_xQueryList->set_size_request(aSize.Width(), aSize.Height());

    m_xCaseTables->connect_toggled(LINK(this, OAddTableDlg, OnTypeSelected));
    m_xAddButton->connect_clicked( LINK( this, OAddTableDlg, AddClickHdl ) );
    m_xCloseButton->connect_clicked( LINK( this, OAddTableDlg, CloseClickHdl ) );
    rTableList.connect_row_activated( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    rTableList.connect_changed( LINK( this, OAddTableDlg, TableListSelectHdl ) );
    m_xQueryList->connect_row_activated( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    m_xQueryList->connect_changed( LINK( this, OAddTableDlg, TableListSelectHdl ) );

    rTableList.set_selection_mode(SelectionMode::Single);
    m_xTableList->SuppressEmptyFolders();

    m_xQueryList->set_selection_mode(SelectionMode::Single);

    if ( !m_rContext.allowQueries() )
    {
        m_xCaseTables->hide();
        m_xCaseQueries->hide();
    }

    m_xDialog->set_title(getDialogTitleForContext(m_rContext));
}

OAddTableDlg::~OAddTableDlg()
{
}

void OAddTableDlg::impl_switchTo( ObjectList _eList )
{
    switch ( _eList )
    {
    case Tables:
        m_xTableList->GetWidget().show(); m_xCaseTables->set_active(true);
        m_xQueryList->hide(); m_xCaseQueries->set_active(false);
        m_xCurrentList.reset( new TableListFacade( *m_xTableList, m_rContext.getConnection() ) );
        m_xTableList->GetWidget().grab_focus();
        break;

    case Queries:
        m_xTableList->GetWidget().hide(); m_xCaseTables->set_active(false);
        m_xQueryList->show();  m_xCaseQueries->set_active(true);
        m_xCurrentList.reset( new QueryListFacade( *m_xQueryList, m_rContext.getConnection() ) );
        m_xQueryList->grab_focus();
        break;
    }
    m_xCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

void OAddTableDlg::Update()
{
    if (!m_xCurrentList)
        impl_switchTo( Tables );
    else
        m_xCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

IMPL_LINK_NOARG( OAddTableDlg, AddClickHdl, weld::Button&, void )
{
    TableListDoubleClickHdl(m_xTableList->GetWidget());
}

IMPL_LINK_NOARG(OAddTableDlg, TableListDoubleClickHdl, weld::TreeView&, bool)
{
    if ( impl_isAddAllowed() )
    {
        if ( m_xCurrentList->isLeafSelected() )
        {
            OUString sSelectedName, sAliasName;
            sSelectedName = m_xCurrentList->getSelectedName( sAliasName );

            m_rContext.addTableWindow( sSelectedName, sAliasName );
        }
        if ( !impl_isAddAllowed() )
            m_xDialog->response(RET_CLOSE);
    }
    return true;
}

IMPL_LINK_NOARG( OAddTableDlg, TableListSelectHdl, weld::TreeView&, void )
{
    m_xAddButton->set_sensitive( m_xCurrentList->isLeafSelected() );
}

IMPL_LINK_NOARG( OAddTableDlg, CloseClickHdl, weld::Button&, void )
{
    m_xDialog->response(RET_CLOSE);
}

IMPL_LINK_NOARG(OAddTableDlg, OnTypeSelected, weld::Toggleable&, void)
{
    if ( m_xCaseTables->get_active() )
        impl_switchTo( Tables );
    else
        impl_switchTo( Queries );
}

void OAddTableDlg::OnClose()
{
    m_rContext.onWindowClosing();
}

bool OAddTableDlg::impl_isAddAllowed()
{
    return  m_rContext.allowAddition();
}

OUString OAddTableDlg::getDialogTitleForContext( IAddTableDialogContext const & _rContext )
{
    OUString sTitle;

    if ( _rContext.allowQueries() )
        sTitle = DBA_RES( STR_ADD_TABLE_OR_QUERY );
    else
        sTitle = DBA_RES( STR_ADD_TABLES );

    return sTitle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
