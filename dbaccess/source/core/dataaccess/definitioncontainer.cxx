/*************************************************************************
 *
 *  $RCSfile: definitioncontainer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:40 $
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
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _UNOTOOLS_ENUMHELPER_HXX_
#include <unotools/enumhelper.hxx>
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
using namespace ::utl;
using namespace ::cppu;
using namespace dbaccess;

//==========================================================================
//= ODefinitionContainer
//==========================================================================
DBG_NAME(ODefinitionContainer)
//--------------------------------------------------------------------------
ODefinitionContainer::ODefinitionContainer(OWeakObject& _rParent, Mutex& _rMutex)
    :m_aContainerListeners(_rMutex)
    ,m_rParent(_rParent)
    ,m_rMutex(_rMutex)
    ,m_bInitialized(sal_False)
{
    DBG_CTOR(ODefinitionContainer, NULL);
}

//--------------------------------------------------------------------------
void ODefinitionContainer::initialize(const Reference< XRegistryKey >& _rxConfigurationRoot)
{
    MutexGuard aGuard(m_rMutex);
    m_xConfigurationNode = _rxConfigurationRoot;

    DBG_ASSERT(m_xConfigurationNode.is(), "ODefinitionContainer::initialize : need a starting point within the configuration !");
    DBG_ASSERT(!m_bInitialized, "ODefinitionContainer::initialize : already initialized !");

    if (m_xConfigurationNode.is())
        initializeFromConfiguration();

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
    m_aDocumentDescKeys.clear();
    m_xConfigurationNode = NULL;

    m_bInitialized = sal_False;
}

//--------------------------------------------------------------------------
void ODefinitionContainer::flush()
{
    MutexGuard aGuard(m_rMutex);

    DBG_ASSERT(m_xConfigurationNode.is(), "ODefinitionContainer::flush : need a starting point within the configuration !");
    DBG_ASSERT(m_bInitialized, "ODefinitionContainer::flush : not initialized !");

    for (   ConstDocumentsIterator aLoop = m_aDocuments.begin();
            aLoop != m_aDocuments.begin();
            ++aLoop
        )
    {
        if (aLoop->xObject.is())
        {
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

    Reference< XRegistryKey > xDescriptionKey, xObjectKey;
    xObjectKey = createConfigKey(_rName, xDescriptionKey);
    if (!xObjectKey.is())
    {   // something went wrong
        DBG_ERROR("ODefinitionContainer::insertByName : could not create the new configuration nodes !");
        throw RuntimeException(::rtl::OUString(), *this);
    }

    implAppend(_rName, xNewElement, xObjectKey, xDescriptionKey);

    pNewElement->inserted(static_cast<OWeakObject*>(this), _rName, xObjectKey);
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
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_True);

    // check the arguments
    if (!_rName.getLength())
        throw IllegalArgumentException();

    if (!checkExistence(_rName))
        throw NoSuchElementException();

    // the old element (for the notifications)
    Reference< XPropertySet > xOldElement = implGetByName(_rName, (m_aContainerListeners.getLength() != 0));
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
    MutexGuard aGuard(m_rMutex);
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
    Reference< XRegistryKey > xObjectDescription;
    implReplace(_rName, xNewElement, xObjectDescription);

    // tell the old element it has been removed
    OContainerElement* pOldElement = NULL;
    if (getImplementation(pOldElement, Reference< XInterface >(xOldElement)))
        pOldElement->removed();

    // and dispose it
    disposeComponent(xOldElement);

    // tell the new element it's new name/location and flush it
    pNewElement->inserted(static_cast<OWeakObject*>(this), _rName, xObjectDescription);
    xNewFlushable->flush();

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
    return new ::utl::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
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
    if (!m_xConfigurationNode.is())
    {
        DBG_ERROR("ODefinitionContainer::initializeFromConfiguration : invalid configuration key !");
        return;
    }

    ORegistryLevelEnumeration aEnum(m_xConfigurationNode);
    while (aEnum.hasMoreElements())
    {
        Reference< XRegistryKey > xCurrentDescription = aEnum.nextElement();
        if (xCurrentDescription.is())
        {
            // get the name of the object
            ::rtl::OUString sObjectName;
            readValue(xCurrentDescription, CONFIGKEY_CONTAINERLEMENT_TITLE, sObjectName);
            // get the key under which the object is stored
            Reference< XRegistryKey > xObjectKey;
            try { xObjectKey = xCurrentDescription->openKey(CONFIGKEY_CONTAINERLEMENT_OBJECT); }
            catch(InvalidRegistryException&) { }

            if ((sObjectName.getLength() == 0) || !xObjectKey.is())
            {
                DBG_ERROR("ODefinitionContainer::initializeFromConfiguration : invalid structure within the configuration !");
                continue;
            }

            Reference< XPropertySet > xNewObject;
            // we don't read this new object here, this may be much to expensive. We read it only on request.

            m_aDocuments.push_back(Document(sObjectName, xNewObject));
            m_aDocumentMap[sObjectName] = xNewObject;
            m_aDocumentDescKeys[sObjectName] = xCurrentDescription;
            m_aDocumentObjectKeys[sObjectName] = xObjectKey;
        }
    }
}

//--------------------------------------------------------------------------
sal_Bool ODefinitionContainer::isReadOnly() const
{
    try
    {
        return m_xConfigurationNode.is() && m_xConfigurationNode->isReadOnly();
    }
    catch (InvalidRegistryException&)
    {
    }
    DBG_ERROR("ODefinitionContainer::isReadOnly : never should have reached this point !");
    // if we have an invalid registry - assume the worst
    return sal_True;
}

//--------------------------------------------------------------------------
Reference< XRegistryKey > ODefinitionContainer::createConfigKey(const ::rtl::OUString& _rName, Reference< XRegistryKey >& _rxDescKey)
{
    ::rtl::OUString sDescriptionKey = getUniqueKeyName(m_xConfigurationNode, ::rtl::OUString::createFromAscii("obj_"));
    Reference< XRegistryKey > xObjectKey;
    try
    {
        _rxDescKey = m_xConfigurationNode->createKey(sDescriptionKey);
        if (_rxDescKey.is())
            if (writeValue(_rxDescKey, CONFIGKEY_CONTAINERLEMENT_TITLE, _rName))
                xObjectKey = _rxDescKey->createKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
    }
    catch (InvalidRegistryException&)
    {
    }

    return xObjectKey;
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

    // and from the configuration
    ::rtl::OUString sDescriptionKeyName;
    try
    {
        sDescriptionKeyName = getShortKeyName(m_aDocumentDescKeys[_rName]);
    }
    catch (InvalidRegistryException&)
    {
        DBG_ERROR("ODefinitionContainer::removeByName : could not get the description key name !");
    }
    // (release our own configuration node references before deleting the node, perhaps it's required for
    // deleting the node ...)
    m_aDocumentDescKeys.erase(_rName);
    m_aDocumentObjectKeys.erase(_rName);
    try
    {
        m_xConfigurationNode->deleteKey(sDescriptionKeyName);
    }
    catch (InvalidRegistryException&)
    {
        DBG_ERROR("ODefinitionContainer::removeByName : could not delete the description key !");
    }
}

//--------------------------------------------------------------------------
void ODefinitionContainer::implAppend(const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxNewObject, const Reference< XRegistryKey >& _rxObjectNode, const Reference< XRegistryKey >& _rxDescNode)
{
    MutexGuard aGuard(m_rMutex);
    try
    {
        // now update our structures
        m_aDocuments.push_back(Document(_rName, _rxNewObject));
        m_aDocumentMap[_rName] = _rxNewObject;
        m_aDocumentDescKeys[_rName] = _rxDescNode;
        m_aDocumentObjectKeys[_rName] = _rxObjectNode;

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
void ODefinitionContainer::implReplace(const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxNewObject, Reference< XRegistryKey >& _rxNewObjectNode)
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

    _rxNewObjectNode.clear();

    // update the configuration
    Reference< XRegistryKey > xObjectDescription = m_aDocumentDescKeys[_rName];
    // delete the old object key
    m_aDocumentObjectKeys[_rName] = Reference< XRegistryKey >();
    try
    {
        xObjectDescription->deleteKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
            // this ensures that all sub keys will be deleted, too
        _rxNewObjectNode = m_aDocumentObjectKeys[_rName] = xObjectDescription->createKey(CONFIGKEY_CONTAINERLEMENT_OBJECT);
    }
    catch(InvalidRegistryException&)
    {
    }
    DBG_ASSERT(_rxNewObjectNode.is(), "ODefinitionContainer::implReplace : could not generate the new key !");
}

//--------------------------------------------------------------------------
void ODefinitionContainer::checkValid(sal_Bool _bIntendWriteAccess) const throw (RuntimeException, DisposedException)
{
    if (!m_bInitialized)
        throw RuntimeException();

    if (_bIntendWriteAccess && isReadOnly())
        RuntimeException(
            ::rtl::OUString::createFromAscii("You have no write access to the configuration tree the object is based on."),
                // TODO : put this into a resource
            Reference< XInterface >(const_cast<XServiceInfo*>(static_cast<const XServiceInfo*>(this))));

    DBG_ASSERT( (m_aDocuments.size() == m_aDocumentDescKeys.size()) &&
                (m_aDocuments.size() == m_aDocumentObjectKeys.size()) &&
                (m_aDocuments.size() == m_aDocumentMap.size()),
        "ODefinitionContainer::checkValid : inconsistent state !");
}

//--------------------------------------------------------------------------
sal_Bool ODefinitionContainer::approveNewObject(const Reference< XPropertySet >& _rxObject) const
{
    return _rxObject.is();
}

