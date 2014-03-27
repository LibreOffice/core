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
#include <hintids.hxx>
#include <format.hxx>
#include <calbck.hxx>
#include <boost/optional.hpp>

class SwPageDesc;
class SwHistory;
class SwPaM;
class IntlWrapper;
class SwEndNoteInfo;

/** Pagedescriptor
 Client of SwPageDesc that is "described" by the attribute. */

class SW_DLLPUBLIC SwFmtPageDesc : public SfxPoolItem, public SwClient
{
    /** This "Doc"-function is made friend in order to be able
     to set the auto-flag after copying!! */
    friend sal_Bool InsAttr( SwDoc*, const SwPaM &, const SfxItemSet&, sal_uInt16,
                        SwHistory* );
    ::boost::optional<sal_uInt16> oNumOffset;          ///< Offset page number.
    sal_uInt16 nDescNameIdx;        ///< SW3-Reader: stringpool-index of style name.
    SwModify* pDefinedIn;       /**< Points to the object in which the
                                 attribute was set (CntntNode/Format). */
protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) SAL_OVERRIDE;
    virtual void SwClientNotify( const SwModify&, const SfxHint& rHint ) SAL_OVERRIDE;

public:
    SwFmtPageDesc( const SwPageDesc *pDesc = 0 );
    SwFmtPageDesc( const SwFmtPageDesc &rCpy );
    SwFmtPageDesc &operator=( const SwFmtPageDesc &rCpy );
    ~SwFmtPageDesc();

    TYPEINFO_OVERRIDE();

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;
    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

          SwPageDesc *GetPageDesc() { return (SwPageDesc*)GetRegisteredIn(); }
    const SwPageDesc *GetPageDesc() const { return (SwPageDesc*)GetRegisteredIn(); }

    ::boost::optional<sal_uInt16>  GetNumOffset() const        { return oNumOffset; }
    void    SetNumOffset( ::boost::optional<sal_uInt16> oNum ) { oNumOffset = oNum; }

    /// Query / set where attribute is anchored.
    inline const SwModify* GetDefinedIn() const { return pDefinedIn; }
    void ChgDefinedIn( const SwModify* pNew ) { pDefinedIn = (SwModify*)pNew; }
    void RegisterToEndNotInfo( SwEndNoteInfo& );
    void RegisterToPageDesc( SwPageDesc& );
    bool KnowsPageDesc() const;
};

inline const SwFmtPageDesc &SwAttrSet::GetPageDesc(sal_Bool bInP) const
    { return (const SwFmtPageDesc&)Get( RES_PAGEDESC,bInP); }

inline const SwFmtPageDesc &SwFmt::GetPageDesc(sal_Bool bInP) const
    { return aSet.GetPageDesc(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
