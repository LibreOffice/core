/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: txatbase.hxx,v $
 * $Revision: 1.22.210.1 $
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
#ifndef _TXATBASE_HXX
#define _TXATBASE_HXX

#include <tools/solar.h>
#include <svtools/poolitem.hxx>
#include <hintids.hxx>
#include <errhdl.hxx>

#include <boost/utility.hpp>


class SfxItemPool;
class SvxBrushItem;
class SvxFontItem;
class SvxPostureItem;
class SvxWeightItem;
class SvxUnderlineItem;
class SvxOverlineItem;
class SvxFontHeightItem;
class SvxPropSizeItem;
class SvxShadowedItem;
class SvxAutoKernItem;
class SvxWordLineModeItem;
class SvxContourItem;
class SvxCrossedOutItem;
class SvxColorItem;
class SvxCharSetColorItem;
class SvXMLAttrContainerItem;
class SwFmtRuby;
class SvxTwoLinesItem;
class SvxEmphasisMarkItem;
class SvxCharScaleWidthItem;
class SvxCharRotateItem;
class SvxCharReliefItem;
class SvxCharHiddenItem;

class SwFmtCharFmt;
class SwFmtAutoFmt;
class SwFmtINetFmt;
class SvxKerningItem;
class SvxCaseMapItem;
class SvxLanguageItem;
class SvxEscapementItem;
class SvxBlinkItem;
class SvxNoHyphenItem;
class SwFmtSoftHyph;
class SwFmtHardBlank;
class SwFmtFld;
class SwFmtFtn;
class SwFmtFlyCnt;
class SwTOXMark;
class SwFmtRefMark;

class SwTxtAttr : private boost::noncopyable
{
    const SfxPoolItem* m_pAttr;
    xub_StrLen m_nStart;
    bool m_bDontExpand          : 1;
    bool m_bLockExpandFlag      : 1;

    bool m_bDontMergeAttr       : 1;    // refmarks, toxmarks, ruby
    bool m_bDontMoveAttr        : 1;    // refmarks, toxmarks
    bool m_bCharFmtAttr         : 1;    // charfmt, inet
    bool m_bOverlapAllowedAttr  : 1;    // refmarks, toxmarks
    bool m_bPriorityAttr        : 1;    // attribute has priority (redlining)
    bool m_bDontExpandStart     : 1;    // don't expand start at paragraph start (ruby)

protected:
    SwTxtAttr( const SfxPoolItem& rAttr, xub_StrLen nStart );

    void SetLockExpandFlag( bool bFlag )    { m_bLockExpandFlag = bFlag; }
    void SetDontMergeAttr( bool bFlag )     { m_bDontMergeAttr = bFlag; }
    void SetDontMoveAttr( bool bFlag )      { m_bDontMoveAttr = bFlag; }
    void SetCharFmtAttr( bool bFlag )       { m_bCharFmtAttr = bFlag; }
    void SetOverlapAllowedAttr( bool bFlag ){ m_bOverlapAllowedAttr = bFlag; }
    void SetDontExpandStartAttr(bool bFlag) { m_bDontExpandStart = bFlag; }

public:
    virtual ~SwTxtAttr();

    // RemoveFromPool muss immer vorm DTOR Aufruf erfolgen!!
    void RemoveFromPool( SfxItemPool& rPool );

    /// start position
                  xub_StrLen* GetStart()        { return & m_nStart; }
            const xub_StrLen* GetStart() const  { return & m_nStart; }

    /// end position
    virtual      xub_StrLen* GetEnd();
    inline const xub_StrLen* GetEnd() const;
    /// end (if available), else start
    inline const xub_StrLen* GetAnyEnd() const;

    inline void SetDontExpand( bool bDontExpand );
    bool DontExpand() const                 { return m_bDontExpand; }
    bool IsLockExpandFlag() const           { return m_bLockExpandFlag; }
    bool IsDontMergeAttr() const            { return m_bDontMergeAttr; }
    bool IsDontMoveAttr() const             { return m_bDontMoveAttr; }
    bool IsCharFmtAttr() const              { return m_bCharFmtAttr; }
    bool IsOverlapAllowedAttr() const       { return m_bOverlapAllowedAttr; }
    bool IsPriorityAttr() const             { return m_bPriorityAttr; }
    void SetPriorityAttr( bool bFlag )      { m_bPriorityAttr = bFlag; }
    bool IsDontExpandStartAttr() const      { return m_bDontExpandStart; }

