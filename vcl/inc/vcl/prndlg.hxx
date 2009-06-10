/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: prndlg.hxx,v $
 * $Revision: 1.3.114.5 $
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

#ifndef _SV_PRNDLG_HXX
#define _SV_PRNDLG_HXX

#include <vcl/dllapi.h>

#include "vcl/print.hxx"
#include "vcl/print.h"

#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/button.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/field.hxx"
#include "vcl/tabctrl.hxx"
#include "vcl/tabpage.hxx"
#include "vcl/arrange.hxx"

#include <boost/shared_ptr.hpp>
#include <map>

namespace vcl
{
    class PrintDialog : public ModalDialog
    {
        class PrinterListBox : public ListBox
        {
        public:
            PrinterListBox( Window* i_pParent, const ResId& i_rId )
            : ListBox( i_pParent, i_rId )
            {}
            virtual ~PrinterListBox() {}
            virtual void RequestHelp( const HelpEvent& i_rHEvt );
        };

        class PrintPreviewWindow : public Window
        {
            GDIMetaFile         maMtf;
            double              mfScaleX;
            double              mfScaleY;
        public:
            PrintPreviewWindow( Window* pParent, const ResId& );
            virtual ~PrintPreviewWindow();

            virtual void Paint( const Rectangle& rRect );

            void setPreview( const GDIMetaFile& );
            void setScale( double fScaleX, double fScaleY );
        };

        class NUpTabPage : public TabPage
        {
        public:
            FixedLine                               maNupLine;
            FixedText                               maNupRowsTxt;
            NumericField                            maNupRowsEdt;
            FixedText                               maNupColTxt;
            NumericField                            maNupColEdt;
            FixedText                               maNupRepTxt;
            NumericField                            maNupRepEdt;
            CheckBox                                maBorderCB;
            RadioButton                             maNupPortrait;
            RadioButton                             maNupLandscape;

            FixedLine                               maMargins;
            FixedText                               maLeftMarginTxt;
            MetricField                             maLeftMarginEdt;
            FixedText                               maRightMarginTxt;
            MetricField                             maRightMarginEdt;
            FixedText                               maTopMarginTxt;
            MetricField                             maTopMarginEdt;
            FixedText                               maBottomMarginTxt;
            MetricField                             maBottomMarginEdt;

            FixedText                               maHSpaceTxt;
            MetricField                             maHSpaceEdt;
            FixedText                               maVSpaceTxt;
            MetricField                             maVSpaceEdt;

            NUpTabPage( Window*, const ResId& );
            virtual ~NUpTabPage();

            void readFromSettings();
            void storeToSettings();
            void initFromMultiPageSetup( const vcl::PrinterListener::MultiPageSetup& );
        };

        class JobTabPage : public TabPage
        {
        public:
            PrinterListBox                          maPrinters;
            PushButton                              maSetupButton;
            CheckBox                                maToFileBox;

            FixedLine                               maCopies;
            FixedText                               maCopyCount;
            NumericField                            maCopyCountField;
            CheckBox                                maCollateBox;
            FixedImage                              maCollateImage;

            Image                                   maCollateImg;
            Image                                   maCollateHCImg;
            Image                                   maNoCollateImg;
            Image                                   maNoCollateHCImg;

            JobTabPage( Window*, const ResId& );
            virtual ~JobTabPage();

            void readFromSettings();
            void storeToSettings();
        };

        OKButton                                maOKButton;
        CancelButton                            maCancelButton;
        PrintPreviewWindow                      maPreviewWindow;
        NumericField                            maPageEdit;
        FixedText                               maNumPagesText;
        PushButton                              maForwardBtn;
        PushButton                              maBackwardBtn;

        TabControl                              maTabCtrl;
        NUpTabPage                              maNUpPage;
        JobTabPage                              maJobPage;

        FixedLine                               maButtonLine;

        boost::shared_ptr< PrinterListener >    maPListener;

        rtl::OUString                           maPageStr;
        rtl::OUString                           maNoPageStr;
        sal_Int32                               mnCurPage;
        sal_Int32                               mnCachedPages;
        Rectangle                               maPreviewSpace;
        Size                                    maCurPageSize;

        std::list< Window* >                    maControls;
        std::map< Window*, rtl::OUString >      maControlToPropertyMap;
        std::multimap< rtl::OUString, Window* > maPropertyToWindowMap;
        std::map< Window*, sal_Int32 >          maControlToNumValMap;

        Size                                    maNupPortraitSize;
        Size                                    maNupLandscapeSize;

        rtl::OUString                           maCommentText;
        rtl::OUString                           maStatusText;
        rtl::OUString                           maLocationText;
        rtl::OUString                           maTypeText;

        vcl::RowOrColumn                        maPreviewCtrlRow;
        Rectangle                               maPreviewBackground;

        void updateNup();
        void preparePreview( bool i_bPrintChanged = true, bool i_bMayUseCache = false );
        void setPreviewText( sal_Int32 );
        void updatePrinterText();
        void checkControlDependencies();
        void checkOptionalControlDependencies();
        void setupOptionalUI();
        void readFromSettings();
        void storeToSettings();
        com::sun::star::beans::PropertyValue* getValueForWindow( Window* ) const;

        virtual void Resize();
        virtual void Paint( const Rectangle& );

        DECL_LINK( SelectHdl, ListBox* );
        DECL_LINK( ClickHdl, Button* );
        DECL_LINK( ModifyHdl, Edit* );
        DECL_LINK( UIOptionsChanged, void* );

        DECL_LINK( UIOption_CheckHdl, CheckBox* );
        DECL_LINK( UIOption_RadioHdl, RadioButton* );
        DECL_LINK( UIOption_SelectHdl, ListBox* );
        DECL_LINK( UIOption_ModifyHdl, Edit* );

    public:
        PrintDialog( Window*, const boost::shared_ptr< PrinterListener >& );
        virtual ~PrintDialog();

        bool isPrintToFile();
        int getCopyCount();
        bool isCollate();
    };

    class PrintProgressDialog : public ModelessDialog
    {
        String              maStr;
        FixedText           maText;
        CancelButton        maButton;

        bool                mbCanceled;
        sal_Int32           mnCur;
        sal_Int32           mnMax;
        long                mnProgressHeight;
        Rectangle           maProgressRect;
        bool                mbNativeProgress;

        DECL_LINK( ClickHdl, Button* );

        void implCalcProgressRect();
    public:
        PrintProgressDialog( Window* i_pParent, int i_nMax );
        ~PrintProgressDialog();

        bool isCanceled() const { return mbCanceled; }
        void setProgress( int i_nCurrent, int i_nMax = -1 );
        void tick();

        virtual void Paint( const Rectangle& );
    };
}


#endif // _SV_PRNDLG_HXX
