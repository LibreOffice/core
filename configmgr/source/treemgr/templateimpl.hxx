/*************************************************************************
 *
 *  $RCSfile: templateimpl.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:48 $
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

#ifndef CONFIGMGR_TEMPLATEIMPL_HXX_
#define CONFIGMGR_TEMPLATEIMPL_HXX_

#ifndef CONFIGMGR_CONFIGTEMPLATE_HXX_
#include "template.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#include "requestoptions.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    struct ITemplateProvider;
//-----------------------------------------------------------------------------
    namespace data { class SetNodeAccess; class TreeSegment; }
//-----------------------------------------------------------------------------

    namespace configuration
    {
//-----------------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;

//-----------------------------------------------------------------------------


        //-----------------------------------------------------------------------------
        // class TemplateName
        //-----------------------------------------------------------------------------
        struct TemplateName
        {
            //-----------------------------------------------------------------
            Name aName, aModule;

            //-----------------------------------------------------------------
            TemplateName()
            : aName()
            , aModule()
            {}

            TemplateName(UnoType const& aType, bool bLocalized)
            : aName(makeSimpleTypeName(aType))
            , aModule(makeSimpleTypeModuleName(bLocalized))
            {}

            TemplateName(Name const& aName_)
            : aName(aName_)
            , aModule()
            {}

            TemplateName(Name const& aName_, Name const& aModule_)
            : aName(aName_)
            , aModule(aModule_)
            {}

            TemplateName(OUString const& sName_, OUString const& sModule_)
            : aName(    makeName(sName_, Name::NoValidate()) )
            , aModule(  makeName(sModule_, Name::NoValidate()) )
            {}
            //-----------------------------------------------------------------

            /// compose the path where the template is located
            OUString makePathString() const
            {
                rtl::OUStringBuffer aBuffer;

                if (!aModule.isEmpty())
                    aBuffer.append( this->aModule.toString() ).append(sal_Unicode('/'));

                aBuffer.append( this->aName.toString() );

                return aBuffer.makeStringAndClear();
            }
            //-----------------------------------------------------------------
            bool isEmpty() const
            {
                return aName.isEmpty();
            }

            bool isSimpleTypeName() const;

            UnoType resolveToSimpleType() const;
            //-----------------------------------------------------------------

            bool operator<(TemplateName const& aOther) const
            {
                return (aModule == aOther.aModule) ? (aName < aOther.aName) : (aModule < aOther.aModule);
            }

            //-----------------------------------------------------------------
            static UnoType resolveSimpleTypeName(Name const& aName);
            //-----------------------------------------------------------------
            static Name makeSimpleTypeName(UnoType const& aType);
            //-----------------------------------------------------------------
            static Name makeNativeTypeModuleName();
            //-----------------------------------------------------------------
            static Name makeLocalizedTypeModuleName();
            //-----------------------------------------------------------------
            static Name makeSimpleTypeModuleName(bool bLocalized)
            {
                return bLocalized ? makeLocalizedTypeModuleName() : makeNativeTypeModuleName();
            }
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

            static UnoType getNoTypeAvailable()
            {
                return getVoidCppuType();
            }
            //-----------------------------------------------------------------

            static void assignActualType (Template& aTemplate,UnoType const& aType);
            //-----------------------------------------------------------------

            static TemplateHolder makeSpecialTemplate (TemplateName const& _aNames, SpecialTemplateProvider const& aProvider, UnoType const& aType);

            static TemplateHolder makeElementTemplateWithType(TemplateName const& aNames, TemplateProvider const& _aProvider, data::SetNodeAccess const& _aSet);
            //-----------------------------------------------------------------

            static TemplateHolder createNew (TemplateName const& aNames,UnoType const& aType);
            //-----------------------------------------------------------------
        };
    //-------------------------------------------------------------------------

        typedef std::map<TemplateName, TemplateHolder> TemplateRepository;
    //-------------------------------------------------------------------------

        //---------------------------------------------------------------------
        // class SpecialTemplateProvider_Impl
        //---------------------------------------------------------------------

        struct SpecialTemplateProvider_Impl : salhelper::SimpleReferenceObject
        {
            SpecialTemplateProvider_Impl();

            TemplateHolder makeTemplate (TemplateName const& aNames, UnoType const& aType);

        private:
            TemplateRepository m_aRepository;
        };

        //---------------------------------------------------------------------
        // class TemplateProvider_Impl
        //---------------------------------------------------------------------

        struct TemplateProvider_Impl : salhelper::SimpleReferenceObject
        {
            typedef TemplateProvider::TemplateManagerRef TemplateManagerRef;

            TemplateProvider_Impl(TemplateManagerRef const & xProvider, RequestOptions const& aOptions);

            data::TreeSegment instantiate(memory::Accessor const& _aSourceAccessor, TemplateHolder const& aTemplate);

            TemplateHolder makeElementTemplateWithType(TemplateName const& _aNames, data::SetNodeAccess const& _aSet);
        private:
            TemplateManagerRef      m_xProvider;
            RequestOptions          m_aOptions;

            TemplateRepository      m_aRepository;
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_TEMPLATEIMPL_HXX_
