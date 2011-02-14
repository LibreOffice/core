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
#include "precompiled_dbaccess.hxx"

#ifndef _DBAUI_DSSELECT_HXX_
#include "dsselect.hxx"
#endif
#ifndef _DBAUI_DSSELECT_HRC_
#include "dsselect.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif

#ifndef _COM_SUN_STAR_SDBCX_XCREATECATALOG_HPP_
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _SFXSTRITEM_HXX
#include <svl/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svl/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svl/eitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svl/itemset.hxx>
#endif

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
ODatasourceSelectDialog::ODatasourceSelectDialog(Window* _pParent, const StringBag& _rDatasources, bool _bAdabas,SfxItemSet* _pOutputSet)
     :ModalDialog(_pParent, ModuleRes(DLG_DATASOURCE_SELECTION))
     ,m_aDescription        (this, ModuleRes(FT_DESCRIPTION))
     ,m_aDatasource         (this, ModuleRes(LB_DATASOURCE))
     ,m_aOk                 (this, ModuleRes(PB_OK))
     ,m_aCancel             (this, ModuleRes(PB_CANCEL))
     ,m_aHelp               (this, ModuleRes(PB_HELP))
#ifdef HAVE_ODBC_ADMINISTRATION
     ,m_aManageDatasources  (this, ModuleRes(PB_MANAGE))
#endif
     ,m_aCreateAdabasDB     (this, ModuleRes(PB_CREATE))
     ,m_pOutputSet(_pOutputSet)
{
    if ( _bAdabas )
    {   // set a new title (indicating that we're browsing local data sources only)
        SetText(ModuleRes(STR_LOCAL_DATASOURCES));
        m_aDescription.SetText(ModuleRes(STR_DESCRIPTION2));

        m_aCreateAdabasDB.Show();
        m_aCreateAdabasDB.SetClickHdl(LINK(this,ODatasourceSelectDialog,CreateDBClickHdl));

        // resize the dialog a little bit, 'cause Adabas data source names are usually somewhat shorter
        // than ODBC ones are

        // shrink the listbox
        Size aOldSize = m_aDatasource.GetSizePixel();
        Size aNewSize(3 * aOldSize.Width() / 4, aOldSize.Height());
        m_aDatasource.SetSizePixel(aNewSize);

        sal_Int32 nLostPixels = aOldSize.Width() - aNewSize.Width();

        // shrink the fixed text
        aOldSize = m_aDescription.GetSizePixel();
        m_aDescription.SetSizePixel(Size(aOldSize.Width() - nLostPixels, aOldSize.Height()));

        // move the buttons
        PushButton* pButtons[] = { &m_aOk, &m_aCancel, &m_aHelp ,&m_aCreateAdabasDB};
        for (size_t i=0; i<sizeof(pButtons)/sizeof(pButtons[0]); ++i)
        {
            Point aOldPos = pButtons[i]->GetPosPixel();
            pButtons[i]->SetPosPixel(Point(aOldPos.X() - nLostPixels, aOldPos.Y()));
        }

        // resize the dialog itself
        aOldSize = GetSizePixel();
        SetSizePixel(Size(aOldSize.Width() - nLostPixels, aOldSize.Height()));
    }

    fillListBox(_rDatasources);
#ifdef HAVE_ODBC_ADMINISTRATION
    // allow ODBC datasource managenment
    if (  !_bAdabas )
    {
        m_aManageDatasources.Show();
        m_aManageDatasources.Enable();
        m_aManageDatasources.SetClickHdl(LINK(this,ODatasourceSelectDialog,ManageClickHdl));
    }
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
IMPL_LINK( ODatasourceSelectDialog, CreateDBClickHdl, PushButton*, /*pButton*/ )
{
    try
    {
        OSL_ENSURE(m_pOutputSet,"No itemset given!");
        Reference< ::com::sun::star::lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
        Reference<XCreateCatalog> xCatalog(xORB->createInstance(SERVICE_EXTENDED_ADABAS_DRIVER),UNO_QUERY);
        if ( xCatalog.is() && m_pOutputSet )
        {
            Sequence< Any > aArgs(2);
            aArgs[0] <<= PropertyValue(::rtl::OUString::createFromAscii("CreateCatalog"), 0,makeAny(xCatalog) , PropertyState_DIRECT_VALUE);
            aArgs[1] <<= PropertyValue(PROPERTY_PARENTWINDOW, 0, makeAny(VCLUnoHelper::GetInterface(this)), PropertyState_DIRECT_VALUE);

            Reference< XExecutableDialog > xDialog(
                xORB->createInstanceWithArguments(SERVICE_SDB_ADABASCREATIONDIALOG, aArgs), UNO_QUERY);
            if (!xDialog.is())
            {
                //  ShowServiceNotAvailableError(this, String(SERVICE_SDB_ADABASCREATIONDIALOG), sal_True);
                return 0L;
            }

            if ( xDialog->execute() == RET_OK )
            {
                Reference<XPropertySet> xProp(xDialog,UNO_QUERY);
                if(xProp.is())
                {
                    Reference<XPropertySetInfo> xPropInfo(xProp->getPropertySetInfo());
                    if(xPropInfo->hasPropertyByName(PROPERTY_DATABASENAME))
                    {
                        String sDatabaseName;
                        sDatabaseName = String(::comphelper::getString(xProp->getPropertyValue(PROPERTY_DATABASENAME)));
                        m_aDatasource.SelectEntryPos(m_aDatasource.InsertEntry( sDatabaseName ));

                    }
                    if ( xPropInfo->hasPropertyByName(PROPERTY_CONTROLUSER) )
                        m_pOutputSet->Put(SfxStringItem(DSID_CONN_CTRLUSER, ::comphelper::getString(xProp->getPropertyValue(PROPERTY_CONTROLUSER))));
                    if ( xPropInfo->hasPropertyByName(PROPERTY_CONTROLPASSWORD) )
                        m_pOutputSet->Put(SfxStringItem(DSID_CONN_CTRLPWD, ::comphelper::getString(xProp->getPropertyValue(PROPERTY_CONTROLPASSWORD))));
                    if ( xPropInfo->hasPropertyByName(PROPERTY_USER) )
                        m_pOutputSet->Put(SfxStringItem(DSID_USER, ::comphelper::getString(xProp->getPropertyValue(PROPERTY_USER))));
                    if ( xPropInfo->hasPropertyByName(PROPERTY_PASSWORD) )
                    {
                        m_pOutputSet->Put(SfxStringItem(DSID_PASSWORD, ::comphelper::getString(xProp->getPropertyValue(PROPERTY_PASSWORD))));
                        m_pOutputSet->Put(SfxBoolItem(DSID_PASSWORDREQUIRED, sal_True));
                    }
                    if ( xPropInfo->hasPropertyByName(PROPERTY_CACHESIZE) )
                        m_pOutputSet->Put(SfxInt32Item(DSID_CONN_CACHESIZE, ::comphelper::getINT32(xProp->getPropertyValue(PROPERTY_CACHESIZE))));
                }
            }
        }
    }
    catch(Exception&)
    {
    }
    return 0L;
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
IMPL_LINK( ODatasourceSelectDialog, ManageClickHdl, PushButton*, EMPTYARG )
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
        if (sSelected.getLength())
            m_aDatasource.SelectEntry(sSelected);
        else        // select the first entry
            m_aDatasource.SelectEntryPos(0);
    }
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

