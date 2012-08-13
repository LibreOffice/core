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

#ifndef INCLUDED_TOOLS_APPENDUNIXSHELLWORD_HXX
#define INCLUDED_TOOLS_APPENDUNIXSHELLWORD_HXX

#include "sal/config.h"

#if defined UNX

#include "tools/toolsdllapi.h"

namespace rtl {
    class OString;
    class OStringBuffer;
}

namespace tools {

/** append arbitrary bytes as a properly quoted Unix-style shell word

    @param accumulator
        the string buffer to which the word is appended (without any surrounding
        whitespace); must not be null
    @param text
        the text to add
*/
TOOLS_DLLPUBLIC void appendUnixShellWord(
    rtl::OStringBuffer * accumulator, rtl::OString const & text);

}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
