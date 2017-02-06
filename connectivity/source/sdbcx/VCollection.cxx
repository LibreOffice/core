/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <algorithm>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <connectivity/sdbcx/VCollection.hxx>
#include <connectivity/sdbcx/VDescriptor.hxx>
#include <connectivity/dbexception.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/container.hxx>
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>
#include "TConnection.hxx"
#include <rtl/ustrbuf.hxx>
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

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

namespace
{
    template < typename T> class OHardRefMap : public connectivity::sdbcx::IObjectCollection
    {
        typedef ::std::multimap< OUString, T , ::comphelper::UStringMixLess> ObjectMap;
        typedef typename ObjectMap::iterator   ObjectIter;
        typedef typename ObjectMap::value_type ObjectEntry;

    //  private:
        // this combination of map and vector is used to have a fast name and index access
        ::std::vector< ObjectIter >             m_aElements;        // hold the iterators which point to map
        ObjectMap                               m_aNameMap;         // hold the elements and a name
    public:
        OHardRefMap(bool _bCase)
            : m_aNameMap(_bCase)
        {
        }

        virtual bool exists(const OUString& _sName ) override
        {
            return m_aNameMap.find(_sName) != m_aNameMap.end();
        }

        virtual bool empty() override
        {
            return m_aNameMap.empty();
        }

        virtual void swapAll() override
        {
            ::std::vector< ObjectIter >(m_aElements).swap(m_aElements);
            ObjectMap(m_aNameMap).swap(m_aNameMap);
        }

        virtual void swap() override
        {
            ::std::vector< ObjectIter >().swap(m_aElements);

            OSL_ENSURE( m_aNameMap.empty(), "swap: what did disposeElements do?" );
            ObjectMap( m_aNameMap ).swap( m_aNameMap );
                // Note that it's /important/ to construct the new ObjectMap from m_aNameMap before
                // swapping. This way, it's ensured that the compare object held by these maps is preserved
                // during the swap. If we would not do this, the UStringMixLess instance which is used would be
                // default constructed (instead of being constructed from the same instance in m_aNameMap), and
                // it's case-sensitive flag would have an unpredictable value.
        }

        virtual void clear() override
        {
            m_aElements.clear();
            m_aNameMap.clear();
        }

        virtual void insert(const OUString& _sName,const ObjectType& _xObject) override
        {
            m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectEntry(_sName,_xObject)));
        }

        virtual void reFill(const TStringVector &_rVector) override
        {
            OSL_ENSURE(!m_aNameMap.size(),"OCollection::reFill: collection isn't empty");
            m_aElements.reserve(_rVector.size());

            for(TStringVector::const_iterator i=_rVector.begin(); i != _rVector.end();++i)
                m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectEntry(*i,ObjectType())));
        }

        virtual bool rename(const OUString& _sOldName, const OUString& _sNewName) override
        {
            bool bRet = false;
            ObjectIter aIter = m_aNameMap.find(_sOldName);
            if ( aIter != m_aNameMap.end() )
            {
                typename ::std::vector< ObjectIter >::iterator aFind = ::std::find(m_aElements.begin(),m_aElements.end(),aIter);
                if(m_aElements.end() != aFind)
                {
                    (*aFind) = m_aNameMap.insert(m_aNameMap.begin(), ObjectEntry(_sNewName,(*aFind)->second));
                    m_aNameMap.erase(aIter);

                    bRet = true;
                }
            }
            return bRet;
        }

        virtual sal_Int32 size() override
        {
            return static_cast<sal_Int32>(m_aNameMap.size());
        }

        virtual Sequence< OUString > getElementNames() override
        {
            Sequence< OUString > aNameList(m_aElements.size());

            OUString* pStringArray = aNameList.getArray();
            typename ::std::vector< ObjectIter >::const_iterator aEnd = m_aElements.end();
            for(typename ::std::vector< ObjectIter >::const_iterator aIter = m_aElements.begin(); aIter != aEnd;++aIter,++pStringArray)
                *pStringArray = (*aIter)->first;

            return aNameList;
        }

        virtual OUString getName(sal_Int32 _nIndex) override
        {
            return m_aElements[_nIndex]->first;
        }

        virtual void disposeAndErase(sal_Int32 _nIndex) override
        {
            OSL_ENSURE(_nIndex >= 0 && _nIndex < static_cast<sal_Int32>(m_aElements.size()),"Illegal argument!");
            Reference<XComponent> xComp(m_aElements[_nIndex]->second.get(),UNO_QUERY);
            ::comphelper::disposeComponent(xComp);
            m_aElements[_nIndex]->second = T();

            OUString sName = m_aElements[_nIndex]->first;
            m_aElements.erase(m_aElements.begin()+_nIndex);
            m_aNameMap.erase(sName);
        }

        virtual void disposeElements() override
        {
            for( ObjectIter aIter = m_aNameMap.begin(); aIter != m_aNameMap.end(); ++aIter)
            {
                Reference<XComponent> xComp(aIter->second.get(),UNO_QUERY);
                if ( xComp.is() )
                {
                    ::comphelper::disposeComponent(xComp);
                    (*aIter).second = T();
                }
            }
            m_aElements.clear();
            m_aNameMap.clear();
        }

        virtual sal_Int32 findColumn( const OUString& columnName ) override
        {
            ObjectIter aIter = m_aNameMap.find(columnName);
            OSL_ENSURE(aIter != m_aNameMap.end(),"findColumn:: Illegal name!");
            return m_aElements.size() - (m_aElements.end() - ::std::find(m_aElements.begin(),m_aElements.end(),aIter));
        }

        virtual ObjectType getObject(sal_Int32 _nIndex) override
        {
            OSL_ENSURE(_nIndex >= 0 && _nIndex < static_cast<sal_Int32>(m_aElements.size()),"Illegal argument!");
            return m_aElements[_nIndex]->second;
        }

        virtual ObjectType getObject(const OUString& columnName) override
        {
            return m_aNameMap.find(columnName)->second;
        }

        virtual void setObject(sal_Int32 _nIndex,const ObjectType& _xObject) override
        {
            OSL_ENSURE(_nIndex >= 0 && _nIndex < static_cast<sal_Int32>(m_aElements.size()),"Illegal argument!");
            m_aElements[_nIndex]->second = _xObject;
        }

        bool isCaseSensitive() const override
        {
            return m_aNameMap.key_comp().isCaseSensitive();
        }

    };
}

