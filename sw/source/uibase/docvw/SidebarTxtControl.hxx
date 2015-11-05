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

namespace sw { namespace sidebarwindows {

class SwSidebarWin;

class SidebarTextControl : public Control
{
    private:
        SwSidebarWin& mrSidebarWin;
        SwView& mrDocView;
        SwPostItMgr& mrPostItMgr;

    protected:
        virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
        virtual void    KeyInput( const KeyEvent& rKeyEvt ) override;
        virtual void    MouseMove( const MouseEvent& rMEvt ) override;
        virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
        virtual void    Command( const CommandEvent& rCEvt ) override;
        virtual void    LoseFocus() override;
        virtual void    RequestHelp(const HelpEvent &rEvt) override;
        virtual OUString GetSurroundingText() const override;
        virtual Selection GetSurroundingTextSelection() const override;

        DECL_LINK_TYPED( Select, Menu*, bool );

    public:
        SidebarTextControl( SwSidebarWin& rSidebarWin,
                           WinBits nBits,
                           SwView& rDocView,
                           SwPostItMgr& rPostItMgr );
        virtual ~SidebarTextControl();
        virtual void dispose() override;

        virtual void GetFocus() override;

        OutlinerView* GetTextView() const;

        DECL_LINK_TYPED( OnlineSpellCallback, SpellCallbackInfo&, void );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() override;

        virtual void Draw(OutputDevice* pDev, const Point&, const Size&, DrawFlags) override;
        void PaintTile(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
};

} } // end of namespace sw::sidebarwindows

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
