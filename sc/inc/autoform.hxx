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

#include <sal/types.h>
#include <svx/TableAutoFmt.hxx>
#include <svx/TableStylesParser.hxx>
#include "scdllapi.h"

#include <array>
#include <memory>
#include "document.hxx"

class ScAutoFormatDataField : public SvxAutoFormatDataField
{
public:
    ScAutoFormatDataField();
    ScAutoFormatDataField(const ScAutoFormatDataField& rCopy);
    ScAutoFormatDataField(const SvxAutoFormatDataField& rCopy);
    ~ScAutoFormatDataField();

};

class SAL_DLLPUBLIC_RTTI ScAutoFormatData : public SvxAutoFormatData
{
private:
    std::array<std::unique_ptr<ScAutoFormatDataField>, ELEMENT_COUNT> mpDataField;

public:
    ScAutoFormatData();
    SC_DLLPUBLIC ScAutoFormatData(const ScAutoFormatData& rData);
    SC_DLLPUBLIC ScAutoFormatData(const SvxAutoFormatData& rData);

    ScAutoFormatDataField* GetField(size_t nIndex) override;
    const ScAutoFormatDataField* GetField(size_t nIndex) const override;
    bool SetField(size_t nIndex, const SvxAutoFormatDataField& aField) override;
    SvxAutoFormatData* MakeCopy() const override { return new ScAutoFormatData(*this); };
    SvxAutoFormatDataField* GetDefaultField() const override { return new ScAutoFormatDataField; };

    const SfxPoolItem* GetItem(sal_uInt8 nIndex, sal_uInt16 nWhich) const;
    template <class T> const T* GetItem(sal_uInt8 nIndex, TypedWhichId<T> nWhich) const
    {
        return static_cast<const T*>(GetItem(nIndex, sal_uInt16(nWhich)));
    }
    void PutItem(size_t nIndex, const SfxPoolItem& rItem);

    void FillToItemSet(size_t nIndex, SfxItemSet& rItemSet) const;
};

class SAL_DLLPUBLIC_RTTI ScAutoFormat : public SvxAutoFormat
{
private:
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:
    ScAutoFormat();
    ~ScAutoFormat();

    const ScAutoFormatData* GetData(size_t nIndex) const override;
    ScAutoFormatData* GetData(size_t nIndex) override;
    bool InsertAutoFormat(SvxAutoFormatData* pFormat) override;
    ScAutoFormatData* ReleaseAutoFormat(const OUString& rName) override;
    ScAutoFormatData* FindAutoFormat(const OUString& rName) const override;
    const ScAutoFormatData* GetResolvedStyle(const SvxAutoFormatData* pData) const override;
    SvxAutoFormatData* GetDefaultData() override { return new ScAutoFormatData();};
    SvxAutoFormatDataField* GetDefaultField() override { return new ScAutoFormatDataField; };
    size_t size() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
