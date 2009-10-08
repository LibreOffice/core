/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: options.hxx,v $
 * $Revision: 1.22 $
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

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#define CONFIGMGR_MISC_OPTIONS_HXX_

#include "requestoptions.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "utility.hxx"
#include <salhelper/simplereferenceobject.hxx>
#include <vos/ref.hxx>

namespace configmgr
{
    namespace css  = ::com::sun::star;

    /**
       class OOptions is created one time per Configuration[update]Access
       all important options should stored in this class.
       The object will be forwarded to all other objects so we only
       need to extend this classobject and all other class can work with
       the new options or important options etc.
    */

    class OOptions : public salhelper::SimpleReferenceObject
    {
        RequestOptions  m_aRequestOptions;  // current options to use

    public:
        OOptions()
        : m_aRequestOptions()
        {}

        explicit
        OOptions(const RequestOptions& _aDefaultOptions)
        : m_aRequestOptions(_aDefaultOptions)
        {
        }

        OOptions(const OOptions& _aOtherOptions)
        : SimpleReferenceObject()
        , m_aRequestOptions(_aOtherOptions.m_aRequestOptions)
        {
        }

        bool isForSessionUser()     const { return ! m_aRequestOptions.hasEntity(); }

        rtl::OUString    getLocale() const { return m_aRequestOptions.getLocale(); }
        rtl::OUString          getUser()   const { return m_aRequestOptions.getEntity(); }

        RequestOptions const & getRequestOptions() const
        { return m_aRequestOptions; }

        void setUser(const rtl::OUString & _rUser)
        { m_aRequestOptions.setEntity(_rUser); }

        void setLocale(const com::sun::star::lang::Locale & _rLocale)
        { m_aRequestOptions.setLocale(_rLocale); }

        void setMultiLocaleMode()
        { m_aRequestOptions.setAllLocales(); }

        void enableAsync(bool _bEnable)
        { m_aRequestOptions.enableAsync(_bEnable); }
    };

    struct ltOptions
    {
        lessRequestOptions ltData;
        bool operator()(vos::ORef<OOptions> const &o1, vos::ORef<OOptions> const &o2) const
        {
            return ltData(o1->getRequestOptions(),o2->getRequestOptions());
        }
    };
} // namespace

#endif
