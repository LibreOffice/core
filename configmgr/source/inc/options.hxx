/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:22:26 $
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

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#define CONFIGMGR_MISC_OPTIONS_HXX_

#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#include "requestoptions.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

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

    class OOptions : public configmgr::SimpleReferenceObject
    {
        RequestOptions  m_aRequestOptions;  // current options to use

    public:
        typedef RequestOptions::Locale Locale;
        typedef RequestOptions::LocaleString LocaleString;
        typedef RequestOptions::Entity Entity;

        OOptions()
        : m_aRequestOptions()
        {}

        explicit
        OOptions(const RequestOptions& _aDefaultOptions)
        : m_aRequestOptions(_aDefaultOptions)
        {
        }

        OOptions(const OOptions& _aOtherOptions)
        : configmgr::SimpleReferenceObject()
        , m_aRequestOptions(_aOtherOptions.m_aRequestOptions)
        {
        }

        bool isForSessionUser()     const { return ! m_aRequestOptions.hasEntity(); }

        LocaleString    getLocale() const { return m_aRequestOptions.getLocale(); }
        Entity          getUser()   const { return m_aRequestOptions.getEntity(); }

        RequestOptions const & getRequestOptions() const
        { return m_aRequestOptions; }

        void setUser(const Entity & _rUser)
        { m_aRequestOptions.setEntity(_rUser); }

        void setLocale(const Locale & _rLocale)
        { m_aRequestOptions.setLocale(_rLocale); }

        void setMultiLocaleMode()
        { m_aRequestOptions.setAllLocales(); }

        void enableAsync(bool _bEnable)
        { m_aRequestOptions.enableAsync(_bEnable); }
    };
    typedef vos::ORef<OOptions> OptionsRef;

    struct ltOptions
    {
        lessRequestOptions ltData;
        bool operator()(OptionsRef const &o1, OptionsRef const &o2) const
        {
            return ltData(o1->getRequestOptions(),o2->getRequestOptions());
        }
    };
} // namespace

#endif
