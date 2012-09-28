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

#ifndef _DBAUI_GENERALPAGE_HXX_
#define _DBAUI_GENERALPAGE_HXX_

#include "adminpages.hxx"
#include "opendoccontrols.hxx"
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <svtools/dialogcontrolling.hxx>
#include <memory>

//.........................................................................
namespace dbaui
{
//.........................................................................
    //=========================================================================
    //= OGeneralPage
    //=========================================================================
    class OGeneralPage : public OGenericAdministrationPage
    {
        OGeneralPage(Window* pParent, const SfxItemSet& _rItems, sal_Bool _bDBWizardMode = sal_False);
        ~OGeneralPage();

    public:
        enum CreationMode
        {
            eCreateNew,
            eConnectExternal,
            eOpenExisting
        };

        struct DocumentDescriptor
        {
            String  sURL;
            String  sFilter;
        };

    private:
        // dialog controls
        FixedText           m_aFTHeaderText;
        FixedText           m_aFTHelpText;
        FixedText           m_aFT_DatasourceTypeHeader;
        RadioButton         m_aRB_CreateDatabase;
        RadioButton         m_aRB_OpenDocument;
        RadioButton         m_aRB_GetExistingDatabase;
        FixedText           m_aFT_DocListLabel;
        ::std::auto_ptr< OpenDocumentListBox >
                            m_pLB_DocumentList;
        OpenDocumentButton  m_aPB_OpenDocument;
        FixedText           m_aTypePreLabel;
        FixedText           m_aDatasourceTypeLabel;
        ::std::auto_ptr< ListBox >
                            m_pDatasourceType;
        FixedText           m_aFTDataSourceAppendix;
        FixedText           m_aTypePostLabel;
        FixedText           m_aSpecialMessage;
        sal_Bool            m_DBWizardMode;
        String              m_sMySQLEntry;
        CreationMode        m_eOriginalCreationMode;
        DocumentDescriptor  m_aBrowsedDocument;

        ::svt::ControlDependencyManager
                            m_aControlDependencies;
        ::std::vector< ::rtl::OUString> m_aURLPrefixes;


        ::dbaccess::ODsnTypeCollection*
                            m_pCollection;  /// the DSN type collection instance
        ::rtl::OUString     m_eCurrentSelection;    /// currently selected type
        ::dbaccess::DATASOURCE_TYPE     m_eNotSupportedKnownType;   /// if a data source of an unsupported, but known type is encountered ....

        enum SPECIAL_MESSAGE
        {
            smNone,
            smUnsupportedType
        };
        SPECIAL_MESSAGE     m_eLastMessage;

        Link                m_aTypeSelectHandler;   /// to be called if a new type is selected
        Link                m_aCreationModeHandler; /// to be called if a new type is selected
        Link                m_aDocumentSelectionHandler;    /// to be called when a document in the RecentDoc list is selected
        Link                m_aChooseDocumentHandler;       /// to be called when a recent document has been definately chosen
        sal_Bool            m_bDisplayingInvalid : 1;   // the currently displayed data source is deleted
        bool                m_bInitTypeList : 1;
        bool                approveDataSourceType( const ::rtl::OUString& _sURLPrefix, String& _inout_rDisplayName );
        void                insertDatasourceTypeEntryData(const ::rtl::OUString& _sType, String sDisplayName);

    public:
        static SfxTabPage*  Create(Window* pParent, const SfxItemSet& _rAttrSet, sal_Bool _bDBWizardMode = sal_False);

        /// set a handler which gets called every time the user selects a new type
        void            SetTypeSelectHandler(const Link& _rHandler) { m_aTypeSelectHandler = _rHandler; }
        void            SetCreationModeHandler(const Link& _rHandler) { m_aCreationModeHandler = _rHandler; }
        void            SetDocumentSelectionHandler( const Link& _rHandler) { m_aDocumentSelectionHandler = _rHandler; }
        void            SetChooseDocumentHandler( const Link& _rHandler) { m_aChooseDocumentHandler = _rHandler; }
        CreationMode    GetDatabaseCreationMode() const;

        DocumentDescriptor  GetSelectedDocument() const;

        /// get the currently selected datasource type
        ::rtl::OUString     GetSelectedType() const { return m_eCurrentSelection; }

    protected:
        // SfxTabPage overridables
        virtual sal_Bool FillItemSet(SfxItemSet& _rCoreAttrs);
        virtual void Reset(const SfxItemSet& _rCoreAttrs);

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        virtual void GetFocus();

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    protected:

        void onTypeSelected(const ::rtl::OUString& _sURLPrefix);
        void initializeTypeList();

        void implSetCurrentType( const ::rtl::OUString& _eType );

        void switchMessage(const ::rtl::OUString& _sURLPrefix);

        /// sets the the title of the parent dialog
        void setParentTitle(const ::rtl::OUString& _sURLPrefix);

        DECL_LINK(OnDatasourceTypeSelected, ListBox*);
        DECL_LINK(OnSetupModeSelected, RadioButton*);
        DECL_LINK(OnDocumentSelected, ListBox*);
        DECL_LINK(OnOpenDocument, PushButton*);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................
#endif // _DBAUI_GENERALPAGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
