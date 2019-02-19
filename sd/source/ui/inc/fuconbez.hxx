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

class FuConstructBezierPolygon final
    : public FuConstruct
{
public:

    static rtl::Reference<FuPoor> Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq, bool bPermanent );
    void DoExecute( SfxRequest& rReq ) override;

    // Mouse- & Key-Events
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

    virtual void Activate() override;
    virtual void Deactivate() override;

    virtual void SelectionHasChanged() override;

    void    SetEditMode(sal_uInt16 nMode);
    sal_uInt16  GetEditMode() { return nEditMode; }

    /**
     * set attribute for the object to be created
     */
    void SetAttributes(SfxItemSet& rAttr, SdrObject* pObj);

    virtual SdrObjectUniquePtr CreateDefaultObject(const sal_uInt16 nID, const ::tools::Rectangle& rRectangle) override;

private:
    FuConstructBezierPolygon (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    sal_uInt16    nEditMode;
    css::uno::Any maTargets;   // used for creating a path for custom animations

    //Extra attributes coming from parameters
    sal_uInt16  mnTransparence;  // Default: 0
    OUString    msColor;         // Default: ""
    sal_uInt16  mnWidth;         // Default: 0
    OUString    msShapeName;     // Default: ""
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
