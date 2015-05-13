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
        virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect) SAL_OVERRIDE;
        virtual void    KeyInput( const KeyEvent& rKeyEvt ) SAL_OVERRIDE;
        virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
        virtual void    LoseFocus() SAL_OVERRIDE;
        virtual void    RequestHelp(const HelpEvent &rEvt) SAL_OVERRIDE;
        virtual OUString GetSurroundingText() const SAL_OVERRIDE;
        virtual Selection GetSurroundingTextSelection() const SAL_OVERRIDE;

        DECL_LINK( Select, Menu* );

    public:
        SidebarTextControl( SwSidebarWin& rSidebarWin,
                           WinBits nBits,
                           SwView& rDocView,
                           SwPostItMgr& rPostItMgr );
        virtual ~SidebarTextControl();
        virtual void dispose() SAL_OVERRIDE;

        virtual void GetFocus() SAL_OVERRIDE;

        OutlinerView* GetTextView() const;

        DECL_LINK( OnlineSpellCallback, SpellCallbackInfo*);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

        virtual void Draw(OutputDevice* pDev, const Point&, const Size&, sal_uLong) SAL_OVERRIDE;
};

} } // end of namespace sw::sidebarwindows

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
