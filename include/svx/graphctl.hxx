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

#define WB_SDRMODE      ((WinBits)0x0080)
#define WB_ANIMATION    ((WinBits)0x0100)

class SVX_DLLPUBLIC GraphCtrl : public Control
{
    friend class GraphCtrlView;
    friend class GraphCtrlUserCall;

    Graphic             aGraphic;
    Timer               aUpdateTimer;
    Link                aMousePosLink;
    Link                aGraphSizeLink;
    Link                aMarkObjLink;
    Link                aUpdateLink;
    MapMode             aMap100;
    Size                aGraphSize;
    Point               aMousePos;
    GraphCtrlUserCall*  pUserCall;
    WinBits             nWinStyle;
    SdrObjKind          eObjKind;
    sal_uInt16          nPolyEdit;
    bool                bEditMode;
    bool                bSdrMode;
    bool                bAnim;

                        DECL_LINK( UpdateHdl, Timer* );

    SvxGraphCtrlAccessibleContext* mpAccContext;

protected:

    SdrModel*           pModel;
    SdrView*            pView;

    virtual void        Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void        Resize() SAL_OVERRIDE;
    virtual void        KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual void        MouseButtonDown(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
    virtual void        MouseMove(const MouseEvent& rMEvt) SAL_OVERRIDE;

    virtual void        InitSdrModel();

    virtual void        SdrObjCreated( const SdrObject& rObj );
    virtual void        SdrObjChanged( const SdrObject& rObj );
    virtual void        MarkListHasChanged();

    SdrObjUserCall*     GetSdrUserCall() { return (SdrObjUserCall*) pUserCall; }

public:

                        GraphCtrl( Window* pParent, const ResId& rResId );
                        GraphCtrl( Window* pParent, WinBits nStyle );
                        virtual ~GraphCtrl();

    void                SetWinStyle( WinBits nWinBits );
    WinBits             GetWinStyle() const { return nWinStyle; }

    void                SetGraphic( const Graphic& rGraphic, bool bNewModel = true );
    const Graphic&      GetGraphic() const { return aGraphic; }
    const Size&         GetGraphicSize() const { return aGraphSize; }

    const Point&        GetMousePos() const { return aMousePos; }

    void                SetEditMode( const bool bEditMode );
    bool                IsEditMode() const { return bEditMode; }

    void                SetPolyEditMode( const sal_uInt16 nPolyEdit );
    sal_uInt16          GetPolyEditMode() const { return nPolyEdit; }

    void                SetObjKind( const SdrObjKind eObjKind );
    SdrObjKind          GetObjKind() const { return eObjKind; }

    SdrModel*           GetSdrModel() const { return pModel; }
    SdrView*            GetSdrView() const { return pView; }
    SdrObject*          GetSelectedSdrObject() const;
    bool                IsChanged() const { return bSdrMode ? pModel->IsChanged() : sal_False; }

    void                SetMousePosLink( const Link& rLink ) { aMousePosLink = rLink; }
    const Link&         GetMousePosLink() const { return aMousePosLink; }

    void                SetGraphSizeLink( const Link& rLink ) { aGraphSizeLink = rLink; }
    const Link&         GetGraphSizeLink() const { return aGraphSizeLink; }

    void                SetMarkObjLink( const Link& rLink ) { aMarkObjLink = rLink; }
    const Link&         GetMarkObjLink() const { return aMarkObjLink; }

    void                SetUpdateLink( const Link& rLink ) { aUpdateLink = rLink; }
    const Link&         GetUpdateLink() const { return aUpdateLink; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;
};


class GraphCtrlUserCall : public SdrObjUserCall
{
    GraphCtrl&      rWin;

public:

                    GraphCtrlUserCall( GraphCtrl& rGraphWin ) : rWin( rGraphWin ) {};
    virtual         ~GraphCtrlUserCall() {};

    virtual void    Changed( const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect ) SAL_OVERRIDE;
};

class GraphCtrlView : public SdrView
{
    GraphCtrl&      rGraphCtrl;

protected:

    virtual void    MarkListHasChanged() SAL_OVERRIDE
                    {
                        SdrView::MarkListHasChanged();
                        rGraphCtrl.MarkListHasChanged();
                    }

public:

                    GraphCtrlView( SdrModel* pModel, GraphCtrl* pWindow) :
                            SdrView     ( pModel, pWindow ),
                            rGraphCtrl  ( *pWindow ) {};

    virtual         ~GraphCtrlView() {};
};

#endif // INCLUDED_SVX_GRAPHCTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
