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

#ifndef SC_FUDRAW_HXX
#define SC_FUDRAW_HXX

#include "fupoor.hxx"
#include <vcl/pointr.hxx>

/** Base class for all Drawmodule specific functions */
class FuDraw : public FuPoor
{
 protected:
    Pointer aNewPointer;
    Pointer aOldPointer;

 public:
    FuDraw(ScTabViewShell* pViewSh, Window* pWin, ScDrawView* pView,
           SdrModel* pDoc, SfxRequest& rReq);
    virtual ~FuDraw();

    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);

    virtual void Activate();
    virtual void Deactivate();

    virtual void ForcePointer(const MouseEvent* pMEvt);

    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    // II
    virtual void SelectionHasChanged();

    sal_Bool IsSizingOrMovingNote( const MouseEvent& rMEvt ) const;

 private:
    void    DoModifiers(const MouseEvent& rMEvt);
    void    ResetModifiers();
};



#endif      // _SD_FUDRAW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
