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



/** Defines possible text directions in frames. */
enum SvxFrameDirection
{
    /** Horizontal, from left to right, from top to bottom
        (typical for western languages). */
    FRMDIR_HORI_LEFT_TOP,

    /** Horizontal, from right to left, from top to bottom
        (typical for ararbic/hebrew languages). */
    FRMDIR_HORI_RIGHT_TOP,

    /** Vertical, from top to bottom, from right to left
        (typical for asian languages). */
    FRMDIR_VERT_TOP_RIGHT,

    /** Vertical, from top to bottom, from left to right
        (typical for mongol language). */
    FRMDIR_VERT_TOP_LEFT,

    /** Use the value from the environment, can only be used in frames. */
    FRMDIR_ENVIRONMENT
};



#endif // INCLUDED_EDITENG_FRMDIR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
