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
#ifndef INCLUDED_SW_INC_FMTFOLLOWTEXTFLOW_HXX
#define INCLUDED_SW_INC_FMTFOLLOWTEXTFLOW_HXX

#include <svl/eitem.hxx>
#include "hintids.hxx"
#include "format.hxx"
#include "swdllapi.h"

class IntlWrapper;

class SW_DLLPUBLIC SwFormatFollowTextFlow : public SfxBoolItem
{
private:
    bool mbLayoutInCell = false;

public:

    SwFormatFollowTextFlow( bool bFlag = false )
        : SfxBoolItem( RES_FOLLOW_TEXT_FLOW, bFlag )
        {}

    SwFormatFollowTextFlow( bool bFlag, bool _bLayoutInCell  )
        : SfxBoolItem( RES_FOLLOW_TEXT_FLOW, bFlag )
        , mbLayoutInCell( _bLayoutInCell )
        {}


    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;

    bool GetLayoutInCell() const { return mbLayoutInCell; }


    bool PutValue(const css::uno::Any& rVal, sal_uInt8 aInt) override;

    bool QueryValue(css::uno::Any& rVal, sal_uInt8 aInt = 0) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

    bool operator==(const SfxPoolItem& rItem) const override;
};

inline const SwFormatFollowTextFlow &SwAttrSet::GetFollowTextFlow(bool bInP) const
    { return Get( RES_FOLLOW_TEXT_FLOW, bInP ); }

inline const SwFormatFollowTextFlow &SwFormat::GetFollowTextFlow(bool bInP) const
    { return m_aSet.GetFollowTextFlow( bInP ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
