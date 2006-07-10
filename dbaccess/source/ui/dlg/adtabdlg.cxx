/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adtabdlg.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:25:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef DBAUI_QYDLGTAB_HXX
#include "adtabdlg.hxx"
#endif
#ifndef DBAUI_ADTABDLG_HRC
#include "adtabdlg.hrc"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

#include <algorithm>


// slot ids
using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace dbtools;

//==============================================================================
//= TableObjectListFacade
//==============================================================================
TableObjectListFacade::~TableObjectListFacade()
{
}

//==============================================================================
//= TableListFacade
//==============================================================================
class TableListFacade : public TableObjectListFacade
{
    OTableTreeListBox&          m_rTableList;
    Reference< XConnection >    m_xConnection;

public:
    TableListFacade( OTableTreeListBox& _rTableList, const Reference< XConnection >& _rxConnection )
        :m_rTableList( _rTableList )
        ,m_xConnection( _rxConnection )
    {
    }

    virtual void    updateTableObjectList( bool _bAllowViews );
    virtual String  getSelectedName( String& _out_rAliasName ) const;
    virtual bool    isLeafSelected() const;
};

//------------------------------------------------------------------------------
String TableListFacade::getSelectedName( String& _out_rAliasName ) const
{
    SvLBoxEntry* pEntry = m_rTableList.FirstSelected();
    if ( !pEntry )
        return String();

    ::rtl::OUString aCatalog, aSchema, aTableName;
    SvLBoxEntry* pSchema = m_rTableList.GetParent(pEntry);
    if(pSchema && pSchema != m_rTableList.getAllObjectsEntry())
    {
        SvLBoxEntry* pCatalog = m_rTableList.GetParent(pSchema);
        if(pCatalog && pCatalog != m_rTableList.getAllObjectsEntry())
            aCatalog = m_rTableList.GetEntryText(pCatalog);
        aSchema = m_rTableList.GetEntryText(pSchema);
    }
    aTableName = m_rTableList.GetEntryText(pEntry);

    ::rtl::OUString aComposedName;
    try
    {
        Reference< XDatabaseMetaData > xMeta( m_xConnection->getMetaData(), UNO_QUERY_THROW );
        if (  !aCatalog.getLength()
            && aSchema.getLength()
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

//------------------------------------------------------------------------------
void TableListFacade::updateTableObjectList( bool _bAllowViews )
{
    m_rTableList.Clear();
    try
    {
        Reference< XTablesSupplier > xTableSupp( m_xConnection, UNO_QUERY_THROW );
        Reference< XViewsSupplier > xViewSupp;
        Reference< XNameAccess > xTables, xViews;
        Sequence< ::rtl::OUString > sTables, sViews;

        xTables = xTableSupp->getTables();
        if ( xTables.is() )
            sTables = xTables->getElementNames();

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

        m_rTableList.UpdateTableList( m_xConnection->getMetaData(), sTables, sViews );
        SvLBoxEntry* pEntry = m_rTableList.First();
        while( pEntry && m_rTableList.GetModel()->HasChilds( pEntry ) )
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

//------------------------------------------------------------------------------
bool TableListFacade::isLeafSelected() const
{
    SvLBoxEntry* pEntry = m_rTableList.FirstSelected();
    return pEntry && !m_rTableList.GetModel()->HasChilds( pEntry );
}

//==============================================================================
//= QueryListFacade
//==============================================================================
class QueryListFacade : public TableObjectListFacade
{
    SvTreeListBox&              m_rQueryList;
    Reference< XConnection >    m_xConnection;

public:
    QueryListFacade( SvTreeListBox& _rQueryList, const Reference< XConnection >& _rxConnection )
        :m_rQueryList( _rQueryList )
        ,m_xConnection( _rxConnection )
    {
    }

    virtual void    updateTableObjectList( bool _bAllowViews );
    virtual String  getSelectedName( String& _out_rAliasName ) const;
    virtual bool    isLeafSelected() const;
};

//------------------------------------------------------------------------------
void QueryListFacade::updateTableObjectList( bool /*_bAllowViews*/ )
{
    m_rQueryList.Clear();
    try
    {
        Image aQueryImage( ModuleRes( QUERY_TREE_ICON ) );
        Image aQueryImageHC( ModuleRes( QUERY_TREE_ICON_SCH ) );
        m_rQueryList.SetDefaultExpandedEntryBmp( aQueryImage, BMP_COLOR_NORMAL );
        m_rQueryList.SetDefaultCollapsedEntryBmp( aQueryImage, BMP_COLOR_NORMAL );
        m_rQueryList.SetDefaultExpandedEntryBmp( aQueryImageHC, BMP_COLOR_HIGHCONTRAST );
        m_rQueryList.SetDefaultCollapsedEntryBmp( aQueryImageHC, BMP_COLOR_HIGHCONTRAST );

        Reference< XQueriesSupplier > xSuppQueries( m_xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xQueries( xSuppQueries->getQueries(), UNO_QUERY_THROW );
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

//------------------------------------------------------------------------------
String QueryListFacade::getSelectedName( String& _out_rAliasName ) const
{
    String sSelected;
    SvLBoxEntry* pEntry = m_rQueryList.FirstSelected();
    if ( pEntry )
        sSelected = _out_rAliasName = m_rQueryList.GetEntryText( pEntry );
    return sSelected;
}

//------------------------------------------------------------------------------
bool QueryListFacade::isLeafSelected() const
{
    SvLBoxEntry* pEntry = m_rQueryList.FirstSelected();
    return pEntry && !m_rQueryList.GetModel()->HasChilds( pEntry );
}

//==============================================================================
//= OAddTableDlg
//==============================================================================
//------------------------------------------------------------------------------
OAddTableDlg::OAddTableDlg( Window* pParent, IAddTableDialogContext& _rContext )
             :ModelessDialog( pParent, ModuleRes(DLG_JOIN_TABADD) )
             ,m_aCaseTables( this, ResId( RB_CASE_TABLES ) )
             ,m_aCaseQueries( this, ResId( RB_CASE_QUERIES ) )
             ,m_aTableList( this, NULL, ResId( LB_TABLE_OR_QUERY ), sal_False )
             ,m_aQueryList( this, ResId( LB_TABLE_OR_QUERY ) )
             ,aAddButton( this, ResId( PB_ADDTABLE ) )
             ,aCloseButton( this, ResId( PB_CLOSE ) )
             ,aHelpButton( this, ResId( PB_HELP ) )
             ,m_rContext( _rContext )
{
    // der Close-Button hat schon einen Standard-Help-Text, den ich aber hier nicht haben moechte, also den Text ruecksetzen
    // und eine neue ID verteilen
    aCloseButton.SetHelpText(String());
    aCloseButton.SetHelpId(HID_JOINSH_ADDTAB_CLOSE);

    m_aTableList.SetHelpId( HID_JOINSH_ADDTAB_TABLELIST );
    m_aQueryList.SetHelpId( HID_JOINSH_ADDTAB_QUERYLIST );

    //////////////////////////////////////////////////////////////////////
    m_aCaseTables.SetClickHdl( LINK( this, OAddTableDlg, OnTypeSelected ) );
    m_aCaseQueries.SetClickHdl( LINK( this, OAddTableDlg, OnTypeSelected ) );
    aAddButton.SetClickHdl( LINK( this, OAddTableDlg, AddClickHdl ) );
    aCloseButton.SetClickHdl( LINK( this, OAddTableDlg, CloseClickHdl ) );
    m_aTableList.SetDoubleClickHdl( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    m_aTableList.SetSelectHdl( LINK( this, OAddTableDlg, TableListSelectHdl ) );
    m_aQueryList.SetDoubleClickHdl( LINK( this, OAddTableDlg, TableListDoubleClickHdl ) );
    m_aQueryList.SetSelectHdl( LINK( this, OAddTableDlg, TableListSelectHdl ) );

    //////////////////////////////////////////////////////////////////////
    m_aTableList.EnableInplaceEditing( FALSE );
    m_aTableList.SetWindowBits(WB_BORDER | WB_HASLINES |WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HASLINESATROOT | WB_SORT | WB_HSCROLL );
    m_aTableList.EnableCheckButton( NULL ); // do not show any buttons
    m_aTableList.SetSelectionMode( SINGLE_SELECTION );
    m_aTableList.notifyHiContrastChanged();

    //////////////////////////////////////////////////////////////////////
    m_aQueryList.EnableInplaceEditing( FALSE );
    m_aQueryList.SetSelectionMode( SINGLE_SELECTION );

    //////////////////////////////////////////////////////////////////////
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

//------------------------------------------------------------------------------
OAddTableDlg::~OAddTableDlg()
{
    m_rContext.onWindowClosing( this );
}

//------------------------------------------------------------------------------
void OAddTableDlg::impl_switchTo( ObjectList _eList )
{
    switch ( _eList )
    {
    case Tables:
        m_aTableList.Show( TRUE );  m_aCaseTables.Check( TRUE );
        m_aQueryList.Show( FALSE ); m_aCaseQueries.Check( FALSE );
        m_pCurrentList.reset( new TableListFacade( m_aTableList, m_rContext.getConnection() ) );
        m_aTableList.GrabFocus();
        break;

    case Queries:
        m_aTableList.Show( FALSE ); m_aCaseTables.Check( FALSE );
        m_aQueryList.Show( TRUE );  m_aCaseQueries.Check( TRUE );
        m_pCurrentList.reset( new QueryListFacade( m_aQueryList, m_rContext.getConnection() ) );
        m_aQueryList.GrabFocus();
        break;
    }
    m_pCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

//------------------------------------------------------------------------------
void OAddTableDlg::Update()
{
    if ( !m_pCurrentList.get() )
        impl_switchTo( Tables );
    else
        m_pCurrentList->updateTableObjectList( m_rContext.allowViews() );
}

//------------------------------------------------------------------------------
void OAddTableDlg::impl_addTable()
{
    if ( m_pCurrentList->isLeafSelected() )
    {
        String sSelectedName, sAliasName;
        sSelectedName = m_pCurrentList->getSelectedName( sAliasName );

        m_rContext.addTableWindow( sSelectedName, sAliasName );
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, AddClickHdl, Button*, /*pButton*/ )
{
    TableListDoubleClickHdl(NULL);
    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, TableListDoubleClickHdl, void*, /*EMPTY_ARG*/ )
{
    if ( impl_isAddAllowed() )
        impl_addTable();

    if ( !impl_isAddAllowed() )
        Close();

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, TableListSelectHdl, void*, /*EMPTY_ARG*/ )
{
    aAddButton.Enable( m_pCurrentList->isLeafSelected() );
    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, CloseClickHdl, Button*, /*pButton*/ )
{
    return Close();
}

//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, OnTypeSelected, void*, /*EMPTY_ARG*/ )
{
    if ( m_aCaseTables.IsChecked() )
        impl_switchTo( Tables );
    else
        impl_switchTo( Queries );
    return 0;
}

//------------------------------------------------------------------------------
BOOL OAddTableDlg::Close()
{
    m_rContext.onWindowClosing( this );
    return ModelessDialog::Close();
}

//------------------------------------------------------------------------------
bool OAddTableDlg::impl_isAddAllowed()
{
    return  m_rContext.allowAddition();
}

//------------------------------------------------------------------------------
String OAddTableDlg::getDialogTitleForContext( IAddTableDialogContext& _rContext )
{
    String sTitle;

    ::svt::OLocalResourceAccess aLocalRes( ModuleRes( DLG_JOIN_TABADD ), RSC_MODELESSDIALOG );
    if ( _rContext.allowQueries() )
        sTitle = String( ResId( STR_ADD_TABLE_OR_QUERY ) );
    else
        sTitle = String( ResId( STR_ADD_TABLES ) );

    return sTitle;
}

// -----------------------------------------------------------------------------

