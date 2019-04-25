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

#pragma once

#include <scdllapi.h>
#include <cppuhelper/weakref.hxx>
#include <tools/wintypes.hxx>
#include <editeng/editview.hxx>
#include <editeng/svxenum.hxx>
#include <vcl/customweld.hxx>

#include <functional>

namespace com { namespace sun { namespace star { namespace accessibility { class XAccessible; } } } }

class ScHeaderEditEngine;
class ScPatternAttr;
class EditView;
class EditTextObject;
class SvxFieldItem;
class ScAccessibleEditObject;
namespace vcl { class Window; }

enum ScEditWindowLocation
{
    Left,
    Center,
    Right
};

class SC_DLLPUBLIC ScEditWindow : public weld::CustomWidgetController
                                , public EditViewCallbacks
{
public:
    ScEditWindow(ScEditWindowLocation eLoc, weld::Window* pParent);
    virtual void SetDrawingArea(weld::DrawingArea* pArea) override;
    virtual ~ScEditWindow() override;

    void            SetFont( const ScPatternAttr& rPattern );
    void            SetText( const EditTextObject& rTextObject );
    std::unique_ptr<EditTextObject> CreateTextObject();
    void            SetCharAttributes();

    void            InsertField( const SvxFieldItem& rFld );

    void            SetNumType(SvxNumType eNumType);

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    ScHeaderEditEngine*  GetEditEngine() const { return pEdEngine.get(); }
    void SetObjectSelectHdl( const Link<ScEditWindow&,void>& aLink) { aObjectSelectLink = aLink; }
    void SetGetFocusHdl(const std::function<void (ScEditWindow&)>& rLink) { m_GetFocusLink = rLink; }

protected:
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual bool    MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual bool    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    Resize() override;

    virtual void EditViewInvalidate(const tools::Rectangle& rRect) const override
    {
        weld::DrawingArea* pDrawingArea = GetDrawingArea();
        pDrawingArea->queue_draw_area(rRect.Left(), rRect.Top(), rRect.GetWidth(), rRect.GetHeight());
    }

    virtual void EditViewSelectionChange() const override
    {
        weld::DrawingArea* pDrawingArea = GetDrawingArea();
        pDrawingArea->queue_draw();
    }

    virtual OutputDevice& EditViewOutputDevice() const override
    {
        return GetDrawingArea()->get_ref_device();
    }

private:
    std::unique_ptr<ScHeaderEditEngine> pEdEngine;
    std::unique_ptr<EditView>           pEdView;
    ScEditWindowLocation eLocation;
    bool mbRTL;
    weld::Window* mpDialog;

    css::uno::WeakReference< css::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;

    Link<ScEditWindow&,void> aObjectSelectLink;
    std::function<void (ScEditWindow&)> m_GetFocusLink;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
