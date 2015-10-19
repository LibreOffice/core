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
#include "dbu_dlg.hrc"
#include "dsitems.hxx"
#include "moduledbu.hxx"

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>

namespace dbaui
{

    // TextResetOperatorEventFilter
    class TextResetOperatorEventFilter : public ::svt::IWindowEventFilter
    {
    public:
        TextResetOperatorEventFilter()
        {
        }

        // IWindowEventFilter
        virtual bool payAttentionTo( const VclWindowEvent& _rEvent ) const override
        {
            return  ( _rEvent.GetId() == VCLEVENT_WINDOW_ENABLED )
                ||  ( _rEvent.GetId() == VCLEVENT_WINDOW_DISABLED )
                ||  ( _rEvent.GetId() == VCLEVENT_EDIT_MODIFY );
        }
    };

    // TextResetOperator
    class TextResetOperator :public ::svt::IWindowOperator
    {
    public:
        explicit TextResetOperator( const OUString& _rDisabledText )
            :m_sDisabledText( _rDisabledText )
        {
        }

        // IWindowOperator
        virtual void operateOn( const VclWindowEvent& _rTrigger, vcl::Window& _rOperateOn ) const override;

    private:
        const OUString    m_sDisabledText;
              OUString    m_sUserText;
    };

    void TextResetOperator::operateOn( const VclWindowEvent& _rTrigger, vcl::Window& _rOperateOn ) const
    {
        OSL_ENSURE( _rTrigger.GetWindow() == &_rOperateOn, "TextResetOperator::operateOn: you're misusing this implementation!" );

        switch ( _rTrigger.GetId() )
        {
        case 0:
            // initial call
            const_cast< TextResetOperator* >( this )->m_sUserText = _rTrigger.GetWindow()->GetText();
            break;

        case VCLEVENT_EDIT_MODIFY:
            if ( _rTrigger.GetWindow()->IsEnabled() )
                const_cast< TextResetOperator* >( this )->m_sUserText = _rTrigger.GetWindow()->GetText();
            break;

        case VCLEVENT_WINDOW_ENABLED:
            _rOperateOn.SetText( m_sUserText );
            break;

        case VCLEVENT_WINDOW_DISABLED:
            _rOperateOn.SetText( m_sDisabledText );
            break;

        default:
            OSL_FAIL( "TextResetOperator::operateOn: unexpected event ID!" );
            // all those IDs should have been filtered out by payAttentionTo
            break;
        }
    }

    // TextResetOperatorController
    class TextResetOperatorController_Base
    {
    protected:
        explicit TextResetOperatorController_Base( const OUString& _rDisabledText )
            :m_pEventFilter( new TextResetOperatorEventFilter )
            ,m_pOperator( new TextResetOperator( _rDisabledText ) )
        {
        }

        inline ::svt::PWindowEventFilter getEventFilter() const   { return m_pEventFilter; }
        inline ::svt::PWindowOperator    getOperator() const      { return m_pOperator; }

    private:
        ::svt::PWindowEventFilter   m_pEventFilter;
        ::svt::PWindowOperator      m_pOperator;
    };

    class TextResetOperatorController   :public TextResetOperatorController_Base
                                ,public ::svt::DialogController
    {
    public:
        TextResetOperatorController( vcl::Window& _rObservee, const OUString& _rDisabledText )
            :TextResetOperatorController_Base( _rDisabledText )
            ,::svt::DialogController( _rObservee, getEventFilter(), getOperator() )
        {
            addDependentWindow( _rObservee );
        }
    };

    // MySQLNativeSettings
    MySQLNativeSettings::MySQLNativeSettings( vcl::Window& _rParent, const Link<void*,void>& _rControlModificationLink )
        :TabPage( &_rParent, "MysqlNativeSettings", "dbaccess/ui/mysqlnativesettings.ui" ),
        m_aControlModificationLink(_rControlModificationLink)
    {
        get(m_pDatabaseNameLabel, "dbnamelabel");
        get(m_pDatabaseName, "dbname");
        get(m_pHostPortRadio, "hostport");
        get(m_pSocketRadio, "socketlabel");
        get(m_pNamedPipeRadio, "namedpipelabel");
        get(m_pHostNameLabel, "serverlabel");
        get(m_pHostName, "server");
        get(m_pPortLabel, "portlabel");
        get(m_pPort, "port");
        m_pPort->SetUseThousandSep(false);
        get(m_pDefaultPort, "defaultport");
        get(m_pSocket, "socket");
        get(m_pNamedPipe, "namedpipe");

        m_pDatabaseName->SetModifyHdl( LINK(this, MySQLNativeSettings, EditModifyHdl) );
        m_pHostName->SetModifyHdl( LINK(this, MySQLNativeSettings, EditModifyHdl) );
        m_pPort->SetModifyHdl( LINK(this, MySQLNativeSettings, EditModifyHdl) );
        m_pSocket->SetModifyHdl( LINK(this, MySQLNativeSettings, EditModifyHdl) );
        m_pNamedPipe->SetModifyHdl( LINK(this, MySQLNativeSettings, EditModifyHdl) );
        m_pSocketRadio->SetToggleHdl( LINK(this, MySQLNativeSettings, RadioToggleHdl) );
        m_pNamedPipeRadio->SetToggleHdl( LINK(this, MySQLNativeSettings, RadioToggleHdl) );

        m_aControlDependencies.enableOnRadioCheck( *m_pHostPortRadio, *m_pHostNameLabel, *m_pHostName, *m_pPortLabel, *m_pPort, *m_pDefaultPort );
        m_aControlDependencies.enableOnRadioCheck( *m_pSocketRadio, *m_pSocket );
        m_aControlDependencies.enableOnRadioCheck( *m_pNamedPipeRadio, *m_pNamedPipe );

        m_aControlDependencies.addController( ::svt::PDialogController(
            new TextResetOperatorController( *m_pHostName, OUString("localhost") )
        ) );

        // sockets are available on Unix systems only, named pipes only on Windows
#ifdef UNX
        m_pNamedPipeRadio->Hide();
        m_pNamedPipe->Hide();
#else
        m_pSocketRadio->Hide();
        m_pSocket->Hide();
#endif
    }

