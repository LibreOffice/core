/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propsetaccessimpl.cxx,v $
 * $Revision: 1.24.10.5 $
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

#include "propsetaccessimpl.hxx"
#include "propertyinfohelper.hxx"
#include "apinodeaccess.hxx"
#include "apinodeupdate.hxx"
#include "noderef.hxx"
#include "valueref.hxx"
#include "anynoderef.hxx"
#include "nodechange.hxx"
#include "configgroup.hxx"
#include "confignotifier.hxx"
#include "broadcaster.hxx"
#include "apitypes.hxx"
#include "attributes.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/implbase1.hxx>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

namespace configmgr
{
    namespace configapi
    {
//-----------------------------------------------------------------------------------
        namespace lang = css::lang;
        namespace beans = css::beans;

//-----------------------------------------------------------------------------------
// a helper class
//-----------------------------------------------------------------------------------

class CollectProperties : configuration::NodeVisitor
{
    std::vector< beans::Property >      m_aProperties;
    sal_Bool                    m_bReadonly;
public:
    CollectProperties(sal_Bool _bReadonly)
    : m_bReadonly(_bReadonly)
    { }
    CollectProperties(sal_Bool _bReadonly, sal_Int32 _nCount)
    : m_bReadonly(_bReadonly)
    { m_aProperties.reserve(_nCount); }

    uno::Sequence<beans::Property> forChildren(rtl::Reference< configuration::Tree > const& _aPropertyTree, configuration::NodeRef const& _rNode)
    {
        OSL_ENSURE( _rNode.isValid() && _aPropertyTree->isValidNode(_rNode.getOffset()), "Node to retrieve properties from does not match tree");
        reset();
        _aPropertyTree->dispatchToChildren(_rNode, *this);
        return makeSequence(m_aProperties);
    }

private:
    void    reset() { m_aProperties.clear(); }

    node::Attributes    adjustAttributes(node::Attributes nNodeAttr);

    Result  handle(rtl::Reference< configuration::Tree > const& _aTree, configuration::NodeRef const& _rValue);
    Result  handle(rtl::Reference< configuration::Tree > const& _aTree, configuration::ValueRef const& _rValue);
};

//-----------------------------------------------------------------------------------
node::Attributes CollectProperties::adjustAttributes(node::Attributes nNodeAttr)
{
    if (m_bReadonly) nNodeAttr.markReadonly();

    return nNodeAttr;
}

//-----------------------------------------------------------------------------------
CollectProperties::Result CollectProperties::handle(rtl::Reference< configuration::Tree > const& _aTree, configuration::ValueRef const& _rValue)
{
    // can be default ?
    m_aProperties.push_back(
                helperMakeProperty( _rValue.m_sNodeName,
                                    adjustAttributes(_aTree->getAttributes(_rValue)),
                                    _aTree->getUnoType(_rValue),
                                    _aTree->hasNodeDefault(_rValue)
                                )
            );

    return CONTINUE;
}

//-----------------------------------------------------------------------------------
CollectProperties::Result CollectProperties::handle(rtl::Reference< configuration::Tree > const& _aTree, configuration::NodeRef const& _rNode)
{
    // can be default ?
    OSL_ENSURE( configuration::isStructuralNode(_aTree,_rNode),
                "Unexpected value element node. Cannot get proper type for this node as property" );

    m_aProperties.push_back(
        helperMakeProperty( _aTree->getSimpleNodeName(_rNode.getOffset()),
                                    adjustAttributes(_aTree->getAttributes(_rNode)),
                                    getUnoInterfaceType(),
                                    _aTree->hasNodeDefault(_rNode)
                                )
            );

    return CONTINUE;
}

//-----------------------------------------------------------------------------------
// yet another helper class (more robust, but can't well be extended to be a HierarchicalPropertySetInfo though)
//-----------------------------------------------------------------------------------

class NodePropertySetInfo
    :public ::cppu::WeakImplHelper1< beans::XPropertySetInfo >
{
    uno::Sequence< beans::Property > const m_aProperties;

public:
    NodePropertySetInfo(uno::Sequence< beans::Property > const& _aProperties) throw(uno::RuntimeException)
    : m_aProperties(_aProperties)
    {
    }

    static NodePropertySetInfo* create(NodeGroupInfoAccess& _rNode, sal_Bool _bReadonly ) throw(uno::RuntimeException);
    beans::Property const* begin() const throw() { return m_aProperties.getConstArray(); }
    beans::Property const* end()   const throw() { return m_aProperties.getConstArray() + m_aProperties.getLength(); }

    beans::Property const* find(const rtl::OUString& _rPropertyName) const throw(uno::RuntimeException);

    // XPropertySetInfo
    virtual uno::Sequence< beans::Property > SAL_CALL getProperties() throw(uno::RuntimeException);
    virtual beans::Property SAL_CALL    getPropertyByName(const rtl::OUString& _rPropertyName) throw(beans::UnknownPropertyException, uno::RuntimeException);
    virtual sal_Bool SAL_CALL   hasPropertyByName(const rtl::OUString& _rPropertyName) throw(uno::RuntimeException);
};

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
NodePropertySetInfo* NodePropertySetInfo::create(NodeGroupInfoAccess& _rNode, sal_Bool _bReadonly ) throw(uno::RuntimeException)
{
    UnoApiLock aLock;

    rtl::Reference< configuration::Tree > aTree( _rNode.getTree() );
    OSL_ENSURE( !configuration::isEmpty(aTree.get()), "WARNING: Getting Tree information requires a valid tree");
    if (configuration::isEmpty(aTree.get())) return NULL;

    configuration::NodeRef aNode( _rNode.getNodeRef() );
    OSL_ENSURE( aNode.isValid() && aTree->isValidNode(aNode.getOffset()), "ERROR: Tree does not match node");

    uno::Sequence< beans::Property > aProperties = CollectProperties(_bReadonly).forChildren(aTree,aNode);
    OSL_ENSURE( aProperties.getLength() > 0, "ERROR: PropertySet (Configuration group) has no Properties");

    return new NodePropertySetInfo( aProperties );
}

//-----------------------------------------------------------------------------------
struct MatchName // : std::unary_function< beans::Property, bool >
{
    rtl::OUString sName;
    MatchName(rtl::OUString const& _sName)  throw(uno::RuntimeException)
    : sName(_sName)
    {
    }

