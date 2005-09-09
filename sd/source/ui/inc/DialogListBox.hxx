/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DialogListBox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:03:13 $
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
#ifndef SD_DIALOGLISTBOX_HXX
#define SD_DIALOGLISTBOX_HXX

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif

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
