/*************************************************************************
 *
 *  $RCSfile: templateimpl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:33:42 $
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

#include "apitypes.hxx"

#include "template.hxx"

#include "configpath.hxx"
#include "options.hxx"

#include <vos/refernce.hxx>
#include <map>
#include <memory>

namespace configmgr
{
//-----------------------------------------------------------------------------
    class INode;
    class ISubtree;
    class ITemplateProvider;
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
            : aName(sName_, Name::NoValidate())
            , aModule(sModule_, Name::NoValidate())
            {}
            //-----------------------------------------------------------------

            /// compose the path where the template is located
            RelativePath makePath() const
            {
                Path::Components aPath;
                if (!aModule.isEmpty()) aPath.push_back(aModule);
                aPath.push_back(aName);
                return RelativePath( aPath  );
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
            // static TemplateName parseTemplatePath(OUString const& sName);
            //-----------------------------------------------------------------
            static TemplateName parseTemplateNames(OUString const& sName, OUString const& sModule);
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

            static TemplateHolder makeSpecialTemplate (TemplateName const& aNames, SpecialTemplateProvider const& aProvider, UnoType const& aType, Attributes const& aAttrs);

            static TemplateHolder makeElementTemplateWithType(TemplateName const& aNames, TemplateProvider const& aProvider, ISubtree const& aSet);
            //-----------------------------------------------------------------

            static TemplateHolder createNew (TemplateName const& aNames,UnoType const& aType, Attributes const& aAttrs);
            //-----------------------------------------------------------------
            static TemplateHolder createNew (TemplateName const& aNames)
            {
                return createNew(aNames, getNoTypeAvailable(), Attributes());
            }
            //-----------------------------------------------------------------
        };
    //-------------------------------------------------------------------------

        typedef std::map<TemplateName, TemplateHolder> TemplateRepository;
    //-------------------------------------------------------------------------

        //---------------------------------------------------------------------
        // class SpecialTemplateProvider_Impl
        //---------------------------------------------------------------------

        struct SpecialTemplateProvider_Impl : vos::OReference
        {
            SpecialTemplateProvider_Impl();

            TemplateHolder makeTemplate (TemplateName const& aNames, UnoType const& aType, Attributes const& aAttrs);

        private:
            TemplateRepository m_aRepository;
        };

        //---------------------------------------------------------------------
        // class TemplateProvider_Impl
        //---------------------------------------------------------------------

        struct TemplateProvider_Impl : vos::OReference
        {
            TemplateProvider_Impl(ITemplateProvider& rProvider, vos::ORef< OOptions > const& xOptions);

            std::auto_ptr<INode> instantiate(TemplateHolder const& aTemplate);

            TemplateHolder makeElementTemplateWithType(TemplateName const& aNames, ISubtree const& aSet);
        private:
            ITemplateProvider& m_rProvider;
            vos::ORef< OOptions > m_xOptions;

            TemplateRepository m_aRepository;
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_TEMPLATEIMPL_HXX_
