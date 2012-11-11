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
#include "adtabdlg.hrc"
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

// TableObjectListFacade -------------------------------------------------------
TableObjectListFacade::~TableObjectListFacade()
{
}

// TableListFacade -------------------------------------------------------------
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
    virtual void    updateTableObjectList( bool _bAllowViews );
    virtual String  getSelectedName( String& _out_rAliasName ) const;
    virtual bool    isLeafSelected() const;
    // OContainerListener
    virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void _elementRemoved( const  ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
};

TableListFacade::~TableListFacade()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

String TableListFacade::getSelectedName( String& _out_rAliasName ) const
{
    SvTreeListEntry* pEntry = m_rTableList.FirstSelected();
    if ( !pEntry )
        return String();

    ::rtl::OUString aCatalog, aSchema, aTableName;
    SvTreeListEntry* pSchema = m_rTableList.GetParent(pEntry);
    if(pSchema && pSchema != m_rTableList.getAllObjectsEntry())
    {
        SvTreeListEntry* pCatalog = m_rTableList.GetParent(pSchema);
        if(pCatalog && pCatalog != m_rTableList.getAllObjectsEntry())
            aCatalog = m_rTableList.GetEntryText(pCatalog);
        aSchema = m_rTableList.GetEntryText(pSchema);
    }
    aTableName = m_rTableList.GetEntryText(pEntry);

    ::rtl::OUString aComposedName;
    try
    {
        Reference< XDatabaseMetaData > xMeta( m_xConnection->getMetaData(), UNO_QUERY_THROW );
        if (  aCatalog.isEmpty()
            && !aSchema.isEmpty()
            && xMeta->supportsCatalogsInDataManipulation()
            && !xMeta->supportsSchemasInDataManipulation() )
        {
            aCatalog = aSchema;
            aSchema = ::rtl::OUString();
        }

        aComposedName = ::dbtools::composeTableName(
            xMeta, aCatalog, aSchema, aTableName, sal_False, ::dbtools::eInDataManipulation );
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    _out_rAliasName = aTableName;
    return aComposedName;
}

void TableListFacade::_elementInserted( const container::ContainerEvent& /*_rEvent*/ )  throw(::com::sun::star::uno::RuntimeException)
{
    updateTableObjectList(m_bAllowViews);
}

void TableListFacade::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    updateTableObjectList(m_bAllowViews);
}

void TableListFacade::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ ) throw(::com::sun::star::uno::RuntimeException)
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
        Sequence< ::rtl::OUString > sTables, sViews;

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
            const ::rtl::OUString* pTableBegin  = sTables.getConstArray();
            const ::rtl::OUString* pTableEnd    = pTableBegin + sTables.getLength();
            ::std::vector< ::rtl::OUString > aTables(pTableBegin,pTableEnd);

            const ::rtl::OUString* pViewBegin = sViews.getConstArray();
            const ::rtl::OUString* pViewEnd   = pViewBegin + sViews.getLength();
            ::comphelper::TStringMixEqualFunctor aEqualFunctor;
            for(;pViewBegin != pViewEnd;++pViewBegin)
                aTables.erase(::std::remove_if(aTables.begin(),aTables.end(),::std::bind2nd(aEqualFunctor,*pViewBegin)),aTables.end());
            ::rtl::OUString* pTables = aTables.empty() ? 0 : &aTables[0];
            sTables = Sequence< ::rtl::OUString>(pTables, aTables.size());
            sViews = Sequence< ::rtl::OUString>();
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

// QueryListFacade -------------------------------------------------------------
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
    virtual void    updateTableObjectList( bool _bAllowViews );
    virtual String  getSelectedName( String& _out_rAliasName ) const;
    virtual bool    isLeafSelected() const;
    // OContainerListener
    virtual void _elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void _elementRemoved( const  ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
    virtual void _elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);
};

QueryListFacade::~QueryListFacade()
{
    if ( m_pContainerListener.is() )
        m_pContainerListener->dispose();
}

void QueryListFacade::_elementInserted( const container::ContainerEvent& _rEvent )  throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;
    if ( _rEvent.Accessor >>= sName )
        m_rQueryList.InsertEntry( sName );
}

