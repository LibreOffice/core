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

#include "docuno.hxx"
#include "docoptio.hxx"

enum class ScPropUnoId
{
    CalcAsShown      = 1,
    DefTabStop       = 2,
    IgnoreCase       = 3,
    IterEnabled      = 4,
    IterCount        = 5,
    IterEpsilon      = 6,
    LookupLabels     = 7,
    MatchWhole       = 8,
    NullDate         = 9,
    StandardDec      = 10,
    RegexEnabled     = 11,
    WildcardsEnabled = 12
};
class ScDocOptionsHelper
{
public:
    static bool setPropertyValue( ScDocOptions& rOptions,
                                    const SfxItemPropertyMap& rPropMap,
                                    std::u16string_view aPropertyName,
                                    const css::uno::Any& aValue );
    static css::uno::Any getPropertyValue(
                                    const ScDocOptions& rOptions,
                                    const SfxItemPropertyMap& rPropMap,
                                    std::u16string_view PropertyName );
};

//  empty doc object to supply only doc options

class ScDocOptionsObj final : public ScModelObj
{
private:
    ScDocOptions    aOptions;

public:
                            ScDocOptionsObj( const ScDocOptions& rOpt );
    virtual                 ~ScDocOptionsObj() override;

    // get/setPropertyValue override to used stored options instead of document

    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
