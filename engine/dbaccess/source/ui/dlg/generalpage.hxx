/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include "adminpages.hxx"

namespace dbaui
{
    // OGeneralPage
    class OGeneralPage : public OGenericAdministrationPage
    {
    protected:
        OGeneralPage(weld::Container* pPage, weld::DialogController* pController, const OUString& _rUIXMLDescription, const SfxItemSet& _rItems);

        OUString            m_eCurrentSelection;    /// currently selected type

    private:
        std::unique_ptr<weld::Label> m_xSpecialMessage;

        enum SPECIAL_MESSAGE
        {
            smNone,
            smUnsupportedType
        };
        SPECIAL_MESSAGE     m_eLastMessage;

        Link<OGeneralPage&,void>   m_aTypeSelectHandler;   /// to be called if a new type is selected
        bool                m_bInitTypeList : 1;
        bool                approveDatasourceType( std::u16string_view _sURLPrefix, OUString& _inout_rDisplayName );
        void                insertDatasourceTypeEntryData( const OUString& _sType, const OUString& sDisplayName );

    protected:
        std::unique_ptr<weld::ComboBox> m_xDatasourceType;

        ::dbaccess::ODsnTypeCollection*
                            m_pCollection;  /// the DSN type collection instance

        std::vector< OUString>
                            m_aURLPrefixes;

    public:
        virtual ~OGeneralPage() override;

        /// set a handler which gets called every time the user selects a new type
        void            SetTypeSelectHandler( const Link<OGeneralPage&,void>& _rHandler ) { m_aTypeSelectHandler = _rHandler; }

        /// get the currently selected datasource type
        const OUString&  GetSelectedType() const { return m_eCurrentSelection; }

    protected:
        // SfxTabPage overridables
        virtual void Reset( const SfxItemSet* _rCoreAttrs ) override;

        virtual void implInitControls( const SfxItemSet& _rSet, bool _bSaveValue ) override;
        virtual OUString getDatasourceName( const SfxItemSet& _rSet );
        virtual bool approveDatasourceType( ::dbaccess::DATASOURCE_TYPE eType, OUString& _inout_rDisplayName );

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList) override;

        void onTypeSelected(const OUString& _sURLPrefix);

        /**
         * Initializes the listbox, which contains entries each representing a
         * connection to an existing database.
         */
        void initializeTypeList();

        void implSetCurrentType( const OUString& _eType );

        void switchMessage(std::u16string_view _sURLPrefix);

        /// sets the title of the parent dialog
        virtual void setParentTitle( const OUString& _sURLPrefix );

        DECL_LINK(OnDatasourceTypeSelected, weld::ComboBox&, void);
    };

    // OGeneralPageDialog
    class OGeneralPageDialog : public OGeneralPage
    {
    public:
        OGeneralPageDialog(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& _rItems);

    protected:
        virtual bool FillItemSet( SfxItemSet* _rCoreAttrs ) override;

        virtual void implInitControls( const SfxItemSet& _rSet, bool _bSaveValue ) override;
        virtual void setParentTitle( const OUString& _sURLPrefix ) override;
    };

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