    inline const SfxPoolItem& GetAttr() const;
    inline USHORT Which() const { return GetAttr().Which(); }

    virtual int         operator==( const SwTxtAttr& ) const;

    inline const SvxFontItem            &GetFont() const;
    inline const SvxPostureItem         &GetPosture() const;
    inline const SvxWeightItem          &GetWeight() const;
    inline const SvxUnderlineItem       &GetUnderline() const;
    inline const SvxOverlineItem        &GetOverline() const;
    inline const SvxFontHeightItem      &GetFontSize() const;
    inline const SvxPropSizeItem        &GetPropSize() const;
    inline const SvxShadowedItem        &GetShadowed() const;
    inline const SvxAutoKernItem        &GetAutoKern() const;
    inline const SvxWordLineModeItem    &GetWordLineMode() const;
    inline const SvxContourItem         &GetContour() const;
    inline const SvxCrossedOutItem      &GetCrossedOut() const;
    inline const SvxColorItem           &GetColor() const;
    inline const SvxCharSetColorItem    &GetCharSetColor() const;
    inline const SwFmtCharFmt           &GetCharFmt() const;
    inline const SwFmtAutoFmt           &GetAutoFmt() const;
    inline const SvxKerningItem         &GetKerning() const;
    inline const SvxCaseMapItem         &GetCaseMap() const;
    inline const SvxLanguageItem        &GetLanguage() const;
    inline const SvxEscapementItem      &GetEscapement() const;
    inline const SvxBlinkItem           &GetBlink() const;
    inline const SvxBrushItem           &GetChrBackground() const;
    inline const SvxNoHyphenItem        &GetNoHyphenHere() const;
    inline const SwFmtSoftHyph          &GetSoftHyph() const;
    inline const SwFmtHardBlank         &GetHardBlank() const;
    inline const SwFmtFld               &GetFld() const;
    inline const SwFmtFtn               &GetFtn() const;
    inline const SwFmtFlyCnt            &GetFlyCnt() const;
    inline const SwTOXMark              &GetTOXMark() const;
    inline const SwFmtRefMark           &GetRefMark() const;
    inline const SwFmtINetFmt           &GetINetFmt() const;
    inline const SvXMLAttrContainerItem &GetXMLAttrContainer() const;
    inline const SwFmtRuby              &GetRuby() const;
    inline const SvxTwoLinesItem        &Get2Lines() const;
    inline const SvxEmphasisMarkItem    &GetEmphasisMark() const;
    inline const SvxCharScaleWidthItem &GetCharScaleW() const;
    inline const SvxCharRotateItem      &GetCharRotate() const;
    inline const SvxCharReliefItem      &GetCharRelief() const;
    inline const SvxCharHiddenItem      &GetCharHidden() const;

};

class SwTxtAttrEnd : public SwTxtAttr
{
    using SwTxtAttr::GetEnd;

protected:
    xub_StrLen m_nEnd;

public:
    SwTxtAttrEnd( const SfxPoolItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual xub_StrLen* GetEnd();
};


// --------------- Inline Implementierungen ------------------------

inline const xub_StrLen* SwTxtAttr::GetEnd() const
{
    return const_cast<SwTxtAttr * >(this)->GetEnd();
}

inline const xub_StrLen* SwTxtAttr::GetAnyEnd() const
{
    const xub_StrLen* pEnd = GetEnd();
    return pEnd ? pEnd : GetStart();
}

inline const SfxPoolItem& SwTxtAttr::GetAttr() const
{
    ASSERT( m_pAttr, "SwTxtAttr: where is my attribute?" );
    return *m_pAttr;
}

inline void SwTxtAttr::SetDontExpand( bool bDontExpand )
{
    if ( !m_bLockExpandFlag )
    {
        m_bDontExpand = bDontExpand;
    }
}

inline const SvxFontItem& SwTxtAttr::GetFont() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_FONT, "Wrong attribute" );
    return (const SvxFontItem&)(*m_pAttr);
}

