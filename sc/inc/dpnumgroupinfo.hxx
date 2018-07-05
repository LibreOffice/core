/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_DPNUMGROUPINFO_HXX
#define INCLUDED_SC_INC_DPNUMGROUPINFO_HXX

#include "scdllapi.h"
#include "calcmacros.hxx"

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

#if DUMP_PIVOT_TABLE
    void Dump() const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
