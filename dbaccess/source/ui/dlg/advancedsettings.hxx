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
#include "dsmeta.hxx"

#include <svtools/dialogcontrolling.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>

#include <vector>

namespace dbaui
{
    struct BooleanSettingDesc;
    typedef ::std::vector< BooleanSettingDesc > BooleanSettingDescs;

    // SpecialSettingsPage
    // implements the "Special Settings" page of the advanced database settings
    class SpecialSettingsPage : public OGenericAdministrationPage
    {
    protected:
        CheckBox*           m_pIsSQL92Check;
        CheckBox*           m_pAppendTableAlias;
        CheckBox*           m_pAsBeforeCorrelationName;
        CheckBox*           m_pEnableOuterJoin;
        CheckBox*           m_pIgnoreDriverPrivileges;
        CheckBox*           m_pParameterSubstitution;
        CheckBox*           m_pSuppressVersionColumn;
        CheckBox*           m_pCatalog;
        CheckBox*           m_pSchema;
        CheckBox*           m_pIndexAppendix;
        CheckBox*           m_pDosLineEnds;
        CheckBox*           m_pCheckRequiredFields;
        CheckBox*           m_pIgnoreCurrency;
        CheckBox*           m_pEscapeDateTime;
        CheckBox*           m_pPrimaryKeySupport;
        CheckBox*           m_pRespectDriverResultSetType;

        FixedText*          m_pBooleanComparisonModeLabel;
        ListBox*            m_pBooleanComparisonMode;

        FixedText*          m_pMaxRowScanLabel;
        NumericField*       m_pMaxRowScan;

        ::svt::ControlDependencyManager
                            m_aControlDependencies;

        BooleanSettingDescs m_aBooleanSettings;

        bool                m_bHasBooleanComparisonMode;
        bool                m_bHasMaxRowScan;

    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        SpecialSettingsPage(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta );

    protected:
        virtual ~SpecialSettingsPage();

        // OGenericAdministrationPage overridables
        virtual void implInitControls (const SfxItemSet& _rSet, bool _bSaveValue ) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    private:
        void    impl_initBooleanSettings();
    };

    // GeneratedValuesPage
    class GeneratedValuesPage : public OGenericAdministrationPage
    {
    protected:

        VclFrame*   m_pAutoFrame;
        CheckBox*   m_pAutoRetrievingEnabled;
        FixedText*  m_pAutoIncrementLabel;
        Edit*       m_pAutoIncrement;
        FixedText*  m_pAutoRetrievingLabel;
        Edit*       m_pAutoRetrieving;

        ::svt::ControlDependencyManager
                    m_aControlDependencies;

    public:
        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) SAL_OVERRIDE;

        GeneratedValuesPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:

        // nControlFlags is a combination of the CBTP_xxx-constants
        virtual ~GeneratedValuesPage();

        // must be overloaded by subclasses, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    private:
        DECL_LINK( OnCheckBoxClick, CheckBox * );
    };

}

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_ADVANCEDSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
