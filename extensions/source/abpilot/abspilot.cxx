/*************************************************************************
 *
 *  $RCSfile: abspilot.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:34:56 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_ABSPILOT_HXX
#include "abspilot.hxx"
#endif
#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include "extensio.hrc"
#endif
#ifndef EXTENSIONS_ABPRESID_HRC
#include "abpresid.hrc"
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "componentmodule.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif
#ifndef EXTENSIONS_ABP_TYPESELECTIONPAGE_HXX
#include "typeselectionpage.hxx"
#endif
#ifndef EXTENSIONS_ABP_ADMINDIALOG_INVOKATION_PAGE_HXX
#include "admininvokationpage.hxx"
#endif
#ifndef EXTENSIONS_ABP_TABLESELECTIONPAGE_HXX
#include "tableselectionpage.hxx"
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef EXTENSIONS_ABP_ABPFINALPAGE_HXX
#include "abpfinalpage.hxx"
#endif
#ifndef EXTENSIONS_ABP_FIELDMAPPINGPAGE_HXX
#include "fieldmappingpage.hxx"
#endif
#ifndef EXTENSIONS_ABP_FIELDMAPPINGIMPL_HXX
#include "fieldmappingimpl.hxx"
#endif

//.........................................................................
namespace abp
{
//.........................................................................

#define STATE_SELECT_ABTYPE         0
#define STATE_INVOKE_ADMIN_DIALOG   1
#define STATE_TABLE_SELECTION       2
#define STATE_MANUAL_FIELD_MAPPING  3
#define STATE_FINAL_CONFIRM         4

#define PATH_DYN_TABLE_SELECTION    1   // path where we dynamically decide whether we need table selection
#define PATH_LDAP                   2

    using namespace ::svt;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= OAddessBookSourcePilot
    //=====================================================================
    //---------------------------------------------------------------------
    OAddessBookSourcePilot::OAddessBookSourcePilot(Window* _pParent, const Reference< XMultiServiceFactory >& _rxORB)
        :OAddessBookSourcePilot_Base( _pParent, ModuleRes( RID_DLG_ADDRESSBOOKSOURCEPILOT ),
#if defined( ABP_USE_ROADMAP )
            WZB_HELP | WZB_FINISH | WZB_CANCEL | WZB_NEXT | WZB_PREVIOUS, ResId( STR_ROADMAP_TITLE ) )
#else
            WZB_HELP | WZB_FINISH | WZB_CANCEL | WZB_NEXT | WZB_PREVIOUS )
#endif
        ,m_xORB(_rxORB)
        ,m_aNewDataSource(_rxORB)
        ,m_eNewDataSourceType( AST_INVALID )
    {
        SetPageSizePixel(LogicToPixel(Size(WINDOW_SIZE_X, WINDOW_SIZE_Y), MAP_APPFONT));

        ShowButtonFixedLine(sal_True);

#if defined( ABP_USE_ROADMAP )
        // we have two paths: One with, and one without table selection
        declarePath( PATH_DYN_TABLE_SELECTION,
            STATE_SELECT_ABTYPE,
            STATE_INVOKE_ADMIN_DIALOG,
            STATE_TABLE_SELECTION,
            STATE_MANUAL_FIELD_MAPPING,
            STATE_FINAL_CONFIRM,
            WZS_INVALID_STATE
        );
        declarePath( PATH_LDAP,
            STATE_SELECT_ABTYPE,
            STATE_INVOKE_ADMIN_DIALOG,
            STATE_FINAL_CONFIRM,
            WZS_INVALID_STATE
        );
            // Note that in theory, we could make only one path out of it. But, to determine whether we
            // need a table selection page, we need to establish the connection. This is done immediately
            // before the pre-table-selection page is left.
            // As a result, if we would have only one path, then the user could click the "Table selection"
            // in the roadmap, and it could happen that we only at this very moment that we do *not need*
            // this page at all - which would look pretty strange to the user.

        // unless told otherwise, don't allow traveling to the last page
        enableState( STATE_FINAL_CONFIRM, false );
#else
        enableHeader( Bitmap( ModuleRes( BMP_HEADERIMAGE ) ) );
#endif

        m_pPrevPage->SetHelpId(HID_ABSPILOT_PREVIOUS);
        m_pNextPage->SetHelpId(HID_ABSPILOT_NEXT);
        m_pCancel->SetHelpId(HID_ABSPILOT_CANCEL);
        m_pFinish->SetHelpId(HID_ABSPILOT_FINISH);
        m_pHelp->SetUniqueId(UID_ABSPILOT_HELP);

        m_pCancel->SetClickHdl( LINK( this, OAddessBookSourcePilot, OnCancelClicked) );

        // some initial settings
#ifdef UNX
        m_aSettings.eType = AST_MORK;
#else
        m_aSettings.eType = AST_OE;
#endif
        m_aSettings.sDataSourceName = String(ModuleRes(RID_STR_DEFAULT_NAME));
        m_aSettings.bRegisterDataSource = sal_False;

        defaultButton(WZB_NEXT);
        enableButtons(WZB_FINISH, sal_False);
        ActivatePage();
    }

#if defined( ABP_USE_ROADMAP )
    //---------------------------------------------------------------------
    String OAddessBookSourcePilot::getStateDisplayName( WizardState _nState )
    {
        USHORT nResId = 0;
        switch ( _nState )
        {
            case STATE_SELECT_ABTYPE:        nResId = STR_SELECT_ABTYPE; break;
            case STATE_INVOKE_ADMIN_DIALOG:  nResId = STR_INVOKE_ADMIN_DIALOG; break;
            case STATE_TABLE_SELECTION:      nResId = STR_TABLE_SELECTION; break;
            case STATE_MANUAL_FIELD_MAPPING: nResId = STR_MANUAL_FIELD_MAPPING; break;
            case STATE_FINAL_CONFIRM:        nResId = STR_FINAL_CONFIRM; break;
        }
        DBG_ASSERT( nResId, "OAddessBookSourcePilot::getStateDisplayName: don't know this state!" );

        String sDisplayName;
        if ( nResId )
        {
            svt::OLocalResourceAccess aAccess( ModuleRes( RID_DLG_ADDRESSBOOKSOURCEPILOT ), RSC_MODALDIALOG );
            sDisplayName = String( ResId( nResId ) );
        }

        return sDisplayName;
    }
#endif

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
    IMPL_LINK( OAddessBookSourcePilot, OnCancelClicked, void*, NOTINTERESTEDIN )
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
    BOOL OAddessBookSourcePilot::Close()
    {
        implCleanup();

        return OAddessBookSourcePilot_Base::Close();
    }

    //---------------------------------------------------------------------
    sal_Bool OAddessBookSourcePilot::onFinish(sal_Int32 _nResult)
    {
        if (!OAddessBookSourcePilot_Base::onFinish(_nResult))
            return sal_False;

        if (RET_OK != _nResult)
            return sal_True;

        implCommitAll();

        addressconfig::markPilotSuccess( getORB() );

        return sal_True;
    }

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::enterState( WizardState _nState )
    {
        switch ( _nState )
        {
#if defined( ABP_USE_ROADMAP )
            case STATE_SELECT_ABTYPE:
                implUpdateTypeDependentStates( static_cast< TypeSelectionPage* >( getPage( STATE_SELECT_ABTYPE ) )->getSelectedType() );
                break;
#endif

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

#if defined( ABP_USE_ROADMAP )
    //---------------------------------------------------------------------
    sal_Bool OAddessBookSourcePilot::prepareLeaveCurrentState( CommitPageReason _eReason )
    {
        if ( !OAddessBookSourcePilot_Base::prepareLeaveCurrentState( _eReason ) )
            return sal_False;

        if ( _eReason == eTravelBackward )
            return sal_True;

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
                // connecting did not succeed -> do not allow proceeding, and do not
                // allow the final page
                enableState( STATE_FINAL_CONFIRM, false );
                return sal_False;
            }
            enableState( STATE_FINAL_CONFIRM, true );

            if ( m_aSettings.eType == AST_LDAP )
                break;

            // ........................................................
            // now that we connected to the data source, check whether we need the "table selection" page
            const StringBag& aTables = m_aNewDataSource.getTableNames();
            enableState( STATE_TABLE_SELECTION, aTables.size() > 1 );

            bool bNeedFieldMapping = needManualFieldMapping( m_aSettings.eType );
            if ( aTables.size() == 0 )
            {
                if ( _eReason == eValidateNoUI )
                    // cannot ask the user
                    return sal_False;

                QueryBox aQuery( this, ModuleRes( RID_QRY_NOTABLES ) );
                if ( RET_YES == aQuery.Execute() )
                {   // the user chose to use this data source, though there are no tables
                    bNeedFieldMapping = false;
                }
                else
                    return sal_False;
            }
            enableState( STATE_MANUAL_FIELD_MAPPING, bNeedFieldMapping );

            // from now on, we're definately on the "dynamic table selection" path
            activatePath( PATH_DYN_TABLE_SELECTION, true );

            if ( aTables.size() == 1 )
                // remember the one and only table we have
                m_aSettings.sSelectedTable = *aTables.begin();

            break;
        }

        return sal_True;
    }
#endif

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
            case AST_MORK       : pGuess = "Personal Address Book"; break;
            case AST_LDAP       : pGuess = "LDAP Directory"; break;
        }
        const ::rtl::OUString sGuess = ::rtl::OUString::createFromAscii( pGuess );
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

            case AST_EVOLUTION:
                m_aNewDataSource = aContext.createNewEvolution( m_aSettings.sDataSourceName );
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
                DBG_ERROR("OAddessBookSourcePilot::createPage: invalid state!");
                return NULL;
        }
    }

#if defined( ABP_USE_ROADMAP )
    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::implUpdateTypeDependentStates( AddressSourceType _eType )
    {
        enableState( STATE_INVOKE_ADMIN_DIALOG, needAdminInvokationPage( _eType ) );
        enableState( STATE_MANUAL_FIELD_MAPPING, needManualFieldMapping( _eType ) );
    }
#endif

    //---------------------------------------------------------------------
    void OAddessBookSourcePilot::typeSelectionChanged( AddressSourceType _eType )
    {
#if defined( ABP_USE_ROADMAP )
        implUpdateTypeDependentStates( _eType );
        enableState( STATE_FINAL_CONFIRM, false );

        // for LDAP, there is no "table selection" page
        if ( _eType == AST_LDAP )
            activatePath( PATH_LDAP, true );
        else
            activatePath( PATH_DYN_TABLE_SELECTION, false );
#endif
    }

#if !defined( ABP_USE_ROADMAP )
    //---------------------------------------------------------------------
    WizardTypes::WizardState OAddessBookSourcePilot::determineNextState(WizardState _nCurrentState)
    {
        switch (_nCurrentState)
        {
            case STATE_SELECT_ABTYPE:

                if  ( needAdminInvokationPage() )
                    return STATE_INVOKE_ADMIN_DIALOG;
                // _NO_ break !!!

            case STATE_INVOKE_ADMIN_DIALOG:
            {
                // determining the next state here is somewhat more complex: it depends on the type selected,
                // and on the number of tables in the data source specified by the user

                // ........................................................
                // create the new data source
                implCreateDataSource( );

                // ........................................................
                // if we're here, we have all settings necessary for connecting to the data source
                if ( !connectToDataSource( sal_False ) )
                    // connecting did not succeed -> do not allow traveling (indicated by returning WZS_INVALID_STATE)
                    return WZS_INVALID_STATE;

                // ........................................................
                // get the tables of the connection
                const StringBag& aTables = m_aNewDataSource.getTableNames();
                if ( aTables.size() > 1 )
                    // in the next step, we need to determine the primary address table
                    return STATE_TABLE_SELECTION;

                // ........................................................
                // do we have any tables ?
                if ( 0 == aTables.size() )
                {   // ... no
                    QueryBox aQuery( this, ModuleRes( RID_QRY_NOTABLES ) );
                    if ( RET_YES == aQuery.Execute() )
                    {   // but the user chose to use this data source, anyway
                        // -> go to the final page (no field mapping possible at all ...)
                        return STATE_FINAL_CONFIRM;
                    }

                    // not allowed to leave the page
                    return WZS_INVALID_STATE;
                }

                // remember the one and only table we have
                m_aSettings.sSelectedTable = *aTables.begin();
            }
                // _NO_ break !!!

            case STATE_TABLE_SELECTION:

                // ........................................................
                // do we need a field mapping provided by the user?
                if ( needManualFieldMapping( ) )
                    return STATE_MANUAL_FIELD_MAPPING;

                // ........................................................
                // we're nearly done ...
                return STATE_FINAL_CONFIRM;

            case STATE_MANUAL_FIELD_MAPPING:
                return STATE_FINAL_CONFIRM;
        }

        return WZS_INVALID_STATE;
    }
#endif

//.........................................................................
}   // namespace abp
//.........................................................................

