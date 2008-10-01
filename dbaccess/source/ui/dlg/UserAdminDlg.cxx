/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UserAdminDlg.cxx,v $
 * $Revision: 1.14.68.1 $
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

#ifndef DBAUI_USERADMINDLG_HXX
#include "UserAdminDlg.hxx"
#endif
#ifndef DBAUI_USERADMINDLG_HRC
#include "UserAdminDlg.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef _DBAUI_DBADMINIMPL_HXX_
#include "DbAdminImpl.hxx"
#endif
#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef DBAUI_USERADMIN_HXX
#include "UserAdmin.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XUSERSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XUsersSupplier.hpp>
#endif
//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;

    //========================================================================
    //= OUserAdminDlg
DBG_NAME(OUserAdminDlg)
//========================================================================
    OUserAdminDlg::OUserAdminDlg(Window* _pParent
                                            , SfxItemSet* _pItems
                                            ,const Reference< XMultiServiceFactory >& _rxORB
                                            ,const ::com::sun::star::uno::Any& _aDataSourceName
                                            ,const Reference< XConnection >& _xConnection)
        :SfxTabDialog(_pParent, ModuleRes(DLG_DATABASE_USERADMIN), _pItems)
        ,m_pItemSet(_pItems)
        ,m_xConnection(_xConnection)
        ,m_bOwnConnection(!_xConnection.is())
    {
        DBG_CTOR(OUserAdminDlg,NULL);

        m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,_pParent,this));
        m_pImpl->setDataSourceOrName(_aDataSourceName);
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        m_pImpl->translateProperties(xDatasource, *_pItems);
        SetInputSet(_pItems);
        // propagate this set as our new input set and reset the example set
        delete pExampleSet;
        pExampleSet = new SfxItemSet(*GetInputSetImpl());

        AddTabPage(TAB_PAGE_USERADMIN, String(ModuleRes(STR_PAGETITLE_USERADMIN)), OUserAdmin::Create,0, sal_False, 1);

        // remove the reset button - it's meaning is much too ambiguous in this dialog
        RemoveResetButton();
        FreeResource();
    }

    // -----------------------------------------------------------------------
    OUserAdminDlg::~OUserAdminDlg()
    {
        if ( m_bOwnConnection )
            try
            {
                ::comphelper::disposeComponent(m_xConnection);
            }
            catch(Exception){}

        SetInputSet(NULL);
        DELETEZ(pExampleSet);

        DBG_DTOR(OUserAdminDlg,NULL);
    }
    // -----------------------------------------------------------------------
    short OUserAdminDlg::Execute()
    {
        try
        {
            Reference<XUsersSupplier> xUsersSup(m_xConnection,UNO_QUERY);
            sal_Bool bError = sal_False;
            if ( !xUsersSup.is() )
            {
                Reference< XDataDefinitionSupplier > xDriver(getDriver(),UNO_QUERY);
                bError = !xDriver.is();
                if ( !bError )
                {
                    m_xConnection = createConnection().first;
                    bError = !m_xConnection.is();

                    if ( !bError )
                    {
                        // now set the tables supplier at the table control
                        xUsersSup.set(xDriver->getDataDefinitionByConnection(m_xConnection),UNO_QUERY);
                    }
                }
            }
            bError = ! ( xUsersSup.is() && xUsersSup->getUsers().is());

            if ( bError )
            {
                String sError(ModuleRes(STR_USERADMIN_NOT_AVAILABLE));
                throw SQLException(sError,NULL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")) ,0,Any());
            }
        }
        catch(SQLException& e)
        {
            ::dbaui::showError(::dbtools::SQLExceptionInfo(e),GetParent(),getORB());
            return RET_CANCEL;
        }
        short nRet = SfxTabDialog::Execute();
        if ( nRet == RET_OK )
            m_pImpl->saveChanges(*GetOutputItemSet());
        return nRet;
    }
    //-------------------------------------------------------------------------
    void OUserAdminDlg::PageCreated(USHORT _nId, SfxTabPage& _rPage)
    {
        // register ourself as modified listener
        static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory(m_pImpl->getORB());
        static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

        AdjustLayout();
        Window *pWin = GetViewWindow();
        if(pWin)
            pWin->Invalidate();

        SfxTabDialog::PageCreated(_nId, _rPage);
    }
    // -----------------------------------------------------------------------------
    const SfxItemSet* OUserAdminDlg::getOutputSet() const
    {
        return m_pItemSet;
    }
    // -----------------------------------------------------------------------------
    SfxItemSet* OUserAdminDlg::getWriteOutputSet()
    {
        return m_pItemSet;
    }
    // -----------------------------------------------------------------------------
    ::std::pair< Reference<XConnection>,sal_Bool> OUserAdminDlg::createConnection()
    {
        if ( !m_xConnection.is() )
        {
            m_xConnection = m_pImpl->createConnection().first;
            m_bOwnConnection = m_xConnection.is();
        }
        return ::std::pair< Reference<XConnection>,sal_Bool> (m_xConnection,sal_False);
    }
    // -----------------------------------------------------------------------------
    Reference< XMultiServiceFactory > OUserAdminDlg::getORB() const
    {
        return m_pImpl->getORB();
    }
    // -----------------------------------------------------------------------------
    Reference< XDriver > OUserAdminDlg::getDriver()
    {
        return m_pImpl->getDriver();
    }
    // -----------------------------------------------------------------------------
    ::dbaccess::DATASOURCE_TYPE OUserAdminDlg::getDatasourceType(const SfxItemSet& _rSet) const
    {
        return m_pImpl->getDatasourceType(_rSet);
    }
    // -----------------------------------------------------------------------------
    void OUserAdminDlg::clearPassword()
    {
        m_pImpl->clearPassword();
    }
    // -----------------------------------------------------------------------------
    void OUserAdminDlg::setTitle(const ::rtl::OUString& _sTitle)
    {
        SetText(_sTitle);
    }
    //-------------------------------------------------------------------------
    void OUserAdminDlg::enableConfirmSettings( bool _bEnable )
    {
        (void)_bEnable;
    }
    //-------------------------------------------------------------------------
    sal_Bool OUserAdminDlg::saveDatasource()
    {
        return PrepareLeaveCurrentPage();
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................