inline const SvxPostureItem& SwTxtAttr::GetPosture() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_POSTURE,
        "Wrong attribute" );
    return (const SvxPostureItem&)(*m_pAttr);
}

inline const SvxWeightItem& SwTxtAttr::GetWeight() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_WEIGHT,
        "Wrong attribute" );
    return (const SvxWeightItem&)(*m_pAttr);
}

inline const SvxUnderlineItem& SwTxtAttr::GetUnderline() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_UNDERLINE,
        "Wrong attribute" );
    return (const SvxUnderlineItem&)(*m_pAttr);
}

inline const SvxOverlineItem& SwTxtAttr::GetOverline() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_OVERLINE,
        "Wrong attribute" );
    return (const SvxOverlineItem&)(*m_pAttr);
}

inline const SvxFontHeightItem& SwTxtAttr::GetFontSize() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_FONTSIZE,
        "Wrong attribute" );
    return (const SvxFontHeightItem&)(*m_pAttr);
}

inline const SvxPropSizeItem& SwTxtAttr::GetPropSize() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_PROPORTIONALFONTSIZE,
        "Wrong attribute" );
    return (const SvxPropSizeItem&)(*m_pAttr);
}

inline const SvxShadowedItem& SwTxtAttr::GetShadowed() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_SHADOWED,
        "Wrong attribute" );
    return (const SvxShadowedItem&)(*m_pAttr);
}

inline const SvxAutoKernItem& SwTxtAttr::GetAutoKern() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_AUTOKERN,
        "Wrong attribute" );
    return (const SvxAutoKernItem&)(*m_pAttr);
}

inline const SvxWordLineModeItem& SwTxtAttr::GetWordLineMode() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_WORDLINEMODE,
        "Wrong attribute" );
    return (const SvxWordLineModeItem&)(*m_pAttr);
}

inline const SvxContourItem& SwTxtAttr::GetContour() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_CONTOUR,
        "Wrong attribute" );
    return (const SvxContourItem&)(*m_pAttr);
}

inline const SvxCrossedOutItem& SwTxtAttr::GetCrossedOut() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_CROSSEDOUT,
        "Wrong attribute" );
    return (const SvxCrossedOutItem&)(*m_pAttr);
}

inline const SvxColorItem& SwTxtAttr::GetColor() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_COLOR,
        "Wrong attribute" );
    return (const SvxColorItem&)(*m_pAttr);
}

inline const SvxCharSetColorItem& SwTxtAttr::GetCharSetColor() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_CHARSETCOLOR,
        "Wrong attribute" );
    return (const SvxCharSetColorItem&)(*m_pAttr);
}

inline const SwFmtCharFmt& SwTxtAttr::GetCharFmt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_CHARFMT,
        "Wrong attribute" );
    return (const SwFmtCharFmt&)(*m_pAttr);
}

inline const SwFmtAutoFmt& SwTxtAttr::GetAutoFmt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_AUTOFMT,
        "Wrong attribute" );
    return (const SwFmtAutoFmt&)(*m_pAttr);
}

inline const SvxKerningItem& SwTxtAttr::GetKerning() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_KERNING,
        "Wrong attribute" );
    return (const SvxKerningItem&)(*m_pAttr);
}

inline const SvxCaseMapItem& SwTxtAttr::GetCaseMap() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_CASEMAP,
        "Wrong attribute" );
    return (const SvxCaseMapItem&)(*m_pAttr);
}

inline const SvxLanguageItem& SwTxtAttr::GetLanguage() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_LANGUAGE,
        "Wrong attribute" );
    return (const SvxLanguageItem&)(*m_pAttr);
}

inline const SvxEscapementItem& SwTxtAttr::GetEscapement() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_ESCAPEMENT,
        "Wrong attribute" );
    return (const SvxEscapementItem&)(*m_pAttr);
}

