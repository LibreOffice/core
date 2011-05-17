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

#ifndef SD_FU_CONSTRUCT_HXX
#define SD_FU_CONSTRUCT_HXX

#include "fudraw.hxx"

class KeyEvent;
class SdrObject;
class SfxItemSet;

namespace sd {

/*************************************************************************
|*
|* Rechteck zeichnen
|*
\************************************************************************/

class FuConstruct
    : public FuDraw
{
public:
    static const int MIN_FREEHAND_DISTANCE = 10;

    TYPEINFO();

    virtual void DoExecute( SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void SelectionHasChanged() { bSelectionChanged = sal_True; }

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determined by the member nSlotId :
    void SetStyleSheet(SfxItemSet& rAttr, SdrObject* pObj);

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determinded by the parameters bUseFillStyle and bUseNoFillStyle :
    void SetStyleSheet( SfxItemSet& rAttr, SdrObject* pObj,
            const sal_Bool bUseFillStyle, const sal_Bool bUseNoFillStyle );

protected:
    FuConstruct (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    bool bSelectionChanged;
};

} // end of namespace sd

#endif      // _SD_FUCONSTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
