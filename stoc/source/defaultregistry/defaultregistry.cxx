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

#include <osl/mutex.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <registry/registry.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

using namespace css::uno;
using namespace css::registry;
using namespace css::lang;
using namespace css::container;
using namespace cppu;
using namespace osl;

namespace {

class NestedKeyImpl;

class NestedRegistryImpl    : public WeakAggImplHelper4 < XSimpleRegistry, XInitialization, XServiceInfo, XEnumerationAccess >
{
public:
    NestedRegistryImpl( );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XSimpleRegistry
    virtual OUString SAL_CALL getURL() override;
    virtual void SAL_CALL open( const OUString& rURL, sal_Bool bReadOnly, sal_Bool bCreate ) override;
    virtual sal_Bool SAL_CALL isValid(  ) override;
    virtual void SAL_CALL close(  ) override;
    virtual void SAL_CALL destroy(  ) override;
    virtual Reference< XRegistryKey > SAL_CALL getRootKey(  ) override;
    virtual sal_Bool SAL_CALL isReadOnly(  ) override;
    virtual void SAL_CALL mergeKey( const OUString& aKeyName, const OUString& aUrl ) override;

    // XEnumerationAccess
    virtual Reference< XEnumeration > SAL_CALL createEnumeration(  ) override;
    virtual Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    friend class NestedKeyImpl;
protected:
    Mutex                       m_mutex;
    sal_uInt32                  m_state;
    Reference<XSimpleRegistry>  m_localReg;
    Reference<XSimpleRegistry>  m_defaultReg;

};


// class NestedKeyImpl the implementation of interface XRegistryKey

class NestedKeyImpl : public WeakImplHelper< XRegistryKey >
{
public:
    NestedKeyImpl( NestedRegistryImpl* pDefaultRegistry,
                   Reference<XRegistryKey>& localKey,
                   Reference<XRegistryKey>& defaultKey);

    NestedKeyImpl( const OUString& aKeyName,
                    NestedKeyImpl* pKey);

    // XRegistryKey
    virtual OUString SAL_CALL getKeyName() override;
    virtual sal_Bool SAL_CALL isReadOnly(  ) override;
    virtual sal_Bool SAL_CALL isValid(  ) override;
    virtual RegistryKeyType SAL_CALL getKeyType( const OUString& rKeyName ) override;
    virtual RegistryValueType SAL_CALL getValueType(  ) override;
    virtual sal_Int32 SAL_CALL getLongValue(  ) override;
    virtual void SAL_CALL setLongValue( sal_Int32 value ) override;
    virtual Sequence< sal_Int32 > SAL_CALL getLongListValue(  ) override;
    virtual void SAL_CALL setLongListValue( const css::uno::Sequence< sal_Int32 >& seqValue ) override;
    virtual OUString SAL_CALL getAsciiValue(  ) override;
    virtual void SAL_CALL setAsciiValue( const OUString& value ) override;
    virtual Sequence< OUString > SAL_CALL getAsciiListValue(  ) override;
    virtual void SAL_CALL setAsciiListValue( const css::uno::Sequence< OUString >& seqValue ) override;
    virtual OUString SAL_CALL getStringValue(  ) override;
    virtual void SAL_CALL setStringValue( const OUString& value ) override;
    virtual Sequence< OUString > SAL_CALL getStringListValue(  ) override;
    virtual void SAL_CALL setStringListValue( const css::uno::Sequence< OUString >& seqValue ) override;
    virtual Sequence< sal_Int8 > SAL_CALL getBinaryValue(  ) override;
    virtual void SAL_CALL setBinaryValue( const css::uno::Sequence< sal_Int8 >& value ) override;
    virtual Reference< XRegistryKey > SAL_CALL openKey( const OUString& aKeyName ) override;
    virtual Reference< XRegistryKey > SAL_CALL createKey( const OUString& aKeyName ) override;
    virtual void SAL_CALL closeKey(  ) override;
    virtual void SAL_CALL deleteKey( const OUString& rKeyName ) override;
    virtual Sequence< Reference< XRegistryKey > > SAL_CALL openKeys(  ) override;
    virtual Sequence< OUString > SAL_CALL getKeyNames(  ) override;
    virtual sal_Bool SAL_CALL createLink( const OUString& aLinkName, const OUString& aLinkTarget ) override;
    virtual void SAL_CALL deleteLink( const OUString& rLinkName ) override;
    virtual OUString SAL_CALL getLinkTarget( const OUString& rLinkName ) override;
    virtual OUString SAL_CALL getResolvedName( const OUString& aKeyName ) override;

protected:
    void        computeChanges();
    OUString    computeName(const OUString& name);

