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

#include "ConnectionPageSetup.hxx"
#include "dbu_dlg.hrc"
#include <svl/itemset.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include "dsitems.hxx"
#include "dbaccess_helpid.hrc"
#include "localresaccess.hxx"
#include <osl/process.h>
#include <vcl/msgbox.hxx>
#include "dbadmin.hxx"
#include <comphelper/types.hxx>
#include <vcl/stdtext.hxx>
#include "sqlmessage.hxx"
#include "odbcconfig.hxx"
#include "dsselect.hxx"
#include <svl/filenotation.hxx>
#include "dbustrings.hrc"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include "UITools.hxx"
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include "finteraction.hxx"
#include <connectivity/CommonTools.hxx>
#include <sfx2/docfilt.hxx>
#include <vcl/mnemonic.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::dbtools;
    using namespace ::svt;

    VclPtr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateDbaseTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OConnectionTabPageSetup>::Create ( pParent, "ConnectionPage", "dbaccess/ui/dbwizconnectionpage.ui", _rAttrSet, STR_DBASE_HELPTEXT, STR_DBASE_HEADERTEXT, STR_DBASE_PATH_OR_FILE );
    }

    VclPtr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateMSAccessTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OConnectionTabPageSetup>::Create( pParent, "ConnectionPage", "dbaccess/ui/dbwizconnectionpage.ui", _rAttrSet, STR_MSACCESS_HELPTEXT, STR_MSACCESS_HEADERTEXT, STR_MSACCESS_MDB_FILE );
    }

    VclPtr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateADOTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OConnectionTabPageSetup>::Create( pParent, "ConnectionPage", "dbaccess/ui/dbwizconnectionpage.ui", _rAttrSet, STR_ADO_HELPTEXT, STR_ADO_HEADERTEXT, STR_COMMONURL );
    }

    VclPtr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateODBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OConnectionTabPageSetup>::Create( pParent, "ConnectionPage", "dbaccess/ui/dbwizconnectionpage.ui", _rAttrSet, STR_ODBC_HELPTEXT, STR_ODBC_HEADERTEXT, STR_NAME_OF_ODBC_DATASOURCE );
    }

    VclPtr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateUserDefinedTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet )
    {
        return VclPtr<OConnectionTabPageSetup>::Create( pParent, "ConnectionPage", "dbaccess/ui/dbwizconnectionpage.ui", _rAttrSet, USHRT_MAX, USHRT_MAX, STR_COMMONURL );
    }

    OConnectionTabPageSetup::OConnectionTabPageSetup(vcl::Window* pParent, const OString& _rId, const OUString& _rUIXMLDescription, const SfxItemSet& _rCoreAttrs, sal_uInt16 _nHelpTextResId, sal_uInt16 _nHeaderResId, sal_uInt16 _nUrlResId)
        :OConnectionHelper(pParent, _rId, _rUIXMLDescription, _rCoreAttrs)
        ,m_bUserGrabFocus(true)
    {
        get(m_pHelpText, "helptext");
        get(m_pHeaderText, "header");

        if ( USHRT_MAX != _nHelpTextResId )
        {
            OUString sHelpText = ModuleRes(_nHelpTextResId);
            m_pHelpText->SetText(sHelpText);
        }
        else
            m_pHelpText->Hide();

        if ( USHRT_MAX != _nHeaderResId )
            m_pHeaderText->SetText(ModuleRes(_nHeaderResId));

        if ( USHRT_MAX != _nUrlResId )
        {
            OUString sLabelText = ModuleRes(_nUrlResId);
            m_pFT_Connection->SetText(sLabelText);
        }
        else
            m_pFT_Connection->Hide();

        m_pConnectionURL->SetModifyHdl(LINK(this, OConnectionTabPageSetup, OnEditModified));

        SetRoadmapStateValue(false);
    }

    OConnectionTabPageSetup::~OConnectionTabPageSetup()
    {
        disposeOnce();
    }

    void OConnectionTabPageSetup::dispose()
    {
        m_pHelpText.clear();
        m_pHeaderText.clear();
        OConnectionHelper::dispose();
    }

    void OConnectionTabPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
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
        //  OUString sDisplayName = m_pCollection->getTypeDisplayName(m_eType);
        //  FixedText* ppTextControls[] ={&m_aFT_Connection};
        //  for (size_t i = 0; i < sizeof(ppTextControls)/sizeof(ppTextControls[0]); ++i)
        //  {
        //      ppTextControls[i]->SetText(sDisplayName);
        //  }
        //}

        callModifiedHdl();
    }

    bool OConnectionTabPageSetup::commitPage( ::svt::WizardTypes::CommitPageReason /*_eReason*/ )
    {
        return commitURL();
    }

    bool OConnectionTabPageSetup::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_pConnectionURL, DSID_CONNECTURL, bChangedSomething);
        return bChangedSomething;
    }
    bool OConnectionTabPageSetup::checkTestConnection()
    {
        return !m_pConnectionURL->IsVisible() || !m_pConnectionURL->GetTextNoPrefix().isEmpty();
    }

    IMPL_LINK_NOARG_TYPED(OConnectionTabPageSetup, OnEditModified, Edit&, void)
    {
        SetRoadmapStateValue(checkTestConnection());
        callModifiedHdl();
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
