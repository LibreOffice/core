/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layermerge.cxx,v $
 * $Revision: 1.28 $
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

#include "layermerge.hxx"
#include "treenodefactory.hxx"
#include "matchlocale.hxx"
#include "valuetypeconverter.hxx"
#include "typeconverter.hxx"
#include <com/sun/star/configuration/backend/SchemaAttribute.hpp>
#include <com/sun/star/configuration/backend/NodeAttribute.hpp>

#include <rtl/ustrbuf.hxx>

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
//#if OSL_DEBUG_LEVEL > 0
// currently not used in debug builds
#if 0
        static void check_if_complete(uno::Reference< uno::XComponentContext > const & _xContext)
        {
            MergedComponentData aData;

            uno::Reference< backenduno::XLayerHandler >
                test(new LayerMergeHandler(_xContext, aData));
        }
#endif

// -----------------------------------------------------------------------------

struct LayerMergeHandler::Converter
{
    explicit
    Converter(uno::Reference< uno::XComponentContext > const & xContext);

    uno::Any convertValue(uno::Type const & _aTargetType, uno::Any const & _aValue);

    static uno::Reference< com::sun::star::script::XTypeConverter > createTCV(uno::Reference< uno::XComponentContext > const & xContext);

    ValueConverter m_aConverter;
    bool m_bConvertData;
};
// -----------------------------------------------------------------------------
LayerMergeHandler::LayerMergeHandler(uno::Reference< uno::XComponentContext > const & xContext, MergedComponentData & _rData, ITemplateDataProvider* aTemplateProvider )
: m_rData(_rData)
//, m_aContext(xContext,static_cast<backenduno::XLayerHandler*>(this),aTemplateProvider )
, m_aContext(xContext)
, m_aFactory()
, m_aLocale()
, m_pProperty(NULL)
, m_pConverter( new Converter(xContext) )
, m_nSkipping(0)
, m_bSublayer(false)
{
    m_aContext = DataBuilderContext(xContext,static_cast<backenduno::XLayerHandler*>(this),aTemplateProvider );
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
    if (!isDone())
        m_aContext.getLogger().error("Previous layer or schema not terminated properly", "prepareLayer()", "configmgr::LayerMergeHandler");

    m_aLocale = localehelper:: getDefaultLanguage();
    m_bSublayer = false;

    promoteToDefault(m_rData);
}
// -----------------------------------------------------------------------------

bool LayerMergeHandler::prepareSublayer(rtl::OUString const & aLocale)
{
    OSL_ENSURE(isDone(), "LayerMergeHandler: Warning: Previous layer not terminated properly");
    if (!isDone())
        m_aContext.getLogger().error("Previous layer not terminated properly", "prepareSublayer()", "configmgr::LayerMergeHandler");

    m_aLocale = aLocale;
    m_bSublayer = (aLocale.getLength() != 0);

    return m_bSublayer;
}
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
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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

    //Set state, removable and mandatory attribute flags
    aAttributes.setState(node::isAdded);
    aAttributes.setRemovability(true,true);


    return aAttributes;

}
// -----------------------------------------------------------------------------

void LayerMergeHandler::checkPropertyType(uno::Type const & _aType)
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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
                {
                    OSL_TRACE("Layer merging: Illegal property type: VOID overriding ANY");
                    m_aContext.getLogger().warning("Illegal property type: VOID overriding ANY - ignoring",
                                                    "checkPropertyType()", "configmgr::LayerMergeHandler");
                    // m_aContext.raiseIllegalTypeException("Layer merging: Illegal property type: VOID overriding ANY");
                }
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
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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

