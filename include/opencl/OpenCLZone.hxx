/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OPENCL_INC_OPENCL_ZONE_HXX
#define INCLUDED_OPENCL_INC_OPENCL_ZONE_HXX

#include <opencl/opencldllapi.h>

// FIXME: post back-port, templatize me and share with OpenGLZone.
class OPENCL_DLLPUBLIC OpenCLZone
{
    /// how many times have we entered a CL zone
    static volatile sal_uInt64 gnEnterCount;
    /// how many times have we left a new CL zone
    static volatile sal_uInt64 gnLeaveCount;
    static volatile bool gbInInitialTest;

public:
    OpenCLZone()
    {
        gnEnterCount++;
    }

    ~OpenCLZone()
    {
        gnLeaveCount++;
        if (!isInZone())
            gbInInitialTest = false;
    }

    static bool isInZone()
    {
        return gnEnterCount != gnLeaveCount;
    }

    static bool isInInitialTest()
    {
        return gbInInitialTest;
    }

    static void hardDisable();
    static void enterInitialTest();
};

#endif // INCLUDED_OPENCL_INC_OPENCL_ZONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
