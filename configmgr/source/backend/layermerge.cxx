/*************************************************************************
 *
 *  $RCSfile: layermerge.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:35:35 $
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

#include "layermerge.hxx"

#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif

#ifndef CONFIGMGR_VALUECONVERTER_HXX
#include "valuetypeconverter.hxx"
#endif

#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_SCHEMAATTRIBUTE_HPP_
#include <com/sun/star/configuration/backend/SchemaAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_NODEATTRIBUTE_HPP_
#include <com/sun/star/configuration/backend/NodeAttribute.hpp>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

        namespace SchemaAttribute   = backenduno::SchemaAttribute;
        namespace NodeAttribute     = backenduno::NodeAttribute;
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
        static void check_if_complete(uno::Reference< lang::XMultiServiceFactory > const & _xServiceFactory)
        {
            MergedComponentData aData;

            uno::Reference< backenduno::XLayerHandler >
                test(new LayerMergeHandler(_xServiceFactory, aData));
        }

// -----------------------------------------------------------------------------

struct LayerMergeHandler::Converter
{
    typedef uno::Reference< com::sun::star::script::XTypeConverter > TypeConverter;

    explicit
    Converter(ServiceFactory const & _xServiceFactory);

    uno::Any convertValue(uno::Type const & _aTargetType, uno::Any const & _aValue);

    static TypeConverter createTCV(ServiceFactory const & _xServiceFactory);

    ValueConverter m_aConverter;
    bool m_bConvertData;
};
// -----------------------------------------------------------------------------
LayerMergeHandler::LayerMergeHandler(ServiceFactory const & _xServiceFactory, MergedComponentData & _rData, ITemplateDataProvider* aTemplateProvider )
: m_rData(_rData)
, m_aContext(static_cast<backenduno::XLayerHandler*>(this),aTemplateProvider )
, m_aFactory()
, m_aLocale()
, m_pProperty(NULL)
, m_pConverter( new Converter(_xServiceFactory) )
, m_nSkipping(0)
, m_bSublayer(false)
{
    OSL_ENSURE( m_rData.hasSchema(), "Creating layer merger without default data" );
}
// -----------------------------------------------------------------------------

LayerMergeHandler::~LayerMergeHandler(  )
{
    delete m_pConverter;
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::prepareLayer()
{
    OSL_ENSURE(isDone(), "LayerMergeHandler: Warning: Previous layer or schema not terminated properly");
    m_aLocale = localehelper:: getDefaultLanguage();
    m_bSublayer = false;

    promoteToDefault(m_rData);
}
// -----------------------------------------------------------------------------

bool LayerMergeHandler::prepareSublayer(OUString const & aLocale)
{
    OSL_ENSURE(isDone(), "LayerMergeHandler: Warning: Previous layer not terminated properly");

    m_aLocale = aLocale;
    m_bSublayer = (aLocale.getLength() != 0);

    return m_bSublayer;
}
// -----------------------------------------------------------------------------

MergedComponentData & LayerMergeHandler::result()
{
    OSL_ENSURE(isDone(), "LayerMergeHandler: Warning: Layer not terminated properly");

    return m_rData;
}
// -----------------------------------------------------------------------------

MergedComponentData const & LayerMergeHandler::result() const
{
    OSL_ENSURE(isDone(), "LayerMergeHandler: Warning: Layer not terminated properly");

    return m_rData;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

namespace
{
    struct CheckRestrictedAccessVisitor : NodeAction
    {
        node::Access m_access;

        CheckRestrictedAccessVisitor(node::Access _access) : m_access(_access) {}

        void handle(ValueNode const & _aNode) { check(_aNode); }

        void handle(ISubtree const & _aNode)
        {
            node::Access aNext = check(_aNode);
            CheckRestrictedAccessVisitor(aNext).applyToChildren(_aNode);
        }

        node::Access check(INode const & _aNode)
        {
            node::Attributes const aFoundAttr = _aNode.getAttributes();
            node::Access const aFoundAccess = aFoundAttr.getAccess();
            OSL_ENSURE(m_access <= aFoundAccess, "Subnode has more access than its parent");

            return aFoundAccess;
        }
    };
// --------------------
    struct RestrictAccessVisitor : NodeModification
    {
        node::Access m_access;

        RestrictAccessVisitor(bool _bFinalize)
        : m_access(_bFinalize ? node::accessFinal : node::accessReadonly)
        {}

        void handle(ValueNode & _aNode) { restrict(_aNode); }

        void handle(ISubtree  & _aNode)
        {
            if (restrict(_aNode))
                this->applyToChildren(_aNode);
            else
                OSL_DEBUG_ONLY(CheckRestrictedAccessVisitor(m_access).applyToNode(_aNode));
        }

        bool restrict(INode & _aNode)
        {
            node::Attributes const aFoundAttr = _aNode.getAttributes();

            if (aFoundAttr.getAccess() >= m_access) return false; // already restricted enough

            _aNode.modifyAccess(m_access);
            return true;
        }
    };
}
// -----------------------------------------------------------------------------
void LayerMergeHandler::propagateAttributes(ISubtree & _rParent)
{
    node::Attributes aAttributes = _rParent.getAttributes();

    if (aAttributes.isReadonly() || aAttributes.isFinalized())
        RestrictAccessVisitor(aAttributes.isWritable()).applyToChildren(_rParent);
}
// -----------------------------------------------------------------------------

node::Attributes LayerMergeHandler::makePropertyAttributes(sal_Int16 aSchemaAttributes)
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    const sal_uInt16 k_allPropertySchemaAttributes =
        SchemaAttribute::REQUIRED;

    if ((aSchemaAttributes & k_allPropertySchemaAttributes) !=
        (aSchemaAttributes & SchemaAttribute::MASK))
    {
        sal_Char const * pMsg = (aSchemaAttributes & SchemaAttribute::LOCALIZED) ?
            "Layer merging: Cannot add localized property to extensible node" :
            "Layer merging: Unreckognized Schema Attribute for new Property" ;

        m_aContext.raiseIllegalArgumentException(pMsg,2);
    }
    OSL_ASSERT( !(aSchemaAttributes & SchemaAttribute::LOCALIZED) ); // check the check

    node::Attributes aAttributes = m_aContext.getCurrentAttributes();

    if (aSchemaAttributes & SchemaAttribute::REQUIRED)
        aAttributes.setNullable (false);

    //Set removable and mandatory attribute flags
    aAttributes.setRemovability(true,true);


    return aAttributes;

}
// -----------------------------------------------------------------------------

void LayerMergeHandler::checkPropertyType(uno::Type const & _aType)
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    OSL_ASSERT(m_pProperty);

    if (ValueNode * pValue = m_pProperty->asValueNode())
    {
        if (pValue->getValueType() != _aType)
        {
            if (pValue->getValueType().getTypeClass() == uno::TypeClass_ANY)
            {
                OSL_ENSURE( pValue->isNull(), "Layer merging: Non-null 'any' value" );

                if (_aType != uno::Type())
                    OSL_VERIFY( pValue->setValueType(_aType) );

                else
                    OSL_TRACE("Layer merging: Illegal property type: VOID overriding ANY");
                    // m_aContext.raiseIllegalTypeException("Layer merging: Illegal property type: VOID overriding ANY");
            }
            else if (_aType == uno::Type() && m_pConverter)
                m_pConverter->m_bConvertData = true;

            else
                m_aContext.raiseIllegalTypeException("Layer merging: Cannot merge property value: types does not match");
        }
    }
    else if (ISubtree *localisedSet = m_pProperty->asISubtree()) {
        // We're dealing with localised data.
        uno::Type valueType = parseTemplateName(
                                    localisedSet->getElementTemplateName()) ;

        if (valueType != _aType) {
            if (valueType.getTypeClass() == uno::TypeClass_ANY) {
                if (_aType == uno::Type()) {
                    // VOID value
                    m_aContext.raiseIllegalTypeException(
                        "Layer merging: VOID value for localised ANY type") ;
                }
                // TODO Could we have to set the localised data type?
            }
            else if (_aType == uno::Type() && m_pConverter) {
                m_pConverter->m_bConvertData = sal_True ;
            }
            else {
                m_aContext.raiseIllegalTypeException("Layer merging: property value does not match localised type") ;
            }
        }
    }

}
// -----------------------------------------------------------------------------

void LayerMergeHandler::setValueAndCheck(ValueNode& _rValueNode, uno::Any const & _aValue)
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    if (_aValue.hasValue() && m_pConverter && m_pConverter->m_bConvertData)
    {
        uno::Any aConvertedValue = m_pConverter->convertValue(_rValueNode.getValueType(),_aValue);
        if (!aConvertedValue.hasValue())
            m_aContext.raiseIllegalTypeException("Layer merging: Cannot merge property value: cannot convert data to type of property");

        if (! _rValueNode.setValue(aConvertedValue) )
            m_aContext.raiseIllegalTypeException("Layer merging: Cannot merge property value: converted type does not match");
    }
    else if (! _rValueNode.setValue(_aValue) )
    {
        m_aContext.raiseIllegalTypeException("Layer merging: Cannot merge property value: type does not match");
    }

}
// -----------------------------------------------------------------------------

void LayerMergeHandler::setLocalizedValue(ISubtree * pProperty, uno::Any const & _aValue, OUString const & _aLocale)
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    if (ISubtree * pLocalizedCont = pProperty->asISubtree())
    {
        OSL_ENSURE(isLocalizedValueSet(*pLocalizedCont),"Layer merging: property node is not a value");

        if (INode * pLocale = pLocalizedCont->getChild(_aLocale))
        {
            if (ValueNode * pLocValue = pLocale->asValueNode())
            {
                setValueAndCheck(*pLocValue,_aValue);
            }
            else
                OSL_ENSURE(false,"Layer merging: Localized subnode is not a value");
        }
        else {
            node::Attributes attributes = pLocalizedCont->getAttributes() ;
            uno::Type valueType = parseTemplateName(
                                    pLocalizedCont->getElementTemplateName()) ;

            attributes.setLocalized(false) ;
            OSL_ENSURE(valueType != uno::Type(),
                                "Cannot determine type for localised value") ;
            std::auto_ptr<ValueNode> localisedValue =
                m_aFactory.getNodeFactory().createNullValueNode(_aLocale,
                                                                valueType,
                                                                attributes) ;

            if (_aValue.hasValue()) {
                setValueAndCheck(*localisedValue, _aValue) ;
            }
            pLocalizedCont->addChild(base_ptr(localisedValue)) ;
        }
    }

    else if (ValueNode * pValue = pProperty->asValueNode())
    {
        OSL_ENSURE(false, "Layer merging: Got locale-dependent value for non localized node");
        setValueAndCheck(*pValue,_aValue);
    }

    else
        OSL_ENSURE(false, "Layer merging: Unknown node type for localized node");
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::applyPropertyValue(uno::Any const & _aValue)
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    OSL_ASSERT(m_pProperty);

    if (ValueNode * pValue = m_pProperty->asValueNode())
    {
        setValueAndCheck(*pValue,_aValue);
    }

    else if (ISubtree * pLocalizedCont = m_pProperty->asISubtree())
    {
        setLocalizedValue(pLocalizedCont,_aValue,m_aLocale);
    }

    else
        OSL_ENSURE(false, "Layer merging: Unknown node type for property");
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::applyPropertyValue(uno::Any const & _aValue, OUString const & _aLocale)
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    OSL_ASSERT(m_pProperty);

    if (_aLocale.getLength() == 0)
        m_aContext.raiseIllegalArgumentException("Locale string is empty");

    if (ISubtree * pLocalizedCont = m_pProperty->asISubtree())
    {
        setLocalizedValue(pLocalizedCont,_aValue, _aLocale);
    }

    else if (ValueNode * pValue = m_pProperty->asValueNode())
    {
        //OSL_ENSURE(false, "Layer merging: Got locale-dependent value for non localized node");
        setValueAndCheck(*pValue,_aValue);
    }

    else
        OSL_ENSURE(false, "Layer merging: Unknown node type for localized property");
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::applyAttributes(INode * pNode, sal_Int16 aNodeAttributes)
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    sal_Int16 const k_allNodeAttributes =
            NodeAttribute::MANDATORY |
            NodeAttribute::FINALIZED |
            NodeAttribute::READONLY;

    if ((aNodeAttributes & k_allNodeAttributes) !=
        (aNodeAttributes & NodeAttribute::MASK))
    {
        sal_Char const * pMsg =
            "Layer merging: Unreckognized Node Attribute" ;

        m_aContext.raiseIllegalArgumentException(pMsg,2);
    }

    if (aNodeAttributes & NodeAttribute::READONLY)
    {
        OSL_ENSURE(!(aNodeAttributes & NodeAttribute::FINALIZED),
                    "Layer merging: Warning: Node is both read-only and finalized");

        pNode->modifyAccess(node::accessReadonly);
    }
    else if (aNodeAttributes & NodeAttribute::FINALIZED)
    {
        pNode->modifyAccess(node::accessFinal);
    }

    if ( m_aContext.isNode(pNode) )
    {
        if (aNodeAttributes & NodeAttribute::MANDATORY)
        {
            pNode->markMandatory();
        }
    }
    else if (aNodeAttributes) // do this only if there actually was something to do
    {
        if (ISubtree * pLocCont = pNode->asISubtree())
        {
            OSL_ENSURE(isLocalizedValueSet(*pLocCont),"Layer merging: Property subtree must be a localized value set");
            propagateAttributes(*pLocCont);
        }
    }


}
// -----------------------------------------------------------------------------

bool LayerMergeHandler::startOverride(INode * pNode, sal_Bool bClear) /* ensure writable, mark merged */
    CFG_NOTHROW( )
{
    if (!m_aContext.isWritable(pNode)) return false;

    if (pNode->isDefault()) pNode->modifyState( node::isMerged );

    OSL_ENSURE(!bClear,"'clear' operation is not yet supported");

    return true;
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::ensureUnchanged(INode const * pNode) const
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    // to do: change state handling to detect this within sets
    OSL_PRECOND(pNode,"INTERNAL ERROR: Unexpected NULL node pointer");

    if (!this->isInSublayer())
        if (pNode->getAttributes().state() == node::isMerged)
            m_aContext.raiseMalformedDataException("Layer merging: Duplicate node or property in this layer");
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// XLayerHandler

void SAL_CALL LayerMergeHandler::startLayer( )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    ISubtree * pSchema = m_rData.getSchemaTree();
    OSL_ENSURE(pSchema,"No base data to merge layer into");

    if (!pSchema)
        throw uno::RuntimeException(OUString::createFromAscii("Layer merging: No data to merge with"),*this);

    m_aContext.startActiveComponent(pSchema->getName());

    m_pProperty = NULL;
    m_nSkipping = 0;

    OSL_POSTCOND( m_aContext.hasActiveComponent(),  "Layer merging: could not set active component");
    OSL_POSTCOND( m_aContext.isDone(),              "Layer merging: newly started component is not empty");
    OSL_POSTCOND( !this->isSkipping(),              "Layer merging: newly started component is in skipping state");
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::endLayer( )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        m_aContext.raiseMalformedDataException("Layer merging: Unmatched data being skipped was not terminated properly.");

    m_aContext.endActiveComponent();

    m_bSublayer = false;

    OSL_POSTCOND( !m_aContext.hasActiveComponent(), "Layer merging: could not clear active component");
    OSL_POSTCOND(  m_aContext.isDone(), "Layer merging: could not finish processing");
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::overrideLayerRoot( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    OSL_PRECOND( m_aContext.hasActiveComponent(),  "Layer merging: active component is not set");
    OSL_PRECOND( m_aContext.isDone(), "Layer merging: node is not root");

    if (m_aContext.getActiveComponent() != aName)
        m_aContext.raiseIllegalArgumentException("Layer merging: Name of layer being merged does not match component name",1);

    // check the argument
    if (ISubtree * pSchema = m_rData.getSchemaTree())
    {
        OSL_ENSURE(pSchema->getName() == aName,"Schema name does not match active component");

        ensureUnchanged(pSchema);

        if (startOverride(pSchema,bClear))
        {
            applyAttributes(pSchema,aAttributes);

            m_aContext.pushNode(pSchema);

            OSL_POSTCOND( m_aContext.hasActiveComponent(),  "Layer merging: could not set active component");
            OSL_POSTCOND( !m_aContext.isDone(),             "Layer merging: could not start component");
        }
        else
            this->skipNode();
    }
    else
    {
        OSL_ENSURE(false,"No base data to merge layer into");
        this->skipNode();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
    {
        this->skipNode();
    }
    else if (m_aContext.isDone())
    {
        this->overrideLayerRoot(aName,aAttributes,bClear);
    }
    else if (ISubtree * pNode = m_aContext.findNode(aName))
    {
        ensureUnchanged(pNode);

        if (startOverride(pNode,bClear))
        {
            applyAttributes(pNode, aAttributes);

            m_aContext.pushNode(pNode);
        }
        else
            this->skipNode();
    }
    else // ignore non-matched data
    {
        OSL_ENSURE(false,"Layer merging: The node to be overridden does not exist.");
        // m_aContext.raiseNoSuchElementException("Layer merging: The node to be overridden does not exist.",aName);
        this->skipNode();
    }
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::implAddOrReplaceNode( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    CFG_UNO_THROW1( configuration::backend::MalformedDataException )
{
    ISubtree * pReplacedNode = m_aContext.findNode(aName);
    if (pReplacedNode)
    {
        this->ensureUnchanged(pReplacedNode);

        if (!m_aContext.isRemovable(pReplacedNode))
        {
            this->skipNode();
            return;
        }
    }

    std::auto_ptr<INode> apNewInstance;
    if (aTemplate.Component == m_aContext.getActiveComponent())
    {
        apNewInstance = m_rData.instantiateTemplate(aName, aTemplate.Name);
    }
    else
    {
        TemplateRequest aTemplateRequest(configuration::makeName(aTemplate.Name, configuration::Name::NoValidate()),
                                         configuration::makeName(aTemplate.Component, configuration::Name::NoValidate()) );
        apNewInstance = m_aContext.getTemplateData( aTemplateRequest ).extractDataAndClear();
        if (apNewInstance.get())
            apNewInstance->setName( aName );
    }

    if (NULL == apNewInstance.get())
        m_aContext.raiseNoSuchElementException("Layer merging: Cannot instantiate template.", aTemplate.Name);

    applyAttributes(apNewInstance.get(), aAttributes);
    //Set removable flag
    apNewInstance->markRemovable();



    if (pReplacedNode) m_aContext.getCurrentParent().removeChild( aName );

    INode * pAddedInstance = m_aContext.getCurrentParent().addChild( apNewInstance );

    m_aContext.pushNode(pAddedInstance->asISubtree());
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
    {
        this->skipNode();
        return;
    }

    implAddOrReplaceNode( aName, m_aContext.getCurrentItemType(), aAttributes);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
    {
        this->skipNode();
        return;
    }

    // TODO: correct argument position (from 2 to 3) for an illegal argument exception wrt attributes
    implAddOrReplaceNode( aName, m_aContext.getValidItemType(aTemplate), aAttributes);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::endNode( )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->leaveSkippedNode())
        return;

    this->propagateAttributes(m_aContext.getCurrentParent());

    m_aContext.popNode();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::dropNode( const OUString& aName )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    if (!m_aContext.getCurrentParent().isSetNode())
        m_aContext.raiseMalformedDataException("Layer merging: Removing child nodes is only possible in set nodes.");

    if (ISubtree * pDropped = m_aContext.findNode(aName))
    {
        this->ensureUnchanged(pDropped);
        if (!m_aContext.isRemovable(pDropped))
            return;
    }
    else
    {
        // m_aContext.raiseNoSuchElementException("Layer merging: The node to be removed does not exist.",aName);
        OSL_TRACE("Layer merging: The node to be removed does not exist.");
    }
    m_aContext.getCurrentParent().removeChild(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
    {
        this->skipNode();
    }
    else if (INode * pProp = m_aContext.findProperty(aName))
    {
        ensureUnchanged(pProp);

        if (startOverride(pProp,bClear))
        {
            applyAttributes(pProp,aAttributes);

            m_pProperty = pProp;

            checkPropertyType(aType);
        }
        else
            this->skipNode();
    }
    else // ignore non-matched data
    {
        OSL_TRACE("Layer merging: The property to be overridden does not exist.");
        //   m_aContext.raiseUnknownPropertyException("Layer merging: The property to be overridden does not exist.",aName);
        this->skipNode();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::endProperty( )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->leaveSkippedNode())
        return;

    if (!m_pProperty)
        m_aContext.raiseMalformedDataException("Layer merging: Invalid data: Ending a property that wasn't started.");

    if (ISubtree * pLocalizedSet = m_pProperty->asISubtree())
        this->propagateAttributes(*pLocalizedSet);

    m_pProperty = NULL;
    if (m_pConverter) m_pConverter->m_bConvertData = false;
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    // TODO: add type validation
    node::Attributes aValueAttributes = makePropertyAttributes(aAttributes & SchemaAttribute::MASK);

    std::auto_ptr<ValueNode> aPropertyValue =
        m_aFactory.getNodeFactory().createNullValueNode(aName,aType,aValueAttributes);

    applyAttributes(aPropertyValue.get(),aAttributes & NodeAttribute::MASK);

    // can be a replace for dynamic properties (current update limitation)
    m_aContext.addPropertyToCurrent(aPropertyValue, true);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    node::Attributes aValueAttributes = makePropertyAttributes(aAttributes & SchemaAttribute::MASK);

    std::auto_ptr<ValueNode> aPropertyValue =
        m_aFactory.getNodeFactory().createValueNode(aName,aValue,aValueAttributes);

    applyAttributes(aPropertyValue.get(),aAttributes & NodeAttribute::MASK);

    // can be a replace for dynamic properties (current update limitation)
    m_aContext.addPropertyToCurrent(aPropertyValue, true);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::setPropertyValue( const uno::Any& aValue )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    if (!m_pProperty)
        m_aContext.raiseMalformedDataException("Layer merging: Invalid data: Overriding a value without a property.");

    applyPropertyValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::setPropertyValueForLocale( const uno::Any& aValue, OUString const & aLocale )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    if (!m_pProperty)
        m_aContext.raiseMalformedDataException("Layer merging: Invalid data: Overriding a (localized) value without a property.");

    applyPropertyValue(aValue,aLocale);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

LayerMergeHandler::Converter::TypeConverter
    LayerMergeHandler::Converter::createTCV(ServiceFactory const & _xServiceFactory)
{
    OSL_ENSURE(_xServiceFactory.is(),"Cannot create TypeConverter for LayerMergeHandler without a ServiceManager");

    TypeConverter xTCV;
    if (_xServiceFactory.is())
    {
        static const rtl::OUString k_sTCVService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter"));

        xTCV = TypeConverter::query(_xServiceFactory->createInstance(k_sTCVService));
    }
    return xTCV;
}
// -----------------------------------------------------------------------------

LayerMergeHandler::Converter::Converter(ServiceFactory const & _xServiceFactory)
: m_aConverter( createTCV(_xServiceFactory) )
, m_bConvertData(false)
{
}
// -----------------------------------------------------------------------------
typedef uno::Sequence< sal_Int8 > Binary;
// -----------------------------------------------------------------------------
static
inline
uno::Type getBinaryDataType()
{
    Binary const * const forBinary = 0;
    return ::getCppuType(forBinary);
}
// -----------------------------------------------------------------------------

uno::Any LayerMergeHandler::Converter::convertValue(uno::Type const & _aTargetType, uno::Any const & _aValue)
{
    OSL_ENSURE( m_bConvertData, "Unexpected: Calling convert data, when data conversion is not active");
    OSL_ENSURE( _aValue.hasValue(), "Unexpected: Calling convert data, when data to convert is VOID");

    if (_aTargetType == _aValue.getValueType()) return _aValue;

    m_aConverter.reset(_aTargetType);

    if (m_aConverter.isList())
    {
        uno::Sequence< OUString > aStringList;
        if (_aValue >>= aStringList)
            return m_aConverter.convertListToAny(aStringList);
    }

    OUString aContent;
    if (_aValue >>= aContent)
        return m_aConverter.convertToAny(aContent);


    OSL_ENSURE(false, "Cannot convert typed value (not a string)");
    return uno::Any();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
namespace
{
// -----------------------------------------------------------------------------
    static inline bool isFinal(node::Attributes const& _aAttributes)
    {
        return _aAttributes.isFinalized() || _aAttributes.isReadonly();
    }
    // --------------------------------- AttributeSetter ---------------------------------

    class DefaultPromoter : NodeModification
    {
    public:
        explicit
        DefaultPromoter()
        {}

        void adjustAccess(INode& _rNode);

        using NodeModification::applyToNode;
    private:
        void handle(ValueNode& _rValueNode);
        void handle(ISubtree& _rSubtree);
    };
// -----------------------------------------------------------------------------

    void DefaultPromoter::adjustAccess(INode& _rNode)
    {
        _rNode.promoteAccessToDefault();
    }
// -----------------------------------------------------------------------------

    void DefaultPromoter::handle(ValueNode& _rValueNode)
    {
        _rValueNode.promoteToDefault();
        adjustAccess(_rValueNode);
    }
// -----------------------------------------------------------------------------

    void DefaultPromoter::handle(ISubtree& _rSubtree)
    {
        _rSubtree.markAsDefault();
        this->applyToChildren(_rSubtree);
        adjustAccess(_rSubtree);
    }
//--------------------------------------------------------------------------

}
// -----------------------------------------------------------------------------
void promoteToDefault(MergedComponentData & _rTree)
{
    if (ISubtree * pTreeData = _rTree.getSchemaTree())
        DefaultPromoter().applyToNode(*pTreeData);

    else
        OSL_ENSURE(false,"No Data to promote to default");

}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
