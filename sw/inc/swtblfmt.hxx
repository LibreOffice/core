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
#ifndef INCLUDED_SW_INC_SWTBLFMT_HXX
#define INCLUDED_SW_INC_SWTBLFMT_HXX

#include "hintids.hxx"
#include <charatr.hxx>
#include <cmdid.h>
#include <doc.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/shaditem.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>
#include <fmtornt.hxx>
#include <fmtrowsplt.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <paratr.hxx>

class SwDoc;
class SwTableLineFormat;
class SwTableBoxFormat;

class SW_DLLPUBLIC SwTableFormat : public SwFrameFormat
{
    friend class SwDoc;

protected:
    SwTableFormat(SwAttrPool& rPool, const sal_Char* pFormatNm, SwFrameFormat *pDrvdFrm);
    SwTableFormat(SwAttrPool& rPool, const OUString &rFormatNm, SwFrameFormat *pDrvdFrm);

    SwTableLineFormat* pFstLineFormat;
    SwTableLineFormat* pLstLineFormat;
    SwTableLineFormat* pOddLineFormat;
    SwTableLineFormat* pEvnLineFormat;

    SwTableLineFormat* m_pFstLineFormat;
    SwTableLineFormat* m_pLstLineFormat;
    SwTableLineFormat* m_pOddLineFormat;
    SwTableLineFormat* m_pEvnLineFormat;

    SwTableLineFormat* m_pFstColFormat;
    SwTableLineFormat* m_pLstColFormat;
    SwTableLineFormat* m_pOddColFormat;
    SwTableLineFormat* m_pEvnColFormat;

public:
    SwTableFormat( SwTableFormat& rNew );
    SwTableFormat& operator=( const SwTableFormat& rNew );

    void SetFirstLineFormat( SwTableLineFormat* pNew ) { m_pFstLineFormat = pNew; }
    void SetLastLineFormat( SwTableLineFormat* pNew ) { m_pLstLineFormat = pNew; }
    void SetOddLineFormat( SwTableLineFormat* pNew ) { m_pOddLineFormat = pNew; }
    void SetEvenLineFormat( SwTableLineFormat* pNew ) { m_pEvnLineFormat = pNew; }

    void SetFirstColFormat( SwTableLineFormat* pNew ) { m_pFstColFormat = pNew; }
    void SetLastColFormat( SwTableLineFormat* pNew ) { m_pLstColFormat = pNew; }
    void SetOddColFormat( SwTableLineFormat* pNew ) { m_pOddColFormat = pNew; }
    void SetEvenColFormat( SwTableLineFormat* pNew ) { m_pEvnColFormat = pNew; }

    SwTableLineFormat* GetFirstLineFormat() { return m_pFstLineFormat; }
    SwTableLineFormat* GetLastLineFormat() { return m_pLstLineFormat; }
    SwTableLineFormat* GetOddLineFormat() { return m_pOddLineFormat; }
    SwTableLineFormat* GetEvenLineFormat() { return m_pEvnLineFormat; }

    SwTableLineFormat* GetFirstColFormat() { return m_pFstColFormat; }
    SwTableLineFormat* GetLastColFormat() { return m_pLstColFormat; }
    SwTableLineFormat* GetOddColFormat() { return m_pOddColFormat; }
    SwTableLineFormat* GetEvenColFormat() { return m_pEvnColFormat; }

    void SetBreak( const SvxFormatBreakItem& rNew );
    void SetPageDesc( const SwFormatPageDesc& rNew );
    void SetKeepWithNextPara( const SvxFormatKeepItem& rNew );
    void SetLayoutSplit( const sal_Bool& rNew );
    void SetCollapsingBorders( const sal_Bool& rNew );
    void SetRowSplit( const sal_Bool& rNew );
    void SetRepeatHeading( const sal_uInt16& rNew );
    void SetShadow( const SvxShadowItem& rNew );

    const SvxFormatKeepItem& GetKeepWithNextPara() const;
    sal_Bool GetLayoutSplit() const;
    sal_Bool GetCollapsingBorders() const;
    sal_Bool GetRowSplit() const;
    sal_uInt16 GetRepeatHeading() const;

