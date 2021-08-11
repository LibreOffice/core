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

#pragma once

#include <editeng/eeitem.hxx>
#include <svl/poolitem.hxx>
#include <optional>
#include <tools/color.hxx>
#include <tools/debug.hxx>

class SvxFont;
class SvxFontItem;
class SvxWeightItem;
class SvxPostureItem;
class SvxShadowedItem;
class SvxEscapementItem;
class SvxContourItem;
class SvxCrossedOutItem;
class SvxUnderlineItem;
class SvxOverlineItem;
class SvxFontHeightItem;
class SvxCharScaleWidthItem;
class SvxColorItem;
class SvxAutoKernItem;
class SvxKerningItem;
class SvxWordLineModeItem;
class SvxFieldItem;
class SvxLanguageItem;
class SvxEmphasisMarkItem;
class SvxCharReliefItem;
class SfxVoidItem;
class OutputDevice;
class SvxCaseMapItem;
class SfxGrabBagItem;

#define CH_FEATURE_OLD  (sal_uInt8)         0xFF
#define CH_FEATURE      u'\x0001'

// DEF_METRIC: For my pool, the DefMetric should always appear when
// GetMetric (nWhich)!
// => To determine the DefMetric simply use GetMetric(0)
#define DEF_METRIC  0


enum class EditCharAttribType : sal_uInt8
{
    Font, Weight, Italic, Shadow, Escapement, Outline, Strikeout, CaseMap, Underline,
    Overline, EmphasisMark, Relief, FontHeight, FontWidth, Color,
    BackgroundColor, Language, Tab, LineBreak, Field,
    PairKerning, Kerning, WordLineMode, GrabBag, Xml
};


// bFeature: Attribute must not expand/shrink, length is always 1
// bEdge: Attribute will not expand, if you want to expand just on the edge
class EditCharAttrib final
{
    const SfxPoolItem*  pItem;
    sal_Int32           nStart;
    sal_Int32           nEnd;
    EditCharAttribType  mnType;
    bool                bFeature    :1;
    bool                bEdge       :1;
    // following fields only valid when mnType == EditCharAttribType::Field
    OUString            aFieldValue;
    std::optional<Color>  mxTxtColor;
    std::optional<Color>  mxFldColor;


    EditCharAttrib( const SfxPoolItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd, EditCharAttribType nType, bool bFeature = false );
public:

    void                dumpAsXml(xmlTextWriterPtr pWriter) const;

    sal_uInt16          Which() const   { return pItem->Which(); }
    const SfxPoolItem*  GetItem() const { return pItem; }

    sal_Int32&          GetStart()                  { return nStart; }
    sal_Int32&          GetEnd()                    { return nEnd; }

    sal_Int32           GetStart() const            { return nStart; }
    sal_Int32           GetEnd() const              { return nEnd; }

    inline sal_Int32    GetLen() const;

    inline void     MoveForward( sal_Int32 nDiff );
    inline void     MoveBackward( sal_Int32 nDiff );

    inline void     Expand( sal_Int32 nDiff );
    inline void     Collaps( sal_Int32 nDiff );

    void    SetFont( SvxFont& rFont, OutputDevice* pOutDev );

    bool    IsIn( sal_Int32 nIndex ) const
                { return ( ( nStart <= nIndex ) && ( nEnd >= nIndex ) ); }
    bool    IsInside( sal_Int32 nIndex ) const
                { return ( ( nStart < nIndex ) && ( nEnd > nIndex ) ); }
    bool        IsEmpty() const
                { return nStart == nEnd; }

    bool    IsFeature() const   { return bFeature; }
    void    SetFeature( bool b) { bFeature = b; }

    bool    IsEdge() const      { return bEdge; }
    void    SetEdge( bool b )   { bEdge = b; }

    bool operator == ( const EditCharAttrib& rAttr ) const;
    bool operator != ( const EditCharAttrib& rAttr ) const { return !(operator == ( rAttr ) ); }

    std::optional<Color>&   GetFieldTextColor() { assert(mnType == EditCharAttribType::Field); return mxTxtColor; }
    std::optional<Color>&   GetFieldColor()     { assert(mnType == EditCharAttribType::Field); return mxFldColor; }
    const OUString& GetFieldValue() const { assert(mnType == EditCharAttribType::Field); return aFieldValue;}
    void SetFieldValue(const OUString& rVal);
    void ResetField();

    static EditCharAttrib MakeFont( const SvxFontItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeWeight( const SvxWeightItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeItalic( const SvxPostureItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeShadow( const SvxShadowedItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeEscapement( const SvxEscapementItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeOutline( const SvxContourItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeStrikeout( const SvxCrossedOutItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeCaseMap( const SvxCaseMapItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeUnderline( const SvxUnderlineItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeOverline( const SvxOverlineItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeEmphasisMark( const SvxEmphasisMarkItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeRelief( const SvxCharReliefItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeFontHeight( const SvxFontHeightItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeFontWidth( const SvxCharScaleWidthItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeColor( const SvxColorItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeBackgroundColor(const SvxColorItem& rAttr,
                                  sal_Int32 nStart,
                                  sal_Int32 nEnd );
    static EditCharAttrib MakeLanguage( const SvxLanguageItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeTab( const SfxVoidItem& rAttr, sal_Int32 nPos );
    static EditCharAttrib MakeLineBreak( const SfxVoidItem& rAttr, sal_Int32 nPos );
    static EditCharAttrib MakeField( const SvxFieldItem& rAttr, sal_Int32 nPos );
    static EditCharAttrib MakeField( const EditCharAttrib& rAttr );
    static EditCharAttrib MakePairKerning( const SvxAutoKernItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeKerning( const SvxKerningItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeWordLineMode( const SvxWordLineModeItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeGrabBag( const SfxGrabBagItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    static EditCharAttrib MakeXml( const SfxPoolItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
};

inline sal_Int32 EditCharAttrib::GetLen() const
{
    DBG_ASSERT( nEnd >= nStart, "EditCharAttrib: nEnd < nStart!" );
    return nEnd-nStart;
}

inline void EditCharAttrib::MoveForward( sal_Int32 nDiff )
{
    DBG_ASSERT( SAL_MAX_INT32 - nDiff > nEnd, "EditCharAttrib: MoveForward?!" );
    nStart = nStart + nDiff;
    nEnd = nEnd + nDiff;
}

inline void EditCharAttrib::MoveBackward( sal_Int32 nDiff )
{
    DBG_ASSERT( (nStart - nDiff) >= 0, "EditCharAttrib: MoveBackward?!" );
    nStart = nStart - nDiff;
    nEnd = nEnd - nDiff;
}

inline void EditCharAttrib::Expand( sal_Int32 nDiff )
{
    DBG_ASSERT( SAL_MAX_INT32 - nDiff > nEnd, "EditCharAttrib: Expand?!" );
    DBG_ASSERT( !bFeature, "Please do not expand any features!" );
    nEnd = nEnd + nDiff;
}

inline void EditCharAttrib::Collaps( sal_Int32 nDiff )
{
    DBG_ASSERT( nEnd - nDiff >= nStart, "EditCharAttrib: Collaps?!" );
    DBG_ASSERT( !bFeature, "Please do not shrink any Features!" );
    nEnd = nEnd - nDiff;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
