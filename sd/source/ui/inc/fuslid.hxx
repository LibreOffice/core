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

#ifndef SD_FU_SLIDE_HXX
#define SD_FU_SLIDE_HXX

#include "fupoor.hxx"

class SdDrawDocument;

namespace sd {

class SlideView;
class SlideViewShell;
class Window;


/*************************************************************************
|*
|* Basisklasse der Funktionen des Diamodus
|*
\************************************************************************/

class FuSlide
    : public FuPoor
{
public:
    TYPEINFO();

    static FunctionReference Create( SlideViewShell* pViewSh, ::sd::Window* pWin, SlideView* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void ScrollStart();
    virtual void ScrollEnd();

protected:
    FuSlide (
        SlideViewShell* pViewSh,
        ::sd::Window* pWin,
        SlideView* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    SlideViewShell* pSlViewShell;
    SlideView*    pSlView;
};

} // end of namespace sd

#endif
