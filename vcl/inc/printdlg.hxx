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

            bool useHCColorReplacement() const;
        public:
            PrintPreviewWindow( Window* pParent, const ResId& );
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

            virtual void Paint( const Rectangle& );

            void setValues( int i_nOrderMode, int i_nColumns, int i_nRows )
            {
                mnOrderMode = i_nOrderMode;
                mnRows = i_nRows;
                mnColumns = i_nColumns;
                Invalidate();
            }
        };

        class NUpTabPage : public TabPage
        {
        public:
            FixedLine                               maNupLine;
            RadioButton                             maPagesBtn;
            RadioButton                             maBrochureBtn;
            FixedText                               maPagesBoxTitleTxt;
            ListBox                                 maNupPagesBox;

            // controls for "Custom" page mode
            FixedText                               maNupNumPagesTxt;
            NumericField                            maNupColEdt;
            FixedText                               maNupTimesTxt;
            NumericField                            maNupRowsEdt;
            FixedText                               maPageMarginTxt1;
            MetricField                             maPageMarginEdt;
            FixedText                               maPageMarginTxt2;
            FixedText                               maSheetMarginTxt1;
            MetricField                             maSheetMarginEdt;
            FixedText                               maSheetMarginTxt2;
            FixedText                               maNupOrientationTxt;
            ListBox                                 maNupOrientationBox;

            // page order ("left to right, then down")
            FixedText                               maNupOrderTxt;
            ListBox                                 maNupOrderBox;
            ShowNupOrderWindow                      maNupOrderWin;
            // border around each page
            CheckBox                                maBorderCB;

            boost::shared_ptr< vcl::RowOrColumn >   mxBrochureDep;
            boost::shared_ptr< vcl::LabeledElement >mxPagesBtnLabel;

            void setupLayout();

            NUpTabPage( Window*, const ResId& );
            virtual ~NUpTabPage();

            void readFromSettings();
            void storeToSettings();
            void initFromMultiPageSetup( const vcl::PrinterController::MultiPageSetup& );
            void enableNupControls( bool bEnable );

            void showAdvancedControls( bool );

            // virtual void Resize();
        };

        class JobTabPage : public TabPage
        {
        public:
            FixedLine                               maPrinterFL;
            ListBox                                 maPrinters;
            DisclosureButton                        maDetailsBtn;
            FixedText                               maStatusLabel;
            FixedText                               maStatusTxt;
            FixedText                               maLocationLabel;
            FixedText                               maLocationTxt;
            FixedText                               maCommentLabel;
            FixedText                               maCommentTxt;

            PushButton                              maSetupButton;

            FixedLine                               maCopies;
            FixedLine                               maCopySpacer;
            FixedText                               maCopyCount;
            NumericField                            maCopyCountField;
            CheckBox                                maCollateBox;
            FixedImage                              maCollateImage;
            CheckBox                                maReverseOrderBox;

            Image                                   maCollateImg;
            Image                                   maCollateHCImg;
            Image                                   maNoCollateImg;
            Image                                   maNoCollateHCImg;

            long                                    mnCollateUIMode;

            boost::shared_ptr<vcl::RowOrColumn>     mxPrintRange;
            boost::shared_ptr<vcl::WindowArranger>  mxDetails;

            JobTabPage( Window*, const ResId& );
            virtual ~JobTabPage();

            void readFromSettings();
            void storeToSettings();

            // virtual void Resize();

            void setupLayout();
        };

        class OutputOptPage : public TabPage
        {
        public:
            FixedLine                           maOptionsLine;
            CheckBox                            maToFileBox;
            CheckBox                            maCollateSingleJobsBox;

            boost::shared_ptr<vcl::RowOrColumn> mxOptGroup;

            OutputOptPage( Window*, const ResId& );
            virtual ~OutputOptPage();

            void readFromSettings();
            void storeToSettings();

            // virtual void Resize();

            void setupLayout();
        };

        OKButton                                maOKButton;
        CancelButton                            maCancelButton;
        HelpButton                              maHelpButton;
        PrintPreviewWindow                      maPreviewWindow;
        NumericField                            maPageEdit;
        FixedText                               maNumPagesText;
        PushButton                              maBackwardBtn;
        PushButton                              maForwardBtn;

        TabControl                              maTabCtrl;
        NUpTabPage                              maNUpPage;
        JobTabPage                              maJobPage;
        OutputOptPage                           maOptionsPage;

        FixedLine                               maButtonLine;

        boost::shared_ptr< PrinterController >  maPController;

        rtl::OUString                           maPageStr;
        rtl::OUString                           maNoPageStr;
        sal_Int32                               mnCurPage;
        sal_Int32                               mnCachedPages;

        std::list< Window* >                    maControls;
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

        void setupLayout();
    public:
        PrintDialog( Window*, const boost::shared_ptr< PrinterController >& );
        virtual ~PrintDialog();

        bool isPrintToFile();
        int getCopyCount();
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
