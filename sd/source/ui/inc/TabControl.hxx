/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef SD_TAB_CONTROL_HXX
#define SD_TAB_CONTROL_HXX

#include <svtools/tabbar.hxx>
#include <svtools/transfer.hxx>

namespace sd {

/**
 * TabControl-Class for page switch
 */

class DrawViewShell;

class TabControl
    : public TabBar,
      public DragSourceHelper,
      public DropTargetHelper
{
public:
    TabControl (DrawViewShell* pDrViewSh, ::Window* pParent);
    virtual ~TabControl (void);

    /** Inform all listeners of this control that the current page has been
        activated.  Call this method after switching the current page and is
        not done elsewhere (like when using page up/down keys).
    */
    void SendActivatePageEvent (void);

    /** Inform all listeners of this control that the current page has been
        deactivated.  Call this method before switching the current page and
        is not done elsewhere (like when using page up/down keys).
    */
    void SendDeactivatePageEvent (void);

protected:
    DrawViewShell*  pDrViewSh;
    sal_Bool                bInternalMove;

    // TabBar
    virtual void        Select();
    virtual void        DoubleClick();
    virtual void        MouseButtonDown(const MouseEvent& rMEvt);

    virtual void        Command(const CommandEvent& rCEvt);

    virtual long        StartRenaming();
    virtual long        AllowRenaming();
    virtual void        EndRenaming();

    virtual void        ActivatePage();
    virtual long        DeactivatePage();

    // DragSourceHelper
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // DropTargetHelper
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

                        DECL_LINK(DeactivatePageHdl, void*);

private:
    // nested class to implement the TransferableHelper
    class TabControlTransferable : public TransferableHelper
    {
    public:
        TabControlTransferable( TabControl& rParent ) :
            mrParent( rParent ) {}
    private:

        TabControl&     mrParent;

        virtual             ~TabControlTransferable();

        virtual void        AddSupportedFormats();
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        virtual void        DragFinished( sal_Int8 nDropAction );

    };

    friend class TabControl::TabControlTransferable;

    void                DragFinished( sal_Int8 nDropAction );

private:
    using TabBar::StartDrag;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
