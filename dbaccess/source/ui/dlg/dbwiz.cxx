/*************************************************************************
 *
 *  $RCSfile: dbwiz.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:45:10 $
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

#ifndef DBAUI_DBWIZ_HXX
#include "dbwiz.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
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
#ifndef DBAUI_CONNECTIONPAGE_HXX
#include "ConnectionPage.hxx"
#endif
#ifndef DBAUI_ADVANCEDPAGEDLG_HXX
#include "AdvancedPageDlg.hxx"
#endif
#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef _DBAUI_DBADMINIMPL_HXX_
#include "DbAdminImpl.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace dbtools;
using namespace svt;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
using namespace com::sun::star::util;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

#define START_PAGE          0
#define CONNECTION_PAGE     1
#define ADDITIONAL_PAGE     2


DBG_NAME(ODbTypeWizDialog)
//=========================================================================
//= ODbTypeWizDialog
//=========================================================================
//-------------------------------------------------------------------------
ODbTypeWizDialog::ODbTypeWizDialog(Window* _pParent
                               ,SfxItemSet* _pItems
                               ,const Reference< XMultiServiceFactory >& _rxORB
                               ,const ::com::sun::star::uno::Any& _aDataSourceName
                               )
    :OWizardMachine(_pParent, ModuleRes(DLG_DATABASE_TYPE_CHANGE), WZB_NEXT | WZB_PREVIOUS | WZB_FINISH | WZB_CANCEL | WZB_HELP, sal_True)
    ,m_bResetting(sal_False)
    ,m_bApplied(sal_False)
    ,m_bUIEnabled( sal_True )
    ,m_pOutSet(NULL)
{
    DBG_CTOR(ODbTypeWizDialog,NULL);
    m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,this,this));
    m_pImpl->setCurrentDataSourceName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    m_pOutSet = new SfxItemSet( *_pItems->GetPool(), _pItems->GetRanges() );

    m_pImpl->translateProperties(xDatasource, *m_pOutSet);
    m_eType = m_pImpl->getDatasourceType(*m_pOutSet);

    SetPageSizePixel(LogicToPixel(::Size(PAGE_X, PAGE_Y), MAP_APPFONT));
    ShowButtonFixedLine(sal_True);
    defaultButton(WZB_NEXT);
    enableButtons(WZB_FINISH, sal_False);

    m_pPrevPage->SetHelpId(HID_DBWIZ_PREVIOUS);
    m_pNextPage->SetHelpId(HID_DBWIZ_NEXT);
    m_pCancel->SetHelpId(HID_DBWIZ_CANCEL);
    m_pFinish->SetHelpId(HID_DBWIZ_FINISH);
    m_pHelp->SetUniqueId(UID_DBWIZ_HELP);
    // no local resources needed anymore
    FreeResource();
    ActivatePage();
}

//-------------------------------------------------------------------------
ODbTypeWizDialog::~ODbTypeWizDialog()
{
    DBG_DTOR(ODbTypeWizDialog,NULL);
    delete m_pOutSet;
}
//-------------------------------------------------------------------------
IMPL_LINK(ODbTypeWizDialog, OnTypeSelected, OGeneralPage*, _pTabPage)
{
    m_eType = _pTabPage->GetSelectedType();
    return 1L;
}
//-------------------------------------------------------------------------
WizardTypes::WizardState ODbTypeWizDialog::determineNextState(WizardState _nCurrentState)
{
    WizardTypes::WizardState nNextState = WZS_INVALID_STATE;
    switch(_nCurrentState)
    {
        case START_PAGE:
            nNextState = CONNECTION_PAGE;
            break;
        case CONNECTION_PAGE:
            switch(m_eType)
            {
                case DST_MOZILLA:
                case DST_OUTLOOK:
                case DST_OUTLOOKEXP:
                case DST_EVOLUTION:
                case DST_MSACCESS:
                case DST_JDBC:
                case DST_CALC:
                    nNextState = WZS_INVALID_STATE;
                    break;
                default:
                    nNextState = ADDITIONAL_PAGE;
                    break;
            }
            break;
    }

    return nNextState;
}

//-------------------------------------------------------------------------
void ODbTypeWizDialog::resetPages(const Reference< XPropertySet >& _rxDatasource)
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
ODbTypeWizDialog::ApplyResult ODbTypeWizDialog::implApplyChanges()
{
    if ( !m_pImpl->saveChanges(*m_pOutSet) )
        return AR_KEEP;

    m_bApplied = sal_True;

    return AR_LEAVE_MODIFIED;
}
// -----------------------------------------------------------------------------
const SfxItemSet* ODbTypeWizDialog::getOutputSet() const
{
    return m_pOutSet;
}
// -----------------------------------------------------------------------------
SfxItemSet* ODbTypeWizDialog::getWriteOutputSet()
{
    return m_pOutSet;
}
// -----------------------------------------------------------------------------
Reference< XConnection > ODbTypeWizDialog::createConnection()
{
    return m_pImpl->createConnection();
}
// -----------------------------------------------------------------------------
Reference< XMultiServiceFactory > ODbTypeWizDialog::getORB()
{
    return m_pImpl->getORB();
}
// -----------------------------------------------------------------------------
Reference< XDriver > ODbTypeWizDialog::getDriver()
{
    return m_pImpl->getDriver();
}
// -----------------------------------------------------------------------------
DATASOURCE_TYPE ODbTypeWizDialog::getDatasourceType(const SfxItemSet& _rSet) const
{
    return m_pImpl->getDatasourceType(_rSet);
}
// -----------------------------------------------------------------------------
void ODbTypeWizDialog::clearPassword()
{
    m_pImpl->clearPassword();
}
// -----------------------------------------------------------------------------
TabPage* ODbTypeWizDialog::createPage(WizardState _nState)
{
    sal_Bool bResetPasswordRequired = sal_False;
    USHORT nStringId = STR_PAGETITLE_ADVANCED;
    TabPage* pPage = NULL;
    switch(_nState)
    {
        case START_PAGE: // start state
            pPage = OGeneralPage::Create(this,*m_pOutSet);
            static_cast<OGeneralPage*>(pPage)->SetTypeSelectHandler(LINK(this, ODbTypeWizDialog, OnTypeSelected));
            nStringId = STR_PAGETITLE_GENERAL;
            break;
        case CONNECTION_PAGE:
            pPage = OConnectionTabPage::Create(this,*m_pOutSet);
            nStringId = STR_PAGETITLE_CONNECTION;
            break;
        case ADDITIONAL_PAGE:
            switch ( m_eType )
            {
                case DST_DBASE:
                    pPage = ODriversSettings::CreateDbase(this,*m_pOutSet);
                    break;

                case DST_ADO:
                    pPage = ODriversSettings::CreateAdo(this,*m_pOutSet);
                    break;

                case DST_FLAT:
                    pPage = ODriversSettings::CreateText(this,*m_pOutSet);
                    break;

                case DST_ODBC:
                    pPage = ODriversSettings::CreateODBC(this,*m_pOutSet);
                    break;

                case DST_MYSQL_ODBC:
                    pPage = ODriversSettings::CreateMySQLODBC(this,*m_pOutSet);
                    break;
                case DST_MYSQL_JDBC:
                    pPage = ODriversSettings::CreateMySQLJDBC(this,*m_pOutSet);
                    break;
                case DST_ORACLE_JDBC:
                    pPage = ODriversSettings::CreateOracleJDBC(this,*m_pOutSet);
                    break;

                case DST_ADABAS:
                    pPage = ODriversSettings::CreateAdabas(this,*m_pOutSet);
                    break;

                case DST_LDAP       :
                    pPage = ODriversSettings::CreateLDAP(this,*m_pOutSet);
                    break;
                case DST_MOZILLA:
                case DST_OUTLOOK:
                case DST_OUTLOOKEXP:
                case DST_EVOLUTION:
                    bResetPasswordRequired = sal_True;
                    break;
                case DST_USERDEFINE1:   /// first user defined driver
                case DST_USERDEFINE2:
                case DST_USERDEFINE3:
                case DST_USERDEFINE4:
                case DST_USERDEFINE5:
                case DST_USERDEFINE6:
                case DST_USERDEFINE7:
                case DST_USERDEFINE8:
                case DST_USERDEFINE9:
                case DST_USERDEFINE10:
                    pPage = ODriversSettings::CreateUser(this,*m_pOutSet);
                    break;
            }
            break;
        default:
            OSL_ENSURE(0,"Wrong state!");
            break;
    }
    //  if (bResetPasswordRequired)
//  {
//      getWriteOutputSet()->Put(SfxBoolItem(DSID_PASSWORDREQUIRED, sal_False));
//      if (pExampleSet)
//          pExampleSet->Put(SfxBoolItem(DSID_PASSWORDREQUIRED, sal_False));
//  }

    // register ourself as modified listener
    if ( pPage )
    {
        static_cast<OGenericAdministrationPage*>(pPage)->SetServiceFactory(m_pImpl->getORB());
        static_cast<OGenericAdministrationPage*>(pPage)->SetAdminDialog(this,this);
        // open our own resource block, as the page titles are strings local to this block
        OLocalResourceAccess aDummy(DLG_DATABASE_ADMINISTRATION, RSC_TABDIALOG);

        pPage->SetText(String(ResId(nStringId)));
        defaultButton( _nState == START_PAGE ? WZB_NEXT : WZB_FINISH );
        enableButtons( WZB_FINISH, _nState == START_PAGE ? sal_False : sal_True);
        pPage->Show();
    }
    return pPage;
}
// -----------------------------------------------------------------------------
sal_Bool ODbTypeWizDialog::leaveState(WizardState _nState)
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(_nState));
    if ( pPage )
        pPage->FillItemSet(*m_pOutSet);
    return sal_True;
}
// -----------------------------------------------------------------------------
void ODbTypeWizDialog::setTitle(const ::rtl::OUString& _sTitle)
{
    SetText(_sTitle);
}
//-------------------------------------------------------------------------
sal_Bool ODbTypeWizDialog::saveDatasource()
{
    SfxTabPage* pPage = static_cast<SfxTabPage*>(WizardDialog::GetPage(getCurrentState()));
    if ( pPage )
        pPage->FillItemSet(*m_pOutSet);
    return sal_True;
}
// -----------------------------------------------------------------------------
IWizardPage* ODbTypeWizDialog::getWizardPage(TabPage* _pCurrentPage) const
{
    OGenericAdministrationPage* pPage = static_cast<OGenericAdministrationPage*>(_pCurrentPage);
    return pPage;
}
// -----------------------------------------------------------------------------
sal_Bool ODbTypeWizDialog::onFinish(sal_Int32 _nResult)
{
    saveDatasource();
    return m_pImpl->saveChanges(*m_pOutSet) ? OWizardMachine::onFinish(_nResult) : sal_False;
}
//.........................................................................
}   // namespace dbaui
//.........................................................................

