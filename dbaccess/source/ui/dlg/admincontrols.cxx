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

#include "admincontrols.hxx"
#include <dbu_dlg.hxx>
#include <dsitems.hxx>

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <vcl/svapp.hxx>

namespace dbaui
{

    // MySQLNativeSettings
    DBMySQLNativeSettings::DBMySQLNativeSettings(weld::Widget* pParent, const Link<void*,void>& rControlModificationLink)
        : m_xBuilder(Application::CreateBuilder(pParent, "dbaccess/ui/mysqlnativesettings.ui"))
        , m_xContainer(m_xBuilder->weld_widget("MysqlNativeSettings"))
        , m_xDatabaseNameLabel(m_xBuilder->weld_label("dbnamelabel"))
        , m_xDatabaseName(m_xBuilder->weld_entry("dbname"))
        , m_xHostPortRadio(m_xBuilder->weld_radio_button("hostport"))
        , m_xSocketRadio(m_xBuilder->weld_radio_button("socketlabel"))
        , m_xNamedPipeRadio(m_xBuilder->weld_radio_button("namedpipelabel"))
        , m_xHostNameLabel(m_xBuilder->weld_label("serverlabel"))
        , m_xHostName(m_xBuilder->weld_entry("server"))
        , m_xPortLabel(m_xBuilder->weld_label("portlabel"))
        , m_xPort(m_xBuilder->weld_spin_button("port"))
        , m_xDefaultPort(m_xBuilder->weld_label("defaultport"))
        , m_xSocket(m_xBuilder->weld_entry("socket"))
        , m_xNamedPipe(m_xBuilder->weld_entry("namedpipe"))
        , m_aControlModificationLink(rControlModificationLink)
    {
        m_xDatabaseName->connect_changed( LINK(this, DBMySQLNativeSettings, EditModifyHdl) );
        m_xHostName->connect_changed( LINK(this, DBMySQLNativeSettings, EditModifyHdl) );
        m_xPort->connect_value_changed( LINK(this, DBMySQLNativeSettings, SpinModifyHdl) );
        m_xSocket->connect_changed( LINK(this, DBMySQLNativeSettings, EditModifyHdl) );
        m_xNamedPipe->connect_changed( LINK(this, DBMySQLNativeSettings, EditModifyHdl) );
        m_xSocketRadio->connect_toggled( LINK(this, DBMySQLNativeSettings, RadioToggleHdl) );
        m_xNamedPipeRadio->connect_toggled( LINK(this, DBMySQLNativeSettings, RadioToggleHdl) );
        m_xHostPortRadio->connect_toggled( LINK(this, DBMySQLNativeSettings, RadioToggleHdl) );

        // sockets are available on Unix systems only, named pipes only on Windows
#ifdef UNX
        m_xNamedPipeRadio->hide();
        m_xNamedPipe->hide();
#else
        m_xSocketRadio->hide();
        m_xSocket->hide();
#endif
        m_xContainer->show();
    }

    IMPL_LINK(DBMySQLNativeSettings, RadioToggleHdl, weld::ToggleButton&, rRadioButton, void)
    {
        m_aControlModificationLink.Call(&rRadioButton);

        const bool bHostPortRadio = m_xHostPortRadio->get_active();
        m_xHostNameLabel->set_sensitive(bHostPortRadio);
        m_xHostName->set_sensitive(bHostPortRadio);
        m_xPortLabel->set_sensitive(bHostPortRadio);
        m_xPort->set_sensitive(bHostPortRadio);
        m_xDefaultPort->set_sensitive(bHostPortRadio);

        m_xSocket->set_sensitive(m_xSocketRadio->get_active());
        m_xNamedPipe->set_sensitive(m_xNamedPipeRadio->get_active());
    }

    IMPL_LINK(DBMySQLNativeSettings, EditModifyHdl, weld::Entry&, rEdit, void)
    {
        m_aControlModificationLink.Call(&rEdit);
    }

    IMPL_LINK(DBMySQLNativeSettings, SpinModifyHdl, weld::SpinButton&, rEdit, void)
    {
        m_aControlModificationLink.Call(&rEdit);
    }

