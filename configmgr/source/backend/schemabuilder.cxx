/*************************************************************************
 *
 *  $RCSfile: schemabuilder.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:49 $
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

#include "schemabuilder.hxx"

#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif

#include <drafts/com/sun/star/configuration/backend/SchemaAttribute.hpp>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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
        static void check_if_complete()
        {
            uno::Reference< backenduno::XSchemaHandler >
                 test(new SchemaBuilder(OUString()));
        }
// -----------------------------------------------------------------------------

SchemaBuilder::SchemaBuilder( const OUString& aExpectedComponentName, ITemplateDataProvider* aTemplateProvider )
: m_aData()
, m_aContext(static_cast<backenduno::XSchemaHandler*>(this), aExpectedComponentName, aTemplateProvider )
, m_aFactory()
, m_aTemplateProvider(aTemplateProvider)
{

}
// -----------------------------------------------------------------------------

SchemaBuilder::~SchemaBuilder(  )
{

}
// -----------------------------------------------------------------------------

MergedComponentData & SchemaBuilder::result()
{
    OSL_ENSURE(isDone(), "SchemaBuilder: Warning: Schema not terminated properly");

    return m_aData;
}
// -----------------------------------------------------------------------------

MergedComponentData const & SchemaBuilder::result() const
{
    OSL_ENSURE(isDone(), "SchemaBuilder: Warning: Schema not terminated properly");

    return m_aData;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// XSchemaHandler

void SAL_CALL SchemaBuilder::startSchema(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    if (!this->isDone())
        m_aContext.raiseMalformedDataException("Schema builder: Unexpected Restart of Schema");

    m_aData.clear();

    OSL_ASSERT(!m_aContext.hasActiveComponent());
    OSL_ASSERT( m_aContext.isDone());
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endSchema(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    if (!this->isDone())
        m_aContext.raiseMalformedDataException("Schema builder: Unexpected End of Schema");

    substituteInstances();
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::importComponent( const OUString& aName )
    throw (MalformedDataException, container::NoSuchElementException, lang::IllegalArgumentException, uno::RuntimeException)
{
    //OSL_TRACE("WARNING: Configuration schema parser: Cross-component references are not yet supported\n");
    // OSL_ENSURE(false, "Cross-component references are not yet supported");
}
// -----------------------------------------------------------------------------


void SAL_CALL SchemaBuilder::startComponent( const OUString& aName )
        throw (MalformedDataException, container::ElementExistException, lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException)
{
    if (m_aData.hasSchema())
        m_aContext.raiseElementExistException("Schema builder: The component schema is already loaded", OUString());

    m_aContext.startActiveComponent(aName);

    std::auto_ptr<ISubtree> apSchema =
        m_aFactory.createGroup(aName,0,getComponentRootAttributes());

    ISubtree * pSchema = m_aData.setSchemaRoot(apSchema);

    m_aContext.pushNode(pSchema);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endComponent( )
        throw (MalformedDataException, lang::NoSupportException, uno::RuntimeException)
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

void SAL_CALL SchemaBuilder::startGroupTemplate( const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
        throw (MalformedDataException, container::ElementExistException, lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException)
{
    if (aTemplate.Component.getLength() == 0)
        m_aContext.raiseIllegalArgumentException("Schema builder: Starting template without owning component",1);

    m_aContext.startActiveComponent(aTemplate.Component);

    if (m_aData.hasTemplate(aTemplate.Name))
        m_aContext.raiseElementExistException("Schema builder: Template already exists",aTemplate.Name);

    OUString aName = m_aData.getTemplateAccessor(aTemplate);
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTemplateTree =
        m_aFactory.createGroup(aName,bExtensible,getTemplateBaseAttributes());

    ISubtree * pTree = m_aData.addTemplate(aTemplateTree,aTemplate);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::startSetTemplate( const TemplateIdentifier& aTemplate, sal_Int16 aAttributes, const TemplateIdentifier& aItemType )
        throw (MalformedDataException, container::ElementExistException, container::NoSuchElementException, lang::IllegalArgumentException, lang::NoSupportException, uno::RuntimeException)
{
    if (aTemplate.Component.getLength() == 0)
        m_aContext.raiseIllegalArgumentException("Schema builder: Starting template without owning component",1);

    m_aContext.startActiveComponent(aTemplate.Component);

    if (m_aData.hasTemplate(aTemplate.Name))
        m_aContext.raiseElementExistException("Schema builder: Template already exists",aTemplate.Name);

    OUString aName = m_aData.getTemplateAccessor(aTemplate);
    TemplateIdentifier aFullType = m_aContext.completeComponent(aItemType);
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTemplateTree =
        m_aFactory.createSet( aName,aFullType,bExtensible,getTemplateBaseAttributes());

    ISubtree * pTree = m_aData.addTemplate(aTemplateTree,aTemplate);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endTemplate( )
        throw (MalformedDataException, lang::NoSupportException, uno::RuntimeException)
{
    m_aContext.popNode();

    m_aContext.endActiveComponent();
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::startGroup( const OUString& aName, sal_Int16 aAttributes )
        throw (MalformedDataException, container::ElementExistException, lang::IllegalArgumentException, uno::RuntimeException)
{
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTree = m_aFactory.createGroup(aName,bExtensible,m_aContext.getCurrentAttributes());

    ISubtree * pTree = m_aContext.addNodeToCurrent(aTree);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::startSet( const OUString& aName, sal_Int16 aAttributes, const TemplateIdentifier& aItemType )
        throw (MalformedDataException, container::ElementExistException, container::NoSuchElementException, lang::IllegalArgumentException, uno::RuntimeException)
{
    TemplateIdentifier aFullType = m_aContext.completeComponent(aItemType);
    bool bExtensible = isExtensible(aAttributes);

    std::auto_ptr<ISubtree> aTree = m_aFactory.createSet(aName,aFullType,bExtensible,m_aContext.getCurrentAttributes());

    ISubtree * pTree = m_aContext.addNodeToCurrent(aTree);

    m_aContext.pushNode(pTree);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::endNode( )
        throw (MalformedDataException, uno::RuntimeException)
{
    m_aContext.popNode();

    if (m_aContext.isDone())
        m_aContext.raiseMalformedDataException("Schema builder: Incorrect Termination");
}
// -----------------------------------------------------------------------------

node::Attributes SchemaBuilder::makePropertyAttributes(sal_Int16 aSchemaAttributes)
{
    const sal_uInt16 c_AllPropertyAttributes =
        SchemaAttribute::REQUIRED | SchemaAttribute::LOCALIZED;

    if ((aSchemaAttributes & c_AllPropertyAttributes) != aSchemaAttributes)
        m_aContext.raiseIllegalArgumentException("SchemaBuilder: Unreckognized Attribute for Property",2);

    node::Attributes aAttributes = m_aContext.getCurrentAttributes();

    if (aSchemaAttributes & SchemaAttribute::REQUIRED)
        aAttributes.bNullable = false;

    return aAttributes;
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
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

void SAL_CALL SchemaBuilder::addPropertyWithDefault( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aDefaultValue )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    // TODO: add parameter validation
    node::Attributes aValueAttributes = makePropertyAttributes(aAttributes);

    if (aAttributes & SchemaAttribute::LOCALIZED)
    {
        std::auto_ptr<ISubtree> aLocalizedProp =
            m_aFactory.createLocalizedContainer(aName,aDefaultValue.getValueType(),aValueAttributes);

        std::auto_ptr<ValueNode> aPropertyValue =
            m_aFactory.getNodeFactory().createValueNode(localehelper::getDefaultLocale(),aDefaultValue,aValueAttributes);

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

void SAL_CALL SchemaBuilder::addInstance( const OUString& aName, const TemplateIdentifier& aTemplate )
        throw (MalformedDataException, container::ElementExistException, container::NoSuchElementException, lang::IllegalArgumentException, uno::RuntimeException)
{
    TemplateIdentifier aFullType = m_aContext.completeComponent(aTemplate);

    std::auto_ptr<ISubtree> aPlaceHolder =
        m_aFactory.createPlaceHolder(aName,aFullType);

    m_aContext.addNodeToCurrent(aPlaceHolder);
}
// -----------------------------------------------------------------------------

void SAL_CALL SchemaBuilder::addItemType( const TemplateIdentifier& aItemType )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalArgumentException, uno::RuntimeException)
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

node::Attributes SchemaBuilder::getComponentRootAttributes()
{
// setting state will be done later (when preparing for merge)
//    node::Attributes aResult;
//    aResult.setState( node::isDefault );
    return node::Attributes();
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
        typedef std::vector< ISubtree const * > TemplateStack;
        typedef std::vector< OUString >         InstanceList;

        MergedComponentData &   m_rData;
        DataBuilderContext      m_aContext;
        ComponentDataFactory    m_aFactory;

        InstanceList            m_aReplacementList;
        TemplateStack           m_aTemplateStack;
    public:
        SubstitutionHelper(MergedComponentData & _rData, uno::XInterface * _pContext, ITemplateDataProvider* aTemplateProvider=NULL  )
        : m_rData(_rData)
        , m_aContext(_pContext,aTemplateProvider)
        , m_aReplacementList()
        , m_aTemplateStack()
        {}

        void substituteInData();
    private:
        void substituteInComponent(ISubtree * _pComponent);
        void substituteInNode(ISubtree & _rNode);
        void substituteInList();

        void substitute(OUString const & _aName);

        virtual void handle(ValueNode&);
        virtual void handle(ISubtree&);
    };
}
// -----------------------------------------------------------------------------

void SchemaBuilder::substituteInstances()
{
    SubstitutionHelper helper(m_aData, static_cast<backenduno::XSchemaHandler*>(this),m_aTemplateProvider);

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
        InstanceList aSaveInstances;
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
        for(InstanceList::iterator it = m_aReplacementList.begin();
            it != m_aReplacementList.end(); ++it)
        {
            this->substitute(*it);
        }
    }
    // -----------------------------------------------------------------------------

    void SubstitutionHelper::substitute(OUString const & _aName)
    {

        ISubtree & rParent = m_aContext.getCurrentParent();

        std::auto_ptr<INode> pReplacedNode = rParent.removeChild(_aName);
        OSL_ASSERT( pReplacedNode.get() != NULL );

        ISubtree * pReplacedInstance = pReplacedNode->asISubtree();
        OSL_ASSERT( pReplacedInstance != NULL );

        TemplateIdentifier aTemplateName = m_aFactory.getInstanceType(*pReplacedInstance);
        if (aTemplateName.Component == m_aContext.getActiveComponent())
        {
            if (ISubtree const * pTemplate = m_rData.findTemplate(aTemplateName.Name))
            {
                TemplateStack::iterator beg = m_aTemplateStack.begin(), end = m_aTemplateStack.end();
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
             TemplateRequest aTemplateRequest(configuration::makeName(aTemplateName.Name, configuration::Name::NoValidate()),
                                              configuration::makeName(aTemplateName.Component, configuration::Name::NoValidate()) );
             TemplateResult aResult = m_aContext.getTemplateData( aTemplateRequest );

             std::auto_ptr<INode> pTemplateInstance = aResult.extractDataAndClear();
             pTemplateInstance->setName(_aName);
             ISubtree * pAddedTree = rParent.addChild(pTemplateInstance)->asISubtree();
             OSL_ENSURE(pAddedTree, "Could not obtain added template instance");
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
