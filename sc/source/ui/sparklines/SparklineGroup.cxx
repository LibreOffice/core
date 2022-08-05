/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <SparklineGroup.hxx>
#include <utility>

namespace sc
{
SparklineGroup::SparklineGroup(SparklineAttributes aSparklineAttributes)
    : m_aAttributes(std::move(aSparklineAttributes))
    , m_aGUID(tools::Guid::Generate)
{
}

SparklineGroup::SparklineGroup()
    : m_aGUID(tools::Guid::Generate)
{
}

SparklineGroup::SparklineGroup(SparklineGroup const& pOtherSparkline)
    : m_aAttributes(pOtherSparkline.m_aAttributes)
    , m_aGUID(pOtherSparkline.m_aGUID)
{
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
