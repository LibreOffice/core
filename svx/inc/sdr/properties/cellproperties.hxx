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
#pragma once

#include "textproperties.hxx"
#include <svx/itextprovider.hxx>
#include <rtl/ref.hxx>

namespace sdr::table
{
class Cell;
typedef rtl::Reference<sdr::table::Cell> CellRef;
}

namespace sdr::properties
{
class CellTextProvider final : public svx::ITextProvider
{
public:
    explicit CellTextProvider(sdr::table::CellRef xCell);
    ~CellTextProvider();

private:
    virtual sal_Int32 getTextCount() const override;
    virtual SdrText* getText(sal_Int32 nIndex) const override;

private:
    const sdr::table::CellRef m_xCell;
};

class CellProperties final : public TextProperties
{
protected:
    // create a new itemset
    SfxItemSet CreateObjectSpecificItemSet(SfxItemPool& rPool) override;

    const svx::ITextProvider& getTextProvider() const override;

public:
    // basic constructor
    CellProperties(SdrObject& rObj, ::sdr::table::Cell* pCell);

    // constructor for copying, but using new object
    CellProperties(const CellProperties& rProps, SdrObject& rObj, sdr::table::Cell* pCell);
    ~CellProperties();

    // Clone() operator, normally just calls the local copy constructor
    std::unique_ptr<BaseProperties> Clone(SdrObject& rObj) const override;

    void ForceDefaultAttributes() override;

    void ItemSetChanged(std::span<const SfxPoolItem* const> aChangedItems, sal_uInt16 nDeletedWhich,
                        bool bAdjustTextFrameWidthAndHeight = true) override;

    void ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem = nullptr) override;

private:
    sdr::table::CellRef mxCell;
    const CellTextProvider maTextProvider;
};

} // namespace sdr::properties

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
