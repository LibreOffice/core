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

#include <svx/weldeditview.hxx>

class OutlinerView;
class SwView;
class SwPostItMgr;
struct SpellCallbackInfo;
namespace sw::annotation { class SwAnnotationWin; }

namespace sw::sidebarwindows {

class SidebarTextControl : public WeldEditView
{
    private:
        sw::annotation::SwAnnotationWin& mrSidebarWin;
        SwView& mrDocView;
        SwPostItMgr& mrPostItMgr;
        bool mbMouseDownGainingFocus;

        void MakeVisible();

    protected:
        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

        virtual bool Command(const CommandEvent& rCEvt) override;
        virtual void GetFocus() override;
        virtual void LoseFocus() override;

        virtual OUString RequestHelp(tools::Rectangle& rRect) override;

    public:
        SidebarTextControl(sw::annotation::SwAnnotationWin& rSidebarWin,
                           SwView& rDocView,
                           SwPostItMgr& rPostItMgr);

        virtual EditView* GetEditView() const override;

        virtual EditEngine* GetEditEngine() const override;

        virtual void EditViewScrollStateChange() override;

        void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

        void SetCursorLogicPosition(const Point& rPosition, bool bPoint, bool bClearMark);

        virtual bool KeyInput(const KeyEvent& rKeyEvt) override;
        virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
        virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;

        OutlinerView* GetTextView() const;

        DECL_LINK( OnlineSpellCallback, SpellCallbackInfo&, void );

        void DrawForPage(OutputDevice* pDev, const Point& rPos);
};

} // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
