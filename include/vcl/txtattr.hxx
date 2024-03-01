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

#include <config_options.h>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <tools/fontenum.hxx>
#include <vcl/dllapi.h>
#include <memory>

namespace vcl { class Font; }

#define TEXTATTR_FONTCOLOR  1
#define TEXTATTR_FONTWEIGHT 3

#define TEXTATTR_USER_START 1000 //start id for user defined text attributes
#define TEXTATTR_PROTECTED  4


class VCL_DLLPUBLIC TextAttrib
{
private:
    sal_uInt16 const        mnWhich;

protected:
                            TextAttrib( sal_uInt16 nWhich ) : mnWhich(nWhich) {}
                            TextAttrib( const TextAttrib& ) = default;

public:

    virtual                 ~TextAttrib();

    sal_uInt16              Which() const   { return mnWhich; }
    virtual void            SetFont( vcl::Font& rFont ) const = 0;
    virtual std::unique_ptr<TextAttrib> Clone() const = 0;

    virtual bool            operator==( const TextAttrib& rAttr ) const = 0;
    bool                    operator!=( const TextAttrib& rAttr ) const
                                { return !(*this == rAttr ); }
};


class VCL_DLLPUBLIC TextAttribFontColor final : public TextAttrib
{
private:
    Color   maColor;

public:
                            TextAttribFontColor( const Color& rColor );

    const Color&            GetColor() const { return maColor; }

    virtual void            SetFont( vcl::Font& rFont ) const override;
    virtual std::unique_ptr<TextAttrib> Clone() const override;
    virtual bool            operator==( const TextAttrib& rAttr ) const override;

};

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) TextAttribFontWeight final : public TextAttrib
{
private:
    FontWeight  meWeight;

public:
                            TextAttribFontWeight( FontWeight eWeight );

    virtual void            SetFont( vcl::Font& rFont ) const override;
    virtual std::unique_ptr<TextAttrib> Clone() const override;
    virtual bool            operator==( const TextAttrib& rAttr ) const override;

    FontWeight getFontWeight() const { return meWeight; }
};

class TextAttribProtect final : public TextAttrib
{
public:
                            TextAttribProtect();

    virtual void            SetFont( vcl::Font& rFont ) const override;
    virtual std::unique_ptr<TextAttrib> Clone() const override;
    virtual bool            operator==( const TextAttrib& rAttr ) const override;

};


class TextCharAttrib
{
private:
    std::unique_ptr<TextAttrib>
                    mpAttr;
    sal_Int32       mnStart;
    sal_Int32       mnEnd;

public:
                    TextCharAttrib( const TextAttrib& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
                    TextCharAttrib( const TextCharAttrib& rTextCharAttrib );

    const TextAttrib&   GetAttr() const         { return *mpAttr; }

    sal_uInt16          Which() const               { return mpAttr->Which(); }

    sal_Int32           GetStart() const            { return mnStart; }
    void                SetStart(sal_Int32 n)       { mnStart = n; }

    sal_Int32           GetEnd() const              { return mnEnd; }
    void                SetEnd(sal_Int32 n)         { mnEnd = n; }

    inline sal_Int32    GetLen() const;

    inline void     MoveForward( sal_Int32 nDiff );
    inline void     MoveBackward( sal_Int32 nDiff );

    inline void     Expand( sal_Int32 nDiff );
    inline void     Collaps( sal_Int32 nDiff );

    inline bool     IsIn( sal_Int32 nIndex ) const;
    inline bool     IsInside( sal_Int32 nIndex ) const;
    inline bool     IsEmpty() const;

};

inline sal_Int32 TextCharAttrib::GetLen() const
{
    DBG_ASSERT( mnEnd >= mnStart, "TextCharAttrib: nEnd < nStart!" );
    return mnEnd-mnStart;
}

inline void TextCharAttrib::MoveForward( sal_Int32 nDiff )
{
    DBG_ASSERT( nDiff <= SAL_MAX_INT32-mnEnd, "TextCharAttrib: MoveForward?!" );
    mnStart = mnStart + nDiff;
    mnEnd = mnEnd + nDiff;
}

inline void TextCharAttrib::MoveBackward( sal_Int32 nDiff )
{
    DBG_ASSERT( mnStart >= nDiff, "TextCharAttrib: MoveBackward?!" );
    mnStart = mnStart - nDiff;
    mnEnd = mnEnd - nDiff;
}

inline void TextCharAttrib::Expand( sal_Int32 nDiff )
{
    DBG_ASSERT( nDiff <= SAL_MAX_INT32-mnEnd, "TextCharAttrib: Expand?!" );
    mnEnd = mnEnd + nDiff;
}

inline void TextCharAttrib::Collaps( sal_Int32 nDiff )
{
    DBG_ASSERT( mnEnd-mnStart >= nDiff, "TextCharAttrib: Collaps?!" );
    mnEnd = mnEnd - nDiff;
}

inline bool TextCharAttrib::IsIn( sal_Int32 nIndex ) const
{
    return ( ( mnStart <= nIndex ) && ( mnEnd >= nIndex ) );
}

inline bool TextCharAttrib::IsInside( sal_Int32 nIndex ) const
{
    return ( ( mnStart < nIndex ) && ( mnEnd > nIndex ) );
}

inline bool TextCharAttrib::IsEmpty() const
{
    return mnStart == mnEnd;
}

#endif // INCLUDED_VCL_TXTATTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
