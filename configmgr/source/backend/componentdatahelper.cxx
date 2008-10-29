/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: componentdatahelper.cxx,v $
 * $Revision: 1.14 $
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

#include "componentdatahelper.hxx"
#include "treenodefactory.hxx"
#include "typeconverter.hxx"
#include "strdecl.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>

// -----------------------------------------------------------------------------
#define OUSTR( str ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(str) )
// -----------------------------------------------------------------------------
namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace beans     = ::com::sun::star::beans;
        namespace container = ::com::sun::star::container;
// -----------------------------------------------------------------------------

DataBuilderContext::DataBuilderContext( uno::Reference< uno::XComponentContext > const & xContext )
: m_aLogger(xContext)
, m_aParentStack()
, m_aActiveComponent()
, m_pContext()
, m_aExpectedComponentName(rtl::OUString())
{

}
// -----------------------------------------------------------------------------

DataBuilderContext::DataBuilderContext( uno::Reference< uno::XComponentContext > const & xContext, uno::XInterface * _pContext, ITemplateDataProvider* aTemplateProvider )
: m_aLogger(xContext)
, m_aParentStack()
, m_aActiveComponent()
, m_pContext(_pContext)
, m_aExpectedComponentName()
, m_aTemplateProvider( aTemplateProvider )
{

}
// -----------------------------------------------------------------------------

DataBuilderContext::DataBuilderContext( uno::Reference< uno::XComponentContext > const & xContext, uno::XInterface * _pContext, const rtl::OUString& aExpectedComponentName, ITemplateDataProvider* aTemplateProvider )
: m_aLogger(xContext)
, m_aParentStack()
, m_aActiveComponent()
, m_pContext(_pContext)
, m_aExpectedComponentName( aExpectedComponentName )
, m_aTemplateProvider( aTemplateProvider )
{

}
// -----------------------------------------------------------------------------

DataBuilderContext::DataBuilderContext(DataBuilderContext const & aBaseContext, uno::XInterface * _pContext)
: m_aLogger(aBaseContext.m_aLogger)
, m_aParentStack()
, m_aActiveComponent()
, m_pContext(_pContext)
, m_aExpectedComponentName()
, m_aTemplateProvider( aBaseContext.m_aTemplateProvider )
{

}
// -----------------------------------------------------------------------------

DataBuilderContext::~DataBuilderContext(  )
{

}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseMalformedDataException(sal_Char const * _pText) const
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException, com::sun::star::uno::RuntimeException))
{
    rtl::OUString const sMessage = makeMessageWithPath(_pText);
    m_aLogger.error(sMessage,"parse","configmgr::backend::DataBuilder");
    throw backenduno::MalformedDataException(sMessage, m_pContext, uno::Any());
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseIllegalTypeException(sal_Char const * _pText) const
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException, com::sun::star::uno::RuntimeException))
{
    rtl::OUString const sMessage = makeMessageWithPath(_pText);
    beans::IllegalTypeException e(sMessage, m_pContext);

    rtl::OUString const sFullMessage = OUSTR("Illegal Type: ").concat(sMessage);
    m_aLogger.error(sFullMessage,"parse","configmgr::backend::DataBuilder");
    throw backenduno::MalformedDataException(sFullMessage, m_pContext, uno::makeAny(e));
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseIllegalArgumentException(sal_Char const * _pText, sal_Int16 _nPos) const
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    rtl::OUString const sMessage = makeMessageWithPath(_pText);
    lang::IllegalArgumentException e(sMessage, m_pContext, _nPos);

    rtl::OUString const sFullMessage = OUSTR("Illegal Argument: ").concat(sMessage);
    m_aLogger.error(sFullMessage,"parse","configmgr::backend::DataBuilder");
    throw backenduno::MalformedDataException(sFullMessage, m_pContext, uno::makeAny(e));
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseNoSuchElementException(sal_Char const * _pText, rtl::OUString const & _sElement) const
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    rtl::OUString const sMessage = makeMessageWithName(_pText,_sElement);
    container::NoSuchElementException e(sMessage, m_pContext);

    rtl::OUString const sFullMessage = OUSTR("No Such Node: ").concat(sMessage);
    m_aLogger.error(sFullMessage,"parse","configmgr::backend::DataBuilder");
    throw backenduno::MalformedDataException(sFullMessage, m_pContext, uno::makeAny(e));
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseElementExistException(sal_Char const * _pText, rtl::OUString const & _sElement) const
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    rtl::OUString const sMessage = makeMessageWithName(_pText,_sElement);
    container::ElementExistException e(sMessage, m_pContext);

    rtl::OUString const sFullMessage = OUSTR("Node Already Exists: ").concat(sMessage);
    m_aLogger.error(sFullMessage,"parse","configmgr::backend::DataBuilder");
    throw backenduno::MalformedDataException(sFullMessage, m_pContext, uno::makeAny(e));
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raisePropertyExistException(sal_Char const * _pText, rtl::OUString const & _sElement) const
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    rtl::OUString const sMessage = makeMessageWithName(_pText,_sElement);
    beans::PropertyExistException e(sMessage, m_pContext);

    rtl::OUString const sFullMessage = OUSTR("No Such Property: ").concat(sMessage);
    m_aLogger.error(sFullMessage,"parse","configmgr::backend::DataBuilder");
    throw backenduno::MalformedDataException(sFullMessage, m_pContext, uno::makeAny(e));
}
// -----------------------------------------------------------------------------

