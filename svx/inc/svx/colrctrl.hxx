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
#ifndef _SVX_COLRCTRL_HXX
#define _SVX_COLRCTRL_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <svtools/valueset.hxx>
#include <svtools/transfer.hxx>
#include <svl/lstner.hxx>
#include "svx/svxdllapi.h"

class XColorTable;
class SvData;

/*************************************************************************
|*
|* SvxColorValueSet
|*
\************************************************************************/

class SvxColorValueSet : public ValueSet, public DragSourceHelper
{
    using ValueSet::StartDrag;

private:

    sal_Bool            bLeft;
    Point           aDragPosPixel;

protected:

    void            DoDrag();

    // ValueSet
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command(const CommandEvent& rCEvt );

    // DragSourceHelper
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPtPixel );

                    DECL_STATIC_LINK(SvxColorValueSet, ExecDragHdl, void*);

public:
                    SvxColorValueSet( Window* pParent, WinBits nWinStyle = WB_ITEMBORDER );
                    SvxColorValueSet( Window* pParent, const ResId& rResId );

    sal_Bool            IsLeftButton() const { return bLeft; }
};

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Controller
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxColorChildWindow : public SfxChildWindow
{
 public:
    SvxColorChildWindow( Window*, sal_uInt16, SfxBindings*,
                         SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW(SvxColorChildWindow);
};

/*************************************************************************
|*
|* SvxColorDockingWindow
|*
\************************************************************************/

class SvxColorDockingWindow : public SfxDockingWindow, public SfxListener
{
 friend class SvxColorChildWindow;

private:
    XColorTable*        pColorTable;
    SvxColorValueSet    aColorSet;
    sal_uInt16              nLeftSlot;
    sal_uInt16              nRightSlot;
    sal_uInt16              nCols;
    sal_uInt16              nLines;
    long                nCount;
    Size                aColorSize;
    Size                aItemSize;

//#if 0 // _SOLAR__PRIVATE
    void                FillValueSet();
    void                SetSize();
                        DECL_LINK( SelectHdl, void * );

    /** This function is called when the window gets the focus.  It grabs
        the focus to the color value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);
//#endif

protected:
    virtual sal_Bool    Close();
    virtual void    Resize();
    virtual void    Resizing( Size& rSize );

public:
                    SvxColorDockingWindow( SfxBindings* pBindings,
                                           SfxChildWindow *pCW,
                                           Window* pParent,
                                           const ResId& rResId );
                    ~SvxColorDockingWindow();

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            SetSlotIDs( sal_uInt16 nLeft, sal_uInt16 nRight )
                        { nLeftSlot = nLeft; nRightSlot = nRight; }

    virtual long    Notify( NotifyEvent& rNEvt );
};

#endif

