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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_GENERALPAGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_GENERALPAGE_HXX

#include "adminpages.hxx"
#include "opendoccontrols.hxx"
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <svtools/dialogcontrolling.hxx>

namespace dbaui
{
    // OGeneralPage
    class OGeneralPage : public OGenericAdministrationPage
    {
    protected:
        OGeneralPage( vcl::Window* pParent, const OUString& _rUIXMLDescription, const SfxItemSet& _rItems );

        OUString            m_eCurrentSelection;    /// currently selected type
        ::dbaccess::DATASOURCE_TYPE
                            m_eNotSupportedKnownType;   /// if a data source of an unsupported, but known type is encountered ....

    private:
        VclPtr<FixedText>          m_pSpecialMessage;

        enum SPECIAL_MESSAGE
        {
            smNone,
            smUnsupportedType
        };
        SPECIAL_MESSAGE     m_eLastMessage;

        Link<OGeneralPage&,void>   m_aTypeSelectHandler;   /// to be called if a new type is selected
        bool                m_bDisplayingInvalid : 1;   /// the currently displayed data source is deleted
        bool                m_bInitTypeList : 1;
        bool                approveDatasourceType( const OUString& _sURLPrefix, OUString& _inout_rDisplayName );
        void                insertDatasourceTypeEntryData( const OUString& _sType, const OUString& sDisplayName );

    protected:
        VclPtr<ListBox>            m_pDatasourceType;

        ::dbaccess::ODsnTypeCollection*
                            m_pCollection;  /// the DSN type collection instance

        ::std::vector< OUString>
                            m_aURLPrefixes;

    public:
        virtual ~OGeneralPage();
        virtual void dispose() SAL_OVERRIDE;

        /// set a handler which gets called every time the user selects a new type
        void            SetTypeSelectHandler( const Link<OGeneralPage&,void>& _rHandler ) { m_aTypeSelectHandler = _rHandler; }

        /// get the currently selected datasource type
        OUString     GetSelectedType() const { return m_eCurrentSelection; }

    protected:
        // SfxTabPage overridables
        virtual void Reset( const SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        virtual void implInitControls( const SfxItemSet& _rSet, bool _bSaveValue ) SAL_OVERRIDE;
        virtual OUString getDatasourceName( const SfxItemSet& _rSet );
        virtual bool approveDatasourceType( ::dbaccess::DATASOURCE_TYPE eType, OUString& _inout_rDisplayName );

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList) SAL_OVERRIDE;

        void onTypeSelected(const OUString& _sURLPrefix);
        void initializeTypeList();

        void implSetCurrentType( const OUString& _eType );

        void switchMessage(const OUString& _sURLPrefix);

        /// sets the title of the parent dialog
        virtual void setParentTitle( const OUString& _sURLPrefix );

        DECL_LINK_TYPED(OnDatasourceTypeSelected, ListBox&, void);
    };

    // OGeneralPageDialog
    class OGeneralPageDialog : public OGeneralPage
    {
    public:
        OGeneralPageDialog( vcl::Window* pParent, const SfxItemSet& _rItems );

    protected:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        virtual void implInitControls( const SfxItemSet& _rSet, bool _bSaveValue ) SAL_OVERRIDE;
        virtual void setParentTitle( const OUString& _sURLPrefix ) SAL_OVERRIDE;
    };

    // OGeneralPageWizard
    class OGeneralPageWizard : public OGeneralPage
    {
    public:
        OGeneralPageWizard( vcl::Window* pParent, const SfxItemSet& _rItems );
        virtual ~OGeneralPageWizard();
        virtual void dispose() SAL_OVERRIDE;
    public:
        enum CreationMode
        {
            eCreateNew,
            eConnectExternal,
            eOpenExisting
        };

        struct DocumentDescriptor
        {
            OUString  sURL;
            OUString  sFilter;
        };

    private:
        // dialog controls
        VclPtr<RadioButton>            m_pRB_CreateDatabase;
        VclPtr<RadioButton>            m_pRB_OpenExistingDatabase;
        VclPtr<RadioButton>            m_pRB_ConnectDatabase;

        VclPtr<FixedText>              m_pFT_EmbeddedDBLabel;
        VclPtr<ListBox>                m_pEmbeddedDBType;

        VclPtr<FixedText>              m_pFT_DocListLabel;
        VclPtr<OpenDocumentListBox>    m_pLB_DocumentList;
        VclPtr<OpenDocumentButton>     m_pPB_OpenDatabase;

        // state
        DocumentDescriptor             m_aBrowsedDocument;
        CreationMode                   m_eOriginalCreationMode;

        Link<OGeneralPageWizard&,void> m_aCreationModeHandler; /// to be called if a new type is selected
        Link<OGeneralPageWizard&,void> m_aDocumentSelectionHandler;    /// to be called when a document in the RecentDoc list is selected
        Link<OGeneralPageWizard&,void> m_aChooseDocumentHandler;       /// to be called when a recent document has been definitely chosen

        ::svt::ControlDependencyManager
                                m_aControlDependencies;

        bool                    m_bInitEmbeddedDBList : 1;
        void                    insertEmbeddedDBTypeEntryData( const OUString& _sType, const OUString& sDisplayName );

    public:
        void                    SetCreationModeHandler( const Link<OGeneralPageWizard&,void>& _rHandler ) { m_aCreationModeHandler = _rHandler; }
        CreationMode            GetDatabaseCreationMode() const;

        void                    SetDocumentSelectionHandler( const Link<OGeneralPageWizard&,void>& _rHandler) { m_aDocumentSelectionHandler = _rHandler; }
        void                    SetChooseDocumentHandler( const Link<OGeneralPageWizard&,void>& _rHandler) { m_aChooseDocumentHandler = _rHandler; }
        DocumentDescriptor      GetSelectedDocument() const;

    protected:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) SAL_OVERRIDE;

        virtual void GetFocus() SAL_OVERRIDE;

        virtual void implInitControls( const SfxItemSet& _rSet, bool _bSaveValue ) SAL_OVERRIDE;
        virtual OUString getDatasourceName( const SfxItemSet& _rSet ) SAL_OVERRIDE;
        virtual bool approveDatasourceType( ::dbaccess::DATASOURCE_TYPE eType, OUString& _inout_rDisplayName ) SAL_OVERRIDE;

        ::std::vector< OUString>
                            m_aEmbeddedURLPrefixes;

        OUString getEmbeddedDBName( const SfxItemSet& _rSet );
        void initializeEmbeddedDBList();

    protected:
        DECL_LINK_TYPED( OnEmbeddedDBTypeSelected, ListBox&, void );
        DECL_LINK_TYPED( OnCreateDatabaseModeSelected, Button*, void );
        DECL_LINK_TYPED( OnSetupModeSelected, Button*, void );
        DECL_LINK_TYPED( OnDocumentSelected, ListBox&, void );
        DECL_LINK_TYPED( OnOpenDocument, Button*, void );
    };

}   // namespace dbaui
#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_GENERALPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
