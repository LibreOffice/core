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
#ifndef SD_DIALOGLISTBOX_HXX
#define SD_DIALOGLISTBOX_HXX

#include <vcl/ctrl.hxx>
#include <vcl/scrbar.hxx>

namespace sd
{

class DialogListBox : public Control
{
private:
    ScrollBar*          mpHScrollBar;
    ScrollBar*          mpVScrollBar;
    ScrollBarBox*       mpScrollBarBox;
    ::Window*           mpChild;
    bool                mbVScroll;
    bool                mbHScroll;
    bool                mbAutoHScroll;
    Size                maMinSize, maInnerSize;

protected:
    virtual void        GetFocus();
    virtual void        StateChanged( StateChangedType nType );

    long                Notify( NotifyEvent& rNEvt );

    void                ImplResizeControls();
    void                ImplCheckScrollBars();
    void                ImplInitScrollBars();
    void                ImplResizeChild();

    DECL_LINK(          ScrollBarHdl, ScrollBar* );

public:
                    DialogListBox( ::Window* pParent, WinBits nWinStyle );
                    ~DialogListBox();

    void            SetChildWindow( ::Window* pChild, const Size& rMinSize );

    ::Window*           GetPreferredKeyInputWindow();
    void            Resize();

};

} //  namespace sd

// SD_DIALOGLISTBOX_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
