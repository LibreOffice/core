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
#ifndef _UNOTOOLS_HXX
#define _UNOTOOLS_HXX


#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <actctrl.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrameControl.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <tools/resary.hxx>
#include "swdllapi.h"

class SwOneExampleFrame;

class SwFrmCtrlWindow : public VclEventBox
{
    SwOneExampleFrame* pExampleFrame;
public:
    SwFrmCtrlWindow(Window* pParent, SwOneExampleFrame* pFrame);

    virtual void Command( const CommandEvent& rCEvt );
    virtual Size GetOptimalSize() const;
    virtual void Resize();
};

class MenuResource : public Resource
{
    ResStringArray      aMenuArray;

public:
    MenuResource(const ResId& rResId);

    ResStringArray& GetMenuArray() {return aMenuArray;}
};

#define EX_SHOW_ONLINE_LAYOUT   0x001

// hard zoom value
#define EX_SHOW_BUSINESS_CARDS  0x02
//don't modify page size
#define EX_SHOW_DEFAULT_PAGE    0x04

class SwView;

class SW_DLLPUBLIC SwOneExampleFrame
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrameControl >  m_xFrameControl;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >         _xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >    _xController;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >     _xCursor;

    SwFrmCtrlWindow aTopWindow;
    Timer           aLoadedTimer;
    Link            aInitializedLink;

    MenuResource    aMenuRes;
    String          sArgumentURL;

    SwView*         pModuleView;

    sal_uInt32          nStyleFlags;

    sal_Bool            bIsInitialized;
    sal_Bool            bServiceAvailable;

    static  bool    bShowServiceNotAvailableMessage;

    SW_DLLPRIVATE DECL_LINK( TimeoutHdl, Timer* );
    SW_DLLPRIVATE DECL_LINK( PopupHdl, Menu* );

    SW_DLLPRIVATE void  CreateControl();
    SW_DLLPRIVATE void  DisposeControl();

public:
    SwOneExampleFrame(Window& rWin,
                    sal_uInt32 nStyleFlags = EX_SHOW_ONLINE_LAYOUT,
                    const Link* pInitalizedLink = 0,
                    String* pURL = 0);
    ~SwOneExampleFrame();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrameControl > & GetFrameControl() {return m_xFrameControl; }
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &        GetModel()      {return _xModel;}
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > &   GetController() {return _xController;}
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > &    GetTextCursor() {return _xCursor;}

    void ClearDocument( sal_Bool bStartTimer = sal_False );

    sal_Bool IsInitialized() const {return bIsInitialized;}
    sal_Bool IsServiceAvailable() const {return bServiceAvailable;}

    void CreatePopup(const Point& rPt);

    static void     CreateErrorMessage(Window* pParent);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
