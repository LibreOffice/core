/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

/** Rendering test result.
 *
 * Test either "Passed", "Failed" or "PassedWithQuirks" which means
 * the test passed but at least one rendering quirk was detected.
 */
#pragma once

namespace vcl::test
{
enum class TestResult
{
    Failed,
    PassedWithQuirks,
    Passed
};
}
