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
#include <SparklineAttributes.hxx>
#include <memory>

class ScDocShell;

namespace sc
{
class SparklineGroup;

/** Changes the sparkline's group attributes to the input one. */
class ChangeSparklineGroupAttributesOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    std::shared_ptr<SparklineGroup> mpExistingSparklineGroup;
    SparklineAttributes maNewAttributes;

    bool runImplementation() override;

public:
    ChangeSparklineGroupAttributesOperation(
        ScDocShell& rDocShell, std::shared_ptr<SparklineGroup> const& pExistingSparklineGroup,
        SparklineAttributes const& rNewAttributes);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