rtl::OUString DataBuilderContext::makeMessageWithPath(sal_Char const * _pText) const
        SAL_THROW((com::sun::star::uno::RuntimeException))
{
    rtl::OUStringBuffer sMessage;
    sMessage.appendAscii(_pText);

    sMessage.appendAscii(" [@").append(getNodeParentagePath()).appendAscii("] ");

    return sMessage.makeStringAndClear();
}
// -----------------------------------------------------------------------------

rtl::OUString DataBuilderContext::makeMessageWithName(sal_Char const * _pText, rtl::OUString const & _aName) const
        SAL_THROW((com::sun::star::uno::RuntimeException))
{
    rtl::OUStringBuffer sMessage;
    sMessage.appendAscii(_pText);

    if (_aName.getLength() != 0)
        sMessage.appendAscii(" [").append(getNodePath(_aName)).appendAscii("] ");
    else
        sMessage.appendAscii(" [@").append(getNodeParentagePath()).appendAscii("] ");

    return sMessage.makeStringAndClear();
}
// -----------------------------------------------------------------------------
const sal_Unicode k_pathsep = '/';

rtl::OUString DataBuilderContext::getNodeParentagePath() const
{
    rtl::OUStringBuffer path;

    for (Stack< ISubtree * >::bottomup_iterator it = m_aParentStack.begin_up();
            it != m_aParentStack.end_up(); ++it)
    {
        OSL_ASSERT(*it);
        path.append(k_pathsep).append((**it).getName());
    }

    return path.makeStringAndClear();
}
// -----------------------------------------------------------------------------

rtl::OUString DataBuilderContext::getNodePath(rtl::OUString const & aNodeName) const
{
    rtl::OUStringBuffer path( getNodeParentagePath() );

    path.append(k_pathsep).append(aNodeName);

    return path.makeStringAndClear();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

bool DataBuilderContext::isDone() const
{
    OSL_ENSURE(m_aParentStack.empty() || hasActiveComponent(),"DataBuilderContext:Inconsistent Activity state");

    return m_aParentStack.empty();
}
// -----------------------------------------------------------------------------

ISubtree & DataBuilderContext::implGetCurrentParent() const
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    if (m_aParentStack.empty())
        raiseMalformedDataException("Invalid Component Data: Operation requires open parent node.");

    OSL_ENSURE( m_aParentStack.top(), "NULL tree on node-stack" );

    return *m_aParentStack.top();
}
// -----------------------------------------------------------------------------

bool DataBuilderContext::isWritable(INode const * pNode) const
        SAL_THROW(())
{
    OSL_PRECOND(pNode,"Unexpected NULL node pointer");
    return pNode->getAttributes().isWritable();

}
// -----------------------------------------------------------------------------