    bool operator()(beans::Property const& _aProperty) const
    {
        return !!(_aProperty.Name == this->sName);
    }
};

beans::Property const* NodePropertySetInfo::find(const rtl::OUString& _rPropertyName)  const throw(uno::RuntimeException)
{
    beans::Property const* const first = this->begin();
    beans::Property const* const last  = this->end();

    return std::find_if(first,last,MatchName(_rPropertyName));
}

//-----------------------------------------------------------------------------------
uno::Sequence< beans::Property > SAL_CALL NodePropertySetInfo::getProperties() throw(uno::RuntimeException)
{
    return m_aProperties;
}
//-----------------------------------------------------------------------------------
beans::Property SAL_CALL NodePropertySetInfo::getPropertyByName(const rtl::OUString& _rPropertyName)
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    UnoApiLock aLock;

    beans::Property const* pFound = find(_rPropertyName);

    if (pFound == this->end())
    {
        rtl::OUString sMessage = rtl::OUString::createFromAscii("Configuration - ");
        sMessage += rtl::OUString::createFromAscii("No Property named '");
        sMessage += _rPropertyName;
        sMessage += rtl::OUString::createFromAscii("' in this PropertySetInfo");
        throw beans::UnknownPropertyException(sMessage, static_cast<XPropertySetInfo*>(this));
    }

