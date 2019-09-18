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

#include <sal/config.h>

#include <cassert>
#include <csignal>

#include <opencl/opencldllapi.h>

// FIXME: post back-port, templatize me and share with OpenGLZone.
class OPENCL_DLLPUBLIC OpenCLZone
{
    /// how many times have we entered a CL zone and not yet left it
    static volatile std::sig_atomic_t gnEnterCount;
    static volatile bool gbInInitialTest;

public:
    OpenCLZone()
    {
        gnEnterCount = gnEnterCount + 1; //TODO: overflow
    }

    ~OpenCLZone()
    {
        assert(gnEnterCount > 0);
        gnEnterCount = gnEnterCount - 1;
        if (!isInZone())
            gbInInitialTest = false;
    }

    static bool isInZone()
    {
        return gnEnterCount > 0;
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
