/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xfont.hxx,v $
 * $Revision: 1.19 $
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
#ifndef EXTENDED_FONTSTRUCT_HXX
#define EXTENDED_FONTSTRUCT_HXX

#ifndef _XLIB_H_
#include <prex.h>
#include <X11/Xlib.h>
#include <postx.h>
#endif
#include <tools/ref.hxx>
#include <rtl/tencinfo.h>
#include <vcl/vclenum.hxx>
#include <vcl/sallayout.hxx>

typedef unsigned short sal_MultiByte;

class ImplFontMetricData;
class ExtendedXlfd;

struct VerticalTextItem
{
    BOOL                mbFixed;
    XFontStruct*        mpXFontStruct;
    const sal_Unicode*  mpString;
    int                 mnLength;
    int                 mnTransX;
    int                 mnTransY;
    int                 mnFixedAdvance;
    int*                mpAdvanceAry;

    VerticalTextItem( XFontStruct* pXFontStruct,
                        const sal_Unicode* pString,
                        int nLength,
                        int nTransX,
                        int nTransY,
                        int nFixedAdvance )
      : mbFixed( TRUE ),
        mpXFontStruct( pXFontStruct ),
        mpString( pString ),
        mnLength( nLength ),
        mnTransX( nTransX ),
        mnTransY( nTransY ),
        mnFixedAdvance( nFixedAdvance )
    {}

    VerticalTextItem( XFontStruct* pXFontStruct,
                        const sal_Unicode* pString,
                        int nLength,
                        int nTransX,
                        int nTransY,
                        int* pAdvanceAry )
      : mbFixed( FALSE ),
        mpXFontStruct( pXFontStruct ),
        mpString( pString ),
        mnLength( nLength ),
        mnTransX( nTransX ),
        mnTransY( nTransY ),
        mpAdvanceAry( pAdvanceAry )
    {}

    ~VerticalTextItem()
    {
        if (!mbFixed)
        {
            delete( mpAdvanceAry );
        }
    }
};

class ExtendedFontStruct : public SvRefBase
{
    private:
        Display*            mpDisplay;
        Size                maPixelSize;
        float               mfXScale;
        float               mfYScale;
        sal_Size            mnDefaultWidth;
        sal_Bool            mbVertical;
        rtl_TextEncoding    mnCachedEncoding;
        rtl_TextEncoding    mnAsciiEncoding;

        ExtendedXlfd*       mpXlfd;
        XFontStruct**       mpXFontStruct;

        // unicode range cache
        mutable sal_uInt32* mpRangeCodes;
        mutable int         mnRangeCount;

        int                 LoadEncoding( rtl_TextEncoding nEncoding );
        FontPitch           GetSpacing( rtl_TextEncoding nEncoding );
        bool                GetFontBoundingBox( XCharStruct *pCharStruct,
                                    int *pAscent, int *pDescent ) ;

        sal_Size            GetDefaultWidth();
        sal_Size            GetCharWidth8( sal_Unicode nFrom, sal_Unicode nTo,
                                    sal_Int32 *pWidthArray,
                                    rtl_TextEncoding nEncoding );
        sal_Size            GetCharWidthUTF16( sal_Unicode nFrom, sal_Unicode nTo,
                                    sal_Int32 *pWidthArray );
        sal_Size            GetCharWidth16( sal_Unicode nFrom, sal_Unicode nTo,
                                    sal_Int32 *pWidthArray, ExtendedFontStruct *pFallback );
    public:
                            ExtendedFontStruct( Display* pDisplay,
                                    const Size& rPixelSize, sal_Bool bVertical,
                                    ExtendedXlfd* pXlfd );
                            ~ExtendedFontStruct();
        bool                Match( const ExtendedXlfd *pXlfd,
                                    const Size& rPixelSize, sal_Bool bVertical ) const;
        XFontStruct*        GetFontStruct( rtl_TextEncoding nEncoding );
        XFontStruct*        GetFontStruct( sal_Unicode nChar,
                                    rtl_TextEncoding *pEncoding );
        bool                ToImplFontMetricData( ImplFontMetricData *pMetric );
        rtl_TextEncoding    GetAsciiEncoding( int *pAsciiRange = NULL ) const;
        sal_Size            GetCharWidth( sal_Unicode,
                                    sal_Int32* pPhysWidth, sal_Int32* pLogWidth );
        int                 GetFontCodeRanges( sal_uInt32* pCodePairs ) const;
        bool                HasUnicodeChar( sal_Unicode ) const;
};

// Declaration and Implementation for ExtendedFontStructRef: Add RefCounting
// to ExtendedFontStruct (it's not possible to separate decl and impl into
// a separate source file: all ref member functions are inline
SV_DECL_IMPL_REF( ExtendedFontStruct );

class X11FontLayout : public GenericSalLayout
{
public:
                    X11FontLayout( ExtendedFontStruct& );
    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

private:
    ExtendedFontStruct& mrFont;
};

#endif /* EXTENDED_FONTSTRUCT_HXX */
