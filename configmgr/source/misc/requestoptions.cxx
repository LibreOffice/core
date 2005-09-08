/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: requestoptions.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:12:26 $
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

#include "requestoptions.hxx"

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------

#if 0 // not used any more
    static sal_Int32 getNextCacheID()
    {
        static oslInterlockedCount nNextID = 0;

        oslInterlockedCount nNewID = osl_incrementInterlockedCount(&nNextID);

        if (nNewID == 0)
        {
            CFG_TRACE_WARNING("Cache ID overflow - restarting sequence !");
            OSL_ENSURE(false, "Cache ID overflow - restarting sequence !");
        }

        return static_cast<sal_Int32>(nNewID);
    }
#endif
// ---------------------------------------------------------------------------

    RequestOptions RequestOptions::forAllLocales()
    {
        RequestOptions aResult;
        aResult.setAllLocales();
        return aResult;
    }

// ---------------------------------------------------------------------------

    RequestOptions::LocaleString RequestOptions::getIsoLocale() const
    {
        return localehelper::makeIsoLocale( m_sLocale );
    }
// ---------------------------------------------------------------------------

    bool RequestOptions::isForAllLocales() const
    {
        return localehelper::designatesAllLocales( m_sLocale );
    }
// ---------------------------------------------------------------------------

    void RequestOptions::setIsoLocale(LocaleString const & _sLocale)
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
    sal_Int32 hashRequestLocale(RequestOptions::Locale const & aLocale)
    {
        return aLocale.Language.hashCode() ^ aLocale.Country.hashCode();
    }
// ---------------------------------------------------------------------------

    sal_Int32 RequestOptions::hashCode() const
    {
        return hashRequestLocale(m_sLocale) ^ m_sEntity.hashCode();
    }
// ---------------------------------------------------------------------------

    static inline
    sal_Int32 compareRequestLocale(RequestOptions::Locale const& lhs, RequestOptions::Locale const& rhs)
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


