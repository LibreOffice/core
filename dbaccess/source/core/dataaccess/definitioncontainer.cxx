/*************************************************************************
 *
 *  $RCSfile: definitioncontainer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-18 16:15:16 $
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

#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#include "definitioncontainer.hxx"
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _DBA_CORE_CONTAINERELEMENT_HXX_
#include "containerelement.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace dbaccess;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= ODefinitionContainer
//==========================================================================
DBG_NAME(ODefinitionContainer)
//--------------------------------------------------------------------------
ODefinitionContainer::ODefinitionContainer(OWeakObject& _rParent, Mutex& _rMutex)
    :OConfigurationFlushable(_rMutex)
    ,m_aContainerListeners(_rMutex)
    ,m_rParent(_rParent)
    ,m_bInitialized(sal_False)
{
    DBG_CTOR(ODefinitionContainer, NULL);
}

//--------------------------------------------------------------------------
void ODefinitionContainer::initialize(const OConfigurationTreeRoot& _rConfigurationRoot, sal_Bool _bRead)
{
    MutexGuard aGuard(m_rMutex);
    m_aConfigurationNode = _rConfigurationRoot;

    DBG_ASSERT(m_aConfigurationNode.isValid(), "ODefinitionContainer::initialize : need a starting point within the configuration !");
    DBG_ASSERT(!m_bInitialized, "ODefinitionContainer::initialize : already initialized !");

    if (m_aConfigurationNode.isValid())
    {
        DBG_ASSERT(m_aConfigurationNode.isSetNode(), "ODefinitionContainer::initialize: our config node should be a set node!");
        m_aConfigurationNode.setEscape(sal_True);
        initializeFromConfiguration();
    }

    m_bInitialized = sal_True;
}

//--------------------------------------------------------------------------
void ODefinitionContainer::dispose()
{
    MutexGuard aGuard(m_rMutex);
    DBG_ASSERT(m_bInitialized, "ODefinitionContainer::dispose : not initialized !");

    // say our listeners goobye
    EventObject aEvt(*this);
    m_aContainerListeners.disposeAndClear(aEvt);

    // dispose our elements
    for (   ConstDocumentsIterator aLoop = m_aDocuments.begin();
            aLoop != m_aDocuments.begin();
            ++aLoop
        )
    {
        Reference< XComponent > xComp(aLoop->xObject, UNO_QUERY);
        if (xComp.is())
        {
            xComp->removeEventListener(this);
            xComp->dispose();
        }
    }

    // remove our elements
    m_aDocumentMap.clear();
    m_aDocuments.clear();
    m_aDocumentObjectKeys.clear();
    m_aConfigurationNode.clear();

    m_bInitialized = sal_False;
}

//--------------------------------------------------------------------------
void ODefinitionContainer::flush_NoBroadcast_NoCommit()
{
    DBG_ASSERT(m_aConfigurationNode.isValid(), "ODefinitionContainer::flush : need a starting point within the configuration !");
    DBG_ASSERT(m_bInitialized, "ODefinitionContainer::flush : not initialized !");

    for (   ConstDocumentsIterator aLoop = m_aDocuments.begin();
            aLoop != m_aDocuments.begin();
            ++aLoop
        )
    {
        if (aLoop->xObject.is())
        {
            // TODO: perhaps we should use the flush_NoBroadcast_NoCommit of the implementations of the objects
            // the method here is called flush_NoBroadcast_NoCommit, which is contradicted by a direct flush call ...
            Reference< XFlushable > xFlush(aLoop->xObject, UNO_QUERY);
            DBG_ASSERT(xFlush.is(), "ODefinitionContainer::flush : have a living object which is not flushable !");
            if (xFlush.is())
                try
                {
                    xFlush->flush();
                }
                catch(Exception&)
                {
                    DBG_ERROR("ODefinitionContainer::flush : could not write one of my objects !");
                }
        }
    }
}

//--------------------------------------------------------------------------
ODefinitionContainer::~ODefinitionContainer()
{
    DBG_DTOR(ODefinitionContainer, NULL);
}

// XServiceInfo
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODefinitionContainer::getImplementationName(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdb.ODefinitionContainer");
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODefinitionContainer::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODefinitionContainer::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aReturn(1);
    aReturn.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DefinitionContainer");
    return aReturn;
}

// XNameContainer
//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::insertByName( const ::rtl::OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_True);

    if (checkExistence(_rName))
        throw ElementExistException();

    // approve the new object
    Reference< XPropertySet > xNewElement;
    ::cppu::extractInterface(xNewElement, aElement);
    if (!approveNewObject(xNewElement))
        throw IllegalArgumentException();

    // get the implementation which allows us to set the new name and the new configuration location
    // TODO : think about doing all this without implementation access, e.g. by calling setParent on
    // a XChild and storeAs on an XStorable
    // or maybe we should allow such objects : they're not persistent, then, and know nothing about their name
    OContainerElement* pNewElement = NULL;
    if (!getImplementation(pNewElement, Reference< XInterface >(xNewElement)))
        throw IllegalArgumentException();

    // for storing the new element
    Reference< XFlushable > xNewFlushable(xNewElement, UNO_QUERY);
    if (!xNewFlushable.is())
        throw IllegalArgumentException();

    // the configuration does not support different types of operations in one transaction, so we must commit
    // before and after we create the new node, to ensure, that every transaction we ever do contains only
    // one type of operation (insert, remove, update)
    OSL_VERIFY(m_aConfigurationNode.commit());
    OConfigurationNode aObjectNode = m_aConfigurationNode.createNode(_rName);
    OSL_VERIFY(m_aConfigurationNode.commit());

    if (!aObjectNode.isValid())
    {   // something went wrong
        DBG_ERROR("ODefinitionContainer::insertByName : could not create the new configuration nodes !");
        throw RuntimeException(::rtl::OUString(), *this);
    }

    implAppend(_rName, xNewElement, aObjectNode);

    pNewElement->inserted(static_cast<OWeakObject*>(this), _rName, aObjectNode.cloneAsRoot());
    xNewFlushable->flush();

    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(xNewElement), Any());
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementInserted(aEvent);
    }
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::removeByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Reference< XPropertySet > xOldElement;
    {
        MutexGuard aGuard(m_rMutex);
        checkValid(sal_True);

        // check the arguments
        if (!_rName.getLength())
            throw IllegalArgumentException();

        if (!checkExistence(_rName))
            throw NoSuchElementException();

        // the old element (for the notifications)
        xOldElement = implGetByName(_rName, (m_aContainerListeners.getLength() != 0));
            // as this is potentially expensive (if the object is not already created and initialized from the registry)
            // we load the element only if we have listeners which may be interested in

        // do the removal
        implRemove(_rName);

        OContainerElement* pOldElement = NULL;
        getImplementation(pOldElement, Reference< XInterface >(xOldElement));
        DBG_ASSERT(pOldElement, "ODefinitionContainer::removeByName : have an invalid object !");
        // we never should have inserted this when it's no OContainerElement
        if (pOldElement)
            pOldElement->removed();

        disposeComponent(xOldElement);
    }

    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(xOldElement), Any());
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementRemoved(aEvent);
    }
}

// XNameReplace
//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::replaceByName( const ::rtl::OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ClearableMutexGuard aGuard(m_rMutex);
    checkValid(sal_True);

    // check the arguments
    if (!_rName.getLength())
        throw IllegalArgumentException();

    // let derived classes approve the new object
    Reference< XPropertySet > xNewElement;
    ::cppu::extractInterface(xNewElement, aElement);
    if (!approveNewObject(xNewElement))
        throw IllegalArgumentException();

    // get the implementation so we can notify the new object about it's new name and locations
    // TODO : think about doing all this without implementation access, e.g. by calling setParent on
    // a XChild and storeAs on an XStorable
    // or maybe we should allow such objects : they're not persistent, then, and know nothing about their name
    OContainerElement* pNewElement = NULL;
    if (!getImplementation(pNewElement, Reference< XInterface >(xNewElement)))
        throw IllegalArgumentException();

    // for storing the new element
    Reference< XFlushable > xNewFlushable(xNewElement, UNO_QUERY);
    if (!xNewFlushable.is())
        throw IllegalArgumentException();

    // the old element (for the notifications)
    Reference< XPropertySet > xOldElement = implGetByName(_rName, m_aContainerListeners.getLength() != 0);
        // as this is potentially expensive (if the object is not already created and initialized from the registry)
        // we get the element only if we have listeners which may be interested in

    // do the replace
    OConfigurationNode aObjectDescription;
    implReplace(_rName, xNewElement, aObjectDescription);

    // tell the old element it has been removed
    OContainerElement* pOldElement = NULL;
    if (getImplementation(pOldElement, Reference< XInterface >(xOldElement)))
        pOldElement->removed();

    // and dispose it
    disposeComponent(xOldElement);

    // tell the new element it's new name/location and flush it
    pNewElement->inserted(static_cast<OWeakObject*>(this), _rName, aObjectDescription.cloneAsRoot());
    xNewFlushable->flush();

    aGuard.clear();
    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(xNewElement), makeAny(xOldElement));
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementReplaced(aEvent);
    }
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

// XElementAccess
//--------------------------------------------------------------------------
Type SAL_CALL ODefinitionContainer::getElementType( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return ::getCppuType( static_cast< Reference< XPropertySet >* >(NULL) );
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODefinitionContainer::hasElements( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return m_aDocuments.size() != 0;
}

// XEnumerationAccess
//--------------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL ODefinitionContainer::createEnumeration(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

//--------------------------------------------------------------------------
// XIndexAccess
sal_Int32 SAL_CALL ODefinitionContainer::getCount(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return m_aDocuments.size();
}

//--------------------------------------------------------------------------
Any SAL_CALL ODefinitionContainer::getByIndex( sal_Int32 _nIndex ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    if ((_nIndex < 0) || (_nIndex >= m_aDocuments.size()))
        throw IndexOutOfBoundsException();

    DocumentsIterator aPos = m_aDocuments.begin() + _nIndex;
    if (!aPos->xObject.is())
    {   // that's the first access to the object
        // -> create it
        aPos->xObject = createObject(aPos->sName, m_aDocumentObjectKeys[aPos->sName]);
        // and update the name-access map
        m_aDocumentMap[aPos->sName] = aPos->xObject;

#if DBG_UTIL
        Reference< XFlushable > xFlushable(aPos->xObject, UNO_QUERY);
        DBG_ASSERT(xFlushable.is(), "ODefinitionContainer::getByIndex : createObject returned an invalid object (no XFlushable) !");
#endif
    }

    return makeAny(aPos->xObject);
}

//--------------------------------------------------------------------------
Any SAL_CALL ODefinitionContainer::getByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    return makeAny(implGetByName(_rName));
}

//--------------------------------------------------------------------------
Reference< XPropertySet > ODefinitionContainer::implGetByName(const ::rtl::OUString& _rName, sal_Bool _bReadIfNeccessary) throw (NoSuchElementException)
{
    PropertySetMapIterator aMapPos = m_aDocumentMap.find(_rName);
    if (aMapPos == m_aDocumentMap.end())
        throw NoSuchElementException();

    if (_bReadIfNeccessary && !aMapPos->second.is())
    {   // the object has never been accessed before, so we have to read it now
        // (that's the expensive part)

        // create the object and insert it into the map
        aMapPos->second = createObject(_rName, m_aDocumentObjectKeys[_rName]);

        // update the vector for the index access
        for (   DocumentsIterator aSearch = m_aDocuments.begin();
                aSearch != m_aDocuments.end();
                ++aSearch
                )
        {
            if (aSearch->sName.equals(_rName))
            {
                aSearch->xObject = aMapPos->second;
                Reference< XComponent > xComp(aSearch->xObject, UNO_QUERY);
                if (xComp.is())
                    xComp->addEventListener(this);
                break;
            }
        }
    }

    return aMapPos->second;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODefinitionContainer::getElementNames(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    Sequence< ::rtl::OUString > aNames(m_aDocumentMap.size());
    ::rtl::OUString* pNames = aNames.getArray();
    ;
    for (   ConstPropertySetMapIterator aNameIter = m_aDocumentMap.begin();
            aNameIter != m_aDocumentMap.end();
            ++pNames, ++aNameIter
        )
    {
        *pNames = aNameIter->first;
    }

    return aNames;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODefinitionContainer::hasByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    return (m_aDocumentMap.find(_rName) != m_aDocumentMap.end());
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::disposing( const EventObject& _rSource ) throw(RuntimeException)
{
    Reference< XPropertySet > xSource(_rSource.Source, UNO_QUERY);
    // it's one of our documents ....
    for (   DocumentsIterator aLoop = m_aDocuments.begin();
            aLoop != m_aDocuments.begin();
            ++aLoop
        )
    {
        if (xSource.get() == aLoop->xObject.get())
        {
            Reference< XComponent > xComp(aLoop->xObject, UNO_QUERY);
            DBG_ASSERT(xComp.is(), "ODefinitionContainer::disposing : a 'disposing' call from a non-XCompoent object ?");
            // stop all listening
            xComp->removeEventListener(this);
            // and clear our document map/vector, so the object will be recreated on next access
            aLoop->xObject.clear();
            m_aDocumentMap[aLoop->sName].clear();
        }
    }
}

//--------------------------------------------------------------------------
void ODefinitionContainer::initializeFromConfiguration()
{
    if (!m_aConfigurationNode.isValid())
    {
        DBG_ERROR("ODefinitionContainer::initializeFromConfiguration : invalid configuration key !");
        return;
    }

    Sequence< ::rtl::OUString > aDefinitionNames = m_aConfigurationNode.getNodeNames();
    const ::rtl::OUString* pDefinitionNames = aDefinitionNames.getConstArray();
    for (sal_Int32 i=0; i<aDefinitionNames.getLength(); ++i, ++pDefinitionNames)
    {
        // get the node under which the object is stored
        OConfigurationNode aObjectNode = m_aConfigurationNode.openNode(*pDefinitionNames);

        if ((0 == pDefinitionNames->getLength()) || !aObjectNode.isValid())
        {
            DBG_ERROR("ODefinitionContainer::initializeFromConfiguration : invalid structure within the configuration !");
            continue;
        }

        Reference< XPropertySet > xNewObject;
        // we don't read this new object here, this may be much to expensive. We read it only on request.

        m_aDocuments.push_back(Document(*pDefinitionNames, xNewObject));
        m_aDocumentMap[*pDefinitionNames] = xNewObject;
        m_aDocumentObjectKeys[*pDefinitionNames] = aObjectNode;
    }
}

//--------------------------------------------------------------------------
sal_Bool ODefinitionContainer::isReadOnly() const
{
    return !m_aConfigurationNode.isValid() || m_aConfigurationNode.isReadonly();
}

//--------------------------------------------------------------------------
void ODefinitionContainer::implRemove(const ::rtl::OUString& _rName)
{
    // from the object maps
    m_aDocumentMap.erase(_rName);
    for (   DocumentsIterator aSearch = m_aDocuments.begin();
            aSearch != m_aDocuments.end();
            ++aSearch
        )
    {
        if (aSearch->sName.equals(_rName))
        {
            m_aDocuments.erase(aSearch);
            break;
        }
    }

    m_aDocumentObjectKeys.erase(_rName);
    // the configuration does not support different types of operations in one transaction, so we must commit
    // before and after we create the new node, to ensure, that every transaction we ever do contains only
    // one type of operation (insert, remove, update)
    OSL_VERIFY(m_aConfigurationNode.commit());
    m_aConfigurationNode.removeNode(_rName);
    OSL_VERIFY(m_aConfigurationNode.commit());
}

//--------------------------------------------------------------------------
void ODefinitionContainer::implAppend(const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxNewObject, const OConfigurationNode& _rObjectNode)
{
    MutexGuard aGuard(m_rMutex);
    try
    {
        // now update our structures
        m_aDocuments.push_back(Document(_rName, _rxNewObject));
        m_aDocumentMap[_rName] = _rxNewObject;
        m_aDocumentObjectKeys[_rName] = _rObjectNode;

        Reference< XComponent > xComp(_rxNewObject, UNO_QUERY);
        if (xComp.is())
            xComp->addEventListener(this);
    }
    catch(Exception&)
    {
        DBG_ERROR("ODefinitionContainer::implAppend : catched something !");
    }
}

//--------------------------------------------------------------------------
void ODefinitionContainer::implReplace(const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxNewObject, OConfigurationNode& _rNewObjectNode)
{
    DBG_ASSERT(checkExistence(_rName), "ODefinitionContainer::implReplace : invalid name !");

    m_aDocumentMap[_rName] = _rxNewObject;
    for (   DocumentsIterator aSearch = m_aDocuments.begin();
            aSearch != m_aDocuments.end();
            ++aSearch
        )
    {
        if (aSearch->sName.equals(_rName))
        {
            Reference< XComponent > xComp(aSearch->xObject, UNO_QUERY);
            if (xComp.is())
                xComp->removeEventListener(this);

            aSearch->xObject = _rxNewObject;

            xComp = Reference< XComponent >(_rxNewObject, UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener(this);

            break;
        }
    }

    _rNewObjectNode.clear();
    // update the configuration
    // clear all old data
    // the configuration does not support different types of operations in one transaction, so we must commit
    // before and after we create the new node, to ensure, that every transaction we ever do contains only
    // one type of operation (insert, remove, update)
    OSL_VERIFY(m_aConfigurationNode.commit());
    m_aConfigurationNode.removeNode(_rName);
    OSL_VERIFY(m_aConfigurationNode.commit());

    // the configuration does not support different types of operations in one transaction, so we must commit
    // before and after we create the new node, to ensure, that every transaction we ever do contains only
    // one type of operation (insert, remove, update)
    OSL_VERIFY(m_aConfigurationNode.commit());
    _rNewObjectNode = m_aDocumentObjectKeys[_rName] = m_aConfigurationNode.createNode(_rName);
    OSL_VERIFY(m_aConfigurationNode.commit());
}

//--------------------------------------------------------------------------
void ODefinitionContainer::checkValid(sal_Bool _bIntendWriteAccess) const throw (RuntimeException, DisposedException)
{
    if (!m_bInitialized)
        throw DisposedException();

    if (_bIntendWriteAccess && isReadOnly())
        DisposedException(
            ::rtl::OUString::createFromAscii("You have no write access to the configuration tree the object is based on."),
                // TODO : put this into a resource
            Reference< XInterface >(const_cast<XServiceInfo*>(static_cast<const XServiceInfo*>(this))));

    DBG_ASSERT( (m_aDocuments.size() == m_aDocumentObjectKeys.size()) &&
                (m_aDocuments.size() == m_aDocumentMap.size()),
        "ODefinitionContainer::checkValid : inconsistent state !");
}

//--------------------------------------------------------------------------
sal_Bool ODefinitionContainer::approveNewObject(const Reference< XPropertySet >& _rxObject) const
{
    return _rxObject.is();
}

//........................................................................
}   // namespace dbaccess
//........................................................................
