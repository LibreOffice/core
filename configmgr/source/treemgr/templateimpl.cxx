/*************************************************************************
 *
 *  $RCSfile: templateimpl.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-16 17:35:23 $
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

#include <stdio.h>
#include "templateimpl.hxx"
#include "treeprovider.hxx"

#include "apitypes.hxx"
#include "strdecl.hxx"
#include "typeconverter.hxx"

#include <vos/refernce.hxx>
#include <map>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

Name TemplateName::makeSimpleTypeName(UnoType const& aType)
{
    OUString sTypeName = toTemplateName(aType);
    return Name(sTypeName, Name::NoValidate());
}
//-----------------------------------------------------------------------------

UnoType TemplateName::resolveSimpleTypeName(Name const& aName)
{
    OUString sTypeName = aName.toString();
    return parseTemplateName(sTypeName);
}
//-----------------------------------------------------------------------------

Name TemplateName::makeNativeTypeModuleName()
{
    OUString aModuleName = TEMPLATE_MODULE_NATIVE_VALUE;
    return Name(aModuleName, Name::NoValidate());
}
//-----------------------------------------------------------------------------
Name TemplateName::makeLocalizedTypeModuleName()
{
    OUString aModuleName = TEMPLATE_MODULE_LOCALIZED_VALUE;
    return Name(aModuleName, Name::NoValidate());
}
//-----------------------------------------------------------------------------
bool TemplateName::isSimpleTypeName() const
{
    OUString aPrefix = TEMPLATE_MODULE_NATIVE_PREFIX;
    if (aModule.toString().compareTo(aPrefix,aPrefix.getLength()) != 0)
        return false;

    OSL_ENSURE( aModule == makeNativeTypeModuleName() ||
                aModule == makeLocalizedTypeModuleName(),
                "ERROR: Invalid template module with native prefix found");

    return true;
}
//-----------------------------------------------------------------------------

UnoType TemplateName::resolveToSimpleType() const
{
    UnoType aType;
    if ( isSimpleTypeName() )
    {
        aType = resolveSimpleTypeName( aName );
    }
    else
        OSL_ENSURE(false, "TemplateName::resolveToSimpleType must be called only for simple type name pairs");
    return aType;
}
//-----------------------------------------------------------------------------
#if 0
TemplateName TemplateName::parseTemplatePath(OUString const& sName)
{
    Path::Components aPath = Path::parse(sName);
    if (aPath.empty())
        return TemplateName();

    TemplateName aNames( aPath.back() );

    switch(aPath.size())
    {
    case 1: break;
    case 2: aNames.aModule = aPath.front(); break;

    case 3: if (aPath[0].isEmpty())
            {
                aNames.aModule = aPath[1];
                break;
            }
            // fall through
    default:
            OSL_ENSURE(false, "Invalid template path - too many components");
            // hack - cram it all into the module part
            aPath.pop_back();
            aNames.aModule = Name( PathRep(aPath).toString(), Name::NoValidate() );
            break;
    }
    return aNames;
}
#endif
//-----------------------------------------------------------------------------

TemplateName TemplateName::parseTemplateNames(OUString const& sName,OUString const& sModule)
{
    return TemplateName( sName, sModule );
}
//-----------------------------------------------------------------------------
// class TemplateImplHelper
//-----------------------------------------------------------------------------

TemplateHolder TemplateImplHelper::createNew (TemplateName const& aNames,UnoType const& aType, Attributes const& aAttrs)
{
    return new Template(aNames.aName, aNames.aModule, aType, aAttrs);
}
//-----------------------------------------------------------------------------

TemplateHolder TemplateImplHelper::findTemplate (TemplateName const& aNames, TemplateProvider const& aProvider)
{
    OSL_ENSURE(aProvider.m_aImpl.isValid(), "Cannot find a template without a provider");

    if (aProvider.m_aImpl.isValid())
        return aProvider.m_aImpl->findTemplate(aNames);

    else
        return TemplateHolder(0);
}
//-----------------------------------------------------------------------------

TemplateHolder TemplateImplHelper::makeTemplate (TemplateName const& aNames, TemplateProvider const& aProvider, UnoType const& aType, Attributes const& aAttrs)
{
    OSL_ENSURE(aProvider.m_aImpl.isValid(), "Cannot find a template without a provider");

    if (aProvider.m_aImpl.isValid())
        return aProvider.m_aImpl->makeTemplate(aNames,aType,aAttrs);

    else
        return TemplateHolder(0);
}
//-----------------------------------------------------------------------------

TemplateHolder TemplateImplHelper::makeElementTemplateWithType(TemplateName const& aNames, TemplateProvider const& aProvider, ISubtree const& aSet)
{
    OSL_ENSURE(aProvider.m_aImpl.isValid(), "Cannot find a template without a provider");

    if (aProvider.m_aImpl.isValid())
        return aProvider.m_aImpl->makeElementTemplateWithType(aNames,aSet);

    else
        return TemplateHolder(0);
}
//-----------------------------------------------------------------------------

void TemplateImplHelper::assignActualType (Template& aTemplate,UnoType const& aType)
{
    OSL_PRECOND( aType != getNoTypeAvailable(), "ERROR: Assigning NO type to a template" );

    if (!aTemplate.isInstanceTypeKnown())
        aTemplate.m_aInstanceType = aType;

    OSL_ENSURE(aTemplate.isInstanceTypeKnown(), "ERROR: Could not assign given type to a template");
    OSL_ENSURE(aTemplate.getInstanceType() == aType, "ERROR: Trying to change instance type of a template");
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class TemplateProvider_Impl
//-----------------------------------------------------------------------------

TemplateProvider_Impl::TemplateProvider_Impl(ITemplateProvider* pProvider)
: m_pProvider(pProvider)
, m_aRepository()
{
}
//-----------------------------------------------------------------------------

std::auto_ptr<INode> TemplateProvider_Impl::instantiate(TemplateHolder const& aTemplate)
{
    std::auto_ptr<INode> pRet;
    if (m_pProvider && aTemplate.isValid())
    {
        pRet = m_pProvider->createInstance(aTemplate->getPath().toString());
    }
    return pRet;
}
//-----------------------------------------------------------------------------

TemplateHolder TemplateProvider_Impl::findTemplate (TemplateName const& aNames)
{
    typedef TemplateRepository::value_type Entry;

    TemplateRepository::iterator it = m_aRepository.find(aNames);
    if (it == m_aRepository.end())
        it = m_aRepository.insert( Entry( aNames, TemplateImplHelper::createNew(aNames) ) ).first;

    return it->second;

}
//-----------------------------------------------------------------------------

TemplateHolder TemplateProvider_Impl::makeTemplate (TemplateName const& aNames, UnoType const& aType, Attributes const& aAttrs)
{
    typedef TemplateRepository::value_type Entry;

    TemplateRepository::iterator it = m_aRepository.find(aNames);
    if (it == m_aRepository.end())
        it = m_aRepository.insert( Entry( aNames, TemplateImplHelper::createNew(aNames,aType, aAttrs) ) ).first;

    else if (!it->second->isInstanceTypeKnown())
        TemplateImplHelper::assignActualType(it->second.getBody(), aType);

    OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
    OSL_ENSURE(it->second->getInstanceType() == aType, "Inconsistent type found for Template");
    return it->second;

}

//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
    struct TypeDetector : NodeAction
    {
        enum Result
        {
            Contradicting = -1,
            NotFound = 0,
            SomeValue,
            VariousValue,
            SomeTree
        };

        Result  result;
        UnoType type;

        TypeDetector() : result(NotFound), type() {}

    private: // NodeAction implementation
        void handle(ValueNode const&);
        void handle(ISubtree const&);
    };
//-----------------------------------------------------------------------------
    static UnoType detectNodeType(INode const* pNode)
    {
        if (pNode == 0)
            throw configuration::Exception("Could not load required template to detect set elements");

        TypeDetector aDetector;
        aDetector.applyToNode( *pNode );

        switch(aDetector.result)
        {
        case TypeDetector::SomeTree:        // found tree
        case TypeDetector::VariousValue:    // found an Any
        case TypeDetector::SomeValue:       // found a particular value type
            break;

#ifdef DBG_UTIL
        case TypeDetector::NotFound:        OSL_ENSURE(false,"Impossible Result: Node not handled");        if (false) // dirty abuse of case
        case TypeDetector::Contradicting:   OSL_ENSURE(false,"Impossible Result: Node contradicts itself"); if (false) // dirty abuse of case
#endif // DBG_UTIL
        default:                            OSL_ENSURE(false,"Impossible Result: Unknown result code");

            throw configuration::Exception("INTERNAL ERROR: Could not detect set element type from loaded instance");
        }
        return aDetector.type;
    }

    //-------------------------------------------------------------------------
    static bool detectElementType(UnoType& aType, ISubtree const& aSet)
    {
        TypeDetector aDetector;
        aDetector.applyToChildren( aSet );

        switch(aDetector.result)
        {
        case TypeDetector::SomeTree:        // found tree
        case TypeDetector::VariousValue:    // found an Any
            aType = aDetector.type;
            return true;

        case TypeDetector::SomeValue:       // found a value or an any
        case TypeDetector::NotFound:        // found no element
            return false;

        case TypeDetector::Contradicting:
            OSL_ENSURE(false,"Invalid Set: contains values and subtrees");
            return false;

        }
        OSL_ENSURE(false,"Unreachable code");
        return false;
    }
//-----------------------------------------------------------------------------

}
//-----------------------------------------------------------------------------
TemplateHolder TemplateProvider_Impl::makeElementTemplateWithType(TemplateName const& aNames, ISubtree const& aSet)
{
    typedef TemplateRepository::value_type Entry;

    TemplateRepository::iterator it = m_aRepository.find(aNames);

    if (it == m_aRepository.end() || !it->second->isInstanceTypeKnown())
    {
        UnoType aType;
        if (aNames.isSimpleTypeName()) // native type found
        {
            aType = aNames.resolveToSimpleType();

            if (aType == TemplateImplHelper::getNoTypeAvailable())
                throw configuration::Exception("INTERNAL ERROR: Could not resolve native type");
        }

        else if (!detectElementType(aType,aSet))
        {
            std::auto_ptr<INode> pTemplateInstance;
            if (m_pProvider)
            {
                OSL_ASSERT(aNames.aName.toString() == aSet.getElementTemplateName());
                OSL_ASSERT(aNames.aModule.toString() == aSet.getElementTemplateModule());

                OUString sPath = aNames.makePath().toString(); // could also be extracted from aSet

                pTemplateInstance = m_pProvider->createInstance(sPath);
            }

            aType = detectNodeType(pTemplateInstance.get()); // throws if necessary
        }
        OSL_ASSERT( aType != TemplateImplHelper::getNoTypeAvailable() );

        if (it == m_aRepository.end())
            it = m_aRepository.insert( Entry( aNames, TemplateImplHelper::createNew(aNames,aType, aSet.getAttributes()) ) ).first;

        else
            TemplateImplHelper::assignActualType(it->second.getBody(), aType);

        OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
        OSL_ENSURE(it->second->getInstanceType() == aType, "Inconsistent type found for Template");
    }

#ifdef DBG_UTIL
    else
    {
        OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
        UnoType aTestType;
        if (detectElementType(aTestType,aSet))
            OSL_ENSURE(it->second->getInstanceType() == aTestType, "Inconsistent type found for Template");
    }
#endif // DBG_UTIL

    return it->second;

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
    void TypeDetector::handle(ValueNode const& aValueNode)
    {
        UnoType aFoundType = aValueNode.getValueType();

        bool isNullType = (aFoundType.getTypeClass() == uno::TypeClass_VOID);
        bool isAnyType  = (aFoundType.getTypeClass() == uno::TypeClass_ANY);

        switch (this->result) // transition depends on previous state
        {
        case NotFound:
            this->type = aFoundType;

            if (isAnyType)
                this->result = VariousValue;

            else if (!isNullType)
                this->result = SomeValue;

            break;

        case SomeValue:
            if (!isNullType && this->type != aFoundType)
            {
                this->result = VariousValue;
                this->type =  configapi::getAnyType();
                OSL_ASSERT(type.getTypeClass() == uno::TypeClass_ANY);
            }
            break;

        case VariousValue: // remain unchanged - type already is 'Any'
            break;

        case SomeTree: OSL_ENSURE(false, "Found value node does not match previous (tree) sibling");
        default:
            this->result = Contradicting;
            break;
        }
    }
//-----------------------------------------------------------------------------
    void TypeDetector::handle(ISubtree const&)
    {
        switch (this->result) // transition depends on previous state
        {
        case NotFound:
            this->type = TemplateImplHelper::getUnoInterfaceType();
            this->result = SomeTree;
            break;

        case SomeTree: // remain unchanged - type already is Tree
            break;

        case SomeValue:
        case VariousValue:  OSL_ENSURE(false, "Found Subtree node does not match previous (value) sibling");
        default:
            this->result = Contradicting;
            break;
        }
    }

//-----------------------------------------------------------------------------
} // anonymous
//-----------------------------------------------------------------------------
    } // configuration
} // configmgr

