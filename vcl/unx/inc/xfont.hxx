/*************************************************************************
 *
 *  $RCSfile: xfont.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:35:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef EXTENDED_FONTSTRUCT_HXX
#define EXTENDED_FONTSTRUCT_HXX

#ifndef _XLIB_H_
#include <prex.h>
#include <X11/Xlib.h>
#include <postx.h>
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _VCL_VCLENUM_HXX
#include <vclenum.hxx>
#endif
#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif

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

    VerticalTextItem::VerticalTextItem( XFontStruct* pXFontStruct,
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

    VerticalTextItem::VerticalTextItem( XFontStruct* pXFontStruct,
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

    VerticalTextItem::~VerticalTextItem()
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
