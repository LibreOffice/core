/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_VCLMAIN_HXX
#define INCLUDED_VCL_VCLMAIN_HXX

#include "sal/config.h"

namespace vclmain {

// Function called from vclmain's implementation of main, needs to be
// implemented by the application and needs to instantiate a (static, derived)
// instance of Application (where the Application constructor takes care to link
// that instance to ImplSVData.mpApp):
void createApplication();

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
