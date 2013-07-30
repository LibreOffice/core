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


#include "dsnItem.hxx"
#include "generalpage.hxx"
#include <connectivity/dbexception.hxx>
#include "dbu_dlg.hrc"
#include "dbadmin.hrc"
#include "dsitems.hxx"
#include "dbustrings.hrc"
#include "dbadmin.hxx"
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>
#include <vcl/stdtext.hxx>
#include "localresaccess.hxx"
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <vcl/waitobj.hxx>
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include "DriverSettings.hxx"
#include "UITools.hxx"
#include <comphelper/processfactory.hxx>
#include <unotools/confignode.hxx>
#include <osl/diagnose.h>

//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //=========================================================================
    //= OGeneralPage
    //=========================================================================
    //-------------------------------------------------------------------------
    OGeneralPage::OGeneralPage( Window* pParent, const OUString& _rUIXMLDescription, const SfxItemSet& _rItems )
        :OGenericAdministrationPage( pParent, "PageGeneral", _rUIXMLDescription, _rItems )
        ,m_pSpecialMessage              ( NULL )
        ,m_eNotSupportedKnownType       ( ::dbaccess::DST_UNKNOWN )
        ,m_eLastMessage                 ( smNone )
        ,m_bDisplayingInvalid           ( sal_False )
        ,m_bInitTypeList                ( true )
        ,m_bInitEmbeddedDBList          ( true )
        ,m_pDatasourceType              ( NULL )
        ,m_pEmbeddedDBType              ( NULL )
        ,m_pCollection                  ( NULL )
    {
        get( m_pDatasourceType, "datasourceType" );
        get( m_pEmbeddedDBType, "embeddeddbList" );
        get( m_pSpecialMessage, "specialMessage" );

        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rItems.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        SAL_WARN_IF(!m_pCollection, "dbaaccess", "OGeneralPage::OGeneralPage : really need a DSN type collection !");

        // do some knittings
        m_pDatasourceType->SetSelectHdl(LINK(this, OGeneralPage, OnDatasourceTypeSelected));
        m_pEmbeddedDBType->SetSelectHdl(LINK(this, OGeneralPage, OnEmbeddedDBTypeSelected));
    }

    //-------------------------------------------------------------------------
    OGeneralPage::~OGeneralPage()
    {
    }

    //-------------------------------------------------------------------------
    namespace
    {
        struct DisplayedType
        {
            OUString eType;
            String          sDisplayName;

            DisplayedType( const OUString& _eType, const String& _rDisplayName ) : eType( _eType ), sDisplayName( _rDisplayName ) { }
        };
        typedef ::std::vector< DisplayedType > DisplayedTypes;

        struct DisplayedTypeLess : ::std::binary_function< DisplayedType, DisplayedType, bool >
        {
            bool operator() ( const DisplayedType& _rLHS, const DisplayedType& _rRHS )
            {
                return _rLHS.eType < _rRHS.eType;
            }
        };
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::initializeTypeList()
    {
        if ( m_bInitTypeList )
        {
            m_bInitTypeList = false;
            m_pDatasourceType->Clear();

            if ( m_pCollection )
            {
                DisplayedTypes aDisplayedTypes;

                ::dbaccess::ODsnTypeCollection::TypeIterator aEnd = m_pCollection->end();
                for (   ::dbaccess::ODsnTypeCollection::TypeIterator aTypeLoop =  m_pCollection->begin();
                        aTypeLoop != aEnd;
                        ++aTypeLoop
                    )
                {
                    const OUString sURLPrefix = aTypeLoop.getURLPrefix();
                    if ( !sURLPrefix.isEmpty() )
                    {
                        OUString sDisplayName = aTypeLoop.getDisplayName();
                        if (   m_pDatasourceType->GetEntryPos( sDisplayName ) == LISTBOX_ENTRY_NOTFOUND
                            && approveDatasourceType( sURLPrefix, sDisplayName ) )
                        {
                            aDisplayedTypes.push_back( DisplayedTypes::value_type( sURLPrefix, sDisplayName ) );
                        }
                    }
                }
                ::std::sort( aDisplayedTypes.begin(), aDisplayedTypes.end(), DisplayedTypeLess() );
                DisplayedTypes::const_iterator aDisplayEnd = aDisplayedTypes.end();
                for (   DisplayedTypes::const_iterator loop = aDisplayedTypes.begin();
                        loop != aDisplayEnd;
                        ++loop
                    )
                    insertDatasourceTypeEntryData( loop->eType, loop->sDisplayName );
            }
        }
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::initializeEmbeddedDBList()
    {
        if ( m_bInitEmbeddedDBList )
        {
            m_bInitEmbeddedDBList = false;
             m_pEmbeddedDBType->Clear();

            if ( m_pCollection )
            {
                DisplayedTypes aDisplayedTypes;

                ::dbaccess::ODsnTypeCollection::TypeIterator aEnd = m_pCollection->end();
                for (   ::dbaccess::ODsnTypeCollection::TypeIterator aTypeLoop =  m_pCollection->begin();
                        aTypeLoop != aEnd;
                        ++aTypeLoop
                    )
                {
                    const OUString sURLPrefix = aTypeLoop.getURLPrefix();
                    if ( !sURLPrefix.isEmpty() )
                    {
                        OUString sDisplayName = aTypeLoop.getDisplayName();
                        if ( m_pEmbeddedDBType->GetEntryPos( sDisplayName ) == LISTBOX_ENTRY_NOTFOUND
                            && m_pCollection->isEmbeddedDatabase( sURLPrefix ) )
                        {
                            aDisplayedTypes.push_back( DisplayedTypes::value_type( sURLPrefix, sDisplayName ) );
                        }
                    }
                }
                ::std::sort( aDisplayedTypes.begin(), aDisplayedTypes.end(), DisplayedTypeLess() );
                DisplayedTypes::const_iterator aDisplayEnd = aDisplayedTypes.end();
                for (   DisplayedTypes::const_iterator loop = aDisplayedTypes.begin();
                        loop != aDisplayEnd;
                        ++loop
                    )
                    insertEmbeddedDBTypeEntryData( loop->eType, loop->sDisplayName );
            }
        }
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::setParentTitle(const OUString&)
    {
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::switchMessage(const OUString& _sURLPrefix)
    {
        SPECIAL_MESSAGE eMessage = smNone;
        if ( _sURLPrefix.isEmpty()/*_eType == m_eNotSupportedKnownType*/ )
        {
            eMessage = smUnsupportedType;
        }


        if ( eMessage != m_eLastMessage )
        {
            sal_uInt16 nResId = 0;
            if ( smUnsupportedType == eMessage )
                nResId = STR_UNSUPPORTED_DATASOURCE_TYPE;
            String sMessage;
            if ( nResId )
                sMessage = String( ModuleRes( nResId ) );

            m_pSpecialMessage->SetText( sMessage );
            m_eLastMessage = eMessage;
        }
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::onTypeSelected(const OUString& _sURLPrefix)
    {
        // the new URL text as indicated by the selection history
        implSetCurrentType( _sURLPrefix );

        switchMessage(_sURLPrefix);

        if ( m_aTypeSelectHandler.IsSet() )
            m_aTypeSelectHandler.Call(this);
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implInitControls( const SfxItemSet& _rSet, sal_Bool _bSaveValue )
    {
        initializeTypeList();
        initializeEmbeddedDBList();

        m_pDatasourceType->SelectEntry( getDatasourceName( _rSet ) );
        m_pEmbeddedDBType->SelectEntry( getEmbeddedDBName( _rSet ) );

        // notify our listener that our type selection has changed (if so)
        // FIXME: how to detect that it did not changed? (fdo#62937)
        setParentTitle( m_eCurrentSelection );
        onTypeSelected( m_eCurrentSelection );

        // a special message for the current page state
        switchMessage( m_eCurrentSelection );

        OGenericAdministrationPage::implInitControls( _rSet, _bSaveValue );
    }

    //-------------------------------------------------------------------------
    OUString OGeneralPage::getEmbeddedDBName( const SfxItemSet& _rSet )
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags( _rSet, bValid, bReadonly );

        // if the selection is invalid, disable everything
        String sName,sConnectURL;
        m_bDisplayingInvalid = !bValid;
        if ( bValid )
        {
            // collect some items and some values
            SFX_ITEMSET_GET( _rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True );
            SFX_ITEMSET_GET( _rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True );
            assert( pUrlItem );
            assert( pNameItem );
            sName = pNameItem->GetValue();
            sConnectURL = pUrlItem->GetValue();
        }

        m_eNotSupportedKnownType =  ::dbaccess::DST_UNKNOWN;
        implSetCurrentType(  OUString() );

        // compare the DSN prefix with the registered ones
        OUString sDisplayName;

        if (m_pCollection && bValid)
        {
            implSetCurrentType( m_pCollection->getEmbeddedDatabase() );
            sDisplayName = m_pCollection->getTypeDisplayName( m_eCurrentSelection );
        }

        // select the correct datasource type
        if  (  m_pCollection->isEmbeddedDatabase( m_eCurrentSelection )
            &&  ( LISTBOX_ENTRY_NOTFOUND == m_pEmbeddedDBType->GetEntryPos( sDisplayName ) )
            )
        {   // this indicates it's really a type which is known in general, but not supported on the current platform
            // show a message saying so
            //  eSpecialMessage = smUnsupportedType;
            insertDatasourceTypeEntryData( m_eCurrentSelection, sDisplayName );
            // remember this type so we can show the special message again if the user selects this
            // type again (without changing the data source)
            m_eNotSupportedKnownType = m_pCollection->determineType( m_eCurrentSelection );
        }

        return sDisplayName;
    }

    //-------------------------------------------------------------------------
    OUString OGeneralPage::getDatasourceName( const SfxItemSet& _rSet )
    {
        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags( _rSet, bValid, bReadonly );

        // if the selection is invalid, disable everything
        String sName,sConnectURL;
        m_bDisplayingInvalid = !bValid;
        if ( bValid )
        {
            // collect some items and some values
            SFX_ITEMSET_GET( _rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True );
            SFX_ITEMSET_GET( _rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True );
            assert( pUrlItem );
            assert( pNameItem );
            sName = pNameItem->GetValue();
            sConnectURL = pUrlItem->GetValue();
        }

        m_eNotSupportedKnownType =  ::dbaccess::DST_UNKNOWN;
        implSetCurrentType(  OUString() );

        // compare the DSN prefix with the registered ones
        OUString sDisplayName;

        if (m_pCollection && bValid)
        {
            implSetCurrentType( m_pCollection->getPrefix( sConnectURL ) );
            sDisplayName = m_pCollection->getTypeDisplayName( m_eCurrentSelection );
        }

        // select the correct datasource type
        if  (   approveDatasourceType( m_eCurrentSelection, sDisplayName )
            &&  ( LISTBOX_ENTRY_NOTFOUND == m_pDatasourceType->GetEntryPos( sDisplayName ) )
            )
        {   // this indicates it's really a type which is known in general, but not supported on the current platform
            // show a message saying so
            //  eSpecialMessage = smUnsupportedType;
            insertDatasourceTypeEntryData( m_eCurrentSelection, sDisplayName );
            // remember this type so we can show the special message again if the user selects this
            // type again (without changing the data source)
            m_eNotSupportedKnownType = m_pCollection->determineType( m_eCurrentSelection );
        }

        return sDisplayName;
    }

    // For the databaseWizard we only have one entry for the MySQL Database,
    // because we have a separate tabpage to retrieve the respective datasource type
    // ( ::dbaccess::DST_MYSQL_ODBC ||  ::dbaccess::DST_MYSQL_JDBC). Therefore we use  ::dbaccess::DST_MYSQL_JDBC as a temporary
    // representative for all MySQl databases)
    // Also, embedded databases (embedded HSQL, at the moment), are not to appear in the list of
    // databases to connect to.
    bool OGeneralPage::approveDatasourceType( const OUString& _sURLPrefix, OUString& _inout_rDisplayName )
    {
        return approveDatasourceType( m_pCollection->determineType(_sURLPrefix), _inout_rDisplayName );
    }

    bool OGeneralPage::approveDatasourceType( ::dbaccess::DATASOURCE_TYPE eType, OUString& _inout_rDisplayName )
    {
        if ( eType == ::dbaccess::DST_MYSQL_NATIVE_DIRECT )
        {
            // do not display the Connector/OOo driver itself, it is always wrapped via the MySQL-Driver, if
            // this driver is installed
            if ( m_pCollection->hasDriver( "sdbc:mysql:mysqlc:" ) )
                _inout_rDisplayName = String();
        }

        if ( eType ==  ::dbaccess::DST_EMBEDDED_HSQLDB
                || eType ==  ::dbaccess::DST_EMBEDDED_FIREBIRD )
            _inout_rDisplayName = String();

        return _inout_rDisplayName.getLength() > 0;
    }


    // -----------------------------------------------------------------------
    void OGeneralPage::insertDatasourceTypeEntryData(const OUString& _sType, String sDisplayName)
    {
        // insert a (temporary) entry
        sal_uInt16 nPos = m_pDatasourceType->InsertEntry(sDisplayName);
        if ( nPos >= m_aURLPrefixes.size() )
            m_aURLPrefixes.resize(nPos+1);
        m_aURLPrefixes[nPos] = _sType;
    }

    // -----------------------------------------------------------------------
    void OGeneralPage::insertEmbeddedDBTypeEntryData(const OUString& _sType, String sDisplayName)
    {
        // insert a (temporary) entry
        sal_uInt16 nPos = m_pEmbeddedDBType->InsertEntry(sDisplayName);
        if ( nPos >= m_aEmbeddedURLPrefixes.size() )
            m_aEmbeddedURLPrefixes.resize(nPos+1);
        m_aEmbeddedURLPrefixes[nPos] = _sType;
    }

    // -----------------------------------------------------------------------
    void OGeneralPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back( new ODisableWrapper<FixedText>( m_pSpecialMessage ) );
    }

    // -----------------------------------------------------------------------
    void OGeneralPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back( new OSaveValueWrapper<ListBox>( m_pDatasourceType ) );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implSetCurrentType( const OUString& _eType )
    {
        if ( _eType == m_eCurrentSelection )
            return;

        m_eCurrentSelection = _eType;
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::Reset(const SfxItemSet& _rCoreAttrs)
    {
        // reset all locale data
        implSetCurrentType(  OUString() );
            // this ensures that our type selection link will be called, even if the new is is the same as the
            // current one
        OGenericAdministrationPage::Reset(_rCoreAttrs);
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( OGeneralPage, OnEmbeddedDBTypeSelected, ListBox*, _pBox )
    {
        // get the type from the entry data
        sal_uInt16 nSelected = _pBox->GetSelectEntryPos();
        if (nSelected >= m_aEmbeddedURLPrefixes.size() )
        {
            SAL_WARN("dbaui.OGeneralPage", "Got out-of-range value '" << nSelected <<  "' from the DatasourceType selection ListBox's GetSelectEntryPos(): no corresponding URL prefix");
            return 0L;
        }
        const OUString sURLPrefix = m_aEmbeddedURLPrefixes[ nSelected ];

        setParentTitle( sURLPrefix );
        // let the impl method do all the stuff
        onTypeSelected( sURLPrefix );
        // tell the listener we were modified
        callModifiedHdl();
        // outta here
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( OGeneralPage, OnDatasourceTypeSelected, ListBox*, _pBox )
    {
        // get the type from the entry data
        sal_uInt16 nSelected = _pBox->GetSelectEntryPos();
        if (nSelected >= m_aURLPrefixes.size() )
        {
            SAL_WARN("dbaccess.ui.OGeneralPage", "Got out-of-range value '" << nSelected <<  "' from the DatasourceType selection ListBox's GetSelectEntryPos(): no corresponding URL prefix");
            return 0L;
        }
        const OUString sURLPrefix = m_aURLPrefixes[ nSelected ];

        setParentTitle( sURLPrefix );
        // let the impl method do all the stuff
        onTypeSelected( sURLPrefix );
        // tell the listener we were modified
        callModifiedHdl();
        // outta here
        return 0L;
    }

    //=========================================================================
    //= OGeneralPageDialog
    //=========================================================================
    //-------------------------------------------------------------------------
    OGeneralPageDialog::OGeneralPageDialog( Window* pParent, const SfxItemSet& _rItems )
        :OGeneralPage( pParent, "dbaccess/ui/generalpagedialog.ui", _rItems )
    {
    }

    //-------------------------------------------------------------------------
    void OGeneralPageDialog::setParentTitle( const OUString& _sURLPrefix )
    {
        const OUString sName = m_pCollection->getTypeDisplayName( _sURLPrefix );
        if ( m_pAdminDialog )
        {
            OUString sMessage = OUString( ModuleRes( STR_PARENTTITLE_GENERAL ) );
            m_pAdminDialog->setTitle( sMessage.replaceAll( "#", sName ) );
        }
    }

    //-------------------------------------------------------------------------
    void OGeneralPageDialog::implInitControls( const SfxItemSet& _rSet, sal_Bool _bSaveValue )
    {
        OGeneralPage::implInitControls( _rSet, _bSaveValue );

        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly );

        m_pDatasourceType->Enable( bValid );
    }

    //-------------------------------------------------------------------------
    sal_Bool OGeneralPageDialog::FillItemSet( SfxItemSet& _rCoreAttrs )
    {
        sal_Bool bChangedSomething = sal_False;

        sal_uInt16 nEntry = m_pDatasourceType->GetSelectEntryPos();
        OUString sURLPrefix = m_aURLPrefixes[ nEntry ];

        if ( m_pDatasourceType->GetSavedValue() != nEntry )
        {
            _rCoreAttrs.Put( SfxStringItem( DSID_CONNECTURL, sURLPrefix ) );
            bChangedSomething = sal_True;
        }

        return bChangedSomething;
    }

    //=========================================================================
    //= OGeneralPageWizard
    //=========================================================================
    //-------------------------------------------------------------------------
    OGeneralPageWizard::OGeneralPageWizard( Window* pParent, const SfxItemSet& _rItems )
        :OGeneralPage( pParent, "dbaccess/ui/generalpagewizard.ui", _rItems )
        ,m_pFT_HeaderText               ( NULL )
        ,m_pFT_HelpText                 ( NULL )
        ,m_pRB_CreateDatabase           ( NULL )
        ,m_pRB_OpenExistingDatabase     ( NULL )
        ,m_pRB_ConnectDatabase          ( NULL )
        ,m_pFT_EmbeddedDBLabel          ( NULL )
        ,m_pFT_DocListLabel             ( NULL )
        ,m_pLB_DocumentList             ( NULL )
        ,m_pPB_OpenDatabase             ( NULL )
        ,m_eOriginalCreationMode        ( eCreateNew )
    {
        get( m_pFT_HeaderText, "headerText" );
        get( m_pFT_HelpText, "helpText" );
        get( m_pRB_CreateDatabase, "createDatabase" );
        get( m_pRB_OpenExistingDatabase, "openExistingDatabase" );
        get( m_pRB_ConnectDatabase, "connectDatabase" );
        get( m_pFT_EmbeddedDBLabel, "embeddeddbLabel" );
        get( m_pFT_DocListLabel, "docListLabel" );
        get( m_pLB_DocumentList, "documentList" );
        get( m_pPB_OpenDatabase, "openDatabase" );

        // If no driver for embedded DBs is installed, and no dBase driver, then hide the "Create new database" option
        sal_Int32 nCreateNewDBIndex = m_pCollection->getIndexOf( m_pCollection->getEmbeddedDatabase() );
        if ( nCreateNewDBIndex == -1 )
            nCreateNewDBIndex = m_pCollection->getIndexOf( OUString( "sdbc:dbase:" ) );
        bool bHideCreateNew = ( nCreateNewDBIndex == -1 );

        // also, if our application policies tell us to hide the option, do it
        ::utl::OConfigurationTreeRoot aConfig( ::utl::OConfigurationTreeRoot::createWithComponentContext(
            ::comphelper::getProcessComponentContext(),
            OUString( "/org.openoffice.Office.DataAccess/Policies/Features/Base" )
        ) );
        sal_Bool bAllowCreateLocalDatabase( sal_True );
        OSL_VERIFY( aConfig.getNodeValue( "CreateLocalDatabase" ) >>= bAllowCreateLocalDatabase );
        if ( !bAllowCreateLocalDatabase )
            bHideCreateNew = true;

        if ( bHideCreateNew )
        {
            m_pRB_CreateDatabase->Hide();
            m_pRB_ConnectDatabase->Check();
        }
        else
            m_pRB_CreateDatabase->Check();

        // do some knittings
        m_pRB_CreateDatabase->SetClickHdl( LINK( this, OGeneralPageWizard, OnCreateDatabaseModeSelected ) );
        m_pRB_ConnectDatabase->SetClickHdl( LINK( this, OGeneralPageWizard, OnSetupModeSelected ) );
        m_pRB_OpenExistingDatabase->SetClickHdl( LINK( this, OGeneralPageWizard, OnSetupModeSelected ) );
        m_pLB_DocumentList->SetSelectHdl( LINK( this, OGeneralPageWizard, OnDocumentSelected ) );
        m_pPB_OpenDatabase->SetClickHdl( LINK( this, OGeneralPageWizard, OnOpenDocument ) );
    }

    //-------------------------------------------------------------------------
    OGeneralPageWizard::CreationMode OGeneralPageWizard::GetDatabaseCreationMode() const
    {
        if ( m_pRB_CreateDatabase->IsChecked() )
            return eCreateNew;
        if ( m_pRB_ConnectDatabase->IsChecked() )
            return eConnectExternal;
        return eOpenExisting;
    }

    //-------------------------------------------------------------------------
    void OGeneralPageWizard::GetFocus()
    {
        OGeneralPage::GetFocus();
        if ( m_pLB_DocumentList && m_pLB_DocumentList->IsEnabled() )
            m_pLB_DocumentList->GrabFocus();
        else if ( m_pDatasourceType && m_pDatasourceType->IsEnabled() )
            m_pDatasourceType->GrabFocus();
    }

    //-------------------------------------------------------------------------
    void OGeneralPageWizard::implInitControls( const SfxItemSet& _rSet, sal_Bool _bSaveValue )
    {
        OGeneralPage::implInitControls( _rSet, _bSaveValue );

        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags( _rSet, bValid, bReadonly );

        SetText( String() );

        LayoutHelper::positionBelow( *m_pRB_ConnectDatabase, *m_pDatasourceType, RelatedControls, INDENT_BELOW_RADIO );

        if ( !bValid || bReadonly )
        {
            m_pFT_EmbeddedDBLabel->Enable( false );
            m_pDatasourceType->Enable( false );
            m_pPB_OpenDatabase->Enable( false );
            m_pFT_DocListLabel->Enable( false );
            m_pLB_DocumentList->Enable( false );
        }
        else
        {
            m_aControlDependencies.enableOnRadioCheck( *m_pRB_CreateDatabase, *m_pEmbeddedDBType, *m_pFT_EmbeddedDBLabel );
            m_aControlDependencies.enableOnRadioCheck( *m_pRB_ConnectDatabase, *m_pDatasourceType );
            m_aControlDependencies.enableOnRadioCheck( *m_pRB_OpenExistingDatabase, *m_pPB_OpenDatabase, *m_pFT_DocListLabel, *m_pLB_DocumentList );
        }

        m_pLB_DocumentList->SetDropDownLineCount( 20 );
        if ( m_pLB_DocumentList->GetEntryCount() )
            m_pLB_DocumentList->SelectEntryPos( 0 );

        m_eOriginalCreationMode = GetDatabaseCreationMode();
    }

    //-------------------------------------------------------------------------
    OUString OGeneralPageWizard::getDatasourceName(const SfxItemSet& _rSet)
    {
        // Sets jdbc as the default selected databse on startup.
        if (m_pRB_CreateDatabase->IsChecked() )
            return m_pCollection->getTypeDisplayName( OUString( "jdbc:" ) );

        return OGeneralPage::getDatasourceName( _rSet );
    }

    //-------------------------------------------------------------------------
    bool OGeneralPageWizard::approveDatasourceType( ::dbaccess::DATASOURCE_TYPE eType, OUString& _inout_rDisplayName )
    {
        switch ( eType )
        {
        case ::dbaccess::DST_MYSQL_JDBC:
            _inout_rDisplayName = "MySQL";
            break;
        case ::dbaccess::DST_MYSQL_ODBC:
        case ::dbaccess::DST_MYSQL_NATIVE:
            // don't display those, the decision whether the user connects via JDBC/ODBC/C-OOo is made on another
            // page
            _inout_rDisplayName = String();
            break;
        default:
            break;
        }

        return OGeneralPage::approveDatasourceType( eType, _inout_rDisplayName );
    }

    //-------------------------------------------------------------------------
    sal_Bool OGeneralPageWizard::FillItemSet(SfxItemSet& _rCoreAttrs)
    {
        sal_Bool bChangedSomething = sal_False;

        bool bCommitTypeSelection = true;

        if ( m_pRB_CreateDatabase->IsChecked() )
        {
            _rCoreAttrs.Put( SfxStringItem( DSID_CONNECTURL, OUString( "sdbc:dbase:" ) ) );
            bChangedSomething = sal_True;
            bCommitTypeSelection = false;
        }
        else if ( m_pRB_OpenExistingDatabase->IsChecked() )
        {
            if ( m_pRB_OpenExistingDatabase->GetSavedValue() != m_pRB_OpenExistingDatabase->IsChecked() )
                bChangedSomething = sal_True;

            // TODO
            bCommitTypeSelection = false;
        }

        if ( bCommitTypeSelection )
        {
            sal_uInt16 nEntry = m_pDatasourceType->GetSelectEntryPos();
            OUString sURLPrefix = m_aURLPrefixes[nEntry];

            if  (  ( m_pDatasourceType->GetSavedValue() != nEntry )
                || ( GetDatabaseCreationMode() != m_eOriginalCreationMode )
                )
            {
                _rCoreAttrs.Put( SfxStringItem( DSID_CONNECTURL,sURLPrefix ) );
                bChangedSomething = sal_True;
            }
            else
                implSetCurrentType( sURLPrefix );
        }
        return bChangedSomething;
    }

    //-------------------------------------------------------------------------
    OGeneralPageWizard::DocumentDescriptor OGeneralPageWizard::GetSelectedDocument() const
    {
        DocumentDescriptor aDocument;
        if ( m_aBrowsedDocument.sURL.Len() )
            aDocument = m_aBrowsedDocument;
        else
        {
            aDocument.sURL = m_pLB_DocumentList->GetSelectedDocumentURL();
            aDocument.sFilter = m_pLB_DocumentList->GetSelectedDocumentFilter();
        }
        return aDocument;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( OGeneralPageWizard, OnCreateDatabaseModeSelected, RadioButton*, /*_pBox*/ )
    {
        if ( m_aCreationModeHandler.IsSet() )
            m_aCreationModeHandler.Call( this );
        OnEmbeddedDBTypeSelected(m_pEmbeddedDBType);
        return 1L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( OGeneralPageWizard, OnSetupModeSelected, RadioButton*, /*_pBox*/ )
    {
        if ( m_aCreationModeHandler.IsSet() )
            m_aCreationModeHandler.Call( this );
        OnDatasourceTypeSelected(m_pDatasourceType);
        return 1L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( OGeneralPageWizard, OnDocumentSelected, ListBox*, /*_pBox*/ )
    {
        m_aDocumentSelectionHandler.Call( this );
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK( OGeneralPageWizard, OnOpenDocument, PushButton*, /*_pBox*/ )
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                0, OUString("sdatabase") );
        const SfxFilter* pFilter = getStandardDatabaseFilter();
        if ( pFilter )
        {
            aFileDlg.SetCurrentFilter(pFilter->GetUIName());
        }
        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            String sPath = aFileDlg.GetPath();
            if ( aFileDlg.GetCurrentFilter() != pFilter->GetUIName() || !pFilter->GetWildcard().Matches(sPath) )
            {
                String sMessage(ModuleRes(STR_ERR_USE_CONNECT_TO));
                InfoBox aError(this, sMessage);
                aError.Execute();
                m_pRB_ConnectDatabase->Check();
                OnSetupModeSelected( m_pRB_ConnectDatabase );
                return 0L;
            }
            m_aBrowsedDocument.sURL = sPath;
            m_aBrowsedDocument.sFilter = String();
            m_aChooseDocumentHandler.Call( this );
            return 1L;
        }

        return 0L;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
