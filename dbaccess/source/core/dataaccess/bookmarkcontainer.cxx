/*************************************************************************
 *
 *  $RCSfile: bookmarkcontainer.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-30 07:58:20 $
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

#ifndef _DBA_CORE_BOOKMARKCONTAINER_HXX_
#include "bookmarkcontainer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_CONTAINERELEMENT_HXX_
#include "containerelement.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::utl;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OBookmarkContainer
//==========================================================================
DBG_NAME(OBookmarkContainer)
//--------------------------------------------------------------------------
OBookmarkContainer::OBookmarkContainer(OWeakObject& _rParent, Mutex& _rMutex)
    :OConfigurationFlushable(_rMutex)
    ,m_aContainerListeners(_rMutex)
    ,m_rParent(_rParent)
    ,m_bInitialized(sal_False)
{
    DBG_CTOR(OBookmarkContainer, NULL);
}

//--------------------------------------------------------------------------
void OBookmarkContainer::initialize(const OConfigurationTreeRoot& _rConfigurationRoot, sal_Bool _bRead)
{
    MutexGuard aGuard(m_rMutex);
    m_aConfigurationNode = _rConfigurationRoot;

    DBG_ASSERT(m_aConfigurationNode.isValid(), "OBookmarkContainer::initialize : need a starting point within the configuration !");
    DBG_ASSERT(!m_bInitialized, "OBookmarkContainer::initialize : already initialized !");

    if (m_aConfigurationNode.isValid())
    {
        DBG_ASSERT(m_aConfigurationNode.isSetNode(), "OBookmarkContainer::initialize: our config node should be a set node!");
        m_aConfigurationNode.setEscape(sal_True);
        initializeFromConfiguration();
    }

    m_bInitialized = sal_True;
}

//--------------------------------------------------------------------------
void OBookmarkContainer::dispose()
{
    MutexGuard aGuard(m_rMutex);
    DBG_ASSERT(m_bInitialized, "OBookmarkContainer::dispose : not initialized !");

    // say our listeners goobye
    EventObject aEvt(*this);
    m_aContainerListeners.disposeAndClear(aEvt);

    // remove our elements
    m_aBookmarks.clear();
    m_aObjectKeys.clear();
    m_aConfigurationNode.clear();

    m_bInitialized = sal_False;
}

//--------------------------------------------------------------------------
Sequence< Type > SAL_CALL OBookmarkContainer::getTypes() throw (RuntimeException)
{
    Sequence< Type > aTypes = OBookmarkContainer_Base::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 1);
    aTypes[nLen] = ::getCppuType( static_cast< const Reference< XFlushable >* >( NULL ) );

    return aTypes;
}

//--------------------------------------------------------------------------
Any SAL_CALL OBookmarkContainer::queryInterface( const Type & _rType ) throw (RuntimeException)
{
    Any aReturn = OBookmarkContainer_Base::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OConfigurationFlushable::queryInterface(_rType);
    return aReturn;
}

//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::acquire(  ) throw(RuntimeException)
{
    m_rParent.acquire();
}

//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::release(  ) throw(RuntimeException)
{
    m_rParent.release();
}

//--------------------------------------------------------------------------
void OBookmarkContainer::flush_NoBroadcast_NoCommit()
{
    DBG_ASSERT(m_aConfigurationNode.isValid(), "OBookmarkContainer::flush_NoBroadcast_NoCommit: need a starting point within the configuration !");
    DBG_ASSERT(m_bInitialized, "OBookmarkContainer::flush_NoBroadcast_NoCommit: not initialized !");

    DBG_ASSERT(m_aBookmarks.size() == m_aObjectKeys.size(), "OBookmarkContainer::flush_NoBroadcast_NoCommit: inconsistence: this may crash!");
    ConstMapString2StringIterator aLinks = m_aBookmarks.begin();
    ConstConfigNodeMapIterator aNodes = m_aObjectKeys.begin();

    for (   ;
            aLinks != m_aBookmarks.begin();
            ++aLinks, ++aNodes
        )
    {
        aNodes->second.setNodeValue(CONFIGKEY_DBLINK_DOCUMENTLOCAITON, makeAny(aLinks->second));
    }
}

//--------------------------------------------------------------------------
OBookmarkContainer::~OBookmarkContainer()
{
    DBG_DTOR(OBookmarkContainer, NULL);
}

// XServiceInfo
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OBookmarkContainer::getImplementationName(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.dba.OBookmarkContainer");
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OBookmarkContainer::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OBookmarkContainer::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aReturn(1);
    aReturn.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DefinitionContainer");
    return aReturn;
}

// XNameContainer
//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::insertByName( const ::rtl::OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_True);

    if (checkExistence(_rName))
        throw ElementExistException();

    if (0 == _rName.getLength())
        throw IllegalArgumentException();

    // approve the new object
    ::rtl::OUString sNewLink;
    if (!(aElement >>= sNewLink))
        throw IllegalArgumentException();

    OConfigurationNode aObjectNode = m_aConfigurationNode.createNode(_rName);
    if (!aObjectNode.isValid())
    {   // something went (heavily) wrong
        DBG_ERROR("OBookmarkContainer::insertByName : could not create the new configuration nodes !");
        throw RuntimeException(::rtl::OUString(), *this);
    }

    aObjectNode.setNodeValue(CONFIGKEY_DBLINK_DOCUMENTLOCAITON, makeAny(sNewLink));
//  OSL_VERIFY(m_aConfigurationNode.commit());

    implAppend(_rName, sNewLink, aObjectNode);

    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(sNewLink), Any());
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementInserted(aEvent);
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::removeByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ::rtl::OUString sOldBookmark;
    {
        MutexGuard aGuard(m_rMutex);
        checkValid(sal_True);

        // check the arguments
        if (!_rName.getLength())
            throw IllegalArgumentException();

        if (!checkExistence(_rName))
            throw NoSuchElementException();

        // the old element (for the notifications)
        sOldBookmark = m_aBookmarks[_rName];

        // do the removal
        implRemove(_rName);
    }

    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(sOldBookmark), Any());
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementRemoved(aEvent);
    }
}

// XNameReplace
//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::replaceByName( const ::rtl::OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ClearableMutexGuard aGuard(m_rMutex);
    checkValid(sal_True);

    // check the arguments
    if (!_rName.getLength())
        throw IllegalArgumentException();

    // do we have such an element?
    if (!checkExistence(_rName))
        throw NoSuchElementException();

    // approve the new object
    ::rtl::OUString sNewLink;
    if (!(aElement >>= sNewLink))
        throw IllegalArgumentException();

    // the old element (for the notifications)
    ::rtl::OUString sOldLink = m_aBookmarks[_rName];

    // do the replace
    implReplace(_rName, sNewLink);

    // notify the listeners
    aGuard.clear();
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(sNewLink), makeAny(sOldLink));
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
            static_cast< XContainerListener* >(aListenerIterator.next())->elementReplaced(aEvent);
    }
}

//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

// XElementAccess
//--------------------------------------------------------------------------
Type SAL_CALL OBookmarkContainer::getElementType( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return ::getCppuType( static_cast< ::rtl::OUString* >(NULL) );
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OBookmarkContainer::hasElements( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return m_aBookmarks.size() != 0;
}

// XEnumerationAccess
//--------------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL OBookmarkContainer::createEnumeration(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

//--------------------------------------------------------------------------
// XIndexAccess
sal_Int32 SAL_CALL OBookmarkContainer::getCount(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);
    return m_aBookmarks.size();
}

//--------------------------------------------------------------------------
Any SAL_CALL OBookmarkContainer::getByIndex( sal_Int32 _nIndex ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    if ((_nIndex < 0) || (_nIndex >= (sal_Int32)m_aBookmarksIndexed.size()))
        throw IndexOutOfBoundsException();

    return makeAny(m_aBookmarksIndexed[_nIndex]->second);
}

//--------------------------------------------------------------------------
Any SAL_CALL OBookmarkContainer::getByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    if (!checkExistence(_rName))
        throw NoSuchElementException();

    return makeAny(m_aBookmarks[_rName]);
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OBookmarkContainer::getElementNames(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    Sequence< ::rtl::OUString > aNames(m_aBookmarks.size());
    ::rtl::OUString* pNames = aNames.getArray();
    ;
    for (   ConstMapIteratorVectorIterator aNameIter = m_aBookmarksIndexed.begin();
            aNameIter != m_aBookmarksIndexed.end();
            ++pNames, ++aNameIter
        )
    {
        *pNames = (*aNameIter)->first;
    }

    return aNames;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OBookmarkContainer::hasByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    checkValid(sal_False);

    return checkExistence(_rName);
}

//--------------------------------------------------------------------------
void OBookmarkContainer::initializeFromConfiguration()
{
    if (!m_aConfigurationNode.isValid())
    {
        DBG_ERROR("OBookmarkContainer::initializeFromConfiguration : invalid configuration key !");
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
            DBG_ERROR("OBookmarkContainer::initializeFromConfiguration : invalid structure within the configuration !");
            continue;
        }

        ::rtl::OUString sBookmark;
        aObjectNode.getNodeValue(CONFIGKEY_DBLINK_DOCUMENTLOCAITON) >>= sBookmark;
        implAppend(*pDefinitionNames, sBookmark, aObjectNode);
    }
}

//--------------------------------------------------------------------------
sal_Bool OBookmarkContainer::isReadOnly() const
{
    return !m_aConfigurationNode.isValid() || m_aConfigurationNode.isReadonly();
}

//--------------------------------------------------------------------------
void OBookmarkContainer::implRemove(const ::rtl::OUString& _rName)
{
    MutexGuard aGuard(m_rMutex);

    // look for the name in the index access vector
    MapString2StringIterator aMapPos = m_aBookmarks.end();
    for (   MapIteratorVectorIterator aSearch = m_aBookmarksIndexed.begin();
            aSearch != m_aBookmarksIndexed.end();
            ++aSearch
        )
    {
#ifdef DBG_UTIL
        ::rtl::OUString sName = (*aSearch)->first;
#endif
        if ((*aSearch)->first == _rName)
        {
            aMapPos = *aSearch;
            m_aBookmarksIndexed.erase(aSearch);
            break;
        }
    }

    if (m_aBookmarks.end() == aMapPos)
    {
        DBG_ERROR("OBookmarkContainer::implRemove: inconsistence!");
        return;
    }

    // remove the map entries
    m_aBookmarks.erase(aMapPos);
    m_aObjectKeys.erase(_rName);

    m_aConfigurationNode.removeNode(_rName);
//  OSL_VERIFY(m_aConfigurationNode.commit());
}

//--------------------------------------------------------------------------
void OBookmarkContainer::implAppend(const ::rtl::OUString& _rName, const ::rtl::OUString& _rDocumentLocation, const OConfigurationNode& _rObjectNode)
{
    MutexGuard aGuard(m_rMutex);

    m_aBookmarks[_rName] = _rDocumentLocation;
    m_aBookmarksIndexed.push_back(m_aBookmarks.find(_rName));
    m_aObjectKeys[_rName] = _rObjectNode;
}

//--------------------------------------------------------------------------
void OBookmarkContainer::implReplace(const ::rtl::OUString& _rName, const ::rtl::OUString& _rNewLink)
{
    MutexGuard aGuard(m_rMutex);
    DBG_ASSERT(checkExistence(_rName), "OBookmarkContainer::implReplace : invalid name !");

    m_aBookmarks[_rName] = _rNewLink;

    // update the configuration
    DBG_ASSERT(m_aObjectKeys.find(_rName) != m_aObjectKeys.end(), "OBookmarkContainer::implReplace: missing the config node!");
    m_aObjectKeys[_rName].setNodeValue(CONFIGKEY_DBLINK_DOCUMENTLOCAITON, makeAny(_rNewLink));
//  OSL_VERIFY(m_aConfigurationNode.commit());
}

//--------------------------------------------------------------------------
void OBookmarkContainer::checkValid(sal_Bool _bIntendWriteAccess) const throw (RuntimeException, DisposedException)
{
    if (!m_bInitialized)
        throw DisposedException();

    if (_bIntendWriteAccess && isReadOnly())
    {
        ::rtl::OUString sMessage = DBACORE_RESSTRING(RID_STR_NEED_CONFIG_WRITE_ACCESS);
        DisposedException(
            sMessage,
            Reference< XInterface >(const_cast<XServiceInfo*>(static_cast<const XServiceInfo*>(this)))
        );
    }

    DBG_ASSERT( (m_aObjectKeys.size() == m_aBookmarks.size()) &&
                (m_aBookmarks.size() == m_aBookmarksIndexed.size()),
        "OBookmarkContainer::checkValid : inconsistent state !");
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OBookmarkContainer::getParent(  ) throw (RuntimeException)
{
    return m_rParent;
}

//--------------------------------------------------------------------------
void SAL_CALL OBookmarkContainer::setParent( const Reference< XInterface >& Parent ) throw (NoSupportException, RuntimeException)
{
    throw NoSupportException();
}

//........................................................................
}   // namespace dbaccess
//........................................................................
