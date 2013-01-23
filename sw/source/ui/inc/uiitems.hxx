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
#ifndef _UIITEMS_HXX
#define _UIITEMS_HXX

#include <svl/intitem.hxx>
#include "swdllapi.h"
#include "cmdid.h"
#include "pagedesc.hxx"
#include "pam.hxx"

class SwNumRule;
class IntlWrapper;

/*--------------------------------------------------------------------
    Description: container for FootNote
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwPageFtnInfoItem : public SfxPoolItem
{
    SwPageFtnInfo aFtnInfo;

public:

    SwPageFtnInfoItem(const sal_uInt16 nId, SwPageFtnInfo& rInfo);
    SwPageFtnInfoItem(const SwPageFtnInfoItem& rItem );
    ~SwPageFtnInfoItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    SwPageFtnInfo& GetPageFtnInfo()             { return aFtnInfo; }
    const SwPageFtnInfo& GetPageFtnInfo() const { return aFtnInfo; }
    void SetPageFtnInfo(SwPageFtnInfo& rInf)    { aFtnInfo = rInf; }
};

class SW_DLLPUBLIC SwPtrItem : public SfxPoolItem
{
    void* pMisc;

public:
    SwPtrItem( const sal_uInt16 nId = FN_PARAM_GRF_DIALOG, void* pPtr = 0);
    SwPtrItem( const SwPtrItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void    SetValue(void * pPtr)   { pMisc= pPtr; }
    void*   GetValue() const        { return pMisc; }
};

class SW_DLLPUBLIC SwUINumRuleItem : public SfxPoolItem
{
    SwNumRule* pRule;

public:
    SwUINumRuleItem( const SwNumRule& rRule, const sal_uInt16 = FN_PARAM_ACT_NUMBER);
    SwUINumRuleItem( const SwUINumRuleItem& rItem );
    virtual ~SwUINumRuleItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const SwNumRule* GetNumRule() const         { return pRule; }
          SwNumRule* GetNumRule()               { return pRule; }
};

class SwBackgroundDestinationItem : public SfxUInt16Item
{
public:
    SwBackgroundDestinationItem(sal_uInt16  nWhich, sal_uInt16 nValue);

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
};

class SW_DLLPUBLIC SwPaMItem : public SfxPoolItem
{
    SwPaM* m_pPaM;

public:
    SwPaMItem( const sal_uInt16 nId = FN_PARAM_PAM, SwPaM* pPaM = NULL);
    SwPaMItem( const SwPaMItem& rItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    void    SetValue(SwPaM * pPaM)   { m_pPaM= pPaM; }
    SwPaM*   GetValue() const        { return m_pPaM; }
};

#endif // _UIITEMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
