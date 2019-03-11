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

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_DOCLINKDIALOG_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_DOCLINKDIALOG_HXX

#include <vcl/weld.hxx>
#include <vcl/fixed.hxx>
#include <svtools/inettbc.hxx>

namespace svx
{
    /** dialog for editing document links associated with data sources
    */
    class ODocumentLinkDialog final : public weld::GenericDialogController
    {
        Link<const OUString&,bool>      m_aNameValidator;

        std::unique_ptr<weld::Button> m_xBrowseFile;
        std::unique_ptr<weld::Entry> m_xName;
        std::unique_ptr<weld::Button> m_xOK;
        std::unique_ptr<weld::Label> m_xAltTitle;
        std::unique_ptr<URLBox> m_xURL;

    public:
        ODocumentLinkDialog(weld::Window* pParent, bool bCreateNew);
        virtual ~ODocumentLinkDialog() override;

        // name validation has to be done by an external instance
        // the validator link gets a pointer to a String, and should return 0 if the string is not
        // acceptable
        void    setNameValidator( const Link<const OUString&,bool>& _rValidator ) { m_aNameValidator = _rValidator; }

        void    setLink( const  OUString& _rName, const   OUString& _rURL );
        void    getLink(        OUString& _rName,         OUString& _rURL ) const;

    private:
        DECL_LINK( OnEntryModified, weld::Entry&, void );
        DECL_LINK( OnComboBoxModified, weld::ComboBox&, void );
        DECL_LINK( OnBrowseFile, weld::Button&, void );
        DECL_LINK( OnOk, weld::Button&, void );

        void validate();
    };
}

#endif // INCLUDED_CUI_SOURCE_OPTIONS_DOCLINKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
