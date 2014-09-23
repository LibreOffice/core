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
        static  OGenericAdministrationPage* CreateSpreadSheetTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OSpreadSheetConnectionPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);

    protected:
        CheckBox *m_pPasswordrequired;
        virtual ~OSpreadSheetConnectionPageSetup();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    };

    // OTextConnectionPage
    class OTextConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
           static   OGenericAdministrationPage* CreateTextTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OTextConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        OTextConnectionHelper*  m_pTextConnectionHelper;
    private:

    protected:
        virtual ~OTextConnectionPageSetup();
        virtual bool prepareLeave() SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        bool    checkTestConnection() SAL_OVERRIDE;

    private:
        DECL_LINK(ImplGetExtensionHdl, OTextConnectionHelper*);
    };

    // OLDAPConnectionPageSetup
    class OLDAPConnectionPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        static  OGenericAdministrationPage* CreateLDAPTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OLDAPConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual Link getControlModifiedLink() SAL_OVERRIDE { return LINK(this, OLDAPConnectionPageSetup, OnEditModified); }

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        DECL_LINK(OnEditModified,Edit*);

    private:
        FixedText*          m_pFTHelpText;
        FixedText*          m_pFTHostServer;
        Edit*               m_pETHostServer;
        FixedText*          m_pFTBaseDN;
        Edit*               m_pETBaseDN;
        FixedText*          m_pFTPortNumber;
        NumericField*       m_pNFPortNumber;
        FixedText*          m_pFTDefaultPortNumber;
        CheckBox*           m_pCBUseSSL;
    };

    // MySQLNativeSetupPage
    class MySQLNativeSetupPage : public OGenericAdministrationPage
    {
    private:
        FixedText           *m_pHelpText;
        MySQLNativeSettings m_aMySQLSettings;

    public:
        MySQLNativeSetupPage( vcl::Window* _pParent, const SfxItemSet& _rCoreAttrs );

        static OGenericAdministrationPage* Create( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual void fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList ) SAL_OVERRIDE;
        virtual void fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList ) SAL_OVERRIDE;

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;

        virtual Link getControlModifiedLink() SAL_OVERRIDE;

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
    static  OGenericAdministrationPage* CreateMySQLJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
    static  OGenericAdministrationPage* CreateOracleJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual Link getControlModifiedLink() SAL_OVERRIDE { return LINK(this, OGeneralSpecialJDBCConnectionPageSetup, OnEditModified); }

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);
        FixedText*          m_pHeaderText;
        FixedText*          m_pFTHelpText;
        FixedText*          m_pFTDatabasename;
        Edit*               m_pETDatabasename;
        FixedText*          m_pFTHostname;
        Edit*               m_pETHostname;
        FixedText*          m_pFTPortNumber;
        FixedText*          m_pFTDefaultPortNumber;
        NumericField*       m_pNFPortNumber;

        FixedText*          m_pFTDriverClass;
        Edit*               m_pETDriverClass;
        PushButton*         m_pPBTestJavaDriver;

        OUString              m_sDefaultJdbcDriverName;
        sal_uInt16              m_nPortId;
    };

    // OJDBCConnectionPageSetup
    class OJDBCConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
                OJDBCConnectionPageSetup( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        static  OGenericAdministrationPage* CreateJDBCTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual bool checkTestConnection() SAL_OVERRIDE;

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);
        FixedText*          m_pFTDriverClass;
        Edit*               m_pETDriverClass;
        PushButton*         m_pPBTestJavaDriver;
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

        static OMySQLIntroPageSetup*    CreateMySQLIntroTabPage( vcl::Window* _pParent, const SfxItemSet& _rAttrSet );
        ConnectionType      getMySQLMode();
        Link                maClickHdl;
        void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
        const Link&         GetClickHdl() const { return maClickHdl; }
        DECL_LINK(ImplClickHdl, OMySQLIntroPageSetup*);

    protected:
        virtual bool FillItemSet(SfxItemSet* _rSet) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual ~OMySQLIntroPageSetup();

    private:
        RadioButton         *m_pODBCDatabase;
        RadioButton         *m_pJDBCDatabase;
        RadioButton         *m_pNATIVEDatabase;

        DECL_LINK(OnSetupModeSelected, RadioButton*);

    };

    // OAuthentificationPageSetup
    class OAuthentificationPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        static  OGenericAdministrationPage* CreateAuthentificationTabPage( vcl::Window* pParent, const SfxItemSet& _rAttrSet );
        OAuthentificationPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);

    protected:
        FixedText*  m_pFTHelpText;
        FixedText*  m_pFTUserName;
        Edit*       m_pETUserName;
        CheckBox*   m_pCBPasswordRequired;
        PushButton* m_pPBTestConnection;
        virtual ~OAuthentificationPageSetup();

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
           static   OGenericAdministrationPage* CreateFinalDBTabPageSetup( vcl::Window* pParent, const SfxItemSet& _rAttrSet);

        FixedText*   m_pFTFinalHeader;
        FixedText*   m_pFTFinalHelpText;
        RadioButton* m_pRBRegisterDataSource;
        RadioButton* m_pRBDontregisterDataSource;
        FixedText*   m_pFTAdditionalSettings;
        CheckBox*    m_pCBOpenAfterwards;
        CheckBox*    m_pCBStartTableWizard;
        FixedText*   m_pFTFinalText;

        OFinalDBPageSetup(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        bool IsDatabaseDocumentToBeRegistered();
        bool IsDatabaseDocumentToBeOpened();
        bool IsTableWizardToBeStarted();
        void enableTableWizardCheckBox( bool _bSupportsTableCreation);

        /// may be used in SetXXXHdl calls to controls, is a link to <method>OnControlModified</method>
        Link getControlModifiedLink() SAL_OVERRIDE { return LINK(this, OGenericAdministrationPage, OnControlModified); }

        DECL_LINK(OnOpenSelected, CheckBox*);
    protected:
        virtual ~OFinalDBPageSetup();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

}   // namespace dbaui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
