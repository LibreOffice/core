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

#ifndef SD_LAYER_TAB_BAR_HXX
#define SD_LAYER_TAB_BAR_HXX

#include <svtools/tabbar.hxx>
#include <svtools/transfer.hxx>

namespace sd {

/*************************************************************************
|*
|* TabBar fuer die Layerverwaltung
|*
\************************************************************************/

class DrawViewShell;

class LayerTabBar
    : public TabBar,
      public DropTargetHelper
{
public:
    LayerTabBar (
        DrawViewShell* pDrViewSh,
        ::Window* pParent);
    LayerTabBar (
        DrawViewShell* pDrViewSh,
        ::Window* pParent,
        const ResId& rResId);
    virtual ~LayerTabBar (void);

    /** Inform all listeners of this control that the current layer has been
        activated.  Call this method after switching the current layer and is
        not done elsewhere (like when using ctrl + page up/down keys).
    */
    void SendActivatePageEvent (void);

    /** Inform all listeners of this control that the current layer has been
        deactivated.  Call this method before switching the current layer
        and is not done elsewhere (like when using ctrl page up/down keys).
    */
    void SendDeactivatePageEvent (void);

protected:
    DrawViewShell* pDrViewSh;

    // TabBar
    virtual void        Select();
    virtual void        DoubleClick();
    virtual void        MouseButtonDown(const MouseEvent& rMEvt);

    virtual void        Command(const CommandEvent& rCEvt);

    virtual long        StartRenaming();
    virtual long        AllowRenaming();
    virtual void        EndRenaming();

    virtual void        ActivatePage();

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
