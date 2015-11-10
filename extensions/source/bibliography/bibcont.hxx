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

#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBCONT_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_BIBCONT_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/splitwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include "bibshortcuthandler.hxx"

#include "bibmod.hxx"

#define TOP_WINDOW                          1
#define BOTTOM_WINDOW                       2

class BibWindowContainer : public BibWindow     //Window
{
    private:
        // !BibShortCutHandler is also always a Window!
        BibShortCutHandler*     pChild;

    protected:
        virtual void            Resize() override;

    public:
        BibWindowContainer( vcl::Window* pParent, BibShortCutHandler* pChild, WinBits nStyle = WB_3DLOOK);
        virtual ~BibWindowContainer();
        virtual void            dispose() override;

        inline vcl::Window*     GetChild();

        virtual void            GetFocus() override;

        virtual bool            HandleShortCutKey( const KeyEvent& rKeyEvent ) override; // returns true, if key was handled

        using Window::GetChild;
};

inline vcl::Window* BibWindowContainer::GetChild()
{
    return pChild ? pChild->GetWindow() : nullptr;
}


class BibBookContainer: public BibSplitWindow
{
    private:

        css::uno::Reference< css::frame::XFrame >             xTopFrameRef;
        css::uno::Reference< css::frame::XFrame >             xBottomFrameRef;

        css::uno::Reference< css::awt::XWindow >              xTopPeerRef;
        css::uno::Reference< css::awt::XWindow >              xBottomPeerRef;

        VclPtr<BibWindowContainer>     pTopWin;
        VclPtr<BibWindowContainer>     pBottomWin;
        HdlBibModul             pBibMod;
        Idle                    aIdle;

        DECL_LINK_TYPED( SplitHdl, Idle*, void );

    protected:

        virtual void            Split() override;

        virtual bool            PreNotify( NotifyEvent& rNEvt ) override;

    public:

        BibBookContainer(vcl::Window* pParent, WinBits nStyle = WB_3DLOOK );
        virtual ~BibBookContainer();
        virtual void dispose() override;

        // !BibShortCutHandler is also always a Window!
        void                    createTopFrame( BibShortCutHandler* pWin );

        void                    createBottomFrame( BibShortCutHandler* pWin );

        virtual void            GetFocus() override;

        virtual bool        HandleShortCutKey( const KeyEvent& rKeyEvent ) override; // returns true, if key was handled
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
