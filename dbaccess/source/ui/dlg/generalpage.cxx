/*************************************************************************
 *
 *  $RCSfile: generalpage.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: kz $ $Date: 2005-06-30 16:33:15 $
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

#ifndef _DBAUI_GENERALPAGE_HXX_
#include "generalpage.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef DBACCESS_SOURCE_UI_INC_DIALOGCONTROLLING_HXX
#include "dialogcontrolling.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
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
        ,m_aTypePreLabel                (this, ResId(FT_DATASOURCETYPE_PRE))
        ,m_aTypePostLabel               (this, ResId(FT_DATASOURCETYPE_POST))
        ,m_aSpecialMessage              (this, ResId(FT_SPECIAL_MESSAGE))
        ,m_aDatasourceTypeLabel         (this, ResId(FT_DATATYPE))
        ,m_aDatasourceType              (this, ResId(LB_DATATYPE))
        ,m_aFT_DatasourceTypeHeader     (this, ResId(FT_DATASOURCEHEADER))
        ,m_aRB_CreateDatabase           (this, ResId(RB_CREATEDBDATABASE))
        ,m_aRB_GetExistingDatabase      (this, ResId(RB_GETEXISTINGDATABASE))
        ,m_aRB_OpenDocument             (this, ResId(RB_OPENEXISTINGDOC))
        ,m_aFTHeaderText                (this, ResId(FT_GENERALHEADERTEXT))
        ,m_aFTHelpText                  (this, ResId(FT_GENERALHELPTEXT))
        ,m_aFTDataSourceAppendix        (this, ResId(FT_DATATYPEAPPENDIX))
        ,m_pLB_DocumentList             ( new OpenDocumentListBox( this, "com.sun.star.sdb.OfficeDatabaseDocument", ResId( LB_DOCUMENTLIST ) ) )
        ,m_aFT_DocListLabel             (this, ResId(FT_DOCLISTLABEL))
        ,m_aPB_OpenDocument             (this, "com.sun.star.sdb.OfficeDatabaseDocument", ResId(PB_OPENDOCUMENT))
        ,m_sMySQLEntry                  (ResId(STR_MYSQLENTRY))
        ,m_pCollection                  (NULL)
        ,m_eCurrentSelection            (DST_UNKNOWN)
        ,m_eNotSupportedKnownType       (DST_UNKNOWN)
        ,m_bDisplayingInvalid           (sal_False)
        ,m_bUserGrabFocus               (sal_True)
        ,m_eLastMessage                 (smNone)
        ,m_DBWizardMode                 (_bDBWizardMode)
    {
        // fill the listbox with the UI descriptions for the possible types
        // and remember the respective DSN prefixes
        FreeResource();
        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rItems.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();

        DBG_ASSERT(m_pCollection, "OGeneralPage::OGeneralPage : really need a DSN type collection !");

        // do some knittings
        m_aDatasourceType.SetSelectHdl(LINK(this, OGeneralPage, OnDatasourceTypeSelected));
           m_aRB_CreateDatabase.SetClickHdl(LINK(this, OGeneralPage, OnSetupModeSelected));
           m_aRB_GetExistingDatabase.SetClickHdl(LINK(this, OGeneralPage, OnSetupModeSelected));
           m_aRB_OpenDocument.SetClickHdl(LINK(this, OGeneralPage, OnSetupModeSelected));
        m_aRB_CreateDatabase.Check();

        m_pLB_DocumentList->SetSelectHdl( LINK( this, OGeneralPage, OnDocumentSelected ) );
        m_aPB_OpenDocument.SetClickHdl( LINK( this, OGeneralPage, OnOpenDocument ) );
    }

    //-------------------------------------------------------------------------
    OGeneralPage::~OGeneralPage()
    {
        m_pLB_DocumentList.reset( NULL );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::initializeTypeList()
    {
        m_aDatasourceType.Clear();

        Reference< XDriverAccess > xDriverManager;

        // get the driver manager, to ask it for all known URL prefixes
        DBG_ASSERT(m_xORB.is(), "OGeneralPage::initializeTypeList: have no service factory!");
        if (m_xORB.is())
        {
            {
                // if the connection pool (resp. driver manager) may be expensive to load if it is accessed the first time,
                // so display a wait cursor
                WaitObject aWaitCursor(GetParent());
                xDriverManager = Reference< XDriverAccess >(m_xORB->createInstance(SERVICE_SDBC_CONNECTIONPOOL), UNO_QUERY);
                if (!xDriverManager.is())
                    xDriverManager = Reference< XDriverAccess >(m_xORB->createInstance(SERVICE_SDBC_DRIVERMANAGER), UNO_QUERY);
            }
            if (!xDriverManager.is())
                ShowServiceNotAvailableError(GetParent(), String(SERVICE_SDBC_DRIVERMANAGER), sal_True);
        }

        if ( m_pCollection )
        {
            for (   ODsnTypeCollection::TypeIterator aTypeLoop =  m_pCollection->begin();
                    aTypeLoop != m_pCollection->end();
                    ++aTypeLoop
                )
            {
                DATASOURCE_TYPE eType = aTypeLoop.getType();

                if ( xDriverManager.is() )
                {   // we have a driver manager to check
                    ::rtl::OUString sURLPrefix = m_pCollection->getDatasourcePrefix(eType);
                    if (!xDriverManager->getDriverByURL(sURLPrefix).is())
                        // we have no driver for this prefix
                        // -> omit it
                        continue;
                }
                String sDisplayName = aTypeLoop.getDisplayName();
                if ( m_aDatasourceType.GetEntryPos( sDisplayName ) == LISTBOX_ENTRY_NOTFOUND )
                {
                    sDisplayName = VerifyDisplayName(eType, sDisplayName);
                    if (sDisplayName.Len() > 0)
                        insertDatasourceTypeEntryData(eType, sDisplayName);
                }
            }
        }
    }



    //-------------------------------------------------------------------------
    void OGeneralPage::initializeHistory()
    {
        m_aSelectionHistory.clear();
        if (m_pCollection)
        {
            for (   ODsnTypeCollection::TypeIterator aTypeLoop =  m_pCollection->begin();
                    aTypeLoop != m_pCollection->end();
                    ++aTypeLoop
                )
                m_aSelectionHistory[aTypeLoop.getType()] = m_pCollection->getDatasourcePrefix(aTypeLoop.getType());
        }

    }
    //-------------------------------------------------------------------------
    void OGeneralPage::setParentTitle(DATASOURCE_TYPE _eSelectedType)
    {
        if (!m_DBWizardMode)
        {
            String sName = m_pCollection->getTypeDisplayName(_eSelectedType);
            Dialog* pDialog = reinterpret_cast<Dialog*>(m_pAdminDialog);
            if ( m_pAdminDialog )
            {
                OLocalResourceAccess aStringResAccess( PAGE_GENERAL, RSC_TABPAGE );
                String sMessage = String(ResId(STR_PARENTTITLE));
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
        if (m_aDatasourceType.IsEnabled())
            m_aDatasourceType.GrabFocus();
        if ( m_pLB_DocumentList.get() && m_pLB_DocumentList->IsEnabled() )
            m_pLB_DocumentList->GrabFocus();
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::switchMessage(sal_Bool _bDeleted,const DATASOURCE_TYPE _eType)
    {
        SPECIAL_MESSAGE eMessage = smNone;
        if ( _eType == m_eNotSupportedKnownType )
        {
            eMessage = smUnsupportedType;
        }


        if ( eMessage != m_eLastMessage )
        {
            sal_uInt16 nResId = 0;
            switch (eMessage)
            {
                case smUnsupportedType:     nResId = STR_UNSUPPORTED_DATASOURCE_TYPE; break;
            }
            String sMessage;
            if ( nResId )
            {
                OLocalResourceAccess aStringResAccess( PAGE_GENERAL, RSC_TABPAGE );
                sMessage = String(ResId(nResId));
            }
            m_aSpecialMessage.SetText(sMessage);

            m_eLastMessage = eMessage;
        }
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::onTypeSelected(const DATASOURCE_TYPE _eType)
    {
        // the the new URL text as indicated by the selection history
        implSetCurrentType( _eType );

        switchMessage(sal_False,_eType);

        if ( m_aTypeSelectHandler.IsSet() )
            m_aTypeSelectHandler.Call(this);
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        initializeHistory();
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

            m_aDatasourceType.SetPosPixel( MovePoint( m_aRB_GetExistingDatabase.GetPosPixel(), INDENT_BELOW_RADIO, 14 ) );

            m_pSelectTypeController.reset( new RadioDependentEnabler( m_aRB_GetExistingDatabase, !bValid || bReadonly ) );
            m_pSelectTypeController->addDependentWindow( m_aDatasourceTypeLabel );
            m_pSelectTypeController->addDependentWindow( m_aDatasourceType );
            m_pSelectTypeController->addDependentWindow( m_aFTDataSourceAppendix );

            m_pOpenDocController.reset( new RadioDependentEnabler( m_aRB_OpenDocument, !bValid || bReadonly ) );
            m_pOpenDocController->addDependentWindow( m_aPB_OpenDocument );
            m_pOpenDocController->addDependentWindow( m_aFT_DocListLabel );
            m_pOpenDocController->addDependentWindow( *m_pLB_DocumentList );

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
            m_aFTDataSourceAppendix.Enable(bValid);
            m_aDatasourceType.Enable(bValid);
        }
        // if the selection is invalid, disable evrything
        String sName,sConnectURL;
        sal_Bool bDeleted = sal_False;
        m_bDisplayingInvalid = !bValid;
        if ( bValid )
        {
            // collect some items and some values
            SFX_ITEMSET_GET(_rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True);
            SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
            DBG_ASSERT(pUrlItem, "OGeneralPage::implInitControls : missing the type attribute !");
            DBG_ASSERT(pNameItem, "OGeneralPage::implInitControls : missing the type attribute !");
            sName = pNameItem->GetValue();
            sConnectURL = pUrlItem->GetValue();
        }

        DATASOURCE_TYPE eOldSelection = m_eCurrentSelection;
        m_eNotSupportedKnownType = DST_UNKNOWN;
        implSetCurrentType( DST_UNKNOWN );

        // compare the DSN prefix with the registered ones
        String sDisplayName;

        if (m_pCollection && bValid)
        {
            implSetCurrentType( m_pCollection->getType(sConnectURL) );
            sDisplayName = m_pCollection->getTypeDisplayName(m_eCurrentSelection);
        }
        sDisplayName = VerifyDisplayName(m_eCurrentSelection, sDisplayName);
        // select the correct datasource type
        if (LISTBOX_ENTRY_NOTFOUND == m_aDatasourceType.GetEntryPos(sDisplayName))
        {   // the type is not available on this platform (we omitted it in initializeTypeList)
            if (sDisplayName.Len())
            {   // this indicates it's really a type which is known in general, but not supported on the current platform
                // show a message saying so
                //  eSpecialMessage = smUnsupportedType;
                insertDatasourceTypeEntryData(m_eCurrentSelection, sDisplayName);
                // remember this type so we can show the special message again if the user selects this
                // type again (without changing the data source)
                m_eNotSupportedKnownType = m_eCurrentSelection;
            }
        }
        if (m_aRB_CreateDatabase.IsChecked() && m_DBWizardMode)
            sDisplayName = m_pCollection->getTypeDisplayName(DST_JDBC);
        m_aDatasourceType.SelectEntry(sDisplayName);

        // notify our listener that our type selection has changed (if so)
        if ( eOldSelection != m_eCurrentSelection )
        {
            setParentTitle(m_eCurrentSelection);
            onTypeSelected(m_eCurrentSelection);
        }

        // a special message for the current page state
        switchMessage(bDeleted,m_eCurrentSelection);

        OGenericAdministrationPage::implInitControls(_rSet, _bSaveValue);
    }


    // For the databaseWizard we only have one entry for the MySQL Database,
    // because we have a seperate tabpage to retrieve the respective datasource type
    //(DST_MYSQL_ODBC || DST_MYSQL_JDBC) Therefor we use DST_MYSQL_JDBC as a temporary
    // representative for all MySQl databases)
    String OGeneralPage::VerifyDisplayName(DATASOURCE_TYPE eType, String _sDisplayName)
    {
        String sLocDisplayName =  ::rtl::OUString::createFromAscii("");
        if ((!m_DBWizardMode) || (eType != DST_MYSQL_ODBC))
        {
            if ((m_DBWizardMode) && (eType == DST_MYSQL_JDBC))
                sLocDisplayName = m_sMySQLEntry;
            else
                sLocDisplayName = _sDisplayName;
        }
        return sLocDisplayName;
    }


    // -----------------------------------------------------------------------
    void OGeneralPage::insertDatasourceTypeEntryData(DATASOURCE_TYPE _eType, String sDisplayName)
    {
        // insert a (temporary) entry
        sal_uInt16 nPos = m_aDatasourceType.InsertEntry(sDisplayName);
        m_aDatasourceType.SetEntryData(nPos, reinterpret_cast<void*>(_eType));
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
        _rControlList.push_back(new OSaveValueWrapper<ListBox>(&m_aDatasourceType));
    }

    //-------------------------------------------------------------------------
    SfxTabPage* OGeneralPage::Create(Window* _pParent, const SfxItemSet& _rAttrSet, sal_Bool _bWizardMode)
    {
           return ( new OGeneralPage( _pParent, _rAttrSet, _bWizardMode ) );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implSetCurrentType( const DATASOURCE_TYPE _eType )
    {
        if ( _eType == m_eCurrentSelection )
            return;

        m_eCurrentSelection = _eType;
        ::std::vector< sal_Int32> aDetailIds;
        ODriversSettings::fillDetailIds(m_eCurrentSelection,aDetailIds);
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::Reset(const SfxItemSet& _rCoreAttrs)
    {
        // reset all locale data
        implSetCurrentType( DST_UNKNOWN );
            // this ensures that our type selection link will be called, even if the new is is the same as the
            // current one
        OGenericAdministrationPage::Reset(_rCoreAttrs);
    }

    //-------------------------------------------------------------------------
    BOOL OGeneralPage::FillItemSet(SfxItemSet& _rCoreAttrs)
    {
        sal_Bool bChangedSomething = sal_False;

        bool bCommitTypeSelection = true;
        if ( m_DBWizardMode )
        {
            if ( m_aRB_CreateDatabase.IsChecked() )
            {
                _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getDatasourcePrefix(DST_DBASE)));
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
            USHORT nEntry = m_aDatasourceType.GetSelectEntryPos();
            DATASOURCE_TYPE eSelectedType = static_cast<DATASOURCE_TYPE>(reinterpret_cast<sal_Int32>(m_aDatasourceType.GetEntryData(nEntry)));
            if (m_DBWizardMode)
            {
                if  (  ( m_aDatasourceType.GetSavedValue() != nEntry )
                    || ( GetDatabaseCreationMode() != m_eOriginalCreationMode )
                    )
                {
                    _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getDatasourcePrefix(eSelectedType)));
                    bChangedSomething = sal_True;
                }
                else
                    implSetCurrentType(eSelectedType);
            }
            else
            {
                if ( m_aDatasourceType.GetSavedValue() != nEntry)
                {
                    _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getDatasourcePrefix(eSelectedType)));
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
        DATASOURCE_TYPE eSelectedType = static_cast<DATASOURCE_TYPE>(reinterpret_cast<sal_Int32>(_pBox->GetEntryData(nSelected)));

        setParentTitle(eSelectedType);
        // let the impl method do all the stuff
        onTypeSelected(eSelectedType);
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
    IMPL_LINK(OGeneralPage, OnSetupModeSelected, RadioButton*, _pBox)
    {
        // TODO: nearly all of this could be outsourced to a IWindowOperator, and
        // triggered via a DialogController, couldn't it?
//      String sDisplayName;
//        if (m_aRB_CreateDatabase.IsChecked())
//          sDisplayName = m_pCollection->getTypeDisplayName(DST_DBASE);
//      else
//          sDisplayName = m_pCollection->getTypeDisplayName(m_eCurrentSelection);
//      USHORT n = m_aDatasourceType.GetEntryPos(sDisplayName);
//      if ((n < m_aDatasourceType.GetEntryCount()) && (n >= 0))
//          m_aDatasourceType.SelectEntry(sDisplayName);
//      else
//          m_aDatasourceType.SelectEntry(m_sMySQLEntry);
        if ( m_aCreationModeHandler.IsSet() )
            m_aCreationModeHandler.Call(this);
        return 1L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnDocumentSelected, ListBox*, _pBox)
    {
        m_aDocumentSelectionHandler.Call( this );
        return 0L;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnOpenDocument, PushButton*, _pBox)
    {
        ::sfx2::FileDialogHelper aFileDlg( WB_OPEN, ::String::CreateFromAscii("sdatabase") );
        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            m_aBrowsedDocument.sURL = aFileDlg.GetPath();
            m_aBrowsedDocument.sFilter = String();
            m_aChooseDocumentHandler.Call( this );
            return 1L;
        }

        return 0L;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