    return *pFound;
}

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL NodePropertySetInfo::hasPropertyByName(const rtl::OUString& _rPropertyName)
    throw(uno::RuntimeException)
{
    UnoApiLock aLock;

    beans::Property const* pFound = find(_rPropertyName);

    return (pFound != this->end());
}

// Interface methods
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// getting Property Metadata
//-----------------------------------------------------------------------------------

// XPropertySet & XMultiPropertySet
//-----------------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > implGetPropertySetInfo( NodeGroupInfoAccess& rNode, sal_Bool _bWriteable )
    throw(uno::RuntimeException)
{
    GuardedNodeData<NodeAccess> lock( rNode );
    return NodePropertySetInfo::create(rNode, !_bWriteable);
}

// XHierarchicalPropertySet & XHierarchicalMultiPropertySet
//-----------------------------------------------------------------------------------
uno::Reference< beans::XHierarchicalPropertySetInfo > implGetHierarchicalPropertySetInfo( NodeGroupInfoAccess& /*rNode*/ )
    throw(uno::RuntimeException)
{
    // TODO: Implement
    return 0;
}

//-----------------------------------------------------------------------------------
// setting values - may all throw (PropertyVeto)Exceptions on read-only property sets
//-----------------------------------------------------------------------------------

// XPropertySet
//-----------------------------------------------------------------------------------
void implSetPropertyValue( NodeGroupAccess& rNode, const rtl::OUString& sPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
              lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeGroupAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateChildName(sPropertyName,aTree,aNode);

        configuration::ValueRef aChild( aTree->getChildValue(aNode, aChildName) );

        if (!aChild.isValid())
        {
            if ( configuration::hasChildOrElement(aTree, aNode, aChildName) )
            {
                OSL_ENSURE(aTree->hasChildNode(aNode, aChildName),"ERROR: Configuration: Existing Property not found by implementation");

                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value.") );
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += sPropertyName;
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.") );

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw beans::PropertyVetoException( sMessage, xContext );
            }
            else
            {
                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value.") );
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += sPropertyName;
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ") );
                sMessage += aTree->getAbsolutePath(aNode).toString();

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw beans::UnknownPropertyException( sMessage, xContext );
            }
        }

        configuration::NodeChange aChange = lock.getNodeUpdater().validateSetValue( aChild, aValue );
        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

            aSender.queryConstraints(aChange);

            aTree->integrate(aChange, aNode, true);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::ConstraintViolation& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XMultiPropertySet
//-----------------------------------------------------------------------------------
void implSetPropertyValues( NodeGroupAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
              lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeGroupAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aValues.getLength(); i < count; ++i)
        {
            rtl::OUString aChildName( aPropertyNames[i] ); // not validated

            configuration::ValueRef aChild( aTree->getChildValue(aNode, aChildName) );

            if (!aChild.isValid())
            {
                if ( configuration::hasChildOrElement(aTree, aNode, aChildName) )
                {
                    OSL_ENSURE(aTree->hasChildNode(aNode, aChildName),"ERROR: Configuration: Existing Property not found by implementation");

                    rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Values.") );
                    sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                    sMessage += aChildName;
                    sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.") );

                    uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                    throw beans::PropertyVetoException( sMessage, xContext );
                }

                OSL_TRACE("Configuration: MultiPropertySet: trying to set unknown property - ignored");
                continue;
            }

            configuration::NodeChange aChange = lock.getNodeUpdater().validateSetValue( aChild, aValues[i] );
            if (aChange.maybeChange())
            {
                aChanges.add(aChange);
            }
        }

        if (!aChanges.test().compact().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges,true));

            aSender.queryConstraints(aChanges);

            aTree->integrate(aChanges, aNode, true);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChanges, true);
        }
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::ConstraintViolation& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XHierarchicalPropertySet
//-----------------------------------------------------------------------------------
void implSetHierarchicalPropertyValue( NodeGroupAccess& rNode, const rtl::OUString& aPropertyName, const uno::Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeGroupAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::RelativePath const aRelPath = configuration::validateRelativePath( aPropertyName, aTree, aNode );

        configuration::AnyNodeRef aNestedValue = configuration::getLocalDescendant( aTree, aNode, aRelPath );

        if (!aNestedValue.isValid())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value. Property '") );
            sMessage += aRelPath.toString();
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' was not found in ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw beans::UnknownPropertyException( sMessage, xContext );
        }
        if (aNestedValue.isNode())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value. Property '") );
            sMessage += aRelPath.toString();
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value property.")  );

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw beans::PropertyVetoException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        configuration::NodeChange aChange = lock.getNodeUpdater().validateSetValue( aNestedValue.toValue(), aValue );
        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,false));

            aSender.queryConstraints(aChange);

            aTree->integrate(aChange, aNode, false);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::ConstraintViolation& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

