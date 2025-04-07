/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

/// Decides how new redline flags affect the views
enum class SfxRedlineRecordingMode
{
    /// Model and current view only: to set new flags & restore old ones later
    ViewAgnostic,
    /// Have recording on in this view
    ThisView,
    /// Have recording on in all views
    AllViews,
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
