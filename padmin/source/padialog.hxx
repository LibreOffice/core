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

#ifndef _PAD_PADIALOG_HXX_
#define _PAD_PADIALOG_HXX_
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _RTL_USTRING
#include <rtl/ustring.hxx>
#endif
#include <vcl/dialog.hxx>
#include <tools/config.hxx>
#include <vcl/lstbox.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <helper.hxx>

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

        FixedLine                           m_aCUPSFL;
        CheckBox                            m_aCUPSCB;

        FixedLine                           m_aSepButtonFL;
        PushButton                          m_aAddPB;
        PushButton                          m_aFontsPB;
        CancelButton                        m_aCancelButton;

        String                              m_aDefPrt;
        String                              m_aRenameStr;

        ::psp::PrinterInfoManager&          m_rPIManager;
        ::std::list< ::rtl::OUString >      m_aPrinters;

        Image                               m_aPrinterImg;
        Image                               m_aFaxImg;
        Image                               m_aPdfImg;

        DECL_LINK( ClickBtnHdl, PushButton* );
        DECL_LINK( DoubleClickHdl, ListBox* );
        DECL_LINK( SelectHdl, ListBox* );
        DECL_LINK( DelPressedHdl, ListBox* );

        PADialog( Window*,  sal_Bool );
        void Init();

        void UpdateDefPrt();
        void UpdateText();
        void UpdateDevice();
        void AddDevice();
        void RemDevice();
        void ConfigureDevice();
        void RenameDevice();
        void PrintTestPage();
        void updateSettings();

        virtual long Notify( NotifyEvent& rEv );
        virtual void DataChanged( const DataChangedEvent& rEv );

        String getSelectedDevice();
    public:
        ~PADialog();

        static PADialog* Create( Window*,  sal_Bool );
    };

} // namespace

#endif