IObjectCollection::~IObjectCollection() {}

IMPLEMENT_SERVICE_INFO(OCollection,"com.sun.star.sdbcx.VContainer" , "com.sun.star.sdbcx.Container")

OCollection::OCollection(::cppu::OWeakObject& _rParent
                         , bool _bCase
                         , ::osl::Mutex& _rMutex
                         , const TStringVector &_rVector
                         , bool _bUseIndexOnly
                         , bool _bUseHardRef)
                     :m_aContainerListeners(_rMutex)
                     ,m_aRefreshListeners(_rMutex)
                     ,m_rParent(_rParent)
                     ,m_rMutex(_rMutex)
                     ,m_bUseIndexOnly(_bUseIndexOnly)
{
    if ( _bUseHardRef )
    {
        m_pElements.reset(new OHardRefMap< ObjectType >(_bCase));
    }
    else
    {
        m_pElements.reset(new OHardRefMap< WeakReference< XPropertySet> >(_bCase));
    }
    m_pElements->reFill(_rVector);
}

OCollection::~OCollection()
{
}

Any SAL_CALL OCollection::queryInterface( const Type & rType )
{
    if ( m_bUseIndexOnly && rType == cppu::UnoType<XNameAccess>::get() )
    {
        return Any();
    }
    return OCollectionBase::queryInterface( rType );
}

Sequence< Type > SAL_CALL OCollection::getTypes()
{
    if ( m_bUseIndexOnly )
    {
        Sequence< Type > aTypes(OCollectionBase::getTypes());
        Type* pBegin    = aTypes.getArray();
        Type* pEnd      = pBegin + aTypes.getLength();

        ::std::vector<Type> aOwnTypes;
        aOwnTypes.reserve(aTypes.getLength());
        Type aType = cppu::UnoType<XNameAccess>::get();
        for(;pBegin != pEnd; ++pBegin)
        {
            if ( *pBegin != aType )
                aOwnTypes.push_back(*pBegin);
        }
        return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
    }
    return OCollectionBase::getTypes( );
}

void OCollection::clear_NoDispose()
{
    ::osl::MutexGuard aGuard(m_rMutex);

    m_pElements->clear();
    m_pElements->swapAll();
}


