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
    cout << "--- ScDPNumGroupInfo" << endl;
    cout << "  enabled: " << mbEnable << endl;
    cout << "  auto start: " << mbAutoStart << endl;
    cout << "  auto end: " << mbAutoEnd << endl;
    cout << "  start: " << mfStart << endl;
    cout << "  end: " << mfEnd << endl;
    cout << "  step: " << mfStep << endl;
    cout << "---" << endl;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