    OUString                    m_name;
    sal_uInt32                  m_state;
    rtl::Reference<NestedRegistryImpl> m_xRegistry;
    Reference<XRegistryKey>     m_localKey;
    Reference<XRegistryKey>     m_defaultKey;
};


NestedKeyImpl::NestedKeyImpl( NestedRegistryImpl* pDefaultRegistry,
                              Reference<XRegistryKey>& localKey,
                              Reference<XRegistryKey>& defaultKey )
    : m_xRegistry(pDefaultRegistry)
{
    m_localKey = localKey;
    m_defaultKey = defaultKey;

    if (m_localKey.is())
    {
        m_name = m_localKey->getKeyName();
    }
    else if (m_defaultKey.is())
    {
        m_name = m_defaultKey->getKeyName();
    }

    m_state = m_xRegistry->m_state;
}


NestedKeyImpl::NestedKeyImpl( const OUString& rKeyName,
                              NestedKeyImpl* pKey)
    : m_xRegistry(pKey->m_xRegistry)
{
    if (pKey->m_localKey.is() && pKey->m_localKey->isValid())
    {
        m_localKey = pKey->m_localKey->openKey(rKeyName);
    }
    if (pKey->m_defaultKey.is() && pKey->m_defaultKey->isValid())
    {
        m_defaultKey = pKey->m_defaultKey->openKey(rKeyName);
    }

    if (m_localKey.is())
    {
        m_name = m_localKey->getKeyName();
    }
    else if (m_defaultKey.is())
    {
        m_name = m_defaultKey->getKeyName();
    }

    m_state = m_xRegistry->m_state;
}

void NestedKeyImpl::computeChanges()
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( m_state == m_xRegistry->m_state )
        return;

    Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());

    Reference<XRegistryKey> tmpKey = rootKey->openKey(m_name);

    if ( tmpKey.is() )
    {
        m_localKey = rootKey->openKey(m_name);
    }

    m_state = m_xRegistry->m_state;
}


// NestedKey_Impl::computeName()

OUString NestedKeyImpl::computeName(const OUString& name)
{
    OUString resLocalName, resDefaultName;

    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    try
    {
        if ( m_localKey.is() && m_localKey->isValid() )
        {
            resLocalName = m_localKey->getResolvedName(name);
        }
        else if ( m_defaultKey.is() && m_defaultKey->isValid() )
        {
            return m_defaultKey->getResolvedName(name);
        }

        if ( !resLocalName.isEmpty() && m_xRegistry->m_defaultReg->isValid() )
        {
            Reference<XRegistryKey> localRoot(m_xRegistry->m_localReg->getRootKey());
            Reference<XRegistryKey> defaultRoot(m_xRegistry->m_defaultReg->getRootKey());

            resDefaultName = defaultRoot->getResolvedName(resLocalName);

            sal_uInt32 count = 100;

            while (resLocalName != resDefaultName && count > 0)
            {
                count--;

                if (resLocalName.isEmpty() || resDefaultName.isEmpty())
                    throw InvalidRegistryException();

                resLocalName = localRoot->getResolvedName(resDefaultName);
                resDefaultName = defaultRoot->getResolvedName(resLocalName);
            }
        }
    }
    catch(InvalidRegistryException& )
    {
    }

    return resLocalName;
}