void QueryListFacade::_elementRemoved( const container::ContainerEvent& /*_rEvent*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    updateTableObjectList(true);
}

void QueryListFacade::_elementReplaced( const container::ContainerEvent& /*_rEvent*/ ) throw(::com::sun::star::uno::RuntimeException)
{
}

void QueryListFacade::updateTableObjectList( bool /*_bAllowViews*/ )
{
    m_rQueryList.Clear();
    try
    {
        ImageProvider aImageProvider( m_xConnection );
        Image aQueryImage( aImageProvider.getDefaultImage( DatabaseObject::QUERY ) );

        m_rQueryList.SetDefaultExpandedEntryBmp( aQueryImage );
        m_rQueryList.SetDefaultCollapsedEntryBmp( aQueryImage );

        Reference< XQueriesSupplier > xSuppQueries( m_xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xQueries( xSuppQueries->getQueries(), UNO_QUERY_THROW );
        if ( !m_pContainerListener.is() )
        {
            Reference< XContainer> xContainer(xQueries,UNO_QUERY_THROW);
            m_pContainerListener = new ::comphelper::OContainerListenerAdapter(this,xContainer);
        }
        Sequence< ::rtl::OUString > aQueryNames = xQueries->getElementNames();

        const ::rtl::OUString* pQuery = aQueryNames.getConstArray();
        const ::rtl::OUString* pQueryEnd = aQueryNames.getConstArray() + aQueryNames.getLength();
        while ( pQuery != pQueryEnd )
            m_rQueryList.InsertEntry( *pQuery++ );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

String QueryListFacade::getSelectedName( String& _out_rAliasName ) const
{
    String sSelected;
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

// OAddTableDlg ----------------------------------------------------------------
OAddTableDlg::OAddTableDlg( Window* pParent, IAddTableDialogContext& _rContext )
             :ModelessDialog( pParent, ModuleRes(DLG_JOIN_TABADD) )
             ,m_aCaseTables( this, ModuleRes( RB_CASE_TABLES ) )
             ,m_aCaseQueries( this, ModuleRes( RB_CASE_QUERIES ) )
             ,m_aTableList( this, NULL, ModuleRes( LB_TABLE_OR_QUERY ), sal_False )
             ,m_aQueryList( this, ModuleRes( LB_TABLE_OR_QUERY ) )
             ,aAddButton( this, ModuleRes( PB_ADDTABLE ) )
             ,aCloseButton( this, ModuleRes( PB_CLOSE ) )
             ,aHelpButton( this, ModuleRes( PB_HELP ) )
             ,m_rContext( _rContext )
{
    // the Close-Button already has a standard help text which should not
    // occur here. Therefore, resetting the text and giving a new ID
    aCloseButton.SetHelpText(String());
    aCloseButton.SetHelpId(HID_JOINSH_ADDTAB_CLOSE);

    m_aTableList.SetHelpId( HID_JOINSH_ADDTAB_TABLELIST );
    m_aQueryList.SetHelpId( HID_JOINSH_ADDTAB_QUERYLIST );

    m_aCaseTables.SetClickHdl( LINK( this, OAddTableDlg, OnTypeSelected ) );
    m_aCaseQueries.SetClickHdl( LINK( this, OAddTableDlg, OnTypeSelected ) );
    aAddButton.SetClickHdl( LINK( this, OAddTableDlg, AddClickHdl ) );
    aCloseButton.SetClickHdl( LINK( this, OAddTableDlg, CloseClickHdl ) );
    m_aTableList.SetDoubleClickHdl( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    m_aTableList.SetSelectHdl( LINK( this, OAddTableDlg, TableListSelectHdl ) );
    m_aQueryList.SetDoubleClickHdl( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    m_aQueryList.SetSelectHdl( LINK( this, OAddTableDlg, TableListSelectHdl ) );

    m_aTableList.EnableInplaceEditing( sal_False );
    m_aTableList.SetStyle(m_aTableList.GetStyle() | WB_BORDER | WB_HASLINES |WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HASLINESATROOT | WB_SORT | WB_HSCROLL );
    m_aTableList.EnableCheckButton( NULL ); // do not show any buttons
    m_aTableList.SetSelectionMode( SINGLE_SELECTION );
    m_aTableList.notifyHiContrastChanged();
    m_aTableList.suppressEmptyFolders();

    m_aQueryList.EnableInplaceEditing( sal_False );
    m_aQueryList.SetSelectionMode( SINGLE_SELECTION );

    if ( !m_rContext.allowQueries() )
    {
        m_aCaseTables.Hide();
        m_aCaseQueries.Hide();

        long nPixelDiff = m_aTableList.GetPosPixel().Y() - m_aCaseTables.GetPosPixel().Y();

        Point aListPos( m_aTableList.GetPosPixel() );
        aListPos.Y() -= nPixelDiff;

        Size aListSize( m_aTableList.GetSizePixel() );
        aListSize.Height() += nPixelDiff;

        m_aTableList.SetPosSizePixel( aListPos, aListSize );
    }

    FreeResource();

    SetText( getDialogTitleForContext( m_rContext ) );
}

OAddTableDlg::~OAddTableDlg()
{
    m_rContext.onWindowClosing( this );
}

void OAddTableDlg::impl_switchTo( ObjectList _eList )
{
    switch ( _eList )
    {
    case Tables:
        m_aTableList.Show( sal_True );  m_aCaseTables.Check( sal_True );
        m_aQueryList.Show( sal_False ); m_aCaseQueries.Check( sal_False );
        m_pCurrentList.reset( new TableListFacade( m_aTableList, m_rContext.getConnection() ) );
        m_aTableList.GrabFocus();
        break;

    case Queries:
        m_aTableList.Show( sal_False ); m_aCaseTables.Check( sal_False );
        m_aQueryList.Show( sal_True );  m_aCaseQueries.Check( sal_True );
        m_pCurrentList.reset( new QueryListFacade( m_aQueryList, m_rContext.getConnection() ) );
        m_aQueryList.GrabFocus();
        break;
    }
    m_pCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

void OAddTableDlg::Update()
{
    if ( !m_pCurrentList.get() )
        impl_switchTo( Tables );
    else
        m_pCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

void OAddTableDlg::impl_addTable()
{
    if ( m_pCurrentList->isLeafSelected() )
    {
        String sSelectedName, sAliasName;
        sSelectedName = m_pCurrentList->getSelectedName( sAliasName );

        m_rContext.addTableWindow( sSelectedName, sAliasName );
    }
}

IMPL_LINK( OAddTableDlg, AddClickHdl, Button*, /*pButton*/ )
{
    TableListDoubleClickHdl(NULL);
    return 0;
}

IMPL_LINK( OAddTableDlg, TableListDoubleClickHdl, void*, /*EMPTY_ARG*/ )
{
    if ( impl_isAddAllowed() )
    {
        impl_addTable();
        if ( !impl_isAddAllowed() )
            Close();
        return 1L;  // handled
    }

    return 0L;  // not handled
}

IMPL_LINK( OAddTableDlg, TableListSelectHdl, void*, /*EMPTY_ARG*/ )
{
    aAddButton.Enable( m_pCurrentList->isLeafSelected() );
    return 0;
}

IMPL_LINK( OAddTableDlg, CloseClickHdl, Button*, /*pButton*/ )
{
    return Close();
}

IMPL_LINK( OAddTableDlg, OnTypeSelected, void*, /*EMPTY_ARG*/ )
{
    if ( m_aCaseTables.IsChecked() )
        impl_switchTo( Tables );
    else
        impl_switchTo( Queries );
    return 0;
}

sal_Bool OAddTableDlg::Close()
{
    m_rContext.onWindowClosing( this );
    return ModelessDialog::Close();
}

bool OAddTableDlg::impl_isAddAllowed()
{
    return  m_rContext.allowAddition();
}

String OAddTableDlg::getDialogTitleForContext( IAddTableDialogContext& _rContext )
{
    String sTitle;

    ::svt::OLocalResourceAccess aLocalRes( ModuleRes( DLG_JOIN_TABADD ), RSC_MODELESSDIALOG );
    if ( _rContext.allowQueries() )
        sTitle = String( ModuleRes( STR_ADD_TABLE_OR_QUERY ) );
    else
        sTitle = String( ModuleRes( STR_ADD_TABLES ) );

    return sTitle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
