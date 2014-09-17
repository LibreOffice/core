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

#ifndef INCLUDED_VCL_TXTATTR_HXX
#define INCLUDED_VCL_TXTATTR_HXX

#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/dllapi.h>

namespace vcl { class Font; }

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
    virtual void            SetFont( vcl::Font& rFont ) const = 0;
    virtual TextAttrib*     Clone() const = 0;

    virtual bool            operator==( const TextAttrib& rAttr ) const = 0;
    bool                    operator!=( const TextAttrib& rAttr ) const
                                { return !(*this == rAttr ); }
};



class VCL_DLLPUBLIC TextAttribFontColor : public TextAttrib
{
private:
    Color   maColor;

public:
                            TextAttribFontColor( const Color& rColor );
                            TextAttribFontColor( const TextAttribFontColor& rAttr );
                            virtual ~TextAttribFontColor();

    const Color&            GetColor() const { return maColor; }

    virtual void            SetFont( vcl::Font& rFont ) const SAL_OVERRIDE;
    virtual TextAttrib*     Clone() const SAL_OVERRIDE;
    virtual bool            operator==( const TextAttrib& rAttr ) const SAL_OVERRIDE;

};

class VCL_DLLPUBLIC TextAttribFontWeight : public TextAttrib
{
private:
    FontWeight  meWeight;

public:
                            TextAttribFontWeight( FontWeight eWeight );
                            TextAttribFontWeight( const TextAttribFontWeight& rAttr );
                            virtual ~TextAttribFontWeight();

    virtual void            SetFont( vcl::Font& rFont ) const SAL_OVERRIDE;
    virtual TextAttrib*     Clone() const SAL_OVERRIDE;
    virtual bool            operator==( const TextAttrib& rAttr ) const SAL_OVERRIDE;

    inline FontWeight getFontWeight() const { return meWeight; }
};


class TextAttribHyperLink : public TextAttrib
{
private:
    OUString    maURL;
    OUString    maDescription;
    Color       maColor;

public:
                            TextAttribHyperLink( const TextAttribHyperLink& rAttr );
                            virtual ~TextAttribHyperLink();

    void                    SetURL( const OUString& rURL )             { maURL = rURL; }
    const OUString&         GetURL() const                              { return maURL; }

    void                    SetDescription( const OUString& rDescr )   { maDescription = rDescr; }
    const OUString&         GetDescription() const                      { return maDescription; }

    void                    SetColor( const Color& rColor )             { maColor = rColor; }
    const Color&            GetColor() const                            { return maColor; }

    virtual void            SetFont( vcl::Font& rFont ) const SAL_OVERRIDE;
    virtual TextAttrib*     Clone() const SAL_OVERRIDE;
    virtual bool            operator==( const TextAttrib& rAttr ) const SAL_OVERRIDE;
};

class VCL_DLLPUBLIC TextAttribProtect : public TextAttrib
{
public:
                            TextAttribProtect();
                            TextAttribProtect( const TextAttribProtect& rAttr );
                            virtual ~TextAttribProtect();

    virtual void            SetFont( vcl::Font& rFont ) const SAL_OVERRIDE;
    virtual TextAttrib*     Clone() const SAL_OVERRIDE;
    virtual bool            operator==( const TextAttrib& rAttr ) const SAL_OVERRIDE;

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

    inline bool     IsIn( sal_uInt16 nIndex );
    inline bool     IsInside( sal_uInt16 nIndex );
    inline bool     IsEmpty() const;

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

inline bool TextCharAttrib::IsIn( sal_uInt16 nIndex )
{
    return ( ( mnStart <= nIndex ) && ( mnEnd >= nIndex ) );
}

inline bool TextCharAttrib::IsInside( sal_uInt16 nIndex )
{
    return ( ( mnStart < nIndex ) && ( mnEnd > nIndex ) );
}

inline bool TextCharAttrib::IsEmpty() const
{
    return mnStart == mnEnd;
}

#endif // INCLUDED_VCL_TXTATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