void OCollection::disposing()
{
    m_aContainerListeners.disposeAndClear(EventObject(static_cast<XTypeProvider*>(this)));
    m_aRefreshListeners.disposeAndClear(EventObject(static_cast<XTypeProvider*>(this)));

    ::osl::MutexGuard aGuard(m_rMutex);

    disposeElements();

    m_pElements->swap();
}

Any SAL_CALL OCollection::getByIndex( sal_Int32 Index )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (Index < 0 || Index >= m_pElements->size() )
        throw IndexOutOfBoundsException(OUString::number(Index),static_cast<XTypeProvider*>(this));

    return makeAny(getObject(Index));
}

Any SAL_CALL OCollection::getByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard(m_rMutex);

    if ( !m_pElements->exists(aName) )
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceStringWithSubstitution(
                STR_NO_ELEMENT_NAME,
                "$name$", aName
             ) );
        throw NoSuchElementException( sError, static_cast< XTypeProvider* >( this ) );
    }

    return makeAny(getObject(m_pElements->findColumn(aName)));
}

Sequence< OUString > SAL_CALL OCollection::getElementNames(  )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_pElements->getElementNames();
}

void SAL_CALL OCollection::refresh(  )
{
    ::osl::MutexGuard aGuard(m_rMutex);

    disposeElements();

    impl_refresh();
    EventObject aEvt(static_cast<XTypeProvider*>(this));
    m_aRefreshListeners.notifyEach( &XRefreshListener::refreshed, aEvt );
}

void OCollection::reFill(const TStringVector &_rVector)
{
    m_pElements->reFill(_rVector);
}

// XDataDescriptorFactory
Reference< XPropertySet > SAL_CALL OCollection::createDataDescriptor(  )
{
    ::osl::MutexGuard aGuard(m_rMutex);

    return createDescriptor();
}

OUString OCollection::getNameForObject(const ObjectType& _xObject)
{
    OSL_ENSURE(_xObject.is(),"OCollection::getNameForObject: Object is NULL!");
    OUString sName;
    _xObject->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sName;
    return sName;
}

// XAppend
void SAL_CALL OCollection::appendByDescriptor( const Reference< XPropertySet >& descriptor )
{
    ::osl::ClearableMutexGuard aGuard(m_rMutex);

    OUString sName = getNameForObject( descriptor );

    if ( m_pElements->exists(sName) )
        throw ElementExistException(sName,static_cast<XTypeProvider*>(this));

    ObjectType xNewlyCreated = appendObject( sName, descriptor );
    if ( !xNewlyCreated.is() )
        throw RuntimeException();

    ODescriptor* pDescriptor = ODescriptor::getImplementation( xNewlyCreated );
    if ( pDescriptor )
        pDescriptor->setNew( false );

    sName = getNameForObject( xNewlyCreated );
    if ( !m_pElements->exists( sName ) ) // this may happen when the derived class included it itself
        m_pElements->insert( sName, xNewlyCreated );

    // notify our container listeners
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(sName), makeAny(xNewlyCreated), Any());
    aGuard.clear();
    m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvent );
}

// XDrop
void SAL_CALL OCollection::dropByName( const OUString& elementName )
{
    ::osl::MutexGuard aGuard(m_rMutex);

    if ( !m_pElements->exists(elementName) )
        throw NoSuchElementException(elementName,static_cast<XTypeProvider*>(this));

    dropImpl(m_pElements->findColumn(elementName));
}

void SAL_CALL OCollection::dropByIndex( sal_Int32 index )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if(index <0 || index >= getCount())
        throw IndexOutOfBoundsException(OUString::number(index),static_cast<XTypeProvider*>(this));

    dropImpl(index);
}

void OCollection::dropImpl(sal_Int32 _nIndex, bool _bReallyDrop)
{
    OUString elementName = m_pElements->getName(_nIndex);

    if ( _bReallyDrop )
        dropObject(_nIndex,elementName);

    m_pElements->disposeAndErase(_nIndex);

    // notify our container listeners
    notifyElementRemoved(elementName);
}

void OCollection::notifyElementRemoved(const OUString& _sName)
{
    ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_sName), Any(), Any());
    // note that xExistent may be empty, in case somebody removed the data source while it is not alive at this moment
    OInterfaceIteratorHelper2 aListenerLoop(m_aContainerListeners);
    while (aListenerLoop.hasMoreElements())
        static_cast<XContainerListener*>(aListenerLoop.next())->elementRemoved(aEvent);
}