OUString SAL_CALL NestedKeyImpl::getKeyName()
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    return m_name;
}


sal_Bool SAL_CALL NestedKeyImpl::isReadOnly(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( !m_localKey.is() || !m_localKey->isValid() )
        throw InvalidRegistryException();

    return m_localKey->isReadOnly();
}


sal_Bool SAL_CALL NestedKeyImpl::isValid(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    return ((m_localKey.is() && m_localKey->isValid()) ||
            (m_defaultKey.is() && m_defaultKey->isValid()) );
}


RegistryKeyType SAL_CALL NestedKeyImpl::getKeyType( const OUString& rKeyName )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getKeyType(rKeyName);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getKeyType(rKeyName);
    }

    return RegistryKeyType_KEY;
}


RegistryValueType SAL_CALL NestedKeyImpl::getValueType(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getValueType();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getValueType();
    }

    return RegistryValueType_NOT_DEFINED;
}


sal_Int32 SAL_CALL NestedKeyImpl::getLongValue(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getLongValue();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getLongValue();
    }
    else
    {
        throw InvalidRegistryException();
    }
}


void SAL_CALL NestedKeyImpl::setLongValue( sal_Int32 value )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setLongValue(value);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setLongValue(value);
        m_state = m_xRegistry->m_state++;
    }
    else
    {
        throw InvalidRegistryException();
    }
}


Sequence< sal_Int32 > SAL_CALL NestedKeyImpl::getLongListValue(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getLongListValue();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getLongListValue();
    }
    else
    {
        throw InvalidRegistryException();
    }
}


void SAL_CALL NestedKeyImpl::setLongListValue( const Sequence< sal_Int32 >& seqValue )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setLongListValue(seqValue);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setLongListValue(seqValue);
        m_state = m_xRegistry->m_state++;
    }
    else
    {
        throw InvalidRegistryException();
    }
}


OUString SAL_CALL NestedKeyImpl::getAsciiValue(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getAsciiValue();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getAsciiValue();
    }
    else
    {
        throw InvalidRegistryException();
    }
}


void SAL_CALL NestedKeyImpl::setAsciiValue( const OUString& value )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setAsciiValue(value);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setAsciiValue(value);
        m_state = m_xRegistry->m_state++;
    }
    else
    {
        throw InvalidRegistryException();
    }
}


Sequence< OUString > SAL_CALL NestedKeyImpl::getAsciiListValue(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getAsciiListValue();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getAsciiListValue();
    }
    else
    {
        throw InvalidRegistryException();
    }
}


void SAL_CALL NestedKeyImpl::setAsciiListValue( const Sequence< OUString >& seqValue )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setAsciiListValue(seqValue);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setAsciiListValue(seqValue);
        m_state = m_xRegistry->m_state++;
    }
    else
    {
        throw InvalidRegistryException();
    }
}


OUString SAL_CALL NestedKeyImpl::getStringValue(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getStringValue();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getStringValue();
    }
    else
    {
        throw InvalidRegistryException();
    }
}


void SAL_CALL NestedKeyImpl::setStringValue( const OUString& value )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setStringValue(value);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setStringValue(value);
        m_state = m_xRegistry->m_state++;
    }
    else
    {
        throw InvalidRegistryException();
    }
}


Sequence< OUString > SAL_CALL NestedKeyImpl::getStringListValue(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getStringListValue();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getStringListValue();
    }
    else
    {
        throw InvalidRegistryException();
    }
}


void SAL_CALL NestedKeyImpl::setStringListValue( const Sequence< OUString >& seqValue )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setStringListValue(seqValue);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setStringListValue(seqValue);
        m_state = m_xRegistry->m_state++;
    }
    else
    {
        throw InvalidRegistryException();
    }
}


Sequence< sal_Int8 > SAL_CALL NestedKeyImpl::getBinaryValue(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getBinaryValue();
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getBinaryValue();
    }
    else
    {
        throw InvalidRegistryException();
    }
}


