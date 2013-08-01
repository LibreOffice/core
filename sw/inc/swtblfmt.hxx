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
#include <boost/scoped_ptr.hpp>
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

struct SwAfVersions;

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

    boost::scoped_ptr<SwTableLineFormat> m_pFstLineFormat;
    boost::scoped_ptr<SwTableLineFormat> m_pLstLineFormat;
    boost::scoped_ptr<SwTableLineFormat> m_pOddLineFormat;
    boost::scoped_ptr<SwTableLineFormat> m_pEvnLineFormat;

    boost::scoped_ptr<SwTableLineFormat> m_pFstColFormat;
    boost::scoped_ptr<SwTableLineFormat> m_pLstColFormat;
    boost::scoped_ptr<SwTableLineFormat> m_pOddColFormat;
    boost::scoped_ptr<SwTableLineFormat> m_pEvnColFormat;

public:
    SwTableFormat( const SwTableFormat& rNew );
    SwTableFormat& operator=( const SwTableFormat& rNew );

    void SetFirstLineFormat( SwTableLineFormat* pNew ) { m_pFstLineFormat.reset( pNew ); }
    void SetLastLineFormat( SwTableLineFormat* pNew ) { m_pLstLineFormat.reset( pNew ); }
    void SetOddLineFormat( SwTableLineFormat* pNew ) { m_pOddLineFormat.reset( pNew ); }
    void SetEvenLineFormat( SwTableLineFormat* pNew ) { m_pEvnLineFormat.reset( pNew ); }

    void SetFirstColFormat( SwTableLineFormat* pNew ) { m_pFstColFormat.reset( pNew ); }
    void SetLastColFormat( SwTableLineFormat* pNew ) { m_pLstColFormat.reset( pNew ); }
    void SetOddColFormat( SwTableLineFormat* pNew ) { m_pOddColFormat.reset( pNew ); }
    void SetEvenColFormat( SwTableLineFormat* pNew ) { m_pEvnColFormat.reset( pNew ); }

    SwTableLineFormat* GetFirstLineFormat() const { return m_pFstLineFormat.get(); }
    SwTableLineFormat* GetLastLineFormat() const { return m_pLstLineFormat.get(); }
    SwTableLineFormat* GetOddLineFormat() const { return m_pOddLineFormat.get(); }
    SwTableLineFormat* GetEvenLineFormat() const { return m_pEvnLineFormat.get(); }

    SwTableLineFormat* GetFirstColFormat() const { return m_pFstColFormat.get(); }
    SwTableLineFormat* GetLastColFormat() const { return m_pLstColFormat.get(); }
    SwTableLineFormat* GetOddColFormat() const { return m_pOddColFormat.get(); }
    SwTableLineFormat* GetEvenColFormat() const { return m_pEvnColFormat.get(); }

    void SetBoxFormat( const SwTableBoxFormat& rNew, sal_uInt8 nPos );
    SwTableBoxFormat* GetBoxFormat( sal_uInt8 nPos ) const;

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

    static void RestoreTableProperties( SwTableFormat* pSrcFormat, SwTable &table );
    static SwTableFormat* StoreTableProperties( const SwTable &table );

    static void AssignLineParents( SwTableFormat* pSrcFormat, SwTable &table );
    static void AssignBoxParents( SwTableLineFormat* pSrcLineFormat, SwTableLine &line );
    static void AssignLineParents_Complex( SwTableLineFormat* pSrcLineFormat, SwTableBoxFormat* pSrcBoxFormat, SwTableBox& rBox );
    static void AssignBoxParents_Complex( SwTableLineFormat* pSrcLineFormat, SwTableBoxFormat* pSrcBoxFormat, SwTableLine& rLine );

    bool Load( SvStream& rStream, const SwAfVersions& rVersions, SwDoc* pDoc, sal_uInt16 nVal );

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
    boost::scoped_ptr<SwTableBoxFormat> m_pFstBoxFormat;
    boost::scoped_ptr<SwTableBoxFormat> m_pLstBoxFormat;
    boost::scoped_ptr<SwTableBoxFormat> m_pOddBoxFormat;
    boost::scoped_ptr<SwTableBoxFormat> m_pEvnBoxFormat;

public:
    SwTableLineFormat( const SwTableLineFormat& rNew );
    SwTableLineFormat& operator=( const SwTableLineFormat& rNew );

    void SetFirstBoxFormat( SwTableBoxFormat* pNew ) { m_pFstBoxFormat.reset( pNew ); }
    void SetLastBoxFormat( SwTableBoxFormat* pNew ) { m_pLstBoxFormat.reset( pNew ); }
    void SetOddBoxFormat( SwTableBoxFormat* pNew ) { m_pOddBoxFormat.reset( pNew ); }
    void SetEvenBoxFormat( SwTableBoxFormat* pNew ) { m_pEvnBoxFormat.reset( pNew ); }

    SwTableBoxFormat* GetFirstBoxFormat() { return m_pFstBoxFormat.get(); }
    SwTableBoxFormat* GetLastBoxFormat() { return m_pLstBoxFormat.get(); }
    SwTableBoxFormat* GetOddBoxFormat() { return m_pOddBoxFormat.get(); }
    SwTableBoxFormat* GetEvenBoxFormat() { return m_pEvnBoxFormat.get(); }

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

    const SvxFontHeightItem& GetHeight() const;
    const SvxFontHeightItem& GetCJKHeight() const;
    const SvxFontHeightItem& GetCTLHeight() const;

    const SvxFrameDirectionItem& GetTextOrientation() const;
    const SwFormatVertOrient& GetVerticalAlignment() const;

    void GetValueFormat( OUString& rFormat, LanguageType& rLng, LanguageType& rSys ) const;

    bool Load( SvStream& rStream, const SwAfVersions& rVersions, sal_uInt16 nVer );
    bool Save( SvStream& rStream, sal_uInt16 fileVersion ) const;
    bool SaveVersionNo( SvStream& rStream, sal_uInt16 fileVersion ) const;

    TYPEINFO_OVERRIDE();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBoxFormat)

    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
