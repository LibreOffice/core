/*************************************************************************
 *
 *  $RCSfile: sqlmessage.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-05 10:09:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

// some forwards
namespace com { namespace sun { namespace star {
    namespace sdb {
        class SQLContext;
    }
    namespace sdbc {
        class SQLException;
    }
} } }
namespace dbtools
{
    class SQLExceptionInfo;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

//==================================================================
// OSQLMessageBox
//==================================================================
class OSQLMessageBox : public ButtonDialog
{
    FixedImage      m_aInfoImage;
    FixedText       m_aTitle;
    FixedText       m_aMessage;

    UniString                       m_sInfo;
    ::com::sun::star::uno::Any      m_aNextChainElement;
    PushButton*                     m_pInfoButton;

public:
    enum MessageType
    {
        Info,
        Error,
        Warning,
        Query
    };

public:
    /** display a SQLException
        <p/>
        @param      _rTitle     main message
        @param      _rError     detailed message, may contain an exception chain to be displayed on an user request
    */
    OSQLMessageBox(Window* _pParent,
                const UniString& _rTitle,
                const ::com::sun::star::sdbc::SQLException& _rError,
                WinBits _nStyle = WB_OK | WB_DEF_OK,
                MessageType _eImage = Info);

    /** display an SQLException with auto-recognizing a main and a detailed message
        <p/>
        The first two messages from the exception chain are used as main and detailed message (recognizing the
        detailed field of an <type scope="com::sun::star::sdb">SQLContext</type>).
        <br/>
        If only one message is present, a default title ("error reading ...") is used
    */
    OSQLMessageBox(Window* _pParent,
                const ::com::sun::star::sdbc::SQLException& _rError,
                WinBits _nStyle = WB_OK | WB_DEF_OK,
                MessageType _eImage = Info);

    /** display an SQLException with auto-recognizing a main and a detailed message
        <p/>
        The first two messages from the exception chain are used as main and detailed message (recognizing the
        detailed field of an <type scope="com::sun::star::sdb">SQLContext</type>).
        <br/>
        If only one message is present, a default title ("error reading ...") is used
    */
    OSQLMessageBox(Window* _pParent,
                const dbtools::SQLExceptionInfo& _rException,
                WinBits _nStyle = WB_OK | WB_DEF_OK,
                MessageType _eImage = Info);

    /** display a database related error message
        <p/>
        @param  rTitle      the title to display
        @param  rMessage    the detailed message to display
    */
    OSQLMessageBox(Window* pParent,
                const UniString& rTitle,
                const UniString& rMessage,
                WinBits nStyle = WB_OK | WB_DEF_OK,
                MessageType eImage = Info);

    ~OSQLMessageBox();

private:
    void Construct(const dbtools::SQLExceptionInfo& _rException, WinBits nStyle, MessageType eImage);
    void Construct(const UniString& rTitle,
                   const UniString& rMessage,
                   WinBits nStyle,
                   MessageType eImage);

    DECL_LINK(ButtonClickHdl, Button* );
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_SQLMESSAGE_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 02.10.00 12:29:04  fs
 ************************************************************************/

