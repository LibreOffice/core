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

#include "adminpages.hxx"
#include <core_resource.hxx>
#include <dbu_dlg.hxx>
#include <IItemSetHelper.hxx>
#include <strings.hrc>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <dsitems.hxx>
#include "dsselect.hxx"
#include "odbcconfig.hxx"
#include "optionalboolitem.hxx"
#include <sqlmessage.hxx>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <comphelper/types.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;

    ISaveValueWrapper::~ISaveValueWrapper()
    {
    }

    OGenericAdministrationPage::OGenericAdministrationPage(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription, const OUString& rId, const SfxItemSet& rAttrSet)
        : SfxTabPage(pPage, pController, rUIXMLDescription, rId, &rAttrSet)
        , m_abEnableRoadmap(false)
        , m_pAdminDialog(nullptr)
        , m_pItemSetHelper(nullptr)
    {
        SetExchangeSupport();

        m_xContainer->set_size_request(m_xContainer->get_approximate_digit_width() * WIZARD_PAGE_X,
                                       m_xContainer->get_text_height() * WIZARD_PAGE_Y);
    }

    DeactivateRC OGenericAdministrationPage::DeactivatePage(SfxItemSet* _pSet)
    {
        if (_pSet)
        {
            if (!prepareLeave())
                return DeactivateRC::KeepPage;
            FillItemSet(_pSet);
        }

        return DeactivateRC::LeavePage;
    }

    void OGenericAdministrationPage::Reset(const SfxItemSet* _rCoreAttrs)
    {
        implInitControls(*_rCoreAttrs, false);
    }

    void OGenericAdministrationPage::Activate()
    {
        BuilderPage::Activate();
        OSL_ENSURE(m_pItemSetHelper,"NO ItemSetHelper set!");
        if ( m_pItemSetHelper )
            ActivatePage(*m_pItemSetHelper->getOutputSet());
    }

    void OGenericAdministrationPage::ActivatePage(const SfxItemSet& _rSet)
    {
        implInitControls(_rSet, true);
    }

    void OGenericAdministrationPage::getFlags(const SfxItemSet& _rSet, bool& _rValid, bool& _rReadonly)
    {
        const SfxBoolItem* pInvalid = _rSet.GetItem<SfxBoolItem>(DSID_INVALID_SELECTION);
        _rValid = !pInvalid || !pInvalid->GetValue();
        const SfxBoolItem* pReadonly = _rSet.GetItem<SfxBoolItem>(DSID_READONLY);
        _rReadonly = !_rValid || (pReadonly && pReadonly->GetValue());
    }

    IMPL_LINK(OGenericAdministrationPage, OnControlModified, weld::Widget*, pCtrl, void)
    {
        callModifiedHdl(pCtrl);
    }

    IMPL_LINK(OGenericAdministrationPage, OnControlModifiedButtonClick, weld::Toggleable&, rCtrl, void)
    {
        callModifiedHdl(&rCtrl);
    }

    IMPL_LINK(OGenericAdministrationPage, OnControlEntryModifyHdl, weld::Entry&, rCtrl, void)
    {
        callModifiedHdl(&rCtrl);
    }

    IMPL_LINK(OGenericAdministrationPage, OnControlSpinButtonModifyHdl, weld::SpinButton&, rCtrl, void)
    {
        callModifiedHdl(&rCtrl);
    }

    bool OGenericAdministrationPage::getSelectedDataSource(OUString& _sReturn, OUString const & _sCurr)
    {
        // collect all ODBC data source names
        std::set<OUString> aOdbcDatasources;
        OOdbcEnumeration aEnumeration;
        if (!aEnumeration.isLoaded())
        {
            // show an error message
            OUString sError(DBA_RES(STR_COULD_NOT_LOAD_ODBC_LIB));
            sError = sError.replaceFirst("#lib#", aEnumeration.getLibraryName());
            std::unique_ptr<weld::MessageDialog> xDialog(Application::CreateMessageDialog(GetFrameWeld(),
                                                         VclMessageType::Warning, VclButtonsType::Ok,
                                                         sError));
            xDialog->run();
            return false;
        }
        else
        {
            aEnumeration.getDatasourceNames(aOdbcDatasources);
            // execute the select dialog
            ODatasourceSelectDialog aSelector(GetFrameWeld(), aOdbcDatasources);
            if (!_sCurr.isEmpty())
                aSelector.Select(_sCurr);
            if (RET_OK == aSelector.run())
                _sReturn = aSelector.GetSelected();
        }
        return true;
    }

    void OGenericAdministrationPage::implInitControls(const SfxItemSet& _rSet, bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        std::vector< std::unique_ptr<ISaveValueWrapper> > aControlList;
        if ( _bSaveValue )
        {
            fillControls(aControlList);
            for( const auto& pValueWrapper : aControlList )
            {
                pValueWrapper->SaveValue();
            }
        }

        if ( bReadonly )
        {
            fillWindows(aControlList);
            for( const auto& pValueWrapper : aControlList )
            {
                pValueWrapper->Disable();
            }
        }
    }

    void OGenericAdministrationPage::initializePage()
    {
        OSL_ENSURE(m_pItemSetHelper,"NO ItemSetHelper set!");
        if ( m_pItemSetHelper )
            Reset(m_pItemSetHelper->getOutputSet());
    }
    bool OGenericAdministrationPage::commitPage( ::vcl::WizardTypes::CommitPageReason )
    {
        return true;
    }
    bool OGenericAdministrationPage::canAdvance() const
    {
        return true;
    }
    void OGenericAdministrationPage::fillBool( SfxItemSet& _rSet, const weld::CheckButton* pCheckBox, sal_uInt16 _nID, bool bOptionalBool, bool& _bChangedSomething, bool _bRevertValue )
    {
        if (!(pCheckBox && pCheckBox->get_state_changed_from_saved()))
            return;

        bool bValue = pCheckBox->get_active();
        if ( _bRevertValue )
            bValue = !bValue;

        if (bOptionalBool)
        {
            OptionalBoolItem aValue( _nID );
            if ( pCheckBox->get_state() != TRISTATE_INDET )
                aValue.SetValue( bValue );
            _rSet.Put( aValue );
        }
        else
            _rSet.Put( SfxBoolItem( _nID, bValue ) );

        _bChangedSomething = true;
    }
    void OGenericAdministrationPage::fillInt32(SfxItemSet& _rSet, const weld::SpinButton* pEdit, TypedWhichId<SfxInt32Item> _nID, bool& _bChangedSomething)
    {
        if (pEdit && pEdit->get_value_changed_from_saved())
        {
            _rSet.Put(SfxInt32Item(_nID, pEdit->get_value()));
            _bChangedSomething = true;
        }
    }
    void OGenericAdministrationPage::fillString(SfxItemSet& _rSet, const weld::Entry* pEdit, TypedWhichId<SfxStringItem> _nID, bool& _bChangedSomething)
    {
        if (pEdit && pEdit->get_value_changed_from_saved())
        {
            _rSet.Put(SfxStringItem(_nID, pEdit->get_text().trim()));
            _bChangedSomething = true;
        }
    }
    void OGenericAdministrationPage::fillString(SfxItemSet& _rSet, const dbaui::OConnectionURLEdit* pEdit, TypedWhichId<SfxStringItem> _nID, bool& _bChangedSomething)
    {
        if (pEdit && pEdit->get_value_changed_from_saved())
        {
            _rSet.Put(SfxStringItem(_nID, pEdit->GetText().trim()));
            _bChangedSomething = true;
        }
    }

    IMPL_LINK_NOARG(OGenericAdministrationPage, OnTestConnectionButtonClickHdl, weld::Button&, void)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        bool bSuccess = false;
        if ( !m_pAdminDialog )
            return;

        m_pAdminDialog->saveDatasource();
        OGenericAdministrationPage::implInitControls(*m_pItemSetHelper->getOutputSet(), true);
        bool bShowMessage = true;
        try
        {
            std::pair< Reference<XConnection>,bool> aConnectionPair = m_pAdminDialog->createConnection();
            bShowMessage = aConnectionPair.second;
            bSuccess = aConnectionPair.first.is();
            ::comphelper::disposeComponent(aConnectionPair.first);
        }
        catch(Exception&)
        {
        }
        if ( bShowMessage )
        {
            MessageType eImage = MessageType::Info;
            OUString aMessage,sTitle;
            sTitle = DBA_RES(STR_CONNECTION_TEST);
            if ( bSuccess )
            {
                aMessage = DBA_RES(STR_CONNECTION_SUCCESS);
            }
            else
            {
                eImage = MessageType::Error;
                aMessage = DBA_RES(STR_CONNECTION_NO_SUCCESS);
            }
            OSQLMessageBox aMsg(GetFrameWeld(), sTitle, aMessage, MessBoxStyle::Ok, eImage);
            aMsg.run();
        }
        if ( !bSuccess )
            m_pAdminDialog->clearPassword();
    }
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
