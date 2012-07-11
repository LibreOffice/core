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

#ifndef INCLUDED_WARNINGS_GUARD_BOOST_SIGNALS2_HPP
#define INCLUDED_WARNINGS_GUARD_BOOST_SIGNALS2_HPP

// Because the GCC system_header mechanism doesn't work in .c/.cxx compilation
// units and more important affects the rest of the current include file, the
// warnings guard is separated into this header file on its own.

// boost 1.44 boost/signals2.hpp with gcc 4.6.3 causes
// solver/unxlngx6/inc/boost/signals2/detail/slot_template.hpp:122:7: error: 
// declaration of ‘slot’ shadows a member of 'this' [-Werror=shadow]
// We want to minimize the patches to external headers, so the warnings are
// disabled here instead of in the header file itself.
#ifdef _MSC_VER
#pragma warning(push, 1)
#elif defined __GNUC__
#pragma GCC system_header
#endif
#include <boost/signals2.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // INCLUDED_WARNINGS_GUARD_BOOST_SIGNALS2_HPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
