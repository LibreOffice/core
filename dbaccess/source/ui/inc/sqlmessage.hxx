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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_SQLMESSAGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_SQLMESSAGE_HXX

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/btndlg.hxx>
#include <vcl/msgbox.hxx>

#include <connectivity/dbexception.hxx>

#include <memory>

// some forwards
namespace com { namespace sun { namespace star {
    namespace sdb {
        class SQLContext;
    }
    namespace sdbc {
        class SQLException;
    }
} } }

namespace dbaui
{

// OSQLMessageBox
struct SQLMessageBox_Impl;
class OSQLMessageBox : public ButtonDialog
{
    VclPtr<FixedImage>      m_aInfoImage;
    VclPtr<FixedText>       m_aTitle;
    VclPtr<FixedText>       m_aMessage;
    OUString m_sHelpURL;

    std::unique_ptr< SQLMessageBox_Impl >   m_pImpl;

public:
    enum MessageType
    {
        Info,
        Error,
        Warning,
        Query,

        AUTO
    };

public:
    /** display an SQLException with auto-recognizing a main and a detailed message

        The first two messages from the exception chain are used as main and detailed message (recognizing the
        detailed field of an <type scope="css::sdb">SQLContext</type>).
    */
    OSQLMessageBox(
        vcl::Window* _pParent,
        const dbtools::SQLExceptionInfo& _rException,
        MessBoxStyle _nStyle = MessBoxStyle::Ok | MessBoxStyle::DefaultOk,
        const OUString& _rHelpURL = OUString()
    );

    /** display a database related error message

        @param  rTitle      the title to display
        @param  rMessage    the detailed message to display
        @param  _eType      determines the image to use. AUTO is disallowed in this constructor version
    */
    OSQLMessageBox(vcl::Window* pParent,
                const OUString& rTitle,
                const OUString& rMessage,
                MessBoxStyle nStyle = MessBoxStyle::Ok | MessBoxStyle::DefaultOk,
                MessageType _eType = Info,
                const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo = nullptr );

    virtual ~OSQLMessageBox() override;
    virtual void dispose() override;

private:
    void Construct( MessBoxStyle nStyle, MessageType eImage );

    DECL_LINK(ButtonClickHdl, Button*, void );

private:
    void    impl_positionControls();
    void    impl_createStandardButtons( MessBoxStyle _nStyle );
    void    impl_addDetailsButton();
};

// OSQLWarningBox
class OSQLWarningBox : public OSQLMessageBox
{
public:
    OSQLWarningBox( vcl::Window* _pParent,
                    const OUString& _rMessage,
                    MessBoxStyle _nStyle = MessBoxStyle::Ok | MessBoxStyle::DefaultOk,
                    const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo = nullptr );
};

// OSQLErrorBox
class OSQLErrorBox : public OSQLMessageBox
{
public:
    OSQLErrorBox( vcl::Window* _pParent,
                  const OUString& _rMessage );
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_SQLMESSAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
