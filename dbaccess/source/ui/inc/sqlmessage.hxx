/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlmessage.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:26:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
        <p/>
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

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_SQLMESSAGE_HXX_

