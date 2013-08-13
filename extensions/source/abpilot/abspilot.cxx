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

#include "abspilot.hxx"
#include "abpilot.hrc"
#include "abpresid.hrc"
#include "componentmodule.hxx"
#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <svtools/localresaccess.hxx>
#include "typeselectionpage.hxx"
#include "admininvokationpage.hxx"
#include "tableselectionpage.hxx"
#include <vcl/waitobj.hxx>
#include <vcl/msgbox.hxx>
#include "abpfinalpage.hxx"
#include "fieldmappingpage.hxx"
#include "fieldmappingimpl.hxx"

//.........................................................................
namespace abp
{
//.........................................................................

#define STATE_SELECT_ABTYPE         0
#define STATE_INVOKE_ADMIN_DIALOG   1
#define STATE_TABLE_SELECTION       2
#define STATE_MANUAL_FIELD_MAPPING  3
#define STATE_FINAL_CONFIRM         4

#define PATH_COMPLETE               1
#define PATH_NO_SETTINGS            2
#define PATH_NO_FIELDS              3
#define PATH_NO_SETTINGS_NO_FIELDS  4

    using namespace ::svt;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= OAddessBookSourcePilot
    //=====================================================================
    //---------------------------------------------------------------------
    OAddessBookSourcePilot::OAddessBookSourcePilot(Window* _pParent, const Reference< XComponentContext >& _rxORB)
        :OAddessBookSourcePilot_Base( _pParent, ModuleRes( RID_DLG_ADDRESSBOOKSOURCEPILOT ),
            WZB_HELP | WZB_FINISH | WZB_CANCEL | WZB_NEXT | WZB_PREVIOUS )
        ,m_xORB(_rxORB)
        ,m_aNewDataSource(_rxORB)
        ,m_eNewDataSourceType( AST_INVALID )
    {
        SetPageSizePixel(LogicToPixel(Size(WINDOW_SIZE_X, WINDOW_SIZE_Y), MAP_APPFONT));

        ShowButtonFixedLine(sal_True);

        declarePath( PATH_COMPLETE,
            STATE_SELECT_ABTYPE,
            STATE_INVOKE_ADMIN_DIALOG,
            STATE_TABLE_SELECTION,
            STATE_MANUAL_FIELD_MAPPING,
            STATE_FINAL_CONFIRM,
            WZS_INVALID_STATE
        );
        declarePath( PATH_NO_SETTINGS,
            STATE_SELECT_ABTYPE,
            STATE_TABLE_SELECTION,
            STATE_MANUAL_FIELD_MAPPING,
            STATE_FINAL_CONFIRM,
            WZS_INVALID_STATE
        );
        declarePath( PATH_NO_FIELDS,
            STATE_SELECT_ABTYPE,
            STATE_INVOKE_ADMIN_DIALOG,
            STATE_TABLE_SELECTION,
            STATE_FINAL_CONFIRM,
            WZS_INVALID_STATE
        );
        declarePath( PATH_NO_SETTINGS_NO_FIELDS,
            STATE_SELECT_ABTYPE,
            STATE_TABLE_SELECTION,
            STATE_FINAL_CONFIRM,
            WZS_INVALID_STATE
        );

        m_pPrevPage->SetHelpId(HID_ABSPILOT_PREVIOUS);
        m_pNextPage->SetHelpId(HID_ABSPILOT_NEXT);
        m_pCancel->SetHelpId(HID_ABSPILOT_CANCEL);
        m_pFinish->SetHelpId(HID_ABSPILOT_FINISH);
        m_pHelp->SetUniqueId(UID_ABSPILOT_HELP);

        m_pCancel->SetClickHdl( LINK( this, OAddessBookSourcePilot, OnCancelClicked) );

        // some initial settings
#ifdef UNX
#ifdef MACOSX
        m_aSettings.eType = AST_MACAB;
#else
// FIXME: if KDE use KAB instead
        m_aSettings.eType = AST_EVOLUTION;
#endif
#else
        m_aSettings.eType = AST_OTHER;
#endif
        m_aSettings.sDataSourceName = ModuleRes(RID_STR_DEFAULT_NAME).toString();
        m_aSettings.bRegisterDataSource = false;
        m_aSettings.bIgnoreNoTable = false;

        defaultButton(WZB_NEXT);
        enableButtons(WZB_FINISH, sal_False);
        ActivatePage();

        typeSelectionChanged( m_aSettings.eType );
    }