    void DBMySQLNativeSettings::fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xDatabaseName.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xHostName.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xPort.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xSocket.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Entry>(m_xNamedPipe.get()));
    }

    void DBMySQLNativeSettings::fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList )
    {
        _rControlList.emplace_back( new ODisableWidgetWrapper<weld::Label>( m_xDatabaseNameLabel.get() ) );
        _rControlList.emplace_back( new ODisableWidgetWrapper<weld::Label>( m_xHostNameLabel.get() ) );
        _rControlList.emplace_back( new ODisableWidgetWrapper<weld::Label>( m_xPortLabel.get() ) );
        _rControlList.emplace_back( new ODisableWidgetWrapper<weld::Label>( m_xDefaultPort.get() ) );
        _rControlList.emplace_back( new ODisableWidgetWrapper<weld::RadioButton>( m_xSocketRadio.get() ) );
        _rControlList.emplace_back( new ODisableWidgetWrapper<weld::RadioButton>( m_xNamedPipeRadio.get() ) );
    }

    bool DBMySQLNativeSettings::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;

        OGenericAdministrationPage::fillString( *_rSet, m_xHostName.get(),     DSID_CONN_HOSTNAME,    bChangedSomething );
        OGenericAdministrationPage::fillString( *_rSet, m_xDatabaseName.get(), DSID_DATABASENAME,     bChangedSomething );
        OGenericAdministrationPage::fillInt32 ( *_rSet, m_xPort.get(),         DSID_MYSQL_PORTNUMBER, bChangedSomething );
#ifdef UNX
        OGenericAdministrationPage::fillString( *_rSet, m_xSocket.get(),       DSID_CONN_SOCKET,      bChangedSomething );
#else
        OGenericAdministrationPage::fillString( *_rSet, m_xNamedPipe.get(),    DSID_NAMED_PIPE,       bChangedSomething );
#endif

        return bChangedSomething;
    }

    void DBMySQLNativeSettings::implInitControls(const SfxItemSet& _rSet )
    {
        const SfxBoolItem* pInvalid = _rSet.GetItem<SfxBoolItem>(DSID_INVALID_SELECTION);
        bool bValid = !pInvalid || !pInvalid->GetValue();
        if ( !bValid )
            return;

        const SfxStringItem* pDatabaseName = _rSet.GetItem<SfxStringItem>(DSID_DATABASENAME);
        const SfxStringItem* pHostName = _rSet.GetItem<SfxStringItem>(DSID_CONN_HOSTNAME);
        const SfxInt32Item* pPortNumber = _rSet.GetItem<SfxInt32Item>(DSID_MYSQL_PORTNUMBER);
        const SfxStringItem* pSocket = _rSet.GetItem<SfxStringItem>(DSID_CONN_SOCKET);
        const SfxStringItem* pNamedPipe = _rSet.GetItem<SfxStringItem>(DSID_NAMED_PIPE);

        m_xDatabaseName->set_text( pDatabaseName->GetValue() );
        m_xDatabaseName->save_value();

        m_xHostName->set_text( pHostName->GetValue() );
        m_xHostName->save_value();

        m_xPort->set_value( pPortNumber->GetValue() );
        m_xPort->save_value();

        m_xSocket->set_text( pSocket->GetValue() );
        m_xSocket->save_value();

        m_xNamedPipe->set_text( pNamedPipe->GetValue() );
        m_xNamedPipe->save_value();

        // if a socket (on Unix) or a pipe name (on Windows) is given, this is preferred over
        // the port
#ifdef UNX
        weld::RadioButton& rSocketPipeRadio = *m_xSocketRadio;
        const SfxStringItem* pSocketPipeItem = pSocket;
#else
        weld::RadioButton& rSocketPipeRadio = *m_xNamedPipeRadio;
        const SfxStringItem* pSocketPipeItem = pNamedPipe;
#endif
        const OUString& rSocketPipe( pSocketPipeItem->GetValue() );
        if (!rSocketPipe.isEmpty())
            rSocketPipeRadio.set_active(true);
        else
            m_xHostPortRadio->set_active(true);
    }

    bool DBMySQLNativeSettings::canAdvance() const
    {
        if (m_xDatabaseName->get_text().isEmpty())
            return false;

        if  (   m_xHostPortRadio->get_active()
            &&  (   ( m_xHostName->get_text().isEmpty() )
                ||  ( m_xPort->get_text().isEmpty() )
                )
            )
            return false;

#ifdef UNX
        if  (   ( m_xSocketRadio->get_active() )
            &&  ( m_xSocket->get_text().isEmpty() )
            )
#else
        if  (   ( m_xNamedPipeRadio->get_active() )
            &&  ( m_xNamedPipe->get_text().isEmpty() )
            )
#endif
            return false;

        return true;
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
