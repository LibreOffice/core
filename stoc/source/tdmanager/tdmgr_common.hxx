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

#ifndef _STOC_TDMGR_COMMON_HXX
#define _STOC_TDMGR_COMMON_HXX

#include <rtl/unload.h>

#include "com/sun/star/reflection/XTypeDescription.hpp"

#define ARLEN(x) (sizeof (x) / sizeof *(x))


namespace stoc_tdmgr
{
    extern rtl_StandardModuleCount g_moduleCount;

struct IncompatibleTypeException
{
    OUString m_cause;
    IncompatibleTypeException( OUString const & cause )
        : m_cause( cause ) {}
};

void check(
    css::uno::Reference<css::reflection::XTypeDescription> const & xNewTD,
    css::uno::Reference<css::reflection::XTypeDescription> const & xExistingTD,
    OUString const & context = OUString() );
/* throw (css::uno::RuntimeException, IncompatibleTypeException) */

} // namespace stoc_tdmgr

#endif /* _STOC_TDMGR_COMMON_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
