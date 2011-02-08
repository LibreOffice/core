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

#ifndef DBACCESS_ADVANCEDSETTINGS_HXX
#define DBACCESS_ADVANCEDSETTINGS_HXX

#include "adminpages.hxx"
#include "dsmeta.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svtools/dialogcontrolling.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>

#include <vector>

//........................................................................
namespace dbaui
{
//........................................................................

    struct BooleanSettingDesc;
    typedef ::std::vector< BooleanSettingDesc > BooleanSettingDescs;

    //====================================================================
    //= SpecialSettingsPage
    //====================================================================
    /** implements the "Special Settings" page of the advanced database settings
    */
    class SpecialSettingsPage : public OGenericAdministrationPage
    {
    protected:
        FixedLine           m_aTopLine;
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
        virtual sal_Bool        FillItemSet ( SfxItemSet& _rCoreAttrs );

        SpecialSettingsPage(Window* pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta );

    protected:
        virtual ~SpecialSettingsPage();

        // OGenericAdministrationPage overridables
        virtual void implInitControls (const SfxItemSet& _rSet, sal_Bool _bSaveValue );

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        void    impl_initBooleanSettings();
    };

    //====================================================================
    //= GeneratedValuesPage
    //====================================================================
    class GeneratedValuesPage : public OGenericAdministrationPage
    {
    protected:

        FixedLine   m_aAutoFixedLine;
        CheckBox    m_aAutoRetrievingEnabled;
        FixedText   m_aAutoIncrementLabel;
        Edit        m_aAutoIncrement;
        FixedText   m_aAutoRetrievingLabel;
        Edit        m_aAutoRetrieving;

        ::svt::ControlDependencyManager
                    m_aControlDependencies;

    public:
        virtual sal_Bool        FillItemSet (SfxItemSet& _rCoreAttrs);

        GeneratedValuesPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:

            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~GeneratedValuesPage();

        // must be overloaded by subclasses, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        DECL_LINK( OnCheckBoxClick, CheckBox * );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ADVANCEDSETTINGS_HXX
