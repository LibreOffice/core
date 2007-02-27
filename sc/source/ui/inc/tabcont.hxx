/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabcont.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:25:53 $
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

#ifndef SC_TABCONT_HXX
#define SC_TABCONT_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _TABBAR_HXX //autogen wg. TabBar
#include <svtools/tabbar.hxx>
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif


class ScViewData;

// ---------------------------------------------------------------------------

//  initial size
#define SC_TABBAR_DEFWIDTH      270


class ScTabControl : public TabBar, public DropTargetHelper, public DragSourceHelper
{
private:
    ScViewData*     pViewData;
    USHORT          nMouseClickPageId;      /// Last page ID after mouse button down/up
    USHORT          nSelPageIdByMouse;      /// Selected page ID, if selected with mouse
    BOOL            bErrorShown;

    void            DoDrag( const Region& rRegion );

    USHORT          GetMaxId() const;
    SCTAB           GetPrivatDropPos(const Point& rPos );

protected:
    virtual void    Select();
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    virtual long    StartRenaming();
    virtual long    AllowRenaming();
    virtual void    EndRenaming();
    virtual void    Mirror();

public:
                    ScTabControl( Window* pParent, ScViewData* pData );
                    ~ScTabControl();

    using TabBar::StartDrag;

    void            UpdateStatus();
    void            ActivateView(BOOL bActivate);

    void            SetSheetLayoutRTL( BOOL bSheetRTL );
};



#endif
