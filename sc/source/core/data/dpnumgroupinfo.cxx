/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dpnumgroupinfo.hxx>

ScDPNumGroupInfo::ScDPNumGroupInfo() :
    mbEnable(false),
    mbDateValues(false),
    mbAutoStart(false),
    mbAutoEnd(false),
    mbIntegerOnly(true),
    mfStart(0.0), mfEnd(0.0), mfStep(0.0) {}

#if DUMP_PIVOT_TABLE

void ScDPNumGroupInfo::Dump() const
{
    std::cout << "--- ScDPNumGroupInfo" << std::endl;
    std::cout << "  enabled: " << mbEnable << std::endl;
    std::cout << "  auto start: " << mbAutoStart << std::endl;
    std::cout << "  auto end: " << mbAutoEnd << std::endl;
    std::cout << "  start: " << mfStart << std::endl;
    std::cout << "  end: " << mfEnd << std::endl;
    std::cout << "  step: " << mfStep << std::endl;
    std::cout << "---" << std::endl;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
