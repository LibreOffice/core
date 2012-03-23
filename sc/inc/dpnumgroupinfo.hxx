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
