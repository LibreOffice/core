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


#include "comphelper/flagguard.hxx"
#include "osl/diagnose.h"
#include "com/sun/star/uno/Exception.hpp"

namespace comphelper {

ScopeGuard::~ScopeGuard()
{
    if (m_func)
    {
        if (m_excHandling == IGNORE_EXCEPTIONS)
        {
            try {
                m_func();
            }
            catch (com::sun::star::uno::Exception & exc) {
                (void) exc; // avoid warning about unused variable
                OSL_FAIL(
                    OUStringToOString( "UNO exception occurred: " +
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            }
            catch (...) {
                OSL_FAIL( "unknown exception occurred!" );
            }
        }
        else
        {
            m_func();
        }
    }
}

void ScopeGuard::dismiss()
{
    m_func.clear();
}

FlagGuard::~FlagGuard()
{
}

FlagRestorationGuard::~FlagRestorationGuard()
{
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