void LayerMergeHandler::setLocalizedValue(ISubtree * pProperty, uno::Any const & _aValue, rtl::OUString const & _aLocale)
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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
            {
                OSL_ENSURE(false,"Layer merging: Localized subnode is not a value");
                m_aContext.getLogger().error("Localized subnode is not a value - ignoring data",
                                             "setLocalizedValue()", "configmgr::LayerMergeHandler");
            }
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
        OSL_ENSURE(false, "Layer merging: Got locale-dependent value for non-localized node");
        m_aContext.getLogger().error("Got locale-dependent value for non-localized node",
                                     "setLocalizedValue()", "configmgr::LayerMergeHandler");
        setValueAndCheck(*pValue,_aValue);
    }

    else
    {
        OSL_ENSURE(false, "Layer merging: Unknown node type for localized node");
        m_aContext.getLogger().error("Unknown node type for localized node",
                                     "setLocalizedValue()", "configmgr::LayerMergeHandler");
    }
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::applyPropertyValue(uno::Any const & _aValue)
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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
    {
        OSL_ENSURE(false, "Layer merging: Unknown node type for property");
        m_aContext.getLogger().error("Unknown node type for property",
                                     "applyPropertyValue()", "configmgr::LayerMergeHandler");
    }
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::applyPropertyValue(uno::Any const & _aValue, rtl::OUString const & _aLocale)
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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
    {
        OSL_ENSURE(false, "Layer merging: Unknown node type for localized property");
        m_aContext.getLogger().error("Unknown node type for localized property",
                                     "applyPropertyValue()", "configmgr::LayerMergeHandler");
    }
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::applyAttributes(INode * pNode, sal_Int16 aNodeAttributes)
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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
        if (aNodeAttributes & NodeAttribute::FINALIZED)
            m_aContext.getLogger().warning("Node is both read-only and finalized - treating as readonly",
                                           "applyAttributes()", "configmgr::LayerMergeHandler");

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

static
void doLogRejection(sal_Int16 loglevel, DataBuilderContext const & aContext,
                    INode * pNode, bool bMandatory)
{
    rtl::OUStringBuffer aMessage;
    aMessage.appendAscii("Rejecting override: Node/Property ")
            .append(aContext.getNodePath(pNode->getName()))
            .appendAscii(" is ").appendAscii(bMandatory ? "mandatory" : "finalized")
            .appendAscii(" in a prior layer.");

    aContext.getLogger().log(loglevel,aMessage.makeStringAndClear(),
                              bMandatory ? "addOrReplace/dropNode()" : "startOverride()",
                              "configmgr::LayerMergeHandler");
}

static inline
void logRejection(DataBuilderContext const & aContext, INode * pNode, bool bMandatory=false)
{
    const sal_Int16 loglevel = LogLevel::INFO;
    if (aContext.getLogger().isLogging(loglevel))
    {
        doLogRejection(loglevel, aContext, pNode, bMandatory);
    }
}
// -----------------------------------------------------------------------------

bool LayerMergeHandler::startOverride(INode * pNode, sal_Bool bClear) /* ensure writable, mark merged */
    SAL_THROW(())
{
    OSL_PRECOND(pNode,"startOverride: non-NULL base node required");
    if (!m_aContext.isWritable(pNode))
    {
        // #i41700# write-protection is enforced, unless merging localizations
        if (!m_bSublayer)
        {
            logRejection(m_aContext,pNode);
            return false;
        }
        else
            OSL_ASSERT(m_aLocale.getLength() != 0);
    }

    OSL_ENSURE(!bClear,"'clear' operation is not yet supported");
    if (bClear)
        m_aContext.getLogger().warning("'clear' operation is not yet supported",
                                       "startOverride()", "configmgr::LayerMergeHandler");

    return true;
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::ensureUnchanged(INode const * pNode) const
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    ISubtree * pSchema = m_rData.getSchemaTree();
    OSL_ENSURE(pSchema,"No base data to merge layer into");

    if (!pSchema)
    {
        m_aContext.getLogger().error("No schema data for merging layer", "startLayer", "configmgr::LayerMergeHandler");
        throw uno::RuntimeException(rtl::OUString::createFromAscii("Layer merging: No data to merge with"),*this);
    }

    m_aContext.startActiveComponent(pSchema->getName());

    m_pProperty = NULL;
    m_nSkipping = 0;

    OSL_POSTCOND( m_aContext.hasActiveComponent(),  "Layer merging: could not set active component");
    OSL_POSTCOND( m_aContext.isDone(),              "Layer merging: newly started component is not empty");
    OSL_POSTCOND( !this->isSkipping(),              "Layer merging: newly started component is in skipping state");
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::endLayer( )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        m_aContext.raiseMalformedDataException("Layer merging: Unmatched data being skipped was not terminated properly.");

    m_aContext.endActiveComponent();

    m_bSublayer = false;

    OSL_POSTCOND( !m_aContext.hasActiveComponent(), "Layer merging: could not clear active component");
    OSL_POSTCOND(  m_aContext.isDone(), "Layer merging: could not finish processing");
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::overrideLayerRoot( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
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
        m_aContext.getLogger().warning("No component data in schema for merging layer",
                                       "overrideNode() [for layer root]", "configmgr::LayerMergeHandler");
        this->skipNode();
    }
}
// -----------------------------------------------------------------------------

