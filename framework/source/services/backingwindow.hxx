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

#ifndef FRAMEWORK_BACKINGWINDOW_HXX
#define FRAMEWORK_BACKINGWINDOW_HXX

#include "rtl/ustring.hxx"

#include "vcl/button.hxx"
#include "vcl/menubtn.hxx"
#include "vcl/fixed.hxx"
#include "vcl/bitmapex.hxx"
#include "vcl/toolbox.hxx"

#include "unotools/moduleoptions.hxx"
#include "svtools/acceleratorexecute.hxx"

#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/frame/XTerminateListener.hpp"
#include "com/sun/star/document/XEventListener.hpp"
#include "com/sun/star/document/XEventBroadcaster.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/ui/dialogs/XFolderPicker.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"

#include <set>

class MnemonicGenerator;

namespace framework
{
        // To get the transparent mouse-over look, the closer is actually a toolbox
    // overload DataChange to handle style changes correctly
    class DecoToolBox : public ToolBox
    {
        Size maMinSize;

        using Window::ImplInit;
    public:
                DecoToolBox( Window* pParent, WinBits nStyle = 0 );
                DecoToolBox( Window* pParent, const ResId& rResId );

        void    DataChanged( const DataChangedEvent& rDCEvt );

        void    calcMinSize();
        Size    getMinSize();
    };

    class BackingWindow : public Window
    {
        struct LoadRecentFile
        {
            rtl::OUString                                                             aTargetURL;
            com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >     aArgSeq;
        };

        com::sun::star::uno::Reference<com::sun::star::frame::XDesktop>                  mxDesktop;
        com::sun::star::uno::Reference<com::sun::star::frame::XDispatchProvider >        mxDesktopDispatchProvider;
        com::sun::star::uno::Reference<com::sun::star::frame::XFrame>                    mxFrame;
        com::sun::star::uno::Reference<com::sun::star::document::XEventBroadcaster>      mxBroadcaster;

        ImageButton                     maWriterButton;
        ImageButton                     maCalcButton;
        ImageButton                     maImpressButton;
        MenuButton                      maOpenButton;
        ImageButton                     maDrawButton;
        ImageButton                     maDBButton;
        ImageButton                     maMathButton;
        ImageButton                     maTemplateButton;

        DecoToolBox                     maToolbox;

        BitmapEx                        maBackgroundLeft;
        BitmapEx                        maBackgroundMiddle;
        BitmapEx                        maBackgroundRight;

        String                          maCreateString;
        String                          maOpenString;
        String                          maTemplateString;

        Font                            maTextFont;
        Rectangle                       maControlRect;

        long                            mnColumnWidth[2];
        long                            mnTextColumnWidth[2];
        Color                           maLabelTextColor;

        Size                            maButtonImageSize;

        bool                            mbInitControls;
        sal_Int32                       mnHideExternalLinks;
        svt::AcceleratorExecute*        mpAccExec;
        long                            mnBtnPos;
        long                            mnBtnTop;

        PopupMenu*                      mpRecentMenu;
        std::vector< LoadRecentFile >   maRecentFiles;

        static const int nItemId_Extensions = 1;
        static const int nItemId_Info = 3;
        static const int nItemId_TplRep = 4;
        static const int nShadowTop = 37;
        static const int nShadowLeft = 38;
        static const int nShadowRight = 38;
        static const int nShadowBottom = 38;

        void loadImage( const ResId& i_rId, PushButton& i_rButton );

        void layoutButton( const char* i_pURL, int nColumn, int i_nExtraWidth, const std::set<rtl::OUString>& i_rURLS,
                           SvtModuleOptions& i_rOpt, SvtModuleOptions::EModule i_eMod,
                           PushButton& i_rBtn,
                           MnemonicGenerator& i_rMnemonicGen,
                           const String& i_rStr = String()
                           );

        void dispatchURL( const rtl::OUString& i_rURL,
                          const rtl::OUString& i_rTarget = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ),
                          const com::sun::star::uno::Reference< com::sun::star::frame::XDispatchProvider >& i_xProv = com::sun::star::uno::Reference< com::sun::star::frame::XDispatchProvider >(),
                          const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& = com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >()
                          );

        DECL_LINK( ClickHdl, Button* );
        DECL_LINK( SelectHdl, Button* );
        DECL_LINK( ActivateHdl, Button* );
        DECL_LINK( ToolboxHdl, void* );
        DECL_LINK( WindowEventListener, VclSimpleEvent* );

        void initControls();
        void initBackground();
        void prepareRecentFileMenu();
        public:
        BackingWindow( Window* pParent );
        ~BackingWindow();

        virtual void        Paint( const Rectangle& rRect );
        virtual void        Resize();
        virtual long        Notify( NotifyEvent& rNEvt );
        virtual void        GetFocus();

        void setOwningFrame( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& xFrame );
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
