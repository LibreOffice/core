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



#ifndef _PAD_FONTENTRY_HXX_
#define _PAD_FONTENTRY_HXX_

#include "progress.hxx"
#include "helper.hxx"

#include "vcl/timer.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/combobox.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/group.hxx"
#include "vcl/fontmanager.hxx"

#include "tools/urlobj.hxx"

namespace padmin {

    class FontImportDialog :
        public ModalDialog,
        public ::psp::PrintFontManager::ImportFontCallback
    {
        OKButton                            m_aOKBtn;
        CancelButton                        m_aCancelBtn;
        PushButton                          m_aSelectAllBtn;
        ListBox                             m_aNewFontsBox;
        FixedLine                           m_aFromFL;
        Edit                                m_aFromDirEdt;
        PushButton                          m_aFromBtn;
        CheckBox                            m_aSubDirsBox;
        FixedLine                           m_aTargetOptFL;
        CheckBox                            m_aLinkOnlyBox;
        FixedText                           m_aFixedText;
        bool                                m_bOverwriteAll;
        bool                                m_bOverwriteNone;
        ProgressDialog*                     m_pProgress;
        int                                 m_nFont;

        String                              m_aImportOperation;
        String                              m_aOverwriteQueryText;
        String                              m_aOverwriteAllText;
        String                              m_aOverwriteNoneText;
        String                              m_aNoAfmText;
        String                              m_aAfmCopyFailedText;
        String                              m_aFontCopyFailedText;
        String                              m_aNoWritableFontsDirText;
        String                              m_aFontsImportedText;

        ::std::hash_map< ::rtl::OString, ::std::list< ::psp::FastPrintFontInfo >, ::rtl::OStringHash >
                                            m_aNewFonts;

        Timer                               m_aRefreshTimer;
        DECL_LINK( RefreshTimeoutHdl, void* );


        ::psp::PrintFontManager&            m_rFontManager;

        DECL_LINK( ClickBtnHdl, Button* );
        DECL_LINK( ModifyHdl, Edit* );
        DECL_LINK( ToggleHdl, CheckBox* );

        // implement ImportFontCallback
        virtual void importFontsFailed( ::psp::PrintFontManager::ImportFontCallback::FailCondition eReason );
        virtual void progress( const ::rtl::OUString& rFile );
        virtual bool queryOverwriteFile( const ::rtl::OUString& rFile );
        virtual void importFontFailed( const ::rtl::OUString& rFile, ::psp::PrintFontManager::ImportFontCallback::FailCondition eReason );
        virtual bool isCanceled();

        void copyFonts();
        void fillFontBox();
    public:
        FontImportDialog( Window* );
        ~FontImportDialog();
    };

    class FontNameDlg : public ModalDialog
    {
    private:
        OKButton                    m_aOKButton;
        PushButton                  m_aRenameButton;
        PushButton                  m_aRemoveButton;
        PushButton                  m_aImportButton;

        DelListBox                  m_aFontBox;
        FixedText                   m_aFixedText;

        String                      m_aRenameString;
        String                      m_aRenameTTCString;
        String                      m_aNoRenameString;

        ::psp::PrintFontManager&    m_rFontManager;

        // maps fontID to XLFD
        ::std::hash_map< ::psp::fontID, String >
                                    m_aFonts;
        void init();
    public:
        FontNameDlg( Window* );
        ~FontNameDlg();

        DECL_LINK( ClickBtnHdl, Button* );
        DECL_LINK( DelPressedHdl, ListBox* );
        DECL_LINK( SelectHdl, ListBox* );

        static String fillFontEntry( ::psp::FastPrintFontInfo& rInfo, const String& rFile, bool bAddRegular );
        static String fillFontEntry( const ::std::list< ::psp::FastPrintFontInfo >& rInfos, const String& rFile );
    };
} // namespace

#endif
