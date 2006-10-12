/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtattr.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:04:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _TXTATTR_HXX
#define _TXTATTR_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _VCL_VCLENUM_HXX
#include <vcl/vclenum.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

class Font;

#define TEXTATTR_INVALID    0
#define TEXTATTR_FONTCOLOR  1
#define TEXTATTR_HYPERLINK  2
#define TEXTATTR_FONTWEIGHT 3

#define TEXTATTR_USER_START 1000 //start id for user defined text attributes
#define TEXTATTR_PROTECTED  4


class SVT_DLLPUBLIC TextAttrib
{
private:
    USHORT                  mnWhich;

protected:
                            TextAttrib( USHORT nWhich ) { mnWhich = nWhich; }
                            TextAttrib( const TextAttrib& rAttr ) { mnWhich = rAttr.mnWhich; }

public:

    virtual                 ~TextAttrib();

    USHORT                  Which() const   { return mnWhich; }

    virtual void            SetFont( Font& rFont ) const = 0;
    virtual TextAttrib*     Clone() const = 0;
    virtual int             operator==( const TextAttrib& rAttr ) const = 0;
    int                     operator!=( const TextAttrib& rAttr ) const
                                { return !(*this == rAttr ); }
};



class SVT_DLLPUBLIC TextAttribFontColor : public TextAttrib
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

class SVT_DLLPUBLIC TextAttribFontWeight : public TextAttrib
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
                            TextAttribHyperLink( const XubString& rURL );
                            TextAttribHyperLink( const XubString& rURL, const XubString& rDescription );
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

class SVT_DLLPUBLIC TextAttribProtect : public TextAttrib
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
    USHORT          mnStart;
    USHORT          mnEnd;

protected:

public:

                    TextCharAttrib( const TextAttrib& rAttr, USHORT nStart, USHORT nEnd );
                    TextCharAttrib( const TextCharAttrib& rTextCharAttrib );
                    ~TextCharAttrib();

    const TextAttrib&   GetAttr() const         { return *mpAttr; }

    USHORT          Which() const               { return mpAttr->Which(); }

    USHORT          GetStart() const            { return mnStart; }
    USHORT&         GetStart()                  { return mnStart; }

    USHORT          GetEnd() const              { return mnEnd; }
    USHORT&         GetEnd()                    { return mnEnd; }

    inline USHORT   GetLen() const;

    inline void     MoveForward( USHORT nDiff );
    inline void     MoveBackward( USHORT nDiff );

    inline void     Expand( USHORT nDiff );
    inline void     Collaps( USHORT nDiff );

    inline BOOL     IsIn( USHORT nIndex );
    inline BOOL     IsInside( USHORT nIndex );
    inline BOOL     IsEmpty();

};

inline USHORT TextCharAttrib::GetLen() const
{
    DBG_ASSERT( mnEnd >= mnStart, "TextCharAttrib: nEnd < nStart!" );
    return mnEnd-mnStart;
}

inline void TextCharAttrib::MoveForward( USHORT nDiff )
{
    DBG_ASSERT( ((long)mnEnd + nDiff) <= 0xFFFF, "TextCharAttrib: MoveForward?!" );
    mnStart = mnStart + nDiff;
    mnEnd = mnEnd + nDiff;
}

inline void TextCharAttrib::MoveBackward( USHORT nDiff )
{
    DBG_ASSERT( ((long)mnStart - nDiff) >= 0, "TextCharAttrib: MoveBackward?!" );
    mnStart = mnStart - nDiff;
    mnEnd = mnEnd - nDiff;
}

inline void TextCharAttrib::Expand( USHORT nDiff )
{
    DBG_ASSERT( ( ((long)mnEnd + nDiff) <= (long)0xFFFF ), "TextCharAttrib: Expand?!" );
    mnEnd = mnEnd + nDiff;
}

inline void TextCharAttrib::Collaps( USHORT nDiff )
{
    DBG_ASSERT( (long)mnEnd - nDiff >= (long)mnStart, "TextCharAttrib: Collaps?!" );
    mnEnd = mnEnd - nDiff;
}

inline BOOL TextCharAttrib::IsIn( USHORT nIndex )
{
    return ( ( mnStart <= nIndex ) && ( mnEnd >= nIndex ) );
}

inline BOOL TextCharAttrib::IsInside( USHORT nIndex )
{
    return ( ( mnStart < nIndex ) && ( mnEnd > nIndex ) );
}

inline BOOL TextCharAttrib::IsEmpty()
{
    return mnStart == mnEnd;
}

#endif // _TXTATTR_HXX