void SAL_CALL NestedKeyImpl::setBinaryValue( const Sequence< sal_Int8 >& value )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setBinaryValue(value);
    }
    else if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setBinaryValue(value);
        m_state = m_xRegistry->m_state++;
    }
    else
    {
        throw InvalidRegistryException();
    }
}


Reference< XRegistryKey > SAL_CALL NestedKeyImpl::openKey( const OUString& aKeyName )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    OUString resolvedName = computeName(aKeyName);

    if ( resolvedName.isEmpty() )
        throw InvalidRegistryException();

    Reference<XRegistryKey> localKey, defaultKey;

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        localKey = m_xRegistry->m_localReg->getRootKey()->openKey(resolvedName);
    }
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        defaultKey = m_xRegistry->m_defaultReg->getRootKey()->openKey(resolvedName);
    }

    if ( localKey.is() || defaultKey.is() )
    {
        return new NestedKeyImpl(m_xRegistry.get(), localKey, defaultKey);
    }
    else
    {
        return Reference<XRegistryKey>();
    }
}


Reference< XRegistryKey > SAL_CALL NestedKeyImpl::createKey( const OUString& aKeyName )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( (!m_localKey.is() && !m_defaultKey.is()) ||
         (m_localKey.is() && m_localKey->isReadOnly()) )
    {
        throw InvalidRegistryException();
    }

    OUString resolvedName = computeName(aKeyName);

    if ( resolvedName.isEmpty() )
        throw InvalidRegistryException();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        Reference<XRegistryKey> localKey, defaultKey;

        localKey = m_xRegistry->m_localReg->getRootKey()->createKey(resolvedName);
        if ( localKey.is() )
        {
            if ( m_defaultKey.is() && m_defaultKey->isValid() )
            {
                defaultKey = m_xRegistry->m_defaultReg->getRootKey()->openKey(resolvedName);
            }

            m_state = m_xRegistry->m_state++;

            return new NestedKeyImpl(m_xRegistry.get(), localKey, defaultKey);
        }
    }
    else
    {
        Reference<XRegistryKey> localKey, defaultKey;

        if ( m_defaultKey.is() && m_defaultKey->isValid() )
        {
            Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
            m_localKey = rootKey->createKey(m_name);

            localKey = m_xRegistry->m_localReg->getRootKey()->createKey(resolvedName);

            if ( localKey.is() )
            {
                defaultKey = m_xRegistry->m_defaultReg->getRootKey()->openKey(resolvedName);

                m_state = m_xRegistry->m_state++;

                return new NestedKeyImpl(m_xRegistry.get(), localKey, defaultKey);
            }
        }
    }

    return Reference<XRegistryKey>();
}


void SAL_CALL NestedKeyImpl::closeKey(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->closeKey();
    }
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        m_defaultKey->closeKey();
    }
}


void SAL_CALL NestedKeyImpl::deleteKey( const OUString& rKeyName )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( !m_localKey.is() || !m_localKey->isValid() ||
         m_localKey->isReadOnly() )
    {
        throw InvalidRegistryException();
    }

    OUString resolvedName = computeName(rKeyName);

    if ( resolvedName.isEmpty() )
    {
        throw InvalidRegistryException();
    }

    m_xRegistry->m_localReg->getRootKey()->deleteKey(resolvedName);
}


