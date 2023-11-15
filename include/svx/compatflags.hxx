/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// SdrCompatibilityFlags that are implemented in SdrModelImpl
enum class SdrCompatibilityFlag
{
    AnchoredTextOverflowLegacy, ///< for tdf#99729
    LegacyFontwork, ///< for tdf#148000 false == Fontwork works in PowerPoint compat mode
    ConnectorUseSnapRect, ///< for tdf#149756
    IgnoreBreakAfterMultilineField, ///< for tdf#148966
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
