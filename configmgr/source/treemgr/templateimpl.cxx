/*************************************************************************
 *
 *  $RCSfile: templateimpl.cxx,v $
 *
 *  $Revision: 1.17 $
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

#include <stdio.h>
#include "templateimpl.hxx"

#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif

#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif

#ifndef _CONFIGMGR_STRDECL_HXX_
#include "strdecl.hxx"
#endif
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef CONFIGMGR_LOCALIZEDTREEACTIONS_HXX
#include "localizedtreeactions.hxx"
#endif
#ifndef _CONFIGMGR_TREEACTIONS_HXX_
#include "treeactions.hxx"
#endif
#ifndef CONFIGMGR_API_APITYPES_HXX_
#include "apitypes.hxx"
#endif

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

Name TemplateName::makeSimpleTypeName(UnoType const& aType)
{
    OUString sTypeName = toTemplateName(aType);
    return makeName(sTypeName, Name::NoValidate());
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
    return makeName(aModuleName, Name::NoValidate());
}
//-----------------------------------------------------------------------------
Name TemplateName::makeLocalizedTypeModuleName()
{
    OUString aModuleName = TEMPLATE_MODULE_LOCALIZED_VALUE;
    return makeName(aModuleName, Name::NoValidate());
}
//-----------------------------------------------------------------------------
bool TemplateName::isSimpleTypeName() const
{
    bool bIsSimple = (aModule.toString().compareToAscii(TEMPLATE_MODULE_NATIVE_PREFIX,
                                                        TEMPLATE_MODULE_NATIVE_PREFIX.getLength()) == 0);

    OSL_ENSURE(!bIsSimple ||
                aModule == makeNativeTypeModuleName() ||
                aModule == makeLocalizedTypeModuleName(),
                "ERROR: Invalid template module with native prefix found");

    return bIsSimple;
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
// class TemplateImplHelper
//-----------------------------------------------------------------------------

TemplateHolder TemplateImplHelper::createNew (TemplateName const& aNames,UnoType const& aType)
{
    return new Template(aNames.aName, aNames.aModule, aType);
}
//-----------------------------------------------------------------------------

TemplateHolder TemplateImplHelper::makeSpecialTemplate (TemplateName const& aNames, SpecialTemplateProvider const& aProvider, UnoType const& aType)
{
    OSL_ENSURE(aProvider.m_aImpl.is(), "Cannot find a template without a provider");

    if (aProvider.m_aImpl.is())
        return aProvider.m_aImpl->makeTemplate(aNames,aType);

    else
        return TemplateHolder(0);
}
//-----------------------------------------------------------------------------

TemplateHolder TemplateImplHelper::makeElementTemplateWithType(TemplateName const& _aNames, TemplateProvider const& _aProvider, data::SetNodeAccess const& _aSet)
{
    OSL_ENSURE(_aProvider.m_aImpl.is(), "ERROR: Cannot find a template without a provider");

    if (_aProvider.m_aImpl.is())
        return _aProvider.m_aImpl->makeElementTemplateWithType(_aNames,_aSet);

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
// class SpecialTemplateProvider_Impl
//-----------------------------------------------------------------------------

SpecialTemplateProvider_Impl::SpecialTemplateProvider_Impl()
: m_aRepository()
{
}
//-----------------------------------------------------------------------------

TemplateHolder SpecialTemplateProvider_Impl::makeTemplate (TemplateName const& aNames, UnoType const& aType)
{
    typedef TemplateRepository::value_type Entry;

    TemplateRepository::iterator it = m_aRepository.find(aNames);
    if (it == m_aRepository.end())
        it = m_aRepository.insert( Entry( aNames, TemplateImplHelper::createNew(aNames,aType) ) ).first;

    else if (!it->second->isInstanceTypeKnown())
        TemplateImplHelper::assignActualType(*it->second, aType);

    OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
    OSL_ENSURE(it->second->getInstanceType() == aType, "Inconsistent type found for Template");
    return it->second;

}

//-----------------------------------------------------------------------------
// class TemplateProvider_Impl
//-----------------------------------------------------------------------------

TemplateProvider_Impl::TemplateProvider_Impl(TemplateManagerRef const & xProvider, RequestOptions const& aOptions)
: m_xProvider(xProvider)
, m_aOptions(aOptions)
, m_aRepository()
{
}
//-----------------------------------------------------------------------------

data::TreeSegment TemplateProvider_Impl::instantiate(data::Accessor const& _aSourceAccessor, TemplateHolder const& aTemplate)
{
    data::TreeSegment pRet;
    if (aTemplate.is())
    {
        data::TreeAccessor aTemplateData = m_xProvider->requestTemplate(_aSourceAccessor, aTemplate->getName(), aTemplate->getModule());

        pRet = cloneExpandedForLocale(aTemplateData, m_aOptions.getLocale());
    }
    return pRet;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
namespace
{
    using namespace data;
//-----------------------------------------------------------------------------
    struct TypeDetector : SetVisitor
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
        UnoType type;

        TypeDetector() : result(NotFound), type() {}

    private: // NodeAction implementation
        Result handle(ValueNodeAccess const& _aValueNode);
        Result handle(NodeAccess const& _aNonValueNode);
    };
//-----------------------------------------------------------------------------
    static UnoType detectNodeType(TreeAccessor const& _aElement)
    {
        if (!_aElement.isValid())
            throw configuration::Exception("Could not load required template to detect set elements");

        TypeDetector aDetector;
        aDetector.visitTree( _aElement );

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
    static bool detectElementType(UnoType& aType, data::SetNodeAccess const& _aSet)
    {
        TypeDetector aDetector;
        aDetector.visitElements( _aSet );

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
TemplateHolder TemplateProvider_Impl::makeElementTemplateWithType(TemplateName const& _aNames, data::SetNodeAccess const& _aSet)
{
    typedef TemplateRepository::value_type Entry;

    TemplateRepository::iterator it = m_aRepository.find(_aNames);

    if (it == m_aRepository.end() || !it->second->isInstanceTypeKnown())
    {
        UnoType aType;
        if (_aNames.isSimpleTypeName()) // native type found
        {
            aType = _aNames.resolveToSimpleType();

            if (aType == TemplateImplHelper::getNoTypeAvailable())
                throw configuration::Exception("INTERNAL ERROR: Could not resolve native type");
        }

        else if (!detectElementType(aType,_aSet))
        {
            OSL_ASSERT(_aNames.aName == _aSet.getElementTemplateName());
            OSL_ASSERT(_aNames.aModule == _aSet.getElementTemplateModule());

            data::TreeAccessor aTemplateData = m_xProvider->requestTemplate(_aSet.accessor(), _aNames.aName, _aNames.aModule);

            aType = detectNodeType(aTemplateData); // throws if necessary
        }
        OSL_ASSERT( aType != TemplateImplHelper::getNoTypeAvailable() );

        if (it == m_aRepository.end())
            it = m_aRepository.insert( Entry( _aNames, TemplateImplHelper::createNew(_aNames,aType) ) ).first;

        else
            TemplateImplHelper::assignActualType(*it->second, aType);

        OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
        OSL_ENSURE(it->second->getInstanceType() == aType, "Inconsistent type found for Template");
    }

#ifdef DBG_UTIL
    else
    {
        OSL_ENSURE(it->second->isInstanceTypeKnown(), "No type assigned to Template");
        UnoType aTestType;
        if (detectElementType(aTestType,_aSet))
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
    TypeDetector::Result TypeDetector::handle(ValueNodeAccess const& _aValueNode)
    {
        UnoType aFoundType = _aValueNode.getValueType();

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
        return CONTINUE; // always continue to detect errors in data
    }
//-----------------------------------------------------------------------------
    TypeDetector::Result TypeDetector::handle(NodeAccess const& _aNonValueNode)
    {
        OSL_ENSURE(!ValueNodeAccess::isInstance(_aNonValueNode),"Value node dipatched to wrong handler");
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
        return CONTINUE; // always continue to detect errors in data
    }

//-----------------------------------------------------------------------------
} // anonymous
//-----------------------------------------------------------------------------
    } // configuration
} // configmgr

