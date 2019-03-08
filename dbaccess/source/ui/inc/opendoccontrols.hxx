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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <rtl/ustring.hxx>
#include <map>

namespace dbaui
{

    // OpenDocumentButton
    /** a button which can be used to open a document

        The text of the button is the same as for the "Open" command in the application
        UI. Additionally, the icon for this command is also displayed on the button.
    */
    class OpenDocumentButton final : public PushButton
    {
    private:
        OUString     m_sModule;

    public:
        OpenDocumentButton( vcl::Window* _pParent, const sal_Char* _pAsciiModuleName );

    private:
        void    impl_init( const sal_Char* _pAsciiModuleName );
    };

    // OpenDocumentListBox
    class OpenDocumentListBox final : public ListBox
    {
    private:
        typedef std::pair< OUString, OUString >       StringPair;
        typedef std::map< sal_uInt16, StringPair >    MapIndexToStringPair;

        MapIndexToStringPair    m_aURLs;

    public:
        OpenDocumentListBox( vcl::Window* _pParent, const sal_Char* _pAsciiModuleName );

        OUString  GetSelectedDocumentURL() const;

    private:
        virtual void        RequestHelp( const HelpEvent& _rHEvt ) override;

        StringPair  impl_getDocumentAtIndex( sal_uInt16 _nListIndex, bool _bSystemNotation = false ) const;

        void    impl_init( const sal_Char* _pAsciiModuleName );
    };

} // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
