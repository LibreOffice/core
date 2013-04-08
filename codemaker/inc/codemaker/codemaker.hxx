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

#ifndef INCLUDED_CODEMAKER_CODEMAKER_HXX
#define INCLUDED_CODEMAKER_CODEMAKER_HXX

#include "sal/config.h"
#include "codemaker/unotype.hxx"
#include "registry/types.h"
#include "rtl/ref.hxx"
#include "sal/types.h"

#include <vector>

namespace rtl {
    class OString;
    class OUString;
}
class TypeManager;

namespace codemaker {

rtl::OString convertString(rtl::OUString const & string);

codemaker::UnoType::Sort decomposeAndResolve(
    rtl::Reference< TypeManager > const & manager, rtl::OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, rtl::OString * name, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments);

}

#endif // INCLUDED_CODEMAKER_CODEMAKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