Sequence< Reference< XRegistryKey > > SAL_CALL NestedKeyImpl::openKeys(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    Sequence<OUString> localSeq, defaultSeq;

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        localSeq = m_localKey->getKeyNames();
    }
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        defaultSeq = m_defaultKey->getKeyNames();
    }

    sal_uInt32 local = localSeq.getLength();
    sal_uInt32 def = defaultSeq.getLength();
    sal_uInt32 len = 0;

    sal_uInt32 i, j;
    for (i=0; i < local; i++)
    {
        for (j=0 ; j < def; j++)
        {
            if ( localSeq.getConstArray()[i] == defaultSeq.getConstArray()[j] )
            {
                len++;
                break;
            }
        }
    }

    Sequence< Reference<XRegistryKey> > retSeq(local + def - len);
    OUString                            name;
    sal_Int32                           lastIndex;

    for (i=0; i < local; i++)
    {
        name = localSeq.getConstArray()[i];
        lastIndex = name.lastIndexOf('/');
        name = name.copy(lastIndex);
        retSeq.getArray()[i] = new NestedKeyImpl(name, this);
    }

    sal_uInt32 k = local;
    for (i=0; i < def; i++)
    {
        bool insert = true;

        for (j=0 ; j < local; j++)
        {
            if ( retSeq.getConstArray()[j]->getKeyName()
                    == defaultSeq.getConstArray()[i] )
            {
                insert = false;
                break;
            }
        }

        if ( insert )
        {
            name = defaultSeq.getConstArray()[i];
            lastIndex = name.lastIndexOf('/');
            name = name.copy(lastIndex);
            retSeq.getArray()[k++] = new NestedKeyImpl(name, this);
        }
    }

    return retSeq;
}


Sequence< OUString > SAL_CALL NestedKeyImpl::getKeyNames(  )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    Sequence<OUString> localSeq, defaultSeq;

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        localSeq = m_localKey->getKeyNames();
    }
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        defaultSeq = m_defaultKey->getKeyNames();
    }

    sal_uInt32 local = localSeq.getLength();
    sal_uInt32 def = defaultSeq.getLength();
    sal_uInt32 len = 0;

    sal_uInt32 i, j;
    for (i=0; i < local; i++)
    {
        for (j=0 ; j < def; j++)
        {
            if ( localSeq.getConstArray()[i] == defaultSeq.getConstArray()[j] )
            {
                len++;
                break;
            }
        }
    }

    Sequence<OUString>  retSeq(local + def - len);

    for (i=0; i < local; i++)
    {
        retSeq.getArray()[i] = localSeq.getConstArray()[i];
    }

    sal_uInt32 k = local;
    for (i=0; i < def; i++)
    {
        bool insert = true;

        for (j=0 ; j < local; j++)
        {
            if ( retSeq.getConstArray()[j] == defaultSeq.getConstArray()[i] )
            {
                insert = false;
                break;
            }
        }

        if ( insert )
            retSeq.getArray()[k++] = defaultSeq.getConstArray()[i];
    }

    return retSeq;
}


sal_Bool SAL_CALL NestedKeyImpl::createLink( const OUString& aLinkName, const OUString& aLinkTarget )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );

    bool isCreated = false;
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    OUString    linkName;
    OUString    resolvedName;
    sal_Int32   lastIndex = aLinkName.lastIndexOf('/');

    if ( lastIndex > 0 )
    {
        linkName = aLinkName.copy(0, lastIndex);

        resolvedName = computeName(linkName);

        if ( resolvedName.isEmpty() )
        {
            throw InvalidRegistryException();
        }

        resolvedName = resolvedName + aLinkName.copy(lastIndex);
    }
    else
    {
        if ( lastIndex == 0 )
            resolvedName = m_name + aLinkName;
        else
            resolvedName = m_name + "/" + aLinkName;
    }

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        isCreated = m_xRegistry->m_localReg->getRootKey()->createLink(resolvedName, aLinkTarget);
    }
    else
    {
        if ( m_defaultKey.is() && m_defaultKey->isValid() )
        {
            Reference<XRegistryKey> rootKey(m_xRegistry->m_localReg->getRootKey());
            m_localKey = rootKey->createKey(m_name);

            isCreated = m_xRegistry->m_localReg->getRootKey()->createLink(resolvedName, aLinkTarget);
        }
    }

    if ( isCreated )
        m_state = m_xRegistry->m_state++;

    return isCreated;
}


