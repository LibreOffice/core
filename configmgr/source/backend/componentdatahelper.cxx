/*************************************************************************
 *
 *  $RCSfile: componentdatahelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:46 $
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

#include "componentdatahelper.hxx"

#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef _CONFIGMGR_STRDECL_HXX_
#include "strdecl.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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

DataBuilderContext::DataBuilderContext(  )
: m_aParentStack()
, m_aActiveComponent()
, m_pContext()
, m_aExpectedComponentName(OUString())
{

}
// -----------------------------------------------------------------------------

DataBuilderContext::DataBuilderContext( uno::XInterface * _pContext, ITemplateDataProvider* aTemplateProvider )
: m_aParentStack()
, m_aActiveComponent()
, m_pContext(_pContext)
, m_aExpectedComponentName(OUString())
, m_aTemplateProvider( aTemplateProvider )
{

}
// -----------------------------------------------------------------------------

DataBuilderContext::DataBuilderContext( uno::XInterface * _pContext, const OUString& aExpectedComponentName, ITemplateDataProvider* aTemplateProvider )
: m_aParentStack()
, m_aActiveComponent()
, m_pContext(_pContext)
, m_aExpectedComponentName( aExpectedComponentName )
, m_aTemplateProvider( aTemplateProvider )
{

}
// -----------------------------------------------------------------------------

DataBuilderContext::~DataBuilderContext(  )
{

}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseMalformedDataException(sal_Char const * _pText) const
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    OUString sMessage = OUString::createFromAscii(_pText);
    throw MalformedDataException(sMessage, m_pContext);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseIllegalAccessException(sal_Char const * _pText) const
        CFG_UNO_THROW1( lang::IllegalAccessException )
{
    OUString sMessage = OUString::createFromAscii(_pText);
    throw lang::IllegalAccessException(sMessage, m_pContext);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseIllegalTypeException(sal_Char const * _pText) const
        CFG_UNO_THROW1( beans::IllegalTypeException )
{
    OUString sMessage = OUString::createFromAscii(_pText);
    throw beans::IllegalTypeException(sMessage, m_pContext);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseNoSupportException(sal_Char const * _pText) const
        CFG_UNO_THROW1( lang::NoSupportException )
{
    OUString sMessage = OUString::createFromAscii(_pText);
    throw lang::NoSupportException(sMessage, m_pContext);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseIllegalArgumentException(sal_Char const * _pText, sal_Int16 _nPos) const
        CFG_UNO_THROW1( lang::IllegalArgumentException )
{
    OUString sMessage = OUString::createFromAscii(_pText);
    throw lang::IllegalArgumentException(sMessage, m_pContext, _nPos);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseNoSuchElementException(sal_Char const * _pText, OUString const & _sElement) const
        CFG_UNO_THROW1( container::NoSuchElementException )
{
    throw container::NoSuchElementException(makeMessageWithName(_pText,_sElement), m_pContext);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseElementExistException(sal_Char const * _pText, OUString const & _sElement) const
        CFG_UNO_THROW1( container::ElementExistException )
{
    throw container::ElementExistException(makeMessageWithName(_pText,_sElement), m_pContext);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raiseUnknownPropertyException(sal_Char const * _pText, OUString const & _sElement) const
        CFG_UNO_THROW1( beans::UnknownPropertyException )
{
    throw beans::UnknownPropertyException(makeMessageWithName(_pText,_sElement), m_pContext);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::raisePropertyExistException(sal_Char const * _pText, OUString const & _sElement) const
        CFG_UNO_THROW1( beans::PropertyExistException )
{
    throw beans::PropertyExistException(makeMessageWithName(_pText,_sElement), m_pContext);
}
// -----------------------------------------------------------------------------

OUString DataBuilderContext::makeMessageWithName(sal_Char const * _pText, OUString const & _aName) const
        CFG_UNO_THROW_RTE(  )
{
    rtl::OUStringBuffer sMessage;
    sMessage.appendAscii(_pText);

    if (_aName.getLength() != 0)
        sMessage.appendAscii(" [").append(_aName).appendAscii("]");

    return sMessage.makeStringAndClear();
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
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    if (m_aParentStack.empty())
        raiseMalformedDataException("Invalid Component Data: Operation requires open parent node.");

    OSL_ENSURE( m_aParentStack.top(), "NULL tree on node-stack" );

    return *m_aParentStack.top();
}
// -----------------------------------------------------------------------------

bool DataBuilderContext::isWritable(INode const * pNode) const
        CFG_NOTHROW( )
{
    OSL_PRECOND(pNode,"Unexpected NULL node pointer");
    return pNode->getAttributes().bWritable;
}
// -----------------------------------------------------------------------------

bool DataBuilderContext::isRemovable(ISubtree const * pItem) const
        CFG_NOTHROW( )
{
    OSL_PRECOND(pItem,"Unexpected NULL item pointer");
    return pItem->getAttributes().bNullable || pItem->getAttributes().isReplacedForUser();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

OUString DataBuilderContext::getTemplateComponent( const TemplateIdentifier& aItemType ) const
{
    if (aItemType.Component.getLength() != 0)
        return aItemType.Component;

    else
        return getActiveComponent();
}
// -----------------------------------------------------------------------------

TemplateIdentifier DataBuilderContext::stripComponent( const TemplateIdentifier& aItemType ) const
{
    TemplateIdentifier aStripped(aItemType);
    if (aStripped.Component == getActiveComponent() )
        aStripped.Component = OUString();

    return aStripped;
}
// -----------------------------------------------------------------------------

TemplateIdentifier DataBuilderContext::completeComponent( const TemplateIdentifier& aItemType ) const
{
    TemplateIdentifier aComplete(aItemType);
    if (aComplete.Component.getLength() == 0)
        aComplete.Component = getActiveComponent();

    return aComplete;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

TemplateIdentifier DataBuilderContext::getCurrentItemType() const
                CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    ISubtree const * pCurrentSet = getCurrentParent().asISubtree();
    if (!pCurrentSet || !pCurrentSet->isSetNode())
        raiseMalformedDataException("Component Builder Context: Cannot add/replace node - context is not a set");

    TemplateIdentifier aCompleteType;

    aCompleteType.Name      = pCurrentSet->getElementTemplateName();
    aCompleteType.Component = pCurrentSet->getElementTemplateModule();

    return aCompleteType;
}
// -----------------------------------------------------------------------------

TemplateIdentifier DataBuilderContext::getValidItemType(TemplateIdentifier const & aItemType) const
                CFG_THROW3( MalformedDataException, beans::IllegalTypeException, uno::RuntimeException )
{
    ISubtree const * pCurrentSet = getCurrentParent().asISubtree();
    if (!pCurrentSet || !pCurrentSet->isSetNode())
        raiseMalformedDataException("Component Builder Context: Cannot add/replace node - context is not a set");

    TemplateIdentifier aCompleteType = completeComponent( aItemType );

    // for now only a single item-type is supported
    if (aCompleteType.Name != pCurrentSet->getElementTemplateName())
        raiseIllegalTypeException("Component Builder Context: Cannot add/replace node - template is not permitted in containing set");

    if (aCompleteType.Component != pCurrentSet->getElementTemplateModule())
        raiseIllegalTypeException("Component Builder Context: Cannot add/replace node - template is not permitted in containing set (component mismatch)");

    return aCompleteType;
}
// -----------------------------------------------------------------------------

ISubtree  * DataBuilderContext::addNodeToCurrent(std::auto_ptr<ISubtree>  _aNode)
    CFG_THROW3( MalformedDataException, container::ElementExistException, uno::RuntimeException )
{
    OSL_PRECOND(_aNode.get(), "ERROR: Adding a NULL node");

    if (this->findChild(_aNode->getName()))
        raiseElementExistException("Component Builder Context: The node to be added does already exist", _aNode->getName());

    return getCurrentParent().addChild( base_ptr(_aNode) )->asISubtree();
}
// -----------------------------------------------------------------------------

ISubtree  * DataBuilderContext::addLocalizedToCurrent(std::auto_ptr<ISubtree>  _aNode)
    CFG_THROW3( MalformedDataException, beans::PropertyExistException, uno::RuntimeException )
{
    OSL_PRECOND(_aNode.get(), "ERROR: Adding a NULL node");

    if (this->findChild(_aNode->getName()))
        raisePropertyExistException("Component Builder Context: The property to be added does already exist", _aNode->getName());

    return getCurrentParent().addChild( base_ptr(_aNode) )->asISubtree();
}
// -----------------------------------------------------------------------------

ValueNode * DataBuilderContext::addPropertyToCurrent(std::auto_ptr<ValueNode> _aNode, bool _bMayReplace)
    CFG_THROW3( MalformedDataException, beans::PropertyExistException, uno::RuntimeException )
{
    OSL_PRECOND(_aNode.get(), "ERROR: Adding a NULL node");

    if (this->findChild(_aNode->getName()))
    {
        // We currently may get a 'replace', when overriding an added property
        if (_bMayReplace && getCurrentParent().isSetNode())
            getCurrentParent().removeChild(_aNode->getName());
        else

            raisePropertyExistException("Component Builder Context: The property to be added does already exist", _aNode->getName());
    }
    return getCurrentParent().addChild( base_ptr(_aNode) )->asValueNode();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

bool DataBuilderContext::isProperty(INode * pProp) const
        CFG_UNO_THROW_RTE(  )
{
    OSL_PRECOND(pProp, "ERROR: Unexpected NULL node");

    if (pProp == NULL)
    {
        OUString sMsg =  makeMessageWithName("INTERNAL ERROR (DataBuilderContext): Trying to inspect NULL node",OUString() );
        throw uno::RuntimeException( sMsg, m_pContext );
    }

    if ( ISubtree * pTree = pProp->asISubtree() )
        return isLocalizedValueSet( *pTree );

    OSL_ENSURE( pProp->ISA(ValueNode), "Unexpected node type");
    return true;
}
// -----------------------------------------------------------------------------

INode * DataBuilderContext::findChild(OUString const & _aName)
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    return getCurrentParent().getChild(_aName);
}
// -----------------------------------------------------------------------------

INode * DataBuilderContext::findProperty(OUString const & _aName)
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    INode * pResult = findChild(_aName);
    if (pResult && !isProperty(pResult))
    {
        raiseMalformedDataException("Component Builder Context: Found an existing inner node, where a property was expected");
    }
    return pResult;
}
// -----------------------------------------------------------------------------

ISubtree * DataBuilderContext::findNode(OUString const & _aName)
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    INode * pResult = findChild(_aName);

    if (!pResult)
        return NULL;

    if (!isNode(pResult))
        raiseMalformedDataException("Component Builder Context: Found an existing property, where an inner node was expected");

    OSL_ASSERT(pResult->ISA(ISubtree));
    return pResult->asISubtree();
}
// -----------------------------------------------------------------------------

void DataBuilderContext::pushNode(ISubtree * pTree)
        CFG_UNO_THROW_RTE(  )
{
    OSL_PRECOND(hasActiveComponent(), "Component Builder Context: Entering a node without having an active component");
    OSL_PRECOND(pTree, "ERROR: Pushing a NULL tree");

    if (pTree == NULL)
    {
        OUString sMsg =  makeMessageWithName("INTERNAL ERROR (DataBuilderContext): Trying to push NULL tree",OUString() );
        throw uno::RuntimeException( sMsg, m_pContext );
    }
    m_aParentStack.push(pTree);
}
// -----------------------------------------------------------------------------

void DataBuilderContext::popNode()
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    OSL_PRECOND(hasActiveComponent(), "Component Builder Context: Leaving a node without having an active component");
    if (m_aParentStack.empty())
        raiseMalformedDataException("Invalid Component Data: Unmatched end of node");

    OSL_ENSURE( m_aParentStack.top(), "NULL tree on node-stack" );

    m_aParentStack.pop();
}
// -----------------------------------------------------------------------------

void DataBuilderContext::startActiveComponent(OUString const & _aComponent)
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
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
        CFG_THROW2( MalformedDataException, uno::RuntimeException )
{
    OSL_PRECOND( hasActiveComponent(), "Component Builder Context: No Component active");
    OSL_PRECOND(m_aParentStack.empty(), "Component Builder Context: Ending Component/Template while inside a node");

    if (!m_aParentStack.empty())
        raiseMalformedDataException("Invalid Component Data: Ending component while node is still open");

    m_aActiveComponent = OUString();

    OSL_POSTCOND(!hasActiveComponent(), "Component Builder Context: Could not end Component/Template");
}
// -----------------------------------------------------------------------------
TemplateResult DataBuilderContext::getTemplateData (TemplateRequest const & _aRequest  )
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

std::auto_ptr<ISubtree> ComponentDataFactory::createGroup( OUString const & _aName,
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

std::auto_ptr<ISubtree> ComponentDataFactory::createSet(OUString const & _aName,
                                                        TemplateIdentifier const & _aItemType,
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

std::auto_ptr<ISubtree> ComponentDataFactory::createLocalizedContainer( OUString const & _aName,
                                                                        uno::Type const & _aValueType,
                                                                        node::Attributes const & _aAttributes) const
{
    node::Attributes aLocalizedAttributes(_aAttributes);
    aLocalizedAttributes.bLocalized = true;

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

std::auto_ptr<ISubtree> ComponentDataFactory::createPlaceHolder(OUString const & _aName,
                                                                TemplateIdentifier const & _aInstanceType) const
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

TemplateIdentifier ComponentDataFactory::getInstanceType(ISubtree const & _aInstanceTree)
{
    OSL_ENSURE( isInstancePlaceHolder(_aInstanceTree), "Instance placeholder tree expected" );

    TemplateIdentifier aResult;

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
