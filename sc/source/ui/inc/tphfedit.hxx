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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPHFEDIT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPHFEDIT_HXX

#include <scdllapi.h>
#include <cppuhelper/weakref.hxx>
#include <tools/wintypes.hxx>
#include <svx/weldeditview.hxx>
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

    ScHeaderEditEngine* GetEditEngine() const;
    void SetObjectSelectHdl( const Link<ScEditWindow&,void>& aLink) { aObjectSelectLink = aLink; }
    void SetGetFocusHdl(const std::function<void (ScEditWindow&)>& rLink) { m_GetFocusLink = rLink; }

protected:
    virtual void makeEditEngine() override;
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

private:
    ScEditWindowLocation eLocation;
    bool mbRTL;
    weld::Window* mpDialog;

    css::uno::WeakReference< css::accessibility::XAccessible > xAcc;
    ScAccessibleEditObject* pAcc;

    Link<ScEditWindow&,void> aObjectSelectLink;
    std::function<void (ScEditWindow&)> m_GetFocusLink;

};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPHFEDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
