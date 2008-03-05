/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DBSetupConnectionPages.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:54:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef DBAUI_DBSETUPCONNECTIONPAGES_HXX
#define DBAUI_DBSETUPCONNECTIONPAGES_HXX

#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#include "ConnectionPageSetup.hxx"
#endif

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _DBAUI_CURLEDIT_HXX_
#include "curledit.hxx"
#endif
#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#include <svtools/roadmapwizard.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef DBAUI_TEXTCONNECTIONHELPER_HXX
#include "TextConnectionHelper.hxx"
#endif


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
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );
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
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );
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
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );
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
    //= OGeneralSpecialJDBCConnectionPageSetup
    //========================================================================
    class OGeneralSpecialJDBCConnectionPageSetup : public OGenericAdministrationPage
    {
    public:
        OGeneralSpecialJDBCConnectionPageSetup(   Window* pParent
                                        , USHORT _nResId
                                        , const SfxItemSet& _rCoreAttrs
                                        , USHORT _nPortId
                                        , USHORT _nDefaultPortResId
                                        , const sal_Char* _pDriverName
                                        , USHORT _nHelpTextResId
                                        , USHORT _nHeaderTextResId
                                        , USHORT _nDriverClassId );
    static  OGenericAdministrationPage* CreateMySQLJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
    static  OGenericAdministrationPage* CreateMySQLNATIVETabPage( Window* pParent, const SfxItemSet& _rAttrSet );
    static  OGenericAdministrationPage* CreateOracleJDBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
    virtual Link getControlModifiedLink() { return LINK(this, OGeneralSpecialJDBCConnectionPageSetup, OnEditModified); }


    protected:


        virtual BOOL FillItemSet( SfxItemSet& _rCoreAttrs );
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);
        FixedText*          m_pFTHeaderText;
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
        USHORT              m_nPortId;
        bool                m_bUseClass;
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

        virtual BOOL FillItemSet( SfxItemSet& _rCoreAttrs );
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
        virtual BOOL FillItemSet(SfxItemSet& _rSet);
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
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );
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
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );
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
