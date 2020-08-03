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

#pragma once

#include "ConnectionPageSetup.hxx"

#include "adminpages.hxx"
#include "admincontrols.hxx"
#include "TextConnectionHelper.hxx"

namespace dbaui
{
    class ODbTypeWizDialogSetup;

    // OSpreadSheetConnectionPageSetup
    class OSpreadSheetConnectionPageSetup final : public OConnectionTabPageSetup
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static std::unique_ptr<OGenericAdministrationPage> CreateDocumentOrSpreadSheetTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet);
        OSpreadSheetConnectionPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs);
        virtual ~OSpreadSheetConnectionPageSetup() override;

    private:
        std::unique_ptr<weld::CheckButton> m_xPasswordrequired;

        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    };

    // OTextConnectionPage
    class OTextConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static std::unique_ptr<OGenericAdministrationPage> CreateTextTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet );
        OTextConnectionPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs);
        virtual ~OTextConnectionPageSetup() override;
    protected:
        virtual bool prepareLeave() override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        bool    checkTestConnection() override;
    private:
        DECL_LINK(ImplGetExtensionHdl, OTextConnectionHelper*, void);
        std::unique_ptr<weld::Widget> m_xSubContainer;
        std::unique_ptr<OTextConnectionHelper> m_xTextConnectionHelper;
    };

    // OLDAPConnectionPageSetup
    class OLDAPConnectionPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static std::unique_ptr<OGenericAdministrationPage> CreateLDAPTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet );
        OLDAPConnectionPageSetup( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs );
        virtual ~OLDAPConnectionPageSetup() override;
        virtual void callModifiedHdl(weld::Widget* pControl = nullptr) override;

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

    private:
        std::unique_ptr<weld::Label> m_xFTHelpText;
        std::unique_ptr<weld::Label> m_xFTHostServer;
        std::unique_ptr<weld::Entry> m_xETHostServer;
        std::unique_ptr<weld::Label> m_xFTBaseDN;
        std::unique_ptr<weld::Entry> m_xETBaseDN;
        std::unique_ptr<weld::Label> m_xFTPortNumber;
        std::unique_ptr<weld::SpinButton> m_xNFPortNumber;
        std::unique_ptr<weld::Label> m_xFTDefaultPortNumber;
        std::unique_ptr<weld::CheckButton> m_xCBUseSSL;
    };

    // MySQLNativeSetupPage
    class MySQLNativeSetupPage : public OGenericAdministrationPage
    {
    private:
        std::unique_ptr<weld::Label> m_xHelpText;
        std::unique_ptr<weld::Container> m_xSettingsContainer;
        std::unique_ptr<MySQLNativeSettings> m_xMySQLSettings;

    public:
        MySQLNativeSetupPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs);
        virtual ~MySQLNativeSetupPage() override;

        static std::unique_ptr<OGenericAdministrationPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet);

    protected:
        virtual void fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList ) override;
        virtual void fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList ) override;

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        virtual void callModifiedHdl(weld::Widget* pControl = nullptr) override;
    };

    // OGeneralSpecialJDBCConnectionPageSetup
    class OGeneralSpecialJDBCConnectionPageSetup final : public OGenericAdministrationPage
    {
    public:
        OGeneralSpecialJDBCConnectionPageSetup(weld::Container* pPage, weld::DialogController* pController
                                        , const SfxItemSet& _rCoreAttrs
                                        , sal_uInt16 _nPortId
                                        , const char* pDefaultPortResId
                                        , const char* pHelpTextResId
                                        , const char* pHeaderTextResId
                                        , const char* pDriverClassId );
    virtual ~OGeneralSpecialJDBCConnectionPageSetup() override;
    static std::unique_ptr<OGenericAdministrationPage> CreateMySQLJDBCTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet );
    static std::unique_ptr<OGenericAdministrationPage> CreateOracleJDBCTabPage( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet );

    private:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void callModifiedHdl(weld::Widget* pControl = nullptr) override;

        DECL_LINK(OnTestJavaClickHdl, weld::Button&, void);

        OUString              m_sDefaultJdbcDriverName;
        sal_uInt16              m_nPortId;

        std::unique_ptr<weld::Label> m_xHeaderText;
        std::unique_ptr<weld::Label> m_xFTHelpText;
        std::unique_ptr<weld::Label> m_xFTDatabasename;
        std::unique_ptr<weld::Entry> m_xETDatabasename;
        std::unique_ptr<weld::Label> m_xFTHostname;
        std::unique_ptr<weld::Entry> m_xETHostname;
        std::unique_ptr<weld::Label> m_xFTPortNumber;
        std::unique_ptr<weld::Label> m_xFTDefaultPortNumber;
        std::unique_ptr<weld::SpinButton> m_xNFPortNumber;

        std::unique_ptr<weld::Label> m_xFTDriverClass;
        std::unique_ptr<weld::Entry> m_xETDriverClass;
        std::unique_ptr<weld::Button> m_xPBTestJavaDriver;
    };

    // OJDBCConnectionPageSetup
    class OJDBCConnectionPageSetup final : public OConnectionTabPageSetup
    {
    public:
        OJDBCConnectionPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs);
        virtual ~OJDBCConnectionPageSetup() override;
        static std::unique_ptr<OGenericAdministrationPage> CreateJDBCTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttrSet);

    private:
        virtual bool checkTestConnection() override;

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

        DECL_LINK(OnTestJavaClickHdl, weld::Button&, void);
        DECL_LINK(OnEditModified, weld::Entry&, void);
        std::unique_ptr<weld::Label> m_xFTDriverClass;
        std::unique_ptr<weld::Entry> m_xETDriverClass;
        std::unique_ptr<weld::Button> m_xPBTestJavaDriver;
    };

    // OMySQLIntroPageSetup
    class OMySQLIntroPageSetup : public OGenericAdministrationPage
    {
    public:
        enum ConnectionType
        {
            VIA_ODBC,
            VIA_JDBC,
            VIA_NATIVE
        };

        OMySQLIntroPageSetup(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& rCoreAttrs);
        virtual ~OMySQLIntroPageSetup() override;

        static std::unique_ptr<OMySQLIntroPageSetup> CreateMySQLIntroTabPage(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& rAttrSet);
        ConnectionType      getMySQLMode() const;
        void                SetClickHdl( const Link<OMySQLIntroPageSetup *, void>& rLink ) { maClickHdl = rLink; }

    protected:
        virtual bool FillItemSet(SfxItemSet* _rSet) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

    private:
        std::unique_ptr<weld::RadioButton> m_xODBCDatabase;
        std::unique_ptr<weld::RadioButton> m_xJDBCDatabase;
        std::unique_ptr<weld::RadioButton> m_xNATIVEDatabase;
        Link<OMySQLIntroPageSetup *, void> maClickHdl;

        DECL_LINK(OnSetupModeSelected, weld::ToggleButton&, void);
    };

    // OAuthentificationPageSetup
    class OAuthentificationPageSetup final : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static std::unique_ptr<OGenericAdministrationPage> CreateAuthentificationTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rAttrSet);
        OAuthentificationPageSetup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rCoreAttrs);
        virtual ~OAuthentificationPageSetup() override;

    private:
        std::unique_ptr<weld::Label> m_xFTHelpText;
        std::unique_ptr<weld::Label> m_xFTUserName;
        std::unique_ptr<weld::Entry> m_xETUserName;
        std::unique_ptr<weld::CheckButton> m_xCBPasswordRequired;
        std::unique_ptr<weld::Button> m_xPBTestConnection;

        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    };

    // OFinalDBPageSetup
    class OFinalDBPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static std::unique_ptr<OGenericAdministrationPage> CreateFinalDBTabPageSetup(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& _rAttrSet);

        OFinalDBPageSetup(weld::Container* pPage, ODbTypeWizDialogSetup* pController, const SfxItemSet& _rCoreAttrs);
        virtual ~OFinalDBPageSetup() override;
        bool IsDatabaseDocumentToBeRegistered() const;
        bool IsDatabaseDocumentToBeOpened() const;
        bool IsTableWizardToBeStarted() const;
        void enableTableWizardCheckBox( bool _bSupportsTableCreation);

        DECL_LINK(OnOpenSelected, weld::ToggleButton&, void);
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    private:
        std::unique_ptr<weld::Label> m_xFTFinalHeader;
        std::unique_ptr<weld::Label> m_xFTFinalHelpText;
        std::unique_ptr<weld::RadioButton> m_xRBRegisterDataSource;
        std::unique_ptr<weld::RadioButton> m_xRBDontregisterDataSource;
        std::unique_ptr<weld::Label> m_xFTAdditionalSettings;
        std::unique_ptr<weld::CheckButton> m_xCBOpenAfterwards;
        std::unique_ptr<weld::CheckButton> m_xCBStartTableWizard;
        std::unique_ptr<weld::Label> m_xFTFinalText;
    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
