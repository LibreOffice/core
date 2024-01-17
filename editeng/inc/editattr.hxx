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
#include <tools/fontenum.hxx>
#include <svl/itemset.hxx>

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
#define CH_SOFTHYPHEN   u'\x00AD'

// DEF_METRIC: For my pool, the DefMetric should always appear when
// GetMetric (nWhich)!
// => To determine the DefMetric simply use GetMetric(0)
#define DEF_METRIC  0



// bFeature: Attribute must not expand/shrink, length is always 1
// bEdge: Attribute will not expand, if you want to expand just on the edge
class EditCharAttrib
{
    SfxPoolItemHolder   maItemHolder;

    sal_Int32               nStart;
    sal_Int32               nEnd;
    bool                bFeature    :1;
    bool                bEdge       :1;

public:
    EditCharAttrib(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);
    virtual ~EditCharAttrib();

    EditCharAttrib(const EditCharAttrib&) = delete;
    EditCharAttrib& operator=(const EditCharAttrib&) = delete;

    void                dumpAsXml(xmlTextWriterPtr pWriter) const;

    const SfxPoolItemHolder& GetHolder() const {  return maItemHolder; }
    const SfxPoolItem* GetItem() const { return GetHolder().getItem(); }
    sal_uInt16 Which() const { if(GetItem()) return GetItem()->Which(); return 0; }

    sal_Int32&          GetStart()                  { return nStart; }
    sal_Int32&          GetEnd()                    { return nEnd; }

    sal_Int32           GetStart() const            { return nStart; }
    sal_Int32           GetEnd() const              { return nEnd; }

    inline sal_Int32    GetLen() const;

    inline void     MoveForward( sal_Int32 nDiff );
    inline void     MoveBackward( sal_Int32 nDiff );

    inline void     Expand( sal_Int32 nDiff );
    inline void     Collaps( sal_Int32 nDiff );

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev );

    bool    IsIn( sal_Int32 nIndex ) const
                { return ( ( nStart <= nIndex ) && ( nEnd >= nIndex ) ); }
    bool    IsInLeftClosedRightOpen( sal_Int32 nIndex ) const
                { return ( ( nStart <= nIndex ) && ( nEnd > nIndex ) ); }
    bool    IsInside( sal_Int32 nIndex ) const
                { return ( ( nStart < nIndex ) && ( nEnd > nIndex ) ); }
    bool        IsEmpty() const
                { return nStart == nEnd; }

    bool    IsFeature() const   { return bFeature; }
    void    SetFeature( bool b) { bFeature = b; }

    bool    IsEdge() const      { return bEdge; }
    void    SetEdge( bool b )   { bEdge = b; }
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



class EditCharAttribFont final : public EditCharAttrib
{
public:
    EditCharAttribFont(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribWeight final : public EditCharAttrib
{
public:
    EditCharAttribWeight(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};


class EditCharAttribItalic final : public EditCharAttrib
{
public:
    EditCharAttribItalic(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribShadow final : public EditCharAttrib
{
public:
    EditCharAttribShadow(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribEscapement final : public EditCharAttrib
{
public:
    EditCharAttribEscapement(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribOutline final : public EditCharAttrib
{
public:
    EditCharAttribOutline(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribStrikeout final : public EditCharAttrib
{
public:
    EditCharAttribStrikeout(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribCaseMap final : public EditCharAttrib
{
public:
    EditCharAttribCaseMap(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribUnderline final : public EditCharAttrib
{
public:
    EditCharAttribUnderline(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribOverline final : public EditCharAttrib
{
public:
    EditCharAttribOverline(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribEmphasisMark final : public EditCharAttrib
{
public:
    EditCharAttribEmphasisMark(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribRelief final : public EditCharAttrib
{
public:
    EditCharAttribRelief(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribFontHeight final : public EditCharAttrib
{
public:
    EditCharAttribFontHeight(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribFontWidth final : public EditCharAttrib
{
public:
    EditCharAttribFontWidth(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribColor final : public EditCharAttrib
{
public:
    EditCharAttribColor(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};


class EditCharAttribBackgroundColor final : public EditCharAttrib
{
public:
    EditCharAttribBackgroundColor(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);
    virtual void    SetFont(SvxFont& rFont, OutputDevice* pOutDev) override;
};



class EditCharAttribLanguage final : public EditCharAttrib
{
public:
    EditCharAttribLanguage(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribTab final : public EditCharAttrib
{
public:
    EditCharAttribTab(SfxItemPool&, const SfxPoolItem&, sal_Int32 nPos);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribLineBreak final : public EditCharAttrib
{
public:
    EditCharAttribLineBreak(SfxItemPool&, const SfxPoolItem&, sal_Int32 nPos);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribField final : public EditCharAttrib
{
    OUString   aFieldValue;
    std::optional<Color>  mxTxtColor;
    std::optional<Color>  mxFldColor;
    std::optional<FontLineStyle> mxFldLineStyle;

    EditCharAttribField& operator = ( const EditCharAttribField& rAttr ) = delete;

public:
    EditCharAttribField(SfxItemPool&, const SfxPoolItem&, sal_Int32 nPos);
    EditCharAttribField( const EditCharAttribField& rAttr );
    virtual ~EditCharAttribField() override;

    bool operator == ( const EditCharAttribField& rAttr ) const;
    bool operator != ( const EditCharAttribField& rAttr ) const
                                    { return !(operator == ( rAttr ) ); }

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
    std::optional<Color>&   GetTextColor()      { return mxTxtColor; }
    std::optional<Color>&   GetFieldColor()     { return mxFldColor; }
    std::optional<FontLineStyle>& GetFldLineStyle() { return mxFldLineStyle; }

    const OUString& GetFieldValue() const { return aFieldValue;}
    void SetFieldValue(const OUString& rVal);

    void Reset();
};



class EditCharAttribPairKerning final : public EditCharAttrib
{
public:
    EditCharAttribPairKerning(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribKerning final : public EditCharAttrib
{
public:
    EditCharAttribKerning(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};



class EditCharAttribWordLineMode final : public EditCharAttrib
{
public:
    EditCharAttribWordLineMode(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);

    virtual void    SetFont( SvxFont& rFont, OutputDevice* pOutDev ) override;
};


class EditCharAttribGrabBag final : public EditCharAttrib
{
public:
    EditCharAttribGrabBag(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
