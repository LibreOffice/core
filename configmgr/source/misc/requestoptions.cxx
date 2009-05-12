/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: requestoptions.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_configmgr.hxx"

#include "requestoptions.hxx"
#include "matchlocale.hxx"
#include "tracer.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
// ---------------------------------------------------------------------------

    rtl::OUString RequestOptions::getIsoLocale() const
    {
        return localehelper::makeIsoLocale( m_sLocale );
    }
// ---------------------------------------------------------------------------

    bool RequestOptions::isForAllLocales() const
    {
        return localehelper::designatesAllLocales( m_sLocale );
    }
// ---------------------------------------------------------------------------

    void RequestOptions::setIsoLocale(rtl::OUString const & _sLocale)
    {
        setLocale( localehelper::makeLocale( _sLocale ) );
    }
// ---------------------------------------------------------------------------

    void RequestOptions::setAllLocales()
    {
        m_sLocale = localehelper::getAnyLocale();
    }
// ---------------------------------------------------------------------------

    void RequestOptions::ensureLocaleSet()
    {
        if (!hasLocale())
            m_sLocale = localehelper::getDefaultLocale();
    }
// ---------------------------------------------------------------------------

    static inline
    sal_Int32 hashRequestLocale(com::sun::star::lang::Locale const & aLocale)
    {
        return aLocale.Language.hashCode() ^ aLocale.Country.hashCode();
    }
// ---------------------------------------------------------------------------

    static inline
    sal_Int32 compareRequestLocale(com::sun::star::lang::Locale const& lhs, com::sun::star::lang::Locale const& rhs)
    {
        sal_Int32 nDiff = lhs.Language.compareTo(rhs.Language);
        if (nDiff == 0)
        {
            nDiff = lhs.Country.compareTo(rhs.Country);
        }

        return nDiff;
    }
// ---------------------------------------------------------------------------

    sal_Int32 compareRequestOptions(RequestOptions const& lhs, RequestOptions const& rhs)
    {
        sal_Int32 nDiff = lhs.getEntity().compareTo(rhs.getEntity());
        if (nDiff == 0)
        {
            nDiff = compareRequestLocale(lhs.getUnoLocale(),rhs.getUnoLocale());
        }

        return nDiff;
    }
// ---------------------------------------------------------------------------
} // namespace config


