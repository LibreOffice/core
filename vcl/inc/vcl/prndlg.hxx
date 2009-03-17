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
#include "vcl/scrbar.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/lstbox.hxx"
#include "vcl/field.hxx"
#include "vcl/tabctrl.hxx"
#include "vcl/tabpage.hxx"

#include "tools/multisel.hxx"

#include <boost/shared_ptr.hpp>
#include <map>

namespace vcl
{
    class PrintDialog : public ModalDialog
    {
        class PrintPreviewWindow : public Window
        {
            GDIMetaFile         maMtf;
        public:
            PrintPreviewWindow( Window* pParent, const ResId& );
            virtual ~PrintPreviewWindow();

            virtual void Paint( const Rectangle& rRect );

            void setPreview( const GDIMetaFile& );
        };

        class PrinterTabPage : public TabPage
        {
        public:
            ListBox                                 maPrinters;
            PushButton                              maSetupButton;
            FixedText                               maType;
            FixedText                               maTypeText;
            FixedText                               maStatus;
            FixedText                               maStatusText;
            FixedText                               maLocation;
            FixedText                               maLocText;
            FixedText                               maComment;
            FixedText                               maCommentText;

            PrinterTabPage( Window*, const ResId& );
            virtual ~PrinterTabPage();
        };

        class JobTabPage : public TabPage
        {
        public:
            ListBox                                 maPrinters;
            CheckBox                                maToFileBox;
            FixedLine                               maPrintRange;
            RadioButton                             maAllButton;
            RadioButton                             maPagesButton;
            RadioButton                             maSelectionButton;
            Edit                                    maPagesEdit;

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
        };

        OKButton                                maOKButton;
        CancelButton                            maCancelButton;
        PrintPreviewWindow                      maPreviewWindow;
        FixedText                               maPageText;
        ScrollBar                               maPageScrollbar;

        TabControl                              maTabCtrl;
        PrinterTabPage                          maPrinterPage;
        JobTabPage                              maJobPage;

        FixedLine                               maButtonLine;

        boost::shared_ptr< PrinterListener >    maPListener;

        rtl::OUString                           maPageStr;
        sal_Int32                               mnCurPage;
        sal_Int32                               mnCachedPages;
        Rectangle                               maPreviewSpace;

        std::list< Window* >                    maControls;
        std::map< Window*, rtl::OUString >      maControlToPropertyMap;
        std::multimap< rtl::OUString, Window* > maPropertyToWindowMap;
        std::map< Window*, sal_Int32 >          maControlToNumValMap;

        void preparePreview();
        void setPreviewText( sal_Int32 );
        void updatePrinterText();
        void checkControlDependencies();
        void checkOptionalControlDependencies();
        void setupOptionalUI();
        com::sun::star::beans::PropertyValue* getValueForWindow( Window* ) const;

        virtual void Resize();

        DECL_LINK( ScrollHdl, ScrollBar* );
        DECL_LINK( ScrollEndHdl, ScrollBar* );
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
        MultiSelection getPageSelection();
        int getCopyCount();
        bool isCollate();
    };
}


#endif // _SV_PRNDLG_HXX
