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
    tools::PolyPolygon         aUpdatePolyPoly;
    Timer               aUpdateTimer;
    Timer               aCreateTimer;
    Size                aLastSize;
    void*               pUpdateEditingObject;
    void*               pCheckObj;
    SvxContourDlgItem   aContourItem;
    ToolBox*            m_pTbx1;
    MetricField*        m_pMtfTolerance;
    ContourWindow*      m_pContourWnd;
    StatusBar*          m_pStbStatus;
    sal_uIntPtr         nGrfChanged;
    bool                bExecState;
    bool                bUpdateGraphicLinked;
    bool                bGraphicLinked;

    sal_uInt16          mnApplyId;
    sal_uInt16          mnWorkSpaceId;
    sal_uInt16          mnSelectId;
    sal_uInt16          mnRectId;
    sal_uInt16          mnCircleId;
    sal_uInt16          mnPolyId;
    sal_uInt16          mnPolyEditId;
    sal_uInt16          mnPolyMoveId;
    sal_uInt16          mnPolyInsertId;
    sal_uInt16          mnPolyDeleteId;
    sal_uInt16          mnAutoContourId;
    sal_uInt16          mnUndoId;
    sal_uInt16          mnRedoId;
    sal_uInt16          mnPipetteId;

    virtual bool        Close() SAL_OVERRIDE;

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

                        SvxSuperContourDlg(SfxBindings *pBindings, SfxChildWindow *pCW,
                                           vcl::Window* pParent);
                        virtual ~SvxSuperContourDlg();

    void                SetExecState( bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    void                SetGraphicLinked( bool bLinked ) { bGraphicLinked = bLinked; }
    const Graphic&      GetGraphic() const { return aGraphic; }
    bool                IsGraphicChanged() const { return nGrfChanged > 0UL; }

    void                SetPolyPolygon( const tools::PolyPolygon& rPolyPoly );
    tools::PolyPolygon         GetPolyPolygon( bool bRescaleToGraphic = true );

    void                SetEditingObject( void* pObj ) { pCheckObj = pObj; }
    const void*         GetEditingObject() const { return pCheckObj; }

    bool                IsUndoPossible() const;
    bool                IsRedoPossible() const;

    void                UpdateGraphic( const Graphic& rGraphic, bool bGraphicLinked,
                                const tools::PolyPolygon* pPolyPoly = NULL,
                                void* pEditingObj = NULL );
};


#endif // INCLUDED_SVX_SOURCE_DIALOG_CONTIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
