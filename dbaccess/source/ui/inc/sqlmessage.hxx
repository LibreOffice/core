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

#include <connectivity/dbexception.hxx>
#include <vcl/weld.hxx>
#include <memory>

// some forwards
namespace com::sun::star {
    namespace sdb {
        class SQLContext;
    }
    namespace sdbc {
        class SQLException;
    }
}

namespace dbaui
{

enum MessageType
{
    Info,
    Error,
    Warning,
    Query,
    AUTO
};

enum class MessBoxStyle {
    NONE              = 0x0000,
    Ok                = 0x0001,
    OkCancel          = 0x0002,
    YesNo             = 0x0004,
    YesNoCancel       = 0x0008,
    RetryCancel       = 0x0010,
    DefaultOk         = 0x0020,
    DefaultCancel     = 0x0040,
    DefaultRetry      = 0x0080,
    DefaultYes        = 0x0100,
    DefaultNo         = 0x0200,
};

}

namespace o3tl {
    template<> struct typed_flags<dbaui::MessBoxStyle> : is_typed_flags<dbaui::MessBoxStyle, 0x03ff> {};
}


namespace dbaui
{

// OSQLMessageBox
struct SQLMessageBox_Impl;
class OSQLMessageBox : public weld::DialogController
{
    std::unique_ptr<weld::MessageDialog> m_xDialog;
    std::unique_ptr<weld::Button> m_xMoreButton;
    std::unique_ptr<SQLMessageBox_Impl> m_pImpl;
    OUString m_sHelpURL;

    virtual weld::Dialog* getDialog() override { return m_xDialog.get(); }
public:
    /** display an SQLException with auto-recognizing a main and a detailed message

        The first two messages from the exception chain are used as main and detailed message (recognizing the
        detailed field of an <type scope="css::sdb">SQLContext</type>).
    */
    OSQLMessageBox(
        weld::Window* pParent,
        const dbtools::SQLExceptionInfo& _rException,
        MessBoxStyle _nStyle = MessBoxStyle::Ok | MessBoxStyle::DefaultOk,
        const OUString& _rHelpURL = OUString()
    );

    /** display a database related error message

        @param  rTitle      the title to display
        @param  rMessage    the detailed message to display
        @param  _eType      determines the image to use. AUTO is disallowed in this constructor version
    */
    OSQLMessageBox(weld::Window* pParent,
                const OUString& rTitle,
                const OUString& rMessage,
                MessBoxStyle nStyle = MessBoxStyle::Ok | MessBoxStyle::DefaultOk,
                MessageType _eType = Info,
                const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo = nullptr );

    void set_title(const OUString& rTitle) { m_xDialog->set_title(rTitle); }
    void add_button(const OUString& rText, int nResponse, const OString& rHelpId = OString()) { m_xDialog->add_button(rText, nResponse, rHelpId); }
    void set_default_response(int nResponse) { m_xDialog->set_default_response(nResponse); }

    virtual ~OSQLMessageBox() override;

private:
    void Construct(weld::Window* pParent, MessBoxStyle nStyle, MessageType eImage);

    DECL_LINK(ButtonClickHdl, weld::Button&, void);

private:
    void    impl_fillMessages();
    void    impl_createStandardButtons( MessBoxStyle _nStyle );
    void    impl_addDetailsButton();
};

// OSQLWarningBox
class OSQLWarningBox : public OSQLMessageBox
{
public:
    OSQLWarningBox( weld::Window* pParent,
                    const OUString& _rMessage,
                    MessBoxStyle _nStyle = MessBoxStyle::Ok | MessBoxStyle::DefaultOk,
                    const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo = nullptr );
};

// OSQLErrorBox
class OSQLErrorBox : public OSQLMessageBox
{
public:
    OSQLErrorBox( weld::Window* pParent,
                  const OUString& _rMessage );
};

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
