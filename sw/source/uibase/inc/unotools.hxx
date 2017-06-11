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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UNOTOOLS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UNOTOOLS_HXX

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/idle.hxx>
#include <actctrl.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include "swdllapi.h"

class SwOneExampleFrame;

class SwFrameCtrlWindow : public VclEventBox
{
    SwOneExampleFrame* pExampleFrame;
public:
    SwFrameCtrlWindow(vcl::Window* pParent, SwOneExampleFrame* pFrame);

    virtual void Command( const CommandEvent& rCEvt ) override;
    virtual Size GetOptimalSize() const override;
    virtual void Resize() override;
};

#define EX_SHOW_ONLINE_LAYOUT   0x001

// hard zoom value
#define EX_SHOW_BUSINESS_CARDS  0x02
//don't modify page size
#define EX_SHOW_DEFAULT_PAGE    0x04

class SwView;

class SW_DLLPUBLIC SwOneExampleFrame
{
    css::uno::Reference< css::awt::XControl >         m_xControl;
    css::uno::Reference< css::frame::XModel >         m_xModel;
    css::uno::Reference< css::frame::XController >    m_xController;
    css::uno::Reference< css::text::XTextCursor >     m_xCursor;

    VclPtr<SwFrameCtrlWindow> m_aTopWindow;
    Idle            m_aLoadedIdle;
    Link<SwOneExampleFrame&,void> m_aInitializedLink;

    OUString        m_sArgumentURL;

    SwView*         m_pModuleView;

    sal_uInt32          m_nStyleFlags;

    bool            m_bIsInitialized;
    bool            m_bServiceAvailable;

    static  bool    bShowServiceNotAvailableMessage;

    DECL_DLLPRIVATE_LINK( TimeoutHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK( PopupHdl, Menu*, bool );

    SAL_DLLPRIVATE void  CreateControl();
    SAL_DLLPRIVATE void  DisposeControl();

public:
    SwOneExampleFrame(vcl::Window& rWin,
                    sal_uInt32 nStyleFlags,
                    const Link<SwOneExampleFrame&,void>* pInitalizedLink,
                    const OUString* pURL = nullptr);
    ~SwOneExampleFrame();

    css::uno::Reference< css::frame::XModel > &       GetModel()      {return m_xModel;}
    css::uno::Reference< css::frame::XController > &  GetController() {return m_xController;}
    css::uno::Reference< css::text::XTextCursor > &   GetTextCursor() {return m_xCursor;}

    void ClearDocument();

    bool IsInitialized() const {return m_bIsInitialized;}
    bool IsServiceAvailable() const {return m_bServiceAvailable;}

    void CreatePopup(const Point& rPt);

    static void     CreateErrorMessage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