// XMultiHierarchicalPropertySet
//-----------------------------------------------------------------------------------
void implSetHierarchicalPropertyValues( NodeGroupAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Sequence< uno::Any >& aValues )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeUpdate<NodeGroupAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aValues.getLength(); i < count; ++i)
        try
        {
            configuration::RelativePath aRelPath = configuration::validateRelativePath( aPropertyNames[i], aTree, aNode );

            configuration::AnyNodeRef aNestedValue = configuration::getLocalDescendant( aTree, aNode, aRelPath );

            if (!aNestedValue.isValid())
            {
                OSL_TRACE("Configuration: MultiPropertySet: trying to set unknown property - ignored");
                continue;
            }
            if ( aNestedValue.isNode() )
            {
                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Values.") );
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += aRelPath.toString();
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value property.") );

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw beans::PropertyVetoException( sMessage, xContext );
            }
            OSL_ASSERT(aNode.isValid());

            configuration::NodeChange aChange = lock.getNodeUpdater().validateSetValue( aNestedValue.toValue(), aValues[i] );
            if (aChange.maybeChange())
            {
                aChanges.add(aChange);
            }
        }
        catch (configuration::InvalidName& )
        {
            OSL_TRACE("Configuration: MultiHierarchicalPropertySet: trying to set property <invalid path> - ignored");
            continue;
        }

        if (!aChanges.test().compact().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges,false));

            aSender.queryConstraints(aChanges);

            aTree->integrate(aChanges, aNode, false);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChanges, true); // if we use 'false' we don't need 'Deep' change objects
        }
    }
    catch (configuration::TypeMismatch& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.illegalArgument(2);
    }
    catch (configuration::ConstraintViolation& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

//-----------------------------------------------------------------------------------
// getting values
//-----------------------------------------------------------------------------------

// XPropertySet
//-----------------------------------------------------------------------------------
uno::Any implGetPropertyValue( NodeGroupInfoAccess& rNode,const rtl::OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateChildName(aPropertyName,aTree,aNode);

        configuration::AnyNodeRef aChild( aTree->getAnyChild(aNode, aChildName) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Property not found by implementation");

            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Property Value. Property '") );
            sMessage += aPropertyName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw beans::UnknownPropertyException( sMessage, xContext );
        }

        return configapi::makeElement( rNode.getFactory(), aTree, aChild );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return uno::Any();
}

// XMultiPropertySet
//-----------------------------------------------------------------------------------
uno::Sequence< uno::Any > implGetPropertyValues( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(uno::RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    uno::Sequence<uno::Any> aRet(count);

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            rtl::OUString aChildName( aPropertyNames[i] ); // not validated

            configuration::AnyNodeRef aChild( aTree->getAnyChild(aNode, aChildName) );

            if (aChild.isValid())
            {
                aRet[i] = configapi::makeElement( rNode.getFactory(), aTree, aChild );
            }
            else
            {
                OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Property not found by implementation");
                OSL_TRACE("Configuration: MultiPropertySet: trying to get unknown property - returning void");
            }
        }
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aRet;
}

// XHierarchicalPropertySet
//-----------------------------------------------------------------------------------
uno::Any implGetHierarchicalPropertyValue( NodeGroupInfoAccess& rNode, const rtl::OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::RelativePath aRelPath = configuration::validateRelativePath( aPropertyName, aTree, aNode );

        configuration::AnyNodeRef aNestedNode = configuration::getLocalDescendant( aTree, aNode, aRelPath );

        if (!aNestedNode.isValid())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Property Value. Property '") );
            sMessage += aRelPath.toString();
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw beans::UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        return configapi::makeElement( rNode.getFactory(), aTree, aNestedNode );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return uno::Any();
}

// XMultiHierarchicalPropertySet
//-----------------------------------------------------------------------------------
uno::Sequence< uno::Any > implGetHierarchicalPropertyValues( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(uno::RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    uno::Sequence<uno::Any> aRet(count);

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        try
        {
            configuration::RelativePath aRelPath = configuration::validateRelativePath( aPropertyNames[i], aTree, aNode );

            configuration::AnyNodeRef aNestedValue = configuration::getLocalDescendant( aTree, aNode, aRelPath );

            if (aNestedValue.isValid())
            {
                aRet[i] = configapi::makeElement( rNode.getFactory(), aTree, aNestedValue );
            }
            else
            {
                OSL_TRACE("Configuration: MultiPropertySet: trying to get unknown property - returning void");
            }
        }
        catch (configuration::InvalidName& )
        {
            OSL_TRACE("Configuration: MultiPropertySet: trying to get property from unknown path - returning void");
            OSL_ASSERT(!aRet[i].hasValue());
        }
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aRet;
}

//------------------------------------------------------------------------------------------------------------------
// SPECIAL: XMultiPropertySet::firePropertiesChangeEvent
//------------------------------------------------------------------------------------------------------------------

void implFirePropertiesChangeEvent( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames, const uno::Reference< beans::XPropertiesChangeListener >& xListener )
    throw(uno::RuntimeException)
{
    OSL_ENSURE(xListener.is(), "ERROR: requesting to fire Events to a NULL listener.");
    if (!xListener.is())
    {
        return; // should this be an exception ??
    }

    sal_Int32 const count = aPropertyNames.getLength();
    uno::Sequence<beans::PropertyChangeEvent> aEvents(count);

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );
        configapi::Factory& rFactory = rNode.getFactory();

        sal_Int32 nFire = 0;

        for(sal_Int32 i = 0; i < count; ++i)
        {
            rtl::OUString aChildName( aPropertyNames[i] ); // not validated

            configuration::AnyNodeRef aChild( aTree->getAnyChild(aNode, aChildName) );

            if (aChild.isValid())
            {
                aEvents[nFire].Source = rNode.getUnoInstance();
                aEvents[nFire].PropertyName = aChildName;
                aEvents[nFire].PropertyHandle = -1;

                aEvents[nFire].NewValue = aEvents[nFire].OldValue = configapi::makeElement( rFactory, aTree, aChild );
                nFire++;
            }
            else
            {
                OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Property not found by implementation");
                OSL_TRACE("Configuration: MultiPropertySet: request to fire unknown property - skipping");
            }
        }

        if (nFire < count) aEvents.realloc(nFire);;
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    xListener->propertiesChange(aEvents);
}

//------------------------------------------------------------------------------------------------------------------
// XPropertyState
//------------------------------------------------------------------------------------------------------------------

beans::PropertyState implGetPropertyState( NodeAccess& rNode, const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateChildOrElementName(sPropertyName,aTree,aNode);

        configuration::AnyNodeRef aChild = configuration::getChildOrElement(aTree,aNode,aChildName);
        if (!aChild.isValid())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get PropertyState. Property '") );
            sMessage += sPropertyName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw beans::UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        return aTree->isNodeDefault(aChild)  ? beans::PropertyState_DEFAULT_VALUE :
               aChild.isNode()              ? beans::PropertyState_AMBIGUOUS_VALUE :
                                              beans::PropertyState_DIRECT_VALUE;
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return beans::PropertyState_AMBIGUOUS_VALUE;
}

//-----------------------------------------------------------------------------------
uno::Sequence< beans::PropertyState > implGetPropertyStates( NodeAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    uno::Sequence<beans::PropertyState> aRet(count);

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            rtl::OUString aChildName = configuration::validateChildOrElementName(aPropertyNames[i],aTree,aNode);

            rtl::Reference< configuration::Tree > aChildTree( aTree);

            configuration::AnyNodeRef aChildNode = configuration::getChildOrElement(aChildTree,aNode,aChildName);
            if (!aChildNode.isValid())
            {
                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get PropertyStates. Property '") );
                sMessage += aPropertyNames[i];
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
                sMessage += aTree->getAbsolutePath(aNode).toString();

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw beans::UnknownPropertyException( sMessage, xContext );
            }
            OSL_ASSERT(aChildNode.isValid());

            aRet[i] = aChildTree->isNodeDefault(aChildNode)  ? beans::PropertyState_DEFAULT_VALUE :
                      aChildNode.isNode()                   ? beans::PropertyState_AMBIGUOUS_VALUE :
                                                              beans::PropertyState_DIRECT_VALUE;
        }

    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aRet;
}

