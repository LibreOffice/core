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
#include <editutil.hxx>
#include <cppuhelper/weakref.hxx>
#include <svx/weldeditview.hxx>
#include <editeng/svxenum.hxx>
#include <unotools/weakref.hxx>

#include <functional>

namespace com::sun::star::accessibility { class XAccessible; }

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

class SC_DLLPUBLIC ScEditWindow : public WeldEditView
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

    ScHeaderEditEngine* GetEditEngine() const override;
    void SetObjectSelectHdl( const Link<ScEditWindow&,void>& aLink) { aObjectSelectLink = aLink; }
    void SetGetFocusHdl(const std::function<void (ScEditWindow&)>& rLink) { m_GetFocusLink = rLink; }

protected:
    virtual void makeEditEngine() override;
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

private:
    ScEditWindowLocation eLocation;
    bool mbRTL;
    weld::Window* mpDialog;

    unotools::WeakReference<ScAccessibleEditObject> mxAcc;

    Link<ScEditWindow&,void> aObjectSelectLink;
    std::function<void (ScEditWindow&)> m_GetFocusLink;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
