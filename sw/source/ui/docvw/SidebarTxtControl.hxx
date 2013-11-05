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

#ifndef INCLUDED_SW_SOURCE_UI_DOCVW_SIDEBARTXTCONTROL_HXX
#define INCLUDED_SW_SOURCE_UI_DOCVW_SIDEBARTXTCONTROL_HXX

#include <vcl/ctrl.hxx>

class OutlinerView;
class SwView;
class SwPostItMgr;
struct SpellCallbackInfo;

namespace sw { namespace sidebarwindows {

class SwSidebarWin;

class SidebarTxtControl : public Control
{
    private:
        SwSidebarWin& mrSidebarWin;
        SwView& mrDocView;
        SwPostItMgr& mrPostItMgr;

    protected:
        virtual void    Paint( const Rectangle& rRect);
        virtual void    KeyInput( const KeyEvent& rKeyEvt );
        virtual void    MouseMove( const MouseEvent& rMEvt );
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
        virtual void    Command( const CommandEvent& rCEvt );
        virtual void    LoseFocus();
        virtual void    RequestHelp(const HelpEvent &rEvt);
        virtual OUString GetSurroundingText() const;
        virtual Selection GetSurroundingTextSelection() const;

        DECL_LINK( Select, Menu* );

    public:
        SidebarTxtControl( SwSidebarWin& rSidebarWin,
                           WinBits nBits,
                           SwView& rDocView,
                           SwPostItMgr& rPostItMgr );
        ~SidebarTxtControl();

        virtual void GetFocus();

        OutlinerView* GetTextView() const;

        DECL_LINK( OnlineSpellCallback, SpellCallbackInfo*);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

} } // end of namespace sw::sidebarwindows

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