//-----------------------------------------------------------------------------------
static inline configuration::NodeChange validateSetToDefaultHelper(configuration::GroupDefaulter& _rDefaulter, configuration::AnyNodeRef _aNode)
{
    if (!_aNode.isNode())
        return _rDefaulter.validateSetToDefaultValue( _aNode.toValue() );

    else
        return _rDefaulter.validateSetToDefaultState( _aNode.toNode() );
}
//-----------------------------------------------------------------------------------
void implSetPropertyToDefault( NodeGroupAccess& rNode, const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    try
    {
        UnoApiLock aWithDefaultLock;
        GuardedNodeUpdate<NodeGroupAccess> lock( withDefaultData( rNode ) );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::GroupDefaulter aDefaulter = lock.getNodeDefaulter();

        rtl::OUString aChildName = configuration::validateChildName(sPropertyName,aTree,aNode);

        configuration::AnyNodeRef aChild( aTree->getAnyChild(aNode, aChildName) );

        configuration::NodeChange aChange = validateSetToDefaultHelper( aDefaulter, aChild );

        const bool bLocal = !aDefaulter.hasDoneSet();

        if (aChange.test().isChange() )
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,bLocal));

            aSender.queryConstraints(aChange);

            aTree->integrate(aChange, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}
//-----------------------------------------------------------------------------------
void implSetPropertiesToDefault( NodeGroupAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    try
    {
        UnoApiLock aWithDefaultLock;
        GuardedNodeUpdate<NodeGroupAccess> lock( withDefaultData( rNode ) );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::GroupDefaulter aDefaulter = lock.getNodeDefaulter();

        configuration::NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aPropertyNames.getLength(); i < count; ++i)
        {
            rtl::OUString aChildName = configuration::validateChildName( aPropertyNames[i], aTree, aNode ); // validated

            configuration::AnyNodeRef aChild( aTree->getAnyChild(aNode, aChildName) );

            if (!aChild.isValid())
            {
                OSL_ENSURE(!configuration::hasChildOrElement(aTree, aNode, aChildName),"ERROR: Configuration: Existing Property not found by implementation");

                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default.") );
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += aChildName;
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ") );
                sMessage += aTree->getAbsolutePath(aNode).toString();

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw beans::UnknownPropertyException( sMessage, xContext );
            }
            OSL_ASSERT(aNode.isValid());

            if (!aTree->hasNodeDefault(aChild))
            {
                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default.") );
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += aChildName;

                if (aChild.isNode())
                    sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.") );

                else
                    sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' does not have a default value.") );

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw beans::UnknownPropertyException( sMessage, xContext );
            }

            configuration::NodeChange aChildChange = validateSetToDefaultHelper(aDefaulter, aChild );
            if (aChildChange.maybeChange())
                aChanges.add(aChildChange);
        }

        const bool bLocal = !aDefaulter.hasDoneSet();

        if (!aChanges.test().compact().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges,bLocal));

            aSender.queryConstraints(aChanges);

            aTree->integrate(aChanges, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChanges, bLocal);
        }

    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

