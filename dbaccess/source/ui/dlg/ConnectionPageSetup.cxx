/*************************************************************************
 *
 *  $RCSfile: ConnectionPageSetup.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-10-27 12:58:48 $
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

#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#include "ConnectionPageSetup.hxx"
#endif
#ifndef _DBAUI_AUTOCONTROLS_HRC_
#include "AutoControls.hrc"
#endif
#ifndef _DBAUI_DBADMINSETUP_HRC_
#include "dbadminsetup.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
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
#include <svtools/filenotation.hxx>
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


    //========================================================================
    //= OConnectionTabPageSetup
    //========================================================================
    DBG_NAME(OConnectionTabPageSetup)
    OConnectionTabPageSetup::OConnectionTabPageSetup(Window* pParent, USHORT _rId, const SfxItemSet& _rCoreAttrs, USHORT _nHelpTextResId, USHORT _nHeaderResId, USHORT _nUrlResId, sal_Bool _bgetConnection)
        :OConnectionHelper(pParent, ModuleRes(_rId), _rCoreAttrs)
        , m_aFT_HelpText(this, ResId(FT_AUTOWIZARDHELPTEXT))
//      , m_aET_Connection(this, ResId(ET_BROWSEURL))
        , m_pCollection(NULL)
        ,m_bUserGrabFocus(sal_True)
    {
        DBG_CTOR(OConnectionTabPageSetup, NULL);
        String sHelpText = String(ModuleRes(_nHelpTextResId));
        m_aFT_HelpText.SetText(sHelpText);
        String sLabelText = String(ModuleRes(_nUrlResId));
        m_aFT_Connection.SetText(sLabelText);
        SetHeaderText(this, FT_AUTOWIZARDHEADER, _nHeaderResId);
        m_aET_Connection.SetModifyHdl(LINK(this, OConnectionTabPageSetup, OnEditModified));

        // extract the datasource type collection from the item set
        DbuTypeCollectionItem* pCollectionItem = PTR_CAST(DbuTypeCollectionItem, _rCoreAttrs.GetItem(DSID_TYPECOLLECTION));
        if (pCollectionItem)
            m_pCollection = pCollectionItem->getCollection();
        DBG_ASSERT(m_pCollection, "OConnectionTabPageSetup::OConnectionTabPageSetup : really need a DSN type collection !");
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
        OConnectionHelper::implInitControls(_rSet, _bSaveValue);
        callModifiedHdl();
    }

/*  // -----------------------------------------------------------------------
    void OConnectionTabPageSetup::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFT_Connection));
        _rControlList.push_back(new ODisableWrapper<PushButton>(&m_aPB_Connection));
    }


    // -----------------------------------------------------------------------
    void OConnectionTabPageSetup::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<Edit>(&m_aET_Connection));
    }
*/
       sal_Bool OConnectionTabPageSetup::commitPage(COMMIT_REASON _eReason)
    {
        return commitURL();
    }

    // -----------------------------------------------------------------------
    sal_Bool OConnectionTabPageSetup::FillItemSet(SfxItemSet& _rSet)
    {
        sal_Bool bChangedSomething = sal_False;
        fillString(_rSet,&m_aET_Connection, DSID_CONNECTURL, bChangedSomething);
        return bChangedSomething;
    }

    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPageSetup, OnTestConnectionClickHdl, PushButton*, _pButton)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        sal_Bool bSuccess = sal_False;
        if ( m_pAdminDialog )
        {
            m_pAdminDialog->saveDatasource();
            OGenericAdministrationPage::implInitControls(*m_pItemSetHelper->getOutputSet(), sal_True);
            try
            {
                Reference< XConnection > xConnection = m_pAdminDialog->createConnection();
                bSuccess = xConnection.is();
                ::comphelper::disposeComponent(xConnection);
            }
            catch(Exception&)
            {
            }

            if ( bSuccess )
            {
                String aMessage = String(ModuleRes(STR_CONNECTION_SUCCESS));
                String sTitle(ModuleRes(STR_CONNECTION_TEST));
                OSQLMessageBox aMsg(this,sTitle,aMessage);
                aMsg.Execute();
            }
            else
            {
                m_pAdminDialog->clearPassword();
            }
        }
        return 0L;
    }


    bool OConnectionTabPageSetup::checkTestConnection()
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        BOOL bEnableTestConnection = !m_aET_Connection.IsVisible() || (m_aET_Connection.GetTextNoPrefix().Len() != 0);
        return bEnableTestConnection;
    }



    // -----------------------------------------------------------------------
    IMPL_LINK(OConnectionTabPageSetup, OnEditModified, Edit*, _pEdit)
    {
        SetRoadmapStateValue(checkTestConnection());
        callModifiedHdl();
        return 0L;
    }

    // -----------------------------------------------------------------------
    void OConnectionTabPageSetup::toggleURLControlGroup(BOOL _bEnable)
    {
        m_aET_Connection.Enable(_bEnable);
        m_aPB_Connection.Enable(_bEnable);
        m_aFT_Connection.Enable(_bEnable);
    }


//.........................................................................
}   // namespace dbaui
//.........................................................................


