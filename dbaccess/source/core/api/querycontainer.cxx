/*************************************************************************
 *
 *  $RCSfile: querycontainer.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:19:18 $
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

#ifndef _DBA_CORE_QUERYCONTAINER_HXX_
#include "querycontainer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_COREAPI_QUERY_HXX_
#include "query.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OQueryContainer
//==========================================================================
DBG_NAME(OQueryContainer)
//------------------------------------------------------------------------------
OQueryContainer::OQueryContainer(
                OWeakObject& _rConnection, Mutex& _rMutex, const Reference< XNameContainer >& _rxCommandDefinitions,
                const OConfigurationTreeRoot& _rRootConfigNode, const Reference< XMultiServiceFactory >& _rxORB)
    :OConfigurationFlushable(_rMutex)
    ,m_rParent(_rConnection)
    ,m_rMutex(_rMutex)
    ,m_aContainerListeners(_rMutex)
    ,m_xCommandDefinitions(_rxCommandDefinitions)
    ,m_xORB(_rxORB)
{
    DBG_CTOR(OQueryContainer, NULL);

    m_aConfigurationNode = _rRootConfigNode;

    m_pCommandsListener = new OCommandsListener(this);
    m_pCommandsListener->acquire();

    DBG_ASSERT(m_aConfigurationNode.isValid() && m_xORB.is(), "OQueryContainer::OQueryContainer : invalid arguments !");
    m_aConfigurationNode.setEscape(m_aConfigurationNode.isSetNode());

    increment(m_refCount);
    {
        Reference< XContainer > xContainer(m_xCommandDefinitions, UNO_QUERY);
        DBG_ASSERT(xContainer.is(), "OQueryContainer::OQueryContainer : the CommandDefinitions container is invalid !");
        xContainer->addContainerListener(m_pCommandsListener);

        // fill my structures with dummies
        OQuery* pDummyObject = NULL;

        Sequence< ::rtl::OUString > sDefinitionNames = m_xCommandDefinitions->getElementNames();
        const ::rtl::OUString* pDefinitionNames = sDefinitionNames.getConstArray();
        for (sal_Int32 i=0; i<sDefinitionNames.getLength(); ++i, ++pDefinitionNames)
        {
            m_aQueriesIndexed.push_back(m_aQueries.insert(Queries::value_type(*pDefinitionNames, pDummyObject)).first);
        }
        // (they will be filled as soon as they are accessed)
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------------------
OQueryContainer::~OQueryContainer()
{
    DBG_DTOR(OQueryContainer, NULL);
    //  dispose();
        //  maybe we're already disposed, but this should be uncritical
}

//------------------------------------------------------------------------------
Any SAL_CALL OQueryContainer::queryInterface( const Type& _rType ) throw(RuntimeException)
{
    Any aReturn = OQueryContainer_Base::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OConfigurationFlushable::queryInterface(_rType);
    return aReturn;
}

//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL OQueryContainer::getTypes() throw (RuntimeException)
{
    return concatSequences(OQueryContainer_Base::getTypes(), OConfigurationFlushable::getTypes());
}

//------------------------------------------------------------------------------
void OQueryContainer::dispose()
{
    MutexGuard aGuard(m_rMutex);

    OConfigurationFlushable::disposing();

    // say our listeners goobye
    EventObject aEvt(*this);
    m_aContainerListeners.disposeAndClear(aEvt);

    for (ConstQueriesIterator i = m_aQueries.begin(); i != m_aQueries.end(); ++i)
    {
        OQuery* pQuery = i->second;
        if(pQuery)
        {
            pQuery->dispose();
            pQuery->release();
        }
    }
    m_aQueriesIndexed.clear();
        //  !!! do this before clearing the map which the vector elements refer to !!!
    m_aQueries.clear();

    Reference< XContainer > xContainer(m_xCommandDefinitions, UNO_QUERY);
    if (xContainer.is())
        xContainer->removeContainerListener(m_pCommandsListener);
    if(m_pCommandsListener)
        m_pCommandsListener->release();
    m_pCommandsListener     = NULL;
    m_xCommandDefinitions   = NULL;
}

// XServiceInfo
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2(OQueryContainer, "com.sun.star.sdb.dbaccess.OQueryContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDB_QUERIES)

// XDataDescriptorFactory
//--------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OQueryContainer::createDataDescriptor(  ) throw(RuntimeException)
{
    return new OQueryDescriptor();
}

// XAppend
//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::appendByDescriptor( const Reference< XPropertySet >& _rxDesc ) throw(SQLException, ElementExistException, RuntimeException)
{
    Reference< XPropertySet > xNewObject;
    ::rtl::OUString sNewObjectName;
    {
        MutexGuard aGuard(m_rMutex);

        OQueryDescriptor* pImpl = NULL;
                comphelper::getImplementation(pImpl, Reference< XInterface >(_rxDesc.get()));
        DBG_ASSERT(pImpl != NULL, "OQueryContainer::appendByDescriptor : can't fully handle this descriptor !");

        // first clone this object's CommandDefinition part
        if (!m_xCommandDefinitions.is())
        {
            DBG_ERROR("OQueryContainer::appendByDescriptor : have no CommandDefinition container anymore !");
                // perhaps somebody modified the DataSource, so all connections were separated
            throwGenericSQLException(::rtl::OUString::createFromAscii("Unable to insert objects into containers of standalone connections (not belonging to a data source)."), *this);
                // TODO : resource
        }

        Reference< XPropertySet > xCommandDefinitionPart(m_xORB->createInstance(
            SERVICE_SDB_QUERYDEFINITION), UNO_QUERY);
        if (!xCommandDefinitionPart.is())
        {
            DBG_ERROR("OQueryContainer::appendByDescriptor : could not create a CommandDefinition object !");
            throwGenericSQLException(::rtl::OUString::createFromAscii("Unable to create an object supporting the com.sun.star.sdb.CommandDefinition service"), *this);
                // TODO : resource
        }

        ::comphelper::copyProperties(_rxDesc, xCommandDefinitionPart);
        // and insert it into the CommDef container
        ::rtl::OUString sNewObjectName;
        _rxDesc->getPropertyValue(PROPERTY_NAME) >>= sNewObjectName;
        {
            m_eDoingCurrently = INSERTING;
            OAutoActionReset aAutoReset(this);
            m_xCommandDefinitions->insertByName(sNewObjectName, makeAny(xCommandDefinitionPart));
        }

#if DBG_UTIL
        // check if the object was really inserted
        try
        {
            Reference< XPropertySet > xNewEl;
            ::cppu::extractInterface(xNewEl, m_xCommandDefinitions->getByName(sNewObjectName));
            DBG_ASSERT(xNewEl.get() == xCommandDefinitionPart.get(), "OQueryContainer::appendByDescriptor : the CommandDefinition container worked as it had a descriptor !");
                // normally should not have changed after inserting
        }
        catch(Exception&)
        {
            DBG_ERROR("OQueryContainer::appendByDescriptor : could not find the just inserted CommandDefinition !");
        }
#endif
        // TODO : the columns part of the descriptor has to be copied
        OQuery* pNewObject = m_aContainerListeners.getLength() ? implCreateWrapper(xCommandDefinitionPart) : NULL;
            // need this new object only if we have listeners, else it will be created on request
        m_aQueriesIndexed.push_back(m_aQueries.insert(Queries::value_type(sNewObjectName, pNewObject)).first);
        xNewObject = m_aQueriesIndexed[m_aQueriesIndexed.size() - 1]->second;
    }

    // notify our listeners
    ContainerEvent aEvent(*this, makeAny(sNewObjectName), makeAny(xNewObject), Any());
    OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
    while (aListenerIterator.hasMoreElements())
        static_cast< XContainerListener* >(aListenerIterator.next())->elementInserted(aEvent);
}

// XDrop
//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::dropByName( const ::rtl::OUString& _rName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    sal_Int32 nIndex = implGetIndex(_rName);
    if (-1 == nIndex)
        throw NoSuchElementException(_rName, *this);

    dropByIndex(nIndex);
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::dropByIndex( sal_Int32 _nIndex ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if ((_nIndex<0) || (_nIndex>getCount()))
        throw IndexOutOfBoundsException();

    if (!m_xCommandDefinitions.is())
    {
        DBG_ERROR("OQueryContainer::dropByIndex : have no CommandDefinition container anymore !");
            // perhaps somebody modified the DataSource, so all connections were separated
        throwGenericSQLException(::rtl::OUString::createFromAscii("Unable to remove objects from containers of standalone connections (not belonging to a data source)."), *this);
            // TODO : resource
    }

    // now simply forward the remove request to the CommandDefinition container, we're a listener for the removal
    // and thus we do everything neccessary in ::elementRemoved
    m_xCommandDefinitions->removeByName(m_aQueriesIndexed[_nIndex]->first);
}

// ::com::sun::star::container::XElementAccess
//------------------------------------------------------------------------------
Type OQueryContainer::getElementType(  ) throw(RuntimeException)
{
    return::getCppuType(static_cast<Reference<XPropertySet>*>(NULL));
}


//------------------------------------------------------------------------------
sal_Bool OQueryContainer::hasElements(void) throw( RuntimeException )
{
    return getCount() != 0;
}

// ::com::sun::star::container::XEnumerationAccess
//------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XEnumeration >  OQueryContainer::createEnumeration(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_rMutex);
    return new OEnumerationByIndex( static_cast< ::com::sun::star::container::XIndexAccess* >(this));
}

// ::com::sun::star::container::XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 OQueryContainer::getCount(void) throw( RuntimeException )
{
    return m_aQueriesIndexed.size();
}

//------------------------------------------------------------------------------
Any OQueryContainer::getByIndex(sal_Int32 _nIndex) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    if ((_nIndex < 0) || (_nIndex > m_aQueriesIndexed.size()))
        throw IndexOutOfBoundsException();

    Reference< XPropertySet > xReturn = m_aQueriesIndexed[_nIndex]->second;
    if (!xReturn.is())
    {   // the object was never accessed before -> create and store it
        xReturn = m_aQueriesIndexed[_nIndex]->second = implCreateWrapper(m_aQueriesIndexed[_nIndex]->first);
    }
    return makeAny(xReturn);
}

// ::com::sun::star::container::XNameAccess
//------------------------------------------------------------------------------
Any OQueryContainer::getByName(const rtl::OUString& _rName) throw( NoSuchElementException, WrappedTargetException, RuntimeException )
{
    QueriesIterator aPos = m_aQueries.find(_rName);
    if (aPos == m_aQueries.end())
        throw NoSuchElementException();

    Reference< XPropertySet > xReturn = aPos->second;
    if (!xReturn.is())
    {   // the object was never accessed before -> create and store it
        aPos->second = implCreateWrapper(aPos->first);
        xReturn = aPos->second;
    }
    return makeAny(xReturn);
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OQueryContainer::hasByName( const ::rtl::OUString& _rName ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_aQueries.find(_rName) != m_aQueries.end();
}

//------------------------------------------------------------------------------
Sequence< rtl::OUString > OQueryContainer::getElementNames(void) throw( RuntimeException )
{
    Sequence< rtl::OUString > aReturn(m_aQueries.size());
    ::rtl::OUString* pReturn = aReturn.getArray();

    for (   ConstQueriesIterator aLoop = m_aQueries.begin();
            aLoop != m_aQueries.end();
            ++aLoop, ++pReturn
        )
    {
        *pReturn = aLoop->first;
    }

    return aReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XPropertySet > xNewElement;
    sal_Int32 nAccessor = -1;
    {
        MutexGuard aGuard(m_rMutex);

        if (INSERTING == m_eDoingCurrently)
            // nothing to do, we're inserting via an "appendByDescriptor"
            return;

        // get the object
        Reference< XPropertySet > xProps;
        extractInterface(xProps, _rEvent.Element);
        ::rtl::OUString sElementName;
        xProps->getPropertyValue(PROPERTY_NAME) >>= sElementName;
        DBG_ASSERT(sElementName.getLength(), "OQueryContainer::elementInserted : invalid name !");
        DBG_ASSERT(!hasByName(sElementName), "OQueryContainer::elementInserted : oops .... we're inconsistent with our master container !");
        if (!sElementName.getLength() || hasByName(sElementName))
            return;

        // insert an own new element
        m_aQueriesIndexed.push_back(m_aQueries.insert(Queries::value_type(sElementName, implCreateWrapper(sElementName))).first);

        // notify our own listeners
        nAccessor = m_aQueriesIndexed.size() - 1;
        xNewElement = m_aQueriesIndexed[nAccessor]->second;
    }

    ContainerEvent aEvent(*this, makeAny((sal_Int32)nAccessor), makeAny(xNewElement), Any());
    OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
    while (aListenerIterator.hasMoreElements())
        static_cast< XContainerListener* >(aListenerIterator.next())->elementInserted(aEvent);
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::elementRemoved( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XPropertySet > xRemovedElement;
    ::rtl::OUString sAccessor;
    OQuery* pRemoved = NULL;
    {
        MutexGuard aGuard(m_rMutex);

        // get the object
        Reference< XPropertySet > xProps;
        extractInterface(xProps, _rEvent.Element);
        _rEvent.Accessor >>= sAccessor;

        DBG_ASSERT(sAccessor.getLength(), "OQueryContainer::elementRemoved : invalid name !");
        DBG_ASSERT(hasByName(sAccessor), "OQueryContainer::elementRemoved : oops .... we're inconsistent with our master container !");
        if (!sAccessor.getLength() || !hasByName(sAccessor))
            return;

        // the index within my structures
        sal_Int32 nMyIndex = implGetIndex(sAccessor);
        QueriesIterator aMapPos = m_aQueriesIndexed[nMyIndex];

        if (!aMapPos->second && m_aContainerListeners.getLength())
            // need this object for notifications
            aMapPos->second = implCreateWrapper(xProps);

        pRemoved = aMapPos->second;
        xRemovedElement = pRemoved;

        // remove all my refs
        m_aQueriesIndexed.erase(m_aQueriesIndexed.begin() + nMyIndex);
        m_aQueries.erase(aMapPos);
    }

    // notify our own listeners
    ContainerEvent aEvent(*this, makeAny(sAccessor), makeAny(xRemovedElement), Any());
    OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
    while (aListenerIterator.hasMoreElements())
        static_cast< XContainerListener* >(aListenerIterator.next())->elementRemoved(aEvent);

    if (pRemoved)
    {
        pRemoved->dispose();
        pRemoved->release();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XPropertySet > xReplacedElement;
    Reference< XPropertySet > xNewElement;
    OQuery* pReplaced = NULL;
    ::rtl::OUString sAccessor;
    {
        MutexGuard aGuard(m_rMutex);

        // get the object
        Reference< XPropertySet > xNewElementProps;
        extractInterface(xNewElementProps, _rEvent.Element);
        xNewElementProps->getPropertyValue(PROPERTY_NAME) >>= sAccessor;

        DBG_ASSERT(sAccessor.getLength(), "OQueryContainer::elementReplaced : invalid name !");
        DBG_ASSERT(hasByName(sAccessor), "OQueryContainer::elementReplaced : oops .... we're inconsistent with our master container !");
        if (!sAccessor.getLength() || !hasByName(sAccessor))
            return;

        // the index within my structures
        sal_Int32 nMyIndex = implGetIndex(sAccessor);
        QueriesIterator aMapPos = m_aQueriesIndexed[nMyIndex];

        pReplaced = aMapPos->second;
        xReplacedElement = pReplaced;
        if (!xReplacedElement.is() && m_aContainerListeners.getLength())
        {   // need this object for notifications
            pReplaced = implCreateWrapper(xNewElementProps);
                // don't use the version taking a string, it would ask the CommandDescription container ....
            xReplacedElement = pReplaced;
        }

        // remove all my refs to the replaced element's wrapper
        m_aQueriesIndexed.erase(m_aQueriesIndexed.begin() + nMyIndex);
        m_aQueries.erase(aMapPos);

        // insert an own new element
        m_aQueriesIndexed.push_back(m_aQueries.insert(Queries::value_type(sAccessor, implCreateWrapper(xNewElementProps))).first);

        // our own new element
        xNewElement = m_aQueriesIndexed[m_aQueriesIndexed.size() - 1]->second;
    }

    // notify our own listeners
    ContainerEvent aEvent(*this, makeAny(sAccessor), makeAny(xNewElement), makeAny(xReplacedElement));
    OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
    while (aListenerIterator.hasMoreElements())
        static_cast< XContainerListener* >(aListenerIterator.next())->elementReplaced(aEvent);

    if (pReplaced)
    {
        pReplaced->dispose();
        pReplaced->release();
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw(::com::sun::star::uno::RuntimeException)
{
    if (_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinitions, UNO_QUERY).get())
    {   // our "master container" (with the command definitions) is beeing disposed
        DBG_ERROR("OQueryContainer::disposing : nobody should dispose the CommandDefinition container before disposing my connection !");
        dispose();
    }
    else
    {
        QueriesIndexAccessIterator aIter = m_aQueriesIndexed.begin();
        for(;aIter != m_aQueriesIndexed.end();++aIter)
        {
            if(Reference<XInterface>(static_cast< ::cppu::OWeakObject*>((*aIter)->second),UNO_QUERY) == _rSource.Source)
            {
                m_xCommandDefinitions->removeByName((*aIter)->first);
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OQueryContainer::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL OQueryContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

//--------------------------------------------------------------------------
void OQueryContainer::flush_NoBroadcast_NoCommit()
{
    MutexGuard aGuard(m_rMutex);

    DBG_ASSERT(m_aConfigurationNode.isValid(), "ODefinitionContainer::flush : need a starting point within the configuration !");

    OConfigurationNode aQueryNode;
    OConfigurationTreeRoot aQueryRootNode;

    for (   ConstQueriesIterator aLoop = m_aQueries.begin();
            aLoop != m_aQueries.begin();
            ++aLoop
        )
    {
        if (aLoop->second)
        {
            aQueryNode = implGetObjectKey(aLoop->first, sal_True);
            aQueryRootNode = aQueryNode.cloneAsRoot();
            aLoop->second->storeTo(aQueryRootNode);
            aQueryRootNode.commit();
        }
    }
}

//--------------------------------------------------------------------------
OQuery* OQueryContainer::implCreateWrapper(const ::rtl::OUString& _rName)
{
    Reference< XPropertySet > xObject;
    extractInterface(xObject, m_xCommandDefinitions->getByName(_rName));
    return implCreateWrapper(xObject);
}

//--------------------------------------------------------------------------
OQuery* OQueryContainer::implCreateWrapper(const Reference< XPropertySet >& _rxCommandDesc)
{
    OQuery* pNewObject = new OQuery(_rxCommandDesc, Reference< XConnection >((Reference< XInterface >)m_rParent, UNO_QUERY));
    pNewObject->acquire();

    ::rtl::OUString sName;
    pNewObject->getPropertyValue(PROPERTY_NAME) >>= sName;
    pNewObject->initializeFrom(implGetObjectKey(sName, sal_True));

    return pNewObject;
}

//--------------------------------------------------------------------------
OConfigurationNode OQueryContainer::implGetObjectKey(const ::rtl::OUString& _rName, sal_Bool _bCreate)
{
    if (m_aConfigurationNode.hasByName(_rName))
        return m_aConfigurationNode.openNode(_rName);

    if (_bCreate)
    {
        // the configuration does not support different types of operations in one transaction, so we must commit
        // before and after we create the new node, to ensure, that every transaction we ever do contains only
        // one type of operation (insert, remove, update)
        OSL_VERIFY(m_aConfigurationNode.commit());
        OConfigurationNode aNode = m_aConfigurationNode.createNode(_rName);
        OSL_VERIFY(m_aConfigurationNode.commit());
        return aNode;
    }

    return OConfigurationNode();
}

//--------------------------------------------------------------------------
sal_Int32 OQueryContainer::implGetIndex(const ::rtl::OUString& _rName)
{
    for (   ConstQueriesIndexAccessIterator aSearch = m_aQueriesIndexed.begin();
            aSearch < m_aQueriesIndexed.end();
            ++aSearch
        )
        if ((*aSearch)->first.equals(_rName))
            return aSearch - m_aQueriesIndexed.begin();


    return -1;
}

//........................................................................
}   // namespace dbaccess
//........................................................................