sal_Int32 SAL_CALL OCollection::findColumn( const OUString& columnName )
{
    if ( !m_pElements->exists(columnName) )
    {
        ::dbtools::throwInvalidColumnException( columnName, static_cast< XIndexAccess*>(this) );
#if !(defined(_MSC_VER) && defined(ENABLE_LTO))
        assert(false);
#endif
    }

    return m_pElements->findColumn(columnName) + 1; // because columns start at one
}

Reference< XEnumeration > SAL_CALL OCollection::createEnumeration(  )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return new OEnumerationByIndex( static_cast< XIndexAccess*>(this));
}

void SAL_CALL OCollection::addContainerListener( const Reference< XContainerListener >& _rxListener )
{
    m_aContainerListeners.addInterface(_rxListener);
}


void SAL_CALL OCollection::removeContainerListener( const Reference< XContainerListener >& _rxListener )
{
    m_aContainerListeners.removeInterface(_rxListener);
}

void SAL_CALL OCollection::acquire() throw()
{
    m_rParent.acquire();
}

void SAL_CALL OCollection::release() throw()
{
    m_rParent.release();
}

Type SAL_CALL OCollection::getElementType(  )
{
    return cppu::UnoType<XPropertySet>::get();
}

sal_Bool SAL_CALL OCollection::hasElements(  )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return !m_pElements->empty();
}

sal_Int32 SAL_CALL OCollection::getCount(  )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_pElements->size();
}

sal_Bool SAL_CALL OCollection::hasByName( const OUString& aName )
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_pElements->exists(aName);
}

void SAL_CALL OCollection::addRefreshListener( const Reference< XRefreshListener >& l )
{
    m_aRefreshListeners.addInterface(l);
}

void SAL_CALL OCollection::removeRefreshListener( const Reference< XRefreshListener >& l )
{
    m_aRefreshListeners.removeInterface(l);
}

void OCollection::insertElement(const OUString& _sElementName,const ObjectType& _xElement)
{
    OSL_ENSURE(!m_pElements->exists(_sElementName),"Element already exists");
    if ( !m_pElements->exists(_sElementName) )
        m_pElements->insert(_sElementName,_xElement);
}

void OCollection::renameObject(const OUString& _sOldName, const OUString& _sNewName)
{
    OSL_ENSURE(m_pElements->exists(_sOldName),"Element doesn't exist");
    OSL_ENSURE(!m_pElements->exists(_sNewName),"Element already exists");
    OSL_ENSURE(!_sNewName.isEmpty(),"New name must not be empty!");
    OSL_ENSURE(!_sOldName.isEmpty(),"Old name must not be empty!");

    if ( m_pElements->rename(_sOldName,_sNewName) )
    {
        ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_sNewName), makeAny(m_pElements->getObject(_sNewName)),makeAny(_sOldName));
        // note that xExistent may be empty, in case somebody removed the data source while it is not alive at this moment
        OInterfaceIteratorHelper2 aListenerLoop(m_aContainerListeners);
        while (aListenerLoop.hasMoreElements())
            static_cast<XContainerListener*>(aListenerLoop.next())->elementReplaced(aEvent);
    }
}

ObjectType OCollection::getObject(sal_Int32 _nIndex)
{
    ObjectType xName = m_pElements->getObject(_nIndex);
    if ( !xName.is() )
    {
        try
        {
            xName = createObject(m_pElements->getName(_nIndex));
        }
        catch(const SQLException& e)
        {
            try
            {
                dropImpl(_nIndex,false);
            }
            catch(const Exception& )
            {
            }
            throw WrappedTargetException(e.Message,static_cast<XTypeProvider*>(this),makeAny(e));
        }
        m_pElements->setObject(_nIndex,xName);
    }
    return xName;
}

void OCollection::disposeElements()
{
    m_pElements->disposeElements();
}

Reference< XPropertySet > OCollection::createDescriptor()
{
    OSL_FAIL("createDescriptor() needs to be overridden when used!");
    throw SQLException();
}

ObjectType OCollection::cloneDescriptor( const ObjectType& _descriptor )
{
    ObjectType xNewDescriptor( createDescriptor() );
    ::comphelper::copyProperties( _descriptor, xNewDescriptor );
    return xNewDescriptor;
}

ObjectType OCollection::appendObject( const OUString& /*_rForName*/, const Reference< XPropertySet >& descriptor )
{
    return cloneDescriptor( descriptor );
}

void OCollection::dropObject(sal_Int32 /*_nPos*/, const OUString& /*_sElementName*/)
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
