/*************************************************************************
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-28 09:00:39 $
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

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
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

    class OOptions : public salhelper::SimpleReferenceObject
    {
        RequestOptions  m_aRequestOptions;  // current options to use
        RequestOptions  m_aDefaultOptions;  // default options used as base

        bool            m_bLazyWrite;       // true, if tree use lazy writing
        bool            m_bForceWritable;   // true, if write-protection should be ignored
    public:
        typedef RequestOptions::Locale Locale;
        typedef RequestOptions::Entity Entity;

        OOptions()
        : m_aRequestOptions()
        , m_aDefaultOptions()
        , m_bLazyWrite(true)
        , m_bForceWritable(false)
        {}

        explicit
        OOptions(const RequestOptions& _aDefaultOptions)
        : m_aRequestOptions(_aDefaultOptions, true)
        , m_aDefaultOptions(_aDefaultOptions)
        , m_bLazyWrite(true)
        , m_bForceWritable(false)
        {
        }

        OOptions(const OOptions& _rOptions)
        : m_aRequestOptions(_rOptions.m_aRequestOptions, true)
        , m_aDefaultOptions(_rOptions.m_aDefaultOptions)
        , m_bLazyWrite(_rOptions.m_bLazyWrite)
        , m_bForceWritable(_rOptions.m_bForceWritable)
        {
        }

        bool isForSessionUser()     const { return ! m_aRequestOptions.hasEntity(); }
        bool canUseCache()          const { return ! m_aRequestOptions.isForcingReload(); }
        bool getLazyWrite()         const { return m_bLazyWrite; }
        bool isForcingWritable()    const { return m_bForceWritable; }

        Locale getLocale()          const { return m_aRequestOptions.getLocale(); }
        Entity getUser()            const { return m_aRequestOptions.getEntity(); }

        Locale getDefaultLocale()   const { return m_aDefaultOptions.getLocale(); }
        Entity getDefaultUser()     const { return m_aDefaultOptions.getEntity(); }

        RequestOptions const & getRequestOptions() const
        { return m_aRequestOptions; }

        void setNoCache(bool _bNoCache = true)
        { m_aRequestOptions.forceReload(_bNoCache); }

        void setUser(const Entity & _rUser)
        { m_aRequestOptions.setEntity(_rUser); }

        void setDefaultUser(const Entity & _rUser)
        {
            m_aDefaultOptions.setEntity(_rUser);
            if (!m_aRequestOptions.hasEntity())
                m_aRequestOptions.setEntity(_rUser);
        }

        void setLocale(const Locale & _rLocale)
        { m_aRequestOptions.setLocale(_rLocale); }

        void setDefaultLocale(const Locale & _rLocale)
        {
            m_aDefaultOptions.setLocale(_rLocale);
            if (!m_aRequestOptions.hasLocale())
                m_aRequestOptions.setLocale(_rLocale);
        }

        void setMultiLocaleMode()
        { m_aRequestOptions.setAllLocales(); }

        void setMultiLocaleDefault()
        {
            m_aDefaultOptions.setAllLocales();
            if (!m_aRequestOptions.hasLocale())
                m_aRequestOptions.setAllLocales();
        }

        void setLazyWrite(bool _bLazyWrite) { m_bLazyWrite = _bLazyWrite; }
        void setForceWritable(bool _bForce) { m_bForceWritable = _bForce; }
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
