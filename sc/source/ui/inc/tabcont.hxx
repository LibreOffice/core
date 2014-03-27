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

#ifndef SC_TABCONT_HXX
#define SC_TABCONT_HXX

#include "address.hxx"
#include <svtools/tabbar.hxx>
#include <svtools/transfer.hxx>


class ScViewData;



//  initial size
#define SC_TABBAR_DEFWIDTH      270


class ScTabControl : public TabBar, public DropTargetHelper, public DragSourceHelper
{
private:
    ScViewData*     pViewData;
    sal_uInt16      nMouseClickPageId;      /// Last page ID after mouse button down/up
    sal_uInt16      nSelPageIdByMouse;      /// Selected page ID, if selected with mouse
    bool            bErrorShown;

    void            DoDrag( const Region& rRegion );

    sal_uInt16          GetMaxId() const;
    SCTAB           GetPrivatDropPos(const Point& rPos );

    DECL_LINK(ShowPageList, const CommandEvent*);

    void SwitchToPageId(sal_uInt16 nId);
protected:
    virtual void    Select() SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

    virtual long    StartRenaming() SAL_OVERRIDE;
    virtual long    AllowRenaming() SAL_OVERRIDE;
    virtual void    EndRenaming() SAL_OVERRIDE;
    virtual void    Mirror() SAL_OVERRIDE;

public:
                    ScTabControl( Window* pParent, ScViewData* pData );
                    ~ScTabControl();

    using TabBar::StartDrag;

    void            UpdateInputContext();
    void            UpdateStatus();

    void            SetSheetLayoutRTL( bool bSheetRTL );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
