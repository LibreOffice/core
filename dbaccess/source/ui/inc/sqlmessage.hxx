/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_SQLMESSAGE_HXX_
#define _DBAUI_SQLMESSAGE_HXX_

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/btndlg.hxx>

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

//.........................................................................
namespace dbaui
{
//.........................................................................

//==================================================================
// OSQLMessageBox
//==================================================================
struct SQLMessageBox_Impl;
class OSQLMessageBox : public ButtonDialog
{
    FixedImage      m_aInfoImage;
    FixedText       m_aTitle;
    FixedText       m_aMessage;
    ::rtl::OUString m_sHelpURL;

    ::std::auto_ptr< SQLMessageBox_Impl >   m_pImpl;

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
        detailed field of an <type scope="com::sun::star::sdb">SQLContext</type>).
    */
    OSQLMessageBox(
        Window* _pParent,
        const dbtools::SQLExceptionInfo& _rException,
        WinBits _nStyle = WB_OK | WB_DEF_OK,
        const ::rtl::OUString& _rHelpURL = ::rtl::OUString()
    );

    /** display a database related error message

        @param  rTitle      the title to display
        @param  rMessage    the detailed message to display
        @param  _eType      determines the image to use. AUTO is disallowed in this constructor version
    */
    OSQLMessageBox(Window* pParent,
                const UniString& rTitle,
                const UniString& rMessage,
                WinBits nStyle = WB_OK | WB_DEF_OK,
                MessageType _eType = Info,
                const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo = NULL );

    ~OSQLMessageBox();

private:
    void Construct( WinBits nStyle, MessageType eImage );

    DECL_LINK(ButtonClickHdl, Button* );

private:
    void    impl_positionControls();
    void    impl_initImage( MessageType _eImage );
    void    impl_createStandardButtons( WinBits _nStyle );
    void    impl_addDetailsButton();
};

//==================================================================
// OSQLWarningBox
//==================================================================
class OSQLWarningBox : public OSQLMessageBox
{
public:
    OSQLWarningBox( Window* _pParent,
                    const UniString& _rMessage,
                    WinBits _nStyle = WB_OK | WB_DEF_OK,
                    const ::dbtools::SQLExceptionInfo* _pAdditionalErrorInfo = NULL );
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_SQLMESSAGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