static inline
sal_Int16 getOverrideViolationLogLevel(bool bIsSublayer)
{ return bIsSublayer ? LogLevel::FINER : LogLevel::INFO; }
// -----------------------------------------------------------------------------

void LayerMergeHandler::implOverrideNode(
    ISubtree * node, sal_Int16 attributes, bool clear)
{
    ensureUnchanged(node);
    if (startOverride(node, clear)) {
        applyAttributes(node, attributes);
        m_aContext.pushNode(node);
    } else {
        skipNode();
    }
}

void SAL_CALL LayerMergeHandler::overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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
        implOverrideNode(pNode, aAttributes, bClear);
    }
    else // ignore non-matched data
    {
        const sal_Int16 loglevel = getOverrideViolationLogLevel(m_bSublayer);
        if (m_aContext.getLogger().isLogging(loglevel))
        {
            rtl::OUStringBuffer aMessage;
            aMessage.appendAscii("Node ").append(m_aContext.getNodePath(aName))
                    .appendAscii(" to be overridden does not exist - skipping");

            m_aContext.getLogger().log(loglevel,aMessage.makeStringAndClear(), "overrideNode()", "configmgr::LayerMergeHandler");
        }
        // m_aContext.raiseNoSuchElementException("Layer merging: The node to be overridden does not exist.",aName);
        this->skipNode();
    }
}
// -----------------------------------------------------------------------------

