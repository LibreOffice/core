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
#include <vcl/idle.hxx>

class SvxSuperContourDlg;

class SvxContourDlgItem : public SfxControllerItem
{
    SvxSuperContourDlg& rDlg;

protected:

    virtual void        StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;

public:

    SvxContourDlgItem( SvxSuperContourDlg& rDlg, SfxBindings& rBindings );
};

class SvxSuperContourDlg : public SvxContourDlg
{
    using SvxContourDlg::GetPolyPolygon;

    Graphic             aGraphic;
    Graphic             aUndoGraphic;
    Graphic             aRedoGraphic;
    Graphic             aUpdateGraphic;
    tools::PolyPolygon         aUpdatePolyPoly;
    Idle                aUpdateIdle;
    Idle                aCreateIdle;
    Size                aLastSize;
    void*               pUpdateEditingObject;
    void*               pCheckObj;
    SvxContourDlgItem   aContourItem;
    VclPtr<ToolBox>     m_pTbx1;
    VclPtr<MetricField> m_pMtfTolerance;
    VclPtr<ContourWindow> m_pContourWnd;
    VclPtr<StatusBar>   m_pStbStatus;
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

    virtual bool        Close() override;

                        DECL_LINK( Tbx1ClickHdl, ToolBox*, void );
                        DECL_LINK( MousePosHdl, GraphCtrl*, void );
                        DECL_LINK( GraphSizeHdl, GraphCtrl*, void );
                        DECL_LINK( UpdateHdl, Timer *, void );
                        DECL_LINK( CreateHdl, Timer *, void );
                        DECL_LINK( StateHdl, GraphCtrl*, void );
                        DECL_LINK( PipetteHdl, ContourWindow&, void );
                        DECL_LINK( PipetteClickHdl, ContourWindow&, void );
                        DECL_LINK( WorkplaceClickHdl, ContourWindow&, void );
                        DECL_LINK( MiscHdl, LinkParamNone*, void );

public:

                        SvxSuperContourDlg(SfxBindings *pBindings, SfxChildWindow *pCW,
                                           vcl::Window* pParent);
                        virtual ~SvxSuperContourDlg() override;
    virtual void        dispose() override;

    void                SetExecState( bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    const Graphic&      GetGraphic() const { return aGraphic; }
    bool                IsGraphicChanged() const { return nGrfChanged > 0; }

    void                SetPolyPolygon( const tools::PolyPolygon& rPolyPoly );
    tools::PolyPolygon  GetPolyPolygon();

    const void*         GetEditingObject() const { return pCheckObj; }

    void                UpdateGraphic( const Graphic& rGraphic, bool bGraphicLinked,
                                const tools::PolyPolygon* pPolyPoly,
                                void* pEditingObj );
};


#endif // INCLUDED_SVX_SOURCE_DIALOG_CONTIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
