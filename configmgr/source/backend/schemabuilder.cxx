/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: schemabuilder.cxx,v $
 * $Revision: 1.18 $
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

#include "schemabuilder.hxx"
#include "treenodefactory.hxx"
#include "matchlocale.hxx"
#include <com/sun/star/configuration/backend/SchemaAttribute.hpp>
#include <rtl/ustrbuf.hxx>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

        namespace SchemaAttribute = backenduno::SchemaAttribute;
// -----------------------------------------------------------------------------
//#if OSL_DEBUG_LEVEL > 0
// currently not used in debug builds
#if 0
        static void check_if_complete(MergedComponentData & md, uno::Reference< uno::XComponentContext > const & xContext)
        {
            uno::Reference< backenduno::XSchemaHandler >
                 test(new SchemaBuilder(xContext, rtl::OUString(),md,NULL));
        }
#endif
// -----------------------------------------------------------------------------

SchemaBuilder::SchemaBuilder(uno::Reference< uno::XComponentContext > const & xContext, const rtl::OUString& aExpectedComponentName, MergedComponentData & rData, ITemplateDataProvider* aTemplateProvider )
: m_aData(rData)
, m_aContext(xContext)
//, m_aContext(xContext,static_cast<backenduno::XSchemaHandler*>(this), aExpectedComponentName, aTemplateProvider )
, m_aFactory()
{
    m_aContext = DataBuilderContext(xContext,static_cast<backenduno::XSchemaHandler*>(this), aExpectedComponentName, aTemplateProvider );
}
// -----------------------------------------------------------------------------

SchemaBuilder::~SchemaBuilder(  )
{

}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// XSchemaHandler

void SAL_CALL SchemaBuilder::startSchema(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!this->isDone())
        m_aContext.raiseMalformedDataException("Schema builder: Unexpected Restart of Schema");

    m_aData.clear();

    OSL_ASSERT(!m_aContext.hasActiveComponent());
    OSL_ASSERT( m_aContext.isDone());
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endSchema(  )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!this->isDone())
        m_aContext.raiseMalformedDataException("Schema builder: Unexpected End of Schema");

    substituteInstances();
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::importComponent( const rtl::OUString& /*aName*/ )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    //OSL_TRACE("WARNING: Configuration schema parser: Cross-component references are not yet supported\n");
}
// -----------------------------------------------------------------------------


void SAL_CALL SchemaBuilder::startComponent( const rtl::OUString& aName )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (m_aData.hasSchema())
        m_aContext.raiseElementExistException("Schema builder: The component schema is already loaded", rtl::OUString());

    m_aContext.startActiveComponent(aName);

    std::auto_ptr<ISubtree> apSchema =
        m_aFactory.createGroup(aName,0,getComponentRootAttributes());

    ISubtree * pSchema = m_aData.setSchemaRoot(apSchema);

    m_aContext.pushNode(pSchema);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endComponent( )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aContext.popNode();

    m_aContext.endActiveComponent();
}
// -----------------------------------------------------------------------------

