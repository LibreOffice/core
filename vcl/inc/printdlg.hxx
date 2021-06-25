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

#include <vcl/bitmapex.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/idle.hxx>
#include <vcl/print.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <map>

namespace vcl {
    class PrintDialog;
}

namespace vcl
{
    class PrintDialog final : public weld::GenericDialogController
    {
        friend class MoreOptionsDialog;
    public:

        class PrintPreviewWindow final : public weld::CustomWidgetController
        {
            PrintDialog*        mpDialog;
            GDIMetaFile         maMtf;
            Size                maOrigSize;
            Size                maPreviewSize;
            sal_Int32           mnDPIX;
            sal_Int32           mnDPIY;
            BitmapEx            maPreviewBitmap;
            OUString            maReplacementString;
            bool                mbGreyscale;

            OUString            maHorzText;
            OUString            maVertText;

            void preparePreviewBitmap();

        public:
            PrintPreviewWindow(PrintDialog* pDialog);
            virtual ~PrintPreviewWindow() override;

            virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
            virtual bool Command( const CommandEvent& ) override;
            virtual void Resize() override;

            void setPreview( const GDIMetaFile&, const Size& i_rPaperSize,
                             std::u16string_view i_rPaperName,
                             const OUString& i_rNoPageString,
                             sal_Int32 i_nDPIX, sal_Int32 i_nDPIY,
                             bool i_bGreyscale
                            );
        };

        class ShowNupOrderWindow final : public weld::CustomWidgetController
        {
            NupOrderType mnOrderMode;
            int mnRows;
            int mnColumns;
        public:
            ShowNupOrderWindow();

            virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

            virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& ) override;

            void setValues( NupOrderType i_nOrderMode, int i_nColumns, int i_nRows )
            {
                mnOrderMode = i_nOrderMode;
                mnRows = i_nRows;
                mnColumns = i_nColumns;
                Invalidate();
            }
        };

        PrintDialog(weld::Window*, const std::shared_ptr<PrinterController>&);
        virtual ~PrintDialog() override;

        bool isPrintToFile() const;
        bool isCollate() const;
        bool isSingleJobs() const;
        bool hasPreview() const;

        void setPaperSizes();
        void previewForward();
        void previewBackward();
        void previewFirst();
        void previewLast();

    private:

        std::unique_ptr<weld::Builder>          mxCustomOptionsUIBuilder;

        std::shared_ptr<PrinterController>      maPController;

        std::unique_ptr<weld::Notebook>         mxTabCtrl;
        std::unique_ptr<weld::ScrolledWindow>   mxScrolledWindow;
        std::unique_ptr<weld::Frame>            mxPageLayoutFrame;
        std::unique_ptr<weld::ComboBox>         mxPrinters;
        std::unique_ptr<weld::Label>            mxStatusTxt;
        std::unique_ptr<weld::Button>           mxSetupButton;

        std::unique_ptr<weld::SpinButton>       mxCopyCountField;
        std::unique_ptr<weld::CheckButton>      mxCollateBox;
        std::unique_ptr<weld::Image>            mxCollateImage;
        std::unique_ptr<weld::Entry>            mxPageRangeEdit;
        std::unique_ptr<weld::RadioButton>      mxPageRangesRadioButton;
        std::unique_ptr<weld::ComboBox>         mxPaperSidesBox;
        std::unique_ptr<weld::CheckButton>      mxSingleJobsBox;
        std::unique_ptr<weld::CheckButton>      mxReverseOrderBox;

        std::unique_ptr<weld::Button>           mxOKButton;
        std::unique_ptr<weld::Button>           mxCancelButton;
        std::unique_ptr<weld::Button>           mxHelpButton;
        std::unique_ptr<weld::Button>           mxMoreOptionsBtn;

        std::unique_ptr<weld::Button>           mxBackwardBtn;
        std::unique_ptr<weld::Button>           mxForwardBtn;
        std::unique_ptr<weld::Button>           mxFirstBtn;
        std::unique_ptr<weld::Button>           mxLastBtn;

        std::unique_ptr<weld::CheckButton>      mxPreviewBox;
        std::unique_ptr<weld::Label>            mxNumPagesText;
        std::unique_ptr<PrintPreviewWindow>     mxPreview;
        std::unique_ptr<weld::CustomWeld>       mxPreviewWindow;
        std::unique_ptr<weld::Entry>            mxPageEdit;

        std::unique_ptr<weld::RadioButton>      mxPagesBtn;
        std::unique_ptr<weld::RadioButton>      mxBrochureBtn;
        std::unique_ptr<weld::Label>            mxPagesBoxTitleTxt;
        std::unique_ptr<weld::ComboBox>         mxNupPagesBox;

