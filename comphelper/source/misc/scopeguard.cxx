/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scopeguard.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 22:49:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

