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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UIITEMS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UIITEMS_HXX

#include <memory>
#include <svl/poolitem.hxx>
#include <swdllapi.h>
#include <cmdid.h>
#include <pagedesc.hxx>

class SwNumRule;
class IntlWrapper;
class SwPaM;

// container for FootNote
class SW_DLLPUBLIC SwPageFootnoteInfoItem : public SfxPoolItem
{
    SwPageFootnoteInfo aFootnoteInfo;

public:

    SwPageFootnoteInfoItem(SwPageFootnoteInfo const & rInfo);
    virtual ~SwPageFootnoteInfoItem() override;

    SwPageFootnoteInfoItem(SwPageFootnoteInfoItem const &) = default;
    SwPageFootnoteInfoItem(SwPageFootnoteInfoItem &&) = default;
    SwPageFootnoteInfoItem & operator =(SwPageFootnoteInfoItem const &) = default;
    SwPageFootnoteInfoItem & operator =(SwPageFootnoteInfoItem &&) = default;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    SwPageFootnoteInfo& GetPageFootnoteInfo()             { return aFootnoteInfo; }
    const SwPageFootnoteInfo& GetPageFootnoteInfo() const { return aFootnoteInfo; }
};

class SW_DLLPUBLIC SwPtrItem : public SfxPoolItem
{
    void* pMisc;

public:
    SwPtrItem( const sal_uInt16 nId, void* pPtr);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    void*   GetValue() const        { return pMisc; }
};

class SW_DLLPUBLIC SwUINumRuleItem : public SfxPoolItem
{
    std::unique_ptr<SwNumRule> pRule;

public:
    SwUINumRuleItem( const SwNumRule& rRule );
    SwUINumRuleItem( const SwUINumRuleItem& rItem );
    virtual ~SwUINumRuleItem() override;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const SwNumRule* GetNumRule() const         { return pRule.get(); }
          SwNumRule* GetNumRule()               { return pRule.get(); }
};

class SW_DLLPUBLIC SwPaMItem : public SfxPoolItem
{
    SwPaM* m_pPaM;

public:
    SwPaMItem( const sal_uInt16 nId, SwPaM* pPaM);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    SwPaM*   GetValue() const        { return m_pPaM; }
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_UIITEMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
