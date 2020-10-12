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
#ifndef INCLUDED_TOOLS_LONG_HXX
#define INCLUDED_TOOLS_LONG_HXX

/**
 * We have an unfortunate problem in that, on Windows (both 32 and 64-bit), long is always 32-bit.
 * But on Linux (64-bit) long is 64-bit. Which leads to nasty situations where things that work
 * on Linux, fail on Windows.
 * So.....,
 * (1) introduce a typedef that will replace all usage of long in our codebase.
 * (2) to start with, that typedef acts that same as the long it replaces
 * (3) once the replacement is complete, this typedef will, on windows, become 64-bit.
 * (4) and once the fallout from (3) is fixed, we can start replacing tools::Long with things
 *     like sal_Int64.
 */
namespace tools
{
#if defined _WIN32
//typedef long long Long;
//typedef unsigned long long ULong;
typedef long Long;
typedef unsigned long ULong;
#else
typedef long Long;
typedef unsigned long ULong;
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
