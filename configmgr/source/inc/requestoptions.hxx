/*************************************************************************
 *
 *  $RCSfile: requestoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-15 11:40:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#define CONFIGMGR_MISC_REQUESTOPTIONS_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace configmgr
{
// ---------------------------------------------------------------------------

    /**
        Options which can be used to modify a request for data
    */
    class RequestOptions
    {
    public:
        typedef rtl::OUString Locale;
        typedef rtl::OUString Entity;
    public:
        // create Request options for all locales
        static RequestOptions forAllLocales();

        /// Default constructor. Sets options to use defaults.
        RequestOptions()
        : m_sLocale()
        , m_sEntity()
        , m_nCacheID(0)
        {}

        /// Copy constructor. Copies all options (optionally copying or renewing the cache ID)
        RequestOptions(const RequestOptions& _aOther, bool _bRenewCacheID = false)
        : m_sLocale(_aOther.m_sLocale)
        , m_sEntity(_aOther.m_sEntity)
        {
            if (_bRenewCacheID)
            {
                m_nCacheID = 0;
                this->forceReload(_aOther.isForcingReload());
            }
            else
            {
                m_nCacheID = _aOther.m_nCacheID;
            }
        }

        /// @returns <TRUE/>, if data must be loaded freshly into a private cache line
        bool isForcingReload() const { return m_nCacheID != 0; }
        /// @returns an ID number identifying the private cache line to use
        sal_Int32 getCacheID() const { return m_nCacheID; }

        /** @returns
                <TRUE/>,  if a locale is specified, <BR/>
                <FALSE/>, if the default locale should be used
        */
        bool    hasLocale() const { return m_sLocale.getLength() != 0; }
        /// @returns the locale to get data for
        Locale  getLocale() const { return m_sLocale; }

        /** @returns
            <TRUE/>,  if an entity is specified, <BR/>
            <FALSE/>, if data of the session user is requested
        */
        bool    hasEntity() const { return m_sEntity.getLength() != 0; }
        /// @returns the entity to get data for
        Entity  getEntity() const { return m_sEntity; }

        /// sets the entity to get data for to the given entity
        void setEntity(Entity const & _sEntity) { m_sEntity = _sEntity; }
        /// resets the entity to get data for to be the session user
        void clearEntity()   { m_sEntity = rtl::OUString(); }

        /// sets the locale so data is gotten for all locales
        void setAllLocales();
        /// sets the locale to get data for to the given locale
        void setLocale(Locale const & _sLocale) { m_sLocale = _sLocale; }
        /// resets the locale to get data for to use the default
        void clearLocale()   { m_sLocale = rtl::OUString(); }

        /// forces data to be freshly loaded into a new private cache line
        void forceReload(bool _bNoCache = true);

    // comparison/container helpers
        /// return a hash code for this object
        sal_Int32 hashCode() const;
        /// function that defines a weak strict ordering on RequestOptions
        friend sal_Int32 compareRequestOptions(RequestOptions const& lhs, RequestOptions const& rhs);
    private:
        Locale      m_sLocale;                              /// locale to fetch data for
        Entity      m_sEntity;                              /// user/group/role to fetch data for
        sal_Int32   m_nCacheID;                             /// nonzero, if data should not be fetched from the cache, but reloaded
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
    struct hashRequestOptions
    {
        sal_Int32 operator()(RequestOptions const & _opt) const
        { return _opt.hashCode(); }
    };
// ---------------------------------------------------------------------------
} // namespace

#endif
