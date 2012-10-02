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

#ifndef TOOLKIT_AWT_SCROLLABLEDIALOG_HXX
#define TOOLKIT_AWT_SCROLLABLEDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/scrbar.hxx>

//........................................................................
namespace toolkit
{
    class ScrollableInterface
    {
    public:
        virtual ~ScrollableInterface() {}
        virtual void    SetScrollWidth( long nWidth ) = 0;
        virtual long    GetScrollWidth() = 0;
        virtual void    SetScrollHeight( long nHeight ) = 0;
        virtual long    GetScrollHeight() = 0;
        virtual void    SetScrollLeft( long nLeft ) = 0;
        virtual long    GetScrollLeft() = 0;
        virtual void    SetScrollTop( long Top ) = 0;
        virtual long    GetScrollTop() = 0;
    };

  template < class T >
    class ScrollableWrapper : public T, public ScrollableInterface
    {
        ScrollBar          maHScrollBar;
        ScrollBar          maVScrollBar;
        Size               maScrollArea;
        bool               mbHasHoriBar;
        bool               mbHasVertBar;
        Point              mnScrollPos;
        long               mnScrWidth;

    public:
        enum ScrollBarVisibility { None, Vert, Hori, Both };
    private:
        ScrollBarVisibility maScrollVis;
        void    lcl_Scroll( long nX, long nY );
    public:
        ScrollableWrapper( Window* pParent, WinBits nStyle = WB_STDDIALOG );
        virtual ~ScrollableWrapper();
        virtual void    SetScrollWidth( long nWidth );
        virtual long    GetScrollWidth() { return maScrollArea.Width(); }
        virtual void    SetScrollHeight( long nHeight );
        virtual long    GetScrollHeight() { return maScrollArea.Height(); }
        virtual void    SetScrollLeft( long nLeft );
        virtual long    GetScrollLeft() { return mnScrollPos.X(); }
        virtual void    SetScrollTop( long Top );
        virtual long    GetScrollTop() { return mnScrollPos.Y() ; }

        void setScrollVisibility( ScrollBarVisibility rState );
        DECL_LINK( ScrollBarHdl, ScrollBar* );
        virtual void ResetScrollBars();
        // Window
        virtual void Resize();
    };
//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_AWT_SCROLLABLEDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
