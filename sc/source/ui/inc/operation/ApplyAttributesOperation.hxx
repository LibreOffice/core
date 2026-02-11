/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rangelst.hxx>

class ScDocShell;
class ScMarkData;
class ScPatternAttr;
class ScDocFunc;

namespace sc
{
/** Operation which applies input attributes to marked cells or range. */
class ApplyAttributesOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScMarkData const& mrMark;
    ScPatternAttr const& mrPattern;

    bool runImplementation() override;

public:
    ApplyAttributesOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScMarkData& rMark,
                             const ScPatternAttr& rPattern, bool bApi);
};

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
