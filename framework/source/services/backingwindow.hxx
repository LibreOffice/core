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
        static const int nShadowTop = 31;
        static const int nShadowLeft = 35;
        static const int nShadowRight = 45;
        static const int nShadowBottom = 40;

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
