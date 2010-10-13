/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_framework.hxx"

#include "services/uriabbreviation.hxx"
#include "services.h"

#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"

#include "tools/urlobj.hxx"

// component helper namespace
namespace css = ::com::sun::star;

// framework namespace
namespace framework
{

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_MULTISERVICE_2 (    UriAbbreviation                                                         ,
                                        ::cppu::OWeakObject                                                     ,
                                        SERVICENAME_STRINGABBREVIATION                                          ,
                                        IMPLEMENTATIONNAME_URIABBREVIATION
                                    )

DEFINE_INIT_SERVICE                 (   UriAbbreviation,
                                        {
                                        }
                                    )

UriAbbreviation::UriAbbreviation(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
}

// ::com::sun::star::util::XStringAbbreviation:
::rtl::OUString SAL_CALL UriAbbreviation::abbreviateString(const css::uno::Reference< css::util::XStringWidth > & xStringWidth, ::sal_Int32 nWidth, const ::rtl::OUString & aString) throw (css::uno::RuntimeException)
{
    ::rtl::OUString aResult( aString );
    if ( xStringWidth.is() )
    {
        // Use INetURLObject to abbreviate URLs
        INetURLObject aURL( aString );
        aResult = aURL.getAbbreviated( xStringWidth, nWidth, INetURLObject::DECODE_UNAMBIGUOUS );
    }

    return aResult;
}

} // namespace framework





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
