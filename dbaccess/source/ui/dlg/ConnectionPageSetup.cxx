/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#include "ConnectionPageSetup.hxx"
#endif
#ifndef _DBAUI_AUTOCONTROLS_HRC_
#include "AutoControls.hrc"
#endif
#ifndef _DBAUI_DBADMINSETUP_HRC_
#include "dbadminsetup.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SFXITEMSET_HXX
#include <svl/itemset.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <unotools/pathoptions.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svl/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svl/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svl/intitem.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBAUI_ODBC_CONFIG_HXX_
#include "odbcconfig.hxx"
#endif
#ifndef _DBAUI_DSSELECT_HXX_
#include "dsselect.hxx"
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svl/filenotation.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
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
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif
#ifndef DBAUI_FILEPICKER_INTERACTION_HXX
#include "finteraction.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;
    using namespace ::svt;



    OGenericAdministrationPage* OConnectionTabPageSetup::CreateDbaseTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        OConnectionTabPageSetup* oDBWizardPage = new OConnectionTabPageSetup( pParent, PAGE_DBWIZARD_DBASE,    _rAttrSet, STR_DBASE_HELPTEXT,    STR_DBASE_HEADERTEXT,    STR_DBASE_PATH_OR_FILE);
        oDBWizardPage->FreeResource();
        return oDBWizardPage;
    }


    OGenericAdministrationPage* OConnectionTabPageSetup::CreateMSAccessTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        OConnectionTabPageSetup* oDBWizardPage = new OConnectionTabPageSetup( pParent, PAGE_DBWIZARD_MSACCESS, _rAttrSet, STR_MSACCESS_HELPTEXT, STR_MSACCESS_HEADERTEXT, STR_MSACCESS_MDB_FILE);
        oDBWizardPage->FreeResource();
        return oDBWizardPage;
    }

    OGenericAdministrationPage* OConnectionTabPageSetup::CreateAdabasTabPage( Window* pParent,  const SfxItemSet& _rAttrSet )
    {
        OConnectionTabPageSetup* oDBWizardPage = new OConnectionTabPageSetup( pParent, PAGE_DBWIZARD_ADABAS, _rAttrSet, STR_ADABAS_HELPTEXT, STR_ADABAS_HEADERTEXT, STR_ADABAS_DATABASE_NAME);
        oDBWizardPage->FreeResource();
        return oDBWizardPage;
    }

    OGenericAdministrationPage* OConnectionTabPageSetup::CreateADOTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        OConnectionTabPageSetup* oDBWizardPage = new OConnectionTabPageSetup( pParent, PAGE_DBWIZARD_ADO, _rAttrSet, STR_ADO_HELPTEXT, STR_ADO_HEADERTEXT, STR_COMMONURL);
        oDBWizardPage->FreeResource();
        return oDBWizardPage;
    }

    OGenericAdministrationPage* OConnectionTabPageSetup::CreateODBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        OConnectionTabPageSetup* oDBWizardPage = new OConnectionTabPageSetup( pParent, PAGE_DBWIZARD_ODBC, _rAttrSet, STR_ODBC_HELPTEXT, STR_ODBC_HEADERTEXT, STR_NAME_OF_ODBC_DATASOURCE);
        oDBWizardPage->FreeResource();
        return oDBWizardPage;
    }

    OGenericAdministrationPage* OConnectionTabPageSetup::CreateUserDefinedTabPage( Window* pParent, const SfxItemSet& _rAttrSet )
    {
        OConnectionTabPageSetup* oDBWizardPage = new OConnectionTabPageSetup( pParent, PAGE_DBWIZARD_USERDEFINED, _rAttrSet, USHRT_MAX, USHRT_MAX, STR_COMMONURL);
        oDBWizardPage->FreeResource();
        return oDBWizardPage;
    }


    //========================================================================
    //= OConnectionTabPageSetup
    //========================================================================
    DBG_NAME(OConnectionTabPageSetup)
    OConnectionTabPageSetup::OConnectionTabPageSetup(Window* pParent, sal_uInt16 _rId, const SfxItemSet& _rCoreAttrs, sal_uInt16 _nHelpTextResId, sal_uInt16 _nHeaderResId, sal_uInt16 _nUrlResId)
        :OConnectionHelper(pParent, ModuleRes(_rId), _rCoreAttrs)
        ,m_bUserGrabFocus(sal_True)
        ,m_aFT_HelpText(this, ModuleRes(FT_AUTOWIZARDHELPTEXT))
    {
        DBG_CTOR(OConnectionTabPageSetup, NULL);

        if ( USHRT_MAX != _nHelpTextResId )
        {
            String sHelpText = String(ModuleRes(_nHelpTextResId));
            m_aFT_HelpText.SetText(sHelpText);
        }
        else
            m_aFT_HelpText.Hide();


        if ( USHRT_MAX != _nHeaderResId )
            SetHeaderText(FT_AUTOWIZARDHEADER, _nHeaderResId);

        if ( USHRT_MAX != _nUrlResId )
        {
            String sLabelText = String(ModuleRes(_nUrlResId));
            m_aFT_Connection.SetText(sLabelText);
            if ( USHRT_MAX == _nHelpTextResId )
            {
                Point aPos = m_aFT_HelpText.GetPosPixel();
                Point aFTPos = m_aFT_Connection.GetPosPixel();
                Point aEDPos = m_aConnectionURL.GetPosPixel();
                Point aPBPos = m_aPB_Connection.GetPosPixel();

                aEDPos.Y() = aPos.Y() + aEDPos.Y() - aFTPos.Y();
                aPBPos.Y() = aPos.Y() + aPBPos.Y() - aFTPos.Y();
                aFTPos.Y() = aPos.Y();
                m_aFT_Connection.SetPosPixel(aFTPos);
                m_aConnectionURL.SetPosPixel(aEDPos);
                m_aPB_Connection.SetPosPixel(aPBPos);
            }
        }
        else
            m_aFT_Connection.Hide();

        m_aConnectionURL.SetModifyHdl(LINK(this, OConnectionTabPageSetup, OnEditModified));

        SetRoadmapStateValue(sal_False);
    }

    // -----------------------------------------------------------------------
    OConnectionTabPageSetup::~OConnectionTabPageSetup()
    {
        DBG_DTOR(OConnectionTabPageSetup,NULL);
    }

    // -----------------------------------------------------------------------
    void OConnectionTabPageSetup::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        m_eType = m_pAdminDialog->getDatasourceType(_rSet);
        // special handling for oracle, this can only happen
        // if the user enters the same url as used for Oracle and we are on the JDBC path
        //! TODO
        //if (  ::dbaccess::DST_ORACLE_JDBC == m_eType )
        //    m_eType =  ::dbaccess::DST_JDBC;

        OConnectionHelper::implInitControls(_rSet, _bSaveValue);

        //! TODO
        //if ( m_eType >=  ::dbaccess::DST_USERDEFINE1 )
        //{
        //  String sDisplayName = m_pCollection->getTypeDisplayName(m_eType);
        //  FixedText* ppTextControls[] ={&m_aFT_Connection};
        //  for (size_t i = 0; i < sizeof(ppTextControls)/sizeof(ppTextControls[0]); ++i)
        //  {
        //      ppTextControls[i]->SetText(sDisplayName);
        //  }
        //}

        callModifiedHdl();
    }
    // -----------------------------------------------------------------------
       sal_Bool OConnectionTabPageSetup::commitPage( ::svt::WizardTypes::CommitPageReason /*_eReason*/ )
    {
        return commitURL();
    }

    // -----------------------------------------------------------------------
    sal_Bool OConnectionTabPageSetup::FillItemSet(SfxItemSet& _rSet)
    {
        sal_Bool bChangedSomething = sal_False;
        fillString(_rSet,&m_aConnectionURL, DSID_CONNECTURL, bChangedSomething);
        return bChangedSomething;
    }
    // -----------------------------------------------------------------------
    bool OConnectionTabPageSetup::checkTestConnection()
    {
        return !m_aConnectionURL.IsVisible() || (m_aConnectionURL.GetTextNoPrefix().Len() != 0);
    }

    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPageSetup, OnEditModified, Edit*, /*_pEdit*/)
    {
        SetRoadmapStateValue(checkTestConnection());
        callModifiedHdl();
        return 0L;
    }
//.........................................................................
}   // namespace dbaui
//.........................................................................


