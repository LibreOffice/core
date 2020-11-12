/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SETUPVCL_HXX
#define INCLUDED_TEST_SETUPVCL_HXX

#include <sal/config.h>

#include <test/testdllapi.hxx>

namespace test
{
// Calls InitVCL etc.; needed from multiple places in the test infrastructure:
OOO_DLLPUBLIC_TEST_SETUPVCL void setUpVcl(bool forceHeadless = false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
