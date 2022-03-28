/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "scdllapi.h"
#include <tools/color.hxx>
#include <optional>
#include <SparklineAttributes.hxx>
#include <tools/Guid.hxx>

namespace sc
{
class SC_DLLPUBLIC SparklineGroup
{
private:
    SparklineAttributes m_aAttributes;
    tools::Guid m_aGUID;

public:
    SparklineAttributes& getAttributes() { return m_aAttributes; }
    SparklineAttributes const& getAttributes() const { return m_aAttributes; }

    tools::Guid& getID() { return m_aGUID; }

    void setID(tools::Guid const& rGuid) { m_aGUID = rGuid; }

    SparklineGroup();

    SparklineGroup(SparklineGroup const& pOtherSparkline);

    SparklineGroup& operator=(const SparklineGroup&) = delete;
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
