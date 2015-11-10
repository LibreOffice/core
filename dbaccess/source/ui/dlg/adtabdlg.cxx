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

#include "adtabdlg.hxx"
#include "sqlmessage.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <svtools/localresaccess.hxx>
#include "dbaccess_helpid.hrc"
#include "dbu_resource.hrc"
#include "dbu_dlg.hrc"
#include <sfx2/sfxsids.hrc>
#include "QueryTableView.hxx"
#include "QueryDesignView.hxx"
#include "querycontroller.hxx"
#include <connectivity/dbtools.hxx>
#include "browserids.hxx"
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "UITools.hxx"
#include "imageprovider.hxx"
#include <comphelper/containermultiplexer.hxx>
#include "cppuhelper/basemutex.hxx"
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

class TableListFacade : public ::cppu::BaseMutex
                    ,   public TableObjectListFacade
                    ,   public ::comphelper::OContainerListener
{
    OTableTreeListBox&          m_rTableList;
    Reference< XConnection >    m_xConnection;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                m_pContainerListener;
    bool                        m_bAllowViews;

public:
    TableListFacade( OTableTreeListBox& _rTableList, const Reference< XConnection >& _rxConnection )
        : ::comphelper::OContainerListener(m_aMutex)
        ,m_rTableList( _rTableList )
        ,m_xConnection( _rxConnection )
        ,m_bAllowViews(true)
    {
    }
    virtual ~TableListFacade();

private:
    virtual void    updateTableObjectList( bool _bAllowViews ) override;
    virtual OUString  getSelectedName( OUString& _out_rAliasName ) const override;
    virtual bool    isLeafSelected() const override;
    // OContainerListener
    virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
};

TableListFacade::~TableListFacade()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

OUString TableListFacade::getSelectedName( OUString& _out_rAliasName ) const
{
    SvTreeListEntry* pEntry = m_rTableList.FirstSelected();
    if ( !pEntry )
        return OUString();

    OUString aCatalog, aSchema, aTableName;
    SvTreeListEntry* pSchema = m_rTableList.GetParent(pEntry);
    if(pSchema && pSchema != m_rTableList.getAllObjectsEntry())
    {
        SvTreeListEntry* pCatalog = m_rTableList.GetParent(pSchema);
        if(pCatalog && pCatalog != m_rTableList.getAllObjectsEntry())
            aCatalog = m_rTableList.GetEntryText(pCatalog);
        aSchema = m_rTableList.GetEntryText(pSchema);
    }
    aTableName = m_rTableList.GetEntryText(pEntry);

    OUString aComposedName;
    try
    {
        Reference< XDatabaseMetaData > xMeta( m_xConnection->getMetaData(), UNO_QUERY_THROW );
        if (  aCatalog.isEmpty()
            && !aSchema.isEmpty()
            && xMeta->supportsCatalogsInDataManipulation()
            && !xMeta->supportsSchemasInDataManipulation() )
        {
            aCatalog = aSchema;
            aSchema.clear();
        }

        aComposedName = ::dbtools::composeTableName(
            xMeta, aCatalog, aSchema, aTableName, false, ::dbtools::eInDataManipulation );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    _out_rAliasName = aTableName;
    return aComposedName;
}

void TableListFacade::_elementInserted( const container::ContainerEvent& /*_rEvent*/ )  throw(css::uno::RuntimeException, std::exception)
{
    updateTableObjectList(m_bAllowViews);
}

void TableListFacade::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ ) throw(css::uno::RuntimeException, std::exception)
{
    updateTableObjectList(m_bAllowViews);
}

void TableListFacade::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ ) throw(css::uno::RuntimeException, std::exception)
{
}

void TableListFacade::updateTableObjectList( bool _bAllowViews )
{
    m_bAllowViews = _bAllowViews;
    m_rTableList.Clear();
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
            const OUString* pTableBegin  = sTables.getConstArray();
            const OUString* pTableEnd    = pTableBegin + sTables.getLength();
            ::std::vector< OUString > aTables(pTableBegin,pTableEnd);

            const OUString* pViewBegin = sViews.getConstArray();
            const OUString* pViewEnd   = pViewBegin + sViews.getLength();
            ::comphelper::UStringMixEqual aEqualFunctor;
            for(;pViewBegin != pViewEnd;++pViewBegin)
                aTables.erase(::std::remove_if(aTables.begin(),aTables.end(),::std::bind2nd(aEqualFunctor,*pViewBegin)),aTables.end());
            sTables = Sequence< OUString>(aTables.data(), aTables.size());
            sViews = Sequence< OUString>();
        }

        m_rTableList.UpdateTableList( m_xConnection, sTables, sViews );
        SvTreeListEntry* pEntry = m_rTableList.First();
        while( pEntry && m_rTableList.GetModel()->HasChildren( pEntry ) )
        {
            m_rTableList.Expand( pEntry );
            pEntry = m_rTableList.Next( pEntry );
        }
        if ( pEntry )
            m_rTableList.Select(pEntry);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

