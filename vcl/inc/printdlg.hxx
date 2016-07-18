/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_INC_PRINTDLG_HXX
#define INCLUDED_VCL_INC_PRINTDLG_HXX

#include <vcl/dllapi.h>

#include <vcl/print.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/field.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/virdev.hxx>

namespace vcl
{
    class PrintDialog : public ModalDialog
    {
    public:
        class PrintPreviewWindow : public vcl::Window
        {
            static const sal_Int32 PREVIEW_BITMAP_WIDTH;

            GDIMetaFile         maMtf;
            Size                maOrigSize;
            Size                maPreviewSize;
            VclPtr<VirtualDevice> maPageVDev;
            Bitmap              maPreviewBitmap;
            OUString            maReplacementString;
            OUString            maToolTipString;
            bool                mbGreyscale;
            VclPtr<FixedLine>   maHorzDim;
            VclPtr<FixedLine>   maVertDim;

            void preparePreviewBitmap();

        public:
            PrintPreviewWindow( vcl::Window* pParent );
            virtual ~PrintPreviewWindow();
            virtual void dispose() override;

            virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
            virtual void Command( const CommandEvent& ) override;
            virtual void Resize() override;
            virtual void DataChanged( const DataChangedEvent& ) override;

            void setPreview( const GDIMetaFile&, const Size& i_rPaperSize,
                             const OUString& i_rPaperName,
                             const OUString& i_rNoPageString,
                             sal_Int32 i_nDPIX, sal_Int32 i_nDPIY,
                             bool i_bGreyscale
                            );
        };

        class ShowNupOrderWindow : public vcl::Window
        {
            NupOrderType mnOrderMode;
            int mnRows;
            int mnColumns;
            void ImplInitSettings();
        public:
            ShowNupOrderWindow( vcl::Window* pParent );

            virtual Size GetOptimalSize() const override;

            virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& ) override;

            void setValues( NupOrderType i_nOrderMode, int i_nColumns, int i_nRows )
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
            VclPtr<RadioButton>                     mpPagesBtn;
            VclPtr<RadioButton>                     mpBrochureBtn;
            VclPtr<FixedText>                       mpPagesBoxTitleTxt;
            VclPtr<ListBox>                         mpNupPagesBox;

            // controls for "Custom" page mode
            VclPtr<FixedText>                       mpNupNumPagesTxt;
            VclPtr<NumericField>                    mpNupColEdt;
            VclPtr<FixedText>                       mpNupTimesTxt;
            VclPtr<NumericField>                    mpNupRowsEdt;
            VclPtr<FixedText>                       mpPageMarginTxt1;
            VclPtr<MetricField>                     mpPageMarginEdt;
            VclPtr<FixedText>                       mpPageMarginTxt2;
            VclPtr<FixedText>                       mpSheetMarginTxt1;
            VclPtr<MetricField>                     mpSheetMarginEdt;
            VclPtr<FixedText>                       mpSheetMarginTxt2;
            VclPtr<FixedText>                       mpNupOrientationTxt;
            VclPtr<ListBox>                         mpNupOrientationBox;

            // page order ("left to right, then down")
            VclPtr<FixedText>                       mpNupOrderTxt;
            VclPtr<ListBox>                         mpNupOrderBox;
            VclPtr<ShowNupOrderWindow>              mpNupOrderWin;
            /// border around each page
            VclPtr<CheckBox>                        mpBorderCB;

            NUpTabPage( VclBuilder* );

            void initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& );
            void enableNupControls( bool bEnable );

