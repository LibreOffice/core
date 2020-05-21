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

#ifndef INCLUDED_SW_SOURCE_UIBASE_DOCVW_SIDEBARTXTCONTROL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_DOCVW_SIDEBARTXTCONTROL_HXX

#include <vcl/ctrl.hxx>

class OutlinerView;
class SwView;
class SwPostItMgr;
struct SpellCallbackInfo;
namespace sw::annotation { class SwAnnotationWin; }

namespace sw::sidebarwindows {

class SidebarTextControl : public Control
{
    private:
        sw::annotation::SwAnnotationWin& mrSidebarWin;
        SwView& mrDocView;
        SwPostItMgr& mrPostItMgr;

    protected:
        virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
        /// @see Window::LogicInvalidate().
        void LogicInvalidate(const tools::Rectangle* pRectangle) override;
        virtual void    Command( const CommandEvent& rCEvt ) override;
        virtual void    LoseFocus() override;
        virtual void    RequestHelp(const HelpEvent &rEvt) override;
        virtual OUString GetSurroundingText() const override;
        virtual Selection GetSurroundingTextSelection() const override;

    public:
        SidebarTextControl( sw::annotation::SwAnnotationWin& rSidebarWin,
                           WinBits nBits,
                           SwView& rDocView,
                           SwPostItMgr& rPostItMgr );
        virtual ~SidebarTextControl() override;
        virtual void dispose() override;

        virtual void GetFocus() override;
        virtual void KeyInput( const KeyEvent& rKeyEvt ) override;
        virtual void MouseButtonDown(const MouseEvent& rMouseEvent) override;
        virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
        virtual void MouseMove(const MouseEvent& rMEvt) override;

        OutlinerView* GetTextView() const;

        DECL_LINK( OnlineSpellCallback, SpellCallbackInfo&, void );

        virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

        virtual void Draw(OutputDevice* pDev, const Point&, DrawFlags) override;
};

} // end of namespace sw::sidebarwindows

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
