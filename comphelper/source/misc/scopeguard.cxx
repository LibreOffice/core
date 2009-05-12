/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scopeguard.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper/scopeguard.hxx"
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
                                           "UNO exception occured: ") ) +
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            }
            catch (...) {
                OSL_ENSURE( false, "unknown exception occured!" );
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

} // namespace comphelper

