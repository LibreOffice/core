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
#ifndef _CONTIMP_HXX_
#define _CONTIMP_HXX_

#include <svx/contdlg.hxx>
#include "contwnd.hxx"
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>

#define CONT_RESID(nId)  ResId( nId, DIALOG_MGR() )

class SvxSuperContourDlg : public SvxContourDlg
{
    using SvxContourDlg::GetPolyPolygon;

    Graphic             aGraphic;
    Graphic             aUndoGraphic;
    Graphic             aRedoGraphic;
    Graphic             aUpdateGraphic;
    PolyPolygon         aUpdatePolyPoly;
    Timer               aUpdateTimer;
    Timer               aCreateTimer;
    Size                aLastSize;
    void*               pUpdateEditingObject;
    void*               pCheckObj;
    SvxContourDlgItem   aContourItem;
    ToolBox             aTbx1;
    MetricField         aMtfTolerance;
    ContourWindow       aContourWnd;
    StatusBar           aStbStatus;
    sal_uIntPtr             nGrfChanged;
    sal_Bool                bExecState;
    sal_Bool                bUpdateGraphicLinked;
    sal_Bool                bGraphicLinked;
    ImageList           maImageList;

    virtual void        Resize();
    virtual sal_Bool        Close();

                        DECL_LINK( Tbx1ClickHdl, ToolBox* );
                        DECL_LINK( MousePosHdl, ContourWindow* );
                        DECL_LINK( GraphSizeHdl, ContourWindow* );
                        DECL_LINK(UpdateHdl, void *);
                        DECL_LINK(CreateHdl, void *);
                        DECL_LINK( StateHdl, ContourWindow* );
                        DECL_LINK( PipetteHdl, ContourWindow* );
                        DECL_LINK( PipetteClickHdl, ContourWindow* );
                        DECL_LINK( WorkplaceClickHdl, ContourWindow* );
                        DECL_LINK( MiscHdl, void* );

public:

                        SvxSuperContourDlg( SfxBindings *pBindings, SfxChildWindow *pCW,
                                            Window* pParent, const ResId& rResId );
                        ~SvxSuperContourDlg();

    void                SetExecState( sal_Bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    void                SetGraphicLinked( sal_Bool bLinked ) { bGraphicLinked = bLinked; }
    const Graphic&      GetGraphic() const { return aGraphic; }
    sal_Bool                IsGraphicChanged() const { return nGrfChanged > 0UL; }

    void                SetPolyPolygon( const PolyPolygon& rPolyPoly );
    PolyPolygon         GetPolyPolygon( sal_Bool bRescaleToGraphic = sal_True );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    sal_Bool                IsUndoPossible() const;
    sal_Bool                IsRedoPossible() const;

    void                UpdateGraphic( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL,
                                void* pEditingObj = NULL );

    /** switches the toolbox images depending on the actuall high contrast display mode state */
    void                ApplyImageList();

    /** virtual method from Window is used to detect change in high contrast display mode
        to switch the toolbox images */
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

};


#endif // _CONTIMP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
