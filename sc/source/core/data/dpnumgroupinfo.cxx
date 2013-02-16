/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "dpnumgroupinfo.hxx"

ScDPNumGroupInfo::ScDPNumGroupInfo() :
    mbEnable(false),
    mbDateValues(false),
    mbAutoStart(false),
    mbAutoEnd(false),
    mbIntegerOnly(true),
    mfStart(0.0), mfEnd(0.0), mfStep(0.0) {}

ScDPNumGroupInfo::ScDPNumGroupInfo(const ScDPNumGroupInfo& r) :
    mbEnable(r.mbEnable),
    mbDateValues(r.mbDateValues),
    mbAutoStart(r.mbAutoStart),
    mbAutoEnd(r.mbAutoEnd),
    mbIntegerOnly(r.mbIntegerOnly),
    mfStart(r.mfStart),
    mfEnd(r.mfEnd), mfStep(r.mfStep) {}

#if DEBUG_PIVOT_TABLE
using std::cout;
using std::endl;

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