bool DataBuilderContext::isRemovable(ISubtree const * pItem) const
        SAL_THROW(())
{
    OSL_PRECOND(pItem,"Unexpected NULL item pointer");
    return pItem->getAttributes().isRemovable();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

rtl::OUString DataBuilderContext::getTemplateComponent( const backenduno::TemplateIdentifier& aItemType ) const
{
    if (aItemType.Component.getLength() != 0)
        return aItemType.Component;

    else
        return getActiveComponent();
}
// -----------------------------------------------------------------------------

backenduno::TemplateIdentifier DataBuilderContext::completeComponent( const backenduno::TemplateIdentifier& aItemType ) const
{
    backenduno::TemplateIdentifier aComplete(aItemType);
    if (aComplete.Component.getLength() == 0)
        aComplete.Component = getActiveComponent();

    return aComplete;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

backenduno::TemplateIdentifier DataBuilderContext::getCurrentItemType() const
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    ISubtree const * pCurrentSet = getCurrentParent().asISubtree();
    if (!pCurrentSet || !pCurrentSet->isSetNode())
        raiseMalformedDataException("Component Builder Context: Cannot add/replace node - context is not a set");

    backenduno::TemplateIdentifier aCompleteType;

    aCompleteType.Name      = pCurrentSet->getElementTemplateName();
    aCompleteType.Component = pCurrentSet->getElementTemplateModule();

    return aCompleteType;
}
// -----------------------------------------------------------------------------

backenduno::TemplateIdentifier DataBuilderContext::getValidItemType(backenduno::TemplateIdentifier const & aItemType) const
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    ISubtree const * pCurrentSet = getCurrentParent().asISubtree();
    if (!pCurrentSet || !pCurrentSet->isSetNode())
        raiseMalformedDataException("Component Builder Context: Cannot add/replace node - context is not a set");

    backenduno::TemplateIdentifier aCompleteType = completeComponent( aItemType );

    // for now only a single item-type is supported
    if (aCompleteType.Name != pCurrentSet->getElementTemplateName())
        raiseIllegalTypeException("Component Builder Context: Cannot add/replace node - template is not permitted in containing set");

    if (aCompleteType.Component != pCurrentSet->getElementTemplateModule())
        raiseIllegalTypeException("Component Builder Context: Cannot add/replace node - template is not permitted in containing set (component mismatch)");

    return aCompleteType;
}
// -----------------------------------------------------------------------------

ISubtree  * DataBuilderContext::addNodeToCurrent(std::auto_ptr<ISubtree>  _aNode)
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(_aNode.get(), "ERROR: Adding a NULL node");

    if (this->findChild(_aNode->getName()))
        raiseElementExistException("Component Builder Context: The node to be added does already exist", _aNode->getName());

    return getCurrentParent().addChild( base_ptr(_aNode) )->asISubtree();
}
// -----------------------------------------------------------------------------

ISubtree  * DataBuilderContext::addLocalizedToCurrent(std::auto_ptr<ISubtree>  _aNode)
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(_aNode.get(), "ERROR: Adding a NULL node");

    if (this->findChild(_aNode->getName()))
        raisePropertyExistException("Component Builder Context: The property to be added does already exist", _aNode->getName());

    return getCurrentParent().addChild( base_ptr(_aNode) )->asISubtree();
}
// -----------------------------------------------------------------------------

ValueNode * DataBuilderContext::addPropertyToCurrent(std::auto_ptr<ValueNode> _aNode, bool _bMayReplace)
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(_aNode.get(), "ERROR: Adding a NULL node");
    OSL_PRECOND(!_bMayReplace || _aNode->getAttributes().isReplacedForUser(), "Wrong status for added property");

    if (this->findChild(_aNode->getName()))
    {
        // We currently may get a 'replace', when overriding an added property
        if (_bMayReplace && getCurrentParent().isSetNode())
        {
            getCurrentParent().removeChild(_aNode->getName());
            _aNode->modifyState(node::isReplaced);
        }
        else

            raisePropertyExistException("Component Builder Context: The property to be added does already exist", _aNode->getName());
    }
    return getCurrentParent().addChild( base_ptr(_aNode) )->asValueNode();
}
// -----------------------------------------------------------------------------

