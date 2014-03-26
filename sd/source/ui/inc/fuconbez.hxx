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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FUCONBEZ_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FUCONBEZ_HXX

#include <com/sun/star/uno/Any.hxx>
#include "fuconstr.hxx"

class SdDrawDocument;

namespace sd {

class Window;


class FuConstructBezierPolygon
    : public FuConstruct
{
public:
    TYPEINFO_OVERRIDE();

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent );
    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();
    virtual void Deactivate();

    virtual void SelectionHasChanged();

    void    SetEditMode(sal_uInt16 nMode);
    sal_uInt16  GetEditMode() { return nEditMode; }

    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);

protected:
    FuConstructBezierPolygon (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    sal_uInt16      nEditMode;

    ::com::sun::star::uno::Any maTargets;   // used for creating a path for custom animations
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