    OAddessBookSourcePilot::~OAddessBookSourcePilot()
    {
        FreeResource();
    }

    //---------------------------------------------------------------------
    OUString OAddessBookSourcePilot::getStateDisplayName( WizardState _nState ) const
    {
        sal_uInt16 nResId = 0;
        switch ( _nState )
        {
            case STATE_SELECT_ABTYPE:        nResId = STR_SELECT_ABTYPE; break;
            case STATE_INVOKE_ADMIN_DIALOG:  nResId = STR_INVOKE_ADMIN_DIALOG; break;
            case STATE_TABLE_SELECTION:      nResId = STR_TABLE_SELECTION; break;
            case STATE_MANUAL_FIELD_MAPPING: nResId = STR_MANUAL_FIELD_MAPPING; break;
            case STATE_FINAL_CONFIRM:        nResId = STR_FINAL_CONFIRM; break;
        }
        DBG_ASSERT( nResId, "OAddessBookSourcePilot::getStateDisplayName: don't know this state!" );

        OUString sDisplayName;
        if ( nResId )
        {
            svt::OLocalResourceAccess aAccess( ModuleRes( RID_DLG_ADDRESSBOOKSOURCEPILOT ), RSC_MODALDIALOG );
            sDisplayName = ModuleRes(nResId).toString();
        }

        return sDisplayName;
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::implCommitAll()
    {
        // in real, the data source already exists in the data source context
        // Thus, if the user changed the name, we have to rename the data source
        if ( m_aSettings.sDataSourceName != m_aNewDataSource.getName() )
            m_aNewDataSource.rename( m_aSettings.sDataSourceName );

        // 1. the data source
        m_aNewDataSource.store();

        // 2. check if we need to register the data source
        if ( m_aSettings.bRegisterDataSource )
            m_aNewDataSource.registerDataSource(m_aSettings.sRegisteredDataSourceName);

        // 3. write the data source / table names into the configuration
        addressconfig::writeTemplateAddressSource( getORB(), m_aSettings.bRegisterDataSource ? m_aSettings.sRegisteredDataSourceName : m_aSettings.sDataSourceName, m_aSettings.sSelectedTable );

        // 4. write the field mapping
        fieldmapping::writeTemplateAddressFieldMapping( getORB(), m_aSettings.aFieldMapping );
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::implCleanup()
    {
        if ( m_aNewDataSource.isValid() )
            m_aNewDataSource.remove();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( OAddessBookSourcePilot, OnCancelClicked, void*, /*NOTINTERESTEDIN*/ )
    {
        // do cleanups
        implCleanup();

        // reset the click hdl
        m_pCancel->SetClickHdl( Link() );
        // simulate the click again - this time, the default handling of the button will strike ....
        m_pCancel->Click();

        return 0L;
    }

    //---------------------------------------------------------------------
    sal_Bool OAddessBookSourcePilot::Close()
    {
        implCleanup();

        return OAddessBookSourcePilot_Base::Close();
    }

    //---------------------------------------------------------------------
    sal_Bool OAddessBookSourcePilot::onFinish()
    {
        if ( !OAddessBookSourcePilot_Base::onFinish() )
            return sal_False;

        implCommitAll();

        addressconfig::markPilotSuccess( getORB() );

        return sal_True;
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::enterState( WizardState _nState )
    {
        switch ( _nState )
        {
            case STATE_SELECT_ABTYPE:
                impl_updateRoadmap( static_cast< TypeSelectionPage* >( GetPage( STATE_SELECT_ABTYPE ) )->getSelectedType() );
                break;

            case STATE_FINAL_CONFIRM:
                if ( !needManualFieldMapping( ) )
                    implDoAutoFieldMapping();
                break;

            case STATE_TABLE_SELECTION:
                implDefaultTableName();
                break;
        }

        OAddessBookSourcePilot_Base::enterState(_nState);
    }

    //---------------------------------------------------------------------
    sal_Bool OAddessBookSourcePilot::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        if ( !OAddessBookSourcePilot_Base::prepareLeaveCurrentState( _eReason ) )
            return sal_False;

        if ( _eReason == eTravelBackward )
            return sal_True;

        sal_Bool bAllow = sal_True;

        switch ( getCurrentState() )
        {
        case STATE_SELECT_ABTYPE:
            implCreateDataSource();
            if ( needAdminInvokationPage() )
                break;
            // no break here

        case STATE_INVOKE_ADMIN_DIALOG:
            if ( !connectToDataSource( sal_False ) )
            {
                // connecting did not succeed -> do not allow proceeding
                bAllow = sal_False;
                break;
            }

            // ........................................................
            // now that we connected to the data source, check whether we need the "table selection" page
            const StringBag& aTables = m_aNewDataSource.getTableNames();

            if ( aTables.empty() )
            {
                if ( RET_YES != QueryBox( this, ModuleRes( ( getSettings().eType == AST_EVOLUTION_GROUPWISE ? RID_QRY_NO_EVO_GW : RID_QRY_NOTABLES ) ) ).Execute() )
                {
                    // cannot ask the user, or the user chose to use this data source, though there are no tables
                    bAllow = sal_False;
                    break;
                }

                m_aSettings.bIgnoreNoTable = true;
            }

            if ( aTables.size() == 1 )
                // remember the one and only table we have
                m_aSettings.sSelectedTable = *aTables.begin();

            break;
        }

        impl_updateRoadmap( m_aSettings.eType );
        return bAllow;
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::implDefaultTableName()
    {
        const StringBag& rTableNames = getDataSource().getTableNames();
        if ( rTableNames.end() != rTableNames.find( getSettings().sSelectedTable ) )
            // already a valid table selected
            return;

        const sal_Char* pGuess = NULL;
        switch ( getSettings().eType )
        {
            case AST_MORK               :
            case AST_THUNDERBIRD        : pGuess = "Personal Address book"; break;
            case AST_LDAP               : pGuess = "LDAP Directory"; break;
            case AST_EVOLUTION          :
            case AST_EVOLUTION_GROUPWISE:
            case AST_EVOLUTION_LDAP     : pGuess = "Personal"; break;
            default:
                OSL_FAIL( "OAddessBookSourcePilot::implDefaultTableName: unhandled case!" );
                return;
        }
        const OUString sGuess = OUString::createFromAscii( pGuess );
        if ( rTableNames.end() != rTableNames.find( sGuess ) )
            getSettings().sSelectedTable = sGuess;
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::implDoAutoFieldMapping()
    {
        DBG_ASSERT( !needManualFieldMapping( ), "OAddessBookSourcePilot::implDoAutoFieldMapping: invalid call!" );

        fieldmapping::defaultMapping( getORB(), m_aSettings.aFieldMapping );
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::implCreateDataSource()
    {
        if (m_aNewDataSource.isValid())
        {   // we already have a data source object
            if ( m_aSettings.eType == m_eNewDataSourceType )
                // and it already has the correct type
                return;

            // it has a wrong type -> remove it
            m_aNewDataSource.remove();
        }

        ODataSourceContext aContext( getORB() );
        aContext.disambiguate( m_aSettings.sDataSourceName );

        switch (m_aSettings.eType)
        {
            case AST_MORK:
                m_aNewDataSource = aContext.createNewMORK( m_aSettings.sDataSourceName );
                break;

            case AST_THUNDERBIRD:
                m_aNewDataSource = aContext.createNewThunderbird( m_aSettings.sDataSourceName );
                break;

            case AST_EVOLUTION:
                m_aNewDataSource = aContext.createNewEvolution( m_aSettings.sDataSourceName );
                break;

            case AST_EVOLUTION_GROUPWISE:
                m_aNewDataSource = aContext.createNewEvolutionGroupwise( m_aSettings.sDataSourceName );
                break;

            case AST_EVOLUTION_LDAP:
                m_aNewDataSource = aContext.createNewEvolutionLdap( m_aSettings.sDataSourceName );
                break;

            case AST_KAB:
                m_aNewDataSource = aContext.createNewKab( m_aSettings.sDataSourceName );
                break;

            case AST_MACAB:
                m_aNewDataSource = aContext.createNewMacab( m_aSettings.sDataSourceName );
                break;

            case AST_LDAP:
                m_aNewDataSource = aContext.createNewLDAP( m_aSettings.sDataSourceName );
                break;

            case AST_OUTLOOK:
                m_aNewDataSource = aContext.createNewOutlook( m_aSettings.sDataSourceName );
                break;

            case AST_OE:
                m_aNewDataSource = aContext.createNewOE( m_aSettings.sDataSourceName );
                break;

            case AST_OTHER:
                m_aNewDataSource = aContext.createNewDBase( m_aSettings.sDataSourceName );
                break;

            case AST_INVALID:
                OSL_FAIL( "OAddessBookSourcePilot::implCreateDataSource: illegal data source type!" );
                break;
        }
        m_eNewDataSourceType = m_aSettings.eType;
    }

    //---------------------------------------------------------------------
    sal_Bool OAddessBookSourcePilot::connectToDataSource( sal_Bool _bForceReConnect )
    {
        DBG_ASSERT( m_aNewDataSource.isValid(), "OAddessBookSourcePilot::implConnect: invalid current data source!" );

        WaitObject aWaitCursor( this );
        if ( _bForceReConnect && m_aNewDataSource.isConnected( ) )
            m_aNewDataSource.disconnect( );

        return m_aNewDataSource.connect( this );
    }

    //---------------------------------------------------------------------
    OWizardPage* OAddessBookSourcePilot::createPage(WizardState _nState)
    {
        switch (_nState)
        {
            case STATE_SELECT_ABTYPE:
                return new TypeSelectionPage( this );

            case STATE_INVOKE_ADMIN_DIALOG:
                return new AdminDialogInvokationPage( this );

            case STATE_TABLE_SELECTION:
                return new TableSelectionPage( this );

            case STATE_MANUAL_FIELD_MAPPING:
                return new FieldMappingPage( this );

            case STATE_FINAL_CONFIRM:
                return new FinalPage( this );

            default:
                OSL_FAIL("OAddessBookSourcePilot::createPage: invalid state!");
                return NULL;
        }
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::impl_updateRoadmap( AddressSourceType _eType )
    {
        bool bSettingsPage = needAdminInvokationPage( _eType );
        bool bTablesPage   = needTableSelection( _eType );
        bool bFieldsPage   = needManualFieldMapping( _eType );

        bool bConnected = m_aNewDataSource.isConnected();
        bool bCanSkipTables =
                (   m_aNewDataSource.hasTable( m_aSettings.sSelectedTable )
                ||  m_aSettings.bIgnoreNoTable
                );

        enableState( STATE_INVOKE_ADMIN_DIALOG, bSettingsPage );

        enableState( STATE_TABLE_SELECTION,
            bTablesPage &&  ( bConnected ? !bCanSkipTables : !bSettingsPage )
            // if we do not need a settings page, we connect upon "Next" on the first page
        );

        enableState( STATE_MANUAL_FIELD_MAPPING,
                bFieldsPage && bConnected && m_aNewDataSource.hasTable( m_aSettings.sSelectedTable )
        );

        enableState( STATE_FINAL_CONFIRM,
            bConnected && bCanSkipTables
        );
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::typeSelectionChanged( AddressSourceType _eType )
    {
        PathId nCurrentPathID( PATH_COMPLETE );
        bool bSettingsPage = needAdminInvokationPage( _eType );
        bool bFieldsPage = needManualFieldMapping( _eType );
        if ( !bSettingsPage )
            if ( !bFieldsPage )
                nCurrentPathID = PATH_NO_SETTINGS_NO_FIELDS;
            else
                nCurrentPathID = PATH_NO_SETTINGS;
        else
            if ( !bFieldsPage )
                nCurrentPathID = PATH_NO_FIELDS;
            else
                nCurrentPathID = PATH_COMPLETE;
        activatePath( nCurrentPathID, true );

        m_aNewDataSource.disconnect();
        m_aSettings.bIgnoreNoTable = false;
        impl_updateRoadmap( _eType );
    }

//.........................................................................
}   // namespace abp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
