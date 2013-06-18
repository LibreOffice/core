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

#include "vcl/builder.hxx"
#include "vcl/button.hxx"
#include "vcl/menubtn.hxx"
#include "vcl/fixed.hxx"
#include "vcl/bitmapex.hxx"
#include "vcl/toolbox.hxx"
#include "vcl/layout.hxx"

#include "unotools/moduleoptions.hxx"
#include "svtools/acceleratorexecute.hxx"

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
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
    class BackingWindow : public Window, public VclBuilderContainer
    {
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >         mxContext;
        com::sun::star::uno::Reference<com::sun::star::frame::XDispatchProvider >        mxDesktopDispatchProvider;
        com::sun::star::uno::Reference<com::sun::star::frame::XFrame>                    mxFrame;
        com::sun::star::uno::Reference< com::sun::star::frame::XUIControllerFactory >    mxPopupMenuFactory;
        com::sun::star::uno::Reference< com::sun::star::frame::XPopupMenuController >    mxPopupMenuController;
        com::sun::star::uno::Reference< com::sun::star::awt::XPopupMenu >                mxPopupMenu;

        PushButton*                    mpWriterButton;
        PushButton*                    mpCalcButton;
        PushButton*                    mpImpressButton;
        //MenuButton*                     mpOpenButton;
        PushButton*                    mpOpenButton;
        PushButton*                    mpDrawButton;
        PushButton*                    mpDBButton;
        PushButton*                    mpMathButton;
        PushButton*                    mpTemplateButton;


        PushButton*                    mpExtensionsButton;
        PushButton*                    mpInfoButton;
        PushButton*                    mpTplRepButton;

        VclGrid*                       mpStartCenterContainer;

        BitmapEx                        maBackgroundLeft;
        BitmapEx                        maBackgroundMiddle;
        BitmapEx                        maBackgroundRight;

        String                          maCreateString;
        String                          maOpenString;
        String                          maTemplateString;

        Font                            maTextFont;
        Rectangle                       maControlRect;

        Rectangle                       maStartCentButtons;

        Color                           maLabelTextColor;


        bool                            mbInitControls;
        sal_Int32                       mnHideExternalLinks;
        svt::AcceleratorExecute*        mpAccExec;

        static const int nItemId_Extensions = 1;
        static const int nItemId_Info = 3;
        static const int nItemId_TplRep = 4;

        static const int nShadowTop = 30;
        static const int nShadowLeft = 30;
        static const int nShadowRight = 30;
        static const int nShadowBottom = 30;

        static const int nPaddingTop = 30;
        static const int nPaddingLeft = 50;
        static const int nPaddingRight = 50;
        static const int nPaddingBottom = 30;

        static const int nLogoHeight = 150;
        int nSCWidth;
        int nSCHeight;

        void loadImage( const ResId& i_rId, PushButton& i_rButton );

        void layoutButton( const char* i_pURL, const std::set<OUString>& i_rURLS,
                           SvtModuleOptions& i_rOpt, SvtModuleOptions::EModule i_eMod,
                           PushButton& i_rBtn,
                           MnemonicGenerator& i_rMnemonicGen,
                           const String& i_rStr = String()
                           );

        void layoutExternalLink( PushButton& i_rBtn );

        void dispatchURL( const OUString& i_rURL,
                          const OUString& i_rTarget = OUString( "_default" ),
                          const com::sun::star::uno::Reference< com::sun::star::frame::XDispatchProvider >& i_xProv = com::sun::star::uno::Reference< com::sun::star::frame::XDispatchProvider >(),
                          const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& = com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >()
                          );

        DECL_LINK( ClickHdl, Button* );
        DECL_LINK( ExtLinkClickHdl, Button* );
        //DECL_LINK( ActivateHdl, Button* );
        DECL_LINK( WindowEventListener, VclSimpleEvent* );

        void initControls();
        void initBackground();
        //void prepareRecentFileMenu();
        public:
        BackingWindow( Window* pParent );
        ~BackingWindow();

        virtual void        Paint( const Rectangle& rRect );
        virtual void        Resize();
        virtual long        Notify( NotifyEvent& rNEvt );
        virtual void        GetFocus();

        virtual Size GetOptimalSize() const;

        void setOwningFrame( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& xFrame );
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