inline const SvxBlinkItem& SwTxtAttr::GetBlink() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_BLINK,
        "Wrong attribute" );
    return (const SvxBlinkItem&)(*m_pAttr);
}

inline const SvxBrushItem& SwTxtAttr::GetChrBackground() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_BACKGROUND,
        "Wrong attribute" );
    return (const SvxBrushItem&)(*m_pAttr);
}

inline const SvxNoHyphenItem& SwTxtAttr::GetNoHyphenHere() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_NOHYPHEN,
        "Wrong attribute" );
    return (const SvxNoHyphenItem&)(*m_pAttr);
}

inline const SwFmtSoftHyph& SwTxtAttr::GetSoftHyph() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_SOFTHYPH,
        "Wrong attribute" );
    return (const SwFmtSoftHyph&)(*m_pAttr);
}

inline const SwFmtHardBlank& SwTxtAttr::GetHardBlank() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_HARDBLANK,
        "Wrong attribute" );
    return (const SwFmtHardBlank&)(*m_pAttr);
}

inline const SwFmtFld& SwTxtAttr::GetFld() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_FIELD,
        "Wrong attribute" );
    return (const SwFmtFld&)(*m_pAttr);
}

inline const SwFmtFtn& SwTxtAttr::GetFtn() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_FTN, "Wrong attribute" );
    return (const SwFmtFtn&)(*m_pAttr);
}

inline const SwFmtFlyCnt& SwTxtAttr::GetFlyCnt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_FLYCNT,
        "Wrong attribute" );
    return (const SwFmtFlyCnt&)(*m_pAttr);
}

inline const SwTOXMark& SwTxtAttr::GetTOXMark() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_TOXMARK,
        "Wrong attribute" );
    return (const SwTOXMark&)(*m_pAttr);
}

inline const SwFmtRefMark& SwTxtAttr::GetRefMark() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_REFMARK,
        "Wrong attribute" );
    return (const SwFmtRefMark&)(*m_pAttr);
}

inline const SwFmtINetFmt& SwTxtAttr::GetINetFmt() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_INETFMT,
        "Wrong attribute" );
    return (const SwFmtINetFmt&)(*m_pAttr);
}

inline const SvXMLAttrContainerItem& SwTxtAttr::GetXMLAttrContainer() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_UNKNOWNATR_CONTAINER,
        "Wrong attribute" );
    return (const SvXMLAttrContainerItem&)(*m_pAttr);
}

inline const SwFmtRuby& SwTxtAttr::GetRuby() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_TXTATR_CJK_RUBY,
        "Wrong attribute" );
    return (const SwFmtRuby&)(*m_pAttr);
}
inline const SvxTwoLinesItem& SwTxtAttr::Get2Lines() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_TWO_LINES,
        "Wrong attribute" );
    return (const SvxTwoLinesItem&)(*m_pAttr);
}

inline const SvxEmphasisMarkItem& SwTxtAttr::GetEmphasisMark() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_EMPHASIS_MARK,
        "Wrong attribute" );
    return (const SvxEmphasisMarkItem&)(*m_pAttr);
}

inline const SvxCharScaleWidthItem&    SwTxtAttr::GetCharScaleW() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_SCALEW,
        "Wrong attribute" );
    return (const SvxCharScaleWidthItem&)(*m_pAttr);
}

inline const SvxCharRotateItem&    SwTxtAttr::GetCharRotate() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_ROTATE,
        "Wrong attribute" );
    return (const SvxCharRotateItem&)(*m_pAttr);
}

inline const SvxCharReliefItem&    SwTxtAttr::GetCharRelief() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_RELIEF,
        "Wrong attribute" );
    return (const SvxCharReliefItem&)(*m_pAttr);
}

inline const SvxCharHiddenItem& SwTxtAttr::GetCharHidden() const
{
    ASSERT( m_pAttr && m_pAttr->Which() == RES_CHRATR_HIDDEN,
        "Wrong attribute" );
    return (const SvxCharHiddenItem&)(*m_pAttr);
}

#endif
