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

#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <comphelper/componentmodule.hxx>

#include <vector>

namespace comphelper
{

    using namespace ::cppu;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;

    /** implementation for <type>OModule</type>. not threadsafe, has to be guarded by its owner
    */
    class OModuleImpl
    {
    public:
        OModuleImpl();
    };

    OModuleImpl::OModuleImpl()
    {
    }

    OModule::OModule()
        : m_pImpl(new OModuleImpl)
    {
    }

    OModule::~OModule()
    {
    }

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