//-----------------------------------------------------------------------------------
void implSetAllPropertiesToDefault( NodeGroupAccess& rNode )
    throw(uno::RuntimeException)
{
    try
    {
        UnoApiLock aWithDefaultLock;
        GuardedNodeUpdate<NodeGroupAccess> lock( withDefaultData( rNode ) );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        configuration::GroupDefaulter aDefaulter = lock.getNodeDefaulter();

        configuration::NodeChanges aChanges = aDefaulter.validateSetAllToDefault( );

        const bool bLocal = !aDefaulter.hasDoneSet();

        if (!aChanges.test().compact().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges,bLocal));

            aSender.queryConstraints(aChanges);

            aTree->integrate(aChanges, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChanges, bLocal);
        }

    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

//-----------------------------------------------------------------------------------
uno::Any implGetPropertyDefault( NodeGroupInfoAccess& rNode, const rtl::OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aDefault;
    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        rtl::OUString aChildName = configuration::validateChildName(sPropertyName,aTree,aNode);

        configuration::AnyNodeRef aChildNode = aTree->getAnyChild(aNode, aChildName);
        if (!aChildNode.isValid())
        {
            rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default. Property '") );
            sMessage += sPropertyName;
            sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
            sMessage += aTree->getAbsolutePath(aNode).toString();

            uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw beans::UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        if (!aChildNode.isNode())
        {
            aDefault = aTree->getNodeDefaultValue(aChildNode.toValue());
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aDefault;
}

//-----------------------------------------------------------------------------------
uno::Sequence< uno::Any > implGetPropertyDefaults( NodeGroupInfoAccess& rNode, const uno::Sequence< rtl::OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    uno::Sequence<uno::Any> aDefaults(count);

    try
    {
        GuardedNodeData<NodeAccess> lock( rNode );

        rtl::Reference< configuration::Tree > const aTree( lock.getTree() );
        configuration::NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            rtl::OUString aChildName = configuration::validateChildName(aPropertyNames[i],aTree,aNode);

            configuration::AnyNodeRef aChildNode = aTree->getAnyChild(aNode, aChildName);
            if (!aChildNode.isValid())
            {
                rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default. Property '") );
                sMessage += aPropertyNames[i];
                sMessage += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
                sMessage += aTree->getAbsolutePath(aNode).toString();

                uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw beans::UnknownPropertyException( sMessage, xContext );
            }
            OSL_ASSERT(aNode.isValid());

            if (!aChildNode.isNode())
            {
                aDefaults[i] = aTree->getNodeDefaultValue(aChildNode.toValue());
            }
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default: ") );
        uno::Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw beans::UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    return aDefaults;
}

//-----------------------------------------------------------------------------------
    } // namespace configapi

} // namespace configmgr


