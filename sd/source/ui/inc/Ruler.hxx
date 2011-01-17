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

#ifndef SD_RULER_HXX
#define SD_RULER_HXX


#include <svx/ruler.hxx>


namespace sd {

class DrawViewShell;
class RulerCtrlItem;
class View;
class Window;

/*************************************************************************
|*
|* das Fenster der Diashow leitet Mouse- und Key-Events an die SlideViewShell
|*
\************************************************************************/

class Ruler
    : public SvxRuler
{
public:
    Ruler (
        DrawViewShell& rViewSh,
        ::Window* pParent,
        ::sd::Window* pWin,
        sal_uInt16 nRulerFlags,
        SfxBindings& rBindings,
        WinBits nWinStyle);
    virtual ~Ruler (void);

    void SetNullOffset(const Point& rOffset);

    sal_Bool IsHorizontal() const { return bHorz; }

    using ::Ruler::SetNullOffset;
protected:
    ::sd::View* pSdView;
    ::sd::Window* pSdWin;
    DrawViewShell* pDrViewShell;
    RulerCtrlItem* pCtrlItem;
    sal_Bool bHorz;

    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt);
    virtual void    MouseMove(const MouseEvent& rMEvt);
    virtual void    Command(const CommandEvent& rCEvt);
    virtual void    ExtraDown();
};

} // end of namespace sd

#endif
