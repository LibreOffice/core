/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: templateimpl.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:47:13 $
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
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
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

        struct SpecialTemplateProvider_Impl : configmgr::SimpleReferenceObject
        {
            SpecialTemplateProvider_Impl();

            TemplateHolder makeTemplate (TemplateName const& aNames, UnoType const& aType);

        private:
            TemplateRepository m_aRepository;
        };

        //---------------------------------------------------------------------
        // class TemplateProvider_Impl
        //---------------------------------------------------------------------

        struct TemplateProvider_Impl : configmgr::SimpleReferenceObject
        {
            typedef TemplateProvider::TemplateManagerRef TemplateManagerRef;

            TemplateProvider_Impl(TemplateManagerRef const & xProvider, RequestOptions const& aOptions);

            data::TreeSegment instantiate(TemplateHolder const& aTemplate);

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
