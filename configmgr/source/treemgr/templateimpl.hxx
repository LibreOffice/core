/*************************************************************************
 *
 *  $RCSfile: templateimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 09:06:32 $
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

#include "template.hxx"

#include "configpath.hxx"

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

            TemplateName(UnoType const& aType)
            : aName(makeSimpleTypeName(aType))
            , aModule(makeSimpleTypeModuleName())
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

            bool isEmpty() const
            {
                return aName.isEmpty();
            }

            bool isSimpleTypeName() const
            {
                return aModule == makeSimpleTypeModuleName();
            }
            //-----------------------------------------------------------------

            bool operator<(TemplateName const& aOther) const
            {
                return (aModule == aOther.aModule) ? (aName < aOther.aName) : (aModule < aOther.aModule);
            }

            //-----------------------------------------------------------------
            static TemplateName parseTemplatePath(OUString const& sName);
            //-----------------------------------------------------------------
            static Name makeSimpleTypeName(UnoType const& aType);
            //-----------------------------------------------------------------
            static Name makeSimpleTypeModuleName();
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

            static UnoType getUnoInterfaceType()
            {
                uno::Reference<uno::XInterface> const * const p = 0;
                return getCppuType(p);
            }
            //-----------------------------------------------------------------

            static UnoType getNoTypeAvailable()
            {
                return getVoidCppuType();
            }
            //-----------------------------------------------------------------

            static void assignActualType (Template& aTemplate,UnoType const& aType);
            //-----------------------------------------------------------------

            static TemplateHolder findTemplate (TemplateName const& aNames, TemplateProvider const& aProvider);
            static TemplateHolder makeTemplate (TemplateName const& aNames, TemplateProvider const& aProvider, UnoType const& aType, Attributes const& aAttrs);
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
        // class TemplateProvider_Impl
        //---------------------------------------------------------------------

        struct TemplateProvider_Impl : vos::OReference
        {
            TemplateProvider_Impl(ITemplateProvider* pProvider);

            std::auto_ptr<INode> instantiate(TemplateHolder const& aTemplate);

            TemplateHolder findTemplate (TemplateName const& aNames);
            TemplateHolder makeTemplate (TemplateName const& aNames, UnoType const& aType, Attributes const& aAttrs);
            TemplateHolder makeElementTemplateWithType(TemplateName const& aNames, ISubtree const& aSet);

        private:
            ITemplateProvider* m_pProvider;
            TemplateRepository m_aRepository;
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_TEMPLATEIMPL_HXX_
