/*************************************************************************
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: lla $ $Date: 2000-11-30 15:45:30 $
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

    public:
        OOptions(const uno::Reference< script::XTypeConverter >& _xConverter)
            :m_xConverter(_xConverter) {}

        OOptions(const uno::Reference< script::XTypeConverter >& _xConverter,
            const rtl::OUString& _rLocale, const rtl::OUString& _rDefaultLocale,
            const rtl::OUString& _rUser)
            :m_xConverter(_xConverter)
            ,m_sDefaultLocale(_rDefaultLocale)
            ,m_sLocale(_rLocale)
            ,m_sUser(_rUser){}

        OOptions(const OOptions& _rOptions)
            :m_xConverter(_rOptions.getTypeConverter())
            ,m_sDefaultLocale(_rOptions.getDefaultLocale())
            ,m_sLocale(_rOptions.getLocale())
            ,m_sUser(_rOptions.getUser()){}

        uno::Reference< script::XTypeConverter > getTypeConverter() const {return m_xConverter;}
        const rtl::OUString& getUser() const {return m_sUser;}
        const rtl::OUString& getLocale() const {return m_sLocale;}
        const rtl::OUString& getDefaultLocale() const {return m_sDefaultLocale;}

        sal_Bool hasDefaultLocale() const {return !m_sLocale.getLength() || m_sLocale == m_sDefaultLocale;}
        rtl::OUString getCurrentLocale() const {return m_sLocale.getLength() ? m_sLocale : m_sDefaultLocale;}

        void setUser(const rtl::OUString& _rUser) {m_sUser = _rUser;}
        void setLocale(const rtl::OUString& _rLocale) {m_sLocale = _rLocale;}
        void setDefaultLocale(const rtl::OUString& _rLocale) {m_sDefaultLocale = _rLocale;}

    };
} // namespace

#endif
