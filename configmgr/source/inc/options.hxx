/*************************************************************************
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:01 $
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

    public:
        typedef RequestOptions::Locale Locale;
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
        : m_aRequestOptions(_aOtherOptions.m_aRequestOptions)
        {
        }

        bool isForSessionUser()     const { return ! m_aRequestOptions.hasEntity(); }

        Locale getLocale()          const { return m_aRequestOptions.getLocale(); }
        Entity getUser()            const { return m_aRequestOptions.getEntity(); }

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
