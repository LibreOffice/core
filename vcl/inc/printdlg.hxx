/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_PRNDLG_HXX
#define _SV_PRNDLG_HXX

#include <vcl/dllapi.h>

#include "print.h"

#include "vcl/print.hxx"
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/button.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/field.hxx"
#include "vcl/tabctrl.hxx"
#include "vcl/tabpage.hxx"
#include "vcl/arrange.hxx"
#include "vcl/virdev.hxx"

#include <boost/shared_ptr.hpp>
#include <map>

namespace vcl
{
    class PrintDialog : public ModalDialog
    {
    public:
        class PrintPreviewWindow : public Window
        {
            GDIMetaFile         maMtf;
            Size                maOrigSize;
            Size                maPreviewSize;
            VirtualDevice       maPageVDev;
            rtl::OUString       maReplacementString;
            rtl::OUString       maToolTipString;
            bool                mbGreyscale;
            FixedLine           maHorzDim;
            FixedLine           maVertDim;

        public:
            PrintPreviewWindow( Window* pParent );
            virtual ~PrintPreviewWindow();

            virtual void Paint( const Rectangle& rRect );
            virtual void Command( const CommandEvent& );
            virtual void Resize();
            virtual void DataChanged( const DataChangedEvent& );

            void setPreview( const GDIMetaFile&, const Size& i_rPaperSize,
                             const rtl::OUString& i_rPaperName,
                             const rtl::OUString& i_rNoPageString,
                             sal_Int32 i_nDPIX, sal_Int32 i_nDPIY,
                             bool i_bGreyscale
                            );
        };

        class ShowNupOrderWindow : public Window
        {
            int mnOrderMode;
            int mnRows;
            int mnColumns;
            void ImplInitSettings();
        public:
            ShowNupOrderWindow( Window* pParent );
            virtual ~ShowNupOrderWindow();

            virtual Size GetOptimalSize(WindowSizeType eType) const;

            virtual void Paint( const Rectangle& );

            void setValues( int i_nOrderMode, int i_nColumns, int i_nRows )
            {
                mnOrderMode = i_nOrderMode;
                mnRows = i_nRows;
                mnColumns = i_nColumns;
                Invalidate();
            }
        };

    private:

        class NUpTabPage
        {
        public:
            RadioButton*                            mpPagesBtn;
            RadioButton*                            mpBrochureBtn;
            FixedText*                              mpPagesBoxTitleTxt;
            ListBox*                                mpNupPagesBox;

            // controls for "Custom" page mode
            FixedText*                              mpNupNumPagesTxt;
            NumericField*                           mpNupColEdt;
            FixedText*                              mpNupTimesTxt;
            NumericField*                           mpNupRowsEdt;
            FixedText*                              mpPageMarginTxt1;
            MetricField*                            mpPageMarginEdt;
            FixedText*                              mpPageMarginTxt2;
            FixedText*                              mpSheetMarginTxt1;
            MetricField*                            mpSheetMarginEdt;
            FixedText*                              mpSheetMarginTxt2;
            FixedText*                              mpNupOrientationTxt;
            ListBox*                                mpNupOrientationBox;

            // page order ("left to right, then down")
            FixedText*                              mpNupOrderTxt;
            ListBox*                                mpNupOrderBox;
            ShowNupOrderWindow*                     mpNupOrderWin;
            // border around each page
            CheckBox*                               mpBorderCB;

            boost::shared_ptr< vcl::RowOrColumn >   mxBrochureDep;
            boost::shared_ptr< vcl::LabeledElement >mxPagesBtnLabel;

            void setupLayout();

            NUpTabPage( VclBuilder* );

            void readFromSettings();
            void storeToSettings();
            void initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& );
            void enableNupControls( bool bEnable );

            void showAdvancedControls( bool );
        };

        class JobTabPage
        {
        public:
            ListBox*                                mpPrinters;
            FixedText*                              mpStatusTxt;
            FixedText*                              mpLocationTxt;
            FixedText*                              mpCommentTxt;

            PushButton*                             mpSetupButton;

