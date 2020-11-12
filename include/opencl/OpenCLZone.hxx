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

#include <opencl/opencldllapi.h>

#include <comphelper/crashzone.hxx>

class OPENCL_DLLPUBLIC OpenCLZone : public CrashZone<OpenCLZone>
{
public:
    static void hardDisable();
};

// Used during initial testing of OpenCL.
class OPENCL_DLLPUBLIC OpenCLInitialZone : public CrashZone<OpenCLInitialZone>
{
};

#endif // INCLUDED_OPENCL_INC_OPENCL_ZONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
