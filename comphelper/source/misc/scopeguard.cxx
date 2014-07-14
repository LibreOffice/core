/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

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
                OSL_ENSURE(
                    false, rtl::OUStringToOString(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                           "UNO exception occurred: ") ) +
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            }
            catch (...) {
                OSL_ENSURE( false, "unknown exception occurred!" );
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

