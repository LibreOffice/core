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
    OGeneralPage::OGeneralPage(Window* pParent, const SfxItemSet& _rItems, sal_Bool _bDBWizardMode)
        :OGenericAdministrationPage(pParent, ModuleRes(PAGE_GENERAL), _rItems)
        ,m_aFTHeaderText                (this, ModuleRes(FT_GENERALHEADERTEXT))
        ,m_aFTHelpText                  (this, ModuleRes(FT_GENERALHELPTEXT))
        ,m_aFT_DatasourceTypeHeader     (this, ModuleRes(FT_DATASOURCEHEADER))
        ,m_aRB_CreateDatabase           (this, ModuleRes(RB_CREATEDBDATABASE))
        ,m_aRB_OpenDocument             (this, ModuleRes(RB_OPENEXISTINGDOC))
        ,m_aRB_GetExistingDatabase      (this, ModuleRes(RB_GETEXISTINGDATABASE))
        ,m_aFT_DocListLabel             (this, ModuleRes(FT_DOCLISTLABEL))
        ,m_pLB_DocumentList             ( new OpenDocumentListBox( this, "com.sun.star.sdb.OfficeDatabaseDocument", ModuleRes( LB_DOCUMENTLIST ) ) )
        ,m_aPB_OpenDocument             (this, "com.sun.star.sdb.OfficeDatabaseDocument", ModuleRes(PB_OPENDOCUMENT))
        ,m_aTypePreLabel                (this, ModuleRes(FT_DATASOURCETYPE_PRE))
        ,m_aDatasourceTypeLabel         (this, ModuleRes(FT_DATATYPE))
        ,m_pDatasourceType              ( new ListBox(this, ModuleRes(LB_DATATYPE)))
        ,m_aFTDataSourceAppendix        (this, ModuleRes(FT_DATATYPEAPPENDIX))
        ,m_aTypePostLabel               (this, ModuleRes(FT_DATASOURCETYPE_POST))
        ,m_aSpecialMessage              (this, ModuleRes(FT_SPECIAL_MESSAGE))
        ,m_DBWizardMode                 (_bDBWizardMode)
        ,m_sMySQLEntry                  (ModuleRes(STR_MYSQLENTRY))
        ,m_eOriginalCreationMode        (eCreateNew)
        ,m_pCollection                  (NULL)
        ,m_eNotSupportedKnownType       ( ::dbaccess::DST_UNKNOWN)
        ,m_eLastMessage                 (smNone)
        ,m_bDisplayingInvalid           (sal_False)
        ,m_bInitTypeList                (true)
    {
        // fill the listbox with the UI descriptions for the possible types
        // and remember the respective DSN prefixes
        FreeResource();
        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rItems.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        OSL_ENSURE(m_pCollection, "OGeneralPage::OGeneralPage : really need a DSN type collection !");

        // If no driver for embedded DBs is installed, and no dBase driver, then hide the "Create new database" option
        sal_Int32 nCreateNewDBIndex = m_pCollection->getIndexOf( m_pCollection->getEmbeddedDatabase() );
        if ( nCreateNewDBIndex == -1 )
            nCreateNewDBIndex = m_pCollection->getIndexOf( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:dbase:")) );
        bool bHideCreateNew = ( nCreateNewDBIndex == -1 );

        // also, if our application policies tell us to hide the option, do it
        ::utl::OConfigurationTreeRoot aConfig( ::utl::OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(),
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.DataAccess/Policies/Features/Base" ) )
        ) );
        sal_Bool bAllowCreateLocalDatabase( sal_True );
        OSL_VERIFY( aConfig.getNodeValue( "CreateLocalDatabase" ) >>= bAllowCreateLocalDatabase );
        if ( !bAllowCreateLocalDatabase )
            bHideCreateNew = true;

        if ( bHideCreateNew )
        {
            m_aRB_CreateDatabase.Hide();
            Window* pWindowsToMove[] = {
                &m_aRB_OpenDocument, &m_aRB_GetExistingDatabase, &m_aFT_DocListLabel, m_pLB_DocumentList.get(),
                &m_aPB_OpenDocument, &m_aDatasourceTypeLabel, m_pDatasourceType.get(), &m_aFTDataSourceAppendix,
                &m_aTypePostLabel
            };
            const long nOffset = m_aRB_OpenDocument.GetPosPixel().Y() - m_aRB_CreateDatabase.GetPosPixel().Y();
            for ( size_t i=0; i < sizeof( pWindowsToMove ) / sizeof( pWindowsToMove[0] ); ++i )
            {
                Point aPos( pWindowsToMove[i]->GetPosPixel() );
                aPos.Y() -= nOffset;
                pWindowsToMove[i]->SetPosPixel( aPos );
            }
        }

        if ( bHideCreateNew )
            m_aRB_GetExistingDatabase.Check();
        else
            m_aRB_CreateDatabase.Check();

        // do some knittings
        m_pDatasourceType->SetSelectHdl(LINK(this, OGeneralPage, OnDatasourceTypeSelected));
           m_aRB_CreateDatabase.SetClickHdl(LINK(this, OGeneralPage, OnSetupModeSelected));
           m_aRB_GetExistingDatabase.SetClickHdl(LINK(this, OGeneralPage, OnSetupModeSelected));
           m_aRB_OpenDocument.SetClickHdl(LINK(this, OGeneralPage, OnSetupModeSelected));
        m_pLB_DocumentList->SetSelectHdl( LINK( this, OGeneralPage, OnDocumentSelected ) );
        m_aPB_OpenDocument.SetClickHdl( LINK( this, OGeneralPage, OnOpenDocument ) );
    }

    //-------------------------------------------------------------------------
    OGeneralPage::~OGeneralPage()
    {
        m_pDatasourceType.reset( NULL );
        m_pLB_DocumentList.reset( NULL );
    }

    //-------------------------------------------------------------------------
    namespace
    {
        struct DisplayedType
        {
            ::rtl::OUString eType;
            String          sDisplayName;

            DisplayedType( const ::rtl::OUString& _eType, const String& _rDisplayName ) : eType( _eType ), sDisplayName( _rDisplayName ) { }
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
                    const ::rtl::OUString sURLPrefix = aTypeLoop.getURLPrefix();
                    if ( !sURLPrefix.isEmpty() )
                    {
                        String sDisplayName = aTypeLoop.getDisplayName();
                        if (   m_pDatasourceType->GetEntryPos( sDisplayName ) == LISTBOX_ENTRY_NOTFOUND
                            && approveDataSourceType( sURLPrefix, sDisplayName ) )
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
    void OGeneralPage::setParentTitle(const ::rtl::OUString& _sURLPrefix)
    {
        if (!m_DBWizardMode)
        {
            const String sName = m_pCollection->getTypeDisplayName(_sURLPrefix);
            if ( m_pAdminDialog )
            {
                LocalResourceAccess aStringResAccess( PAGE_GENERAL, RSC_TABPAGE );
                String sMessage = String(ModuleRes(STR_PARENTTITLE));
                sMessage.SearchAndReplaceAscii("#",sName);
                m_pAdminDialog->setTitle(sMessage);
            }
        }
    }

    //-------------------------------------------------------------------------
    OGeneralPage::CreationMode OGeneralPage::GetDatabaseCreationMode() const
    {
        if ( m_aRB_CreateDatabase.IsChecked() )
            return eCreateNew;
        if ( m_aRB_GetExistingDatabase.IsChecked() )
            return eConnectExternal;
        return eOpenExisting;
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::GetFocus()
    {
        OGenericAdministrationPage::GetFocus();
        if ( m_pLB_DocumentList.get() && m_pLB_DocumentList->IsEnabled() )
            m_pLB_DocumentList->GrabFocus();
        else if (m_pDatasourceType.get() && m_pDatasourceType->IsEnabled())
            m_pDatasourceType->GrabFocus();
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::switchMessage(const ::rtl::OUString& _sURLPrefix)
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
            {
                LocalResourceAccess aStringResAccess( PAGE_GENERAL, RSC_TABPAGE );
                sMessage = String(ModuleRes(nResId));
            }
            m_aSpecialMessage.SetText(sMessage);

            m_eLastMessage = eMessage;
        }
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::onTypeSelected(const ::rtl::OUString& _sURLPrefix)
    {
        // the the new URL text as indicated by the selection history
        implSetCurrentType( _sURLPrefix );

        switchMessage(_sURLPrefix);

        if ( m_aTypeSelectHandler.IsSet() )
            m_aTypeSelectHandler.Call(this);
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        initializeTypeList();

        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);
        if (m_DBWizardMode)
        {
            m_aTypePreLabel.Hide();
            m_aTypePostLabel.Hide();
            m_aSpecialMessage.Hide();
            SetControlFontWeight(&m_aFTHeaderText);
            SetText(String());

            LayoutHelper::positionBelow( m_aRB_GetExistingDatabase, *m_pDatasourceType, RelatedControls, INDENT_BELOW_RADIO );

            if ( !bValid || bReadonly )
            {
                m_aDatasourceTypeLabel.Enable( false );
                m_pDatasourceType->Enable( false );
                m_aFTDataSourceAppendix.Enable( false );
                m_aPB_OpenDocument.Enable( false );
                m_aFT_DocListLabel.Enable( false );
                m_pLB_DocumentList->Enable( false );
            }
            else
            {
                m_aControlDependencies.enableOnRadioCheck( m_aRB_GetExistingDatabase, m_aDatasourceTypeLabel, *m_pDatasourceType, m_aFTDataSourceAppendix );
                m_aControlDependencies.enableOnRadioCheck( m_aRB_OpenDocument, m_aPB_OpenDocument, m_aFT_DocListLabel, *m_pLB_DocumentList );
            }

            m_pLB_DocumentList->SetDropDownLineCount( 20 );
            if ( m_pLB_DocumentList->GetEntryCount() )
                m_pLB_DocumentList->SelectEntryPos( 0 );

            m_aDatasourceTypeLabel.Hide();
            m_aFTDataSourceAppendix.Hide();

            m_eOriginalCreationMode = GetDatabaseCreationMode();
        }
        else
        {
            m_aFT_DatasourceTypeHeader.Hide();
            m_aRB_CreateDatabase.Hide();
            m_aRB_GetExistingDatabase.Hide();
            m_aRB_OpenDocument.Hide();
            m_aPB_OpenDocument.Hide();
            m_aFT_DocListLabel.Hide();
            m_pLB_DocumentList->Hide();
            m_aFTHeaderText.Hide();
            m_aFTHelpText.Hide();
            m_aTypePreLabel.Enable(bValid);
            m_aTypePostLabel.Enable(bValid);
            m_aDatasourceTypeLabel.Enable(bValid);
            m_pDatasourceType->Enable(bValid);
        }
        // if the selection is invalid, disable evrything
        String sName,sConnectURL;
        m_bDisplayingInvalid = !bValid;
        if ( bValid )
        {
            // collect some items and some values
            SFX_ITEMSET_GET(_rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True);
            SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
            OSL_ENSURE(pUrlItem, "OGeneralPage::implInitControls : missing the type attribute !");
            OSL_ENSURE(pNameItem, "OGeneralPage::implInitControls : missing the type attribute !");
            sName = pNameItem->GetValue();
            sConnectURL = pUrlItem->GetValue();
        }

        ::rtl::OUString eOldSelection = m_eCurrentSelection;
        m_eNotSupportedKnownType =  ::dbaccess::DST_UNKNOWN;
        implSetCurrentType(  ::rtl::OUString() );

        // compare the DSN prefix with the registered ones
        String sDisplayName;

        if (m_pCollection && bValid)
        {
            implSetCurrentType( m_pCollection->getPrefix(sConnectURL) );
            sDisplayName = m_pCollection->getTypeDisplayName(m_eCurrentSelection);
        }

        // select the correct datasource type
        if  (   approveDataSourceType( m_eCurrentSelection, sDisplayName )
            &&  ( LISTBOX_ENTRY_NOTFOUND == m_pDatasourceType->GetEntryPos( sDisplayName ) )
            )
        {   // this indicates it's really a type which is known in general, but not supported on the current platform
            // show a message saying so
            //  eSpecialMessage = smUnsupportedType;
            insertDatasourceTypeEntryData(m_eCurrentSelection, sDisplayName);
            // remember this type so we can show the special message again if the user selects this
            // type again (without changing the data source)
            m_eNotSupportedKnownType = m_pCollection->determineType(m_eCurrentSelection);
        }

        if (m_aRB_CreateDatabase.IsChecked() && m_DBWizardMode)
            sDisplayName = m_pCollection->getTypeDisplayName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jdbc:")));
        m_pDatasourceType->SelectEntry(sDisplayName);

        // notify our listener that our type selection has changed (if so)
        if ( eOldSelection != m_eCurrentSelection )
        {
            setParentTitle(m_eCurrentSelection);
            onTypeSelected(m_eCurrentSelection);
        }

        // a special message for the current page state
        switchMessage(m_eCurrentSelection);

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }


    // For the databaseWizard we only have one entry for the MySQL Database,
    // because we have a seperate tabpage to retrieve the respective datasource type
    // ( ::dbaccess::DST_MYSQL_ODBC ||  ::dbaccess::DST_MYSQL_JDBC). Therefore we use  ::dbaccess::DST_MYSQL_JDBC as a temporary
    // representative for all MySQl databases)
    // Also, embedded databases (embedded HSQL, at the moment), are not to appear in the list of
    // databases to connect to.
    bool OGeneralPage::approveDataSourceType( const ::rtl::OUString& _sURLPrefix, String& _inout_rDisplayName )
    {
        const ::dbaccess::DATASOURCE_TYPE eType = m_pCollection->determineType(_sURLPrefix);

        if ( m_DBWizardMode )
        {
            switch ( eType )
            {
            case ::dbaccess::DST_MYSQL_JDBC:
                _inout_rDisplayName = m_sMySQLEntry;
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
        }

        if ( eType == ::dbaccess::DST_MYSQL_NATIVE_DIRECT )
        {
            // do not display the Connector/OOo driver itself, it is always wrapped via the MySQL-Driver, if
            // this driver is installed
            if ( m_pCollection->hasDriver( "sdbc:mysql:mysqlc:" ) )
                _inout_rDisplayName = String();
        }

        if ( eType ==  ::dbaccess::DST_EMBEDDED_HSQLDB )
            _inout_rDisplayName = String();

        return _inout_rDisplayName.Len() > 0;
    }


    // -----------------------------------------------------------------------
    void OGeneralPage::insertDatasourceTypeEntryData(const ::rtl::OUString& _sType, String sDisplayName)
    {
        // insert a (temporary) entry
        sal_uInt16 nPos = m_pDatasourceType->InsertEntry(sDisplayName);
        if ( nPos >= m_aURLPrefixes.size() )
            m_aURLPrefixes.resize(nPos+1);
        m_aURLPrefixes[nPos] = _sType;
    }

    // -----------------------------------------------------------------------
    void OGeneralPage::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aTypePreLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aDatasourceTypeLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aTypePostLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aSpecialMessage));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFTDataSourceAppendix));
    }
    // -----------------------------------------------------------------------
    void OGeneralPage::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<ListBox>(m_pDatasourceType.get()));
    }

    //-------------------------------------------------------------------------
    SfxTabPage* OGeneralPage::Create(Window* _pParent, const SfxItemSet& _rAttrSet, sal_Bool _bWizardMode)
    {
           return ( new OGeneralPage( _pParent, _rAttrSet, _bWizardMode ) );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implSetCurrentType( const ::rtl::OUString& _eType )
    {
        if ( _eType == m_eCurrentSelection )
            return;

        m_eCurrentSelection = _eType;
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::Reset(const SfxItemSet& _rCoreAttrs)
    {
        // reset all locale data
        implSetCurrentType(  ::rtl::OUString() );
            // this ensures that our type selection link will be called, even if the new is is the same as the
            // current one
        OGenericAdministrationPage::Reset(_rCoreAttrs);
    }

    //-------------------------------------------------------------------------
    sal_Bool OGeneralPage::FillItemSet(SfxItemSet& _rCoreAttrs)
    {
        sal_Bool bChangedSomething = sal_False;

        bool bCommitTypeSelection = true;
        if ( m_DBWizardMode )
        {
            if ( m_aRB_CreateDatabase.IsChecked() )
            {
                _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:dbase:"))));
                bChangedSomething = sal_True;
                bCommitTypeSelection = false;
            }
            else if ( m_aRB_OpenDocument.IsChecked() )
            {
                if ( m_aRB_OpenDocument.GetSavedValue() != m_aRB_OpenDocument.IsChecked() )
                    bChangedSomething = sal_True;

                // TODO
                bCommitTypeSelection = false;
            }
        }

        if ( bCommitTypeSelection )
        {
            sal_uInt16 nEntry = m_pDatasourceType->GetSelectEntryPos();
            ::rtl::OUString sURLPrefix = m_aURLPrefixes[nEntry];
            if (m_DBWizardMode)
            {
                if  (  ( m_pDatasourceType->GetSavedValue() != nEntry )
                    || ( GetDatabaseCreationMode() != m_eOriginalCreationMode )
                    )
                {
                    _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL,sURLPrefix ));
                    bChangedSomething = sal_True;
                }
                else
                    implSetCurrentType(sURLPrefix);
            }
            else
            {
                if ( m_pDatasourceType->GetSavedValue() != nEntry)
                {
                    _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL, sURLPrefix));
                    bChangedSomething = sal_True;
                }
            }
        }
        return bChangedSomething;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnDatasourceTypeSelected, ListBox*, _pBox)
    {
        // get the type from the entry data
        sal_Int16 nSelected = _pBox->GetSelectEntryPos();
        const ::rtl::OUString sURLPrefix = m_aURLPrefixes[nSelected];

        setParentTitle(sURLPrefix);
        // let the impl method do all the stuff
        onTypeSelected(sURLPrefix);
        // tell the listener we were modified
        callModifiedHdl();
        // outta here
        return 0L;
    }

    //-------------------------------------------------------------------------
    OGeneralPage::DocumentDescriptor OGeneralPage::GetSelectedDocument() const
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
    IMPL_LINK(OGeneralPage, OnSetupModeSelected, RadioButton*, /*_pBox*/)
    {
        if ( m_aCreationModeHandler.IsSet() )
            m_aCreationModeHandler.Call(this);
        return 1L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnDocumentSelected, ListBox*, /*_pBox*/)
    {
        m_aDocumentSelectionHandler.Call( this );
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnOpenDocument, PushButton*, /*_pBox*/)
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION,
                0, rtl::OUString::createFromAscii("sdatabase") );
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
                m_aRB_GetExistingDatabase.Check();
                OnSetupModeSelected(&m_aRB_GetExistingDatabase);
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