bool SchemaBuilder::isExtensible(sal_Int16 aSchemaAttributes)
{
    sal_Int16 const aValidAttributes = aSchemaAttributes & SchemaAttribute::EXTENSIBLE;

    if (aValidAttributes != aSchemaAttributes)
        m_aContext.raiseIllegalArgumentException("Schema builder: Illegal attribute specified for node.",2);

    return (aValidAttributes != 0);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::startGroupTemplate( const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (aTemplate.Component.getLength() == 0)
        m_aContext.raiseIllegalArgumentException("Schema builder: Starting template without owning component",1);

    m_aContext.startActiveComponent(aTemplate.Component);

    if (m_aData.hasTemplate(aTemplate.Name))
        m_aContext.raiseElementExistException("Schema builder: Template already exists",aTemplate.Name);

    rtl::OUString aName = m_aData.getTemplateAccessor(aTemplate);
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTemplateTree =
        m_aFactory.createGroup(aName,bExtensible,getTemplateBaseAttributes());

    ISubtree * pTree = m_aData.addTemplate(aTemplateTree,aTemplate);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::startSetTemplate( const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes, const backenduno::TemplateIdentifier& aItemType )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (aTemplate.Component.getLength() == 0)
        m_aContext.raiseIllegalArgumentException("Schema builder: Starting template without owning component",1);

    m_aContext.startActiveComponent(aTemplate.Component);

    if (m_aData.hasTemplate(aTemplate.Name))
        m_aContext.raiseElementExistException("Schema builder: Template already exists",aTemplate.Name);

    rtl::OUString aName = m_aData.getTemplateAccessor(aTemplate);
    backenduno::TemplateIdentifier aFullType = m_aContext.completeComponent(aItemType);
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTemplateTree =
        m_aFactory.createSet( aName,aFullType,bExtensible,getTemplateBaseAttributes());

    ISubtree * pTree = m_aData.addTemplate(aTemplateTree,aTemplate);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endTemplate( )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aContext.popNode();

    m_aContext.endActiveComponent();
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::startGroup( const rtl::OUString& aName, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTree = m_aFactory.createGroup(aName,bExtensible,getNodeAttributes());

    ISubtree * pTree = m_aContext.addNodeToCurrent(aTree);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::startSet( const rtl::OUString& aName, sal_Int16 aAttributes, const backenduno::TemplateIdentifier& aItemType )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    backenduno::TemplateIdentifier aFullType = m_aContext.completeComponent(aItemType);
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTree = m_aFactory.createSet(aName,aFullType,bExtensible,getNodeAttributes());

    ISubtree * pTree = m_aContext.addNodeToCurrent(aTree);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endNode( )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_aContext.popNode();

    if (m_aContext.isDone())
        m_aContext.raiseMalformedDataException("Schema builder: Incorrect Termination");
}
// -----------------------------------------------------------------------------

node::Attributes SchemaBuilder::makePropertyAttributes(sal_Int16 aSchemaAttributes) const
{
    const sal_uInt16 c_AllPropertyAttributes =
        SchemaAttribute::REQUIRED | SchemaAttribute::LOCALIZED;

    if ((aSchemaAttributes & c_AllPropertyAttributes) != aSchemaAttributes)
        m_aContext.raiseIllegalArgumentException("SchemaBuilder: Unreckognized Attribute for Property",2);

    node::Attributes aAttributes = getNodeAttributes();

    if (aSchemaAttributes & SchemaAttribute::REQUIRED)
        aAttributes.setNullable (false);

    return aAttributes;
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    // TODO: add type validation
    node::Attributes aValueAttributes = makePropertyAttributes(aAttributes);

    if (aAttributes & SchemaAttribute::LOCALIZED)
    {
        std::auto_ptr<ISubtree> aLocalizedProp =
            m_aFactory.createLocalizedContainer(aName,aType,aValueAttributes);

        m_aContext.addLocalizedToCurrent(aLocalizedProp);
    }
    else
    {
        std::auto_ptr<ValueNode> aPropertyValue =
            m_aFactory.getNodeFactory().createNullValueNode(aName,aType,aValueAttributes);

        m_aContext.addPropertyToCurrent(aPropertyValue);
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::addPropertyWithDefault( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aDefaultValue )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    // TODO: add parameter validation
    node::Attributes aValueAttributes = makePropertyAttributes(aAttributes);

    if (aAttributes & SchemaAttribute::LOCALIZED)
    {
        std::auto_ptr<ISubtree> aLocalizedProp =
            m_aFactory.createLocalizedContainer(aName,aDefaultValue.getValueType(),aValueAttributes);

        std::auto_ptr<ValueNode> aPropertyValue =
            m_aFactory.getNodeFactory().createValueNode(localehelper::getDefaultLanguage(),aDefaultValue,aValueAttributes);

        aLocalizedProp->addChild( base_ptr(aPropertyValue) );

        m_aContext.addLocalizedToCurrent(aLocalizedProp);
    }
    else
    {
        std::auto_ptr<ValueNode> aPropertyValue =
            m_aFactory.getNodeFactory().createValueNode(aName,aDefaultValue,aValueAttributes);


        m_aContext.addPropertyToCurrent( aPropertyValue);
    }
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::addInstance( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    backenduno::TemplateIdentifier aFullType = m_aContext.completeComponent(aTemplate);

    std::auto_ptr<ISubtree> aPlaceHolder =
        m_aFactory.createPlaceHolder(aName,aFullType);

    m_aContext.addNodeToCurrent(aPlaceHolder);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::addItemType( const backenduno::TemplateIdentifier& aItemType )
        throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if ( m_aContext.getCurrentParent().getElementTemplateName() != aItemType.Name ||
         m_aContext.getCurrentParent().getElementTemplateModule() != m_aContext.getTemplateComponent(aItemType) )
    {
        OSL_ENSURE(false, "SchemaBuilder: Multiple ItemTypes for Sets are currently not supported");
        m_aContext.raiseMalformedDataException("SchemaBuilder: Unsupported Feature: Multiple ItemTypes for Sets");
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

node::Attributes SchemaBuilder::getNodeAttributes() const
{
    node::Attributes aResult = m_aContext.getCurrentAttributes();
    aResult.setState( node::isDefault );
    return aResult;
}
// -----------------------------------------------------------------------------

node::Attributes SchemaBuilder::getComponentRootAttributes()
{
    node::Attributes aResult;
    aResult.setState( node::isDefault );
    return aResult;
}
// -----------------------------------------------------------------------------

node::Attributes SchemaBuilder::getTemplateBaseAttributes()
{
    node::Attributes aResult;
    aResult.setState( node::isReplaced );
    return aResult;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

namespace
{
    class SubstitutionHelper : NodeModification
    {
        MergedComponentData &   m_rData;
        DataBuilderContext      m_aContext;
        ComponentDataFactory    m_aFactory;

        std::vector< rtl::OUString >            m_aReplacementList;
        std::vector< ISubtree const * >           m_aTemplateStack;
    public:
        SubstitutionHelper(DataBuilderContext const & aBaseContext, MergedComponentData & _rData, uno::XInterface * _pContext)
        : m_rData(_rData)
        , m_aContext(aBaseContext,_pContext)
        , m_aReplacementList()
        , m_aTemplateStack()
        {}

        void substituteInData();
    private:
        void substituteInComponent(ISubtree * _pComponent);
        void substituteInNode(ISubtree & _rNode);
        void substituteInList();

        void substitute(rtl::OUString const & _aName);

        virtual void handle(ValueNode&);
        virtual void handle(ISubtree&);
    };
}
// -----------------------------------------------------------------------------

void SchemaBuilder::substituteInstances()
{
    SubstitutionHelper helper(m_aContext, m_aData, static_cast<backenduno::XSchemaHandler*>(this));

    helper.substituteInData();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
namespace
{
// -----------------------------------------------------------------------------
    void SubstitutionHelper::substituteInData()
    {
        if (m_rData.hasTemplates())
            substituteInComponent(m_rData.getTemplatesTree());

        if (m_rData.hasSchema())
            substituteInComponent(m_rData.getSchemaTree());
    }
    // -----------------------------------------------------------------------------

    void SubstitutionHelper::substituteInComponent(ISubtree * _pComponent)
    {
        OSL_ENSURE(_pComponent,"ERROR: Trying to substitute in NULL component");

        if (_pComponent)
        {
            m_aContext.startActiveComponent(_pComponent->getName());
            this->substituteInNode(*_pComponent);
            m_aContext.endActiveComponent();
        }
    }
    // -----------------------------------------------------------------------------

    void SubstitutionHelper::substituteInNode(ISubtree & _rNode)
    {
        std::vector< rtl::OUString > aSaveInstances;
        aSaveInstances.swap(m_aReplacementList);

        // todo: own stack to check against infinite recursion
        m_aContext.pushNode(&_rNode);

        this->applyToChildren(_rNode); // fill the list

        this->substituteInList();

        m_aContext.popNode();

        aSaveInstances.swap(m_aReplacementList);
    }
    // -----------------------------------------------------------------------------

    void SubstitutionHelper::substituteInList()
    {
        for(std::vector< rtl::OUString >::iterator it = m_aReplacementList.begin();
            it != m_aReplacementList.end(); ++it)
        {
            this->substitute(*it);
        }
    }
    // -----------------------------------------------------------------------------

    void SubstitutionHelper::substitute(rtl::OUString const & _aName)
    {

        ISubtree & rParent = m_aContext.getCurrentParent();

        std::auto_ptr<INode> pReplacedNode = rParent.removeChild(_aName);
        OSL_ASSERT( pReplacedNode.get() != NULL );

        ISubtree * pReplacedInstance = pReplacedNode->asISubtree();
        OSL_ASSERT( pReplacedInstance != NULL );

        backenduno::TemplateIdentifier aTemplateName = m_aFactory.getInstanceType(*pReplacedInstance);
        if (aTemplateName.Component == m_aContext.getActiveComponent())
        {
            if (ISubtree const * pTemplate = m_rData.findTemplate(aTemplateName.Name))
            {
                std::vector< ISubtree const * >::iterator beg = m_aTemplateStack.begin(), end = m_aTemplateStack.end();
                if (std::find(beg,end,pTemplate) != end)
                    m_aContext.raiseMalformedDataException("SchemaBuilder: Could not expand instances: Template is recursive");

                m_aTemplateStack.push_back(pTemplate);

                std::auto_ptr< INode > pTemplateInstance = pTemplate->clone();

                pTemplateInstance->setName(_aName);
                // TODO: adjust state/attributes here (?)

                ISubtree * pAddedTree = rParent.addChild(pTemplateInstance)->asISubtree();

                OSL_ENSURE(pAddedTree, "Could not obtain added template instance");

                this->substituteInNode(*pAddedTree);

                m_aTemplateStack.pop_back();
            }
            else
            {
                m_aContext.raiseMalformedDataException("SchemaBuilder: Could not expand instances: Template not found");
            }
        }
         //Import Template from different component
        else
        {
            TemplateRequest aTemplateRequest(aTemplateName.Name,
                                             aTemplateName.Component );
             ResultHolder< TemplateInstance > aResult = m_aContext.getTemplateData( aTemplateRequest );

             std::auto_ptr<INode> pTemplateInstance = aResult.extractDataAndClear();
             pTemplateInstance->setName(_aName);

             // Add template instance - must be a tree as any template
             OSL_VERIFY(
        rParent.addChild(pTemplateInstance)->asISubtree() );
        }
    }
    // -----------------------------------------------------------------------------

    void SubstitutionHelper::handle(ValueNode&)
    {
    }
    // -----------------------------------------------------------------------------

    void SubstitutionHelper::handle(ISubtree& _rTree)
    {
        if (m_aFactory.isInstancePlaceHolder(_rTree))
            m_aReplacementList.push_back(_rTree.getName());

        else
            substituteInNode(_rTree);
    }
// -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
