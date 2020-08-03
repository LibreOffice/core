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

#pragma once

#include "adminpages.hxx"
#include <charsetlistbox.hxx>
#include <rtl/ustring.hxx>

namespace dbaui

{

    #define TC_EXTENSION    (short(0x01))   // a section specifying the extension of the files to connect to
    #define TC_SEPARATORS   (short(0x02))   // a section specifying the various separators
    #define TC_HEADER       (short(0x04))   // a section containing the "Text contains header" check box only
    #define TC_CHARSET      (short(0x08))   // not yet implemented

    class OTextConnectionHelper final
    {
    public:
        OTextConnectionHelper(weld::Widget* pParent , const short _nAvailableSections);

    private:
        OUString    m_aFieldSeparatorList;
        OUString    m_aTextSeparatorList;
        OUString    m_aTextNone;
        OUString    m_aOldExtension;
        Link<OTextConnectionHelper*, void> m_aGetExtensionHandler; /// to be called if a new type is selected

        short       m_nAvailableSections;

        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Widget> m_xContainer;
        std::unique_ptr<weld::Widget> m_xExtensionHeader;
        std::unique_ptr<weld::RadioButton> m_xAccessTextFiles;
        std::unique_ptr<weld::RadioButton> m_xAccessCSVFiles;
        std::unique_ptr<weld::RadioButton> m_xAccessOtherFiles;
        std::unique_ptr<weld::Entry> m_xOwnExtension;
        std::unique_ptr<weld::Label> m_xExtensionExample;
        std::unique_ptr<weld::Widget> m_xFormatHeader;
        std::unique_ptr<weld::Label> m_xFieldSeparatorLabel;
        std::unique_ptr<weld::ComboBox> m_xFieldSeparator;
        std::unique_ptr<weld::Label> m_xTextSeparatorLabel;
        std::unique_ptr<weld::ComboBox> m_xTextSeparator;
        std::unique_ptr<weld::Label> m_xDecimalSeparatorLabel;
        std::unique_ptr<weld::ComboBox> m_xDecimalSeparator;
        std::unique_ptr<weld::Label> m_xThousandsSeparatorLabel;
        std::unique_ptr<weld::ComboBox> m_xThousandsSeparator;
        std::unique_ptr<weld::CheckButton> m_xRowHeader;
        std::unique_ptr<weld::Widget> m_xCharSetHeader;
        std::unique_ptr<weld::Label> m_xCharSetLabel;
        std::unique_ptr<CharSetListBox> m_xCharSet;

        DECL_LINK(OnSetExtensionHdl, weld::ToggleButton&, void);
        DECL_LINK(OnEditModified, weld::Entry&, void);

        OUString    GetSeparator(const weld::ComboBox& rBox, const OUString& rList);
        void        SetSeparator(weld::ComboBox& rBox, const OUString& rList, const OUString& rVal);
        void        SetExtension(const OUString& _rVal);

    public:
        void        implInitControls(const SfxItemSet& _rSet, bool _bValid);
        void        fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList);
        void        fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList);
        void        SetClickHandler(const Link<OTextConnectionHelper*, void>& _rHandler) { m_aGetExtensionHandler = _rHandler; }
        OUString    GetExtension() const;
        bool        FillItemSet( SfxItemSet& rSet, const bool bChangedSomething );
        bool        prepareLeave();
    };
}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
