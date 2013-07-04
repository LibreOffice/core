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
#ifndef _SWTBLFMT_HXX
#define _SWTBLFMT_HXX

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
#include <editsh.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>
#include <fmtornt.hxx>
#include <fmtrowsplt.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <paratr.hxx>
#include <swtable.hxx>

struct SwAfVersions;

class SwDoc;
class SwTableLineFmt;
class SwTableBoxFmt;

class SW_DLLPUBLIC SwTableFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    SwTableFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm );

    SwTableFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm );

    boost::scoped_ptr<SwTableLineFmt> m_pFstLineFmt;
    boost::scoped_ptr<SwTableLineFmt> m_pLstLineFmt;
    boost::scoped_ptr<SwTableLineFmt> m_pOddLineFmt;
    boost::scoped_ptr<SwTableLineFmt> m_pEvnLineFmt;

    boost::scoped_ptr<SwTableLineFmt> m_pFstColFmt;
    boost::scoped_ptr<SwTableLineFmt> m_pLstColFmt;
    boost::scoped_ptr<SwTableLineFmt> m_pOddColFmt;
    boost::scoped_ptr<SwTableLineFmt> m_pEvnColFmt;

public:
    SwTableFmt( const SwTableFmt& rNew );
    SwTableFmt& operator=( const SwTableFmt& rNew );

    void SetFirstLineFmt( SwTableLineFmt* pNew ) { m_pFstLineFmt.reset( pNew ); }
    void SetLastLineFmt( SwTableLineFmt* pNew ) { m_pLstLineFmt.reset( pNew ); }
    void SetOddLineFmt( SwTableLineFmt* pNew ) { m_pOddLineFmt.reset( pNew ); }
    void SetEvenLineFmt( SwTableLineFmt* pNew ) { m_pEvnLineFmt.reset( pNew ); }

    void SetFirstColFmt( SwTableLineFmt* pNew ) { m_pFstColFmt.reset( pNew ); }
    void SetLastColFmt( SwTableLineFmt* pNew ) { m_pLstColFmt.reset( pNew ); }
    void SetOddColFmt( SwTableLineFmt* pNew ) { m_pOddColFmt.reset( pNew ); }
    void SetEvenColFmt( SwTableLineFmt* pNew ) { m_pEvnColFmt.reset( pNew ); }

    SwTableLineFmt* GetFirstLineFmt() const { return m_pFstLineFmt.get(); }
    SwTableLineFmt* GetLastLineFmt() const { return m_pLstLineFmt.get(); }
    SwTableLineFmt* GetOddLineFmt() const { return m_pOddLineFmt.get(); }
    SwTableLineFmt* GetEvenLineFmt() const { return m_pEvnLineFmt.get(); }

    SwTableLineFmt* GetFirstColFmt() const { return m_pFstColFmt.get(); }
    SwTableLineFmt* GetLastColFmt() const { return m_pLstColFmt.get(); }
    SwTableLineFmt* GetOddColFmt() const { return m_pOddColFmt.get(); }
    SwTableLineFmt* GetEvenColFmt() const { return m_pEvnColFmt.get(); }

    void SetBoxFmt( const SwTableBoxFmt& rNew, sal_uInt8 nPos );
    SwTableBoxFmt* GetBoxFmt( sal_uInt8 nPos ) const;

    void SetBreak( const SvxFmtBreakItem& rNew );
    void SetPageDesc( const SwFmtPageDesc& rNew );
    void SetKeepWithNextPara( const SvxFmtKeepItem& rNew );
    void SetLayoutSplit( const sal_Bool& rNew );
    void SetCollapsingBorders( const sal_Bool& rNew );
    void SetRowSplit( const sal_Bool& rNew );
    void SetRepeatHeading( const sal_uInt16& rNew );
    void SetShadow( const SvxShadowItem& rNew );

    const SvxFmtKeepItem& GetKeepWithNextPara() const;
    sal_Bool GetLayoutSplit() const;
    sal_Bool GetCollapsingBorders() const;
    sal_Bool GetRowSplit() const;
    sal_uInt16 GetRepeatHeading() const;

    void RestoreTableProperties( SwTable &table ) const;
    void StoreTableProperties( const SwTable &table );

    void CopyTableFormatInfo( const SwTableFmt* pTableFormat );

    TYPEINFO();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableFmt)
};

class SwTableLineFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    SwTableLineFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm );

    SwTableLineFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm );

    // Odd and Even Box formats will be used to handle alternating columns
    boost::scoped_ptr<SwTableBoxFmt> m_pFstBoxFmt;
    boost::scoped_ptr<SwTableBoxFmt> m_pLstBoxFmt;
    boost::scoped_ptr<SwTableBoxFmt> m_pOddBoxFmt;
    boost::scoped_ptr<SwTableBoxFmt> m_pEvnBoxFmt;

public:
    SwTableLineFmt( const SwTableLineFmt& rNew );
    SwTableLineFmt& operator=( const SwTableLineFmt& rNew );

    void SetFirstBoxFmt( SwTableBoxFmt* pNew ) { m_pFstBoxFmt.reset( pNew ); }
    void SetLastBoxFmt( SwTableBoxFmt* pNew ) { m_pLstBoxFmt.reset( pNew ); }
    void SetOddBoxFmt( SwTableBoxFmt* pNew ) { m_pOddBoxFmt.reset( pNew ); }
    void SetEvenBoxFmt( SwTableBoxFmt* pNew ) { m_pEvnBoxFmt.reset( pNew ); }

    SwTableBoxFmt* GetFirstBoxFmt() { return m_pFstBoxFmt.get(); }
    SwTableBoxFmt* GetLastBoxFmt() { return m_pLstBoxFmt.get(); }
    SwTableBoxFmt* GetOddBoxFmt() { return m_pOddBoxFmt.get(); }
    SwTableBoxFmt* GetEvenBoxFmt() { return m_pEvnBoxFmt.get(); }

    TYPEINFO();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableLineFmt)
};

class SW_DLLPUBLIC SwTableBoxFmt : public SwFrmFmt
{
    friend class SwDoc;

    String              sNumFmtString;
    LanguageType        eSysLanguage, eNumFmtLanguage;

protected:
    SwTableBoxFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm );
    SwTableBoxFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm );

   // For recognition of changes (especially TableBoxAttribute).
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue );

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
    void SetVerticalAlignment( const SwFmtVertOrient& rNew );

    void SetValueFormat( const String& rFmt, LanguageType eLng, LanguageType eSys );

    const SvxFontHeightItem& GetHeight() const;
    const SvxFontHeightItem& GetCJKHeight() const;
    const SvxFontHeightItem& GetCTLHeight() const;

    const SvxFrameDirectionItem& GetTextOrientation() const;
    const SwFmtVertOrient& GetVerticalAlignment() const;

    void GetValueFormat( String& rFmt, LanguageType& rLng, LanguageType& rSys ) const;

    sal_Bool Load( SvStream& rStream, const SwAfVersions& rVersions, sal_uInt16 nVer );
    sal_Bool Save( SvStream& rStream, sal_uInt16 fileVersion ) const;
    sal_Bool SaveVersionNo( SvStream& rStream, sal_uInt16 fileVersion ) const;

    TYPEINFO();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBoxFmt)
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