            void showAdvancedControls( bool );
        };

        class JobTabPage
        {
        public:
            VclPtr<ListBox>                         mpPrinters;
            VclPtr<FixedText>                       mpStatusTxt;
            VclPtr<FixedText>                       mpLocationTxt;
            VclPtr<FixedText>                       mpCommentTxt;

            VclPtr<PushButton>                      mpSetupButton;

            VclPtr<NumericField>                    mpCopyCountField;
            VclPtr<CheckBox>                        mpCollateBox;
            VclPtr<FixedImage>                      mpCollateImage;
            VclPtr<CheckBox>                        mpReverseOrderBox;

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
            VclPtr<CheckBox>                    mpCollateSingleJobsBox;
            VclPtr<CheckBox>                    mpPapersizeFromSetup;

            OutputOptPage( VclBuilder* );

            void readFromSettings();
            void storeToSettings();
        };

        VclBuilder*                             mpCustomOptionsUIBuilder;

        std::shared_ptr<PrinterController>      maPController;
        VclPtr<TabControl>                      mpTabCtrl;
        NUpTabPage                              maNUpPage;
        JobTabPage                              maJobPage;
        OutputOptPage                           maOptionsPage;
        VclPtr<PrintPreviewWindow>              mpPreviewWindow;
        VclPtr<NumericField>                    mpPageEdit;

        VclPtr<FixedText>                       mpNumPagesText;
        VclPtr<PushButton>                      mpBackwardBtn;
        VclPtr<PushButton>                      mpForwardBtn;

        VclPtr<OKButton>                        mpOKButton;
        VclPtr<CancelButton>                    mpCancelButton;
        VclPtr<HelpButton>                      mpHelpButton;

        OUString                                maPageStr;
        OUString                                maNoPageStr;
        sal_Int32                               mnCurPage;
        sal_Int32                               mnCachedPages;

        std::map< VclPtr<vcl::Window>, OUString >      maControlToPropertyMap;
        std::map< OUString, std::vector< VclPtr<vcl::Window> > >
                                                maPropertyToWindowMap;
        std::map< VclPtr<vcl::Window>, sal_Int32 >          maControlToNumValMap;
        std::set< OUString >                    maReverseDependencySet;

        Size                                    maNupPortraitSize;
        Size                                    maNupLandscapeSize;

        /// internal, used for automatic Nup-Portrait/landscape
        Size                                    maFirstPageSize;

        OUString                           maPrintToFileText;
        OUString                           maPrintText;
        OUString                           maDefPrtText;

        bool                                mbShowLayoutPage;

        Size const & getJobPageSize();
        void updateNup();
        void updateNupFromPages();
        void preparePreview( bool i_bPrintChanged = true, bool i_bMayUseCache = false );
        void setPreviewText( sal_Int32 );
        void updatePrinterText();
        void checkControlDependencies();
        void checkOptionalControlDependencies();
        void makeEnabled( vcl::Window* );
        void updateWindowFromProperty( const OUString& );
        void setupOptionalUI();
        void readFromSettings();
        void storeToSettings();
        css::beans::PropertyValue* getValueForWindow( vcl::Window* ) const;

        virtual void Resize() override;
        virtual void Command( const CommandEvent& ) override;
        virtual void DataChanged( const DataChangedEvent& ) override;

        DECL_LINK_TYPED( SelectHdl, ListBox&, void );
        DECL_LINK_TYPED( ClickHdl, Button*, void );
        DECL_LINK_TYPED( ToggleHdl, CheckBox&, void );
        DECL_LINK_TYPED( ToggleRadioHdl, RadioButton&, void );
        DECL_LINK_TYPED( ModifyHdl, Edit&, void );

        DECL_LINK_TYPED( UIOption_CheckHdl, CheckBox&, void );
        DECL_LINK_TYPED( UIOption_RadioHdl, RadioButton&, void );
        DECL_LINK_TYPED( UIOption_SelectHdl, ListBox&, void );
        DECL_LINK_TYPED( UIOption_ModifyHdl, Edit&, void );

    public:
        PrintDialog( vcl::Window*, const std::shared_ptr< PrinterController >& );
        virtual ~PrintDialog();
        virtual void dispose() override;

        bool isPrintToFile();
        bool isCollate();
        bool isSingleJobs();

        void previewForward();
        void previewBackward();
    };

    class PrintProgressDialog : public ModelessDialog
    {
        OUString            maStr;
        VclPtr<FixedText>   mpText;
        VclPtr<ProgressBar> mpProgress;
        VclPtr<CancelButton> mpButton;

        bool                mbCanceled;
        sal_Int32           mnCur;
        sal_Int32           mnMax;

        DECL_LINK_TYPED( ClickHdl, Button*, void );

    public:
        PrintProgressDialog(vcl::Window* i_pParent, int i_nMax);
        virtual ~PrintProgressDialog();
        virtual void dispose() override;
        bool isCanceled() const { return mbCanceled; }
        void setProgress( int i_nCurrent );
        void tick();
        void reset();
    };
}

#endif // INCLUDED_VCL_INC_PRINTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