void LayerMergeHandler::implAddOrReplaceNode( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
{
    ISubtree * pReplacedNode = m_aContext.findNode(aName);
    if (pReplacedNode)
    {
        if ((aAttributes & NodeAttribute::FUSE) == 0) {
            this->ensureUnchanged(pReplacedNode);

            if (!m_aContext.isRemovable(pReplacedNode))
            {
                logRejection(m_aContext,pReplacedNode,true);
                this->skipNode();
                return;
            }
        } else {
            implOverrideNode(
                pReplacedNode, aAttributes & ~NodeAttribute::FUSE, false);
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
        TemplateRequest aTemplateRequest(aTemplate.Name, aTemplate.Component);
        apNewInstance = m_aContext.getTemplateData( aTemplateRequest ).extractDataAndClear();
        if (apNewInstance.get())
            apNewInstance->setName( aName );
    }

    if (NULL == apNewInstance.get())
        m_aContext.raiseNoSuchElementException("Layer merging: Cannot instantiate template.", aTemplate.Name);

    applyAttributes(apNewInstance.get(), aAttributes & ~NodeAttribute::FUSE);
    //Set removable flag
    apNewInstance->markRemovable();

    m_aContext.markCurrentMerged();

    if (pReplacedNode) m_aContext.getCurrentParent().removeChild( aName );

    INode * pAddedInstance = m_aContext.getCurrentParent().addChild( apNewInstance );

    m_aContext.pushNode(pAddedInstance->asISubtree());
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
    {
        this->skipNode();
        return;
    }

    implAddOrReplaceNode( aName, m_aContext.getCurrentItemType(), aAttributes);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->leaveSkippedNode())
        return;

    this->propagateAttributes(m_aContext.getCurrentParent());

    m_aContext.popNode();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::dropNode( const rtl::OUString& aName )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    if (!m_aContext.getCurrentParent().isSetNode())
        m_aContext.raiseMalformedDataException("Layer merging: Removing child nodes is only possible in set nodes.");

    if (ISubtree * pDropped = m_aContext.findNode(aName))
    {
        this->ensureUnchanged(pDropped);
        if (!m_aContext.isRemovable(pDropped))
        {
            logRejection(m_aContext,pDropped,true);
            return;
        }
    }
    else
    {
        const sal_Int16 loglevel = getOverrideViolationLogLevel(m_bSublayer);
        if (m_aContext.getLogger().isLogging(loglevel))
        {
            rtl::OUStringBuffer aMessage;
            aMessage.appendAscii("Node ").append(m_aContext.getNodePath(aName))
                    .appendAscii(" to be removed does not exist - ignoring");

            m_aContext.getLogger().log(loglevel,aMessage.makeStringAndClear(), "dropNode()", "configmgr::LayerMergeHandler");
        }
        // m_aContext.raiseNoSuchElementException("Layer merging: The node to be removed does not exist.",aName);
    }
    m_aContext.markCurrentMerged();
    m_aContext.getCurrentParent().removeChild(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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
        const sal_Int16 loglevel = getOverrideViolationLogLevel(m_bSublayer);
        if (m_aContext.getLogger().isLogging(loglevel))
        {
            rtl::OUStringBuffer aMessage;
            aMessage.appendAscii("Property ").append(m_aContext.getNodePath(aName))
                    .appendAscii(" to be overridden does not exist - skipping");

            m_aContext.getLogger().log(loglevel,aMessage.makeStringAndClear(), "overrideNode()", "configmgr::LayerMergeHandler");
        }
        //   m_aContext.raiseUnknownPropertyException("Layer merging: The property to be overridden does not exist.",aName);
        this->skipNode();
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::endProperty( )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
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

void SAL_CALL LayerMergeHandler::addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    // TODO: add type validation
    node::Attributes aValueAttributes = makePropertyAttributes(aAttributes & SchemaAttribute::MASK);

    std::auto_ptr<ValueNode> aPropertyValue =
        m_aFactory.getNodeFactory().createNullValueNode(aName,aType,aValueAttributes);

    applyAttributes(aPropertyValue.get(),aAttributes & NodeAttribute::MASK);

    // can be a replace for dynamic properties (current update limitation)
    m_aContext.markCurrentMerged();
    m_aContext.addPropertyToCurrent(aPropertyValue, true);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    node::Attributes aValueAttributes = makePropertyAttributes(aAttributes & SchemaAttribute::MASK);

    std::auto_ptr<ValueNode> aPropertyValue =
        m_aFactory.getNodeFactory().createValueNode(aName,aValue,aValueAttributes);

    applyAttributes(aPropertyValue.get(),aAttributes & NodeAttribute::MASK);

    // can be a replace for dynamic properties (current update limitation)
    m_aContext.markCurrentMerged();
    m_aContext.addPropertyToCurrent(aPropertyValue, true);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::setPropertyValue( const uno::Any& aValue )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    if (!m_pProperty)
        m_aContext.raiseMalformedDataException("Layer merging: Invalid data: Overriding a value without a property.");

    OSL_ASSERT( !m_pProperty->getAttributes().isReplacedForUser() );
    m_pProperty->modifyState( node::isMerged );
    m_aContext.markCurrentMerged();

    applyPropertyValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerMergeHandler::setPropertyValueForLocale( const uno::Any& aValue, rtl::OUString const & aLocale )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (this->isSkipping())
        return;

    if (!m_pProperty)
        m_aContext.raiseMalformedDataException("Layer merging: Invalid data: Overriding a (localized) value without a property.");

    OSL_ASSERT( !m_pProperty->getAttributes().isReplacedForUser() );
    m_pProperty->modifyState( node::isMerged );
    m_aContext.markCurrentMerged();

    applyPropertyValue(aValue,aLocale);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

uno::Reference< com::sun::star::script::XTypeConverter >
    LayerMergeHandler::Converter::createTCV(uno::Reference< uno::XComponentContext > const & xContext)
{
    OSL_ENSURE(xContext.is(),"Cannot create TypeConverter for LayerMergeHandler without a Context");

    uno::Reference< lang::XMultiComponentFactory > xFactory = xContext->getServiceManager();
    OSL_ENSURE(xFactory.is(),"Cannot create TypeConverter for LayerMergeHandler without a ServiceManager");

    uno::Reference< com::sun::star::script::XTypeConverter > xTCV;
    if (xFactory.is())
    {
        static const rtl::OUString k_sTCVService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter"));

        xTCV = uno::Reference< com::sun::star::script::XTypeConverter >::query(xFactory->createInstanceWithContext(k_sTCVService,xContext));
    }
    return xTCV;
}
// -----------------------------------------------------------------------------

LayerMergeHandler::Converter::Converter(uno::Reference< uno::XComponentContext > const & xContext)
: m_aConverter( createTCV(xContext) )
, m_bConvertData(false)
{
}
// -----------------------------------------------------------------------------
static
inline
uno::Type getBinaryDataType()
{
    uno::Sequence< sal_Int8 > const * const forBinary = 0;
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
        uno::Sequence< rtl::OUString > aStringList;
        if (_aValue >>= aStringList)
            return m_aConverter.convertListToAny(aStringList);
    }

    rtl::OUString aContent;
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