void DataBuilderContext::markCurrentMerged()
{
    Stack< ISubtree * >::topdown_iterator it = m_aParentStack.begin_down(), end = m_aParentStack.end_down();
    for ( ;it != end && (*it)->isDefault(); ++it)
        (*it)->modifyState( node::isMerged );

#if OSL_DEBUG_LEVEL > 0
    for ( ;it != end; ++it)
        OSL_ENSURE(!(*it)->isDefault(),"Found a default node in ancestry of a merged change");
#endif
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

bool DataBuilderContext::isProperty(INode * pProp) const
        SAL_THROW((com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(pProp, "ERROR: Unexpected NULL node");

    if (pProp == NULL)
    {
        rtl::OUString sMsg =  makeMessageWithName("INTERNAL ERROR (DataBuilderContext): Trying to inspect NULL node",rtl::OUString() );
        throw uno::RuntimeException( sMsg, m_pContext );
    }

    if ( ISubtree * pTree = pProp->asISubtree() )
        return isLocalizedValueSet( *pTree );

    OSL_ASSERT(dynamic_cast< ValueNode * >(pProp) != 0);
    return true;
}
// -----------------------------------------------------------------------------

INode * DataBuilderContext::findChild(rtl::OUString const & _aName)
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    return getCurrentParent().getChild(_aName);
}
// -----------------------------------------------------------------------------

INode * DataBuilderContext::findProperty(rtl::OUString const & _aName)
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    INode * pResult = findChild(_aName);
    if (pResult && !isProperty(pResult))
    {
        raiseMalformedDataException("Component Builder Context: Found an existing inner node, where a property was expected");
    }
    return pResult;
}
// -----------------------------------------------------------------------------

ISubtree * DataBuilderContext::findNode(rtl::OUString const & _aName)
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    INode * pResult = findChild(_aName);

    if (!pResult)
        return NULL;

    if (!isNode(pResult))
        raiseMalformedDataException("Component Builder Context: Found an existing property, where an inner node was expected");

    OSL_ASSERT(dynamic_cast< ISubtree * >(pResult) != 0);
    return pResult->asISubtree();
}
// -----------------------------------------------------------------------------

void DataBuilderContext::pushNode(ISubtree * pTree)
        SAL_THROW((com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(hasActiveComponent(), "Component Builder Context: Entering a node without having an active component");
    OSL_PRECOND(pTree, "ERROR: Pushing a NULL tree");

    if (pTree == NULL)
    {
        rtl::OUString sMsg =  makeMessageWithName("INTERNAL ERROR (DataBuilderContext): Trying to push NULL tree",rtl::OUString() );
        throw uno::RuntimeException( sMsg, m_pContext );
    }
    m_aParentStack.push(pTree);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::popNode()
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(hasActiveComponent(), "Component Builder Context: Leaving a node without having an active component");
    if (m_aParentStack.empty())
        raiseMalformedDataException("Invalid Component Data: Unmatched end of node");

    OSL_ENSURE( m_aParentStack.top(), "NULL tree on node-stack" );

    m_aParentStack.pop();
}
// -----------------------------------------------------------------------------

void DataBuilderContext::startActiveComponent(rtl::OUString const & _aComponent)
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(!hasActiveComponent(), "Component Builder Context: Component is already active");
    OSL_PRECOND(m_aParentStack.empty(),  "Component Builder Context: Starting Component/Template while inside a node");

    if (!m_aParentStack.empty())
        raiseMalformedDataException("Invalid Component Data: Starting component while node is still open");

    if (m_aExpectedComponentName.getLength()!=0)
    {
        if (m_aExpectedComponentName.compareTo ( _aComponent)!= 0 )
            raiseMalformedDataException("Invalid Component Data: Component name does not match request");
    }
    m_aActiveComponent = _aComponent;

    OSL_POSTCOND(hasActiveComponent(), "Component Builder Context: Could not start Component/Template");
}
// -----------------------------------------------------------------------------

