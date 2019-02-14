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

#ifndef VCL_INC_NEWPRINTDLG_HXX
#define VCL_INC_NEWPRINTDLG_HXX

#include <vcl/dllapi.h>

#include <vcl/print.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/field.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>

namespace vcl {
    class PrintDialog;
}

namespace vcl
{
    class MoreOptionsDialog : public weld::GenericDialogController
    {
        VclPtr<PrintDialog>                     mpParent;
        std::unique_ptr<weld::Button>           mxOKButton;
        std::unique_ptr<weld::Button>           mxCancelButton;
        std::unique_ptr<weld::CheckButton>      mxSingleJobsBox;

        DECL_LINK( ClickHdl, weld::Button&, void );

    public:

        MoreOptionsDialog( VclPtr<PrintDialog> i_pParent );
        virtual ~MoreOptionsDialog() override;
    };

    class PrintDialog : public ModalDialog
    {
        friend class MoreOptionsDialog;
    public:

        class PrintPreviewWindow : public vcl::Window
        {
            GDIMetaFile         maMtf;
            Size                maOrigSize;
            Size                maPreviewSize;
            sal_Int32           mnDPIX;
            sal_Int32           mnDPIY;
            BitmapEx            maPreviewBitmap;
            OUString            maReplacementString;
            OUString const      maToolTipString;
            bool                mbGreyscale;
            VclPtr<FixedLine>   maHorzDim;
            VclPtr<FixedLine>   maVertDim;

            void preparePreviewBitmap();

        public:
            PrintPreviewWindow( vcl::Window* pParent );
            virtual ~PrintPreviewWindow() override;
            virtual void dispose() override;

            virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
            virtual void Command( const CommandEvent& ) override;
            virtual void Resize() override;

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
        public:
            ShowNupOrderWindow( vcl::Window* pParent );

            virtual Size GetOptimalSize() const override;

            virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;

            void setValues( NupOrderType i_nOrderMode, int i_nColumns, int i_nRows )
            {
                mnOrderMode = i_nOrderMode;
                mnRows = i_nRows;
                mnColumns = i_nColumns;
                Invalidate();
            }
        };

        PrintDialog( vcl::Window*, const std::shared_ptr< PrinterController >& );
        virtual ~PrintDialog() override;
        virtual void dispose() override;

        bool isPrintToFile();
        bool isCollate();
        bool isSingleJobs() const { return mbSingleJobs; };
        bool hasPreview();

        void setPaperSizes();
        void previewForward();
        void previewBackward();

    private:

        std::unique_ptr<VclBuilder>             mpCustomOptionsUIBuilder;

        std::shared_ptr<PrinterController>      maPController;

        std::unique_ptr<MoreOptionsDialog>      mxMoreOptionsDlg;

        VclPtr<TabControl>                      mpTabCtrl;
        VclPtr<VclFrame>                        mpPageLayoutFrame;
        VclPtr<ListBox>                         mpPrinters;
        VclPtr<FixedText>                       mpStatusTxt;
        VclPtr<PushButton>                      mpSetupButton;
        OUString const                          maPrintToFileText;
        OUString                                maPrintText;
        OUString const                          maDefPrtText;

        VclPtr<NumericField>                    mpCopyCountField;
        VclPtr<CheckBox>                        mpCollateBox;
        VclPtr<FixedImage>                      mpCollateImage;
        VclPtr<ListBox>                         mpPaperSidesBox;
        VclPtr<CheckBox>                        mpReverseOrderBox;

        VclPtr<OKButton>                        mpOKButton;
        VclPtr<CancelButton>                    mpCancelButton;
        VclPtr<HelpButton>                      mpHelpButton;
        VclPtr<PushButton>                      mpMoreOptionsBtn;

        VclPtr<PushButton>                      mpBackwardBtn;
        VclPtr<PushButton>                      mpForwardBtn;
        VclPtr<CheckBox>                        mpPreviewBox;
        VclPtr<FixedText>                       mpNumPagesText;
        VclPtr<PrintPreviewWindow>              mpPreviewWindow;
        VclPtr<NumericField>                    mpPageEdit;

        OUString                                maPageStr;
        OUString const                          maNoPageStr;
        OUString const                          maNoPreviewStr;
        sal_Int32                               mnCurPage;
        sal_Int32                               mnCachedPages;

        bool                                    mbCollateAlwaysOff;

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
        VclPtr<ListBox>                         mpPaperSizeBox;
        VclPtr<ListBox>                         mpOrientationBox;

        // page order ("left to right, then down")
        VclPtr<FixedText>                       mpNupOrderTxt;
        VclPtr<ListBox>                         mpNupOrderBox;
        VclPtr<ShowNupOrderWindow>              mpNupOrderWin;
        /// border around each page
        VclPtr<CheckBox>                        mpBorderCB;

        std::map< VclPtr<vcl::Window>, OUString >
                                                maControlToPropertyMap;
        std::map< OUString, std::vector< VclPtr<vcl::Window> > >
                                                maPropertyToWindowMap;
        std::map< VclPtr<vcl::Window>, sal_Int32 >
                                                maControlToNumValMap;

        Size                                    maNupPortraitSize;
        Size                                    maNupLandscapeSize;
        /// internal, used for automatic Nup-Portrait/landscape
        Size                                    maFirstPageSize;

        bool                                    mbShowLayoutFrame;
        bool                                    mbSingleJobs;

        Paper                                   mePaper;

        DECL_LINK( ClickHdl, Button*, void );
        DECL_LINK( SelectHdl, ListBox&, void );
        DECL_LINK( ModifyHdl, Edit&, void );
        DECL_LINK( ToggleHdl, CheckBox&, void );
        DECL_LINK( ToggleRadioHdl, RadioButton&, void );

        DECL_LINK( UIOption_CheckHdl, CheckBox&, void );
        DECL_LINK( UIOption_RadioHdl, RadioButton&, void );
        DECL_LINK( UIOption_SelectHdl, ListBox&, void );
        DECL_LINK( UIOption_ModifyHdl, Edit&, void );

        css::beans::PropertyValue* getValueForWindow( vcl::Window* ) const;

        void preparePreview( bool i_bMayUseCache );
        void setupPaperSidesBox();
        void storeToSettings();
        void readFromSettings();
        void setPaperOrientation( Orientation eOrientation );
        void updateOrientationBox( bool bAutomatic = true );
        bool hasOrientationChanged() const;
        void checkPaperSize( Size& rPaperSize );
        void setPreviewText();
        void updatePrinterText();
        void checkControlDependencies();
        void checkOptionalControlDependencies();
        void makeEnabled( vcl::Window* );
        void updateWindowFromProperty( const OUString& );
        void initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& );
        void showAdvancedControls( bool );
        void updateNup( bool i_bMayUseCache = true );
        void updateNupFromPages( bool i_bMayUseCache = true );
        void enableNupControls( bool bEnable );
        void setupOptionalUI();
        Size const & getJobPageSize();

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

        DECL_LINK( ClickHdl, Button*, void );

    public:
        PrintProgressDialog(vcl::Window* i_pParent, int i_nMax);
        virtual ~PrintProgressDialog() override;
        virtual void dispose() override;
        bool isCanceled() const { return mbCanceled; }
        void setProgress( int i_nCurrent );
        void tick();
        void reset();
    };
}

#endif // VCL_INC_NEWPRINTDLG_HXX
