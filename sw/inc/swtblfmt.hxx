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

#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/shaditem.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>
#include <frmfmt.hxx>

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

    SwTableLineFormat* pFstColFormat;
    SwTableLineFormat* pLstColFormat;
    SwTableLineFormat* pOddColFormat;
    SwTableLineFormat* pEvnColFormat;

    sal_uInt16 m_aRepeatHeading;
    sal_Bool m_bRowSplit;

public:
    TYPEINFO_OVERRIDE();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableFormat)

    SwTableFormat& operator=( const SwTableFormat& rNew );

    void SetFirstLineFormat( SwTableLineFormat* pNew ) { pFstLineFormat = pNew; }
    void SetLastLineFormat( SwTableLineFormat* pNew ) { pLstLineFormat = pNew; }
    void SetOddLineFormat( SwTableLineFormat* pNew ) { pOddLineFormat = pNew; }
    void SetEvenLineFormat( SwTableLineFormat* pNew ) { pEvnLineFormat = pNew; }

    void SetFirstColFormat( SwTableLineFormat* pNew ) { pFstColFormat = pNew; }
    void SetLastColFormat( SwTableLineFormat* pNew ) { pLstColFormat = pNew; }
    void SetOddColFormat( SwTableLineFormat* pNew ) { pOddColFormat = pNew; }
    void SetEvenColFormat( SwTableLineFormat* pNew ) { pEvnColFormat = pNew; }

    SwTableLineFormat* GetFirstLineFormat() { return pFstLineFormat; }
    SwTableLineFormat* GetLastLineFormat() { return pLstLineFormat; }
    SwTableLineFormat* GetOddLineFormat() { return pOddLineFormat; }
    SwTableLineFormat* GetEvenLineFormat() { return pEvnLineFormat; }

    SwTableLineFormat* GetFirstColFormat() { return pFstColFormat; }
    SwTableLineFormat* GetLastColFormat() { return pLstColFormat; }
    SwTableLineFormat* GetOddColFormat() { return pOddColFormat; }
    SwTableLineFormat* GetEvenColFormat() { return pEvnColFormat; }

    void SetRepeatHeading( const sal_uInt16& rNew ) { m_aRepeatHeading = rNew; }
    void SetRowSplit( const sal_Bool& rNew ) { m_bRowSplit = rNew; }

    const sal_uInt16& GetRepeatHeading() const { return m_aRepeatHeading; }
    const sal_Bool& GetRowSplit() const { return m_bRowSplit; }

    void SetBreak( const SvxFormatBreakItem& rNew );
    void SetPageDesc( const SwFormatPageDesc& rNew );
    void SetKeepWithNextPara( const SvxFormatKeepItem& rNew );
    void SetLayoutSplit( const sal_Bool& rNew );
    void SetCollapsingBorders( const sal_Bool& rNew );
    void SetShadow( const SvxShadowItem& rNew );

    const SvxFormatBreakItem& GetBreak() const;
    const SwFormatPageDesc& GetPageDesc() const;
    const SvxFormatKeepItem& GetKeepWithNextPara() const;
    sal_Bool GetLayoutSplit() const;
    sal_Bool GetCollapsingBorders() const;
    const SvxShadowItem& GetShadow() const;

    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;
};

class SwTableLineFormat : public SwFrameFormat
{
    friend class SwDoc;

protected:
    SwTableLineFormat(SwAttrPool& rPool, const sal_Char* pFormatNm, SwFrameFormat *pDrvdFrm);
    SwTableLineFormat(SwAttrPool& rPool, const OUString &rFormatNm, SwFrameFormat *pDrvdFrm);

    // Odd and Even Box formats will be used to handle alternating columns
    SwTableBoxFormat* pFstBoxFormat;
    SwTableBoxFormat* pLstBoxFormat;
    SwTableBoxFormat* pOddBoxFormat;
    SwTableBoxFormat* pEvnBoxFormat;

public:
    TYPEINFO_OVERRIDE();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableLineFormat)

    void SetFirstBoxFormat( SwTableBoxFormat* pNew ) { pFstBoxFormat = pNew; }
    void SetLastBoxFormat( SwTableBoxFormat* pNew ) { pLstBoxFormat = pNew; }
    void SetOddBoxFormat( SwTableBoxFormat* pNew ) { pOddBoxFormat = pNew; }
    void SetEvenBoxFormat( SwTableBoxFormat* pNew ) { pEvnBoxFormat = pNew; }

    SwTableBoxFormat* GetFirstBoxFormat() { return pFstBoxFormat; }
    SwTableBoxFormat* GetLastBoxFormat() { return pLstBoxFormat; }
    SwTableBoxFormat* GetOddBoxFormat() { return pOddBoxFormat; }
    SwTableBoxFormat* GetEvenBoxFormat() { return pEvnBoxFormat; }

    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;
};

class SW_DLLPUBLIC SwTableBoxFormat : public SwFrameFormat
{
    friend class SwDoc;

protected:
    SwTableBoxFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
        : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}
    SwTableBoxFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
        : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}

   // For recognition of changes (especially TableBoxAttribute).
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue ) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBoxFormat)

    virtual bool supportsFullDrawingLayerFillAttributeSet() const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
