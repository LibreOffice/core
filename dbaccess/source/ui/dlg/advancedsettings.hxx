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
        VclPtr<CheckBox>           m_pIsSQL92Check;
        VclPtr<CheckBox>           m_pAppendTableAlias;
        VclPtr<CheckBox>           m_pAsBeforeCorrelationName;
        VclPtr<CheckBox>           m_pEnableOuterJoin;
        VclPtr<CheckBox>           m_pIgnoreDriverPrivileges;
        VclPtr<CheckBox>           m_pParameterSubstitution;
        VclPtr<CheckBox>           m_pSuppressVersionColumn;
        VclPtr<CheckBox>           m_pCatalog;
        VclPtr<CheckBox>           m_pSchema;
        VclPtr<CheckBox>           m_pIndexAppendix;
        VclPtr<CheckBox>           m_pDosLineEnds;
        VclPtr<CheckBox>           m_pCheckRequiredFields;
        VclPtr<CheckBox>           m_pIgnoreCurrency;
        VclPtr<CheckBox>           m_pEscapeDateTime;
        VclPtr<CheckBox>           m_pPrimaryKeySupport;
        VclPtr<CheckBox>           m_pRespectDriverResultSetType;

        VclPtr<FixedText>          m_pBooleanComparisonModeLabel;
        VclPtr<ListBox>            m_pBooleanComparisonMode;

        VclPtr<FixedText>          m_pMaxRowScanLabel;
        VclPtr<NumericField>       m_pMaxRowScan;

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
        virtual void dispose() SAL_OVERRIDE;

        // OGenericAdministrationPage overridables
        virtual void implInitControls (const SfxItemSet& _rSet, bool _bSaveValue ) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    private:
        void    impl_initBooleanSettings();
        DECL_LINK_TYPED(BooleanComparisonSelectHdl, ListBox&, void);
    };

    // GeneratedValuesPage
    class GeneratedValuesPage : public OGenericAdministrationPage
    {
    protected:

        VclPtr<VclFrame>   m_pAutoFrame;
        VclPtr<CheckBox>   m_pAutoRetrievingEnabled;
        VclPtr<FixedText>  m_pAutoIncrementLabel;
        VclPtr<Edit>       m_pAutoIncrement;
        VclPtr<FixedText>  m_pAutoRetrievingLabel;
        VclPtr<Edit>       m_pAutoRetrieving;

        ::svt::ControlDependencyManager
                    m_aControlDependencies;

    public:
        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) SAL_OVERRIDE;

        GeneratedValuesPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:

        // nControlFlags is a combination of the CBTP_xxx-constants
        virtual ~GeneratedValuesPage();
        virtual void dispose() SAL_OVERRIDE;

        // subclasses must override this, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

}

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_ADVANCEDSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
