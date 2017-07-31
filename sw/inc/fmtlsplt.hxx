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
#ifndef INCLUDED_SW_INC_FMTLSPLT_HXX
#define INCLUDED_SW_INC_FMTLSPLT_HXX

#include <svl/eitem.hxx>
#include <hintids.hxx>
#include <format.hxx>
#include "swdllapi.h"

class IntlWrapper;

class SW_DLLPUBLIC SwFormatLayoutSplit : public SfxBoolItem
{
public:
    SwFormatLayoutSplit( bool bSplit = true ) : SfxBoolItem( RES_LAYOUT_SPLIT, bSplit ) {}

    /// "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
};

inline const SwFormatLayoutSplit &SwAttrSet::GetLayoutSplit(bool bInP) const
    { return static_cast<const SwFormatLayoutSplit&>(Get( RES_LAYOUT_SPLIT,bInP)); }

inline const SwFormatLayoutSplit &SwFormat::GetLayoutSplit(bool bInP) const
    { return m_aSet.GetLayoutSplit(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
