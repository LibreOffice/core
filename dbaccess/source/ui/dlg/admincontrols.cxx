/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include "admincontrols.hxx"
#include "admincontrols.hrc"
#include "dbu_dlg.hrc"
#include "dsitems.hxx"
#include "moduledbu.hxx"

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= TextResetOperatorEventFilter
    //====================================================================
    class TextResetOperatorEventFilter : public ::svt::IWindowEventFilter
    {
    public:
        TextResetOperatorEventFilter()
        {
        }

        // IWindowEventFilter
        virtual bool payAttentionTo( const VclWindowEvent& _rEvent ) const
        {
            return  ( _rEvent.GetId() == VCLEVENT_WINDOW_ENABLED )
                ||  ( _rEvent.GetId() == VCLEVENT_WINDOW_DISABLED )
                ||  ( _rEvent.GetId() == VCLEVENT_EDIT_MODIFY );
        }
    };

    //====================================================================
    //= TextResetOperator
    //====================================================================
    class TextResetOperator :public ::svt::IWindowOperator
    {
    public:
        TextResetOperator( const String& _rDisabledText )
            :m_sDisabledText( _rDisabledText )
        {
        }

        // IWindowOperator
        virtual void operateOn( const VclWindowEvent& _rTrigger, Window& _rOperateOn ) const;

    private:
        const String    m_sDisabledText;
              String    m_sUserText;
              sal_Bool      m_bLastKnownEnabledState;
    };

    //--------------------------------------------------------------------
    void TextResetOperator::operateOn( const VclWindowEvent& _rTrigger, Window& _rOperateOn ) const
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

    //====================================================================
    //= TextResetOperatorController
    //====================================================================
    class TextResetOperatorController_Base
    {
    protected:
        TextResetOperatorController_Base( const String& _rDisabledText )
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
        TextResetOperatorController( Window& _rObservee, const String& _rDisabledText )
            :TextResetOperatorController_Base( _rDisabledText )
            ,::svt::DialogController( _rObservee, getEventFilter(), getOperator() )
        {
            addDependentWindow( _rObservee );
        }
    };

    //====================================================================
    //= MySQLNativeSettings
    //====================================================================
    //--------------------------------------------------------------------
    MySQLNativeSettings::MySQLNativeSettings( Window& _rParent, const Link& _rControlModificationLink )
        :Control( &_rParent, ModuleRes( RID_MYSQL_NATIVE_SETTINGS ).SetAutoRelease( sal_False ) )
        ,m_aDatabaseNameLabel   ( this, ModuleRes( FT_MYSQL_DATABASE_NAME ) )
        ,m_aDatabaseName        ( this, ModuleRes( ED_MYSQL_DATABASE_NAME ) )
        ,m_aHostPortRadio       ( this, ModuleRes( RB_MYSQL_HOST_PORT ) )
        ,m_aSocketRadio         ( this, ModuleRes( RB_MYSQL_SOCKET ) )
        ,m_aNamedPipeRadio      ( this, ModuleRes( RB_MYSQL_NAMED_PIPE ) )
        ,m_aHostNameLabel       ( this, ModuleRes( FT_COMMON_HOST_NAME ) )
        ,m_aHostName            ( this, ModuleRes( ED_COMMON_HOST_NAME ) )
        ,m_aPortLabel           ( this, ModuleRes( FT_COMMON_PORT ) )
        ,m_aPort                ( this, ModuleRes( NF_COMMON_PORT ) )
        ,m_aDefaultPort         ( this, ModuleRes( FT_COMMON_PORT_DEFAULT ) )
        ,m_aSocket              ( this, ModuleRes( ED_MYSQL_SOCKET ) )
        ,m_aNamedPipe           ( this, ModuleRes( ED_MYSQL_NAMED_PIPE ) )
    {
        FreeResource();

        m_aDatabaseName.SetModifyHdl( _rControlModificationLink );
        m_aHostName.SetModifyHdl( _rControlModificationLink );
        m_aPort.SetModifyHdl( _rControlModificationLink );
        m_aNamedPipe.SetModifyHdl( _rControlModificationLink );
        m_aSocketRadio.SetToggleHdl( _rControlModificationLink );
        m_aNamedPipeRadio.SetToggleHdl( _rControlModificationLink );

        m_aControlDependencies.enableOnRadioCheck( m_aHostPortRadio, m_aHostNameLabel, m_aHostName, m_aPortLabel, m_aPort, m_aDefaultPort );
        m_aControlDependencies.enableOnRadioCheck( m_aSocketRadio, m_aSocket );
        m_aControlDependencies.enableOnRadioCheck( m_aNamedPipeRadio, m_aNamedPipe );

        m_aControlDependencies.addController( ::svt::PDialogController(
            new TextResetOperatorController( m_aHostName, String::CreateFromAscii( "localhost" ) )
        ) );

        // sockets are available on Unix systems only, named pipes only on Windows
#ifdef UNX
        m_aNamedPipeRadio.Hide();
        m_aNamedPipe.Hide();
#else
        m_aSocketRadio.Hide();
        m_aSocket.Hide();
#endif
    }

    //--------------------------------------------------------------------
    MySQLNativeSettings::~MySQLNativeSettings()
    {
    }

    //--------------------------------------------------------------------
    void MySQLNativeSettings::fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new OSaveValueWrapper< Edit >( &m_aDatabaseName ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( &m_aHostName ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( &m_aPort ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( &m_aSocket ) );
        _rControlList.push_back( new OSaveValueWrapper< Edit >( &m_aNamedPipe ) );
    }

    //--------------------------------------------------------------------
    void MySQLNativeSettings::fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList )
    {
        _rControlList.push_back( new ODisableWrapper< FixedText >( &m_aDatabaseNameLabel ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( &m_aHostNameLabel ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( &m_aPortLabel ) );
        _rControlList.push_back( new ODisableWrapper< FixedText >( &m_aDefaultPort ) );
        _rControlList.push_back( new ODisableWrapper< RadioButton >( &m_aSocketRadio ) );
        _rControlList.push_back( new ODisableWrapper< RadioButton >( &m_aNamedPipeRadio ) );
    }

    //--------------------------------------------------------------------
    sal_Bool MySQLNativeSettings::FillItemSet( SfxItemSet& _rSet )
    {
        sal_Bool bChangedSomething = sal_False;

        OGenericAdministrationPage::fillString( _rSet, &m_aHostName,     DSID_CONN_HOSTNAME,    bChangedSomething );
        OGenericAdministrationPage::fillString( _rSet, &m_aDatabaseName, DSID_DATABASENAME,     bChangedSomething );
        OGenericAdministrationPage::fillInt32 ( _rSet, &m_aPort,         DSID_MYSQL_PORTNUMBER, bChangedSomething );
#ifdef UNX
        OGenericAdministrationPage::fillString( _rSet, &m_aSocket,       DSID_CONN_SOCKET,      bChangedSomething );
#else
        OGenericAdministrationPage::fillString( _rSet, &m_aNamedPipe,    DSID_NAMED_PIPE,       bChangedSomething );
#endif

        return bChangedSomething;
    }

    //--------------------------------------------------------------------
    void MySQLNativeSettings::implInitControls(const SfxItemSet& _rSet )
    {
        SFX_ITEMSET_GET( _rSet, pInvalid, SfxBoolItem, DSID_INVALID_SELECTION, sal_True );
        bool bValid = !pInvalid || !pInvalid->GetValue();
        if ( !bValid )
            return;

        SFX_ITEMSET_GET( _rSet, pDatabaseName,  SfxStringItem,  DSID_DATABASENAME,      sal_True );
        SFX_ITEMSET_GET( _rSet, pHostName,      SfxStringItem,  DSID_CONN_HOSTNAME,     sal_True );
        SFX_ITEMSET_GET( _rSet, pPortNumber,    SfxInt32Item,   DSID_MYSQL_PORTNUMBER,  sal_True );
        SFX_ITEMSET_GET( _rSet, pSocket,        SfxStringItem,  DSID_CONN_SOCKET,       sal_True );
        SFX_ITEMSET_GET( _rSet, pNamedPipe,     SfxStringItem,  DSID_NAMED_PIPE,       sal_True );

        m_aDatabaseName.SetText( pDatabaseName->GetValue() );
        m_aDatabaseName.ClearModifyFlag();

        m_aHostName.SetText( pHostName->GetValue() );
        m_aHostName.ClearModifyFlag();

        m_aPort.SetValue( pPortNumber->GetValue() );
        m_aPort.ClearModifyFlag();

        m_aSocket.SetText( pSocket->GetValue() );
        m_aSocket.ClearModifyFlag();

        m_aNamedPipe.SetText( pNamedPipe->GetValue() );
        m_aNamedPipe.ClearModifyFlag();

        // if a socket (on Unix) or a pipe name (on Windows) is given, this is preferred over
        // the port
#ifdef UNX
        RadioButton& rSocketPipeRadio = m_aSocketRadio;
        const SfxStringItem* pSocketPipeItem = pSocket;
#else
        RadioButton& rSocketPipeRadio = m_aNamedPipeRadio;
        const SfxStringItem* pSocketPipeItem = pNamedPipe;
#endif
        String sSocketPipe( pSocketPipeItem->GetValue() );
        if ( sSocketPipe.Len() > 0 )
            rSocketPipeRadio.Check();
        else
            m_aHostPortRadio.Check();
    }

    //--------------------------------------------------------------------
    bool MySQLNativeSettings::canAdvance() const
    {
        if ( m_aDatabaseName.GetText().Len() == 0 )
            return false;

        if  (   m_aHostPortRadio.IsChecked()
            &&  (   ( m_aHostName.GetText().Len() == 0 )
                ||  ( m_aPort.GetText().Len() == 0 )
                )
            )
            return false;

#ifdef UNX
        if  (   ( m_aSocketRadio.IsChecked() )
            &&  ( m_aSocket.GetText().Len() == 0 )
            )
#else
        if  (   ( m_aNamedPipeRadio.IsChecked() )
            &&  ( m_aNamedPipe.GetText().Len() == 0 )
            )
#endif
            return false;

        return true;
    }

//........................................................................
} // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
