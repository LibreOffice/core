/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: templateimpl.cxx,v $
 * $Revision: 1.24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include <stdio.h>
#include "templateimpl.hxx"
#include "treemanager.hxx"
#include "node.hxx"
#include "nodevisitor.hxx"
#include "strdecl.hxx"
#include "typeconverter.hxx"
#include "localizedtreeactions.hxx"
#include "treeactions.hxx"
#include "apitypes.hxx"

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

com::sun::star::uno::Type TemplateName::resolveSimpleTypeName(rtl::OUString const& aName)
{
    rtl::OUString sTypeName = aName;
    return parseTemplateName(sTypeName);
}
//-----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
rtl::OUString TemplateName::makeNativeTypeModuleName()
{
    return rtl::OUString( TEMPLATE_MODULE_NATIVE_VALUE );
}

//-----------------------------------------------------------------------------
rtl::OUString TemplateName::makeLocalizedTypeModuleName()
{
    return rtl::OUString( TEMPLATE_MODULE_LOCALIZED_VALUE );
}
#endif
//-----------------------------------------------------------------------------
bool TemplateName::isSimpleTypeName() const
{
    bool bIsSimple = (aModule.compareToAscii(TEMPLATE_MODULE_NATIVE_PREFIX,
                                                        TEMPLATE_MODULE_NATIVE_PREFIX.getLength()) == 0);

    OSL_ENSURE(!bIsSimple ||
                aModule == makeNativeTypeModuleName() ||
                aModule == makeLocalizedTypeModuleName(),
                "ERROR: Invalid template module with native prefix found");

    return bIsSimple;
}
//-----------------------------------------------------------------------------

com::sun::star::uno::Type TemplateName::resolveToSimpleType() const
{
    com::sun::star::uno::Type aType;
    if ( isSimpleTypeName() )
    {
        aType = resolveSimpleTypeName( aName );
    }
    else
        OSL_ENSURE(false, "TemplateName::resolveToSimpleType must be called only for simple type name pairs");
    return aType;
}
//-----------------------------------------------------------------------------
// class TemplateImplHelper
//-----------------------------------------------------------------------------

rtl::Reference<Template> TemplateImplHelper::createNew (TemplateName const& aNames,com::sun::star::uno::Type const& aType)
{
    return new Template(aNames.aName, aNames.aModule, aType);
}
//-----------------------------------------------------------------------------

rtl::Reference<Template> TemplateImplHelper::makeElementTemplateWithType(TemplateName const& _aNames, TemplateProvider const& _aProvider, sharable::SetNode * set)
{
    OSL_ENSURE(_aProvider.m_aImpl.is(), "ERROR: Cannot find a template without a provider");

    if (_aProvider.m_aImpl.is())
        return _aProvider.m_aImpl->makeElementTemplateWithType(_aNames, set);

    else
        return rtl::Reference<Template>(0);
}
//-----------------------------------------------------------------------------

void TemplateImplHelper::assignActualType (Template& aTemplate,com::sun::star::uno::Type const& aType)
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

TemplateProvider_Impl::TemplateProvider_Impl(rtl::Reference< TreeManager > const & xProvider, RequestOptions const& aOptions)
: m_xProvider(xProvider)
, m_aOptions(aOptions)
, m_aRepository()
{
}
//-----------------------------------------------------------------------------

