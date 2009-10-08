/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: requestoptions.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#define CONFIGMGR_MISC_REQUESTOPTIONS_HXX_

#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustring.hxx>

namespace configmgr
{
// ---------------------------------------------------------------------------

    /**
        Options which can be used to modify a request for data
    */
    class RequestOptions
    {
    public:
        /// Default constructor. Sets options to use defaults.
        RequestOptions()
        : m_sLocale()
        , m_sEntity()
        , m_bEnableAsync(true)
        , m_bReload(false)
        {}

        /// @returns <TRUE/>, if data can be written asynchronously
        bool isAsyncEnabled() const { return m_bEnableAsync; }
        /// @returns <TRUE/>, if data is reloaded into cache
        bool isRefreshEnabled() const { return m_bReload; }
        /** @returns
                <TRUE/>,  if a locale is specified, <BR/>
                <FALSE/>, if the default locale should be used
        */
        bool    hasLocale() const { return m_sLocale.Language.getLength() != 0; }
        /// @returns the locale to get data for
        bool    isForAllLocales() const;
        /// @returns the locale to get data for - compatibilty version
        rtl::OUString  getLocale() const { return getIsoLocale(); }
        /// @returns the locale to get data for
        rtl::OUString  getIsoLocale() const;
        /// @returns the locale to get data for
        com::sun::star::lang::Locale const & getUnoLocale() const { return m_sLocale; }

        /** @returns
            <TRUE/>,  if an entity is specified, <BR/>
            <FALSE/>, if data of the session user is requested
        */
        bool    hasEntity() const { return m_sEntity.getLength() != 0; }
        /// @returns the entity to get data for
        rtl::OUString  getEntity() const { return m_sEntity; }

        /// sets the entity to get data for to the given entity
        void setEntity(rtl::OUString const & _sEntity) { m_sEntity = _sEntity; }
        /// resets the entity to get data for to be the session user
        void clearEntity()   { m_sEntity = rtl::OUString(); }

        /// sets the locale so data is gotten for all locales
        void setAllLocales();
        /// sets the locale to get data for to the given locale
        void setLocale(com::sun::star::lang::Locale const & _aLocale) { m_sLocale = _aLocale; }
        /// sets the locale to get data for to the given locale
        void setIsoLocale(rtl::OUString const & _sLocale);
        /// sets a fallback locale, if no locale is set yet
        void ensureLocaleSet();

        /// marks asyncronous access a enabled or disabled
        void enableAsync(bool _bEnable = true)  { m_bEnableAsync = _bEnable; }
        /// enforce a refresh to cache
        void forceRefresh(bool _bEnable = true)  { m_bReload = _bEnable; }
    // comparison/container helpers
        /// function that defines a weak strict ordering on RequestOptions
        friend sal_Int32 compareRequestOptions(RequestOptions const& lhs, RequestOptions const& rhs);
    private:
        com::sun::star::lang::Locale        m_sLocale;                              /// locale to fetch data for
        rtl::OUString       m_sEntity;                              /// user/group/role to fetch data for
        bool        m_bEnableAsync;                         /// true, if data may be
        bool        m_bReload;                              /// reload into cache from backend
    };

// ---------------------------------------------------------------------------
    struct lessRequestOptions
    {
        bool operator()(RequestOptions const & lhs, RequestOptions const & rhs) const
        { return compareRequestOptions(lhs,rhs) < 0; }
    };
// ---------------------------------------------------------------------------
    struct equalRequestOptions
    {
        bool operator()(RequestOptions const & lhs, RequestOptions const & rhs) const
        { return compareRequestOptions(lhs,rhs) == 0; }
    };
// ---------------------------------------------------------------------------
} // namespace

#endif