            FixedLine*                              mpCopySpacer;
            NumericField*                           mpCopyCountField;
            CheckBox*                               mpCollateBox;
            FixedImage*                             mpCollateImage;
            CheckBox*                               mpReverseOrderBox;

            Image                                   maCollateImg;
            Image                                   maNoCollateImg;

            long                                    mnCollateUIMode;

            JobTabPage( VclBuilder* );

            void readFromSettings();
            void storeToSettings();
        };

        class OutputOptPage
        {
        public:
            CheckBox*                           mpToFileBox;
            CheckBox*                           mpCollateSingleJobsBox;

            boost::shared_ptr<vcl::RowOrColumn> mxOptGroup;

            OutputOptPage( VclBuilder* );

            void readFromSettings();
            void storeToSettings();
        };

        VclBuilder*                             mpCustomOptionsUIBuilder;

        boost::shared_ptr< PrinterController >  maPController;
        TabControl*                             mpTabCtrl;
        NUpTabPage                              maNUpPage;
        JobTabPage                              maJobPage;
        OutputOptPage                           maOptionsPage;
        PrintPreviewWindow*                     mpPreviewWindow;
        NumericField*                           mpPageEdit;

        FixedText*                              mpNumPagesText;
        PushButton*                             mpBackwardBtn;
        PushButton*                             mpForwardBtn;

        OKButton*                               mpOKButton;
        CancelButton*                           mpCancelButton;
        HelpButton*                             mpHelpButton;

        rtl::OUString                           maPageStr;
        rtl::OUString                           maNoPageStr;
        sal_Int32                               mnCurPage;
        sal_Int32                               mnCachedPages;

        std::map< Window*, rtl::OUString >      maControlToPropertyMap;
        std::map< rtl::OUString, std::vector< Window* > >
                                                maPropertyToWindowMap;
        std::map< Window*, sal_Int32 >          maControlToNumValMap;
        std::set< rtl::OUString >               maReverseDependencySet;

        Size                                    maNupPortraitSize;
        Size                                    maNupLandscapeSize;

        // internal, used for automatic Nup-Portrait/landscape
        Size                                    maFirstPageSize;

        rtl::OUString                           maPrintToFileText;
        rtl::OUString                           maPrintText;
        rtl::OUString                           maDefPrtText;

        boost::shared_ptr<vcl::RowOrColumn>     mxPreviewCtrls;

        Size                                    maDetailsCollapsedSize;
        Size                                    maDetailsExpandedSize;

        sal_Bool                                mbShowLayoutPage;

        Size getJobPageSize();
        void updateNup();
        void updateNupFromPages();
        void preparePreview( bool i_bPrintChanged = true, bool i_bMayUseCache = false );
        void setPreviewText( sal_Int32 );
        void updatePrinterText();
        void checkControlDependencies();
        void checkOptionalControlDependencies();
        void makeEnabled( Window* );
        void updateWindowFromProperty( const rtl::OUString& );
        void setupOptionalUI();
        void readFromSettings();
        void storeToSettings();
        com::sun::star::beans::PropertyValue* getValueForWindow( Window* ) const;

        virtual void Resize();
        virtual void Command( const CommandEvent& );
        virtual void DataChanged( const DataChangedEvent& );

        DECL_LINK( SelectHdl, ListBox* );
        DECL_LINK( ClickHdl, Button* );
        DECL_LINK( ModifyHdl, Edit* );
        DECL_LINK( UIOptionsChanged, void* );

        DECL_LINK( UIOption_CheckHdl, CheckBox* );
        DECL_LINK( UIOption_RadioHdl, RadioButton* );
        DECL_LINK( UIOption_SelectHdl, ListBox* );
        DECL_LINK( UIOption_ModifyHdl, Edit* );

    public:
        PrintDialog( Window*, const boost::shared_ptr< PrinterController >& );
        virtual ~PrintDialog();

        bool isPrintToFile();
        bool isCollate();
        bool isSingleJobs();

        void previewForward();
        void previewBackward();
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
        void reset();

        virtual void Paint( const Rectangle& );
    };
}


#endif // _SV_PRNDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