void DataBuilderContext::endActiveComponent()
        SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND( hasActiveComponent(), "Component Builder Context: No Component active");
    OSL_PRECOND(m_aParentStack.empty(), "Component Builder Context: Ending Component/Template while inside a node");

    if (!m_aParentStack.empty())
        raiseMalformedDataException("Invalid Component Data: Ending component while node is still open");

    m_aActiveComponent = rtl::OUString();

    OSL_POSTCOND(!hasActiveComponent(), "Component Builder Context: Could not end Component/Template");
}
// -----------------------------------------------------------------------------
ResultHolder< TemplateInstance > DataBuilderContext::getTemplateData (TemplateRequest const & _aRequest  )
{
    return(m_aTemplateProvider->getTemplateData (_aRequest));
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ComponentDataFactory::ComponentDataFactory()
: m_rNodeFactory( getDefaultTreeNodeFactory() )
{
}
// -----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ComponentDataFactory::createGroup( rtl::OUString const & _aName,
                                                            bool _bExtensible,
                                                            node::Attributes const & _aAttributes) const
{
    if (_bExtensible)
    {
        return getNodeFactory().createSetNode( _aName,
                                                toTemplateName(TYPE_ANY,false),
                                                TEMPLATE_MODULE_NATIVE_VALUE,
                                                _aAttributes );
    }
    else
    {
        return getNodeFactory().createGroupNode( _aName,
                                                 _aAttributes );
    }
}
// -----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ComponentDataFactory::createSet(rtl::OUString const & _aName,
                                                        backenduno::TemplateIdentifier const & _aItemType,
                                                        bool _bExtensible,
                                                        node::Attributes const & _aAttributes) const
{
    OSL_ENSURE(!_bExtensible, "DataBuilderContext: Unimplemented feature: Extensible Set node");
    if (_bExtensible)
        return std::auto_ptr<ISubtree>();

    return getNodeFactory().createSetNode( _aName,
                                            _aItemType.Name,
                                            _aItemType.Component,
                                            _aAttributes );
}
// -----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ComponentDataFactory::createLocalizedContainer( rtl::OUString const & _aName,
                                                                        uno::Type const & _aValueType,
                                                                        node::Attributes const & _aAttributes) const
{
    node::Attributes aLocalizedAttributes(_aAttributes);
    aLocalizedAttributes.setLocalized (true);

    return getNodeFactory().createSetNode( _aName,
                                            toTemplateName(_aValueType),
                                            TEMPLATE_MODULE_LOCALIZED_VALUE,
                                            aLocalizedAttributes );

}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

namespace
{
    DECLARE_CONSTASCII_USTRING(INSTANCE_MARKER);
    IMPLEMENT_CONSTASCII_USTRING(INSTANCE_MARKER, "instantiate@");
}
// -----------------------------------------------------------------------------

std::auto_ptr<ISubtree> ComponentDataFactory::createPlaceHolder(rtl::OUString const & _aName,
                                                                backenduno::TemplateIdentifier const & _aInstanceType) const
{
    return getNodeFactory().createSetNode( _aName,
                                            INSTANCE_MARKER + _aInstanceType.Name,
                                            _aInstanceType.Component,
                                            node::Attributes());
}
// -----------------------------------------------------------------------------

bool ComponentDataFactory::isInstancePlaceHolder(ISubtree const & _aInstanceTree)
{
    return !! _aInstanceTree.getElementTemplateName().match(INSTANCE_MARKER);
}
// -----------------------------------------------------------------------------

backenduno::TemplateIdentifier ComponentDataFactory::getInstanceType(ISubtree const & _aInstanceTree)
{
    OSL_ENSURE( isInstancePlaceHolder(_aInstanceTree), "Instance placeholder tree expected" );

    backenduno::TemplateIdentifier aResult;

    if (isInstancePlaceHolder(_aInstanceTree))
    {
        aResult.Name      = _aInstanceTree.getElementTemplateName().copy( INSTANCE_MARKER.getLength() );
        aResult.Component = _aInstanceTree.getElementTemplateModule();
    }

    return aResult;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
