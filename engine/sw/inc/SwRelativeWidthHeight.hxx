/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <svx/svdobj.hxx>
#include <com/sun/star/text/RelOrientation.hpp>

class SwRelativeWidthHeight : public SdrObjUserData
{
    std::optional<double> mnRelativeWidth;
    std::optional<double> mnRelativeHeight;
    sal_Int16 meRelativeWidthRelation;
    sal_Int16 meRelativeHeightRelation;

public:
    SwRelativeWidthHeight()
        : SdrObjUserData(UserDataID::ID_SwRelativeWidthHeight)
        , meRelativeWidthRelation(css::text::RelOrientation::PAGE_FRAME)
        , meRelativeHeightRelation(css::text::RelOrientation::PAGE_FRAME)
    {
    }

    SwRelativeWidthHeight(SwRelativeWidthHeight const&) = default;
    SwRelativeWidthHeight(SwRelativeWidthHeight&&) = default;
    SwRelativeWidthHeight& operator=(SwRelativeWidthHeight const&)
        = delete; // due to SdrObjUserData
    SwRelativeWidthHeight& operator=(SwRelativeWidthHeight&&) = delete; // due to SdrObjUserData

    virtual std::unique_ptr<SdrObjUserData> Clone(SdrObject* /*pObj*/) const override
    {
        return std::unique_ptr<SdrObjUserData>(new SwRelativeWidthHeight(*this));
    }

    void SetRelativeWidth(double nValue) { mnRelativeWidth = nValue; }
    void SetRelativeWidthRelation(sal_Int16 eValue) { meRelativeWidthRelation = eValue; }
    void SetRelativeHeight(double nValue) { mnRelativeHeight = nValue; }
    void SetRelativeHeightRelation(sal_Int16 eValue) { meRelativeHeightRelation = eValue; }

    const double* GetRelativeWidth() const
    {
        if (!mnRelativeWidth)
            return nullptr;
        return &*mnRelativeWidth;
    }
    sal_Int16 GetRelativeWidthRelation() const { return meRelativeWidthRelation; }
    const double* GetRelativeHeight() const
    {
        if (!mnRelativeHeight)
            return nullptr;
        return &*mnRelativeHeight;
    }
    sal_Int16 GetRelativeHeightRelation() const { return meRelativeHeightRelation; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
