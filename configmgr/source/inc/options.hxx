/*************************************************************************
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dg $ $Date: 2000-12-04 19:47:18 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "hashhelper.hxx"
#include <vos/ref.hxx>

namespace com { namespace sun { namespace star {

    namespace script    { class XTypeConverter; }
}   }   }

namespace configmgr
{
    namespace css  = ::com::sun::star;
    namespace uno  = css::uno;
    namespace script = css::script;

    /**
       class OOptions is created one time per Configuration[update]Access
       all important options should stored in this class.
       The object will be [weiterleiten] to all other objects so we only
       need to extend this classobject and all other class can work with
       the new options or important options etc.
    */

    class OOptions : public vos::OReference
    {
        uno::Reference< script::XTypeConverter > m_xConverter;  // typeconverter used
        rtl::OUString   m_sLocale;                              // current locale used for data
        rtl::OUString   m_sDefaultLocale;                       // default locale set for a user
        rtl::OUString   m_sUser;                                // user key used (could be empty)
        rtl::OUString   m_sDefaultUser;                         // user key used (could be empty)

    public:
        OOptions(const uno::Reference< script::XTypeConverter >& _xConverter)
            :m_xConverter(_xConverter) {}

        OOptions(const OOptions& _rOptions)
            :m_xConverter(_rOptions.getTypeConverter())
            ,m_sDefaultLocale(_rOptions.getDefaultLocale())
            ,m_sDefaultUser(_rOptions.getDefaultUser())
            ,m_sLocale(_rOptions.m_sLocale)
            ,m_sUser(_rOptions.m_sUser){}

        uno::Reference< script::XTypeConverter > getTypeConverter() const {return m_xConverter;}

        rtl::OUString getLocale() const {return m_sLocale.getLength() ? m_sLocale : m_sDefaultLocale;}
        const rtl::OUString& getDefaultLocale() const {return m_sDefaultLocale;}
        sal_Bool hasDefaultLocale() const {return !m_sLocale.getLength() || m_sLocale == m_sDefaultLocale;}

        rtl::OUString getUser() const {return m_sUser.getLength() ? m_sUser : m_sDefaultUser;}
        const rtl::OUString& getDefaultUser() const {return m_sDefaultUser;}
        sal_Bool hasDefaultUser() const {return !m_sUser.getLength() || m_sUser == m_sDefaultUser;}

        void setUser(const rtl::OUString& _rUser) {m_sUser = _rUser;}
        void setDefaultUser(const rtl::OUString& _rUser) {m_sDefaultUser = _rUser;}
        void setLocale(const rtl::OUString& _rLocale) {m_sLocale = _rLocale;}
        void setDefaultLocale(const rtl::OUString& _rLocale) {m_sDefaultLocale = _rLocale;}
    };

    struct ltOptions
    {
        bool operator()(const ::vos::ORef<OOptions> &o1, const ::vos::ORef<OOptions> &o2) const
        {
            sal_Int32 nLt = o1->getUser().compareTo(o2->getUser());
            if (!nLt)
                return o1->getLocale().compareTo(o2->getLocale()) < 0 ? true : false;
            else
                return nLt < 0 ? true : false;
        }
    };
} // namespace

#endif
