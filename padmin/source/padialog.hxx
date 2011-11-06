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

    class SPA_DLLPUBLIC PADialog : public ModalDialog
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