    void CopyTableFormatInfo( SwTableFormat* pTableFormat );

    TYPEINFO_OVERRIDE();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableFormat)

    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;
};

class SwTableLineFormat : public SwFrameFormat
{
    friend class SwDoc;

protected:
    SwTableLineFormat(SwAttrPool& rPool, const sal_Char* pFormatNm, SwFrameFormat *pDrvdFrm);
    SwTableLineFormat(SwAttrPool& rPool, const OUString &rFormatNm, SwFrameFormat *pDrvdFrm);

    // Odd and Even Box formats will be used to handle alternating columns
    SwTableBoxFormat* m_pFstBoxFormat;
    SwTableBoxFormat* m_pLstBoxFormat;
    SwTableBoxFormat* m_pOddBoxFormat;
    SwTableBoxFormat* m_pEvnBoxFormat;

public:
    SwTableLineFormat( SwTableLineFormat& rNew );
    SwTableLineFormat& operator=( const SwTableLineFormat& rNew );

    void SetFirstBoxFormat( SwTableBoxFormat* pNew ) { m_pFstBoxFormat = pNew; }
    void SetLastBoxFormat( SwTableBoxFormat* pNew ) { m_pLstBoxFormat = pNew; }
    void SetOddBoxFormat( SwTableBoxFormat* pNew ) { m_pOddBoxFormat = pNew; }
    void SetEvenBoxFormat( SwTableBoxFormat* pNew ) { m_pEvnBoxFormat = pNew; }

    SwTableBoxFormat* GetFirstBoxFormat() { return m_pFstBoxFormat; }
    SwTableBoxFormat* GetLastBoxFormat() { return m_pLstBoxFormat; }
    SwTableBoxFormat* GetOddBoxFormat() { return m_pOddBoxFormat; }
    SwTableBoxFormat* GetEvenBoxFormat() { return m_pEvnBoxFormat; }

    TYPEINFO_OVERRIDE();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableLineFormat)

    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwTableBoxFormat : public SwFrameFormat
{
    friend class SwDoc;

    OUString            sNumFormatString;
    LanguageType        eSysLanguage, eNumFormatLanguage;

protected:
    SwTableBoxFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm );
    SwTableBoxFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm );

   // For recognition of changes (especially TableBoxAttribute).
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue ) SAL_OVERRIDE;

public:
    void SetFont( const SvxFontItem& rNew );
    void SetHeight( const SvxFontHeightItem& rNew );
    void SetWeight( const SvxWeightItem& rNew );
    void SetPosture( const SvxPostureItem& rNew );

    void SetCJKFont( const SvxFontItem& rNew );
    void SetCJKHeight( const SvxFontHeightItem& rNew );
    void SetCJKWeight( const SvxWeightItem& rNew );
    void SetCJKPosture( const SvxPostureItem& rNew );

    void SetCTLFont( const SvxFontItem& rNew );
    void SetCTLHeight( const SvxFontHeightItem& rNew );
    void SetCTLWeight( const SvxWeightItem& rNew );
    void SetCTLPosture( const SvxPostureItem& rNew );

    void SetUnderline( const SvxUnderlineItem& rNew );
    void SetOverline( const SvxOverlineItem& rNew );
    void SetCrossedOut( const SvxCrossedOutItem& rNew );
    void SetContour( const SvxContourItem& rNew );
    void SetShadowed( const SvxShadowedItem& rNew );
    void SetColor( const SvxColorItem& rNew );
    void SetBox( const SvxBoxItem& rNew );
    void SetBackground( const SvxBrushItem& rNew );

    void SetAdjust( const SvxAdjustItem& rNew );
    void SetTextOrientation( const SvxFrameDirectionItem& rNew );
    void SetVerticalAlignment( const SwFormatVertOrient& rNew );

    void SetValueFormat( const OUString& rFormat, LanguageType eLng, LanguageType eSys );

    const SvxFrameDirectionItem& GetTextOrientation() const;
    const SwFormatVertOrient& GetVerticalAlignment() const;

    void GetValueFormat( OUString& rFormat, LanguageType& rLng, LanguageType& rSys ) const;

    TYPEINFO_OVERRIDE();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBoxFormat)

    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
