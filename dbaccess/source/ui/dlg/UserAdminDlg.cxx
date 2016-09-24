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

#include "adminpages.hxx"
#include "DbAdminImpl.hxx"
#include "dbu_dlg.hrc"
#include "DriverSettings.hxx"
#include "dsitems.hxx"
#include "propertysetitem.hxx"
#include "UITools.hxx"
#include "UserAdmin.hxx"
#include "UserAdminDlg.hxx"

#include <comphelper/processfactory.hxx>
#include <connectivity/dbmetadata.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <vcl/stdtext.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;

    // OUserAdminDlg
    OUserAdminDlg::OUserAdminDlg(vcl::Window* _pParent
                                            , SfxItemSet* _pItems
                                            ,const Reference< XComponentContext >& _rxORB
                                            ,const css::uno::Any& _aDataSourceName
                                            ,const Reference< XConnection >& _xConnection)
        : SfxTabDialog(_pParent, "UserAdminDialog", "dbaccess/ui/useradmindialog.ui", _pItems)
        , m_pItemSet(_pItems)
        , m_xConnection(_xConnection)
        , m_bOwnConnection(!_xConnection.is())
    {
        m_pImpl.reset(new ODbDataSourceAdministrationHelper(_rxORB,_pParent,this));
        m_pImpl->setDataSourceOrName(_aDataSourceName);
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        m_pImpl->translateProperties(xDatasource, *_pItems);
        SetInputSet(_pItems);
        // propagate this set as our new input set and reset the example set
        delete m_pExampleSet;
        m_pExampleSet = new SfxItemSet(*GetInputSetImpl());

        AddTabPage("settings", OUserAdmin::Create, nullptr);

        // remove the reset button - it's meaning is much too ambiguous in this dialog
        RemoveResetButton();
    }

    OUserAdminDlg::~OUserAdminDlg()
    {
        disposeOnce();
    }

    void OUserAdminDlg::dispose()
    {
        if ( m_bOwnConnection )
        {
            try
            {
                ::comphelper::disposeComponent(m_xConnection);
            }
            catch(const Exception&)
            {
            }
        }

        SetInputSet(nullptr);
        DELETEZ(m_pExampleSet);
        SfxTabDialog::dispose();
    }

    short OUserAdminDlg::Execute()
    {
        try
        {
            ::dbtools::DatabaseMetaData aMetaData( createConnection().first );
            if ( !aMetaData.supportsUserAdministration( getORB() ) )
            {
                OUString sError(ModuleRes(STR_USERADMIN_NOT_AVAILABLE));
                throw SQLException(sError,nullptr,OUString("S1000") ,0,Any());
            }
        }
        catch(const SQLException&)
        {
            ::dbaui::showError( ::dbtools::SQLExceptionInfo( ::cppu::getCaughtException() ), GetParent(), getORB() );
            return RET_CANCEL;
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        short nRet = SfxTabDialog::Execute();
        if ( nRet == RET_OK )
            m_pImpl->saveChanges(*GetOutputItemSet());
        return nRet;
    }
    void OUserAdminDlg::PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage)
    {
        // register ourself as modified listener
        static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory( m_pImpl->getORB() );
        static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

        vcl::Window *pWin = GetViewWindow();
        if(pWin)
            pWin->Invalidate();

        SfxTabDialog::PageCreated(_nId, _rPage);
    }
    const SfxItemSet* OUserAdminDlg::getOutputSet() const
    {
        return m_pItemSet;
    }
    SfxItemSet* OUserAdminDlg::getWriteOutputSet()
    {
        return m_pItemSet;
    }
    ::std::pair< Reference<XConnection>,sal_Bool> OUserAdminDlg::createConnection()
    {
        if ( !m_xConnection.is() )
        {
            m_xConnection = m_pImpl->createConnection().first;
            m_bOwnConnection = m_xConnection.is();
        }
        return ::std::pair< Reference<XConnection>,sal_Bool> (m_xConnection,false);
    }
    Reference< XComponentContext > OUserAdminDlg::getORB() const
    {
        return m_pImpl->getORB();
    }
    Reference< XDriver > OUserAdminDlg::getDriver()
    {
        return m_pImpl->getDriver();
    }
    OUString OUserAdminDlg::getDatasourceType(const SfxItemSet& _rSet) const
    {
        return dbaui::ODbDataSourceAdministrationHelper::getDatasourceType(_rSet);
    }
    void OUserAdminDlg::clearPassword()
    {
        m_pImpl->clearPassword();
    }
    void OUserAdminDlg::setTitle(const OUString& _sTitle)
    {
        SetText(_sTitle);
    }
    void OUserAdminDlg::enableConfirmSettings( bool _bEnable )
    {
        (void)_bEnable;
    }
    void OUserAdminDlg::saveDatasource()
    {
        PrepareLeaveCurrentPage();
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
