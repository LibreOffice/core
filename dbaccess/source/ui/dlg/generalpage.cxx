/*************************************************************************
 *
 *  $RCSfile: generalpage.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:24 $
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

#ifndef _DBAUI_TABLESPAGE_HXX_
#include "tablespage.hxx"
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
#ifndef _DBAUI_FILENOTATION_HXX_
#include "filenotation.hxx"
#endif
#ifndef _DBAUI_DSSELECT_HXX_
#include "dsselect.hxx"
#endif
#ifndef _DBAUI_ODBC_CONFIG_HXX_
#include "odbcconfig.hxx"
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
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
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBAUI_DETAILPAGES_HXX_
#include "detailpages.hxx"
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCREATECATALOG_HPP_
#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
// #106016# ------------------------------------
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::i18n;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;
    using namespace ::dbtools;
    using namespace ::svt;

    //=========================================================================
    //= OGeneralPage
    //=========================================================================
    //-------------------------------------------------------------------------
    OGeneralPage::OGeneralPage(Window* pParent, const SfxItemSet& _rItems)
        :OGenericAdministrationPage(pParent, ModuleRes(PAGE_GENERAL), _rItems)
        ,m_aNameLabel           (this, ResId(FT_DATASOURCENAME))
        ,m_aName                (this, ResId(ET_DATASOURCENAME))
        ,m_aTypeBox             (this, ResId(FL_SEPARATOR1))
        ,m_aDatasourceTypeLabel (this, ResId(FT_DATATYPE))
        ,m_aDatasourceType      (this, ResId(LB_DATATYPE))
        ,m_aConnectionLabel     (this, ResId(FT_CONNECTURL))
        ,m_aConnection          (this, ResId(ET_CONNECTURL))
        ,m_aBrowseConnection    (this, ResId(PB_BROWSECONNECTION))
        ,m_aCreateDatabase      (this, ResId(PB_CREATEDATABASE))
//      ,m_aTimeoutLabel        (this, ResId(FT_LOGINTIMEOUT))
//      ,m_aTimeoutNumber       (this, ResId(ET_TIMEOUT_NUMBER))
//      ,m_aTimeoutUnit         (this, ResId(LB_TIMEOUT_UNIT))
        ,m_aSpecialMessage      (this, ResId(FT_SPECIAL_MESSAGE))
        ,m_pCollection(NULL)
        ,m_eCurrentSelection(DST_UNKNOWN)
        ,m_eNotSupportedKnownType(DST_UNKNOWN)
        ,m_bDisplayingInvalid(sal_False)
        ,m_pAdminDialog(NULL)
        ,m_nCacheSize(0)
        ,m_bUserGrabFocus(sal_True)
        ,m_eLastMessage(smNone)
    {
        // fill the listbox with the UI descriptions for the possible types
        // and remember the respective DSN prefixes
        FreeResource();

        // first we hide this buton
        m_aCreateDatabase.Hide();

        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rItems.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();

        DBG_ASSERT(m_pCollection, "OGeneralPage::OGeneralPage : really need a DSN type collection !");

        // do some knittings
        m_aDatasourceType.SetSelectHdl(LINK(this, OGeneralPage, OnDatasourceTypeSelected));
        m_aName.SetModifyHdl(LINK(this, OGeneralPage, OnNameModified));
        m_aConnection.SetModifyHdl(getControlModifiedLink());
        m_aBrowseConnection.SetClickHdl(LINK(this, OGeneralPage, OnBrowseConnections));
        m_aCreateDatabase.SetClickHdl(LINK(this, OGeneralPage, OnCreateDatabase));
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implSetURL( const String& _rURL, sal_Bool _bPrefix )
    {
        String sURL( _rURL );
        DBG_ASSERT( m_pCollection, "OGeneralPage::setURL: have no interpreter for the URLs!" );

        if ( m_pCollection && sURL.Len() )
        {
            // determine the type
            DATASOURCE_TYPE eType = m_pCollection->getType( _bPrefix ? sURL : m_aConnection.GetText() );

            if ( m_pCollection->isFileSystemBased( eType ) )
            {
                // get the tow parts: prefix and file URL
                String sTypePrefix, sFileURLEncoded;
                if ( _bPrefix )
                {
                    sTypePrefix = m_pCollection->getDatasourcePrefix( eType );
                    sFileURLEncoded = m_pCollection->cutPrefix( sURL );
                }
                else
                {
                    sFileURLEncoded = sURL;
                }

                // substitute any variables
                sFileURLEncoded = SvtPathOptions().SubstituteVariable( sFileURLEncoded );

                // decode the URL
                INetURLObject aFileURL( sFileURLEncoded, INetURLObject::WAS_ENCODED, RTL_TEXTENCODING_UTF8 );
                String sFileURLDecoded = aFileURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );

                // set this decoded URL as text
                sURL = sTypePrefix;
                sURL += sFileURLDecoded;
            }
        }

        if ( _bPrefix )
            m_aConnection.SetText( sURL );
        else
            m_aConnection.SetTextNoPrefix( sURL );
    }

    //-------------------------------------------------------------------------
    String OGeneralPage::implGetURL( sal_Bool _bPrefix ) const
    {
        // get the pure text
        String sURL = _bPrefix ? m_aConnection.GetText() : m_aConnection.GetTextNoPrefix();

        DBG_ASSERT( m_pCollection, "OGeneralPage::implGetURL: have no interpreter for the URLs!" );

        if ( m_pCollection && sURL.Len() )
        {
            // determine the type
            DATASOURCE_TYPE eType = m_pCollection->getType( _bPrefix ? sURL : m_aConnection.GetText() );
            if ( m_pCollection->isFileSystemBased( eType ) )
            {
                // get the tow parts: prefix and file URL
                String sTypePrefix, sFileURLDecoded;
                if ( _bPrefix )
                {
                    sTypePrefix = m_pCollection->getDatasourcePrefix( eType );
                    sFileURLDecoded = m_pCollection->cutPrefix( sURL );
                }
                else
                {
                    sFileURLDecoded = sURL;
                }

                // encode the URL
                INetURLObject aFileURL( sFileURLDecoded, INetURLObject::ENCODE_ALL, RTL_TEXTENCODING_UTF8 );
                String sFileURLEncoded = aFileURL.GetMainURL( INetURLObject::NO_DECODE );

                // return this encoded URL
                sURL = sTypePrefix;
                sURL += sFileURLEncoded;
            }
        }
        return sURL;
    }

    //-------------------------------------------------------------------------
    String OGeneralPage::getURL( ) const
    {
        return implGetURL( sal_True );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::setURL( const String& _rURL )
    {
        implSetURL( _rURL, sal_True );
    }

    //-------------------------------------------------------------------------
    String OGeneralPage::getURLNoPrefix( ) const
    {
        return implGetURL( sal_False );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::setURLNoPrefix( const String& _rURL )
    {
        implSetURL( _rURL, sal_False );
    }

    //-------------------------------------------------------------------------
    String OGeneralPage::getConnectionURL( ) const
    {
        return getURL( );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::changeConnectionURL( const String& _rNewDSN )
    {
        DBG_ASSERT( m_pCollection && ( m_eCurrentSelection == m_pCollection->getType( _rNewDSN ) ),
            "OGeneralPage::changeConnectionURL: invalid new DSN!" );
        setURL( _rNewDSN );
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
                    sal_Int32 nPos = m_aDatasourceType.InsertEntry(sDisplayName);
                    m_aDatasourceType.SetEntryData((sal_uInt16)nPos, reinterpret_cast<void*>(eType));
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
    void OGeneralPage::GetFocus()
    {
        OGenericAdministrationPage::GetFocus();
        if (m_aName.IsEnabled())
            m_aName.GrabFocus();
    }

    //-------------------------------------------------------------------------
    sal_Bool OGeneralPage::isBrowseable(DATASOURCE_TYPE _eType) const
    {
        switch (_eType)
        {
            case DST_DBASE:
            case DST_TEXT:
            case DST_ADABAS:
            case DST_ODBC:
            case DST_CALC:
            case DST_ADDRESSBOOK:
                return sal_True;
//          case DST_MYSQL:
//              {
//                  const SfxItemSet& rItemSet = GetItemSet();
//                  SFX_ITEMSET_GET(rItemSet, pURL, SfxStringItem, DSID_CONNECTURL, sal_True);
//                  return pURL->GetValue().EqualsIgnoreCaseAscii("sdbc:mysql:odbc:",0,16);
//              }

        }
        return sal_False;
    }
    //-------------------------------------------------------------------------
    void OGeneralPage::checkCreateDatabase(DATASOURCE_TYPE _eType)
    {
        static BOOL bServiceFound = FALSE;
        BOOL bInstallationFound = FALSE;
        OSL_ENSURE(m_pAdminDialog,"No parent set!");
        if ( _eType == DST_ADABAS && m_pAdminDialog && !bServiceFound )
        {
            Reference<XCreateCatalog> xCatalog(m_xORB->createInstance(SERVICE_EXTENDED_ADABAS_DRIVER),UNO_QUERY);
            bServiceFound = xCatalog.is();
        }

        m_aCreateDatabase.Show(_eType == DST_ADABAS && bServiceFound);

        if ( bServiceFound )
        {
            static const ::rtl::OUString sDBWORK(RTL_CONSTASCII_USTRINGPARAM("DBWORK"));
            static const ::rtl::OUString sDBROOT(RTL_CONSTASCII_USTRINGPARAM("DBROOT"));
            static const ::rtl::OUString sDBCONFIG(RTL_CONSTASCII_USTRINGPARAM("DBCONFIG"));
            rtl_uString* pDbVar = NULL;
            if ( (osl_getEnvironment(sDBWORK.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                && ((pDbVar = NULL) == NULL && osl_getEnvironment(sDBROOT.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                && ((pDbVar = NULL) == NULL && osl_getEnvironment(sDBCONFIG.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                )
                bInstallationFound = TRUE;

            m_aCreateDatabase.Enable( bInstallationFound );
        }
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::switchMessage(sal_Bool _bDeleted,const DATASOURCE_TYPE _eType)
    {
        SPECIAL_MESSAGE eMessage = smNone;
        if ( _bDeleted )
        {
            eMessage = smDatasourceDeleted;
        }
        else if ( _eType == m_eNotSupportedKnownType )
        {
            eMessage = smUnsupportedType;
        }
        else if ( m_aName.IsEnabled() && ( 0L == m_aNameValidator.Call( this ) ) )
        {
            eMessage = smInvalidName;
        }
        else if ( (_eType == DST_MYSQL_ODBC) || (_eType == DST_MYSQL_JDBC) )
        {
            eMessage = smMySQL;
        }


        if ( eMessage != m_eLastMessage )
        {
            sal_uInt16 nResId = 0;
            switch (eMessage)
            {
                case smInvalidName:         nResId = STR_NAMEINVALID; break;
                case smDatasourceDeleted:   nResId = STR_DATASOURCEDELETED; break;
                case smUnsupportedType:     nResId = STR_UNSUPPORTED_DATASOURCE_TYPE; break;
                case smMySQL:               nResId = STR_MYSQL_CONFIG_NEXT_PAGE; break;
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
        m_aBrowseConnection.Enable(isBrowseable(_eType));
        checkCreateDatabase(_eType);

        // the the new URL text as indicated by the selection history
        implSetCurrentType( _eType );

        m_aConnection.Enable( (_eType != DST_MYSQL_ODBC) && (_eType != DST_MYSQL_JDBC) );

        switchMessage(sal_False,_eType);

        if ( m_aTypeSelectHandler.IsSet() )
            m_aTypeSelectHandler.Call(this);
    }

    //-------------------------------------------------------------------------
    sal_Bool OGeneralPage::checkItems()
    {
        if ((0 == m_aName.GetText().Len()) && !m_bDisplayingInvalid)
        {
            String sErrorMsg(ModuleRes(STR_ERR_EMPTY_DSN_NAME));
            ErrorBox aErrorBox(GetParent(), WB_OK, sErrorMsg);
            aErrorBox.Execute();
            m_aName.GrabFocus();
            return sal_False;
        }

        if (!commitURL())
            return sal_False;

        return sal_True;
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        initializeHistory();
        initializeTypeList();

        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        // if the selection is invalid, disable evrything
        m_aNameLabel.Enable(bValid);
        m_aName.Enable(bValid);
        m_aTypeBox.Enable(bValid);
        m_aDatasourceTypeLabel.Enable(bValid);
        m_aDatasourceType.Enable(bValid);
        m_aConnectionLabel.Enable(bValid);
        m_aConnection.Enable(bValid);
        m_aBrowseConnection.Enable(bValid);

        String sConnectURL, sName;
        sal_Bool bDeleted = sal_False;
        m_bDisplayingInvalid = !bValid;
        if (bValid)
        {
            // collect some items and some values
            SFX_ITEMSET_GET(_rSet, pUrlItem, SfxStringItem, DSID_CONNECTURL, sal_True);
            SFX_ITEMSET_GET(_rSet, pNameItem, SfxStringItem, DSID_NAME, sal_True);
            DBG_ASSERT(pUrlItem, "OGeneralPage::implInitControls : missing the type attribute !");
            DBG_ASSERT(pNameItem, "OGeneralPage::implInitControls : missing the name attribute !");
            sConnectURL = pUrlItem->GetValue();
            sName = pNameItem->GetValue();
        }
        else
        {
            SFX_ITEMSET_GET(_rSet, pDeleted, SfxBoolItem, DSID_DELETEDDATASOURCE, sal_True);
            bDeleted = pDeleted && pDeleted->GetValue();
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

        m_aBrowseConnection.Enable(bValid && isBrowseable(m_eCurrentSelection));
        m_aConnection.Enable(bValid && (m_eCurrentSelection != DST_MYSQL_ODBC) && (m_eCurrentSelection != DST_MYSQL_JDBC) );
        checkCreateDatabase(m_eCurrentSelection);

        // select the correct datasource type
        if (LISTBOX_ENTRY_NOTFOUND == m_aDatasourceType.GetEntryPos(sDisplayName))
        {   // the type is not available on this platform (we omitted it in initializeTypeList)
            if (sDisplayName.Len())
            {   // this indicates it's really a type which is known in general, but not supported on the current platform
                // show a message saying so
                //  eSpecialMessage = smUnsupportedType;

                // insert a (temporary) entry
                sal_uInt16 nPos = m_aDatasourceType.InsertEntry(sDisplayName);
                m_aDatasourceType.SetEntryData(nPos, reinterpret_cast<void*>(m_eCurrentSelection));

                // remember this type so we can show the special message again if the user selects this
                // type again (without changing the data source)
                m_eNotSupportedKnownType = m_eCurrentSelection;
            }
        }
        m_aDatasourceType.SelectEntry(sDisplayName);
        if (_bSaveValue)
            m_aDatasourceType.SaveValue();

        setURL(sConnectURL);

        // notify our listener that our type selection has changed (if so)
        sal_Bool bTypeChanged = eOldSelection != m_eCurrentSelection;
        if ( m_pCollection && bTypeChanged )
            bTypeChanged = !m_pCollection->areTypesRelated(eOldSelection,m_eCurrentSelection);

        if ( bTypeChanged )
            onTypeSelected(m_eCurrentSelection);

        // are we allowed to change the data source type?
        sal_Bool bSingleTypeEdit = ODbAdminDialog::omSingleEditFixedType == m_pAdminDialog->getMode();
        // is the current data source type "AddressBook"?
        sal_Bool bBrowseNotAllowed = (  DST_ADDRESSBOOK == m_eCurrentSelection
                                    ||  DST_MYSQL_ODBC == m_eCurrentSelection
                                    ||  DST_MYSQL_JDBC == m_eCurrentSelection);

        // don't allow sub-type changes in case of the address book in single-edit mode
        if ( bSingleTypeEdit && bBrowseNotAllowed )
            m_aBrowseConnection.Disable( );

        if (_bSaveValue)
        {
            m_aConnection.SaveValue();
            m_aConnection.SaveValueNoPrefix();
        }

        // the datasource name
        m_aName.SetText(sName);
        if (_bSaveValue)
            m_aName.SaveValue();

//      // is the very current name valid?
//      if ( m_aName.IsEnabled() && ( 0L == m_aNameValidator.Call( this ) ) )
//          eSpecialMessage = smInvalidName;
//
//      if ( eSpecialMessage == smNone && (DST_MYSQL_ODBC == m_eCurrentSelection || DST_MYSQL_JDBC == m_eCurrentSelection) )
//          eSpecialMessage = smMySQL;

        // a special message for the current page state
        switchMessage(bDeleted,m_eCurrentSelection);
    }

    //-------------------------------------------------------------------------
    SfxTabPage* OGeneralPage::Create(Window* _pParent, const SfxItemSet& _rAttrSet)
    {
        return ( new OGeneralPage( _pParent, _rAttrSet ) );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::implSetCurrentType( const DATASOURCE_TYPE _eType )
    {
        if ( _eType == m_eCurrentSelection )
            return;

        m_aSelectionHistory[ m_eCurrentSelection ] = getURL();
        m_eCurrentSelection = _eType;
        setURL( m_aSelectionHistory[ m_eCurrentSelection ] );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::Reset(const SfxItemSet& _rCoreAttrs)
    {
        // reset all locale data
        m_sControlPassword = m_sControlUser = m_sUserPassword = m_sUser = String();
        m_nCacheSize = 0;

        implSetCurrentType( DST_UNKNOWN );
            // this ensures that our type selection link will be called, even if the new is is the same as the
            // current one
        OGenericAdministrationPage::Reset(_rCoreAttrs);

        // there are some things which depend on the current name
        OnNameModified( &m_aName );
    }

    //-------------------------------------------------------------------------
    void OGeneralPage::ActivatePage(const SfxItemSet& _rSet)
    {
        OGenericAdministrationPage::ActivatePage( _rSet );

        // if the dialog is in a mode where only a single data source is beeing edited, and no type change
        // is allowed, disable the type selection
        sal_Bool bSingleTypeEdit = ODbAdminDialog::omSingleEditFixedType == m_pAdminDialog->getMode();
        m_aDatasourceType.Enable( !bSingleTypeEdit );
    }

    //-------------------------------------------------------------------------
    BOOL OGeneralPage::FillItemSet(SfxItemSet& _rCoreAttrs)
    {
        sal_Bool bChangedSomething = sal_False;
        if (m_aName.GetText() != m_aName.GetSavedValue())
        {
            _rCoreAttrs.Put(SfxStringItem(DSID_NAME, m_aName.GetText()));
            bChangedSomething = sal_True;
        }

        if ((getURL() != m_aConnection.GetSavedValue()) || (m_aDatasourceType.GetSavedValue() != m_aDatasourceType.GetSelectEntryPos()))
        {
            _rCoreAttrs.Put(SfxStringItem(DSID_CONNECTURL, getURL()));
            bChangedSomething = sal_True;
        }
        if(m_sControlUser.Len())
        {
            _rCoreAttrs.Put(SfxStringItem(DSID_CONN_CTRLUSER, m_sControlUser));
            bChangedSomething = sal_True;
        }
        if(m_sControlPassword.Len())
        {
            _rCoreAttrs.Put(SfxStringItem(DSID_CONN_CTRLPWD, m_sControlPassword));
            bChangedSomething = sal_True;
        }
        if(m_sUser.Len())
        {
            _rCoreAttrs.Put(SfxStringItem(DSID_USER, m_sUser));
            _rCoreAttrs.Put(SfxBoolItem(DSID_PASSWORDREQUIRED, sal_True));
            bChangedSomething = sal_True;
        }
        if(m_sUserPassword.Len())
        {
            _rCoreAttrs.Put(SfxStringItem(DSID_PASSWORD, m_sUserPassword));
            bChangedSomething = sal_True;
        }
        if(m_nCacheSize)
        {
            _rCoreAttrs.Put(SfxInt32Item(DSID_CONN_CACHESIZE, m_nCacheSize));
            bChangedSomething = sal_True;
        }


        return bChangedSomething;
    }

    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnNameModified, Edit*, _pBox)
    {
        sal_Bool bNewNameValid = sal_True;

        if (m_aNameModifiedHandler.IsSet())
            bNewNameValid = (0L != m_aNameModifiedHandler.Call(this));

        if ( m_aName.IsEnabled() )
            switchMessage(sal_False, m_eCurrentSelection);

        return 0L;
    }
    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnCreateDatabase, PushButton*, _pButton)
    {
        switch (GetSelectedType())
        {
            case DST_ADABAS:
                {
                    OSL_ENSURE(m_pAdminDialog,"Parent not set!");
                    try
                    {
                        Reference<XCreateCatalog> xCatalog(m_xORB->createInstance(SERVICE_EXTENDED_ADABAS_DRIVER),UNO_QUERY);
                        if(xCatalog.is())
                        {
                            Sequence< Any > aArgs(2);
                            aArgs[0] <<= PropertyValue(::rtl::OUString::createFromAscii("CreateCatalog"), 0,makeAny(xCatalog) , PropertyState_DIRECT_VALUE);
                            aArgs[1] <<= PropertyValue(PROPERTY_PARENTWINDOW, 0, makeAny(VCLUnoHelper::GetInterface(this)), PropertyState_DIRECT_VALUE);

                            Reference< XExecutableDialog > xDialog(
                                m_xORB->createInstanceWithArguments(SERVICE_SDB_ADABASCREATIONDIALOG, aArgs), UNO_QUERY);
                            if (!xDialog.is())
                            {
                                ShowServiceNotAvailableError(GetParent(), String(SERVICE_SDB_ADABASCREATIONDIALOG), sal_True);
                                break;
                            }

                            if(xDialog->execute() == RET_OK)
                            {
                                Reference<XPropertySet> xProp(xDialog,UNO_QUERY);
                                if(xProp.is())
                                {
                                    Reference<XPropertySetInfo> xPropInfo(xProp->getPropertySetInfo());
                                    if(xPropInfo->hasPropertyByName(PROPERTY_DATABASENAME))
                                    {
                                        String sDatabaseName;
                                        sDatabaseName = String(::comphelper::getString(xProp->getPropertyValue(PROPERTY_DATABASENAME)));
                                        setURLNoPrefix(sDatabaseName);
                                        callModifiedHdl();
                                    }
                                    if(xPropInfo->hasPropertyByName(PROPERTY_CONTROLUSER))
                                        m_sControlUser = ::comphelper::getString(xProp->getPropertyValue(PROPERTY_CONTROLUSER));
                                    if(xPropInfo->hasPropertyByName(PROPERTY_CONTROLPASSWORD))
                                        m_sControlPassword = ::comphelper::getString(xProp->getPropertyValue(PROPERTY_CONTROLPASSWORD));
                                    if(xPropInfo->hasPropertyByName(PROPERTY_USER))
                                        m_sUser = ::comphelper::getString(xProp->getPropertyValue(PROPERTY_USER));
                                    if(xPropInfo->hasPropertyByName(PROPERTY_PASSWORD))
                                        m_sUserPassword = ::comphelper::getString(xProp->getPropertyValue(PROPERTY_PASSWORD));
                                    if(xPropInfo->hasPropertyByName(PROPERTY_CACHESIZE))
                                        xProp->getPropertyValue(PROPERTY_CACHESIZE) >>= m_nCacheSize;
                                    callModifiedHdl();
                                }
                            }
                        }
                    }
                    catch(Exception&)
                    {
                    }
                }
                break;
            default:
                OSL_ENSURE(sal_False, "OGeneralPage::OnCreateDatabase: invalid type!");
        }
        return 0L;
    }

    //-------------------------------------------------------------------------
    sal_Bool OGeneralPage::createDirectoryDeep(const String& _rPathURL)
    {
        ::rtl::OUString sPath(_rPathURL);

        // get an URL object analyzing the URL for us ...
        INetURLObject aParser;
        aParser.SetURL(_rPathURL);

        INetProtocol eProtocol = aParser.GetProtocol();

        ::std::vector< ::rtl::OUString > aToBeCreated;  // the to-be-created levels

        // search a level which exists
        // #106016# ---------------------
        IS_PATH_EXIST eParentExists = PATH_NOT_EXIST;
        while ( eParentExists == PATH_NOT_EXIST && aParser.getSegmentCount())
        {
            aToBeCreated.push_back(aParser.getName());  // remember the local name for creation
            aParser.removeSegment();                    // cut the local name
            eParentExists = pathExists(aParser.GetMainURL(INetURLObject::NO_DECODE), sal_False);
        }

        if (!aParser.getSegmentCount())
            return sal_False;

        // create all the missing levels
        try
        {
            // the parent content
            Reference< XCommandEnvironment > xEmptyEnv;
            ::ucb::Content aParent(aParser.GetMainURL(INetURLObject::NO_DECODE), xEmptyEnv);

            ::rtl::OUString sContentType;
            if ( INET_PROT_FILE == eProtocol )
            {
                sContentType = ::rtl::OUString::createFromAscii( "application/vnd.sun.staroffice.fsys-folder" );
                // the file UCP currently does not support the ContentType property
            }
            else
            {
                Any aContentType = aParent.getPropertyValue( ::rtl::OUString::createFromAscii( "ContentType" ) );
                aContentType >>= sContentType;
            }

            // the properties which need to be set on the new content
            Sequence< ::rtl::OUString > aNewDirectoryProperties(1);
            aNewDirectoryProperties[0] = ::rtl::OUString::createFromAscii("Title");

            // the values to be set
            Sequence< Any > aNewDirectoryAttributes(1);

            // loop
            for (   ::std::vector< ::rtl::OUString >::reverse_iterator aLocalName = aToBeCreated.rbegin();
                    aLocalName != aToBeCreated.rend();
                    ++aLocalName
                )
            {
                aNewDirectoryAttributes[0] <<= *aLocalName;
                if (!aParent.insertNewContent(sContentType, aNewDirectoryProperties, aNewDirectoryAttributes, aParent))
                    return sal_False;
            }
        }
        catch (const Exception& e)
        {
            OSL_ENSURE( sal_False, "" );
            e; // make compiler happy
            return sal_False;
        }

        return sal_True;
    }

    // #106016# -------------------------------------------------------------------
    IS_PATH_EXIST OGeneralPage::pathExists(const ::rtl::OUString& _rURL, sal_Bool bIsFile) const
    {
        ::ucb::Content aCheckExistence;
        sal_Bool bExists = sal_False;
        IS_PATH_EXIST eExists = PATH_NOT_EXIST;
        Reference< XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = Reference< ::com::sun::star::task::XInteractionHandler >(
            xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler") ) ), UNO_QUERY );
        Reference< XCommandEnvironment > xCmdEnv = new ::ucb::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
        try
        {
            aCheckExistence = ::ucb::Content(_rURL, xCmdEnv );
            bExists = bIsFile? aCheckExistence.isDocument(): aCheckExistence.isFolder();
            eExists = bExists? PATH_EXIST: PATH_NOT_EXIST;
        }
        catch(const Exception&)
        {
            eExists = PATH_NOT_KNOWN;
           }
        return eExists;
    }

    //-------------------------------------------------------------------------
    sal_Int32 OGeneralPage::checkPathExistence(const String& _rURL)
    {
        // #106016# ----------------
        if ( pathExists(_rURL, sal_False) == PATH_NOT_EXIST )
        {
            String sQuery(ModuleRes(STR_ASK_FOR_DIRECTORY_CREATION));
            OFileNotation aTransformer(_rURL);
            sQuery.SearchAndReplaceAscii("$path$", aTransformer.get(OFileNotation::N_SYSTEM));

            m_bUserGrabFocus = sal_False;
            QueryBox aQuery(GetParent(), WB_YES_NO | WB_DEF_YES, sQuery);
            sal_Int32 nQueryResult = aQuery.Execute();
            m_bUserGrabFocus = sal_True;

            switch (nQueryResult)
            {
                case RET_YES:
                {
                    sal_Bool bTryCreate = sal_False;
                    do
                    {
                        if (!createDirectoryDeep(_rURL))
                        {   // could not create the directory
                            sQuery = String(ModuleRes(STR_COULD_NOT_CREATE_DIRECTORY));
                            sQuery.SearchAndReplaceAscii("$name$", aTransformer.get(OFileNotation::N_SYSTEM));

                            m_bUserGrabFocus = sal_False;
                            QueryBox aWhatToDo(GetParent(), WB_RETRY_CANCEL | WB_DEF_RETRY, sQuery);
                            nQueryResult = aWhatToDo.Execute();
                            m_bUserGrabFocus = sal_True;

                            if (RET_RETRY == nQueryResult)
                                bTryCreate = sal_True;
                            else
                                return RET_RETRY;
                        }
                    }
                    while (bTryCreate);
                }
                break;

                case RET_NO:
                    return RET_OK;

                default:
                    // cancelled
                    return RET_CANCEL;
            }
        }
        return RET_OK;
    }

    //-------------------------------------------------------------------------
    sal_Bool OGeneralPage::commitURL()
    {
        if ((DST_DBASE == m_eCurrentSelection) || (DST_TEXT == m_eCurrentSelection) || (DST_CALC == m_eCurrentSelection))
        {
            String sOldPath = m_aConnection.GetSavedValueNoPrefix();
            String sURL = m_aConnection.GetTextNoPrefix();
            if ((sURL != sOldPath) && (0 != sURL.Len()))
            {   // the text changed since entering the control

                // the path may be in system notation ....
                OFileNotation aTransformer(sURL);
                sURL = aTransformer.get(OFileNotation::N_URL);

                if(DST_CALC == m_eCurrentSelection)
                { // #106016# --------------------------
                    if( pathExists(sURL, sal_True) == PATH_NOT_EXIST )
                    {
                        String sFile = String(ModuleRes(STR_CALCDOC_DOESNOTEXIST));
                        sFile.SearchAndReplaceAscii("$file$", aTransformer.get(OFileNotation::N_SYSTEM));
                        OSQLMessageBox(this,String(ModuleRes(STR_STAT_WARNING)),sFile).Execute();
                        setURLNoPrefix(sOldPath);
                        return sal_False;
                    }
                    else
                    {
                        setURLNoPrefix(sURL);
                        m_aConnection.SaveValueNoPrefix();
                    }
                }
                else
                {
                    switch (checkPathExistence(sURL))
                    {
                        case RET_RETRY:
                            m_bUserGrabFocus = sal_False;
                            m_aConnection.GrabFocus();
                            m_bUserGrabFocus = sal_True;
                            return sal_False;

                        case RET_CANCEL:
                            setURLNoPrefix(sOldPath);
                            return sal_False;

                        default:
                            // accept the input
                            setURLNoPrefix(sURL);
                            m_aConnection.SaveValueNoPrefix();
                            break;
                    }
                }
            }
        }

        return sal_True;
    }

    //-------------------------------------------------------------------------
    long OGeneralPage::PreNotify( NotifyEvent& _rNEvt )
    {
        if ((DST_DBASE == m_eCurrentSelection) || (DST_TEXT == m_eCurrentSelection) || (DST_CALC == m_eCurrentSelection))
            switch (_rNEvt.GetType())
            {
                case EVENT_GETFOCUS:
                    if (m_aConnection.IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field got the focus
                        m_aConnection.SaveValueNoPrefix();
                    }
                    break;

                case EVENT_LOSEFOCUS:
                    if (m_aConnection.IsWindowOrChild(_rNEvt.GetWindow()) && m_bUserGrabFocus)
                    {   // a descendant of the URL edit field lost the focus
                        if (!commitURL())
                            return 1L;  // handled
                    }
                    break;
            }

        return OGenericAdministrationPage::PreNotify( _rNEvt );
    }

    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnBrowseConnections, PushButton*, _pButton)
    {
        switch (GetSelectedType())
        {
            case DST_DBASE:
            case DST_TEXT:
            {
                try
                {
                    ::rtl::OUString sFolderPickerService = ::rtl::OUString::createFromAscii(SERVICE_UI_FOLDERPICKER);
                    Reference< XFolderPicker > xFolderPicker(m_xORB->createInstance(sFolderPickerService), UNO_QUERY);
                    if (!xFolderPicker.is())
                    {
                        ShowServiceNotAvailableError(GetParent(), sFolderPickerService, sal_True);
                        break;
                    }

                    sal_Bool bDoBrowse = sal_False;
                    String sOldPath = getURLNoPrefix();
                    do
                    {
                        if (sOldPath.Len())
                            xFolderPicker->setDisplayDirectory(sOldPath);
                        if (0 == xFolderPicker->execute())
                            // cancelled by the user
                            return 0L;

                        sOldPath = xFolderPicker->getDirectory();
                        switch (checkPathExistence(sOldPath))
                        {
                            case RET_RETRY:
                                bDoBrowse = sal_True;
                                break;
                            case RET_CANCEL:
                                return 0L;
                            default:
                                break;
                        }
                    }
                    while (bDoBrowse);

                    String sSelectedDirectory = xFolderPicker->getDirectory();
                    INetURLObject aSelectedDirectory( sSelectedDirectory, INetURLObject::WAS_ENCODED, RTL_TEXTENCODING_UTF8 );

                    // for UI purpose, we don't want to have the path encoded
                    sSelectedDirectory = aSelectedDirectory.GetMainURL( INetURLObject::DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8  );

                    setURLNoPrefix( sSelectedDirectory );
                    callModifiedHdl();
                }
                catch(const Exception&)
                {
                    DBG_ERROR("OGeneralPage::OnBrowseConnections: caught an exception while browsing for the path!");
                }
            }
            break;
            case DST_CALC:
            {
                ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK | WB_STDMODAL | WB_OPEN);

                String sOldPath = getURLNoPrefix();
                if (sOldPath.Len())
                    aFileDlg.SetDisplayDirectory(sOldPath);
                if (0 == aFileDlg.Execute())
                {
                    setURLNoPrefix(aFileDlg.GetPath());
                    callModifiedHdl();
                }
            }
            break;
            case DST_ADABAS:
            {
                // collect all names from the config dir
                // and all dir's of the DBWORK/wrk or DBROOT/wrk dir
                // compare the names

                // collect the names of the installed databases
                StringBag aInstalledDBs;
                ::rtl::OUString sAdabasConfigDir,sAdabasWorkDir,sRootDir;
                ::rtl::OUString sTemp = ::rtl::OUString::createFromAscii("DBWORK");
                rtl_uString* pDbVar = NULL;
                if(osl_getEnvironment(sTemp.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sAdabasWorkDir = pDbVar;
                    String sTemp;
                    sal_Bool bOk = utl::LocalFileHelper::ConvertPhysicalNameToURL(sAdabasWorkDir,sTemp);
                    sAdabasWorkDir = sTemp;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }
                sTemp = ::rtl::OUString::createFromAscii("DBCONFIG");
                if(osl_getEnvironment(sTemp.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sAdabasConfigDir = pDbVar;
                    String sTemp;
                    sal_Bool bOk = utl::LocalFileHelper::ConvertPhysicalNameToURL(sAdabasConfigDir,sTemp);
                    sAdabasConfigDir = sTemp;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }
                sTemp = ::rtl::OUString::createFromAscii("DBROOT");
                if(osl_getEnvironment(sTemp.pData,&pDbVar) == osl_Process_E_None && pDbVar)
                {
                    sRootDir = pDbVar;
                    String sTemp;
                    sal_Bool bOk = utl::LocalFileHelper::ConvertPhysicalNameToURL(sRootDir,sTemp);
                    sRootDir = sTemp;
                    rtl_uString_release(pDbVar);
                    pDbVar = NULL;
                }

                sal_Bool bOldFashion = sAdabasConfigDir.getLength() && sAdabasWorkDir.getLength();



                if(!bOldFashion) // we have a normal adabas installation
                {    // so we check the local database names in $DBROOT/config
                    sAdabasConfigDir    = sRootDir;
                    sAdabasWorkDir      = sRootDir;
                }

                if(sAdabasConfigDir.getLength() && sAdabasWorkDir.getLength() && sRootDir.getLength())
                {

                    aInstalledDBs   = getInstalledAdabasDBs(sAdabasConfigDir,sAdabasWorkDir);

                    if(!aInstalledDBs.size() && bOldFashion)
                    {
                        sAdabasConfigDir    = sRootDir;
                        sAdabasWorkDir      = sRootDir;
                        aInstalledDBs       = getInstalledAdabasDBs(sAdabasConfigDir,sAdabasWorkDir);
                    }

                    ODatasourceSelectDialog aSelector(GetParent(), aInstalledDBs, GetSelectedType());
                    if (RET_OK == aSelector.Execute())
                    {
                        String aSelected;
                        aSelected.AssignAscii(":");
                        aSelected += aSelector.GetSelected();
                        setURLNoPrefix(aSelected);
                        callModifiedHdl();
                    }
                }
                else
                {
                    OLocalResourceAccess aLocRes( PAGE_GENERAL, RSC_TABPAGE );
                    String sError(ResId(STR_NO_ADABASE_DATASOURCES));
                    ErrorBox aBox(this, WB_OK, sError);
                    aBox.Execute();
                }
            }
            break;
            case DST_MYSQL_ODBC:
            case DST_ODBC:
            {
                // collect all ODBC data source names
                ::rtl::OUString sDataSource;
                if ( getSelectedDataSource(GetSelectedType(),sDataSource) && sDataSource.getLength() )
                {
                    setURLNoPrefix(sDataSource);
                    callModifiedHdl();
                }
                else
                    return 1L;
            }
            break;
            case DST_ADDRESSBOOK:
            {
                ::rtl::OUString sAddressBookTypes[]=
                {
                    AddressBookTypes::getAddressURL( ABT_OUTLOOK ),
                    AddressBookTypes::getAddressURL( ABT_OE ),
                    AddressBookTypes::getAddressURL( ABT_LDAP ),
                    AddressBookTypes::getAddressURL( ABT_MORK )
                };
                static String sAddressBookTypesTrans[]=
                {
                    String(ModuleRes(STR_ADDRESSBOOK_OUTLOOK)),
                    String(ModuleRes(STR_ADDRESSBOOK_SYSTEM)),
                    String(ModuleRes(STR_ADDRESSBOOK_LDAP)),
                    String(ModuleRes(STR_ADDRESSBOOK_MOZILLA))
                };
                static sal_Int32 s_nTypes = sizeof(sAddressBookTypes)/sizeof(::rtl::OUString);
                StringBag aAddressBooks;
                Reference< XDriverAccess> xManager(m_pAdminDialog->getORB()->createInstance(SERVICE_SDBC_DRIVERMANAGER), UNO_QUERY);
                for(sal_Int32 i=0;i<sizeof(sAddressBookTypes)/sizeof(::rtl::OUString);++i)
                {
                    if(xManager->getDriverByURL(sAddressBookTypes[i]).is())
                        aAddressBooks.insert(sAddressBookTypesTrans[i]);
                }

                ODatasourceSelectDialog aSelector(GetParent(), aAddressBooks, GetSelectedType());

                // initial selection
                String sType = getURL();
                sal_Int32 nOldPos = 0;
                for (; nOldPos < s_nTypes && sType != String(sAddressBookTypes[nOldPos]); ++nOldPos)
                    ;
                if (nOldPos < s_nTypes)
                    aSelector.Select(sAddressBookTypesTrans[nOldPos]);

                if (RET_OK == aSelector.Execute())
                {
                    String sType = aSelector.GetSelected();
                    sal_Int32 nNewPos=0;
                    for(;nNewPos < s_nTypes && sType != sAddressBookTypesTrans[nNewPos];++nNewPos)
                        ;
                    if(nOldPos != nNewPos)
                    {
                        setURL(sAddressBookTypes[nNewPos]);
                        // as the (sub-)type changed, call the handler
                        if (m_aTypeSelectHandler.IsSet())
                            m_aTypeSelectHandler.Call(this);

                        callModifiedHdl();
                    }
                }
            }
            break;
        }
        return 0L;
    }
    // -----------------------------------------------------------------------------
    StringBag OGeneralPage::getInstalledAdabasDBs(const String &_rConfigDir,const String &_rWorkDir)
    {
        String sAdabasConfigDir(_rConfigDir),sAdabasWorkDir(_rWorkDir);

        if (sAdabasConfigDir.Len() && ('/' == sAdabasConfigDir.GetBuffer()[sAdabasConfigDir.Len() - 1]))
            sAdabasConfigDir.AppendAscii("config");
        else
            sAdabasConfigDir.AppendAscii("/config");

        if (sAdabasWorkDir.Len() && ('/' == sAdabasWorkDir.GetBuffer()[sAdabasWorkDir.Len() - 1]))
            sAdabasWorkDir.AppendAscii("wrk");
        else
            sAdabasWorkDir.AppendAscii("/wrk");
        // collect the names of the installed databases
        StringBag aInstalledDBs;
        // collect the names of the installed databases
        StringBag aConfigDBs,aWrkDBs;
        aConfigDBs  = getInstalledAdabasDBDirs(sAdabasConfigDir,::ucb::INCLUDE_DOCUMENTS_ONLY);
        aWrkDBs     = getInstalledAdabasDBDirs(sAdabasWorkDir,::ucb::INCLUDE_FOLDERS_ONLY);
        ConstStringBagIterator aOuter = aConfigDBs.begin();
        for(;aOuter != aConfigDBs.end();++aOuter)
        {
            ConstStringBagIterator aInner = aWrkDBs.begin();
            for (;aInner != aWrkDBs.end(); ++aInner)
            {
                if (aInner->equalsIgnoreAsciiCase(*aOuter))
                {
                    aInstalledDBs.insert(*aInner);
                    break;
                }
            }
        }
        return aInstalledDBs;
    }
    // -----------------------------------------------------------------------------
    StringBag OGeneralPage::getInstalledAdabasDBDirs(const String& _rPath,const ::ucb::ResultSetInclude& _reResultSetInclude)
    {
        INetURLObject aNormalizer;
        aNormalizer.SetSmartProtocol(INET_PROT_FILE);
        aNormalizer.SetSmartURL(_rPath);
        String sAdabasConfigDir = aNormalizer.GetMainURL(INetURLObject::NO_DECODE);

        ::ucb::Content aAdabasConfigDir;
        try
        {
            aAdabasConfigDir = ::ucb::Content(sAdabasConfigDir, Reference< ::com::sun::star::ucb::XCommandEnvironment >());
        }
        catch(::com::sun::star::ucb::ContentCreationException&)
        {
            return StringBag();
        }

        StringBag aInstalledDBs;
        sal_Bool bIsFolder = sal_False;
        try
        {
            bIsFolder = aAdabasConfigDir.isFolder();
        }
        catch(Exception&) // the exception is thrown when the path doesn't exists
        {
        }
        if (bIsFolder && aAdabasConfigDir.get().is())
        {   // we have a content for the directory, loop through all entries
            Sequence< ::rtl::OUString > aProperties(1);
            aProperties[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));

            try
            {
                Reference< XResultSet > xFiles = aAdabasConfigDir.createCursor(aProperties, _reResultSetInclude);
                Reference< XRow > xRow(xFiles, UNO_QUERY);
                xFiles->beforeFirst();
                while (xFiles->next())
                {
#ifdef DBG_UTIL
                    ::rtl::OUString sName = xRow->getString(1);
#endif
                    aInstalledDBs.insert(xRow->getString(1));
                }
            }
            catch(Exception&)
            {
                DBG_ERROR("OGeneralPage::getInstalledAdabasDBDirs: could not enumerate the adabas config files!");
            }
        }


        return aInstalledDBs;
    }
    //-------------------------------------------------------------------------
    IMPL_LINK(OGeneralPage, OnDatasourceTypeSelected, ListBox*, _pBox)
    {
        // get the type from the entry data
        sal_Int16 nSelected = _pBox->GetSelectEntryPos();
        DATASOURCE_TYPE eSelectedType = static_cast<DATASOURCE_TYPE>(reinterpret_cast<sal_Int32>(_pBox->GetEntryData(nSelected)));
        // let the impl method do all the stuff
        onTypeSelected(eSelectedType);
        // tell the listener we were modified
        callModifiedHdl();
        // outta here
        return 0L;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

