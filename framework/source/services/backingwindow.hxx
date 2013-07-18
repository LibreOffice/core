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

#include <rtl/ustring.hxx>

#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/layout.hxx>

#include <svtools/acceleratorexecute.hxx>
#include <unotools/moduleoptions.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XUIControllerFactory.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

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

        PushButton*                     mpWriterButton;
        PushButton*                     mpCalcButton;
        PushButton*                     mpImpressButton;
        MenuButton*                     mpOpenButton;
        PushButton*                     mpDrawButton;
        PushButton*                     mpDBButton;
        PushButton*                     mpMathButton;
        PushButton*                     mpTemplateButton;

        PushButton*                     mpExtensionsButton;
        PushButton*                     mpInfoButton;
        PushButton*                     mpTplRepButton;

        VclGrid*                        mpStartCenterContainer;

        BitmapEx                        maBackgroundLeft;
        BitmapEx                        maBackgroundMiddle;
        BitmapEx                        maBackgroundRight;

        Rectangle                       maStartCentButtons;

        bool                            mbInitControls;
        sal_Int32                       mnHideExternalLinks;
        svt::AcceleratorExecute*        mpAccExec;

        int                             mnSCWidth;
        int                             mnSCHeight;

        void setupButton( PushButton* pButton, const OUString& rURL, const std::set<OUString>& rURLS,
                          SvtModuleOptions& rOpt, SvtModuleOptions::EModule eMod );

        void setupExternalLink( PushButton* pButton );

        void dispatchURL( const OUString& i_rURL,
                          const OUString& i_rTarget = OUString( "_default" ),
                          const com::sun::star::uno::Reference< com::sun::star::frame::XDispatchProvider >& i_xProv = com::sun::star::uno::Reference< com::sun::star::frame::XDispatchProvider >(),
                          const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& = com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >()
                          );

        DECL_LINK( ClickHdl, Button* );
        DECL_LINK( ExtLinkClickHdl, Button* );
        DECL_LINK( ActivateHdl, Button* );
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

        virtual Size GetOptimalSize() const;

        void setOwningFrame( const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& xFrame );
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
