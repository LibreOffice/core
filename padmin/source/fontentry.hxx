/*************************************************************************
 *
 *  $RCSfile: fontentry.hxx,v $
 *
 *  $Revision: 1.4 $
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

#ifndef _PAD_FONTENTRY_HXX_
#define _PAD_FONTENTRY_HXX_

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _PAD_PRGRESS_HXX
#include <progress.hxx>
#endif
#ifndef _PSPRINT_FONTMANAGER_HXX_
#include <psprint/fontmanager.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif

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
        PushButton                  m_aRemoveButton;
        PushButton                  m_aImportButton;

        DelListBox                  m_aFontBox;
        FixedText                   m_aFixedText;

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

        static String fillFontEntry( ::psp::FastPrintFontInfo& rInfo, const String& rFile, bool bAddRegular );
        static String fillFontEntry( const ::std::list< ::psp::FastPrintFontInfo >& rInfos, const String& rFile );
    };
} // namespace

#endif
