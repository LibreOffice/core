/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: template.hxx,v $
 * $Revision: 1.15 $
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

#ifndef CONFIGMGR_CONFIGTEMPLATE_HXX_
#define CONFIGMGR_CONFIGTEMPLATE_HXX_

#include "configexcept.hxx"
#include "configpath.hxx"
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#ifndef _CONFIGMGR_UTILITY_HXX_
#include <utility.hxx>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    class RequestOptions;
    class TreeManager;
    namespace sharable { struct SetNode; }
    namespace configuration
    {
//-----------------------------------------------------------------------------

        class AbsolutePath;
//-----------------------------------------------------------------------------

        struct TemplateProvider_Impl;
        class TemplateProvider
        {
            friend class SetElementFactory;
            friend class TemplateImplHelper;

            rtl::Reference<TemplateProvider_Impl> m_aImpl;
        public:
            TemplateProvider(); // creates an empty (invalid) template instance provider
            TemplateProvider(rtl::Reference< TreeManager > const & xProvider, RequestOptions const& xOptions);
            TemplateProvider(TemplateProvider const& aOther);
            TemplateProvider& operator=(TemplateProvider const& aOther);
            ~TemplateProvider();

            bool isValid() const { return !!m_aImpl.is(); }
        };
//-----------------------------------------------------------------------------

        /// provides information about the elements of a <type>Node</type> that is a Container ("set").
        class Template : public salhelper::SimpleReferenceObject
        {
            rtl::OUString m_aName;
            rtl::OUString m_aModule;
            com::sun::star::uno::Type       m_aInstanceType;
        private:
            explicit Template(rtl::OUString const& aName, rtl::OUString const& aModule,com::sun::star::uno::Type const& aType);

        public:
        /// checks if the type of an instance of this is known
            bool            isInstanceTypeKnown() const;

        /// checks if this is a 'value' template <p> PRE: the instance type is known </p>
            bool            isInstanceValue() const;

        /// get the UNO type for instances (primarily (only ?) for 'value' templates) <p> PRE: the instance type is known </p>
            com::sun::star::uno::Type           getInstanceType() const;

        /// get the path where the template is located
            rtl::OUString       getPathString() const;

        /// get the local name of the template
            rtl::OUString getName() const { return m_aName; }

        /// get the package name of the template
            rtl::OUString getModule() const { return m_aModule; }

            friend class TemplateImplHelper;
        };

        /// make a template instance that matches the elements of the given set. Ensures that the element type is known
        rtl::Reference<Template> makeSetElementTemplate(sharable::SetNode * set, TemplateProvider const& _aProvider);
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGTEMPLATE_HXX_
