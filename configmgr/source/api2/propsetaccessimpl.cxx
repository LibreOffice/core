/*************************************************************************
 *
 *  $RCSfile: propsetaccessimpl.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:36 $
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

#include "propsetaccessimpl.hxx"

#ifndef CONFIGMGR_API_PROPERTYINFOIMPL_HXX_
#include "propertyinfohelper.hxx"
#endif
#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#include "apinodeaccess.hxx"
#endif
#ifndef CONFIGMGR_API_NODEUPDATE_HXX_
#include "apinodeupdate.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGVALUEREF_HXX_
#include "valueref.hxx"
#endif
#ifndef CONFIGMGR_CONFIGANYNODE_HXX_
#include "anynoderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGGROUP_HXX_
#include "configgroup.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNOTIFIER_HXX_
#include "confignotifier.hxx"
#endif
#ifndef CONFIGMGR_API_BROADCASTER_HXX_
#include "broadcaster.hxx"
#endif
#ifndef CONFIGMGR_API_APITYPES_HXX_
#include "apitypes.hxx"
#endif
#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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

        using uno::RuntimeException;
        using uno::Reference;
        using uno::Any;
        using uno::Sequence;

        using lang::IllegalArgumentException;
        using lang::WrappedTargetException;
        using beans::UnknownPropertyException;
        using beans::PropertyVetoException;

        using configuration::AnyNodeRef;
        using configuration::NodeRef;
        using configuration::ValueRef;

        using configuration::Tree;
        using configuration::NodeChange;
        using configuration::NodeChanges;

        using configuration::Name;
        using configuration::AbsolutePath;
        using configuration::RelativePath;
        using configuration::validateChildName;
        using configuration::validateChildOrElementName;

        using namespace beans;
        using namespace uno;

//-----------------------------------------------------------------------------------
// a helper class
//-----------------------------------------------------------------------------------

class CollectProperties : configuration::NodeVisitor
{
    std::vector< Property >     m_aProperties;
    sal_Bool                    m_bReadonly;
public:
    CollectProperties(sal_Bool _bReadonly)
    : m_bReadonly(_bReadonly)
    { }
    CollectProperties(sal_Bool _bReadonly, sal_Int32 _nCount)
    : m_bReadonly(_bReadonly)
    { m_aProperties.reserve(_nCount); }

    Property forNode(Tree const& _aPropertyTree, AnyNodeRef const& _rNode)
    {
        OSL_ENSURE( _aPropertyTree.isValidNode(_rNode), "Node to retrieve properties from does not match tree");
        reset();
        _aPropertyTree.visit(_rNode, *this);
        OSL_ENSURE(m_aProperties.size() == 1, "CollectProperties::forNode: not exactly one result property!");
        return m_aProperties[0];
    }
    Sequence<Property> forChildren(Tree const& _aPropertyTree, NodeRef const& _rNode)
    {
        OSL_ENSURE( _aPropertyTree.isValidNode(_rNode), "Node to retrieve properties from does not match tree");
        reset();
        _aPropertyTree.dispatchToChildren(_rNode, *this);
        return makeSequence(m_aProperties);
    }

private:
    typedef node::Attributes NodeAttributes;

    void    reset() { m_aProperties.clear(); }

    NodeAttributes  adjustAttributes(NodeAttributes nNodeAttr);

    Result  handle(Tree const& _aTree, NodeRef const& _rValue);
    Result  handle(Tree const& _aTree, ValueRef const& _rValue);
};

//-----------------------------------------------------------------------------------
CollectProperties::NodeAttributes CollectProperties::adjustAttributes(NodeAttributes nNodeAttr)
{
    if (m_bReadonly) nNodeAttr.bWritable = false;

    return nNodeAttr;
}

//-----------------------------------------------------------------------------------
CollectProperties::Result CollectProperties::handle(Tree const& _aTree, ValueRef const& _rValue)
{
    // can be default ?
    m_aProperties.push_back(
                helperMakeProperty( _aTree.getName(_rValue),
                                    adjustAttributes(_aTree.getAttributes(_rValue)),
                                    _aTree.getUnoType(_rValue),
                                    _aTree.hasNodeDefault(_rValue)
                                )
            );

    return CONTINUE;
}

//-----------------------------------------------------------------------------------
CollectProperties::Result CollectProperties::handle(Tree const& _aTree, NodeRef const& _rNode)
{
    // can be default ?
    OSL_ENSURE( configuration::isStructuralNode(_aTree,_rNode),
                "Unexpected value element node. Cannot get proper type for this node as property" );

    m_aProperties.push_back(
                helperMakeProperty( _aTree.getName(_rNode),
                                    adjustAttributes(_aTree.getAttributes(_rNode)),
                                    getUnoInterfaceType(),
                                    _aTree.hasNodeDefault(_rNode)
                                )
            );

    return CONTINUE;
}
//-----------------------------------------------------------------------------------
// another helper class - disabled, as it doesn't work well with Accessor changes
//-----------------------------------------------------------------------------------
#if 0

class TreeNodePropertySetInfo
    :public ::cppu::WeakImplHelper1< beans::XPropertySetInfo >
{
    configapi::NodeAccess&  m_rNodeAccess;
    configuration::Tree     m_aTree;
    configuration::NodeRef  m_aNode;
    sal_Bool                m_bReadonly;

public:
    TreeNodePropertySetInfo(NodeGroupInfoAccess& _rNode, sal_Bool _bReadonly )
        :m_rNodeAccess(_rNode)
        ,m_aTree(_rNode.getTree())
        ,m_aNode(_rNode.getNode())
        ,m_bReadonly(_bReadonly)
    {
        // TODO: need to be a event listener on the node
    }

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() throw(RuntimeException);
    virtual Property SAL_CALL   getPropertyByName(const OUString& _rPropertyName) throw(UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL   hasPropertyByName(const OUString& _rPropertyName) throw(RuntimeException);
};

//-----------------------------------------------------------------------------------
uno::Sequence< beans::Property > SAL_CALL TreeNodePropertySetInfo::getProperties() throw(RuntimeException)
{
    OReadSynchronized aGuard(m_rNodeAccess.getDataLock());
    return CollectProperties(m_bReadonly).forChildren(m_aTree, m_aNode);
}

//-----------------------------------------------------------------------------------
Property SAL_CALL TreeNodePropertySetInfo::getPropertyByName(const OUString& _rPropertyName)
    throw(UnknownPropertyException, RuntimeException)
{
    OReadSynchronized aGuard(m_rNodeAccess.getDataLock());

    Name aName = configuration::makeNodeName(_rPropertyName, Name::NoValidate());

    if (!m_aTree.hasChild(m_aNode, aName))
    {
        OUString sMessage = OUString::createFromAscii("Configuration - ");
        sMessage += OUString::createFromAscii("No Property named '");
        sMessage += _rPropertyName;
        sMessage += OUString::createFromAscii("' in this PropertySetInfo");
        throw UnknownPropertyException(sMessage, static_cast<XPropertySetInfo*>(this));
    }

    AnyNodeRef aNode = m_aTree.getAnyChild(m_aNode, aName);
    OSL_ENSURE(aNode.isValid(), "NodePropertySetInfo::getPropertyByName: got no node although hasChild returned TRUE!");
    return CollectProperties(m_bReadonly, 1).forNode(m_aTree, aNode);
}

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL TreeNodePropertySetInfo::hasPropertyByName(const OUString& _rPropertyName)
    throw(RuntimeException)
{
    OReadSynchronized aGuard(m_rNodeAccess.getDataLock());

    Name aName = configuration::makeNodeName(_rPropertyName, Name::NoValidate());

    return m_aTree.hasChild(m_aNode, aName);
}
#endif
//-----------------------------------------------------------------------------------
// yet another helper class (more robust, but can't well be extended to be a HierarchicalPropertySetInfo though)
//-----------------------------------------------------------------------------------

class NodePropertySetInfo
    :public ::cppu::WeakImplHelper1< beans::XPropertySetInfo >
{
    Sequence< Property > const m_aProperties;

public:
    NodePropertySetInfo(Sequence< Property > const& _aProperties) throw(RuntimeException)
    : m_aProperties(_aProperties)
    {
    }

    static NodePropertySetInfo* create(NodeGroupInfoAccess& _rNode, data::Accessor const& _aDataAccessor, sal_Bool _bReadonly ) throw(RuntimeException);
    Property const* begin() const throw() { return m_aProperties.getConstArray(); }
    Property const* end()   const throw() { return m_aProperties.getConstArray() + m_aProperties.getLength(); }

    Property const* find(const OUString& _rPropertyName) const throw(RuntimeException);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties() throw(RuntimeException);
    virtual Property SAL_CALL   getPropertyByName(const OUString& _rPropertyName) throw(UnknownPropertyException, RuntimeException);
    virtual sal_Bool SAL_CALL   hasPropertyByName(const OUString& _rPropertyName) throw(RuntimeException);
};

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
NodePropertySetInfo* NodePropertySetInfo::create(NodeGroupInfoAccess& _rNode, data::Accessor const& _aDataAccessor, sal_Bool _bReadonly ) throw(RuntimeException)
{
    osl::MutexGuard aGuard(_rNode.getDataLock());

    configuration::Tree aTree( _rNode.getTree(_aDataAccessor) );
    OSL_ENSURE( !aTree.isEmpty(), "WARNING: Getting Tree information requires a valid tree");
    if (aTree.isEmpty()) return NULL;

    configuration::NodeRef aNode( _rNode.getNodeRef() );
    OSL_ENSURE( aTree.isValidNode(aNode), "ERROR: Tree does not match node");

    Sequence< Property > aProperties = CollectProperties(_bReadonly).forChildren(aTree,aNode);
    OSL_ENSURE( aProperties.getLength() > 0, "ERROR: PropertySet (Configuration group) has no Properties");

    return new NodePropertySetInfo( aProperties );
}

//-----------------------------------------------------------------------------------
struct MatchName // : std::unary_function< Property, bool >
{
    OUString sName;
    MatchName(OUString const& _sName)  throw(RuntimeException)
    : sName(_sName)
    {
    }

    bool operator()(Property const& _aProperty) const
    {
        return !!(_aProperty.Name == this->sName);
    }
};

Property const* NodePropertySetInfo::find(const OUString& _rPropertyName)  const throw(RuntimeException)
{
    Property const* const first = this->begin();
    Property const* const last  = this->end();

    return std::find_if(first,last,MatchName(_rPropertyName));
}

//-----------------------------------------------------------------------------------
uno::Sequence< beans::Property > SAL_CALL NodePropertySetInfo::getProperties() throw(RuntimeException)
{
    return m_aProperties;
}
//-----------------------------------------------------------------------------------
Property SAL_CALL NodePropertySetInfo::getPropertyByName(const OUString& _rPropertyName)
    throw(UnknownPropertyException, RuntimeException)
{
    Property const* pFound = find(_rPropertyName);

    if (pFound == this->end())
    {
        OUString sMessage = OUString::createFromAscii("Configuration - ");
        sMessage += OUString::createFromAscii("No Property named '");
        sMessage += _rPropertyName;
        sMessage += OUString::createFromAscii("' in this PropertySetInfo");
        throw UnknownPropertyException(sMessage, static_cast<XPropertySetInfo*>(this));
    }

    return *pFound;
}

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL NodePropertySetInfo::hasPropertyByName(const OUString& _rPropertyName)
    throw(RuntimeException)
{
    Property const* pFound = find(_rPropertyName);

    return (pFound != this->end());
}

// Interface methods
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// getting Property Metadata
//-----------------------------------------------------------------------------------

// XPropertySet & XMultiPropertySet
//-----------------------------------------------------------------------------------
Reference< beans::XPropertySetInfo > implGetPropertySetInfo( NodeGroupInfoAccess& rNode, sal_Bool _bWriteable )
    throw(RuntimeException)
{
    GuardedNodeDataAccess lock( rNode );
    return NodePropertySetInfo::create(rNode, lock.getDataAccessor(), !_bWriteable);
}

// XHierarchicalPropertySet & XHierarchicalMultiPropertySet
//-----------------------------------------------------------------------------------
Reference< beans::XHierarchicalPropertySetInfo > implGetHierarchicalPropertySetInfo( NodeGroupInfoAccess& rNode )
    throw(RuntimeException)
{
    // TODO: Implement
    return 0;
}

//-----------------------------------------------------------------------------------
// setting values - may all throw (PropertyVeto)Exceptions on read-only property sets
//-----------------------------------------------------------------------------------

// XPropertySet
//-----------------------------------------------------------------------------------
void implSetPropertyValue( NodeGroupAccess& rNode, const OUString& sPropertyName, const Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
              lang::WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateChildName(sPropertyName,aTree,aNode);

        ValueRef aChild( aTree.getChildValue(aNode, aChildName) );

        if (!aChild.isValid())
        {
            if ( configuration::hasChildOrElement(aTree, aNode, aChildName) )
            {
                OSL_ENSURE(aTree.hasChildNode(aNode, aChildName),"ERROR: Configuration: Existing Property not found by implementation");

                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value.") );
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += sPropertyName;
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.") );

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw PropertyVetoException( sMessage, xContext );
            }
            else
            {
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value.") );
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += sPropertyName;
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ") );
                sMessage += aTree.getAbsolutePath(aNode).toString();

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw UnknownPropertyException( sMessage, xContext );
            }
        }

        NodeChange aChange = lock.getNodeUpdater().validateSetValue( aChild, aValue );
        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,true));

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, true);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
void implSetPropertyValues( NodeGroupAccess& rNode, const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
              lang::WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aValues.getLength(); i < count; ++i)
        {
            Name aChildName = configuration::makeNodeName( aPropertyNames[i], Name::NoValidate() ); // not validated

            ValueRef aChild( aTree.getChildValue(aNode, aChildName) );

            if (!aChild.isValid())
            {
                if ( configuration::hasChildOrElement(aTree, aNode, aChildName) )
                {
                    OSL_ENSURE(aTree.hasChildNode(aNode, aChildName),"ERROR: Configuration: Existing Property not found by implementation");

                    OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Values.") );
                    sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                    sMessage += aChildName.toString();
                    sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.") );

                    Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                    throw PropertyVetoException( sMessage, xContext );
                }

                OSL_TRACE("Configuration: MultiPropertySet: trying to set unknown property - ignored");
                continue;
            }

            NodeChange aChange = lock.getNodeUpdater().validateSetValue( aChild, aValues[i] );
            if (aChange.maybeChange())
            {
                aChanges.add(aChange);
            }
        }

        if (!aChanges.test().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges.compact(),true));

            aSender.queryConstraints(aChanges);

            aTree.integrate(aChanges, aNode, true);

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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
void implSetHierarchicalPropertyValue( NodeGroupAccess& rNode, const OUString& aPropertyName, const Any& aValue )
    throw(beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, RuntimeException)
{
    using configuration::validateRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::getLocalDescendant; // should actually be found by "Koenig" lookup, but MSVC6 fails

    try
    {
        GuardedGroupUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        RelativePath const aRelPath = validateRelativePath( aPropertyName, aTree, aNode );

        AnyNodeRef aNestedValue = getLocalDescendant( aTree, aNode, aRelPath );

        if (!aNestedValue.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value. Property '") );
            sMessage += aRelPath.toString();
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' was not found in ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        if (aNestedValue.isNode())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value. Property '") );
            sMessage += aRelPath.toString();
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value property.")  );

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw PropertyVetoException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        NodeChange aChange = lock.getNodeUpdater().validateSetValue( aNestedValue.toValue(), aValue );
        if (aChange.test().isChange())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,false));

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, false);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
void implSetHierarchicalPropertyValues( NodeGroupAccess& rNode, const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues )
    throw(beans::PropertyVetoException, lang::IllegalArgumentException,
          lang::WrappedTargetException, RuntimeException)
{
    using configuration::validateRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::getLocalDescendant; // should actually be found by "Koenig" lookup, but MSVC6 fails

    try
    {
        GuardedGroupUpdateAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aValues.getLength(); i < count; ++i)
        try
        {
            RelativePath aRelPath = validateRelativePath( aPropertyNames[i], aTree, aNode );

            AnyNodeRef aNestedValue = getLocalDescendant( aTree, aNode, aRelPath );

            if (!aNestedValue.isValid())
            {
                OSL_TRACE("Configuration: MultiPropertySet: trying to set unknown property - ignored");
                continue;
            }
            if ( aNestedValue.isNode() )
            {
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Values.") );
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += aRelPath.toString();
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value property.") );

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw PropertyVetoException( sMessage, xContext );
            }
            OSL_ASSERT(aNode.isValid());

            NodeChange aChange = lock.getNodeUpdater().validateSetValue( aNestedValue.toValue(), aValues[i] );
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

        if (!aChanges.test().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges.compact(),false));

            aSender.queryConstraints(aChanges);

            aTree.integrate(aChanges, aNode, false);

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
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw PropertyVetoException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot set Property Value: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
Any implGetPropertyValue( NodeGroupInfoAccess& rNode,const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateChildName(aPropertyName,aTree,aNode);

        AnyNodeRef aChild( aTree.getAnyChild(aNode, aChildName) );

        if (!aChild.isValid())
        {
            OSL_ENSURE(!configuration::hasChildOrElement(aTree,aNode,aChildName),"ERROR: Configuration: Existing Property not found by implementation");

            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Property Value. Property '") );
            sMessage += aPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }

        return configapi::makeElement( rNode.getFactory(), aTree, aChild );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Property Value: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return Any();
}

// XMultiPropertySet
//-----------------------------------------------------------------------------------
Sequence< Any > implGetPropertyValues( NodeGroupInfoAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<Any> aRet(count);

    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            Name aChildName = configuration::makeNodeName( aPropertyNames[i], Name::NoValidate() ); // not validated

            AnyNodeRef aChild( aTree.getAnyChild(aNode, aChildName) );

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
Any implGetHierarchicalPropertyValue( NodeGroupInfoAccess& rNode, const OUString& aPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    using configuration::validateRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::getLocalDescendant; // should actually be found by "Koenig" lookup, but MSVC6 fails
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        RelativePath aRelPath = validateRelativePath( aPropertyName, aTree, aNode );

        AnyNodeRef aNestedNode = getLocalDescendant( aTree, aNode, aRelPath );

        if (!aNestedNode.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Property Value. Property '") );
            sMessage += aRelPath.toString();
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        return configapi::makeElement( rNode.getFactory(), aTree, aNestedNode );
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }

    // unreachable, but still there to make some compilers happy
    OSL_ASSERT(!"Unreachable code");
    return Any();
}

// XMultiHierarchicalPropertySet
//-----------------------------------------------------------------------------------
Sequence< Any > implGetHierarchicalPropertyValues( NodeGroupInfoAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(RuntimeException)
{
    using configuration::validateRelativePath; // should actually be found by "Koenig" lookup, but MSVC6 fails
    using configuration::getLocalDescendant; // should actually be found by "Koenig" lookup, but MSVC6 fails

    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<Any> aRet(count);

    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        try
        {
            RelativePath aRelPath = validateRelativePath( aPropertyNames[i], aTree, aNode );

            AnyNodeRef aNestedValue = getLocalDescendant( aTree, aNode, aRelPath );

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

void implFirePropertiesChangeEvent( NodeGroupInfoAccess& rNode, const Sequence< OUString >& aPropertyNames, const Reference< beans::XPropertiesChangeListener >& xListener )
    throw(RuntimeException)
{
    OSL_ENSURE(xListener.is(), "ERROR: requesting to fire Events to a NULL listener.");
    if (!xListener.is())
    {
        return; // should this be an exception ??
    }

    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<beans::PropertyChangeEvent> aEvents(count);

    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );
        configapi::Factory& rFactory = rNode.getFactory();

        sal_Int32 nFire = 0;

        for(sal_Int32 i = 0; i < count; ++i)
        {
            Name aChildName = configuration::makeNodeName( aPropertyNames[i], Name::NoValidate() ); // not validated

            AnyNodeRef aChild( aTree.getAnyChild(aNode, aChildName) );

            if (aChild.isValid())
            {
                aEvents[nFire].Source = rNode.getUnoInstance();
                aEvents[nFire].PropertyName = aChildName.toString();
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

beans::PropertyState implGetPropertyState( NodeAccess& rNode, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    try
    {
        using configuration::getChildOrElement;

        GuardedNodeDataAccess lock( rNode );

        Tree aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateChildOrElementName(sPropertyName,aTree,aNode);

        AnyNodeRef aChild = getChildOrElement(aTree,aNode,aChildName);
        if (!aChild.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get PropertyState. Property '") );
            sMessage += sPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        return aTree.isNodeDefault(aChild)  ? beans::PropertyState_DEFAULT_VALUE :
               aChild.isNode()              ? beans::PropertyState_AMBIGUOUS_VALUE :
                                              beans::PropertyState_DIRECT_VALUE;
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get PropertyState: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
Sequence< beans::PropertyState > implGetPropertyStates( NodeAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<beans::PropertyState> aRet(count);

    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            using configuration::getChildOrElement;

            Name aChildName = validateChildOrElementName(aPropertyNames[i],aTree,aNode);

            Tree aChildTree( aTree);

            AnyNodeRef aChildNode = getChildOrElement(aChildTree,aNode,aChildName);
            if (!aChildNode.isValid())
            {
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get PropertyStates. Property '") );
                sMessage += aPropertyNames[i];
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' could not be found in ")  );
                sMessage += aTree.getAbsolutePath(aNode).toString();

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw UnknownPropertyException( sMessage, xContext );
            }
            OSL_ASSERT(aChildNode.isValid());

            aRet[i] = aChildTree.isNodeDefault(aChildNode)  ? beans::PropertyState_DEFAULT_VALUE :
                      aChildNode.isNode()                   ? beans::PropertyState_AMBIGUOUS_VALUE :
                                                              beans::PropertyState_DIRECT_VALUE;
        }

    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
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
static inline NodeChange validateSetToDefaultHelper(configuration::GroupDefaulter& _rDefaulter, AnyNodeRef _aNode)
{
    if (!_aNode.isNode())
        return _rDefaulter.validateSetToDefaultValue( _aNode.toValue() );

    else
        return _rDefaulter.validateSetToDefaultState( _aNode.toNode() );
}
//-----------------------------------------------------------------------------------
void implSetPropertyToDefault( NodeGroupAccess& rNode, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess lock( withDefaultData( rNode ) );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        configuration::GroupDefaulter aDefaulter = lock.getNodeDefaulter();

        Name aChildName = validateChildName(sPropertyName,aTree,aNode);

        AnyNodeRef aChild( aTree.getAnyChild(aNode, aChildName) );

        NodeChange aChange = validateSetToDefaultHelper( aDefaulter, aChild );

        const bool bLocal = !aDefaulter.hasDoneSet();

        if (aChange.test().isChange() )
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChange,bLocal));

            aSender.queryConstraints(aChange);

            aTree.integrate(aChange, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChange);
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}
//-----------------------------------------------------------------------------------
void implSetPropertiesToDefault( NodeGroupAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess lock( withDefaultData( rNode ) );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        configuration::GroupDefaulter aDefaulter = lock.getNodeDefaulter();

        NodeChanges aChanges;
        for(sal_Int32 i = 0, count= aPropertyNames.getLength(); i < count; ++i)
        {
            Name aChildName = validateChildName( aPropertyNames[i], aTree, aNode ); // validated

            AnyNodeRef aChild( aTree.getAnyChild(aNode, aChildName) );

            if (!aChild.isValid())
            {
                OSL_ENSURE(!configuration::hasChildOrElement(aTree, aNode, aChildName),"ERROR: Configuration: Existing Property not found by implementation");

                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default.") );
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += aChildName.toString();
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ") );
                sMessage += aTree.getAbsolutePath(aNode).toString();

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw UnknownPropertyException( sMessage, xContext );
            }
            OSL_ASSERT(aNode.isValid());

            if (!aTree.hasNodeDefault(aChild))
            {
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Default.") );
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM(" Property '") );
                sMessage += aChildName.toString();

                if (aChild.isNode())
                    sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' is not a simple value.") );

                else
                    sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' does not have a default value.") );

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw UnknownPropertyException( sMessage, xContext );
            }

            NodeChange aChildChange = validateSetToDefaultHelper(aDefaulter, aChild );
            if (aChildChange.maybeChange())
                aChanges.add(aChildChange);
        }

        const bool bLocal = !aDefaulter.hasDoneSet();

        if (!aChanges.test().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges.compact(),bLocal));

            aSender.queryConstraints(aChanges);

            aTree.integrate(aChanges, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChanges, bLocal);
        }

    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
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
    throw(RuntimeException)
{
    try
    {
        GuardedGroupUpdateAccess lock( withDefaultData( rNode ) );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        configuration::GroupDefaulter aDefaulter = lock.getNodeDefaulter();

        NodeChanges aChanges = aDefaulter.validateSetAllToDefault( );

        const bool bLocal = !aDefaulter.hasDoneSet();

        if (!aChanges.test().isEmpty())
        {
            Broadcaster aSender(rNode.getNotifier().makeBroadcaster(aChanges.compact(),bLocal));

            aSender.queryConstraints(aChanges);

            aTree.integrate(aChanges, aNode, bLocal);

            lock.clearForBroadcast();
            aSender.notifyListeners(aChanges, bLocal);
        }

    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot restore Defaults: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::Exception& ex)
    {
        ExceptionMapper e(ex);
        e.setContext( rNode.getUnoInstance() );
        e.unhandled();
    }
}

//-----------------------------------------------------------------------------------
Any implGetPropertyDefault( NodeGroupInfoAccess& rNode, const OUString& sPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    Any aDefault;
    try
    {
        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        Name aChildName = validateChildName(sPropertyName,aTree,aNode);

        AnyNodeRef aChildNode = aTree.getAnyChild(aNode, aChildName);
        if (!aChildNode.isValid())
        {
            OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default. Property '") );
            sMessage += sPropertyName;
            sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
            sMessage += aTree.getAbsolutePath(aNode).toString();

            Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
            throw UnknownPropertyException( sMessage, xContext );
        }
        OSL_ASSERT(aNode.isValid());

        if (!aChildNode.isNode())
        {
            aDefault = aTree.getNodeDefaultValue(aChildNode.toValue());
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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
Sequence< Any > implGetPropertyDefaults( NodeGroupInfoAccess& rNode, const Sequence< OUString >& aPropertyNames )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, RuntimeException)
{
    sal_Int32 const count = aPropertyNames.getLength();
    Sequence<Any> aDefaults(count);

    try
    {
        using configuration::getChildOrElement;

        GuardedNodeDataAccess lock( rNode );

        Tree const aTree( lock.getTree() );
        NodeRef const aNode( lock.getNode() );

        for(sal_Int32 i = 0; i < count; ++i)
        {
            Name aChildName = validateChildName(aPropertyNames[i],aTree,aNode);

            AnyNodeRef aChildNode = aTree.getAnyChild(aNode, aChildName);
            if (!aChildNode.isValid())
            {
                OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default. Property '") );
                sMessage += aPropertyNames[i];
                sMessage += OUString( RTL_CONSTASCII_USTRINGPARAM("' not found in ")  );
                sMessage += aTree.getAbsolutePath(aNode).toString();

                Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
                throw UnknownPropertyException( sMessage, xContext );
            }
            OSL_ASSERT(aNode.isValid());

            if (!aChildNode.isNode())
            {
                aDefaults[i] = aTree.getNodeDefaultValue(aChildNode.toValue());
            }
        }
    }
    catch (configuration::InvalidName& ex)
    {
        ExceptionMapper e(ex);
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( e.message(), xContext );
    }
    catch (configuration::ConstraintViolation & ex)
    {
        ExceptionMapper e(ex);
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default: ") );
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        throw UnknownPropertyException( sMessage += e.message(), xContext );
    }
    catch (configuration::WrappedUnoException& ex)
    {
        Reference<uno::XInterface> xContext( rNode.getUnoInstance() );
        OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration - Cannot get Default: ") );
        throw WrappedTargetException( sMessage += ex.extractMessage(), xContext, ex.getAnyUnoException() );
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


