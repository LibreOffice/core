/*************************************************************************
 *
 *  $RCSfile: adtabdlg.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:43:23 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#include "dbaccess_helpid.hrc"
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
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#include <algorithm>


// slot ids
using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace dbtools;

DBG_NAME(OAddTableDlg)
//------------------------------------------------------------------------------
OAddTableDlg::OAddTableDlg( Window* pParent,OJoinTableView* _pTableView)
             :ModelessDialog( pParent, ModuleRes(DLG_JOIN_TABADD) )
             ,aFTTable( this, ResId( FT_TABLE ) )
             ,aTableList( this,NULL, ResId( LB_TABLE ),sal_False,sal_False )
             ,aAddButton( this, ResId( PB_ADDTABLE ) )
             ,aCloseButton( this, ResId( PB_CLOSE ) )
             ,aHelpButton( this, ResId( PB_HELP ) )
             ,aFixedLineTable( this, ResId( FL_TABLE ) )
             ,aDefaultString( ResId( STR_DEFAULT ) )
             ,m_pTableView(_pTableView)
             ,m_bInitialized(sal_False)
{
    DBG_CTOR(OAddTableDlg,NULL);
    OSL_ENSURE(_pTableView,"Must be a valid pointer!");
    // der Close-Button hat schon einen Standard-Help-Text, den ich aber hier nicht haben moechte, also den Text ruecksetzen
    // und eine neue ID verteilen
    aCloseButton.SetHelpText(String());
    aCloseButton.SetHelpId(HID_JOINSH_ADDTAB_CLOSE);

    aTableList.SetHelpId(HID_JOINSH_ADDTAB_TABLELIST);

    //////////////////////////////////////////////////////////////////////
    // Handler setzen
    aAddButton.SetClickHdl( LINK(this,OAddTableDlg, AddClickHdl) );
    aCloseButton.SetClickHdl( LINK(this,OAddTableDlg, CloseClickHdl) );
    aTableList.SetDoubleClickHdl( LINK(this,OAddTableDlg, TableListDoubleClickHdl) );
    aTableList.SetSelectHdl( LINK(this,OAddTableDlg, TableListSelectHdl) );

    aTableList.EnableInplaceEditing( FALSE );
    aTableList.SetWindowBits(WB_BORDER | WB_HASLINES |WB_HASBUTTONS | WB_HASBUTTONSATROOT | WB_HASLINESATROOT | WB_SORT | WB_HSCROLL );
    aTableList.EnableCheckButton( NULL ); // do not show any buttons
    aTableList.SetSelectionMode( SINGLE_SELECTION );
    aTableList.notifyHiContrastChanged();

    FreeResource();
}

//------------------------------------------------------------------------------
OAddTableDlg::~OAddTableDlg()
{
    DBG_DTOR(OAddTableDlg,NULL);
}

//------------------------------------------------------------------------------
void OAddTableDlg::Update()
{
    if(!m_bInitialized)
    {
        UpdateTableList(m_pTableView->getDesignView()->getController()->isViewAllowed());
        m_bInitialized = sal_True;
    }
}

//------------------------------------------------------------------------------
void OAddTableDlg::AddTable()
{
    //////////////////////////////////////////////////////////////////
    // Tabelle hinzufuegen
    SvLBoxEntry* pEntry = aTableList.FirstSelected();
    if( pEntry && !aTableList.GetModel()->HasChilds(pEntry))
    {
        ::rtl::OUString aCatalog,aSchema,aTableName;
        SvLBoxEntry* pSchema = aTableList.GetParent(pEntry);
        if(pSchema && pSchema != aTableList.getAllObjectsEntry())
        {
            SvLBoxEntry* pCatalog = aTableList.GetParent(pSchema);
            if(pCatalog && pCatalog != aTableList.getAllObjectsEntry())
                aCatalog = aTableList.GetEntryText(pCatalog);
            aSchema = aTableList.GetEntryText(pSchema);
        }
        aTableName = aTableList.GetEntryText(pEntry);

        ::rtl::OUString aComposedName;
        try
        {
            Reference<XDatabaseMetaData> xMeta = m_pTableView->getDesignView()->getController()->getConnection()->getMetaData();
            // den Datenbank-Namen besorgen
            if (  !aCatalog.getLength()
                && aSchema.getLength()
                && xMeta->supportsCatalogsInDataManipulation()
                && !xMeta->supportsSchemasInDataManipulation() )
            {
                aCatalog = aSchema;
                aSchema = ::rtl::OUString();
            }



            ::dbtools::composeTableName(xMeta,
                                        aCatalog,
                                        aSchema,
                                        aTableName,
                                        aComposedName,
                                        sal_False,
                                        ::dbtools::eInDataManipulation);
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Exception catched!");
        }
        // aOrigTableName is used because AddTabWin would like to have this
        // und das Ganze dem Container uebergeben
        m_pTableView->AddTabWin( aComposedName,aTableName, TRUE );
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, AddClickHdl, Button*, pButton )
{
    TableListDoubleClickHdl(NULL);
    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, TableListDoubleClickHdl, ListBox *, EMPTY_ARG )
{
    if (IsAddAllowed())
        AddTable();

    if (!IsAddAllowed())
        Close();

    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, TableListSelectHdl, ListBox *, EMPTY_ARG )
{
    SvLBoxEntry* pEntry = aTableList.FirstSelected();
    aAddButton.Enable( pEntry && !aTableList.GetModel()->HasChilds(pEntry) );
    return 0;
}
//------------------------------------------------------------------------------
IMPL_LINK( OAddTableDlg, CloseClickHdl, Button*, pButton )
{
    return Close();
}

//------------------------------------------------------------------------------
BOOL OAddTableDlg::Close()
{
    m_pTableView->getDesignView()->getController()->InvalidateFeature(ID_BROWSER_ADDTABLE);
    m_pTableView->getDesignView()->getController()->getView()->GrabFocus();
    return ModelessDialog::Close();
}

//------------------------------------------------------------------------------
BOOL OAddTableDlg::IsAddAllowed()
{
    return  m_pTableView && m_pTableView->IsAddAllowed();
}

//------------------------------------------------------------------------------
void OAddTableDlg::UpdateTableList(BOOL bViewsAllowed)
{
    //////////////////////////////////////////////////////////////////////
    // Datenbank- und Tabellennamen setzen
    Reference< XTablesSupplier > xTableSupp(m_pTableView->getDesignView()->getController()->getConnection(),UNO_QUERY);
    Reference< XViewsSupplier > xViewSupp;
    Reference< XNameAccess > xTables, xViews;

    xTables = xTableSupp->getTables();

    // get the views supplier and the views
    Sequence< ::rtl::OUString> sTables,sViews;
    if (xTables.is())
        sTables = xTables->getElementNames();

    xViewSupp.set(xTableSupp,UNO_QUERY);
    if (xViewSupp.is())
    {
        xViews = xViewSupp->getViews();
        if (xViews.is())
            sViews = xViews->getElementNames();
    }
    // if no views are allowed remove the views also out the table name filter
    if ( !bViewsAllowed )
    {
        const ::rtl::OUString* pTableBegin  = sTables.getConstArray();
        const ::rtl::OUString* pTableEnd    = pTableBegin + sTables.getLength();
        ::std::vector< ::rtl::OUString > aTables(pTableBegin,pTableEnd);

        const ::rtl::OUString* pViewBegin = sViews.getConstArray();
        const ::rtl::OUString* pViewEnd   = pViewBegin + sViews.getLength();
        ::comphelper::TStringMixEqualFunctor aEqualFunctor;
        for(;pViewBegin != pViewEnd;++pViewBegin)
            aTables.erase(::std::remove_if(aTables.begin(),aTables.end(),::std::bind2nd(aEqualFunctor,*pViewBegin)));
        sTables = Sequence< ::rtl::OUString>(aTables.begin(),aTables.size());
        sViews = Sequence< ::rtl::OUString>();
    }

    aTableList.UpdateTableList(Reference< XConnection>(xTableSupp,UNO_QUERY)->getMetaData(),sTables,sViews);
    SvLBoxEntry* pEntry = aTableList.First();
    while( pEntry && aTableList.GetModel()->HasChilds(pEntry))
    {
        aTableList.Expand(pEntry);
        pEntry = aTableList.Next(pEntry);
    }
    if ( pEntry )
        aTableList.Select(pEntry);
}
// -----------------------------------------------------------------------------

