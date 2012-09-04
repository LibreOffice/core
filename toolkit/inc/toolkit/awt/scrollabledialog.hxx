/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef TOOLKIT_AWT_SCROLLABLEDIALOG_HXX
#define TOOLKIT_AWT_SCROLLABLEDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/scrbar.hxx>

//........................................................................
namespace toolkit
{
    class ScrollableDialog : public Dialog
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
        ScrollableDialog( Window* pParent, WinBits nStyle = WB_STDDIALOG );
        virtual ~ScrollableDialog();
        void    SetScrollWidth( long nWidth );
        long    GetScrollWidth() { return maScrollArea.Width(); }
        void    SetScrollHeight( long nHeight );
        long    GetScrollHeight() { return maScrollArea.Height(); }
        void    SetScrollLeft( long nLeft );
        long    GetScrollLeft() { return mnScrollPos.X(); }
        void    SetScrollTop( long Top );
        long    GetScrollTop() { return mnScrollPos.Y() ; }
        Window*  getContentWindow();
        ScrollBarVisibility getScrollVisibility() { return maScrollVis; }
        void setScrollVisibility( ScrollBarVisibility rState );
        virtual void      Paint( const Rectangle& rRect );
        virtual void      Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
        DECL_LINK( ScrollBarHdl, ScrollBar* );
        DECL_LINK( ContainerScrolled, void* );
        // Window
        virtual void Resize();
    };
//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_AWT_SCROLLABLEDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