void SAL_CALL NestedKeyImpl::deleteLink( const OUString& rLinkName )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    OUString    linkName;
    OUString    resolvedName;
    sal_Int32   lastIndex = rLinkName.lastIndexOf('/');

    if ( lastIndex > 0 )
    {
        linkName = rLinkName.copy(0, lastIndex);

        resolvedName = computeName(linkName);

        if ( resolvedName.isEmpty() )
        {
            throw InvalidRegistryException();
        }

        resolvedName = resolvedName + rLinkName.copy(lastIndex);
    }
    else
    {
        if ( lastIndex == 0 )
            resolvedName = m_name + rLinkName;
        else
            resolvedName = m_name + "/" + rLinkName;
    }

    if ( !m_localKey.is() || !m_localKey->isValid() ||
         m_localKey->isReadOnly() )
    {
        throw InvalidRegistryException();
    }

    m_xRegistry->m_localReg->getRootKey()->deleteLink(resolvedName);
}


OUString SAL_CALL NestedKeyImpl::getLinkTarget( const OUString& rLinkName )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    OUString    linkName;
    OUString    resolvedName;
    sal_Int32   lastIndex = rLinkName.lastIndexOf('/');

    if ( lastIndex > 0 )
    {
        linkName = rLinkName.copy(0, lastIndex);

        resolvedName = computeName(linkName);

        if ( resolvedName.isEmpty() )
        {
            throw InvalidRegistryException();
        }

        resolvedName = resolvedName + rLinkName.copy(lastIndex);
    }
    else
    {
        if ( lastIndex == 0 )
            resolvedName = m_name + rLinkName;
        else
            resolvedName = m_name + "/" + rLinkName;
    }

    OUString linkTarget;
    if ( m_localKey.is() && m_localKey->isValid() )
    {
        try
        {
            linkTarget = m_xRegistry->m_localReg->getRootKey()->getLinkTarget(resolvedName);
            return linkTarget;
        }
        catch(InvalidRegistryException& )
        {
        }
    }

    if ( m_defaultKey.is() && m_defaultKey->isValid() )
        linkTarget = m_xRegistry->m_defaultReg->getRootKey()->getLinkTarget(resolvedName);

    return linkTarget;
}


