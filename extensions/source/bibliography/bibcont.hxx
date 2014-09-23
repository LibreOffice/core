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
        virtual void            Resize() SAL_OVERRIDE;

    public:
        BibWindowContainer( vcl::Window* pParent, BibShortCutHandler* pChild, WinBits nStyle = WB_3DLOOK);
        virtual ~BibWindowContainer();

        inline vcl::Window*          GetChild();

        virtual void            GetFocus() SAL_OVERRIDE;

        virtual bool            HandleShortCutKey( const KeyEvent& rKeyEvent ) SAL_OVERRIDE; // returns true, if key was handled

        using Window::GetChild;
};

inline vcl::Window* BibWindowContainer::GetChild()
{
    return pChild? pChild->GetWindow() : NULL;
}


class BibBookContainer: public BibSplitWindow
{
    private:

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             xTopFrameRef;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             xBottomFrameRef;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              xTopPeerRef;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              xBottomPeerRef;

        BibWindowContainer*     pTopWin;
        BibWindowContainer*     pBottomWin;
        HdlBibModul             pBibMod;
        Timer                   aTimer;

        DECL_LINK( SplitHdl, Timer*);

    protected:

        virtual void            Split() SAL_OVERRIDE;

        virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    public:

        BibBookContainer(vcl::Window* pParent, WinBits nStyle = WB_3DLOOK );
        virtual ~BibBookContainer();

        inline BibWindow*       GetTopWin() {return pTopWin;}
        inline BibWindow*       GetBottomWin() {return pBottomWin;}

        // !BibShortCutHandler is also always a Window!
        void                    createTopFrame( BibShortCutHandler* pWin );

        void                    createBottomFrame( BibShortCutHandler* pWin );

        virtual void            GetFocus() SAL_OVERRIDE;

        virtual bool        HandleShortCutKey( const KeyEvent& rKeyEvent ) SAL_OVERRIDE; // returns true, if key was handled
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
