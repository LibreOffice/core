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

#include <sal/config.h>
#include <sal/types.h>
#include "unichars.hxx"

bool ImplIsControlOrFormat(sal_uInt32 nUtf32)
{
    // All code points of
    // <http://www.unicode.org/Public/UNIDATA/UnicodeData.txt>, Version 3.1.1,
    // that have a General Category of Cc (Other, Control) or Cf (Other,
    // Format):
    return nUtf32 <= 0x001F || (nUtf32 >= 0x007F && nUtf32 <= 0x009F)
           || nUtf32 == 0x070F // SYRIAC ABBREVIATION MARK
           || nUtf32 == 0x180B // MONGOLIAN FREE VARIATION SELECTOR ONE
           || nUtf32 == 0x180C // MONGOLIAN FREE VARIATION SELECTOR TWO
           || nUtf32 == 0x180D // MONGOLIAN FREE VARIATION SELECTOR THREE
           || nUtf32 == 0x180E // MONGOLIAN VOWEL SEPARATOR
           || nUtf32 == 0x200C // ZERO WIDTH NON-JOINER
           || nUtf32 == 0x200D // ZERO WIDTH JOINER
           || nUtf32 == 0x200E // LEFT-TO-RIGHT MARK
           || nUtf32 == 0x200F // RIGHT-TO-LEFT MARK
           || nUtf32 == 0x202A // LEFT-TO-RIGHT EMBEDDING
           || nUtf32 == 0x202B // RIGHT-TO-LEFT EMBEDDING
           || nUtf32 == 0x202C // POP DIRECTIONAL FORMATTING
           || nUtf32 == 0x202D // LEFT-TO-RIGHT OVERRIDE
           || nUtf32 == 0x202E // RIGHT-TO-LEFT OVERRIDE
           || nUtf32 == 0x206A // INHIBIT SYMMETRIC SWAPPING
           || nUtf32 == 0x206B // ACTIVATE SYMMETRIC SWAPPING
           || nUtf32 == 0x206C // INHIBIT ARABIC FORM SHAPING
           || nUtf32 == 0x206D // ACTIVATE ARABIC FORM SHAPING
           || nUtf32 == 0x206E // NATIONAL DIGIT SHAPES
           || nUtf32 == 0x206F // NOMINAL DIGIT SHAPES
           || nUtf32 == 0xFEFF // ZERO WIDTH NO-BREAK SPACE
           || nUtf32 == 0xFFF9 // INTERLINEAR ANNOTATION ANCHOR
           || nUtf32 == 0xFFFA // INTERLINEAR ANNOTATION SEPARATOR
           || nUtf32 == 0xFFFB // INTERLINEAR ANNOTATION TERMINATOR
           || nUtf32 == 0x1D173 // MUSICAL SYMBOL BEGIN BEAM
           || nUtf32 == 0x1D174 // MUSICAL SYMBOL END BEAM
           || nUtf32 == 0x1D175 // MUSICAL SYMBOL BEGIN TIE
           || nUtf32 == 0x1D176 // MUSICAL SYMBOL END TIE
           || nUtf32 == 0x1D177 // MUSICAL SYMBOL BEGIN SLUR
           || nUtf32 == 0x1D178 // MUSICAL SYMBOL END SLUR
           || nUtf32 == 0x1D179 // MUSICAL SYMBOL BEGIN PHRASE
           || nUtf32 == 0x1D17A // MUSICAL SYMBOL END PHRASE
           || nUtf32 == 0xE0001 // LANGUAGE TAG
           || (nUtf32 >= 0xE0020 && nUtf32 <= 0xE007F);
}

bool ImplIsPrivateUse(sal_uInt32 nUtf32)
{
    // All code points of
    // <http://www.unicode.org/Public/UNIDATA/UnicodeData.txt>, Version 3.1.1,
    // that have a General Category of Co (Other, Private Use):
    return (nUtf32 >= 0xE000 && nUtf32 <= 0xF8FF) || (nUtf32 >= 0xF0000 && nUtf32 <= 0xFFFFD)
           || (nUtf32 >= 0x100000 && nUtf32 <= 0x10FFFD);
}

bool ImplIsZeroWidth(sal_uInt32 nUtf32)
{
    // All code points of
    // <http://www.unicode.org/Public/UNIDATA/UnicodeData.txt>, Version 3.1.1,
    // that have "ZERO WIDTH" in their Character name:
    return nUtf32 == 0x200B // ZERO WIDTH SPACE
           || nUtf32 == 0x200C // ZERO WIDTH NON-JOINER
           || nUtf32 == 0x200D // ZERO WIDTH JOINER
           || nUtf32 == 0xFEFF; // ZERO WIDTH NO-BREAK SPACE
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
