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
#ifndef INCLUDED_SW_INC_FMTPDSC_HXX
#define INCLUDED_SW_INC_FMTPDSC_HXX

#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include "hintids.hxx"
#include "format.hxx"
#include "calbck.hxx"
#include <boost/optional.hpp>
#include "pagedesc.hxx"

class IntlWrapper;

/** Pagedescriptor
 Client of SwPageDesc that is "described" by the attribute. */

class SW_DLLPUBLIC SwFormatPageDesc : public SfxPoolItem, public SwClient
{
    ::boost::optional<sal_uInt16> m_oNumOffset;          ///< Offset page number.
    SwModify* m_pDefinedIn;       /**< Points to the object in which the
                                 attribute was set (ContentNode/Format). */
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;
    virtual void SwClientNotify( const SwModify&, const SfxHint& rHint ) override;

public:
    SwFormatPageDesc( const SwPageDesc *pDesc = nullptr );
    SwFormatPageDesc( const SwFormatPageDesc &rCpy );
    SwFormatPageDesc &operator=( const SwFormatPageDesc &rCpy );
    virtual ~SwFormatPageDesc() override;


    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

          SwPageDesc *GetPageDesc() { return static_cast<SwPageDesc*>(GetRegisteredIn()); }
    const SwPageDesc *GetPageDesc() const { return static_cast<const SwPageDesc*>(GetRegisteredIn()); }

    const ::boost::optional<sal_uInt16>&  GetNumOffset() const        { return m_oNumOffset; }
    void    SetNumOffset( const ::boost::optional<sal_uInt16>& oNum ) { m_oNumOffset = oNum; }

    /// Query / set where attribute is anchored.
    const SwModify* GetDefinedIn() const { return m_pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew ) { m_pDefinedIn = const_cast<SwModify*>(pNew); }
    void RegisterToPageDesc( SwPageDesc& );
    bool KnowsPageDesc() const;
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwFormatPageDesc &SwAttrSet::GetPageDesc(bool bInP) const
    { return Get( RES_PAGEDESC,bInP); }

inline const SwFormatPageDesc &SwFormat::GetPageDesc(bool bInP) const
    { return m_aSet.GetPageDesc(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
