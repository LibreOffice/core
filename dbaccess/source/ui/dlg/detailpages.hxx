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
#include <charsets.hxx>
#include <charsetlistbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include "TextConnectionHelper.hxx"
#include "admincontrols.hxx"

#include <svtools/dialogcontrolling.hxx>
#include <o3tl/typed_flags_set.hxx>

enum class OCommonBehaviourTabPageFlags {
    NONE        = 0x0000,
    UseCharset  = 0x0002,
    UseOptions  = 0x0004,
};
namespace o3tl {
    template<> struct typed_flags<OCommonBehaviourTabPageFlags> : is_typed_flags<OCommonBehaviourTabPageFlags, 0x0006> {};
}

namespace dbaui
{
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

        OCommonBehaviourTabPageFlags m_nControlFlags;

    public:
        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) override;

        OCommonBehaviourTabPage(vcl::Window* pParent, const OString& rId, const OUString& rUIXMLDescription, const SfxItemSet& _rCoreAttrs, OCommonBehaviourTabPageFlags nControlFlags);
    protected:

        virtual ~OCommonBehaviourTabPage() override;
        virtual void dispose() override;

        // subclasses must override this, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    private:
        DECL_LINK(CharsetSelectHdl, ListBox&, void);
    };

    class DBOCommonBehaviourTabPage : public OGenericAdministrationPage
    {
    protected:

        OCommonBehaviourTabPageFlags m_nControlFlags;

        std::unique_ptr<weld::Label> m_xOptionsLabel;
        std::unique_ptr<weld::Entry> m_xOptions;

        std::unique_ptr<weld::Label> m_xDataConvertLabel;
        std::unique_ptr<weld::Label> m_xCharsetLabel;
        std::unique_ptr<DBCharSetListBox> m_xCharset;

        std::unique_ptr<weld::CheckButton> m_xAutoRetrievingEnabled;
        std::unique_ptr<weld::Label> m_xAutoIncrementLabel;
        std::unique_ptr<weld::Entry> m_xAutoIncrement;
        std::unique_ptr<weld::Label> m_xAutoRetrievingLabel;
        std::unique_ptr<weld::Entry> m_xAutoRetrieving;

    public:
        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) override;

        DBOCommonBehaviourTabPage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rId, const SfxItemSet& _rCoreAttrs, OCommonBehaviourTabPageFlags nControlFlags);
    protected:

        virtual ~DBOCommonBehaviourTabPage() override;
        virtual void dispose() override;

        // subclasses must override this, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    private:
        DECL_LINK(CharsetSelectHdl, weld::ComboBox&, void);
    };


    // ODbaseDetailsPage
    class ODbaseDetailsPage : public DBOCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        ODbaseDetailsPage(TabPageParent pParent, const SfxItemSet& _rCoreAttrs);
        virtual ~ODbaseDetailsPage() override;
    private:
        OUString            m_sDsn;

        std::unique_ptr<weld::CheckButton> m_xShowDeleted;
        std::unique_ptr<weld::Label> m_xFT_Message;
        std::unique_ptr<weld::Button> m_xIndexes;

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;

    private:
        DECL_LINK(OnButtonClicked, weld::Button&, void);
    };

    // OAdoDetailsPage
    class OAdoDetailsPage : public DBOCommonBehaviourTabPage
    {
    public:
        OAdoDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs);
    };

    // OOdbcDetailsPage
    class OOdbcDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        OOdbcDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        virtual ~OOdbcDetailsPage() override;
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
        virtual ~OUserDriverDetailsPage() override;
        virtual void dispose() override;
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
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
    class OGeneralSpecialJDBCDetailsPage final : public OCommonBehaviourTabPage
    {
    public:
        OGeneralSpecialJDBCDetailsPage(   vcl::Window* pParent
                                        , const SfxItemSet& _rCoreAttrs
                                        , sal_uInt16 _nPortId
                                        , bool bShowSocket = true
                                        );
        virtual ~OGeneralSpecialJDBCDetailsPage() override;
        virtual void dispose() override;

    private:

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void callModifiedHdl(void* pControl = nullptr) override;

        DECL_LINK(OnTestJavaClickHdl, Button*, void);

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
        virtual ~MySQLNativePage() override;
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
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
    };

    // OOdbcDetailsPage
    class OLDAPDetailsPage : public DBOCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        OLDAPDetailsPage(TabPageParent pParent, const SfxItemSet& rCoreAttrs);
        virtual ~OLDAPDetailsPage() override;
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
    private:
        sal_Int32 m_iSSLPort;
        sal_Int32 m_iNormalPort;

        std::unique_ptr<weld::Entry> m_xETBaseDN;
        std::unique_ptr<weld::CheckButton> m_xCBUseSSL;
        std::unique_ptr<weld::SpinButton> m_xNFPortNumber;
        std::unique_ptr<weld::SpinButton> m_xNFRowCount;

        DECL_LINK(OnCheckBoxClick, weld::ToggleButton&, void);
    };

    // OTextDetailsPage
    class OTextDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) override;

        OTextDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        VclPtr<OTextConnectionHelper>  m_pTextConnectionHelper;

    protected:
        virtual ~OTextDetailsPage() override;
        virtual void dispose() override;
        virtual bool prepareLeave() override;

        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) override;
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

    private:
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DETAILPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
