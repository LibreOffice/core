/*************************************************************************
 *
 *  $RCSfile: cmddlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2001-06-15 15:30:08 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _PAD_COMMANDDLG_HXX_
#define _PAD_COMMANDDLG_HXX_

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_CONFIG_HXX
#include <vcl/config.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif

#include <list>

namespace padmin {

class RTSDialog;

class CommandStore
{
    static void getStoredCommands( const char* pGroup, ::std::list< String >& rCommands );
    static void getSystemPrintCommands( ::std::list< String >& rCommands );
    static void getSystemPdfCommands( ::std::list< String >& rCommands );
    static void setCommands( const char* pGroup, const ::std::list< String >& rCommands, const ::std::list< String >& rSysCommands );

public:
    static void getPrintCommands( ::std::list< String >& rCommands );
    static void getFaxCommands( ::std::list< String >& rCommands );
    static void getPdfCommands( ::std::list< String >& rCommands );

    static void setPrintCommands( const ::std::list< String >& rCommands );
    static void setFaxCommands( const ::std::list< String >& rCommands );
    static void setPdfCommands( const ::std::list< String >& rCommands );
};

class RTSCommandPage : public TabPage
{
private:
    RTSDialog*                      m_pParent;

    ComboBox                        m_aCommandsCB;
    FixedLine                       m_aCommandTitle;
    FixedText                       m_aPrinterName;
    FixedText                       m_aConnectedTo;
    FixedLine                       m_aPrinterFL;

    FixedText                       m_aConfigureText;
    ListBox                         m_aConfigureBox;
    USHORT                          m_nPrinterEntry;
    USHORT                          m_nFaxEntry;
    USHORT                          m_nPdfEntry;
    FixedText                       m_aPdfDirectoryText;
    PushButton                      m_aPdfDirectoryButton;
    Edit                            m_aPdfDirectoryEdit;
    CheckBox                        m_aFaxSwallowBox;

    PushButton                      m_aHelpButton;
    PushButton                      m_aRemovePB;

    ::std::list< String >           m_aPrinterCommands;
    ::std::list< String >           m_aFaxCommands;
    ::std::list< String >           m_aPdfCommands;

    String                          m_aFaxHelp;
    String                          m_aPrinterHelp;
    String                          m_aPdfHelp;

    bool                            m_bWasFax;
    bool                            m_bWasPdf;

    DECL_LINK( DoubleClickHdl, ComboBox* );
    DECL_LINK( ClickBtnHdl, Button* );
    DECL_LINK( SelectHdl, Control* );
    DECL_LINK( ModifyHdl, Edit* );

    void ConnectCommand();
    void UpdateCommands();

public:
    RTSCommandPage( RTSDialog* );
    ~RTSCommandPage();

    void save();
};

} // namespace

#endif
