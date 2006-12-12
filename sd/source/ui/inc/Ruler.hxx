/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Ruler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:37:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_RULER_HXX
#define SD_RULER_HXX


#ifndef _SVX_RULER_HXX
#include <svx/ruler.hxx>
#endif


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
        USHORT nRulerFlags,
        SfxBindings& rBindings,
        WinBits nWinStyle);
    virtual ~Ruler (void);

    void SetNullOffset(const Point& rOffset);

    BOOL IsHorizontal() const { return bHorz; }

    using ::Ruler::SetNullOffset;
protected:
    ::sd::View* pSdView;
    ::sd::Window* pSdWin;
    DrawViewShell* pDrViewShell;
    RulerCtrlItem* pCtrlItem;
    BOOL bHorz;

    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt);
    virtual void    MouseMove(const MouseEvent& rMEvt);
    virtual void    Command(const CommandEvent& rCEvt);
    virtual void    ExtraDown();
};

} // end of namespace sd

#endif
