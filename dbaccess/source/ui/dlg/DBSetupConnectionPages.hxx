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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_DBSETUPCONNECTIONPAGES_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_DBSETUPCONNECTIONPAGES_HXX

#include "ConnectionPageSetup.hxx"

#include "adminpages.hxx"
#include "admincontrols.hxx"
#include <curledit.hxx>
#include "TextConnectionHelper.hxx"

#include <svtools/roadmapwizard.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/field.hxx>

namespace dbaui

{

    // OSpreadSheetConnectionPageSetup
    class OSpreadSheetConnectionPageSetup final : public OConnectionTabPageSetup
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static VclPtr<OGenericAdministrationPage> CreateDocumentOrSpreadSheetTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OSpreadSheetConnectionPageSetup(TabPageParent pParent, const SfxItemSet& _rCoreAttrs);
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
        static VclPtr<OGenericAdministrationPage> CreateTextTabPage(TabPageParent pParent, const SfxItemSet& _rAttrSet );
        OTextConnectionPageSetup(TabPageParent pParent, const SfxItemSet& _rCoreAttrs);
        virtual void dispose() override;
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
        static VclPtr<OGenericAdministrationPage> CreateLDAPTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OLDAPConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OLDAPConnectionPageSetup() override;
        virtual void dispose() override;
        virtual void callModifiedHdl(void* pControl = nullptr) override;

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

    private:
        VclPtr<FixedText>          m_pFTHelpText;
        VclPtr<FixedText>          m_pFTHostServer;
        VclPtr<Edit>               m_pETHostServer;
        VclPtr<FixedText>          m_pFTBaseDN;
        VclPtr<Edit>               m_pETBaseDN;
        VclPtr<FixedText>          m_pFTPortNumber;
        VclPtr<NumericField>       m_pNFPortNumber;
        VclPtr<FixedText>          m_pFTDefaultPortNumber;
        VclPtr<CheckBox>           m_pCBUseSSL;
    };

    // MySQLNativeSetupPage
    class MySQLNativeSetupPage : public OGenericAdministrationPage
    {
    private:
        VclPtr<FixedText>          m_pHelpText;
        VclPtr<MySQLNativeSettings> m_aMySQLSettings;

    public:
        MySQLNativeSetupPage( vcl::Window* _pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~MySQLNativeSetupPage() override;
        virtual void dispose() override;

        static VclPtr<OGenericAdministrationPage> Create( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual void fillControls( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList ) override;
        virtual void fillWindows( std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList ) override;

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        virtual void callModifiedHdl(void* pControl = nullptr) override;
    };

    // OGeneralSpecialJDBCConnectionPageSetup
    class OGeneralSpecialJDBCConnectionPageSetup final : public OGenericAdministrationPage
    {
    public:
        OGeneralSpecialJDBCConnectionPageSetup(   vcl::Window* pParent
                                        , const SfxItemSet& _rCoreAttrs
                                        , sal_uInt16 _nPortId
                                        , const char* pDefaultPortResId
                                        , const char* pHelpTextResId
                                        , const char* pHeaderTextResId
                                        , const char* pDriverClassId );
        virtual ~OGeneralSpecialJDBCConnectionPageSetup() override;
        virtual void dispose() override;
    static VclPtr<OGenericAdministrationPage> CreateMySQLJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
    static VclPtr<OGenericAdministrationPage> CreateOracleJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    private:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void callModifiedHdl(void* pControl = nullptr) override;

        DECL_LINK(OnTestJavaClickHdl, Button*, void);
        VclPtr<FixedText>          m_pHeaderText;
        VclPtr<FixedText>          m_pFTHelpText;
        VclPtr<FixedText>          m_pFTDatabasename;
        VclPtr<Edit>               m_pETDatabasename;
        VclPtr<FixedText>          m_pFTHostname;
        VclPtr<Edit>               m_pETHostname;
        VclPtr<FixedText>          m_pFTPortNumber;
        VclPtr<FixedText>          m_pFTDefaultPortNumber;
        VclPtr<NumericField>       m_pNFPortNumber;

        VclPtr<FixedText>          m_pFTDriverClass;
        VclPtr<Edit>               m_pETDriverClass;
        VclPtr<PushButton>         m_pPBTestJavaDriver;

        OUString              m_sDefaultJdbcDriverName;
        sal_uInt16              m_nPortId;
    };

    // OJDBCConnectionPageSetup
    class OJDBCConnectionPageSetup final : public OConnectionTabPageSetup
    {
    public:
        OJDBCConnectionPageSetup(TabPageParent pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~OJDBCConnectionPageSetup() override;
        static VclPtr<OGenericAdministrationPage> CreateJDBCTabPage(TabPageParent pParent, const SfxItemSet& rAttrSet);

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

        OMySQLIntroPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~OMySQLIntroPageSetup() override;
        virtual void dispose() override;

        static VclPtr<OMySQLIntroPageSetup> CreateMySQLIntroTabPage( vcl::Window* _pParent, const SfxItemSet& _rAttrSet );
        ConnectionType      getMySQLMode();
        void                SetClickHdl( const Link<OMySQLIntroPageSetup *, void>& rLink ) { maClickHdl = rLink; }

    protected:
        virtual bool FillItemSet(SfxItemSet* _rSet) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

    private:
        VclPtr<RadioButton>         m_pODBCDatabase;
        VclPtr<RadioButton>         m_pJDBCDatabase;
        VclPtr<RadioButton>         m_pNATIVEDatabase;
        Link<OMySQLIntroPageSetup *, void> maClickHdl;

        DECL_LINK(OnSetupModeSelected, RadioButton&, void);

    };

    // OAuthentificationPageSetup
    class OAuthentificationPageSetup final : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static VclPtr<OGenericAdministrationPage> CreateAuthentificationTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OAuthentificationPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~OAuthentificationPageSetup() override;
        virtual void dispose() override;

    private:
        VclPtr<FixedText>  m_pFTHelpText;
        VclPtr<FixedText>  m_pFTUserName;
        VclPtr<Edit>       m_pETUserName;
        VclPtr<CheckBox>   m_pCBPasswordRequired;
        VclPtr<PushButton> m_pPBTestConnection;

        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    };

    // OFinalDBPageSetup
    class OFinalDBPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;
        static VclPtr<OGenericAdministrationPage> CreateFinalDBTabPageSetup( vcl::Window* pParent, const SfxItemSet& _rAttrSet);

        OFinalDBPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~OFinalDBPageSetup() override;
        virtual void dispose() override;
        bool IsDatabaseDocumentToBeRegistered();
        bool IsDatabaseDocumentToBeOpened();
        bool IsTableWizardToBeStarted();
        void enableTableWizardCheckBox( bool _bSupportsTableCreation);

        DECL_LINK(OnOpenSelected, Button*, void);
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    private:
        VclPtr<FixedText>   m_pFTFinalHeader;
        VclPtr<FixedText>   m_pFTFinalHelpText;
        VclPtr<RadioButton> m_pRBRegisterDataSource;
        VclPtr<RadioButton> m_pRBDontregisterDataSource;
        VclPtr<FixedText>   m_pFTAdditionalSettings;
        VclPtr<CheckBox>    m_pCBOpenAfterwards;
        VclPtr<CheckBox>    m_pCBStartTableWizard;
        VclPtr<FixedText>   m_pFTFinalText;
    };

}   // namespace dbaui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
