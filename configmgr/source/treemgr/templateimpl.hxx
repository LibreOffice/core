/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: templateimpl.hxx,v $
 * $Revision: 1.17 $
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

#ifndef CONFIGMGR_TEMPLATEIMPL_HXX_
#define CONFIGMGR_TEMPLATEIMPL_HXX_

#include "template.hxx"
#include "configpath.hxx"
#include "requestoptions.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include "utility.hxx"

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace data { class TreeSegment; }
//-----------------------------------------------------------------------------

    namespace configuration
    {
//-----------------------------------------------------------------------------


        //-----------------------------------------------------------------------------
        // class TemplateName
        //-----------------------------------------------------------------------------
        struct TemplateName
        {
            //-----------------------------------------------------------------
            rtl::OUString aName, aModule;

            //-----------------------------------------------------------------
            TemplateName()
            : aName()
            , aModule()
            {}

            TemplateName(rtl::OUString const& aName_)
            : aName(aName_)
            , aModule()
            {}

            TemplateName(rtl::OUString const& aName_, rtl::OUString const& aModule_)
            : aName(aName_)
            , aModule(aModule_)
            {}

            //-----------------------------------------------------------------

            /// compose the path where the template is located
            rtl::OUString makePathString() const
            {
                rtl::OUStringBuffer aBuffer;

                if (aModule.getLength() != 0)
                    aBuffer.append( this->aModule ).append(sal_Unicode('/'));

                aBuffer.append( this->aName );

                return aBuffer.makeStringAndClear();
            }
            //-----------------------------------------------------------------
            bool isEmpty() const
            {
                return aName.getLength() == 0;
            }

            bool isSimpleTypeName() const;

            com::sun::star::uno::Type resolveToSimpleType() const;
            //-----------------------------------------------------------------

            bool operator<(TemplateName const& aOther) const
            {
                return (aModule == aOther.aModule) ? (aName < aOther.aName) : (aModule < aOther.aModule);
            }

            //-----------------------------------------------------------------
            static com::sun::star::uno::Type resolveSimpleTypeName(rtl::OUString const& aName);
            //-----------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
            static rtl::OUString makeNativeTypeModuleName();
            //-----------------------------------------------------------------
            static rtl::OUString makeLocalizedTypeModuleName();
#endif
            //-----------------------------------------------------------------
        };
    //-------------------------------------------------------------------------

        //---------------------------------------------------------------------
        // class TemplateImplHelper
        //---------------------------------------------------------------------

        class TemplateImplHelper
        {
        public:
            //-----------------------------------------------------------------

            static com::sun::star::uno::Type getNoTypeAvailable()
            {
                return getVoidCppuType();
            }
            //-----------------------------------------------------------------

            static void assignActualType (Template& aTemplate,com::sun::star::uno::Type const& aType);
            //-----------------------------------------------------------------

            static rtl::Reference<Template> makeElementTemplateWithType(TemplateName const& aNames, TemplateProvider const& _aProvider, sharable::SetNode * set);
            //-----------------------------------------------------------------

            static rtl::Reference<Template> createNew (TemplateName const& aNames,com::sun::star::uno::Type const& aType);
            //-----------------------------------------------------------------
        };
    //-------------------------------------------------------------------------

        typedef std::map< TemplateName, rtl::Reference<Template> > TemplateRepository;
    //-------------------------------------------------------------------------

        //---------------------------------------------------------------------
        // class TemplateProvider_Impl
        //---------------------------------------------------------------------

        struct TemplateProvider_Impl : salhelper::SimpleReferenceObject
        {
            TemplateProvider_Impl(rtl::Reference< TreeManager > const & xProvider, RequestOptions const& aOptions);

            rtl::Reference< data::TreeSegment > instantiate(rtl::Reference<Template> const& aTemplate);

            rtl::Reference<Template> makeElementTemplateWithType(TemplateName const& _aNames, sharable::SetNode * set);
        private:
            rtl::Reference< TreeManager >      m_xProvider;
            RequestOptions          m_aOptions;

            TemplateRepository      m_aRepository;
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_TEMPLATEIMPL_HXX_
