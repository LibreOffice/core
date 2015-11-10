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

#ifndef INCLUDED_CODEMAKER_COMMONCPP_HXX
#define INCLUDED_CODEMAKER_COMMONCPP_HXX

#include <sal/config.h>

#include <codemaker/unotype.hxx>

namespace rtl {
    class OString;
    class OUString;
}

namespace codemaker { namespace cpp {

/** Stick a namespace scope to c++ type

    @param type
    Undecorated type

    @param ns_alias
    Use common namespace aliases instead of fully specified (nested)
    namespace. currently replaces com::sun::star with css.
 */
rtl::OString scopedCppName(rtl::OString const & type, bool ns_alias=true);

rtl::OString translateUnoToCppType(
    codemaker::UnoType::Sort sort, rtl::OUString const & nucleus);

enum IdentifierTranslationMode {
    ITM_GLOBAL,
    ITM_NONGLOBAL,
    ITM_KEYWORDSONLY
};

rtl::OString translateUnoToCppIdentifier(
    rtl::OString const & identifier, rtl::OString const & prefix,
    IdentifierTranslationMode transmode = ITM_GLOBAL,
    rtl::OString const * forbidden = nullptr);

} }

#endif // INCLUDED_CODEMAKER_COMMONCPP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