OUString SAL_CALL NestedKeyImpl::getResolvedName( const OUString& aKeyName )
{
    Guard< Mutex > aGuard( m_xRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    OUString resolvedName = computeName(aKeyName);

    if ( resolvedName.isEmpty() )
    {
        throw InvalidRegistryException();
    }

    return resolvedName;
}


// DefaultRegistry Implementation


NestedRegistryImpl::NestedRegistryImpl( )
    : m_state(0)
{}

class RegistryEnumueration : public WeakImplHelper< XEnumeration >
{
public:
    RegistryEnumueration(
        const Reference< XSimpleRegistry > &r1,
        const Reference< XSimpleRegistry > &r2 )
        : m_xReg1( r1 ) , m_xReg2( r2 )
        {}
public:
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual Any SAL_CALL nextElement(  ) override;

private:
    Reference< XSimpleRegistry > m_xReg1;
    Reference< XSimpleRegistry > m_xReg2;
};

sal_Bool RegistryEnumueration::hasMoreElements(  )
{
    return m_xReg1.is() || m_xReg2.is();
}

Any RegistryEnumueration::nextElement(  )
{
    Any a;
    if( m_xReg1.is() )
    {
        a <<= m_xReg1;
        m_xReg1.clear();
    }
    else if( m_xReg2.is() )
    {
        a <<= m_xReg2;
        m_xReg2.clear();
    }
    else
    {
        throw NoSuchElementException( "NestedRegistry: no nextElement() !" );
    }
    return a;
}


Reference< XEnumeration > NestedRegistryImpl::createEnumeration(  )
{
    MutexGuard guard( m_mutex );
    return new RegistryEnumueration( m_localReg, m_defaultReg );
}

Type NestedRegistryImpl::getElementType(  )
{
    return cppu::UnoType<decltype(m_localReg)>::get();
}

sal_Bool SAL_CALL NestedRegistryImpl::hasElements(  )
{
    MutexGuard guard( m_mutex );
    return m_localReg.is() || m_defaultReg.is();
}


OUString SAL_CALL NestedRegistryImpl::getImplementationName(  )
{
    return OUString("com.sun.star.comp.stoc.NestedRegistry");
}

sal_Bool SAL_CALL NestedRegistryImpl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence<OUString> SAL_CALL NestedRegistryImpl::getSupportedServiceNames(  )
{
    Sequence< OUString > seqNames { "com.sun.star.registry.NestedRegistry" };
    return seqNames;
}


void SAL_CALL NestedRegistryImpl::initialize( const Sequence< Any >& aArguments )
{
    Guard< Mutex > aGuard( m_mutex );
    if ( (aArguments.getLength() == 2) &&
         (aArguments[0].getValueType().getTypeClass() == TypeClass_INTERFACE) &&
         (aArguments[1].getValueType().getTypeClass() == TypeClass_INTERFACE) )
    {
        aArguments[0] >>= m_localReg;
        aArguments[1] >>= m_defaultReg;
        if ( m_localReg == m_defaultReg )
            m_defaultReg.clear();
    }
}


OUString SAL_CALL NestedRegistryImpl::getURL()
{
    Guard< Mutex > aGuard( m_mutex );
    try
    {
        if ( m_localReg.is() && m_localReg->isValid() )
            return m_localReg->getURL();
    }
    catch(InvalidRegistryException& )
    {
    }

    return OUString();
}


void SAL_CALL NestedRegistryImpl::open( const OUString&, sal_Bool, sal_Bool )
{
    throw InvalidRegistryException(
            "the 'open' method is not specified for a nested registry" );
}


sal_Bool SAL_CALL NestedRegistryImpl::isValid(  )
{
    Guard< Mutex > aGuard( m_mutex );
    try
    {
        if ( (m_localReg.is() && m_localReg->isValid()) ||
             (m_defaultReg.is() && m_defaultReg->isValid()) )
            return true;
    }
    catch(InvalidRegistryException& )
    {
    }

    return false;
}


void SAL_CALL NestedRegistryImpl::close(  )
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_localReg.is() && m_localReg->isValid() )
    {
        m_localReg->close();
    }
    if ( m_defaultReg.is() && m_defaultReg->isValid() )
    {
        m_defaultReg->close();
    }
}


void SAL_CALL NestedRegistryImpl::destroy(  )
{
    throw InvalidRegistryException(
            "the 'destroy' method is not specified for a nested registry" );
}


Reference< XRegistryKey > SAL_CALL NestedRegistryImpl::getRootKey(  )
{
    Guard< Mutex > aGuard( m_mutex );
    if ( !m_localReg.is() || !m_localReg->isValid() )
    {
        throw InvalidRegistryException();
    }

    Reference<XRegistryKey> localKey, defaultKey;

    localKey = m_localReg->getRootKey();

    if ( localKey.is() )
    {
        if ( m_defaultReg.is() && m_defaultReg->isValid() )
        {
            defaultKey = m_defaultReg->getRootKey();
        }

        return new NestedKeyImpl(this, localKey, defaultKey);
    }

    return Reference<XRegistryKey>();
}


sal_Bool SAL_CALL NestedRegistryImpl::isReadOnly(  )
{
    Guard< Mutex > aGuard( m_mutex );
    try
    {
        if ( m_localReg.is() && m_localReg->isValid() )
            return m_localReg->isReadOnly();
    }
    catch(InvalidRegistryException& )
    {
    }

    return false;
}


void SAL_CALL NestedRegistryImpl::mergeKey( const OUString& aKeyName, const OUString& aUrl )
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_localReg.is() && m_localReg->isValid() )
    {
        m_localReg->mergeKey(aKeyName, aUrl);

        m_state++;
    }
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_stoc_NestedRegistry_get_implementation(
    SAL_UNUSED_PARAMETER css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new NestedRegistryImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
