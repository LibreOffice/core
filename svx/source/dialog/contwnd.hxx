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

#ifndef _CONTWND_HXX
#define _CONTWND_HXX

#include <tools/poly.hxx>
#include <svx/graphctl.hxx>

/*************************************************************************
|*
|*
|*
\************************************************************************/

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

    virtual void        MouseButtonDown(const MouseEvent& rMEvt);
    virtual void        MouseMove(const MouseEvent& rMEvt);
    virtual void        MouseButtonUp(const MouseEvent& rMEvt);
    virtual void        SdrObjCreated( const SdrObject& rObj );
    virtual void        InitSdrModel();
    virtual void        Paint( const Rectangle& rRect );

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
    sal_Bool                IsContourChanged() const;

    void                SetWorkplaceMode( const sal_Bool bWorkplace ) { bWorkplaceMode = bWorkplace; }
    sal_Bool                IsWorkplaceMode() const { return bWorkplaceMode; }
    const Rectangle&    GetWorkRect() const { return aWorkRect; }

    void                SetPipetteHdl( const Link& rLink ) { aPipetteLink = rLink; }
    void                SetPipetteClickHdl( const Link& rLink ) { aPipetteClickLink = rLink; }

    void                SetWorkplaceClickHdl( const Link& rLink ) { aWorkplaceClickLink = rLink; }
};


#endif

