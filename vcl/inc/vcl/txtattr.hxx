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

#ifndef _TXTATTR_HXX
#define _TXTATTR_HXX

#include <vcl/dllapi.h>
#include <tools/color.hxx>
#include <vcl/vclenum.hxx>
#include <tools/string.hxx>
#include <tools/debug.hxx>

class Font;

#define TEXTATTR_INVALID    0
#define TEXTATTR_FONTCOLOR  1
#define TEXTATTR_HYPERLINK  2
#define TEXTATTR_FONTWEIGHT 3

#define TEXTATTR_USER_START 1000 //start id for user defined text attributes
#define TEXTATTR_PROTECTED  4


class VCL_DLLPUBLIC TextAttrib
{
private:
    sal_uInt16                  mnWhich;

protected:
                            TextAttrib( sal_uInt16 nWhich ) { mnWhich = nWhich; }
                            TextAttrib( const TextAttrib& rAttr ) { mnWhich = rAttr.mnWhich; }

public:

    virtual                 ~TextAttrib();

    sal_uInt16              Which() const   { return mnWhich; }
    virtual void            SetFont( Font& rFont ) const = 0;
    virtual TextAttrib*     Clone() const = 0;

    virtual int             operator==( const TextAttrib& rAttr ) const = 0;
    int                     operator!=( const TextAttrib& rAttr ) const
                                { return !(*this == rAttr ); }
};



class VCL_DLLPUBLIC TextAttribFontColor : public TextAttrib
{
private:
    Color   maColor;

public:
                            TextAttribFontColor( const Color& rColor );
                            TextAttribFontColor( const TextAttribFontColor& rAttr );
                            ~TextAttribFontColor();

    const Color&            GetColor() const { return maColor; }

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;

};

class VCL_DLLPUBLIC TextAttribFontWeight : public TextAttrib
{
private:
    FontWeight  meWeight;

public:
                            TextAttribFontWeight( FontWeight eWeight );
                            TextAttribFontWeight( const TextAttribFontWeight& rAttr );
                            ~TextAttribFontWeight();

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;

    inline FontWeight getFontWeight() const { return meWeight; }
};


class TextAttribHyperLink : public TextAttrib
{
private:
    XubString   maURL;
    XubString   maDescription;
    Color       maColor;

public:
                            TextAttribHyperLink( const TextAttribHyperLink& rAttr );
                            ~TextAttribHyperLink();

    void                    SetURL( const XubString& rURL )             { maURL = rURL; }
    const XubString&            GetURL() const                              { return maURL; }

    void                    SetDescription( const XubString& rDescr )   { maDescription = rDescr; }
    const XubString&            GetDescription() const                      { return maDescription; }

    void                    SetColor( const Color& rColor )             { maColor = rColor; }
    const Color&            GetColor() const                            { return maColor; }

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};

class VCL_DLLPUBLIC TextAttribProtect : public TextAttrib
{
public:
                            TextAttribProtect();
                            TextAttribProtect( const TextAttribProtect& rAttr );
                            ~TextAttribProtect();

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;

};


class TextCharAttrib
{
private:
    TextAttrib*     mpAttr;
    sal_uInt16          mnStart;
    sal_uInt16          mnEnd;

protected:

public:

                    TextCharAttrib( const TextAttrib& rAttr, sal_uInt16 nStart, sal_uInt16 nEnd );
                    TextCharAttrib( const TextCharAttrib& rTextCharAttrib );
                    ~TextCharAttrib();

    const TextAttrib&   GetAttr() const         { return *mpAttr; }

    sal_uInt16          Which() const               { return mpAttr->Which(); }

    sal_uInt16          GetStart() const            { return mnStart; }
    sal_uInt16&         GetStart()                  { return mnStart; }

    sal_uInt16          GetEnd() const              { return mnEnd; }
    sal_uInt16&         GetEnd()                    { return mnEnd; }

    inline sal_uInt16   GetLen() const;

    inline void     MoveForward( sal_uInt16 nDiff );
    inline void     MoveBackward( sal_uInt16 nDiff );

    inline void     Expand( sal_uInt16 nDiff );
    inline void     Collaps( sal_uInt16 nDiff );

    inline sal_Bool     IsIn( sal_uInt16 nIndex );
    inline sal_Bool     IsInside( sal_uInt16 nIndex );
    inline sal_Bool     IsEmpty();

};

inline sal_uInt16 TextCharAttrib::GetLen() const
{
    DBG_ASSERT( mnEnd >= mnStart, "TextCharAttrib: nEnd < nStart!" );
    return mnEnd-mnStart;
}

inline void TextCharAttrib::MoveForward( sal_uInt16 nDiff )
{
    DBG_ASSERT( ((long)mnEnd + nDiff) <= 0xFFFF, "TextCharAttrib: MoveForward?!" );
    mnStart = mnStart + nDiff;
    mnEnd = mnEnd + nDiff;
}

inline void TextCharAttrib::MoveBackward( sal_uInt16 nDiff )
{
    DBG_ASSERT( ((long)mnStart - nDiff) >= 0, "TextCharAttrib: MoveBackward?!" );
    mnStart = mnStart - nDiff;
    mnEnd = mnEnd - nDiff;
}

inline void TextCharAttrib::Expand( sal_uInt16 nDiff )
{
    DBG_ASSERT( ( ((long)mnEnd + nDiff) <= (long)0xFFFF ), "TextCharAttrib: Expand?!" );
    mnEnd = mnEnd + nDiff;
}

inline void TextCharAttrib::Collaps( sal_uInt16 nDiff )
{
    DBG_ASSERT( (long)mnEnd - nDiff >= (long)mnStart, "TextCharAttrib: Collaps?!" );
    mnEnd = mnEnd - nDiff;
}

inline sal_Bool TextCharAttrib::IsIn( sal_uInt16 nIndex )
{
    return ( ( mnStart <= nIndex ) && ( mnEnd >= nIndex ) );
}

inline sal_Bool TextCharAttrib::IsInside( sal_uInt16 nIndex )
{
    return ( ( mnStart < nIndex ) && ( mnEnd > nIndex ) );
}

inline sal_Bool TextCharAttrib::IsEmpty()
{
    return mnStart == mnEnd;
}

#endif // _TXTATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