    IMPL_LINK_TYPED(MySQLNativeSettings, RadioToggleHdl, RadioButton&, rRadioButton, void)
    {
        m_aControlModificationLink.Call(&rRadioButton);
    }

    IMPL_LINK_TYPED(MySQLNativeSettings, EditModifyHdl, Edit&, rEdit, void)
    {
        m_aControlModificationLink.Call(&rEdit);
    }

    MySQLNativeSettings::~MySQLNativeSettings()
    {
        disposeOnce();
    }

    void MySQLNativeSettings::dispose()
    {
        m_pDatabaseNameLabel.clear();
        m_pDatabaseName.clear();
        m_pHostPortRadio.clear();
        m_pSocketRadio.clear();
        m_pNamedPipeRadio.clear();
        m_pHostNameLabel.clear();
        m_pHostName.clear();
        m_pPortLabel.clear();
        m_pPort.clear();
        m_pDefaultPort.clear();
        m_pSocket.clear();
        m_pNamedPipe.clear();
        TabPage::dispose();
    }

    void MySQLNativeSettings::fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new OSaveValueWrapper< Edit >( m_pDatabaseName ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( m_pHostName ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( m_pPort ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( m_pSocket ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( m_pNamedPipe ) );
    }

    void MySQLNativeSettings::fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new ODisableWrapper< FixedText >( m_pDatabaseNameLabel ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( m_pHostNameLabel ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( m_pPortLabel ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( m_pDefaultPort ) );
        _rControlList.push_back( new ODisableWrapper< RadioButton >( m_pSocketRadio ) );
        _rControlList.push_back( new ODisableWrapper< RadioButton >( m_pNamedPipeRadio ) );
    }

    bool MySQLNativeSettings::FillItemSet( SfxItemSet* _rSet )
    {
        bool bChangedSomething = false;

        OGenericAdministrationPage::fillString( *_rSet, m_pHostName,     DSID_CONN_HOSTNAME,    bChangedSomething );
        OGenericAdministrationPage::fillString( *_rSet, m_pDatabaseName, DSID_DATABASENAME,     bChangedSomething );
        OGenericAdministrationPage::fillInt32 ( *_rSet, m_pPort,         DSID_MYSQL_PORTNUMBER, bChangedSomething );
#ifdef UNX
        OGenericAdministrationPage::fillString( *_rSet, m_pSocket,       DSID_CONN_SOCKET,      bChangedSomething );
#else
        OGenericAdministrationPage::fillString( *_rSet, m_pNamedPipe,    DSID_NAMED_PIPE,       bChangedSomething );
#endif

        return bChangedSomething;
    }

    void MySQLNativeSettings::implInitControls(const SfxItemSet& _rSet )
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

        m_pDatabaseName->SetText( pDatabaseName->GetValue() );
        m_pDatabaseName->ClearModifyFlag();

        m_pHostName->SetText( pHostName->GetValue() );
        m_pHostName->ClearModifyFlag();

        m_pPort->SetValue( pPortNumber->GetValue() );
        m_pPort->ClearModifyFlag();

        m_pSocket->SetText( pSocket->GetValue() );
        m_pSocket->ClearModifyFlag();

        m_pNamedPipe->SetText( pNamedPipe->GetValue() );
        m_pNamedPipe->ClearModifyFlag();

        // if a socket (on Unix) or a pipe name (on Windows) is given, this is preferred over
        // the port
#ifdef UNX
        RadioButton& rSocketPipeRadio = *m_pSocketRadio;
        const SfxStringItem* pSocketPipeItem = pSocket;
#else
        RadioButton& rSocketPipeRadio = *m_pNamedPipeRadio;
        const SfxStringItem* pSocketPipeItem = pNamedPipe;
#endif
        OUString sSocketPipe( pSocketPipeItem->GetValue() );
        if ( !sSocketPipe.isEmpty() )
            rSocketPipeRadio.Check();
        else
            m_pHostPortRadio->Check();
    }

    bool MySQLNativeSettings::canAdvance() const
    {
        if ( m_pDatabaseName->GetText().isEmpty() )
            return false;

        if  (   m_pHostPortRadio->IsChecked()
            &&  (   ( m_pHostName->GetText().isEmpty() )
                ||  ( m_pPort->GetText().isEmpty() )
                )
            )
            return false;

#ifdef UNX
        if  (   ( m_pSocketRadio->IsChecked() )
            &&  ( m_pSocket->GetText().isEmpty() )
            )
#else
        if  (   ( m_pNamedPipeRadio->IsChecked() )
            &&  ( m_pNamedPipe->GetText().isEmpty() )
            )
#endif
            return false;

        return true;
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
