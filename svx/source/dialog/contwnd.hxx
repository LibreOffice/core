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

#ifndef INCLUDED_SVX_SOURCE_DIALOG_CONTWND_HXX
#define INCLUDED_SVX_SOURCE_DIALOG_CONTWND_HXX

#include <tools/poly.hxx>
#include <svx/graphctl.hxx>

class ContourWindow : public GraphCtrl
{
    PolyPolygon         aPolyPoly;
    Color               aPipetteColor;
    Rectangle           aWorkRect;
    Link                aPipetteLink;
    Link                aPipetteClickLink;
    Link                aWorkplaceClickLink;
    sal_Bool                bPipetteMode;
    sal_Bool                bWorkplaceMode;
    sal_Bool                bClickValid;

protected:

    virtual void        MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        SdrObjCreated( const SdrObject& rObj ) SAL_OVERRIDE;
    virtual void        InitSdrModel() SAL_OVERRIDE;
    virtual void        Paint( const Rectangle& rRect ) SAL_OVERRIDE;

    void                CreatePolyPolygon();

public:

                        ContourWindow( Window* pParent, const ResId& rResId );
                        ~ContourWindow();

    void                SetPolyPolygon( const PolyPolygon& rPolyPoly );
    const PolyPolygon&  GetPolyPolygon();

    void                SetPipetteMode( const sal_Bool bPipette ) { bPipetteMode = bPipette; }
    sal_Bool                IsPipetteMode() const { return bPipetteMode; }
    const Color&        GetPipetteColor() const { return aPipetteColor; }

    sal_Bool                IsClickValid() const { return bClickValid; }
    bool                IsContourChanged() const;

    void                SetWorkplaceMode( const sal_Bool bWorkplace ) { bWorkplaceMode = bWorkplace; }
    sal_Bool                IsWorkplaceMode() const { return bWorkplaceMode; }
    const Rectangle&    GetWorkRect() const { return aWorkRect; }

    void                SetPipetteHdl( const Link& rLink ) { aPipetteLink = rLink; }
    void                SetPipetteClickHdl( const Link& rLink ) { aPipetteClickLink = rLink; }

    void                SetWorkplaceClickHdl( const Link& rLink ) { aWorkplaceClickLink = rLink; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
