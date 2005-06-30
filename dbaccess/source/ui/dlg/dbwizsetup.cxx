/*************************************************************************
 *
 *  $RCSfile: dbwizsetup.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2005-06-30 16:32:58 $
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

#ifndef DBAUI_DBWIZ2_HXX
#include "dbwizsetup.hxx"
#endif
#ifndef DBAUI_DBSETUPCONNECTIONPAGES_HXX
#include "DBSetupConnectionPages.hxx"
#endif
#ifndef _DBAUI_DBADMINSETUP_HRC_
#include "dbadminsetup.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _DBAUI_GENERALPAGE_HXX_
#include "generalpage.hxx"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _DBAUI_STRINGLISTITEM_HXX_
#include "stringlistitem.hxx"
#endif
#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _DBAUI_DBADMINIMPL_HXX_
#include "DbAdminImpl.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#include "ConnectionPageSetup.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDOCUMENTDATASOURCE_HPP_
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#include "linkeddocuments.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif



#include <memory>


//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace dbtools;
using namespace svt;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::task;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::frame;
using namespace com::sun::star::ucb;
using namespace ::com::sun::star::sdb;
using namespace ::comphelper;
using namespace ::cppu;

#define START_PAGE      0
#define CONNECTION_PAGE 1


#define PAGE_DBSETUPWIZARD_INTRO                     0
#define PAGE_DBSETUPWIZARD_DBASE                    1
#define PAGE_DBSETUPWIZARD_TEXT                      2
#define PAGE_DBSETUPWIZARD_MSACCESS                  3
#define PAGE_DBSETUPWIZARD_LDAP                      4
#define PAGE_DBSETUPWIZARD_ADABAS                    5
#define PAGE_DBSETUPWIZARD_MYSQL_INTRO               6
#define PAGE_DBSETUPWIZARD_MYSQL_JDBC                7
#define PAGE_DBSETUPWIZARD_MYSQL_ODBC                8
#define PAGE_DBSETUPWIZARD_ORACLE                    9
#define PAGE_DBSETUPWIZARD_JDBC                      10
#define PAGE_DBSETUPWIZARD_ADO                       11
#define PAGE_DBSETUPWIZARD_ODBC                      12
#define PAGE_DBSETUPWIZARD_SPREADSHEET               13
#define PAGE_DBSETUPWIZARD_AUTHENTIFICATION          14
#define PAGE_DBSETUPWIZARD_MOZILLA                   15
#define PAGE_DBSETUPWIZARD_FINAL                     16
#define PAGE_DBSETUPWIZARD_USERDEFINED               17


#define DBASE_PATH             1
#define TEXT_PATH              2
#define MSACCESS_PATH          3
#define LDAP_PATH              4
#define ADABAS_PATH            5
#define ADO_PATH               6
#define JDBC_PATH              7
#define ORACLE_PATH            8
#define MYSQL_JDBC_PATH        9
#define MYSQL_ODBC_PATH        10
#define ODBC_PATH              11
#define SPREADSHEET_PATH       12
#define OUTLOOKEXP_PATH        13
#define OUTLOOK_PATH           14
#define MOZILLA_PATH           15
#define EVOLUTION_PATH         16
#define THUNDERBIRD_PATH       17
#define CREATENEW_PATH         18
#define USERDEFINED_PATH       19
#define OPEN_DOC_PATH          20

OFinalDBPageSetup*          pFinalPage;



DBG_NAME(ODbTypeWizDialogSetup)
//=========================================================================
//= ODbTypeWizDialogSetup
//=========================================================================
//-------------------------------------------------------------------------
ODbTypeWizDialogSetup::ODbTypeWizDialogSetup(Window* _pParent
                               ,SfxItemSet* _pItems
                               ,const Reference< XMultiServiceFactory >& _rxORB
                               ,const ::com::sun::star::uno::Any& _aDataSourceName
                               )
    :svt::RoadmapWizard(_pParent, ModuleRes(DLG_DATABASE_WIZARD),
    WZB_NEXT | WZB_PREVIOUS | WZB_FINISH | WZB_CANCEL | WZB_HELP
    , ResId( STR_ROADMAPHEADER), sal_True)
    , m_bResetting(sal_False)
    , m_bApplied(sal_False)
    , m_bUIEnabled( sal_True )
    , m_bIsConnectable( sal_False)
    , m_pOutSet(NULL)
    , m_pMySQLIntroPage(NULL)
    , m_sRM_IntroText(ResId(STR_PAGETITLE_INTROPAGE))
    , m_sRM_dBaseText(ResId(STR_PAGETITLE_DBASE))
    , m_sRM_TextText(ResId(STR_PAGETITLE_TEXT))
    , m_sRM_MSAccessText(ResId(STR_PAGETITLE_MSACCESS))
    , m_sRM_LDAPText(ResId(STR_PAGETITLE_LDAP))
    , m_sRM_ADABASText(ResId(STR_PAGETITLE_ADABAS))
    , m_sRM_ADOText(ResId(STR_PAGETITLE_ADO))
    , m_sRM_JDBCText(ResId(STR_PAGETITLE_JDBC))
{
    DBG_CTOR(ODbTypeWizDialogSetup,NULL);
    // no local resources needed anymore
    m_sRM_MySQLText = String(ResId(STR_PAGETITLE_MYSQL));
    m_sRM_OracleText = String(ResId(STR_PAGETITLE_ORACLE));
    m_sRM_ODBCText = String(ResId(STR_PAGETITLE_ODBC));
    m_sRM_SpreadSheetText = String(ResId(STR_PAGETITLE_SPREADSHEET));
    m_sRM_AuthentificationText = String(ResId(STR_PAGETITLE_AUTHENTIFICATION));
    m_sRM_FinalText = String(ResId(STR_PAGETITLE_FINAL));
    m_sWorkPath = SvtPathOptions().GetWorkPath();
    pFinalPage = NULL;
    // extract the datasource type collection from the item set
    DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _pItems->GetItem(DSID_TYPECOLLECTION));
    if (pCollectionItem)
        m_pCollection = pCollectionItem->getCollection();

    DBG_ASSERT(m_pCollection, "OGeneralPage::OGeneralPage : really need a DSN type collection !");

    FreeResource();

    m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,this,this));
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    m_pOutSet = new SfxItemSet( *_pItems->GetPool(), _pItems->GetRanges() );

    m_pImpl->translateProperties(xDatasource, *m_pOutSet);
    m_eType = m_pImpl->getDatasourceType(*m_pOutSet);

    SetPageSizePixel(LogicToPixel(::Size(WIZARD_PAGE_X, WIZARD_PAGE_Y), MAP_APPFONT));
    ShowButtonFixedLine(sal_True);
    defaultButton(WZB_NEXT);
    enableButtons(WZB_FINISH, sal_True);

    if ( m_pCollection->hasAuthentication(DST_ADO))
        declarePath( ADO_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ADO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( ADO_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ADO, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_DBASE))
        declarePath( DBASE_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_DBASE, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( DBASE_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_DBASE, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_FLAT))
        declarePath( TEXT_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_TEXT, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( TEXT_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_TEXT, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    declarePath( SPREADSHEET_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_SPREADSHEET, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_ODBC))
        declarePath( ODBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ODBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( ODBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ODBC, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_JDBC))
        declarePath( JDBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_JDBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( JDBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_JDBC, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_MYSQL_ODBC))
        declarePath( MYSQL_ODBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MYSQL_INTRO, PAGE_DBSETUPWIZARD_MYSQL_ODBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( MYSQL_ODBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MYSQL_INTRO, PAGE_DBSETUPWIZARD_MYSQL_ODBC, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_MYSQL_JDBC))
        declarePath( MYSQL_JDBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MYSQL_INTRO, PAGE_DBSETUPWIZARD_MYSQL_JDBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( MYSQL_JDBC_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MYSQL_INTRO, PAGE_DBSETUPWIZARD_MYSQL_JDBC, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_MYSQL_ODBC))
        declarePath( ORACLE_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ORACLE, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( ORACLE_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ORACLE, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_ADABAS))
        declarePath( ADABAS_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ADABAS, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( ADABAS_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ADABAS, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_LDAP))
        declarePath( LDAP_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_LDAP, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( LDAP_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_LDAP, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_MSACCESS))
        declarePath( MSACCESS_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MSACCESS, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( MSACCESS_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MSACCESS, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_OUTLOOKEXP))
        declarePath( OUTLOOKEXP_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( OUTLOOKEXP_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_OUTLOOK))
        declarePath( OUTLOOK_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( OUTLOOK_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_MOZILLA))
        declarePath( MOZILLA_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( MOZILLA_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_THUNDERBIRD))
        declarePath( THUNDERBIRD_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( THUNDERBIRD_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_EVOLUTION))
        declarePath( EVOLUTION_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( EVOLUTION_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(m_pCollection->getEmbeddedDatabaseType(getORB())))
        declarePath( CREATENEW_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( CREATENEW_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    if ( m_pCollection->hasAuthentication(DST_USERDEFINE1))
        declarePath( USERDEFINED_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_USERDEFINED,PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);
    else
        declarePath( USERDEFINED_PATH, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_USERDEFINED,PAGE_DBSETUPWIZARD_FINAL, WZS_INVALID_STATE);

    declarePath( OPEN_DOC_PATH, PAGE_DBSETUPWIZARD_INTRO, WZS_INVALID_STATE );

    m_pPrevPage->SetHelpId(HID_DBWIZ_PREVIOUS);
    m_pNextPage->SetHelpId(HID_DBWIZ_NEXT);
    m_pCancel->SetHelpId(HID_DBWIZ_CANCEL);
    m_pFinish->SetHelpId(HID_DBWIZ_FINISH);
    m_pHelp->SetUniqueId(UID_DBWIZ_HELP);
    SetRoadmapInteractive( sal_True );
    ActivatePage();
}


String ODbTypeWizDialogSetup::getStateDisplayName( WizardState _nState ){
    String sRoadmapItem;
    switch( _nState )
    {
        case PAGE_DBSETUPWIZARD_INTRO:
            sRoadmapItem = m_sRM_IntroText;
            break;

        case PAGE_DBSETUPWIZARD_DBASE:
            sRoadmapItem = m_sRM_dBaseText;
            break;
        case PAGE_DBSETUPWIZARD_ADO:
            sRoadmapItem = m_sRM_ADOText;
            break;
        case PAGE_DBSETUPWIZARD_TEXT:
            sRoadmapItem = m_sRM_TextText;
            break;
        case PAGE_DBSETUPWIZARD_MSACCESS:
            sRoadmapItem = m_sRM_MSAccessText;
            break;
        case PAGE_DBSETUPWIZARD_LDAP:
            sRoadmapItem = m_sRM_LDAPText;
            break;
        case PAGE_DBSETUPWIZARD_ADABAS:
            sRoadmapItem = m_sRM_ADABASText;
            break;
        case PAGE_DBSETUPWIZARD_JDBC:
            sRoadmapItem = m_sRM_JDBCText;
            break;
        case PAGE_DBSETUPWIZARD_ORACLE:
            sRoadmapItem = m_sRM_OracleText;
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_INTRO:
            sRoadmapItem = m_sRM_MySQLText;
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_JDBC:
            sRoadmapItem = m_sRM_JDBCText;
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_ODBC:
            sRoadmapItem = m_sRM_ODBCText;
            break;
        case PAGE_DBSETUPWIZARD_ODBC:
            sRoadmapItem = m_sRM_ODBCText;
            break;
        case PAGE_DBSETUPWIZARD_SPREADSHEET:
            sRoadmapItem = m_sRM_SpreadSheetText;
            break;
        case PAGE_DBSETUPWIZARD_AUTHENTIFICATION:
            sRoadmapItem = m_sRM_AuthentificationText;
            break;
        case PAGE_DBSETUPWIZARD_USERDEFINED:
            {
                OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);
                sRoadmapItem = String(ResId(STR_PAGETITLE_CONNECTION));
            }
            break;
        case PAGE_DBSETUPWIZARD_FINAL:
            sRoadmapItem = m_sRM_FinalText;
            break;
        default:
            break;
    }
    return sRoadmapItem;
}

//-------------------------------------------------------------------------
ODbTypeWizDialogSetup::~ODbTypeWizDialogSetup()
{
    delete m_pOutSet;
    DBG_DTOR(ODbTypeWizDialogSetup,NULL);
}

//-------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, OnTypeSelected, OGeneralPage*, _pTabPage)
{
    activateDatabasePath();
    return 1L;
}

//-------------------------------------------------------------------------
void ODbTypeWizDialogSetup::activateDatabasePath()
{
    switch ( m_pGeneralPage->GetDatabaseCreationMode() )
    {
    case OGeneralPage::eCreateNew:
    {
        activatePath( CREATENEW_PATH, sal_True);
        enableState(PAGE_DBSETUPWIZARD_FINAL, sal_True );
        enableButtons( WZB_FINISH, sal_True);
    }
    break;
    case OGeneralPage::eConnectExternal:
    {
        m_eType = VerifyDataSourceType(m_pGeneralPage->GetSelectedType());
        if (m_eType == DST_UNKNOWN)
            m_eType = m_eOldType;

        struct _map_type_to_path
        {
            DATASOURCE_TYPE             eType;
            RoadmapWizardTypes::PathId  nPathId;
        } aKnownTypesAndPaths[] = {
            { DST_DBASE,        DBASE_PATH          },
            { DST_ADO,          ADO_PATH            },
            { DST_FLAT,         TEXT_PATH           },
            { DST_CALC,         SPREADSHEET_PATH    },
            { DST_ODBC,         ODBC_PATH           },
            { DST_JDBC,         JDBC_PATH           },
            { DST_MYSQL_JDBC,   MYSQL_JDBC_PATH     },
            { DST_MYSQL_ODBC,   MYSQL_ODBC_PATH     },
            { DST_ORACLE_JDBC,  ORACLE_PATH         },
            { DST_ADABAS,       ADABAS_PATH         },
            { DST_LDAP,         LDAP_PATH           },
            { DST_MSACCESS,     MSACCESS_PATH       },
            { DST_OUTLOOKEXP,   OUTLOOKEXP_PATH     },
            { DST_OUTLOOK,      OUTLOOK_PATH        },
            { DST_MOZILLA,      MOZILLA_PATH        },
            { DST_THUNDERBIRD,  THUNDERBIRD_PATH    },
            { DST_EVOLUTION,    EVOLUTION_PATH      },
            { DST_USERDEFINE1,  USERDEFINED_PATH    },
            { DST_USERDEFINE2,  USERDEFINED_PATH    },
            { DST_USERDEFINE3,  USERDEFINED_PATH    },
            { DST_USERDEFINE4,  USERDEFINED_PATH    },
            { DST_USERDEFINE5,  USERDEFINED_PATH    },
            { DST_USERDEFINE6,  USERDEFINED_PATH    },
            { DST_USERDEFINE7,  USERDEFINED_PATH    },
            { DST_USERDEFINE8,  USERDEFINED_PATH    },
            { DST_USERDEFINE9,  USERDEFINED_PATH    },
            { DST_USERDEFINE10, USERDEFINED_PATH    }
        };

        sal_Int32 i = 0;
        for ( ; i < sizeof( aKnownTypesAndPaths ) / sizeof( aKnownTypesAndPaths[0] ); ++i )
        {
            if ( aKnownTypesAndPaths[i].eType == m_eType )
            {
                activatePath( aKnownTypesAndPaths[i].nPathId, sal_True);
                break;
            }
        }
        DBG_ASSERT( i < sizeof( aKnownTypesAndPaths ) / sizeof( aKnownTypesAndPaths[0] ),
            "ODbTypeWizDialogSetup::activateDatabasePath: unknown database type!" );
        updateTypeDependentStates();
    }
    break;
    case OGeneralPage::eOpenExisting:
    {
        activatePath( OPEN_DOC_PATH, sal_True );
        enableButtons( WZB_FINISH, m_pGeneralPage->GetSelectedDocument().sURL.Len() != 0 );
    }
    break;
    default:
        DBG_ERROR( "ODbTypeWizDialogSetup::activateDatabasePath: unknown creation mode!" );
    }

    enableButtons( WZB_NEXT, m_pGeneralPage->GetDatabaseCreationMode() != OGeneralPage::eOpenExisting );
        // TODO: this should go into the base class. Point is, we activate a path whose *last*
        // step is also the current one. The base class should automatically disable
        // the Next button in such a case. However, not for this patch ...
}

//-------------------------------------------------------------------------
void ODbTypeWizDialogSetup::updateTypeDependentStates()
{
    sal_Bool bDoEnable = sal_False;
    sal_Bool bIsConnectionRequired = IsConnectionUrlRequired();
    if (!bIsConnectionRequired)
    {
        bDoEnable = sal_True;
    }
    else
    {
        if (m_eType == m_eOldType)
        {
            bDoEnable = m_bIsConnectable; //(sConnectURL.Len() != 0);
        }
    }
    enableState(PAGE_DBSETUPWIZARD_AUTHENTIFICATION, bDoEnable);
    enableState(PAGE_DBSETUPWIZARD_FINAL, bDoEnable );
    enableButtons( WZB_FINISH, bDoEnable);
}


//-------------------------------------------------------------------------
sal_Bool ODbTypeWizDialogSetup::IsConnectionUrlRequired()
{
    DATASOURCE_TYPE eType = getDatasourceType(*m_pOutSet);
    switch ( m_eType )
    {
        case DST_EVOLUTION:
        case DST_OUTLOOK:
        case DST_OUTLOOKEXP:
        case DST_MOZILLA:
        case DST_THUNDERBIRD:
            return sal_False;
            break;
        default:
            return sal_True;
    }
}

//-------------------------------------------------------------------------
void ODbTypeWizDialogSetup::resetPages(const Reference< XPropertySet >& _rxDatasource)
{
    // remove all items which relate to indirect properties from the input set
    // (without this, the following may happen: select an arbitrary data source where some indirect properties
    // are set. Select another data source of the same type, where the indirect props are not set (yet). Then,
    // the indirect property values of the first ds are shown in the second ds ...)
    const ODbDataSourceAdministrationHelper::MapInt2String& rMap = m_pImpl->getIndirectProperties();
    for (   ODbDataSourceAdministrationHelper::ConstMapInt2StringIterator aIndirect = rMap.begin();
            aIndirect != rMap.end();
            ++aIndirect
        )
        getWriteOutputSet()->ClearItem( (sal_uInt16)aIndirect->first );

    // extract all relevant data from the property set of the data source
    m_pImpl->translateProperties(_rxDatasource, *getWriteOutputSet());
}
//-------------------------------------------------------------------------
ODbTypeWizDialogSetup::ApplyResult ODbTypeWizDialogSetup::implApplyChanges()
{
    if ( !m_pImpl->saveChanges(*m_pOutSet) )
        return AR_KEEP;

    m_bApplied = sal_True;

    return AR_LEAVE_MODIFIED;
}
// -----------------------------------------------------------------------------
const SfxItemSet* ODbTypeWizDialogSetup::getOutputSet() const
{
    return m_pOutSet;
}
// -----------------------------------------------------------------------------
SfxItemSet* ODbTypeWizDialogSetup::getWriteOutputSet()
{
    return m_pOutSet;
}
// -----------------------------------------------------------------------------
::std::pair< Reference<XConnection>,sal_Bool> ODbTypeWizDialogSetup::createConnection()
{
    return m_pImpl->createConnection();
}
// -----------------------------------------------------------------------------
Reference< XMultiServiceFactory > ODbTypeWizDialogSetup::getORB()
{
    return m_pImpl->getORB();
}
// -----------------------------------------------------------------------------
Reference< XDriver > ODbTypeWizDialogSetup::getDriver()
{
    return m_pImpl->getDriver();
}


DATASOURCE_TYPE ODbTypeWizDialogSetup::VerifyDataSourceType(const DATASOURCE_TYPE _DatabaseType) const
{
    DATASOURCE_TYPE LocDatabaseType = _DatabaseType;
    if ((LocDatabaseType == DST_MYSQL_JDBC) || (LocDatabaseType == DST_MYSQL_ODBC))
    {
        if (m_pMySQLIntroPage != NULL)
        {
            if (m_pMySQLIntroPage->getMySQLMode() == 1)                 // TODO: use constant or Enum)
                return DST_MYSQL_JDBC;
            else
                return DST_MYSQL_ODBC;
        }
    }
    return LocDatabaseType;
}



// -----------------------------------------------------------------------------
DATASOURCE_TYPE ODbTypeWizDialogSetup::getDatasourceType(const SfxItemSet& _rSet) const
{
    DATASOURCE_TYPE LocDatabaseType = m_pImpl->getDatasourceType(_rSet);
    return VerifyDataSourceType(LocDatabaseType);
}

// -----------------------------------------------------------------------------
void ODbTypeWizDialogSetup::clearPassword()
{
    m_pImpl->clearPassword();
}

// -----------------------------------------------------------------------------
TabPage* ODbTypeWizDialogSetup::createPage(WizardState _nState)
{
    SfxTabPage* pFirstPage;
    sal_Bool bResetPasswordRequired = sal_False;
    OGenericAdministrationPage* pPage = NULL;
    switch(_nState)
    {
        case PAGE_DBSETUPWIZARD_INTRO:
            pFirstPage = OGeneralPage::Create(this,*m_pOutSet, sal_True);
            pPage = static_cast<OGenericAdministrationPage*> (pFirstPage);
            m_pGeneralPage = static_cast<OGeneralPage*>(pFirstPage);
            m_pGeneralPage->SetTypeSelectHandler(LINK(this, ODbTypeWizDialogSetup, OnTypeSelected));
            m_pGeneralPage->SetCreationModeHandler(LINK( this, ODbTypeWizDialogSetup, OnChangeCreationMode ) );
            m_pGeneralPage->SetDocumentSelectionHandler(LINK( this, ODbTypeWizDialogSetup, OnRecentDocumentSelected ) );
            m_pGeneralPage->SetChooseDocumentHandler(LINK( this, ODbTypeWizDialogSetup, OnSingleDocumentChosen ) );
            break;

        case PAGE_DBSETUPWIZARD_DBASE:
            pPage = OConnectionTabPageSetup::CreateDbaseTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_ADO:
            pPage = OConnectionTabPageSetup::CreateADOTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_TEXT:
            pPage = OTextConnectionPageSetup::CreateTextTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_ODBC:
            pPage = OConnectionTabPageSetup::CreateODBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_JDBC:
            pPage = OJDBCConnectionPageSetup::CreateJDBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_MYSQL_ODBC:
            m_pOutSet->Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getDatasourcePrefix(DST_MYSQL_ODBC)));
            pPage = OConnectionTabPageSetup::CreateODBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_MYSQL_JDBC:
            m_pOutSet->Put(SfxStringItem(DSID_CONNECTURL, m_pCollection->getDatasourcePrefix(DST_MYSQL_JDBC)));
            pPage = OGeneralSpecialJDBCConnectionPageSetup::CreateMySQLJDBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_ORACLE:
            pPage = OGeneralSpecialJDBCConnectionPageSetup::CreateOracleJDBCTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_ADABAS:
            pPage = OConnectionTabPageSetup::CreateAdabasTabPage( this, *m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_LDAP    :
            pPage = OLDAPConnectionPageSetup::CreateLDAPTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_SPREADSHEET:    /// first user defined driver
            pPage = OSpreadSheetConnectionPageSetup::CreateSpreadSheetTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_MSACCESS:
            pPage  = OConnectionTabPageSetup::CreateMSAccessTabPage(this,*m_pOutSet);
            break;
        case PAGE_DBSETUPWIZARD_MYSQL_INTRO:
            m_pMySQLIntroPage = OMySQLIntroPageSetup::CreateMySQLIntroTabPage(this,*m_pOutSet);
            m_pMySQLIntroPage->SetClickHdl(LINK( this, ODbTypeWizDialogSetup, ImplClickHdl ) );
            pPage = m_pMySQLIntroPage;
            break;

        case PAGE_DBSETUPWIZARD_AUTHENTIFICATION:
            pPage = OAuthentificationPageSetup::CreateAuthentificationTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_USERDEFINED:
            pPage = OConnectionTabPageSetup::CreateUserDefinedTabPage(this,*m_pOutSet);
            break;

        case PAGE_DBSETUPWIZARD_FINAL:
            pPage = OFinalDBPageSetup::CreateFinalDBTabPageSetup(this,*m_pOutSet);
            pFinalPage = static_cast<OFinalDBPageSetup*> (pPage);
            break;
    }

    if ((_nState != PAGE_DBSETUPWIZARD_INTRO) && (_nState != PAGE_DBSETUPWIZARD_AUTHENTIFICATION))
    {
        pPage->SetModifiedHandler(LINK( this, ODbTypeWizDialogSetup, ImplModifiedHdl ) );
    }
    // register ourself as modified listener
    if ( pPage )
    {
        pPage->SetServiceFactory(m_pImpl->getORB());
        pPage->SetAdminDialog(this, this);

        // open our own resource block, as the page titles are strings local to this block
        OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);

        defaultButton( _nState == PAGE_DBSETUPWIZARD_FINAL ? WZB_FINISH : WZB_NEXT );
        enableButtons( WZB_FINISH, _nState == START_PAGE ? sal_False : sal_True);
        enableButtons( WZB_NEXT, _nState == PAGE_DBSETUPWIZARD_FINAL ? sal_False : sal_True);
        pPage->Show();
    }
    return pPage;
}


IMPL_LINK(ODbTypeWizDialogSetup, ImplModifiedHdl, OGenericAdministrationPage*, _pConnectionPageSetup)
{
    m_bIsConnectable = _pConnectionPageSetup->GetRoadmapStateValue( );
    enableState(PAGE_DBSETUPWIZARD_FINAL, m_bIsConnectable);
    enableState(PAGE_DBSETUPWIZARD_AUTHENTIFICATION, m_bIsConnectable);
    if (getCurrentState() == PAGE_DBSETUPWIZARD_FINAL)
        enableButtons( WZB_FINISH, sal_True);
    else
        enableButtons( WZB_FINISH, m_bIsConnectable);
    enableButtons( WZB_NEXT, m_bIsConnectable  && (getCurrentState() != PAGE_DBSETUPWIZARD_FINAL));
    return sal_True;
}


// -----------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, ImplClickHdl, OMySQLIntroPageSetup*, _pMySQLIntroPageSetup)
{
    if (getDatasourceType(*m_pOutSet) == DST_MYSQL_ODBC)
        activatePath( MYSQL_ODBC_PATH, sal_True);
    else
        activatePath( MYSQL_JDBC_PATH, sal_True);
    return sal_True;
}

// -----------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, OnChangeCreationMode, OGeneralPage*, _pGeneralPage)
{
    activateDatabasePath();
    return sal_True;
}

// -----------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, OnRecentDocumentSelected, OGeneralPage*, _pGeneralPage)
{
    enableButtons( WZB_FINISH, m_pGeneralPage->GetSelectedDocument().sURL.Len() != 0 );
    return 0L;
}

// -----------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, OnSingleDocumentChosen, OGeneralPage*, _pGeneralPage)
{
    if ( prepareLeaveCurrentState( eFinish ) )
        onFinish( RET_OK );
    return 0L;
}

// -----------------------------------------------------------------------------
void ODbTypeWizDialogSetup::enterState(WizardState _nState)
{
    m_eType = m_pImpl->getDatasourceType(*m_pOutSet);
    RoadmapWizard::enterState(_nState);
    switch(_nState)
    {
        case PAGE_DBSETUPWIZARD_INTRO:
            m_eOldType = m_eType;
            break;
        case PAGE_DBSETUPWIZARD_FINAL:
            enableButtons( WZB_FINISH, sal_True);
            pFinalPage->enableTableWizardCheckBox(m_pCollection->supportsTableCreation(m_eType));
            break;
    }
}

//-------------------------------------------------------------------------
sal_Bool ODbTypeWizDialogSetup::saveDatasource()
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(getCurrentState()));
    if ( pPage )
        pPage->FillItemSet(*m_pOutSet);
    return sal_True;
}


// -----------------------------------------------------------------------------
sal_Bool ODbTypeWizDialogSetup::leaveState(WizardState _nState)
{
    if (_nState == PAGE_DBSETUPWIZARD_MYSQL_INTRO)
        return sal_True;
    if ( _nState == PAGE_DBSETUPWIZARD_INTRO ){
        OGeneralPage* pPage = static_cast<OGeneralPage*>(WizardDialog::GetPage(getCurrentState()));
        OSL_ENSURE(m_eType != DST_UNKNOWN && m_eOldType != DST_UNKNOWN,"Type unknown");
        if ( m_eType != m_eOldType )
            resetPages(m_pImpl->getCurrentDataSource());
    }
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(_nState));
    if ( pPage )
        return pPage->DeactivatePage(m_pOutSet);
    else
        return sal_False;
}

// -----------------------------------------------------------------------------
void ODbTypeWizDialogSetup::setTitle(const ::rtl::OUString& _sTitle)
{
}

//-------------------------------------------------------------------------
sal_Bool ODbTypeWizDialogSetup::SaveDatabaseDocument()
{
    Reference< XInteractionHandler > xHandler(getORB()->createInstance(SERVICE_TASK_INTERACTION_HANDLER), UNO_QUERY);
    try
    {
        if (callSaveAsDialog() == sal_True)
        {
            m_pImpl->saveChanges(*m_pOutSet);
            Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
            SFX_ITEMSET_GET(*m_pOutSet, pDocUrl, SfxStringItem, DSID_DOCUMENT_URL, sal_True);
            Reference<XStorable> xStore(getDataSourceOrModel(xDatasource),UNO_QUERY);
            Reference<XComponent> xComponent(xStore,UNO_QUERY);
            ::rtl::OUString sPath = m_pImpl->getDocumentUrl(*m_pOutSet);
            if ( xStore.is() )
            {
                if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPage::eCreateNew )
                    CreateDatabase();
                Reference< XModel > xModel(xStore, UNO_QUERY);

                Sequence<PropertyValue> aArgs = xModel->getArgs();

                sal_Bool bOverwrite = sal_True;
                sal_Bool bAddOverwrite = sal_True;
                sal_Bool bAddInteractionHandler = sal_True;
                PropertyValue* pIter = aArgs.getArray();
                PropertyValue* pEnd  = pIter + aArgs.getLength();
                for(;pIter != pEnd;++pIter)
                {
                    if ( pIter->Name.equalsAscii("Overwrite") )
                    {
                        pIter->Value <<= bOverwrite;
                        bAddOverwrite = sal_False;
                    }
                    if ( pIter->Name.equalsAscii("InteractionHandler") )
                    {
                        pIter->Value <<= xHandler;
                        bAddInteractionHandler = sal_False;
                    }

                }
                if ( bAddOverwrite )
                {
                    sal_Int32 nLen = aArgs.getLength();
                    aArgs.realloc(nLen+1);
                    aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Overwrite"));
                    aArgs[nLen].Value <<= bOverwrite;
                }
                if ( bAddInteractionHandler )
                {
                    sal_Int32 nLen = aArgs.getLength();
                    aArgs.realloc(nLen+1);
                    aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InteractionHandler"));
                    aArgs[nLen].Value <<= xHandler;
                }
                xStore->storeAsURL(sPath,aArgs);

                if (pFinalPage != NULL)
                {
                    if (pFinalPage->IsDatabaseDocumentToBeRegistered())
                        RegisterDataSourceByLocation(sPath);
                }
                else
                {
                    RegisterDataSourceByLocation(sPath);
                }
                return sal_True;
            }
        }
    }
    catch (Exception& e)
    {
        InteractiveIOException aRequest;
        aRequest.Code = IOErrorCode_GENERAL;
        OInteractionRequest * pRequest  = new OInteractionRequest (makeAny (aRequest));
        Reference < XInteractionRequest > xRequest(pRequest );
        OInteractionAbort* pAbort = new OInteractionAbort;
        pRequest ->addContinuation (pAbort );
        if ( xHandler.is() )
            xHandler->handle( xRequest );
        e;  // make compiler happy
    }
    return sal_False;
}
    // ------------------------------------------------------------------------
    sal_Bool ODbTypeWizDialogSetup::IsDatabaseDocumentToBeOpened() const
    {
        if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPage::eOpenExisting )
            return sal_True;

        if ( pFinalPage != NULL )
            return pFinalPage->IsDatabaseDocumentToBeOpened();

        return sal_True;
    }

    // ------------------------------------------------------------------------
    sal_Bool ODbTypeWizDialogSetup::IsTableWizardToBeStarted() const
    {
        if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPage::eOpenExisting )
            return sal_False;

        if ( pFinalPage != NULL )
            return pFinalPage->IsTableWizardToBeStarted();

        return sal_False;
    }

    //-------------------------------------------------------------------------
    void ODbTypeWizDialogSetup::CreateDatabase()
    {
        ::rtl::OUString sUrl;
        DATASOURCE_TYPE eType = m_pCollection->getEmbeddedDatabaseType(getORB());
        if ( eType == DST_EMBEDDED )
        {
            sUrl = m_pCollection->getEmbeddedDatabaseURL(getORB());
            Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
            OSL_ENSURE(xDatasource.is(),"DataSource is null!");
            if ( xDatasource.is() )
                xDatasource->setPropertyValue(PROPERTY_INFO,makeAny(m_pCollection->getEmbeddedDatabaseProperties(getORB())));
            m_pImpl->translateProperties(xDatasource,*m_pOutSet);
        }
        if ( eType == DST_DBASE )
        {
            Reference< XSimpleFileAccess > xSimpleFileAccess(getORB()->createInstance(::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" )), UNO_QUERY);
            INetURLObject aDBPathURL(m_sWorkPath);
            aDBPathURL.Append(m_aDocURL.getBase());
            createUniqueFolderName(&aDBPathURL);
            ::rtl::OUString sPrefix = m_pCollection->getDatasourcePrefix(DST_DBASE);
            sUrl = aDBPathURL.GetMainURL( INetURLObject::NO_DECODE);
            xSimpleFileAccess->createFolder(sUrl);
            //OFileNotation aFileNotation(sUrl);
            //sUrl = aFileNotation.get(OFileNotation::N_SYSTEM);
             sUrl = sPrefix.concat(sUrl);
        }
        m_pOutSet->Put(SfxStringItem(DSID_CONNECTURL, sUrl));
        m_pImpl->saveChanges(*m_pOutSet);
    }

    //-------------------------------------------------------------------------
    void ODbTypeWizDialogSetup::RegisterDataSourceByLocation(const ::rtl::OUString& _sPath)
    {
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        Reference< XNamingService > xDatabaseContext(getORB()->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
        Reference< XNameAccess > xNameAccessDatabaseContext(xDatabaseContext, UNO_QUERY);
        INetURLObject aURL( _sPath );
        ::rtl::OUString sFilename = aURL.getBase( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
        ::rtl::OUString sDatabaseName = ::dbtools::createUniqueName(xNameAccessDatabaseContext, sFilename,sal_False);
        xDatabaseContext->registerObject(sDatabaseName, xDatasource);
    }


    //-------------------------------------------------------------------------
    short ODbTypeWizDialogSetup::Execute()
    {
        short nResult = ModalDialog::Execute();
        return nResult;
    }

    //-------------------------------------------------------------------------
    sal_Bool ODbTypeWizDialogSetup::callSaveAsDialog()
    {
        sal_Bool bRet = sal_False;
        WinBits nBits(WB_STDMODAL|WB_SAVEAS);
        ::sfx2::FileDialogHelper aFileDlg( ::sfx2::FILESAVE_AUTOEXTENSION, static_cast<sal_uInt32>(nBits), this);
        const SfxFilter* pFilter = getStandardDatabaseFilter();
        if ( pFilter )
        {
            INetURLObject aWorkURL(m_sWorkPath);
            String sBase = String(ModuleRes(STR_DATABASEDEFAULTNAME));
            aWorkURL.Append(sBase);
//            aWorkURL.setBase(sBase);
            ::rtl::OUString sExtension = pFilter->GetDefaultExtension();
            sExtension = sExtension.replaceAt( 0, 2, ::rtl::OUString());
            aWorkURL.setExtension(sExtension);
            createUniqueFileName(&aWorkURL);
            aFileDlg.SetDisplayDirectory( aWorkURL.GetMainURL( INetURLObject::NO_DECODE ));
            aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
            aFileDlg.SetCurrentFilter(pFilter->GetUIName());
        }
        if ( aFileDlg.Execute() == ERRCODE_NONE )
        {
            m_aDocURL = INetURLObject(aFileDlg.GetPath());

            if( m_aDocURL.GetProtocol() != INET_PROT_NOT_VALID )
            {
                ::rtl::OUString sFileName = m_aDocURL.GetMainURL( INetURLObject::NO_DECODE );
                if ( ::utl::UCBContentHelper::IsDocument(sFileName) )
                    ::utl::UCBContentHelper::Kill(sFileName);
                m_pOutSet->Put(SfxStringItem(DSID_DOCUMENT_URL, sFileName));
                bRet = sal_True;
            }
        }
        return bRet;
    }

    //-------------------------------------------------------------------------
    void ODbTypeWizDialogSetup::createUniqueFolderName(INetURLObject* pURL)
    {
        Reference< XSimpleFileAccess > xSimpleFileAccess(getORB()->createInstance(::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" )), UNO_QUERY);
        :: rtl::OUString sLastSegmentName = pURL->getName();
        sal_Bool bFolderExists = sal_True;
        sal_Int32 i = 1;
        while (bFolderExists == sal_True)
        {
            bFolderExists = xSimpleFileAccess->isFolder(pURL->GetMainURL( INetURLObject::NO_DECODE ));
            if (bFolderExists == sal_True)
            {
                i++;
                pURL->setName(sLastSegmentName.concat(::rtl::OUString::valueOf(i)));
            }
        }
    }

    //-------------------------------------------------------------------------
    void ODbTypeWizDialogSetup::createUniqueFileName(INetURLObject* pURL)
    {
        Reference< XSimpleFileAccess > xSimpleFileAccess(getORB()->createInstance(::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" )), UNO_QUERY);
        :: rtl::OUString sFilename = pURL->getName();
        ::rtl::OUString BaseName = pURL->getBase();
        ::rtl::OUString sExtension = pURL->getExtension();
        sal_Bool bElementExists = sal_True;
        sal_Int32 i = 1;
        while (bElementExists == sal_True)
        {
            bElementExists = xSimpleFileAccess->exists(pURL->GetMainURL( INetURLObject::NO_DECODE ));
            if (bElementExists == sal_True)
            {
                i++;
                pURL->setBase(BaseName.concat(::rtl::OUString::valueOf(i)));
            }
        }
    }
    // -----------------------------------------------------------------------------
    IWizardPage* ODbTypeWizDialogSetup::getWizardPage(TabPage* _pCurrentPage) const
    {
        OGenericAdministrationPage* pPage = static_cast<OGenericAdministrationPage*>(_pCurrentPage);
        return pPage;
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODbTypeWizDialogSetup::onFinish(sal_Int32 _nResult)
    {
        if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPage::eOpenExisting )
        {
            if ( !OWizardMachine::onFinish( _nResult ) )
                return sal_False;

            Reference< XModel > xModel( m_pImpl->getCurrentModel() );
            DBG_ASSERT( xModel.is(), "ODbTypeWizDialogSetup::onFinish: no model?" );
            if ( xModel.is() )
            {
                OGeneralPage::DocumentDescriptor aDocument( m_pGeneralPage->GetSelectedDocument() );

                ::comphelper::SequenceAsHashMap aLoadArgs;
                aLoadArgs.createItemIfMissing( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) ),
                    ::rtl::OUString( aDocument.sURL ) );
                aLoadArgs.createItemIfMissing( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ),
                    ::rtl::OUString( aDocument.sURL ) );
                aLoadArgs.createItemIfMissing( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) ),
                    ::rtl::OUString( aDocument.sFilter ) );
                try
                {
                    xModel->attachResource( aDocument.sURL, aLoadArgs.getAsConstPropertyValueList() );
                }
                catch( const Exception& e )
                {
                    DBG_ERROR( "ODbTypeWizDialogSetup::onFinish: caught an exception while loading the document!" );
                    (void)e;
                }
            }

            return sal_True;
        }

        if (getCurrentState() != PAGE_DBSETUPWIZARD_FINAL)
        {
            skipUntil(PAGE_DBSETUPWIZARD_FINAL);
        }
        if (getCurrentState() == PAGE_DBSETUPWIZARD_FINAL)
            return SaveDatabaseDocument() ? OWizardMachine::onFinish(_nResult) : sal_False;
        else
        {
               enableButtons( WZB_FINISH, sal_False);
            return sal_False;
        }
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

