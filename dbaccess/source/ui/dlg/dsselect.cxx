/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "dsselect.hxx"
#include "dsselect.hrc"
#include "dbu_dlg.hrc"
#include <vcl/msgbox.hxx>
#include "localresaccess.hxx"
#include <tools/rcid.h>

#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include "dbustrings.hrc"
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include "dsitems.hxx"
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>

//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::comphelper;
//==================================================================
ODatasourceSelectDialog::ODatasourceSelectDialog(Window* _pParent, const StringBag& _rDatasources, SfxItemSet* _pOutputSet)
     :ModalDialog(_pParent, ModuleRes(DLG_DATASOURCE_SELECTION))
     ,m_aDescription        (this, ModuleRes(FT_DESCRIPTION))
     ,m_aDatasource         (this, ModuleRes(LB_DATASOURCE))
     ,m_aOk                 (this, ModuleRes(PB_OK))
     ,m_aCancel             (this, ModuleRes(PB_CANCEL))
     ,m_aHelp               (this, ModuleRes(PB_HELP))
#ifdef HAVE_ODBC_ADMINISTRATION
     ,m_aManageDatasources  (this, ModuleRes(PB_MANAGE))
#endif
     ,m_pOutputSet(_pOutputSet)
{
    fillListBox(_rDatasources);
#ifdef HAVE_ODBC_ADMINISTRATION
    // allow ODBC datasource managenment
    m_aManageDatasources.Show();
    m_aManageDatasources.Enable();
    m_aManageDatasources.SetClickHdl(LINK(this,ODatasourceSelectDialog,ManageClickHdl));
#endif
    m_aDatasource.SetDoubleClickHdl(LINK(this,ODatasourceSelectDialog,ListDblClickHdl));
    FreeResource();
}

// -----------------------------------------------------------------------
ODatasourceSelectDialog::~ODatasourceSelectDialog()
{
}

// -----------------------------------------------------------------------
IMPL_LINK( ODatasourceSelectDialog, ListDblClickHdl, ListBox *, pListBox )
{
    if (pListBox->GetSelectEntryCount())
        EndDialog(RET_OK);
    return 0;
}

// -----------------------------------------------------------------------
sal_Bool ODatasourceSelectDialog::Close()
{
#ifdef HAVE_ODBC_ADMINISTRATION
    if ( m_pODBCManagement.get() && m_pODBCManagement->isRunning() )
        return sal_False;
#endif

    return ModalDialog::Close();
}

// -----------------------------------------------------------------------
#ifdef HAVE_ODBC_ADMINISTRATION
IMPL_LINK_NOARG(ODatasourceSelectDialog, ManageClickHdl)
{
    if ( !m_pODBCManagement.get() )
        m_pODBCManagement.reset( new OOdbcManagement( LINK( this, ODatasourceSelectDialog, ManageProcessFinished ) ) );

    if ( !m_pODBCManagement->manageDataSources_async() )
    {
        // TODO: error message
        m_aDatasource.GrabFocus();
        m_aManageDatasources.Disable();
        return 1L;
    }

    m_aDatasource.Disable();
    m_aOk.Disable();
    m_aCancel.Disable();
    m_aManageDatasources.Disable();

    OSL_POSTCOND( m_pODBCManagement->isRunning(), "ODatasourceSelectDialog::ManageClickHdl: success, but not running - you were *fast*!" );
    return 0L;
}

IMPL_LINK( ODatasourceSelectDialog, ManageProcessFinished, void*, /**/ )
{
    StringBag aOdbcDatasources;
    OOdbcEnumeration aEnumeration;
    aEnumeration.getDatasourceNames( aOdbcDatasources );
    fillListBox( aOdbcDatasources );

    m_aDatasource.Enable();
    m_aOk.Enable();
    m_aCancel.Enable();
    m_aManageDatasources.Enable();

    return 0L;
}

#endif
// -----------------------------------------------------------------------------
void ODatasourceSelectDialog::fillListBox(const StringBag& _rDatasources)
{
    ::rtl::OUString sSelected;
    if (m_aDatasource.GetEntryCount())
         sSelected = m_aDatasource.GetSelectEntry();
    m_aDatasource.Clear();
    // fill the list
    for (   ConstStringBagIterator aDS = _rDatasources.begin();
            aDS != _rDatasources.end();
            ++aDS
        )
    {
        m_aDatasource.InsertEntry( *aDS );
    }

    if (m_aDatasource.GetEntryCount())
    {
        if (!sSelected.isEmpty())
            m_aDatasource.SelectEntry(sSelected);
        else        // select the first entry
            m_aDatasource.SelectEntryPos(0);
    }
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
