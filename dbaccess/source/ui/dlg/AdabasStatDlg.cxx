/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AdabasStatDlg.cxx,v $
 * $Revision: 1.15.68.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBAUI_ADABASSTATDLG_HXX
#include "AdabasStatDlg.hxx"
#endif
#ifndef DBAUI_ADABASSTATDLG_HRC
#include "AdabasStatDlg.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
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
DBG_NAME(OAdabasStatPageDlg)
//========================================================================
    OAdabasStatPageDlg::OAdabasStatPageDlg(Window* _pParent
                                            , SfxItemSet* _pItems
                                            ,const Reference< XMultiServiceFactory >& _rxORB
                                            ,const ::com::sun::star::uno::Any& _aDataSourceName)
        :SfxTabDialog(_pParent, ModuleRes(DLG_DATABASE_ADABASADMIN), _pItems)
    {
        DBG_CTOR(OAdabasStatPageDlg,NULL);

        m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,_pParent,this));
        m_pImpl->setDataSourceOrName(_aDataSourceName);
        Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
        m_pImpl->translateProperties(xDatasource, *GetInputSetImpl());
        SetInputSet(GetInputSetImpl());
        // propagate this set as our new input set and reset the example set
        delete pExampleSet;
        pExampleSet = new SfxItemSet(*GetInputSetImpl());

        ::dbaccess::DATASOURCE_TYPE eType = m_pImpl->getDatasourceType(*GetInputSetImpl());

        switch ( eType )
        {
            case  ::dbaccess::DST_ADABAS:
                AddTabPage(TAB_PAG_ADABAS_SETTINGS, String(ModuleRes(STR_PAGETITLE_ADABAS_STATISTIC)), ODriversSettings::CreateAdabas,0, sal_False, 1);
                break;
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

        DBG_DTOR(OAdabasStatPageDlg,NULL);
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
    ::std::pair< Reference<XConnection>,sal_Bool> OAdabasStatPageDlg::createConnection()
    {
        return m_pImpl->createConnection();
    }
    // -----------------------------------------------------------------------------
    Reference< XMultiServiceFactory > OAdabasStatPageDlg::getORB() const
    {
        return m_pImpl->getORB();
    }
    // -----------------------------------------------------------------------------
    Reference< XDriver > OAdabasStatPageDlg::getDriver()
    {
        return m_pImpl->getDriver();
    }
    // -----------------------------------------------------------------------------
    ::dbaccess::DATASOURCE_TYPE OAdabasStatPageDlg::getDatasourceType(const SfxItemSet& _rSet) const
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
    void OAdabasStatPageDlg::enableConfirmSettings( bool _bEnable )
    {
        (void)_bEnable;
    }
    //-------------------------------------------------------------------------
    sal_Bool OAdabasStatPageDlg::saveDatasource()
    {
        return PrepareLeaveCurrentPage();
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................
