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
#include "curledit.hxx"
#include "TextConnectionHelper.hxx"

#include <svtools/roadmapwizard.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/field.hxx>

namespace dbaui

{

    // OSpreadSheetConnectionPageSetup
    class OSpreadSheetConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        static VclPtr<OGenericAdministrationPage> CreateSpreadSheetTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OSpreadSheetConnectionPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~OSpreadSheetConnectionPageSetup();
        virtual void dispose() SAL_OVERRIDE;

    protected:
        VclPtr<CheckBox> m_pPasswordrequired;

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    };

    // OTextConnectionPage
    class OTextConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
        VclPtr<OTextConnectionHelper>  m_pTextConnectionHelper;

        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        static VclPtr<OGenericAdministrationPage> CreateTextTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OTextConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OTextConnectionPageSetup();
        virtual void dispose() SAL_OVERRIDE;
    protected:
        virtual bool prepareLeave() SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        bool    checkTestConnection() SAL_OVERRIDE;

    private:
        DECL_LINK_TYPED(ImplGetExtensionHdl, OTextConnectionHelper*, void);
    };

    // OLDAPConnectionPageSetup
    class OLDAPConnectionPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        static VclPtr<OGenericAdministrationPage> CreateLDAPTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OLDAPConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OLDAPConnectionPageSetup();
        virtual void dispose() SAL_OVERRIDE;
        virtual Link<> getControlModifiedLink() SAL_OVERRIDE { return LINK(this, OLDAPConnectionPageSetup, OnEditModified); }

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        DECL_LINK(OnEditModified,Edit*);

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
        virtual ~MySQLNativeSetupPage();
        virtual void dispose() SAL_OVERRIDE;

        static VclPtr<OGenericAdministrationPage> Create( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual void fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList ) SAL_OVERRIDE;
        virtual void fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList ) SAL_OVERRIDE;

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;

        virtual Link<> getControlModifiedLink() SAL_OVERRIDE { return LINK( this, MySQLNativeSetupPage, OnModified ); }

    private:
        DECL_LINK( OnModified, Edit* );
    };

    // OGeneralSpecialJDBCConnectionPageSetup
    class OGeneralSpecialJDBCConnectionPageSetup : public OGenericAdministrationPage
    {
    public:
        OGeneralSpecialJDBCConnectionPageSetup(   vcl::Window* pParent
                                        , const SfxItemSet& _rCoreAttrs
                                        , sal_uInt16 _nPortId
                                        , sal_uInt16 _nDefaultPortResId
                                        , sal_uInt16 _nHelpTextResId
                                        , sal_uInt16 _nHeaderTextResId
                                        , sal_uInt16 _nDriverClassId );
        virtual ~OGeneralSpecialJDBCConnectionPageSetup();
        virtual void dispose() SAL_OVERRIDE;
    static VclPtr<OGenericAdministrationPage> CreateMySQLJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
    static VclPtr<OGenericAdministrationPage> CreateOracleJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual Link<> getControlModifiedLink() SAL_OVERRIDE { return LINK(this, OGeneralSpecialJDBCConnectionPageSetup, OnEditModified); }

        DECL_LINK_TYPED(OnTestJavaClickHdl, Button*, void);
        DECL_LINK(OnEditModified,Edit*);
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
    class OJDBCConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
                OJDBCConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OJDBCConnectionPageSetup();
        virtual void dispose() SAL_OVERRIDE;
        static VclPtr<OGenericAdministrationPage> CreateJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual bool checkTestConnection() SAL_OVERRIDE;

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        DECL_LINK_TYPED(OnTestJavaClickHdl, Button*, void);
        DECL_LINK(OnEditModified,Edit*);
        VclPtr<FixedText>          m_pFTDriverClass;
        VclPtr<Edit>               m_pETDriverClass;
        VclPtr<PushButton>         m_pPBTestJavaDriver;
    };

    // OJDBCConnectionPageSetup
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
        virtual ~OMySQLIntroPageSetup();
        virtual void dispose() SAL_OVERRIDE;

        static VclPtr<OMySQLIntroPageSetup> CreateMySQLIntroTabPage( vcl::Window* _pParent, const SfxItemSet& _rAttrSet );
        ConnectionType      getMySQLMode();
        void                SetClickHdl( const Link<OMySQLIntroPageSetup *, void>& rLink ) { maClickHdl = rLink; }

    protected:
        virtual bool FillItemSet(SfxItemSet* _rSet) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    private:
        VclPtr<RadioButton>         m_pODBCDatabase;
        VclPtr<RadioButton>         m_pJDBCDatabase;
        VclPtr<RadioButton>         m_pNATIVEDatabase;
        Link<OMySQLIntroPageSetup *, void> maClickHdl;

        DECL_LINK_TYPED(OnSetupModeSelected, RadioButton&, void);

    };

    // OAuthentificationPageSetup
    class OAuthentificationPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        static VclPtr<OGenericAdministrationPage> CreateAuthentificationTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OAuthentificationPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~OAuthentificationPageSetup();
        virtual void dispose() SAL_OVERRIDE;

    protected:
        VclPtr<FixedText>  m_pFTHelpText;
        VclPtr<FixedText>  m_pFTUserName;
        VclPtr<Edit>       m_pETUserName;
        VclPtr<CheckBox>   m_pCBPasswordRequired;
        VclPtr<PushButton> m_pPBTestConnection;

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

    // OFinalDBPageSetup
    class OFinalDBPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        static VclPtr<OGenericAdministrationPage> CreateFinalDBTabPageSetup( vcl::Window* pParent, const SfxItemSet& _rAttrSet);

        VclPtr<FixedText>   m_pFTFinalHeader;
        VclPtr<FixedText>   m_pFTFinalHelpText;
        VclPtr<RadioButton> m_pRBRegisterDataSource;
        VclPtr<RadioButton> m_pRBDontregisterDataSource;
        VclPtr<FixedText>   m_pFTAdditionalSettings;
        VclPtr<CheckBox>    m_pCBOpenAfterwards;
        VclPtr<CheckBox>    m_pCBStartTableWizard;
        VclPtr<FixedText>   m_pFTFinalText;

        OFinalDBPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~OFinalDBPageSetup();
        virtual void dispose() SAL_OVERRIDE;
        bool IsDatabaseDocumentToBeRegistered();
        bool IsDatabaseDocumentToBeOpened();
        bool IsTableWizardToBeStarted();
        void enableTableWizardCheckBox( bool _bSupportsTableCreation);

        /// may be used in SetXXXHdl calls to controls, is a link to <method>OnControlModified</method>
        Link<> getControlModifiedLink() SAL_OVERRIDE { return LINK(this, OGenericAdministrationPage, OnControlModified); }

        DECL_LINK_TYPED(OnOpenSelected, Button*, void);
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

}   // namespace dbaui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
