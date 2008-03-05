/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbwizsetup.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:33:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dbwizsetup.hxx"
#include "dsmeta.hxx"
#include "DBSetupConnectionPages.hxx"
#include "dbadminsetup.hrc"
#include "dbu_dlg.hrc"
#include "dsitems.hxx"

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
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
#ifndef DBAUI_ASYNCRONOUSLINK_HXX
#include "AsyncronousLink.hxx"
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
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
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
/** === end UNO includes === **/

#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#include "linkeddocuments.hxx"
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef COMPHELPER_NAMEDVALUECOLLECTION_HXX
#include <comphelper/namedvaluecollection.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX_
#include <comphelper/sequenceashashmap.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"


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
using namespace ::com::sun::star::document;
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


#define DBASE_PATH               1
#define TEXT_PATH                2
#define MSACCESS_PATH            3
#define LDAP_PATH                4
#define ADABAS_PATH              5
#define ADO_PATH                 6
#define JDBC_PATH                7
#define ORACLE_PATH              8
#define MYSQL_JDBC_PATH          9
#define MYSQL_ODBC_PATH          10
#define ODBC_PATH                11
#define SPREADSHEET_PATH         12
#define OUTLOOKEXP_PATH          13
#define OUTLOOK_PATH             14
#define MOZILLA_PATH             15
#define EVOLUTION_PATH           16
#define EVOLUTION_PATH_GROUPWISE 17
#define EVOLUTION_PATH_LDAP      18
#define KAB_PATH                 19
#define MACAB_PATH               20
#define THUNDERBIRD_PATH         21
#define CREATENEW_PATH           22
#define USERDEFINED_PATH         23
#define OPEN_DOC_PATH            24
#define MSACCESS2007_PATH        25

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
    :svt::RoadmapWizard( _pParent, ModuleRes(DLG_DATABASE_WIZARD),
                        WZB_NEXT | WZB_PREVIOUS | WZB_FINISH | WZB_CANCEL | WZB_HELP,
                        ModuleRes( STR_ROADMAPHEADER ), sal_True)

    , m_pOutSet(NULL)
    , m_eType( DST_UNKNOWN )
    , m_eOldType( DST_UNKNOWN )
    , m_bResetting(sal_False)
    , m_bApplied(sal_False)
    , m_bUIEnabled( sal_True )
    , m_bIsConnectable( sal_False)
    , m_sRM_IntroText(ModuleRes(STR_PAGETITLE_INTROPAGE))
    , m_sRM_dBaseText(ModuleRes(STR_PAGETITLE_DBASE))
    , m_sRM_TextText(ModuleRes(STR_PAGETITLE_TEXT))
    , m_sRM_MSAccessText(ModuleRes(STR_PAGETITLE_MSACCESS))
    , m_sRM_LDAPText(ModuleRes(STR_PAGETITLE_LDAP))
    , m_sRM_ADABASText(ModuleRes(STR_PAGETITLE_ADABAS))
    , m_sRM_ADOText(ModuleRes(STR_PAGETITLE_ADO))
    , m_sRM_JDBCText(ModuleRes(STR_PAGETITLE_JDBC))
    , m_pGeneralPage( NULL )
    , m_pMySQLIntroPage(NULL)
    , m_pCollection( NULL )
{
    DBG_CTOR(ODbTypeWizDialogSetup,NULL);
    // no local resources needed anymore
    m_sRM_MySQLText = String(ModuleRes(STR_PAGETITLE_MYSQL));
    m_sRM_OracleText = String(ModuleRes(STR_PAGETITLE_ORACLE));
    m_sRM_ODBCText = String(ModuleRes(STR_PAGETITLE_ODBC));
    m_sRM_SpreadSheetText = String(ModuleRes(STR_PAGETITLE_SPREADSHEET));
    m_sRM_AuthentificationText = String(ModuleRes(STR_PAGETITLE_AUTHENTIFICATION));
    m_sRM_FinalText = String(ModuleRes(STR_PAGETITLE_FINAL));
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

    declareAuthDepPath( DST_ADO,                ADO_PATH,               PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ADO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_DBASE,              DBASE_PATH,             PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_DBASE, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_FLAT,               TEXT_PATH,              PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_TEXT, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declarePath       (                         SPREADSHEET_PATH,       PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_SPREADSHEET, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_ODBC,               ODBC_PATH,              PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ODBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_JDBC,               JDBC_PATH,              PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_JDBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_MYSQL_ODBC,         MYSQL_ODBC_PATH,        PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MYSQL_INTRO, PAGE_DBSETUPWIZARD_MYSQL_ODBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_MYSQL_JDBC,         MYSQL_JDBC_PATH,        PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MYSQL_INTRO, PAGE_DBSETUPWIZARD_MYSQL_JDBC, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_ORACLE_JDBC,        ORACLE_PATH,            PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ORACLE, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_ADABAS,             ADABAS_PATH,            PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_ADABAS, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_LDAP,               LDAP_PATH,              PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_LDAP, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_MSACCESS,           MSACCESS_PATH,          PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MSACCESS, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_MSACCESS_2007,      MSACCESS2007_PATH,      PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_MSACCESS, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_OUTLOOKEXP,         OUTLOOKEXP_PATH,        PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_OUTLOOK,            OUTLOOK_PATH,           PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_MOZILLA,            MOZILLA_PATH,           PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_THUNDERBIRD,        THUNDERBIRD_PATH,       PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_EVOLUTION,          EVOLUTION_PATH,         PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_EVOLUTION_GROUPWISE,EVOLUTION_PATH_GROUPWISE, PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_EVOLUTION_LDAP,     EVOLUTION_PATH_LDAP,    PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_KAB,                KAB_PATH,               PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_MACAB,              MACAB_PATH,             PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1);
    declareAuthDepPath( getDefaultDatabaseType(),CREATENEW_PATH,        PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declareAuthDepPath( DST_USERDEFINE1,        USERDEFINED_PATH,       PAGE_DBSETUPWIZARD_INTRO, PAGE_DBSETUPWIZARD_USERDEFINED,PAGE_DBSETUPWIZARD_AUTHENTIFICATION, PAGE_DBSETUPWIZARD_FINAL, -1 );
    declarePath       (                         OPEN_DOC_PATH,          PAGE_DBSETUPWIZARD_INTRO, -1 );

    m_pPrevPage->SetHelpId(HID_DBWIZ_PREVIOUS);
    m_pNextPage->SetHelpId(HID_DBWIZ_NEXT);
    m_pCancel->SetHelpId(HID_DBWIZ_CANCEL);
    m_pFinish->SetHelpId(HID_DBWIZ_FINISH);
    m_pHelp->SetUniqueId(UID_DBWIZ_HELP);
    SetRoadmapInteractive( sal_True );
    ActivatePage();
}

void ODbTypeWizDialogSetup::declareAuthDepPath( DATASOURCE_TYPE _eType, PathId _nPathId, WizardState _nFirstState, ... )
{
    bool bHasAuthentication = DataSourceMetaData::getAuthentication( _eType ) != AuthNone;

    // collect the elements of the path
    Path aPath;

    va_list aStateList;
    va_start( aStateList, _nFirstState );

    WizardState nState = _nFirstState;
    while ( nState != WZS_INVALID_STATE )
    {
        if ( bHasAuthentication || ( nState != PAGE_DBSETUPWIZARD_AUTHENTIFICATION ) )
            aPath.push_back( nState );

        nState = ::sal::static_int_cast< WizardState >( va_arg( aStateList, int ) );
    }
    va_end( aStateList );

    // call base method
    ::svt::RoadmapWizard::declarePath( _nPathId, aPath );
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
                LocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);
                sRoadmapItem = String(ModuleRes(STR_PAGETITLE_CONNECTION));
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
IMPL_LINK(ODbTypeWizDialogSetup, OnTypeSelected, OGeneralPage*, /*_pTabPage*/)
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
            { DST_MSACCESS_2007,MSACCESS2007_PATH   },
            { DST_OUTLOOKEXP,   OUTLOOKEXP_PATH     },
            { DST_OUTLOOK,      OUTLOOK_PATH        },
            { DST_MOZILLA,      MOZILLA_PATH        },
            { DST_THUNDERBIRD,  THUNDERBIRD_PATH    },
            { DST_EVOLUTION,    EVOLUTION_PATH      },
            { DST_EVOLUTION_GROUPWISE, EVOLUTION_PATH_GROUPWISE },
            { DST_EVOLUTION_LDAP, EVOLUTION_PATH_LDAP },
            { DST_KAB,          KAB_PATH            },
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

        size_t i = 0;
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
    switch ( m_eType )
    {
        case DST_KAB:
        case DST_EVOLUTION:
        case DST_EVOLUTION_GROUPWISE:
        case DST_EVOLUTION_LDAP:
        case DST_OUTLOOK:
        case DST_OUTLOOKEXP:
        case DST_MOZILLA:
        case DST_THUNDERBIRD:
            return sal_False;
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
Reference< XMultiServiceFactory > ODbTypeWizDialogSetup::getORB() const
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

    if ( pPage )
    {
        pPage->SetServiceFactory(m_pImpl->getORB());
        pPage->SetAdminDialog(this, this);

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
IMPL_LINK(ODbTypeWizDialogSetup, ImplClickHdl, OMySQLIntroPageSetup*, /*_pMySQLIntroPageSetup*/)
{
    if (getDatasourceType(*m_pOutSet) == DST_MYSQL_ODBC)
        activatePath( MYSQL_ODBC_PATH, sal_True);
    else
        activatePath( MYSQL_JDBC_PATH, sal_True);
    return sal_True;
}

// -----------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, OnChangeCreationMode, OGeneralPage*, /*_pGeneralPage*/)
{
    activateDatabasePath();
    return sal_True;
}

// -----------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, OnRecentDocumentSelected, OGeneralPage*, /*_pGeneralPage*/)
{
    enableButtons( WZB_FINISH, m_pGeneralPage->GetSelectedDocument().sURL.Len() != 0 );
    return 0L;
}

// -----------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialogSetup, OnSingleDocumentChosen, OGeneralPage*, /*_pGeneralPage*/)
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
            if ( pFinalPage )
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
    if ( _nState == PAGE_DBSETUPWIZARD_INTRO )
    {
        OSL_ENSURE(m_eType != DST_UNKNOWN && m_eOldType != DST_UNKNOWN,"Type unknown");
        if ( m_eType != m_eOldType )
            resetPages(m_pImpl->getCurrentDataSource());
    }
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(_nState));
    if ( pPage )
        return pPage->DeactivatePage(m_pOutSet) != 0;
    else
        return sal_False;
}

// -----------------------------------------------------------------------------
void ODbTypeWizDialogSetup::setTitle(const ::rtl::OUString& /*_sTitle*/)
{
    DBG_ERROR( "ODbTypeWizDialogSetup::setTitle: not implemented!" );
        // why?
}

//-------------------------------------------------------------------------
void ODbTypeWizDialogSetup::enableConfirmSettings( bool _bEnable )
{
    (void)_bEnable;
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
#if OSL_DEBUG_LEVEL > 0
            SFX_ITEMSET_GET(*m_pOutSet, pDocUrl, SfxStringItem, DSID_DOCUMENT_URL, sal_True);
            (void)pDocUrl;
#endif
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
        (void)e;    // make compiler happy
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
    DATASOURCE_TYPE ODbTypeWizDialogSetup::getDefaultDatabaseType() const
    {
        DATASOURCE_TYPE eRet = DST_DBASE;

        ::rtl::OUString sURL = m_pCollection->getDatasourcePrefix( DST_EMBEDDED_HSQLDB );
        Reference< XDriverAccess > xDriverManager( getORB()->createInstance( SERVICE_SDBC_DRIVERMANAGER ), UNO_QUERY );
        if ( xDriverManager.is() && xDriverManager->getDriverByURL( sURL ).is() )
            eRet = DST_EMBEDDED_HSQLDB;

        return eRet;
    }

    //-------------------------------------------------------------------------
    void ODbTypeWizDialogSetup::CreateDatabase()
    {
        ::rtl::OUString sUrl;
        DATASOURCE_TYPE eType = getDefaultDatabaseType();
        if ( eType == DST_EMBEDDED_HSQLDB )
        {
            sUrl = m_pCollection->getDatasourcePrefix( DST_EMBEDDED_HSQLDB );
            Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
            OSL_ENSURE(xDatasource.is(),"DataSource is null!");
            if ( xDatasource.is() )
                xDatasource->setPropertyValue( PROPERTY_INFO, makeAny( m_pCollection->getDefaultDBSettings( DST_EMBEDDED_HSQLDB ) ) );
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
    sal_Bool ODbTypeWizDialogSetup::callSaveAsDialog()
    {
        sal_Bool bRet = sal_False;
        WinBits nBits(WB_STDMODAL|WB_SAVEAS);
        ::sfx2::FileDialogHelper aFileDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION, static_cast<sal_uInt32>(nBits), this);
        const SfxFilter* pFilter = getStandardDatabaseFilter();
        if ( pFilter )
        {
            INetURLObject aWorkURL( m_sWorkPath );
            aFileDlg.SetDisplayFolder( aWorkURL.GetMainURL( INetURLObject::NO_DECODE ));

            ::rtl::OUString sDefaultName = String( ModuleRes( STR_DATABASEDEFAULTNAME ) );
            ::rtl::OUString sExtension = pFilter->GetDefaultExtension();
            sDefaultName += sExtension.replaceAt( 0, 1, ::rtl::OUString() );
            aWorkURL.Append( sDefaultName );
            sDefaultName = createUniqueFileName( aWorkURL );
            aFileDlg.SetFileName( sDefaultName );

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
    String ODbTypeWizDialogSetup::createUniqueFileName(const INetURLObject& _rURL)
    {
        Reference< XSimpleFileAccess > xSimpleFileAccess(getORB()->createInstance(::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" )), UNO_QUERY);
        :: rtl::OUString sFilename = _rURL.getName();
        ::rtl::OUString BaseName = _rURL.getBase();
        ::rtl::OUString sExtension = _rURL.getExtension();

        sal_Bool bElementExists = sal_True;

        INetURLObject aExistenceCheck( _rURL );
        for ( sal_Int32 i = 1; bElementExists; )
        {
            bElementExists = xSimpleFileAccess->exists( aExistenceCheck.GetMainURL( INetURLObject::NO_DECODE ) );
            if ( bElementExists )
            {
                aExistenceCheck.setBase( BaseName.concat( ::rtl::OUString::valueOf( i ) ) );
                ++i;
            }
        }
        return aExistenceCheck.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    }
    // -----------------------------------------------------------------------------
    IWizardPage* ODbTypeWizDialogSetup::getWizardPage(TabPage* _pCurrentPage) const
    {
        OGenericAdministrationPage* pPage = static_cast<OGenericAdministrationPage*>(_pCurrentPage);
        return pPage;
    }

    // -----------------------------------------------------------------------------
    namespace
    {
        // .............................................................................
        typedef ::cppu::WeakImplHelper1 <   XTerminateListener
                                        >   AsyncLoader_Base;
        class AsyncLoader : public AsyncLoader_Base
        {
        private:
            Reference< XComponentLoader >       m_xFrameLoader;
            Reference< XDesktop >               m_xDesktop;
            Reference< XInteractionHandler >    m_xInteractionHandler;
            ::rtl::OUString                     m_sURL;
            OAsyncronousLink                    m_aAsyncCaller;

        public:
            AsyncLoader( const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rURL );

            void doLoadAsync();

            // XTerminateListener
            virtual void SAL_CALL queryTermination( const EventObject& Event ) throw (TerminationVetoException, RuntimeException);
            virtual void SAL_CALL notifyTermination( const EventObject& Event ) throw (RuntimeException);
            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

        private:
            DECL_LINK( OnOpenDocument, void* );
        };

        // .............................................................................
        AsyncLoader::AsyncLoader( const Reference< XMultiServiceFactory >& _rxORB, const ::rtl::OUString& _rURL )
            :m_sURL( _rURL )
            ,m_aAsyncCaller( LINK( this, AsyncLoader, OnOpenDocument ) )
        {
            try
            {
                m_xDesktop.set( _rxORB->createInstance( SERVICE_FRAME_DESKTOP ), UNO_QUERY_THROW );
                m_xFrameLoader.set( m_xDesktop, UNO_QUERY_THROW );
                m_xInteractionHandler.set(
                    _rxORB->createInstance(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.InteractionHandler" ) )
                    ),
                    UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        // .............................................................................
        void AsyncLoader::doLoadAsync()
        {
            OSL_ENSURE( !m_aAsyncCaller.IsRunning(), "AsyncLoader:doLoadAsync: already running!" );

            acquire();
            try
            {
                if ( m_xDesktop.is() )
                    m_xDesktop->addTerminateListener( this );
            }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

            m_aAsyncCaller.Call( NULL );
        }

        // .............................................................................
        IMPL_LINK( AsyncLoader, OnOpenDocument, void*, /*_pEmptyArg*/ )
        {
            try
            {
                if ( m_xFrameLoader.is() )
                {
                    ::comphelper::NamedValueCollection aLoadArgs;
                    aLoadArgs.put( "InteractionHandler", m_xInteractionHandler );
                    aLoadArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );

                    Sequence< PropertyValue > aLoadArgPV;
                    aLoadArgs >>= aLoadArgPV;

                    m_xFrameLoader->loadComponentFromURL( m_sURL,
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ),
                        FrameSearchFlag::ALL,
                        aLoadArgPV
                    );
                }
            }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

            try
            {
                if ( m_xDesktop.is() )
                    m_xDesktop->removeTerminateListener( this );
            }
            catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

            release();
            return 0L;
        }

        // .............................................................................
        void SAL_CALL AsyncLoader::queryTermination( const EventObject& /*Event*/ ) throw (TerminationVetoException, RuntimeException)
        {
            throw TerminationVetoException();
        }

        // .............................................................................
        void SAL_CALL AsyncLoader::notifyTermination( const EventObject& /*Event*/ ) throw (RuntimeException)
        {
        }
        // .............................................................................
        void SAL_CALL AsyncLoader::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
        {
        }
    }

    // -----------------------------------------------------------------------------
    sal_Bool ODbTypeWizDialogSetup::onFinish(sal_Int32 _nResult)
    {
        if ( m_pGeneralPage->GetDatabaseCreationMode() == OGeneralPage::eOpenExisting )
        {
            // we're not going to re-use the XModel we have - since the document the user
            // wants us to load could be a non-database document. Instead, we asynchronously
            // open the selected document. Thus, the wizard's return value is RET_CANCEL,
            // which means to not continue loading the database document
            if ( !OWizardMachine::onFinish( RET_CANCEL ) )
                return sal_False;

            Reference< XComponentLoader > xFrameLoader;
            try
            {
                AsyncLoader* pAsyncLoader = new AsyncLoader( getORB(), m_pGeneralPage->GetSelectedDocument().sURL );
                ::rtl::Reference< AsyncLoader > xKeepAlive( pAsyncLoader );
                pAsyncLoader->doLoadAsync();
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
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

