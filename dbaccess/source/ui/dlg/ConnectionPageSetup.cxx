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
#include <strings.hrc>
#include <core_resource.hxx>
#include <IItemSetHelper.hxx>
#include <svl/itemset.hxx>
#include <dsitems.hxx>
#include <svl/filenotation.hxx>
#include <com/sun/star/ucb/XProgressHandler.hpp>

namespace dbaui
{
    std::unique_ptr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateDbaseTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OConnectionTabPageSetup>( pPage, pController, "dbaccess/ui/dbwizconnectionpage.ui", "ConnectionPage", _rAttrSet, STR_DBASE_HELPTEXT, STR_DBASE_HEADERTEXT, STR_DBASE_PATH_OR_FILE );
    }

    std::unique_ptr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateMSAccessTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OConnectionTabPageSetup>( pPage, pController, "dbaccess/ui/dbwizconnectionpage.ui", "ConnectionPage", _rAttrSet, STR_MSACCESS_HELPTEXT, STR_MSACCESS_HEADERTEXT, STR_MSACCESS_MDB_FILE );
    }

    std::unique_ptr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateADOTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OConnectionTabPageSetup>( pPage, pController, "dbaccess/ui/dbwizconnectionpage.ui", "ConnectionPage", _rAttrSet, STR_ADO_HELPTEXT, STR_ADO_HEADERTEXT, STR_COMMONURL );
    }

    std::unique_ptr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateODBCTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OConnectionTabPageSetup>( pPage, pController, "dbaccess/ui/dbwizconnectionpage.ui", "ConnectionPage", _rAttrSet, STR_ODBC_HELPTEXT, STR_ODBC_HEADERTEXT, STR_NAME_OF_ODBC_DATASOURCE );
    }

    std::unique_ptr<OGenericAdministrationPage> OConnectionTabPageSetup::CreateUserDefinedTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet)
    {
        return std::make_unique<OConnectionTabPageSetup>(pPage, pController, "dbaccess/ui/dbwizconnectionpage.ui", "ConnectionPage", _rAttrSet, TranslateId(), TranslateId(), STR_COMMONURL);
    }

    OConnectionTabPageSetup::OConnectionTabPageSetup(weld::Container* pPage, weld::DialogController* pController, const OUString& _rUIXMLDescription, const OUString& _rId, const SfxItemSet& _rCoreAttrs, TranslateId pHelpTextResId, TranslateId pHeaderResId, TranslateId pUrlResId)
        : OConnectionHelper(pPage, pController, _rUIXMLDescription, _rId, _rCoreAttrs)
        , m_xHelpText(m_xBuilder->weld_label(u"helptext"_ustr))
        , m_xHeaderText(m_xBuilder->weld_label(u"header"_ustr))
    {

        if (pHelpTextResId)
        {
            OUString sHelpText = DBA_RES(pHelpTextResId);
            m_xHelpText->set_label(sHelpText);
        }
        else
            m_xHelpText->hide();

        if (pHeaderResId)
            m_xHeaderText->set_label(DBA_RES(pHeaderResId));

        if (pUrlResId)
        {
            OUString sLabelText = DBA_RES(pUrlResId);
            m_xFT_Connection->set_label(sLabelText);
        }
        else
            m_xFT_Connection->hide();

        m_xConnectionURL->connect_changed(LINK(this, OConnectionTabPageSetup, OnEditModified));

        SetRoadmapStateValue(false);
    }

    OConnectionTabPageSetup::~OConnectionTabPageSetup()
    {
    }

    void OConnectionTabPageSetup::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        m_eType = m_pAdminDialog->getDatasourceType(_rSet);
        // special handling for oracle, this can only happen
        // if the user enters the same url as used for Oracle and we are on the JDBC path
        //! TODO
        //if (  ::dbaccess::DST_ORACLE_JDBC == m_eType )
        //    m_eType =  ::dbaccess::DST_JDBC;
        if(m_pCollection->determineType(m_eType) == ::dbaccess::DST_POSTGRES){
            SetRoadmapStateValue(true);
        }

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

    bool OConnectionTabPageSetup::commitPage( ::vcl::WizardTypes::CommitPageReason /*_eReason*/ )
    {
        return commitURL();
    }

    bool OConnectionTabPageSetup::FillItemSet(SfxItemSet* _rSet)
    {
        bool bChangedSomething = false;
        fillString(*_rSet,m_xConnectionURL.get(), DSID_CONNECTURL, bChangedSomething);
        return bChangedSomething;
    }

    bool OConnectionTabPageSetup::checkTestConnection()
    {
        if ( m_pCollection->determineType(m_eType) ==  ::dbaccess::DST_POSTGRES )
            return true;
        return !m_xConnectionURL->get_visible() || !m_xConnectionURL->GetTextNoPrefix().isEmpty();
    }

    IMPL_LINK_NOARG(OConnectionTabPageSetup, OnEditModified, weld::Entry&, void)
    {
        SetRoadmapStateValue(checkTestConnection());
        callModifiedHdl();
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
