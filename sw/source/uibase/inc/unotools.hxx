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
#include <actctrl.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <tools/resary.hxx>
#include "swdllapi.h"

class SwOneExampleFrame;

class SwFrmCtrlWindow : public VclEventBox
{
    SwOneExampleFrame* pExampleFrame;
public:
    SwFrmCtrlWindow(vcl::Window* pParent, SwOneExampleFrame* pFrame);

    virtual void Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
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
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >         _xControl;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >         _xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >    _xController;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >     _xCursor;

    SwFrmCtrlWindow aTopWindow;
    Timer           aLoadedTimer;
    Link            aInitializedLink;

    MenuResource    aMenuRes;
    OUString        sArgumentURL;

    SwView*         pModuleView;

    sal_uInt32          nStyleFlags;

    bool            bIsInitialized;
    bool            bServiceAvailable;

    static  bool    bShowServiceNotAvailableMessage;

    SAL_DLLPRIVATE DECL_LINK( TimeoutHdl, Timer* );
    SAL_DLLPRIVATE DECL_LINK( PopupHdl, Menu* );

    SAL_DLLPRIVATE void  CreateControl();
    SAL_DLLPRIVATE void  DisposeControl();

public:
    SwOneExampleFrame(vcl::Window& rWin,
                    sal_uInt32 nStyleFlags = EX_SHOW_ONLINE_LAYOUT,
                    const Link* pInitalizedLink = 0,
                    const OUString* pURL = 0);
    ~SwOneExampleFrame();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > &       GetControl()    {return _xControl; }
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > &       GetModel()      {return _xModel;}
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > &  GetController() {return _xController;}
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > &   GetTextCursor() {return _xCursor;}

    void ClearDocument( bool bStartTimer = false );

    bool IsInitialized() const {return bIsInitialized;}
    bool IsServiceAvailable() const {return bServiceAvailable;}

    void CreatePopup(const Point& rPt);

    static void     CreateErrorMessage(vcl::Window* pParent);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
