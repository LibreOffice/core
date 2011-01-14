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

#ifndef DBAUI_DBSETUPCONNECTIONPAGES_HXX
#define DBAUI_DBSETUPCONNECTIONPAGES_HXX

#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#include "ConnectionPageSetup.hxx"
#endif

#include "adminpages.hxx"
#include "admincontrols.hxx"
#include "curledit.hxx"
#include "TextConnectionHelper.hxx"

#include <svtools/roadmapwizard.hxx>
#include <ucbhelper/content.hxx>
#include <vcl/field.hxx>


//.........................................................................
namespace dbaui

{
//.........................................................................

    class IDatabaseSettingsDialog;
//      static  OGenericAdministrationPage* CreateDbaseTabPage( Window* pParent, const SfxItemSet& _rAttrSet );


       //========================================================================
    //= OSpreadSheetConnectionPageSetup
    //========================================================================
    class OSpreadSheetConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
        virtual sal_Bool        FillItemSet ( SfxItemSet& _rCoreAttrs );
        static  OGenericAdministrationPage* CreateSpreadSheetTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        OSpreadSheetConnectionPageSetup(Window* pParent, const SfxItemSet& _rCoreAttrs);

    protected:
        CheckBox m_aCBPasswordrequired;
        virtual ~OSpreadSheetConnectionPageSetup();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    };

    //========================================================================
    //= OTextConnectionPage
    //========================================================================
    class OTextConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
        virtual sal_Bool        FillItemSet ( SfxItemSet& _rCoreAttrs );
           static   OGenericAdministrationPage* CreateTextTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        OTextConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs );
        OTextConnectionHelper*  m_pTextConnectionHelper;
    private:

    protected:
        virtual ~OTextConnectionPageSetup();
        virtual sal_Bool prepareLeave();
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        bool    checkTestConnection();

    private:
        DECL_LINK(ImplGetExtensionHdl, OTextConnectionHelper*);
    };

    //========================================================================
    //= OLDAPConnectionPageSetup
    //========================================================================
    class OLDAPConnectionPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual sal_Bool        FillItemSet ( SfxItemSet& _rCoreAttrs );
        static  OGenericAdministrationPage* CreateLDAPTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        OLDAPConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual Link getControlModifiedLink() { return LINK(this, OLDAPConnectionPageSetup, OnEditModified); }

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        DECL_LINK(OnEditModified,Edit*);

    private:
        FixedText           m_aFTHeaderText;
        FixedText           m_aFTHelpText;
        FixedText           m_aFTHostServer;
        Edit                m_aETHostServer;
        FixedText           m_aFTBaseDN;
        Edit                m_aETBaseDN;
        FixedText           m_aFTPortNumber;
        NumericField        m_aNFPortNumber;
        FixedText           m_aFTDefaultPortNumber;
        CheckBox            m_aCBUseSSL;
    };

    //========================================================================
    //= MySQLNativeSetupPage
    //========================================================================
    class MySQLNativeSetupPage : public OGenericAdministrationPage
    {
    private:
        FixedText           m_aHeader;
        FixedText           m_aHelpText;
        MySQLNativeSettings m_aMySQLSettings;

    public:
        MySQLNativeSetupPage( Window* _pParent, const SfxItemSet& _rCoreAttrs );

        static OGenericAdministrationPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual void fillControls( ::std::vector< ISaveValueWrapper* >& _rControlList );
        virtual void fillWindows( ::std::vector< ISaveValueWrapper* >& _rControlList );

        virtual sal_Bool FillItemSet( SfxItemSet& _rCoreAttrs );
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        virtual Link getControlModifiedLink();

    private:
        DECL_LINK( OnModified, Edit* );
    };

    //========================================================================
    //= OGeneralSpecialJDBCConnectionPageSetup
    //========================================================================
    class OGeneralSpecialJDBCConnectionPageSetup : public OGenericAdministrationPage
    {
    public:
        OGeneralSpecialJDBCConnectionPageSetup(   Window* pParent
                                        , sal_uInt16 _nResId
                                        , const SfxItemSet& _rCoreAttrs
                                        , sal_uInt16 _nPortId
                                        , sal_uInt16 _nDefaultPortResId
                                        , sal_uInt16 _nHelpTextResId
                                        , sal_uInt16 _nHeaderTextResId
                                        , sal_uInt16 _nDriverClassId );
    static  OGenericAdministrationPage* CreateMySQLJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
    static  OGenericAdministrationPage* CreateOracleJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual sal_Bool FillItemSet( SfxItemSet& _rCoreAttrs );
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual Link getControlModifiedLink() { return LINK(this, OGeneralSpecialJDBCConnectionPageSetup, OnEditModified); }

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);
        FixedText           m_aFTHelpText;
        FixedText           m_aFTDatabasename;
        Edit                m_aETDatabasename;
        FixedText           m_aFTHostname;
        Edit                m_aETHostname;
        FixedText           m_aFTPortNumber;
        FixedText           m_aFTDefaultPortNumber;
        NumericField        m_aNFPortNumber;


        FixedText           m_aFTDriverClass;
        Edit                m_aETDriverClass;
        PushButton          m_aPBTestJavaDriver;

        String              m_sDefaultJdbcDriverName;
        sal_uInt16              m_nPortId;
    };


    //========================================================================
    //= OJDBCConnectionPageSetup
    //========================================================================
    class OJDBCConnectionPageSetup : public OConnectionTabPageSetup
    {
    public:
                OJDBCConnectionPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs );
        static  OGenericAdministrationPage* CreateJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet );

    protected:
        virtual bool checkTestConnection();

        virtual sal_Bool FillItemSet( SfxItemSet& _rCoreAttrs );
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);
        FixedText           m_aFTDriverClass;
        Edit                m_aETDriverClass;
        PushButton          m_aPBTestJavaDriver;
    };



    //========================================================================
    //= OJDBCConnectionPageSetup
    //========================================================================
    class OMySQLIntroPageSetup : public OGenericAdministrationPage
    {
    public:
        enum ConnectionType
        {
            VIA_ODBC,
            VIA_JDBC,
            VIA_NATIVE
        };

        OMySQLIntroPageSetup( Window* pParent, const SfxItemSet& _rCoreAttrs);

        static OMySQLIntroPageSetup*    CreateMySQLIntroTabPage( Window* _pParent, const SfxItemSet& _rAttrSet );
        ConnectionType      getMySQLMode();
        Link                maClickHdl;
        void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
        const Link&         GetClickHdl() const { return maClickHdl; }
        DECL_LINK(ImplClickHdl, OMySQLIntroPageSetup*);




    protected:
        virtual sal_Bool FillItemSet(SfxItemSet& _rSet);
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual ~OMySQLIntroPageSetup();

    private:
        RadioButton         m_aRB_ODBCDatabase;
        RadioButton         m_aRB_JDBCDatabase;
        RadioButton         m_aRB_NATIVEDatabase;
        FixedText           m_aFT_ConnectionMode;
        FixedText           m_aFT_Helptext;
        FixedText           m_aFT_Headertext;

        DECL_LINK(OnSetupModeSelected, RadioButton*);

    };




       //========================================================================
    //= OAuthentificationPageSetup
    //========================================================================
    class OAuthentificationPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual sal_Bool        FillItemSet ( SfxItemSet& _rCoreAttrs );
        static  OGenericAdministrationPage* CreateAuthentificationTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        OAuthentificationPageSetup(Window* pParent, const SfxItemSet& _rCoreAttrs);

    protected:
        FixedText   m_aFTHelpText;
        FixedText   m_aFTHeaderText;
        FixedText   m_aFTUserName;
        Edit        m_aETUserName;
        CheckBox    m_aCBPasswordRequired;
        PushButton  m_aPBTestConnection;
        virtual ~OAuthentificationPageSetup();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
    };



       //========================================================================
    //= OFinalDBPageSetup
    //========================================================================
    class OFinalDBPageSetup : public OGenericAdministrationPage
    {
    public:
        virtual sal_Bool        FillItemSet ( SfxItemSet& _rCoreAttrs );
           static   OGenericAdministrationPage* CreateFinalDBTabPageSetup( Window* pParent, const SfxItemSet& _rAttrSet);

        FixedText   m_aFTFinalHeader;
        FixedText   m_aFTFinalHelpText;
        RadioButton m_aRBRegisterDataSource;
        RadioButton m_aRBDontregisterDataSource;
        FixedText   m_aFTAdditionalSettings;
        CheckBox    m_aCBOpenAfterwards;
        CheckBox    m_aCBStartTableWizard;
        FixedText   m_aFTFinalText;

        OFinalDBPageSetup(Window* pParent, const SfxItemSet& _rCoreAttrs);
        sal_Bool IsDatabaseDocumentToBeRegistered();
        sal_Bool IsDatabaseDocumentToBeOpened();
        sal_Bool IsTableWizardToBeStarted();
        void enableTableWizardCheckBox( sal_Bool _bSupportsTableCreation);

        /// may be used in SetXXXHdl calls to controls, is a link to <method>OnControlModified</method>
        Link getControlModifiedLink() { return LINK(this, OGenericAdministrationPage, OnControlModified); }

        DECL_LINK(OnOpenSelected, CheckBox*);
    protected:
        virtual ~OFinalDBPageSetup();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif
