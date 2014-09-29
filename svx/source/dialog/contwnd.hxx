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
    tools::PolyPolygon         aPolyPoly;
    Color               aPipetteColor;
    Rectangle           aWorkRect;
    Link                aPipetteLink;
    Link                aPipetteClickLink;
    Link                aWorkplaceClickLink;
    bool                bPipetteMode;
    bool                bWorkplaceMode;
    bool                bClickValid;

protected:

    virtual void        MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        SdrObjCreated( const SdrObject& rObj ) SAL_OVERRIDE;
    virtual void        InitSdrModel() SAL_OVERRIDE;
    virtual void        Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual Size        GetOptimalSize() const SAL_OVERRIDE;

    void                CreatePolyPolygon();

public:

    ContourWindow(vcl::Window* pParent, WinBits nBits);
    virtual ~ContourWindow();

    void                SetPolyPolygon( const tools::PolyPolygon& rPolyPoly );
    const tools::PolyPolygon&  GetPolyPolygon();

    void                SetPipetteMode( const bool bPipette ) { bPipetteMode = bPipette; }
    bool                IsPipetteMode() const { return bPipetteMode; }
    const Color&        GetPipetteColor() const { return aPipetteColor; }

    bool                IsClickValid() const { return bClickValid; }
    bool                IsContourChanged() const;

    void                SetWorkplaceMode( const bool bWorkplace ) { bWorkplaceMode = bWorkplace; }
    bool                IsWorkplaceMode() const { return bWorkplaceMode; }
    const Rectangle&    GetWorkRect() const { return aWorkRect; }

    void                SetPipetteHdl( const Link& rLink ) { aPipetteLink = rLink; }
    void                SetPipetteClickHdl( const Link& rLink ) { aPipetteClickLink = rLink; }

    void                SetWorkplaceClickHdl( const Link& rLink ) { aWorkplaceClickLink = rLink; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
