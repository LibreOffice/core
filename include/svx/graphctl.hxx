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
#ifndef INCLUDED_SVX_GRAPHCTL_HXX
#define INCLUDED_SVX_GRAPHCTL_HXX

#include <vcl/ctrl.hxx>
#include <vcl/graph.hxx>
#include <svx/svxdllapi.h>

#include <svx/svdview.hxx>
#include <svx/svdobj.hxx>

class GraphCtrlUserCall;
class SvxGraphCtrlAccessibleContext;

class SVX_DLLPUBLIC GraphCtrl : public Control
{
    friend class GraphCtrlView;
    friend class GraphCtrlUserCall;

    Graphic             aGraphic;
    Idle                aUpdateIdle;
    Link<GraphCtrl*,void>  aMousePosLink;
    Link<GraphCtrl*,void>  aGraphSizeLink;
    Link<GraphCtrl*,void>  aUpdateLink;
    MapMode             aMap100;
    Size                aGraphSize;
    Point               aMousePos;
    GraphCtrlUserCall*  pUserCall;
    SdrObjKind          eObjKind;
    sal_uInt16          nPolyEdit;
    bool                bEditMode;
    bool                mbSdrMode;
    bool                mbInIdleUpdate;

                        DECL_LINK( UpdateHdl, Timer*, void );

    rtl::Reference<SvxGraphCtrlAccessibleContext> mpAccContext;

protected:

    SdrModel*           pModel;
    SdrView*            pView;

    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        Resize() override;
    virtual void        KeyInput(const KeyEvent& rKEvt) override;
    virtual void        MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void        MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void        MouseMove(const MouseEvent& rMEvt) override;

    virtual void        InitSdrModel();

    virtual void        SdrObjCreated( const SdrObject& rObj );
    virtual void        SdrObjChanged( const SdrObject& rObj );
    virtual void        MarkListHasChanged();

    inline SdrObjUserCall* GetSdrUserCall();

public:

    GraphCtrl( vcl::Window* pParent, WinBits nStyle );
    virtual ~GraphCtrl() override;
    virtual void dispose() override;

    void                SetGraphic( const Graphic& rGraphic, bool bNewModel = true );
    const Graphic&      GetGraphic() const { return aGraphic; }
    const Size&         GetGraphicSize() const { return aGraphSize; }

    const Point&        GetMousePos() const { return aMousePos; }

    void                SetEditMode( const bool bEditMode );

    void                SetPolyEditMode( const sal_uInt16 nPolyEdit );
    sal_uInt16          GetPolyEditMode() const { return nPolyEdit; }

    void                SetObjKind( const SdrObjKind eObjKind );

    SdrModel*           GetSdrModel() const { return pModel; }
    SdrView*            GetSdrView() const { return pView; }
    SdrObject*          GetSelectedSdrObject() const;
    bool                IsChanged() const { return mbSdrMode && pModel->IsChanged(); }

    void                SetMousePosLink( const Link<GraphCtrl*,void>& rLink ) { aMousePosLink = rLink; }

    void                SetGraphSizeLink( const Link<GraphCtrl*,void>& rLink ) { aGraphSizeLink = rLink; }

    void                SetUpdateLink( const Link<GraphCtrl*,void>& rLink ) { aUpdateLink = rLink; }
    void                QueueIdleUpdate();

    void                SetSdrMode(bool b);

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;
};


class GraphCtrlUserCall : public SdrObjUserCall
{
    GraphCtrl& rWin;

public:

    GraphCtrlUserCall(GraphCtrl& rGraphWin)
        : rWin(rGraphWin)
    {}

    virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const tools::Rectangle& rOldBoundRect) override;
};

SdrObjUserCall* GraphCtrl::GetSdrUserCall()
{
    return pUserCall;
}

class GraphCtrlView : public SdrView
{
    GraphCtrl& rGraphCtrl;

protected:

    virtual void MarkListHasChanged() override
    {
        SdrView::MarkListHasChanged();
        rGraphCtrl.MarkListHasChanged();
    }

public:
    GraphCtrlView(SdrModel* pModel, GraphCtrl* pWindow)
        : SdrView(pModel, pWindow)
        , rGraphCtrl(*pWindow)
    {}
};

#endif // INCLUDED_SVX_GRAPHCTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