        // controls for "Custom" page mode
        std::unique_ptr<weld::Label>            mxNupNumPagesTxt;
        std::unique_ptr<weld::SpinButton>       mxNupColEdt;
        std::unique_ptr<weld::Label>            mxNupTimesTxt;
        std::unique_ptr<weld::SpinButton>       mxNupRowsEdt;
        std::unique_ptr<weld::Label>            mxPageMarginTxt1;
        std::unique_ptr<weld::MetricSpinButton> mxPageMarginEdt;
        std::unique_ptr<weld::Label>            mxPageMarginTxt2;
        std::unique_ptr<weld::Label>            mxSheetMarginTxt1;
        std::unique_ptr<weld::MetricSpinButton> mxSheetMarginEdt;
        std::unique_ptr<weld::Label>            mxSheetMarginTxt2;
        std::unique_ptr<weld::ComboBox>         mxPaperSizeBox;
        std::unique_ptr<weld::ComboBox>         mxOrientationBox;

        // page order ("left to right, then down")
        std::unique_ptr<weld::Label>            mxNupOrderTxt;
        std::unique_ptr<weld::ComboBox>         mxNupOrderBox;
        std::unique_ptr<ShowNupOrderWindow>     mxNupOrder;
        std::unique_ptr<weld::CustomWeld>       mxNupOrderWin;
        /// border around each page
        std::unique_ptr<weld::CheckButton>      mxBorderCB;
        std::unique_ptr<weld::Expander>         mxRangeExpander;
        std::unique_ptr<weld::Expander>         mxLayoutExpander;
        std::unique_ptr<weld::Widget>           mxCustom;

        OUString                                maPrintToFileText;
        OUString                                maPrintText;
        OUString                                maDefPrtText;

        OUString                                maPageStr;
        OUString                                maNoPageStr;
        OUString                                maNoPreviewStr;
        sal_Int32                               mnCurPage;
        sal_Int32                               mnCachedPages;

        bool                                    mbCollateAlwaysOff;

        std::vector<std::unique_ptr<weld::Widget>>
                                                maExtraControls;

        std::map<weld::Widget*, OUString>
                                                maControlToPropertyMap;
        std::map<OUString, std::vector<weld::Widget*>>
                                                maPropertyToWindowMap;
        std::map<weld::Widget*, sal_Int32>
                                                maControlToNumValMap;

        Size                                    maNupPortraitSize;
        Size                                    maNupLandscapeSize;
        /// internal, used for automatic Nup-Portrait/landscape
        Size                                    maFirstPageSize;

        bool                                    mbShowLayoutFrame;

        Paper                                   mePaper;

        Idle maUpdatePreviewIdle;
        DECL_LINK(updatePreviewIdle, Timer*, void);
        Idle maUpdatePreviewNoCacheIdle;
        DECL_LINK(updatePreviewNoCacheIdle, Timer*, void);

        DECL_LINK( ClickHdl, weld::Button&, void );
        DECL_LINK( SelectHdl, weld::ComboBox&, void );
        DECL_LINK( ActivateHdl, weld::Entry&, bool );
        DECL_LINK( FocusOutHdl, weld::Widget&, void );
        DECL_LINK( SpinModifyHdl, weld::SpinButton&, void );
        DECL_LINK( MetricSpinModifyHdl, weld::MetricSpinButton&, void );
        DECL_LINK( ToggleHdl, weld::Toggleable&, void );

        DECL_LINK( UIOption_CheckHdl, weld::Toggleable&, void );
        DECL_LINK( UIOption_RadioHdl, weld::Toggleable&, void );
        DECL_LINK( UIOption_SelectHdl, weld::ComboBox&, void );
        DECL_LINK( UIOption_SpinModifyHdl, weld::SpinButton&, void );
        DECL_LINK( UIOption_EntryModifyHdl, weld::Entry&, void );

        css::beans::PropertyValue* getValueForWindow(weld::Widget*) const;

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
        void makeEnabled( weld::Widget* );
        void updateWindowFromProperty( const OUString& );
        void initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& );
        void showAdvancedControls( bool );
        void updateNup( bool i_bMayUseCache = true );
        void updateNupFromPages( bool i_bMayUseCache = true );
        void enableNupControls( bool bEnable );
        void setupOptionalUI();
        Size const & getJobPageSize();

    };

    class PrintProgressDialog final : public weld::GenericDialogController
    {
        OUString            maStr;
        bool                mbCanceled;
        sal_Int32           mnCur;
        sal_Int32           mnMax;

        std::unique_ptr<weld::Label> mxText;
        std::unique_ptr<weld::ProgressBar> mxProgress;
        std::unique_ptr<weld::Button> mxButton;

        DECL_LINK( ClickHdl, weld::Button&, void );

    public:
        PrintProgressDialog(weld::Window* i_pParent, int i_nMax);
        virtual ~PrintProgressDialog() override;
        bool isCanceled() const { return mbCanceled; }
        void setProgress( int i_nCurrent );
        void tick();
    };
}

#endif // VCL_INC_NEWPRINTDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
