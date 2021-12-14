/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/eitem.hxx>
#include "hintids.hxx"
#include "format.hxx"
#include "swdllapi.h"

class IntlWrapper;

class SW_DLLPUBLIC SwFormatLayoutInCell final : public SfxBoolItem
{
public:
    SwFormatLayoutInCell(bool bFlag = false)
        : SfxBoolItem(RES_LAYOUT_IN_CELL, bFlag)
    {
    }

    virtual SwFormatLayoutInCell* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper& rIntl) const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwFormatLayoutInCell& SwAttrSet::GetLayoutInCell(bool bInP) const
{
    return Get(RES_LAYOUT_IN_CELL, bInP);
};

inline const SwFormatLayoutInCell& SwFormat::GetLayoutInCell(bool bInP) const
{
    return m_aSet.GetLayoutInCell(bInP);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
