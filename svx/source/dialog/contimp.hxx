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
#ifndef INCLUDED_SVX_SOURCE_DIALOG_CONTIMP_HXX
#define INCLUDED_SVX_SOURCE_DIALOG_CONTIMP_HXX

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
    virtual bool        Close();

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
    PolyPolygon         GetPolyPolygon( bool bRescaleToGraphic = true );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    bool                IsUndoPossible() const;
    bool                IsRedoPossible() const;

    void                UpdateGraphic( const Graphic& rGraphic, sal_Bool bGraphicLinked,
                                const PolyPolygon* pPolyPoly = NULL,
                                void* pEditingObj = NULL );

    /** switches the toolbox images depending on the actuall high contrast display mode state */
    void                ApplyImageList();

    /** virtual method from Window is used to detect change in high contrast display mode
        to switch the toolbox images */
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

};


#endif // INCLUDED_SVX_SOURCE_DIALOG_CONTIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
