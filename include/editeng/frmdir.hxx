/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_EDITENG_FRMDIR_HXX
#define INCLUDED_EDITENG_FRMDIR_HXX

#include <com/sun/star/text/WritingMode2.hpp>

/**
 * Defines possible text directions in frames.
 * A scoped enum over the constants in css::text:WritingMode2.
 */
enum class SvxFrameDirection
{
    /** used as an error return value in SW */
    Unknown = -1,

    /** Horizontal, from left to right, from top to bottom
        (typical for western languages). */
    Horizontal_LR_TB = css::text::WritingMode2::LR_TB,

    /** Horizontal, from right to left, from top to bottom
        (typical for arabic/hebrew languages). */
    Horizontal_RL_TB = css::text::WritingMode2::RL_TB,

    /** Vertical, from top to bottom, from right to left
        (typical for asian languages). */
    Vertical_RL_TB = css::text::WritingMode2::TB_RL,

    /** Vertical, from top to bottom, from left to right
        (typical for mongol language). */
    Vertical_LR_TB = css::text::WritingMode2::TB_LR,

    /** Use the value from the environment, can only be used in frames. */
    Environment = css::text::WritingMode2::CONTEXT,

    /** Vertical, from bottom to top, from left to right. */
    Vertical_LR_BT = css::text::WritingMode2::BT_LR,
};

const char* getFrmDirResId(size_t nIndex);

#endif // INCLUDED_EDITENG_FRMDIR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
