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
#ifndef _SVX_PAGECTRL_HXX
#define _SVX_PAGECTRL_HXX

// include ---------------------------------------------------------------

#include <vcl/window.hxx>
#include "svx/svxdllapi.h"

// forward ---------------------------------------------------------------

class SvxBoxItem;
struct PageWindow_Impl;

// class SvxPageWindow ---------------------------------------------------

class SVX_DLLPUBLIC SvxPageWindow : public Window
{
    using Window::GetBorder;

private:
    Size        aWinSize;
    Size        aSize;

    long        nTop;
    long        nBottom;
    long        nLeft;
    long        nRight;
    Color       aColor;

    PageWindow_Impl* pImpl;

    long        nHdLeft;
    long        nHdRight;
    long        nHdDist;
    long        nHdHeight;
    Color       aHdColor;
    SvxBoxItem* pHdBorder;

    long        nFtLeft;
    long        nFtRight;
    long        nFtDist;
    long        nFtHeight;
    Color       aFtColor;
    SvxBoxItem* pFtBorder;

    sal_Bool        bFooter :1;
    sal_Bool        bHeader :1;
    sal_Bool        bTable  :1;
    sal_Bool        bHorz   :1;
    sal_Bool        bVert   :1;

    sal_uInt16      eUsage;

    String      aLeftText;
    String      aRightText;

protected:
    virtual void Paint( const Rectangle& rRect );

    virtual void DrawPage( const Point& rPoint,
                           const sal_Bool bSecond,
                           const sal_Bool bEnabled );

public:
    SvxPageWindow( Window* pParent, const ResId& rId );
    ~SvxPageWindow();

    void        SetWidth( long nW )                     { aSize.Width()  = nW; }
    void        SetHeight( long nH )                    { aSize.Height() = nH; }

    void        SetSize( const Size& rSz )              { aSize = rSz; }
    const Size& GetSize() const                         { return aSize; }

    void        SetTop( long nNew )                     { nTop = nNew; }
    void        SetBottom( long nNew )                  { nBottom = nNew; }
    void        SetLeft( long nNew )                    { nLeft = nNew; }
    void        SetRight( long nNew )                   { nRight = nNew; }
    void        SetColor( const Color& rNew )           { aColor = rNew; }
    void        SetBorder( const SvxBoxItem& rNew );
    void        SetBitmap( Bitmap* pBmp );

    long        GetTop() const                          { return nTop;   }
    long        GetBottom() const                       { return nBottom;}
    long        GetLeft() const                         { return nLeft;  }
    long        GetRight() const                        { return nRight; }

    const Color&        GetColor() const                { return aColor; }
    const SvxBoxItem&   GetBorder() const;

    void        SetHdLeft( long nNew )                  { nHdLeft = nNew; }
    void        SetHdRight( long nNew )                 { nHdRight = nNew; }
    void        SetHdDist( long nNew )                  { nHdDist = nNew; }
    void        SetHdHeight( long nNew )                { nHdHeight = nNew; }
    void        SetHdColor( const Color& aNew )         { aHdColor = aNew; }
    void        SetHdBorder( const SvxBoxItem& rNew );

    long        GetHdLeft() const                       { return nHdLeft;  }
    long        GetHdRight() const                      { return nHdRight; }
    long        GetHdDist() const                       { return nHdDist;  }
    long        GetHdHeight() const                     { return nHdHeight;}

    const Color&        GetHdColor() const              { return aHdColor; }
    const SvxBoxItem&   GetHdBorder() const             { return *pHdBorder; }

    void        SetFtLeft( long nNew )                  { nFtLeft = nNew; }
    void        SetFtRight( long nNew )                 { nFtRight = nNew; }
    void        SetFtDist( long nNew )                  { nFtDist = nNew; }
    void        SetFtHeight( long nNew )                { nFtHeight = nNew; }
    void        SetFtColor( const Color& aNew )         { aFtColor = aNew; }
    void        SetFtBorder( const SvxBoxItem& rNew );

    long        GetFtLeft() const                       { return nFtLeft;  }
    long        GetFtRight() const                      { return nFtRight; }
    long        GetFtDist() const                       { return nFtDist;  }
    long        GetFtHeight() const                     { return nFtHeight;}

    const Color&        GetFtColor() const              { return aFtColor; }
    const SvxBoxItem&   GetFtBorder() const             { return *pFtBorder; }

    void        SetUsage( sal_uInt16 eU )                   { eUsage = eU;  }
    sal_uInt16      GetUsage() const                        { return eUsage;}

    void        SetHeader( sal_Bool bNew )                  { bHeader = bNew; }
    sal_Bool        GetHeader() const                       { return bHeader;}
    void        SetFooter( sal_Bool bNew )                  { bFooter = bNew; }
    sal_Bool        GetFooter() const                       { return bFooter;}

    void        SetTable( sal_Bool bNew )                   { bTable = bNew; }
    sal_Bool        GetTable() const                        { return bTable;}
    void        SetHorz( sal_Bool bNew )                    { bHorz = bNew; }
    sal_Bool        GetHorz() const                         { return bHorz;}
    void        SetVert( sal_Bool bNew )                    { bVert = bNew; }
    sal_Bool        GetVert() const                         { return bVert;}

    void        EnableFrameDirection(sal_Bool bEnable);
    //uses enum SvxFrameDirection
    void        SetFrameDirection(sal_Int32 nFrameDirection);

    void        ResetBackground();
};

#endif // #ifndef _SVX_PAGECTRL_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
