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

#ifndef SC_TABCONT_HXX
#define SC_TABCONT_HXX

#include "address.hxx"
#include <svtools/tabbar.hxx>
#include <svtools/transfer.hxx>


class ScViewData;

// ---------------------------------------------------------------------------

//  initial size
#define SC_TABBAR_DEFWIDTH      270


class ScTabControl : public TabBar, public DropTargetHelper, public DragSourceHelper
{
private:
    ScViewData*     pViewData;
    sal_uInt16          nMouseClickPageId;      /// Last page ID after mouse button down/up
    sal_uInt16          nSelPageIdByMouse;      /// Selected page ID, if selected with mouse
    sal_Bool            bErrorShown;

    void            DoDrag( const Region& rRegion );

    sal_uInt16          GetMaxId() const;
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
    void            ActivateView(sal_Bool bActivate);

    void            SetSheetLayoutRTL( sal_Bool bSheetRTL );
};



#endif
