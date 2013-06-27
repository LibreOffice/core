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

#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/shaditem.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>

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

    SwTableLineFmt* pFstLineFmt;
    SwTableLineFmt* pLstLineFmt;
    SwTableLineFmt* pOddLineFmt;
    SwTableLineFmt* pEvnLineFmt;

    SwTableLineFmt* pFstColFmt;
    SwTableLineFmt* pLstColFmt;
    SwTableLineFmt* pOddColFmt;
    SwTableLineFmt* pEvnColFmt;

    sal_uInt16 m_aRepeatHeading;
    sal_Bool m_bRowSplit;

public:
    SwTableFmt& operator=( const SwTableFmt& rNew );

    void SetFirstLineFmt( SwTableLineFmt* pNew ) { pFstLineFmt = pNew; }
    void SetLastLineFmt( SwTableLineFmt* pNew ) { pLstLineFmt = pNew; }
    void SetOddLineFmt( SwTableLineFmt* pNew ) { pOddLineFmt = pNew; }
    void SetEvenLineFmt( SwTableLineFmt* pNew ) { pEvnLineFmt = pNew; }

    void SetFirstColFmt( SwTableLineFmt* pNew ) { pFstColFmt = pNew; }
    void SetLastColFmt( SwTableLineFmt* pNew ) { pLstColFmt = pNew; }
    void SetOddColFmt( SwTableLineFmt* pNew ) { pOddColFmt = pNew; }
    void SetEvenColFmt( SwTableLineFmt* pNew ) { pEvnColFmt = pNew; }

    SwTableLineFmt* GetFirstLineFmt() { return pFstLineFmt; }
    SwTableLineFmt* GetLastLineFmt() { return pLstLineFmt; }
    SwTableLineFmt* GetOddLineFmt() { return pOddLineFmt; }
    SwTableLineFmt* GetEvenLineFmt() { return pEvnLineFmt; }

    SwTableLineFmt* GetFirstColFmt() { return pFstColFmt; }
    SwTableLineFmt* GetLastColFmt() { return pLstColFmt; }
    SwTableLineFmt* GetOddColFmt() { return pOddColFmt; }
    SwTableLineFmt* GetEvenColFmt() { return pEvnColFmt; }

    void SetRepeatHeading( const sal_uInt16& rNew ) { m_aRepeatHeading = rNew; }
    void SetRowSplit( const sal_Bool& rNew ) { m_bRowSplit = rNew; }

    const sal_uInt16& GetRepeatHeading() const { return m_aRepeatHeading; }
    const sal_Bool& GetRowSplit() const { return m_bRowSplit; }

    void SetBreak( const SvxFmtBreakItem& rNew );
    void SetPageDesc( const SwFmtPageDesc& rNew );
    void SetKeepWithNextPara( const SvxFmtKeepItem& rNew );
    void SetLayoutSplit( const sal_Bool& rNew );
    void SetCollapsingBorders( const sal_Bool& rNew );
    void SetShadow( const SvxShadowItem& rNew );

    const SvxFmtKeepItem& GetKeepWithNextPara() const;
    sal_Bool GetLayoutSplit() const;
    sal_Bool GetCollapsingBorders() const;

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
    SwTableBoxFmt* pFstBoxFmt;
    SwTableBoxFmt* pLstBoxFmt;
    SwTableBoxFmt* pOddBoxFmt;
    SwTableBoxFmt* pEvnBoxFmt;

public:

    void SetFirstBoxFmt( SwTableBoxFmt* pNew ) { pFstBoxFmt = pNew; }
    void SetLastBoxFmt( SwTableBoxFmt* pNew ) { pLstBoxFmt = pNew; }
    void SetOddBoxFmt( SwTableBoxFmt* pNew ) { pOddBoxFmt = pNew; }
    void SetEvenBoxFmt( SwTableBoxFmt* pNew ) { pEvnBoxFmt = pNew; }

    SwTableBoxFmt* GetFirstBoxFmt() { return pFstBoxFmt; }
    SwTableBoxFmt* GetLastBoxFmt() { return pLstBoxFmt; }
    SwTableBoxFmt* GetOddBoxFmt() { return pOddBoxFmt; }
    SwTableBoxFmt* GetEvenBoxFmt() { return pEvnBoxFmt; }

    TYPEINFO();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableLineFmt)
};

class SW_DLLPUBLIC SwTableBoxFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    SwTableBoxFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}
    SwTableBoxFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}

   // For recognition of changes (especially TableBoxAttribute).
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue );

public:
    TYPEINFO();     // Already in base class Content.

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBoxFmt)
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
