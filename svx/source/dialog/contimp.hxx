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

#include <sfx2/ctrlitem.hxx>
#include "contwnd.hxx"
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

class ContourWindow;

class StatusColor : public weld::CustomWidgetController
{
private:
    ContourWindow& m_rWnd;
public:
    StatusColor(ContourWindow& rWnd)
        : m_rWnd(rWnd)
    {
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        weld::CustomWidgetController::SetDrawingArea(pDrawingArea);
        Size aSize(pDrawingArea->get_approximate_digit_width() * 3,
                   pDrawingArea->get_text_height());
        pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
        SetOutputSizePixel(aSize);
    }
};

class SvxSuperContourDlg
{
    Graphic             aGraphic;
    Graphic             aUndoGraphic;
    Graphic             aRedoGraphic;
    Graphic             aUpdateGraphic;
    tools::PolyPolygon  aUpdatePolyPoly;
    Idle                aUpdateIdle;
    Idle                aCreateIdle;
    SfxBindings*        mpBindings;
    void*               pUpdateEditingObject;
    void*               pCheckObj;
    SvxContourDlgItem   aContourItem;
    sal_Int32           mnGrfChanged;
    bool                bExecState;
    bool                bUpdateGraphicLinked;
    bool                bGraphicLinked;

    weld::Dialog& m_rDialog;
    std::unique_ptr<ContourWindow> m_xContourWnd;
    std::unique_ptr<StatusColor> m_xStbStatusColor;
    std::unique_ptr<weld::Toolbar> m_xTbx1;
    std::unique_ptr<weld::MetricSpinButton> m_xMtfTolerance;
    std::unique_ptr<weld::Label> m_xStbStatus2;
    std::unique_ptr<weld::Label> m_xStbStatus3;
    std::unique_ptr<weld::Button> m_xCancelBtn;
    std::unique_ptr<weld::CustomWeld> m_xStbStatusColorWeld;
    std::unique_ptr<weld::CustomWeld> m_xContourWndWeld;

    DECL_LINK( Tbx1ClickHdl, const OString&, void );
    DECL_LINK( MousePosHdl, GraphCtrl*, void );
    DECL_LINK( GraphSizeHdl, GraphCtrl*, void );
    DECL_LINK( UpdateHdl, Timer *, void );
    DECL_LINK( CreateHdl, Timer *, void );
    DECL_LINK( StateHdl, GraphCtrl*, void );
    DECL_LINK( PipetteHdl, ContourWindow&, void );
    DECL_LINK( PipetteClickHdl, ContourWindow&, void );
    DECL_LINK( WorkplaceClickHdl, ContourWindow&, void );
    DECL_LINK( CancelHdl, weld::Button&, void );

    void SetActiveTool(std::string_view rId);
    void SetActivePoly(std::string_view rId);

    SfxBindings& GetBindings() { return *mpBindings; }

public:

    SvxSuperContourDlg(weld::Builder& rBuilder, weld::Dialog& rDialog, SfxBindings* pBindings);
    ~SvxSuperContourDlg();

    void                SetExecState( bool bEnable );

    void                SetGraphic( const Graphic& rGraphic );
    const Graphic&      GetGraphic() const { return aGraphic; }
    bool                IsGraphicChanged() const { return mnGrfChanged > 0; }

    void                SetPolyPolygon( const tools::PolyPolygon& rPolyPoly );
    tools::PolyPolygon  GetPolyPolygon();

    const void*         GetEditingObject() const { return pCheckObj; }

    void                UpdateGraphic( const Graphic& rGraphic, bool bGraphicLinked,
                                const tools::PolyPolygon* pPolyPoly,
                                void* pEditingObj );
};


#endif // INCLUDED_SVX_SOURCE_DIALOG_CONTIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
