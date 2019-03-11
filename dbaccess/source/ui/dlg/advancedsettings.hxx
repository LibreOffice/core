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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_ADVANCEDSETTINGS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_ADVANCEDSETTINGS_HXX

#include "adminpages.hxx"
#include <dsmeta.hxx>

#include <vcl/field.hxx>

#include <vector>

namespace dbaui
{
    struct BooleanSettingDesc;
    typedef std::vector< BooleanSettingDesc > BooleanSettingDescs;

    // SpecialSettingsPage
    // implements the "Special Settings" page of the advanced database settings
    class SpecialSettingsPage final : public OGenericAdministrationPage
    {
        std::unique_ptr<weld::CheckButton> m_xIsSQL92Check;
        std::unique_ptr<weld::CheckButton> m_xAppendTableAlias;
        std::unique_ptr<weld::CheckButton> m_xAsBeforeCorrelationName;
        std::unique_ptr<weld::CheckButton> m_xEnableOuterJoin;
        std::unique_ptr<weld::CheckButton> m_xIgnoreDriverPrivileges;
        std::unique_ptr<weld::CheckButton> m_xParameterSubstitution;
        std::unique_ptr<weld::CheckButton> m_xSuppressVersionColumn;
        std::unique_ptr<weld::CheckButton> m_xCatalog;
        std::unique_ptr<weld::CheckButton> m_xSchema;
        std::unique_ptr<weld::CheckButton> m_xIndexAppendix;
        std::unique_ptr<weld::CheckButton> m_xDosLineEnds;
        std::unique_ptr<weld::CheckButton> m_xCheckRequiredFields;
        std::unique_ptr<weld::CheckButton> m_xIgnoreCurrency;
        std::unique_ptr<weld::CheckButton> m_xEscapeDateTime;
        std::unique_ptr<weld::CheckButton> m_xPrimaryKeySupport;
        std::unique_ptr<weld::CheckButton> m_xRespectDriverResultSetType;

        std::unique_ptr<weld::Label> m_xBooleanComparisonModeLabel;
        std::unique_ptr<weld::ComboBox> m_xBooleanComparisonMode;

        std::unique_ptr<weld::Label> m_xMaxRowScanLabel;
        std::unique_ptr<weld::SpinButton> m_xMaxRowScan;

        std::map<weld::ToggleButton*, TriState> m_aTriStates;

        BooleanSettingDescs m_aBooleanSettings;

        bool                m_bHasBooleanComparisonMode;
        bool                m_bHasMaxRowScan;

    public:
        DECL_LINK(OnToggleHdl, weld::ToggleButton&, void);
        DECL_LINK(OnTriStateToggleHdl, weld::ToggleButton&, void);

        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        SpecialSettingsPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta);

    private:
        virtual ~SpecialSettingsPage() override;

        // OGenericAdministrationPage overridables
        virtual void implInitControls (const SfxItemSet& _rSet, bool _bSaveValue ) override;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

        DECL_LINK(BooleanComparisonSelectHdl, weld::ComboBox&, void);
    };

    // GeneratedValuesPage
    class GeneratedValuesPage final : public OGenericAdministrationPage
    {
        std::unique_ptr<weld::CheckButton> m_xAutoRetrievingEnabled;
        std::unique_ptr<weld::Widget> m_xGrid;
        std::unique_ptr<weld::Label> m_xAutoIncrementLabel;
        std::unique_ptr<weld::Entry> m_xAutoIncrement;
        std::unique_ptr<weld::Label> m_xAutoRetrievingLabel;
        std::unique_ptr<weld::Entry> m_xAutoRetrieving;

    public:
        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) override;

        GeneratedValuesPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs);
    private:
        DECL_LINK(OnAutoToggleHdl, weld::ToggleButton&, void);

        // nControlFlags is a combination of the CBTP_xxx-constants
        virtual ~GeneratedValuesPage() override;

        // subclasses must override this, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_ADVANCEDSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
