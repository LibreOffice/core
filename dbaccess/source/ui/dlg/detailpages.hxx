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
        The controls to be used habe to be defined within the resource, as usual, but
        this class does all the handling necessary.
    */
    class OCommonBehaviourTabPage : public OGenericAdministrationPage
    {
    protected:

        FixedText*          m_pOptionsLabel;
        Edit*               m_pOptions;

        FixedLine*          m_pDataConvertFixedLine;
        FixedText*          m_pCharsetLabel;
        CharSetListBox*     m_pCharset;

        FixedLine*          m_pAutoFixedLine;
        CheckBox*           m_pAutoRetrievingEnabled;
        FixedText*          m_pAutoIncrementLabel;
        Edit*               m_pAutoIncrement;
        FixedText*          m_pAutoRetrievingLabel;
        Edit*               m_pAutoRetrieving;

        sal_uInt32          m_nControlFlags;

    public:
        virtual bool        FillItemSet (SfxItemSet& _rCoreAttrs) SAL_OVERRIDE;

        OCommonBehaviourTabPage(Window* pParent, sal_uInt16 nResId, const SfxItemSet& _rCoreAttrs, sal_uInt32 nControlFlags,bool _bFreeResource = true);
    protected:

            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~OCommonBehaviourTabPage();

        // must be overloaded by subclasses, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

    // ODbaseDetailsPage
    class ODbaseDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet& _rCoreAttrs ) SAL_OVERRIDE;

        ODbaseDetailsPage(Window* pParent, const SfxItemSet& _rCoreAttrs);
    private:
        // please add new controls also to <method>fillControls</method> or <method>fillWindows</method>
        CheckBox            m_aShowDeleted;
        FixedLine           m_aFL_1;
        FixedText           m_aFT_Message;
        PushButton          m_aIndexes;

        OUString            m_sDsn;

    protected:

        virtual ~ODbaseDetailsPage();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    private:
        DECL_LINK( OnButtonClicked, Button * );
    };

    // OAdoDetailsPage
    class OAdoDetailsPage : public OCommonBehaviourTabPage
    {
    protected:
        virtual ~OAdoDetailsPage();
    public:

        OAdoDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    // OOdbcDetailsPage
    class OOdbcDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet& _rCoreAttrs ) SAL_OVERRIDE;

        OOdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    private:
        FixedLine           m_aFL_1;
        CheckBox            m_aUseCatalog;
    };

    // OUserDriverDetailsPage
    class OUserDriverDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet& _rCoreAttrs ) SAL_OVERRIDE;

        OUserDriverDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    private:
        FixedText           m_aFTHostname;
        Edit                m_aEDHostname;
        FixedText           m_aPortNumber;
        NumericField        m_aNFPortNumber;
        CheckBox            m_aUseCatalog;
    };

    // OMySQLODBCDetailsPage
    class OMySQLODBCDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        OMySQLODBCDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    // OGeneralSpecialJDBCDetailsPage
    class OGeneralSpecialJDBCDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        OGeneralSpecialJDBCDetailsPage(   Window* pParent
                                        , sal_uInt16 _nResId
                                        , const SfxItemSet& _rCoreAttrs
                                        , sal_uInt16 _nPortId
                                        );

    protected:

        virtual bool FillItemSet( SfxItemSet& _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);

        FixedLine           m_aFL_1;
        FixedText           m_aFTHostname;
        Edit                m_aEDHostname;
        FixedText           m_aPortNumber;
        NumericField        m_aNFPortNumber;
        FixedText           m_aFTSocket;
        Edit                m_aEDSocket;

        FixedText           m_aFTDriverClass;
        Edit                m_aEDDriverClass;
        PushButton          m_aTestJavaDriver;

        OUString              m_sDefaultJdbcDriverName;
        sal_uInt16              m_nPortId;
        bool                m_bUseClass;
    };

    // MySQLNativePage
    class MySQLNativePage : public OCommonBehaviourTabPage
    {
    public:
        MySQLNativePage(    Window* pParent,
                            const SfxItemSet& _rCoreAttrs );

    private:
        FixedLine           m_aSeparator1;
        MySQLNativeSettings m_aMySQLSettings;

        FixedLine           m_aSeparator2;
        FixedText           m_aUserNameLabel;
        Edit                m_aUserName;
        CheckBox            m_aPasswordRequired;

    protected:
        virtual bool FillItemSet( SfxItemSet& _rCoreAttrs ) SAL_OVERRIDE;
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    };

    // OOdbcDetailsPage
    class OLDAPDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet& _rCoreAttrs ) SAL_OVERRIDE;

        OLDAPDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
    private:
        FixedLine           m_aFL_1;
        FixedText           m_aBaseDN;
        Edit                m_aETBaseDN;
        CheckBox            m_aCBUseSSL;
        FixedText           m_aPortNumber;
        NumericField        m_aNFPortNumber;
        FixedText           m_aFTRowCount;
        NumericField        m_aNFRowCount;

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

        OMozillaDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    // OTextDetailsPage
    class OTextDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual bool        FillItemSet ( SfxItemSet& _rCoreAttrs ) SAL_OVERRIDE;

        OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
        OTextConnectionHelper*  m_pTextConnectionHelper;

    private:

        OUString      m_aFieldSeparatorList;
        OUString      m_aTextSeparatorList;
        OUString      m_aTextNone;
    protected:
        virtual ~OTextDetailsPage();
        virtual sal_Bool prepareLeave() SAL_OVERRIDE;

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) SAL_OVERRIDE;
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

    private:
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DETAILPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
