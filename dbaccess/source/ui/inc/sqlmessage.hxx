/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DBAUI_SQLMESSAGE_HXX_
#define _DBAUI_SQLMESSAGE_HXX_

#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BTNDLG_HXX
#include <vcl/btndlg.hxx>
#endif

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

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