bool TableListFacade::isLeafSelected() const
{
    SvTreeListEntry* pEntry = m_rTableList.FirstSelected();
    return pEntry && !m_rTableList.GetModel()->HasChildren( pEntry );
}

class QueryListFacade : public ::cppu::BaseMutex
                    ,   public TableObjectListFacade
                    ,   public ::comphelper::OContainerListener
{
    SvTreeListBox&              m_rQueryList;
    Reference< XConnection >    m_xConnection;
    ::rtl::Reference< comphelper::OContainerListenerAdapter>
                                m_pContainerListener;

public:
    QueryListFacade( SvTreeListBox& _rQueryList, const Reference< XConnection >& _rxConnection )
        : ::comphelper::OContainerListener(m_aMutex)
        ,m_rQueryList( _rQueryList )
        ,m_xConnection( _rxConnection )
    {
    }
    virtual ~QueryListFacade();

private:
    virtual void    updateTableObjectList( bool _bAllowViews ) override;
    virtual OUString  getSelectedName( OUString& _out_rAliasName ) const override;
    virtual bool    isLeafSelected() const override;
    // OContainerListener
    virtual void _elementInserted( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void _elementRemoved( const  css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void _elementReplaced( const css::container::ContainerEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;
};

QueryListFacade::~QueryListFacade()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

void QueryListFacade::_elementInserted( const container::ContainerEvent& _rEvent )  throw(css::uno::RuntimeException, std::exception)
{
    OUString sName;
    if ( _rEvent.Accessor >>= sName )
        m_rQueryList.InsertEntry( sName );
}

void QueryListFacade::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ ) throw(css::uno::RuntimeException, std::exception)
{
    updateTableObjectList(true);
}

void QueryListFacade::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ ) throw(css::uno::RuntimeException, std::exception)
{
}

void QueryListFacade::updateTableObjectList( bool /*_bAllowViews*/ )
{
    m_rQueryList.Clear();
    try
    {
        ImageProvider aImageProvider( m_xConnection );
        Image aQueryImage( ImageProvider::getDefaultImage( css::sdb::application::DatabaseObject::QUERY ) );

        m_rQueryList.SetDefaultExpandedEntryBmp( aQueryImage );
        m_rQueryList.SetDefaultCollapsedEntryBmp( aQueryImage );

        Reference< XQueriesSupplier > xSuppQueries( m_xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xQueries( xSuppQueries->getQueries(), UNO_QUERY_THROW );
        if ( !m_pContainerListener.is() )
        {
            Reference< XContainer> xContainer(xQueries,UNO_QUERY_THROW);
            m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
        }
        Sequence< OUString > aQueryNames = xQueries->getElementNames();

        const OUString* pQuery = aQueryNames.getConstArray();
        const OUString* pQueryEnd = aQueryNames.getConstArray() + aQueryNames.getLength();
        while ( pQuery != pQueryEnd )
            m_rQueryList.InsertEntry( *pQuery++ );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

OUString QueryListFacade::getSelectedName( OUString& _out_rAliasName ) const
{
    OUString sSelected;
    SvTreeListEntry* pEntry = m_rQueryList.FirstSelected();
    if ( pEntry )
        sSelected = _out_rAliasName = m_rQueryList.GetEntryText( pEntry );
    return sSelected;
}

bool QueryListFacade::isLeafSelected() const
{
    SvTreeListEntry* pEntry = m_rQueryList.FirstSelected();
    return pEntry && !m_rQueryList.GetModel()->HasChildren( pEntry );
}

OAddTableDlg::OAddTableDlg( vcl::Window* pParent, IAddTableDialogContext& _rContext )
   : ModelessDialog(pParent, "TablesJoinDialog", "dbaccess/ui/tablesjoindialog.ui")
   , m_rContext(_rContext)
{
    get(m_pCaseTables, "tables");
    get(m_pCaseQueries, "queries");

    get(m_pTableList, "tablelist");
    get(m_pQueryList, "querylist");
    Size aSize(LogicToPixel(Size(106 , 122), MAP_APPFONT));
    m_pTableList->set_height_request(aSize.Height());
    m_pTableList->set_width_request(aSize.Width());
    get(m_pQueryList, "querylist");
    m_pQueryList->set_height_request(aSize.Height());
    m_pQueryList->set_width_request(aSize.Width());

    get(m_pAddButton, "add");
    get(m_pCloseButton, "close");

    m_pCaseTables->SetClickHdl( LINK( this, OAddTableDlg, OnTypeSelected ) );
    m_pCaseQueries->SetClickHdl( LINK( this, OAddTableDlg, OnTypeSelected ) );
    m_pAddButton->SetClickHdl( LINK( this, OAddTableDlg, AddClickHdl ) );
    m_pCloseButton->SetClickHdl( LINK( this, OAddTableDlg, CloseClickHdl ) );
    m_pTableList->SetDoubleClickHdl( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    m_pTableList->SetSelectHdl( LINK( this, OAddTableDlg, TableListSelectHdl ) );
    m_pQueryList->SetDoubleClickHdl( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    m_pQueryList->SetSelectHdl( LINK( this, OAddTableDlg, TableListSelectHdl ) );

    m_pTableList->EnableInplaceEditing( false );
    m_pTableList->SetStyle(m_pTableList->GetStyle() | WB_BORDER | WB_HASLINES |WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HASLINESATROOT | WB_SORT | WB_HSCROLL );
    m_pTableList->EnableCheckButton( nullptr ); // do not show any buttons
    m_pTableList->SetSelectionMode( SINGLE_SELECTION );
    m_pTableList->notifyHiContrastChanged();
    m_pTableList->suppressEmptyFolders();

    m_pQueryList->EnableInplaceEditing( false );
    m_pQueryList->SetSelectionMode( SINGLE_SELECTION );

    if ( !m_rContext.allowQueries() )
    {
        m_pCaseTables->Hide();
        m_pCaseQueries->Hide();
    }

    SetText( getDialogTitleForContext( m_rContext ) );
}

OAddTableDlg::~OAddTableDlg()
{
    disposeOnce();
}

void OAddTableDlg::dispose()
{
    m_rContext.onWindowClosing( this );
    m_pCaseTables.clear();
    m_pCaseQueries.clear();
    m_pTableList.clear();
    m_pQueryList.clear();
    m_pAddButton.clear();
    m_pCloseButton.clear();
    ModelessDialog::dispose();
}

void OAddTableDlg::impl_switchTo( ObjectList _eList )
{
    switch ( _eList )
    {
    case Tables:
        m_pTableList->Show();  m_pCaseTables->Check();
        m_pQueryList->Show( false ); m_pCaseQueries->Check( false );
        m_xCurrentList.reset( new TableListFacade( *m_pTableList, m_rContext.getConnection() ) );
        m_pTableList->GrabFocus();
        break;

    case Queries:
        m_pTableList->Show( false ); m_pCaseTables->Check( false );
        m_pQueryList->Show();  m_pCaseQueries->Check();
        m_xCurrentList.reset( new QueryListFacade( *m_pQueryList, m_rContext.getConnection() ) );
        m_pQueryList->GrabFocus();
        break;
    }
    m_xCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

void OAddTableDlg::Update()
{
    if ( !m_xCurrentList.get() )
        impl_switchTo( Tables );
    else
        m_xCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

void OAddTableDlg::impl_addTable()
{
    if ( m_xCurrentList->isLeafSelected() )
    {
        OUString sSelectedName, sAliasName;
        sSelectedName = m_xCurrentList->getSelectedName( sAliasName );

        m_rContext.addTableWindow( sSelectedName, sAliasName );
    }
}

IMPL_LINK_NOARG_TYPED( OAddTableDlg, AddClickHdl, Button*, void )
{
    TableListDoubleClickHdl(nullptr);
}

IMPL_LINK_NOARG_TYPED( OAddTableDlg, TableListDoubleClickHdl, SvTreeListBox*, bool )
{
    if ( impl_isAddAllowed() )
    {
        impl_addTable();
        if ( !impl_isAddAllowed() )
            Close();
        return true;  // handled
    }

    return false;  // not handled
}

IMPL_LINK_NOARG_TYPED( OAddTableDlg, TableListSelectHdl, SvTreeListBox*, void )
{
    m_pAddButton->Enable( m_xCurrentList->isLeafSelected() );
}

IMPL_LINK_NOARG_TYPED( OAddTableDlg, CloseClickHdl, Button*, void )
{
    Close();
}

IMPL_LINK_NOARG_TYPED( OAddTableDlg, OnTypeSelected, Button*, void )
{
    if ( m_pCaseTables->IsChecked() )
        impl_switchTo( Tables );
    else
        impl_switchTo( Queries );
}

bool OAddTableDlg::Close()
{
    m_rContext.onWindowClosing( this );
    return ModelessDialog::Close();
}

bool OAddTableDlg::impl_isAddAllowed()
{
    return  m_rContext.allowAddition();
}

OUString OAddTableDlg::getDialogTitleForContext( IAddTableDialogContext& _rContext )
{
    OUString sTitle;

    if ( _rContext.allowQueries() )
        sTitle = ModuleRes( STR_ADD_TABLE_OR_QUERY );
    else
        sTitle = ModuleRes( STR_ADD_TABLES );

    return sTitle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
