/*************************************************************************
 *
 *  $RCSfile: padialog.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2005-08-05 12:55:32 $
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

#ifndef _PAD_PADIALOG_HXX_
#define _PAD_PADIALOG_HXX_
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
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
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif

// forward declaration
namespace psp { class PrinterInfoManager; }
class Printer;

namespace padmin {

    class PADialog : public ModalDialog
    {
    private:
        DelListBox                          m_aDevicesLB;
        PushButton                          m_aConfPB;
        PushButton                          m_aRenamePB;
        PushButton                          m_aStdPB;
        PushButton                          m_aRemPB;
        PushButton                          m_aTestPagePB;
        FixedLine                           m_aPrintersFL;
        FixedText                           m_aDriverTxt;
        FixedText                           m_aDriver;
        FixedText                           m_aLocationTxt;
        FixedText                           m_aLocation;
        FixedText                           m_aCommandTxt;
        FixedText                           m_aCommand;
        FixedText                           m_aCommentTxt;
        FixedText                           m_aComment;

        FixedLine                           m_aSepButtonFL;
        PushButton                          m_aAddPB;
        PushButton                          m_aFontsPB;
        CancelButton                        m_aCancelButton;

        String                              m_aDefPrt;
        String                              m_aRenameStr;

        Printer*                            m_pPrinter;
        ::psp::PrinterInfoManager&          m_rPIManager;
        ::std::list< ::rtl::OUString >      m_aPrinters;

        Image                               m_aPrinterImg;
        Image                               m_aFaxImg;
        Image                               m_aPdfImg;

        DECL_LINK( ClickBtnHdl, PushButton* );
        DECL_LINK( DoubleClickHdl, ListBox* );
        DECL_LINK( SelectHdl, ListBox* );
        DECL_LINK( EndPrintHdl, void* );
        DECL_LINK( DelPressedHdl, ListBox* );

        PADialog( Window*,  BOOL );
        void Init();

        void UpdateDefPrt();
        void UpdateText();
        void UpdateDevice();
        void AddDevice();
        void RemDevice();
        void ConfigureDevice();
        void RenameDevice();
        void PrintTestPage();

        virtual long Notify( NotifyEvent& rEv );

        String getSelectedDevice();
    public:
        ~PADialog();

        static PADialog* Create( Window*,  BOOL );
    };

} // namespace

#endif
