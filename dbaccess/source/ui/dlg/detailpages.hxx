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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_DETAILPAGES_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_DETAILPAGES_HXX

#include "adminpages.hxx"
#include "charsets.hxx"
#include "charsetlistbox.hxx"
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include "TextConnectionHelper.hxx"
#include "admincontrols.hxx"

#include <svtools/dialogcontrolling.hxx>

namespace dbaui
{
    // OCommonBehaviourTabPage
    #define     CBTP_NONE                           0x00000000
    #define     CBTP_USE_CHARSET                    0x00000002
    #define     CBTP_USE_OPTIONS                    0x00000004

    /** eases the implementation of tab pages handling user/password and/or character
        set and/or generic options input
        <BR>
        The controls to be used have to be defined within the resource, as usual, but
        this class does all the handling necessary.
    */
    class OCommonBehaviourTabPage : public OGenericAdministrationPage
    {
    protected:

        VclPtr<FixedText>          m_pOptionsLabel;
        VclPtr<Edit>               m_pOptions;

        VclPtr<FixedText>          m_pCharsetLabel;
        VclPtr<CharSetListBox>     m_pCharset;

        VclPtr<CheckBox>           m_pAutoRetrievingEnabled;
        VclPtr<FixedText>          m_pAutoIncrementLabel;
        VclPtr<Edit>               m_pAutoIncrement;
        VclPtr<FixedText>          m_pAutoRetrievingLabel;
        VclPtr<Edit>               m_pAutoRetrieving;

        sal_uInt32          m_nControlFlags;

    public:
        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) override;

        // nControlFlags is a combination of the CBTP_xxx-constants
        OCommonBehaviourTabPage(vcl::Window* pParent, const OString& rId, const OUString& rUIXMLDescription, const SfxItemSet& _rCoreAttrs, sal_uInt32 nControlFlags);
    protected:

        virtual ~OCommonBehaviourTabPage();
        virtual void dispose() override;

        // subclasses must override this, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) override;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) override;
    private:
        DECL_LINK_TYPED(CharsetSelectHdl, ListBox&, void);
    };

    // ODbaseDetailsPage
    class ODbaseDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        ODbaseDetailsPage(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~ODbaseDetailsPage();
        virtual void dispose() override;
    private:
        VclPtr<CheckBox>           m_pShowDeleted;
        VclPtr<FixedText>          m_pFT_Message;
        VclPtr<PushButton>         m_pIndexes;

        OUString            m_sDsn;

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

    private:
        DECL_LINK_TYPED( OnButtonClicked, Button *, void );
    };

    // OAdoDetailsPage
    class OAdoDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        OAdoDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    // OOdbcDetailsPage
    class OOdbcDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        OOdbcDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OOdbcDetailsPage();
        virtual void dispose() override;
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
    private:
        VclPtr<CheckBox>           m_pUseCatalog;
    };

    // OUserDriverDetailsPage
    class OUserDriverDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        OUserDriverDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OUserDriverDetailsPage();
        virtual void dispose() override;
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) override;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) override;
    private:
        VclPtr<FixedText>          m_pFTHostname;
        VclPtr<Edit>               m_pEDHostname;
        VclPtr<FixedText>          m_pPortNumber;
        VclPtr<NumericField>       m_pNFPortNumber;
        VclPtr<CheckBox>           m_pUseCatalog;
    };

    // OMySQLODBCDetailsPage
    class OMySQLODBCDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        OMySQLODBCDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    // OGeneralSpecialJDBCDetailsPage
    class OGeneralSpecialJDBCDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        OGeneralSpecialJDBCDetailsPage(   vcl::Window* pParent
                                        , const SfxItemSet& _rCoreAttrs
                                        , sal_uInt16 _nPortId
                                        , bool bShowSocket = true
                                        );
        virtual ~OGeneralSpecialJDBCDetailsPage();
        virtual void dispose() override;

    protected:

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void callModifiedHdl(void* pControl = nullptr) override;

        DECL_LINK_TYPED(OnTestJavaClickHdl, Button*, void);

        VclPtr<Edit>               m_pEDHostname;
        VclPtr<NumericField>       m_pNFPortNumber;
        VclPtr<FixedText>          m_pFTSocket;
        VclPtr<Edit>               m_pEDSocket;

        VclPtr<FixedText>          m_pFTDriverClass;
        VclPtr<Edit>               m_pEDDriverClass;
        VclPtr<PushButton>         m_pTestJavaDriver;

        OUString                   m_sDefaultJdbcDriverName;
        sal_uInt16                 m_nPortId;
        bool                       m_bUseClass;
    };

    // MySQLNativePage
    class MySQLNativePage : public OCommonBehaviourTabPage
    {
    public:
        MySQLNativePage(    vcl::Window* pParent,
                            const SfxItemSet& _rCoreAttrs );
        virtual ~MySQLNativePage();
        virtual void dispose() override;

    private:
        VclPtr<FixedText>           m_pSeparator1;
        VclPtr<MySQLNativeSettings> m_aMySQLSettings;

        VclPtr<FixedText>           m_pSeparator2;
        VclPtr<FixedText>           m_pUserNameLabel;
        VclPtr<Edit>                m_pUserName;
        VclPtr<CheckBox>            m_pPasswordRequired;

    protected:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) override;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) override;
    };

    // OOdbcDetailsPage
    class OLDAPDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        OLDAPDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OLDAPDetailsPage();
        virtual void dispose() override;
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
    private:
        VclPtr<Edit>               m_pETBaseDN;
        VclPtr<CheckBox>           m_pCBUseSSL;
        VclPtr<NumericField>       m_pNFPortNumber;
        VclPtr<NumericField>       m_pNFRowCount;

        sal_Int32           m_iSSLPort;
        sal_Int32           m_iNormalPort;
        DECL_LINK_TYPED( OnCheckBoxClick, Button*, void );
    };

    // OTextDetailsPage
    class OTextDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        OTextDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        VclPtr<OTextConnectionHelper>  m_pTextConnectionHelper;

    protected:
        virtual ~OTextDetailsPage();
        virtual void dispose() override;
        virtual bool prepareLeave() override;

        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) override;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) override;

    private:
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DETAILPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
