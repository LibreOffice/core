/*************************************************************************
 *
 *  $RCSfile: VCollection.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 14:47:24 $
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
#include <stdio.h>

#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#include "connectivity/sdbcx/VDescriptor.hxx"
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace comphelper;
using namespace ::cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OCollection,"com.sun.star.sdbcx.VContainer" , "com.sun.star.sdbcx.Container")

OCollection::OCollection(::cppu::OWeakObject& _rParent,sal_Bool _bCase, ::osl::Mutex& _rMutex,const TStringVector &_rVector,sal_Bool _bUseIndexOnly)
                     : m_rParent(_rParent)
                     ,m_rMutex(_rMutex)
                     ,m_aContainerListeners(_rMutex)
                     ,m_aRefreshListeners(_rMutex)
                     ,m_aNameMap(_bCase ? true : false)
                     ,m_bUseIndexOnly(_bUseIndexOnly)
{
    m_aElements.reserve(_rVector.size());
    for(TStringVector::const_iterator i=_rVector.begin(); i != _rVector.end();++i)
        m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(*i,WeakReference< XNamed >())));
}
// -------------------------------------------------------------------------
OCollection::~OCollection()
{
}
// -----------------------------------------------------------------------------
Any SAL_CALL OCollection::queryInterface( const Type & rType ) throw (RuntimeException)
{
    if ( m_bUseIndexOnly && rType == ::getCppuType(static_cast< Reference< XNameAccess > *> (NULL)) )
    {
        return Any();
    }
    return OCollectionBase::queryInterface( rType );
}
// -----------------------------------------------------------------------------
Sequence< Type > SAL_CALL OCollection::getTypes() throw (RuntimeException)
{
    if ( m_bUseIndexOnly )
    {
        Sequence< Type > aTypes(OCollectionBase::getTypes());
        Type* pBegin    = aTypes.getArray();
        Type* pEnd      = pBegin + aTypes.getLength();

        ::std::vector<Type> aOwnTypes;
        aOwnTypes.reserve(aTypes.getLength());
        Type aType = ::getCppuType(static_cast< Reference<XNameAccess> *>(NULL));
        sal_Int32 i=0;
        for(;pBegin != pEnd; ++pBegin)
        {
            if ( *pBegin != aType )
                aOwnTypes.push_back(*pBegin);
        }

        return Sequence< Type >(aOwnTypes.begin(),aOwnTypes.size());
    }
    return OCollectionBase::getTypes( );
}
// -------------------------------------------------------------------------
void OCollection::clear_NoDispose()
{
    ::osl::MutexGuard aGuard(m_rMutex);

    m_aElements.clear();
    m_aNameMap.clear();

    ::std::vector< ObjectIter >(m_aElements).swap(m_aElements);
    ObjectMap(m_aNameMap).swap(m_aNameMap);
}

// -------------------------------------------------------------------------
void OCollection::disposing(void)
{
    m_aContainerListeners.disposeAndClear(EventObject(static_cast<XWeak*>(this)));
    m_aRefreshListeners.disposeAndClear(EventObject(static_cast<XWeak*>(this)));

    ::osl::MutexGuard aGuard(m_rMutex);

    disposeElements();

    ::std::vector< ObjectIter >().swap(m_aElements);

    OSL_ENSURE( m_aNameMap.empty(), "OCollection::disposing: what did disposeElements do?" );
    ObjectMap( m_aNameMap ).swap( m_aNameMap );
        // Note that it's /important/ to construct the new ObjectMap from m_aNameMap before
        // swapping. This way, it's ensured that the compare object held by these maps is preserved
        // during the swap. If we would not do this, the UStringMixLess instance which is used would be
        // default constructed (instead of being constructed from the same instance in m_aNameMap), and
        // it's case-sensitive flag would have an unpredictable value.
        // 2002-01-09 - #106589# - fs@openoffice.org
}
// -------------------------------------------------------------------------
Any SAL_CALL OCollection::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (Index < 0 || Index >= static_cast<sal_Int32>(m_aNameMap.size()))
        throw IndexOutOfBoundsException(::rtl::OUString::valueOf(Index),*this);

    ObjectIter aIter = m_aElements[Index];
    return makeAny(getObject(aIter));
}
// -------------------------------------------------------------------------
Any SAL_CALL OCollection::getByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ObjectIter aIter = m_aNameMap.find(aName);
    if(aIter == m_aNameMap.end())
        throw NoSuchElementException(aName,*this);
    return makeAny(getObject(aIter));
}
// -------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OCollection::getElementNames(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    sal_Int32 nLen = m_aElements.size();
    Sequence< ::rtl::OUString > aNameList(nLen);

    ::rtl::OUString* pStringArray = aNameList.getArray();
    for(::std::vector< ObjectIter >::const_iterator aIter = m_aElements.begin(); aIter != m_aElements.end();++aIter,++pStringArray)
        *pStringArray = (*aIter)->first;

    return aNameList;
}
// -------------------------------------------------------------------------
void SAL_CALL OCollection::refresh(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    disposeElements();

    impl_refresh();
    EventObject aEvt(static_cast<XWeak*>(this));
    NOTIFY_LISTENERS(m_aRefreshListeners, XRefreshListener, refreshed, aEvt);
}
// -----------------------------------------------------------------------------
void OCollection::reFill(const TStringVector &_rVector)
{
    OSL_ENSURE(!m_aNameMap.size(),"OCollection::reFill: collection isn't empty");
    m_aElements.reserve(_rVector.size());

    for(TStringVector::const_iterator i=_rVector.begin(); i != _rVector.end();++i)
        m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(*i,WeakReference< XNamed >())));
}
// -------------------------------------------------------------------------
// XDataDescriptorFactory
Reference< XPropertySet > SAL_CALL OCollection::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    return createEmptyObject();
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OCollection::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    Reference< XNamed > xName(descriptor,UNO_QUERY);
    if(xName.is())
    {
        ::rtl::OUString sName = xName->getName();
        if(m_aNameMap.find(sName) != m_aNameMap.end())
            throw ElementExistException(sName,*this);

        appendObject(descriptor);
        Reference< XNamed > xNewName = cloneObject(descriptor);
        Reference<XUnoTunnel> xTunnel(xNewName,UNO_QUERY);
        if(xTunnel.is())
        {
            ODescriptor* pDescriptor = (ODescriptor*)xTunnel->getSomething(ODescriptor::getUnoTunnelImplementationId());
            if(pDescriptor)
                pDescriptor->setNew(sal_False);
        }

        if(xNewName.is())
        {
            sName = xNewName->getName();
            if(m_aNameMap.find(sName) == m_aNameMap.end()) // this may happen when the drived class included it itself
                m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(sName,WeakReference< XNamed >(xNewName))));
            // notify our container listeners
            ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(sName), makeAny(xNewName), Any());
            OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
            while (aListenerLoop.hasMoreElements())
                static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
        }
        else
            throw SQLException();
    }
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OCollection::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    dropImpl(aIter);
}
// -----------------------------------------------------------------------------
void OCollection::dropImpl(const ObjectIter& _rCurrentObject,sal_Bool _bReallyDrop)
{
    ::std::vector< ObjectIter >::size_type nCount = m_aElements.size();
    for(::std::vector< ObjectIter >::size_type i=0; i < nCount;++i)
    {
        if ( m_aElements[i] == _rCurrentObject )
        {
            dropImpl(i,_bReallyDrop);
            break; // no duplicates possible
        }
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OCollection::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if(index <0 || index >= getCount())
        throw IndexOutOfBoundsException(::rtl::OUString::valueOf(index),*this);

    dropImpl(index);
}
// -----------------------------------------------------------------------------
void OCollection::dropImpl(sal_Int32 _nIndex,sal_Bool _bReallyDrop)
{
    ::rtl::OUString elementName = m_aElements[_nIndex]->first;

    if ( _bReallyDrop )
        dropObject(_nIndex,elementName);

    ::comphelper::disposeComponent(m_aElements[_nIndex]->second);

    m_aNameMap.erase(m_aElements[_nIndex]);
    m_aElements.erase(m_aElements.begin()+_nIndex);

    // notify our container listeners
    notifyElementRemoved(elementName);
}
// -----------------------------------------------------------------------------
void OCollection::notifyElementRemoved(const ::rtl::OUString& _sName)
{
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_sName), Any(), Any());
    // note that xExistent may be empty, in case somebody removed the data source while it is not alive at this moment
    OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementRemoved(aEvent);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCollection::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    ObjectIter aIter = m_aNameMap.find(columnName);
    if(aIter == m_aNameMap.end())
        throw SQLException(::rtl::OUString::createFromAscii("Unknown column name!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,makeAny(NoSuchElementException(columnName,*this)) );

    return m_aElements.size() - (m_aElements.end() - ::std::find(m_aElements.begin(),m_aElements.end(),aIter)) +1; // because cloumns start at one
}
// -------------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL OCollection::createEnumeration(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return new OEnumerationByIndex( static_cast< XIndexAccess*>(this));
}
// -----------------------------------------------------------------------------
void SAL_CALL OCollection::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    m_aContainerListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OCollection::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    m_aContainerListeners.removeInterface(_rxListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OCollection::acquire() throw()
{
    m_rParent.acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OCollection::release() throw()
{
    m_rParent.release();
}
// -----------------------------------------------------------------------------
Type SAL_CALL OCollection::getElementType(  ) throw(RuntimeException)
{
    return::getCppuType(static_cast< Reference< XPropertySet>*>(NULL));
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OCollection::hasElements(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return !m_aNameMap.empty();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OCollection::getCount(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_aNameMap.size();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OCollection::hasByName( const ::rtl::OUString& aName ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_aNameMap.find(aName) != m_aNameMap.end();
}
// -----------------------------------------------------------------------------
void SAL_CALL OCollection::addRefreshListener( const Reference< XRefreshListener >& l ) throw(RuntimeException)
{
    m_aRefreshListeners.addInterface(l);
}
// -----------------------------------------------------------------------------
void SAL_CALL OCollection::removeRefreshListener( const Reference< XRefreshListener >& l ) throw(RuntimeException)
{
    m_aRefreshListeners.removeInterface(l);
}
// -----------------------------------------------------------------------------
void OCollection::insertElement(const ::rtl::OUString& _sElementName,const Object_BASE& _xElement)
{
    OSL_ENSURE(m_aNameMap.find(_sElementName) == m_aNameMap.end(),"Element already exists");
    if(m_aNameMap.find(_sElementName) == m_aNameMap.end())
        m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(_sElementName,_xElement)));
}
// -----------------------------------------------------------------------------
void OCollection::renameObject(const ::rtl::OUString _sOldName,const ::rtl::OUString _sNewName)
{

    OSL_ENSURE(m_aNameMap.find(_sOldName) != m_aNameMap.end(),"Element doesn't exist");
    OSL_ENSURE(m_aNameMap.find(_sNewName) == m_aNameMap.end(),"Element already exists");
    OSL_ENSURE(_sNewName.getLength(),"New name must not be empty!");
    OSL_ENSURE(_sOldName.getLength(),"New name must not be empty!");

    ObjectMap::iterator aIter = m_aNameMap.find(_sOldName);
    if(aIter != m_aNameMap.end())
    {
        ::std::vector< ObjectIter >::iterator aFind = ::std::find(m_aElements.begin(),m_aElements.end(),aIter);
        if(m_aElements.end() != aFind)
        {
            (*aFind) = m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(_sNewName,(*aFind)->second));
            m_aNameMap.erase(aIter);

            ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_sNewName), makeAny((*aFind)->second),makeAny(_sOldName));
            // note that xExistent may be empty, in case somebody removed the data source while it is not alive at this moment
            OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
            while (aListenerLoop.hasMoreElements())
                static_cast<XContainerListener*>(aListenerLoop.next())->elementReplaced(aEvent);
        }
    }
}
// -----------------------------------------------------------------------------
Reference< XNamed > OCollection::getObject(ObjectIter& _rCurrentObject)
{
    Reference< XNamed > xName = (*_rCurrentObject).second;
    if(!(*_rCurrentObject).second.is())
    {
        try
        {
            xName = createObject((*_rCurrentObject).first);
        }
        catch(const SQLException& e)
        {
            try
            {
                dropImpl(_rCurrentObject,sal_False);
            }
            catch(const Exception& )
            {
            }
            throw WrappedTargetException(e.Message,*this,makeAny(e));
        }
        (*_rCurrentObject).second = xName;
    }
    return xName;
}
// -----------------------------------------------------------------------------
void OCollection::disposeElements()
{
    for( ObjectIter aIter = m_aNameMap.begin(); aIter != m_aNameMap.end(); ++aIter)
    {
        if(aIter->second.is())
        {
            ::comphelper::disposeComponent(aIter->second);
            (*aIter).second = NULL;
        }
    }
    m_aNameMap.clear();
    m_aElements.clear();
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OCollection::createEmptyObject()
{
    OSL_ASSERT(!"Need to be overloaded when used!");
    throw SQLException();
}
// -----------------------------------------------------------------------------
void OCollection::appendObject( const Reference< XPropertySet >& descriptor )
{
}
// -----------------------------------------------------------------------------
Reference< XNamed > OCollection::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    OSL_ASSERT(!"Need to be overloaded when used!");
    throw SQLException();
}
// -----------------------------------------------------------------------------
void OCollection::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
}
// -----------------------------------------------------------------------------