rtl::Reference< data::TreeSegment > TemplateProvider_Impl::instantiate(rtl::Reference<Template> const& aTemplate)
{
    if (aTemplate.is())
    {
        sharable::TreeFragment * aTemplateData = m_xProvider->requestTemplate(aTemplate->getName(), aTemplate->getModule());

        return cloneExpandedForLocale(aTemplateData, m_aOptions.getLocale());
    } else {
        return rtl::Reference< data::TreeSegment >();
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
    struct TypeDetector : data::SetVisitor
    {
        enum State
        {
            Contradicting = -1,
            NotFound = 0,
            SomeValue,
            VariousValue,
            SomeTree
        };

        State   result;
        com::sun::star::uno::Type type;

        TypeDetector() : result(NotFound), type() {}

    private:
        using SetVisitor::handle;
        virtual bool handle(sharable::Node * node);
        virtual bool handle(sharable::ValueNode * node);
    };
//-----------------------------------------------------------------------------
    static com::sun::star::uno::Type detectNodeType(sharable::TreeFragment * element)
    {
        if (element == 0)
            throw configuration::Exception("Could not load required template to detect set elements");

        TypeDetector aDetector;
        aDetector.visitTree(element);

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
    static bool detectElementType(com::sun::star::uno::Type& aType, sharable::SetNode * set)
    {
        TypeDetector aDetector;
        aDetector.visitElements(set);

        bool bResult = false;
        switch(aDetector.result)
        {
        case TypeDetector::SomeTree:        // found tree
        case TypeDetector::VariousValue:    // found an Any
            aType = aDetector.type;
            bResult = true;
            break;

        case TypeDetector::SomeValue:       // found a value or an any
        case TypeDetector::NotFound:        // found no element
            break;

        case TypeDetector::Contradicting:
            OSL_ENSURE(false,"Invalid Set: contains values and subtrees");
            break;

        default: OSL_ENSURE(false,"Unreachable code");  break;
        }
        return bResult;
    }
//-----------------------------------------------------------------------------

}
//-----------------------------------------------------------------------------
rtl::Reference<Template> TemplateProvider_Impl::makeElementTemplateWithType(TemplateName const& _aNames, sharable::SetNode * set)
{
    TemplateRepository::iterator it = m_aRepository.find(_aNames);

    if (it == m_aRepository.end() || !it->second->isInstanceTypeKnown())
    {
        com::sun::star::uno::Type aType;
        if (_aNames.isSimpleTypeName()) // native type found
        {
            aType = _aNames.resolveToSimpleType();

            if (aType == TemplateImplHelper::getNoTypeAvailable())
                throw configuration::Exception("INTERNAL ERROR: Could not resolve native type");
        }

        else if (!detectElementType(aType, set))
        {
            OSL_ASSERT(_aNames.aName == set->getElementTemplateName());
            OSL_ASSERT(_aNames.aModule == set->getElementTemplateModule());

            sharable::TreeFragment * aTemplateData = m_xProvider->requestTemplate(_aNames.aName, _aNames.aModule);

            aType = detectNodeType(aTemplateData); // throws if necessary
        }
        OSL_ASSERT( aType != TemplateImplHelper::getNoTypeAvailable() );

        if (it == m_aRepository.end())
            it = m_aRepository.insert( TemplateRepository::value_type( _aNames, TemplateImplHelper::createNew(_aNames,aType) ) ).first;

        else
            TemplateImplHelper::assignActualType(*it->second, aType);

        OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
        OSL_ENSURE(it->second->getInstanceType() == aType, "Inconsistent type found for Template");
    }

#ifdef DBG_UTIL
    else
    {
        OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
        com::sun::star::uno::Type aTestType;
        if (detectElementType(aTestType, set))
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
    bool TypeDetector::handle(sharable::ValueNode * node)
    {
        com::sun::star::uno::Type aFoundType = node->getValueType();

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
        return false; // always continue to detect errors in data
    }
//-----------------------------------------------------------------------------
    bool TypeDetector::handle(sharable::Node * node)
    {
        (void) node; // avoid warnings
        OSL_ENSURE(!node->isValue(), "Value node dipatched to wrong handler");
        switch (this->result) // transition depends on previous state
        {
        case NotFound:
            this->type = configapi::getUnoInterfaceType();
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
        return false; // always continue to detect errors in data
    }

//-----------------------------------------------------------------------------
} // anonymous
//-----------------------------------------------------------------------------
    } // configuration
} // configmgr

