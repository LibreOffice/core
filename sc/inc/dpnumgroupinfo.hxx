/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_DPNUMGROUPINFO_HXX__
#define __SC_DPNUMGROUPINFO_HXX__

#include "scdllapi.h"
#include "dpmacros.hxx"

struct ScDPNumGroupInfo
{
    bool mbEnable:1;
    bool mbDateValues:1;
    bool mbAutoStart:1;
    bool mbAutoEnd:1;
    bool mbIntegerOnly:1;
    double mfStart;
    double mfEnd;
    double mfStep;

    SC_DLLPUBLIC ScDPNumGroupInfo();
    SC_DLLPUBLIC ScDPNumGroupInfo(const ScDPNumGroupInfo& r);

#if DEBUG_PIVOT_TABLE
    void Dump() const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
