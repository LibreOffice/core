/*************************************************************************
 *
 *  $RCSfile: AdabasStatDlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:38:35 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef DBAUI_ADABASSTATDLG_HXX
#include "AdabasStatDlg.hxx"
#endif
#ifndef DBAUI_ADABASSTATDLG_HRC
#include "AdabasStatDlg.hrc"
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
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef _DBAUI_DBADMINIMPL_HXX_
#include "DbAdminImpl.hxx"
#endif
#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef DBAUI_ADABASPAGE_HXX
#include "AdabasPage.hxx"
#endif
//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;

    //========================================================================
    //= OAdabasStatPageDlg
    //========================================================================
    OAdabasStatPageDlg::OAdabasStatPageDlg(Window* _pParent
                                            , SfxItemSet* _pItems
                                            ,const Reference< XMultiServiceFactory >& _rxORB
                                            ,const ::com::sun::star::uno::Any& _aDataSourceName)
        :SfxTabDialog(_pParent, ModuleRes(DLG_DATABASE_ADABASADMIN), _pItems)
    {
        m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,_pParent,this));
        m_pImpl->setCurrentDataSourceName(_aDataSourceName);
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        m_pImpl->translateProperties(xDatasource, *GetInputSetImpl());
        SetInputSet(GetInputSetImpl());
        // propagate this set as our new input set and reset the example set
        delete pExampleSet;
        pExampleSet = new SfxItemSet(*GetInputSetImpl());

        DATASOURCE_TYPE eType = m_pImpl->getDatasourceType(*GetInputSetImpl());

        switch ( eType )
        {
            case DST_ADABAS:
                AddTabPage(TAB_PAG_ADABAS_SETTINGS, String(ResId(STR_PAGETITLE_ADABAS_STATISTIC)), ODriversSettings::CreateAdabas,0, sal_False, 1);
                break;
            case DST_DBASE:
            case DST_FLAT:
            case DST_MSACCESS:
            case DST_ADO:
            case DST_ODBC:
            case DST_MYSQL_ODBC:
            case DST_MYSQL_JDBC:
            case DST_LDAP:
            case DST_CALC:
            case DST_MOZILLA:
            case DST_EVOLUTION:
            case DST_OUTLOOK    :
            case DST_OUTLOOKEXP:
            case DST_JDBC:
            case DST_ORACLE_JDBC:
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
            default:
                OSL_ENSURE(0,"Not supported for other types thasn adabas!");
                break;
        }

        // remove the reset button - it's meaning is much too ambiguous in this dialog
        RemoveResetButton();
        FreeResource();
    }

    // -----------------------------------------------------------------------
    OAdabasStatPageDlg::~OAdabasStatPageDlg()
    {
        SetInputSet(NULL);
        DELETEZ(pExampleSet);
    }
    // -----------------------------------------------------------------------
    short OAdabasStatPageDlg::Execute()
    {
        short nRet = SfxTabDialog::Execute();
        if ( nRet == RET_OK )
        {
            pExampleSet->Put(*GetOutputItemSet());
            m_pImpl->saveChanges(*pExampleSet);
        }
        return nRet;
    }
    //-------------------------------------------------------------------------
    void OAdabasStatPageDlg::PageCreated(USHORT _nId, SfxTabPage& _rPage)
    {
        // register ourself as modified listener
        static_cast<OGenericAdministrationPage&>(_rPage).SetServiceFactory(m_pImpl->getORB());
        static_cast<OGenericAdministrationPage&>(_rPage).SetAdminDialog(this,this);

        AdjustLayout();
        Window *pWin = GetViewWindow();
        if(pWin)
            pWin->Invalidate();

        SfxTabDialog::PageCreated(_nId, _rPage);
    }
    // -----------------------------------------------------------------------------
    const SfxItemSet* OAdabasStatPageDlg::getOutputSet() const
    {
        return GetExampleSet();
    }
    // -----------------------------------------------------------------------------
    SfxItemSet* OAdabasStatPageDlg::getWriteOutputSet()
    {
        return pExampleSet;
    }
    // -----------------------------------------------------------------------------
    Reference< XConnection > OAdabasStatPageDlg::createConnection()
    {
        return m_pImpl->createConnection();
    }
    // -----------------------------------------------------------------------------
    Reference< XMultiServiceFactory > OAdabasStatPageDlg::getORB()
    {
        return m_pImpl->getORB();
    }
    // -----------------------------------------------------------------------------
    Reference< XDriver > OAdabasStatPageDlg::getDriver()
    {
        return m_pImpl->getDriver();
    }
    // -----------------------------------------------------------------------------
    DATASOURCE_TYPE OAdabasStatPageDlg::getDatasourceType(const SfxItemSet& _rSet) const
    {
        return m_pImpl->getDatasourceType(_rSet);
    }
    // -----------------------------------------------------------------------------
    void OAdabasStatPageDlg::clearPassword()
    {
        m_pImpl->clearPassword();
    }
    // -----------------------------------------------------------------------------
    void OAdabasStatPageDlg::setTitle(const ::rtl::OUString& _sTitle)
    {
        SetText(_sTitle);
    }
    //-------------------------------------------------------------------------
    sal_Bool OAdabasStatPageDlg::saveDatasource()
    {
        return PrepareLeaveCurrentPage();
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................
