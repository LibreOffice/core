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

        FixedText*          m_pOptionsLabel;
        Edit*               m_pOptions;

        FixedText*          m_pCharsetLabel;
        CharSetListBox*     m_pCharset;

        CheckBox*           m_pAutoRetrievingEnabled;
        FixedText*          m_pAutoIncrementLabel;
        Edit*               m_pAutoIncrement;
        FixedText*          m_pAutoRetrievingLabel;
        Edit*               m_pAutoRetrieving;

        sal_uInt32          m_nControlFlags;

        bool                m_bDelete;

    public:
        virtual bool        FillItemSet (SfxItemSet* _rCoreAttrs) SAL_OVERRIDE;

        // nControlFlags is a combination of the CBTP_xxx-constants
        OCommonBehaviourTabPage(vcl::Window* pParent, const OString& rId, const OUString& rUIXMLDescription, const SfxItemSet& _rCoreAttrs, sal_uInt32 nControlFlags);
    protected:

        virtual ~OCommonBehaviourTabPage();

        // subclasses must override this, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

    // ODbaseDetailsPage
    class ODbaseDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        ODbaseDetailsPage(vcl::Window* pParent, const SfxItemSet& _rCoreAttrs);
    private:
        CheckBox*           m_pShowDeleted;
        FixedText*          m_pFT_Message;
        PushButton*         m_pIndexes;

        OUString            m_sDsn;

    protected:

        virtual ~ODbaseDetailsPage();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;

    private:
        DECL_LINK( OnButtonClicked, Button * );
    };

    // OAdoDetailsPage
    class OAdoDetailsPage : public OCommonBehaviourTabPage
    {
    protected:
        virtual ~OAdoDetailsPage();
    public:

        OAdoDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    // OOdbcDetailsPage
    class OOdbcDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        OOdbcDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
    private:
        CheckBox*           m_pUseCatalog;
    };

    // OUserDriverDetailsPage
    class OUserDriverDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        OUserDriverDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    private:
        FixedText*          m_pFTHostname;
        Edit*               m_pEDHostname;
        FixedText*          m_pPortNumber;
        NumericField*       m_pNFPortNumber;
        CheckBox*           m_pUseCatalog;
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

    protected:

        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);

        Edit*               m_pEDHostname;
        NumericField*       m_pNFPortNumber;
        FixedText*          m_pFTSocket;
        Edit*               m_pEDSocket;

        FixedText*          m_pFTDriverClass;
        Edit*               m_pEDDriverClass;
        PushButton*         m_pTestJavaDriver;

        OUString              m_sDefaultJdbcDriverName;
        sal_uInt16              m_nPortId;
        bool                m_bUseClass;
    };

    // MySQLNativePage
    class MySQLNativePage : public OCommonBehaviourTabPage
    {
    public:
        MySQLNativePage(    vcl::Window* pParent,
                            const SfxItemSet& _rCoreAttrs );

    private:
        FixedText           *m_pSeparator1;
        MySQLNativeSettings m_aMySQLSettings;

        FixedText           *m_pSeparator2;
        FixedText           *m_pUserNameLabel;
        Edit                *m_pUserName;
        CheckBox            *m_pPasswordRequired;

    protected:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

    // OOdbcDetailsPage
    class OLDAPDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        OLDAPDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
    private:
        Edit*               m_pETBaseDN;
        CheckBox*           m_pCBUseSSL;
        NumericField*       m_pNFPortNumber;
        NumericField*       m_pNFRowCount;

        sal_Int32           m_iSSLPort;
        sal_Int32           m_iNormalPort;
        DECL_LINK( OnCheckBoxClick, CheckBox * );
    };

    // OMozillaDetailsPage Detail page for Mozilla and Thunderbird addressbook
    class OMozillaDetailsPage : public OCommonBehaviourTabPage
    {
    protected:
        virtual ~OMozillaDetailsPage();
    public:

        OMozillaDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    // OTextDetailsPage
    class OTextDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        OTextDetailsPage( vcl::Window* pParent, const SfxItemSet& _rCoreAttrs );
        OTextConnectionHelper*  m_pTextConnectionHelper;

    protected:
        virtual ~OTextDetailsPage();
        virtual bool prepareLeave() SAL_OVERRIDE;

        virtual void implInitControls(const SfxItemSet& _rSet, bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    private:
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DETAILPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
