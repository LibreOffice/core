/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_stoc.hxx"
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <registry/registry.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <bootstrapservices.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace cppu;
using namespace osl;
using ::rtl::OUString;

#define SERVICENAME "com.sun.star.registry.NestedRegistry"
#define IMPLNAME       "com.sun.star.comp.stoc.NestedRegistry"

extern rtl_StandardModuleCount g_moduleCount;

namespace stoc_bootstrap
{
Sequence< OUString > defreg_getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM(SERVICENAME));
            pNames = &seqNames;
        }
    }
    return *pNames;
}

OUString defreg_getImplementationName()
{
    static OUString *pImplName = 0;
    if( ! pImplName )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pImplName )
        {
            static OUString implName( RTL_CONSTASCII_USTRINGPARAM( IMPLNAME ) );
            pImplName = &implName;
        }
    }
    return *pImplName;
}
}

namespace stoc_defreg
{
//*************************************************************************
// NestedRegistryImpl
//*************************************************************************
class NestedKeyImpl;

class NestedRegistryImpl    : public WeakAggImplHelper4 < XSimpleRegistry, XInitialization, XServiceInfo, XEnumerationAccess >
{
public:
    NestedRegistryImpl( );

    ~NestedRegistryImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException);

    // XSimpleRegistry
    virtual OUString SAL_CALL getURL() throw(RuntimeException);
    virtual void SAL_CALL open( const OUString& rURL, sal_Bool bReadOnly, sal_Bool bCreate ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  ) throw(RuntimeException);
    virtual void SAL_CALL close(  ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL destroy(  ) throw(InvalidRegistryException, RuntimeException);
    virtual Reference< XRegistryKey > SAL_CALL getRootKey(  ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL mergeKey( const OUString& aKeyName, const OUString& aUrl ) throw(InvalidRegistryException, MergeConflictException, RuntimeException);

    // XEnumerationAccess
    virtual Reference< XEnumeration > SAL_CALL createEnumeration(  ) throw (RuntimeException);
    virtual Type SAL_CALL getElementType(  ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException);

    friend class NestedKeyImpl;
protected:
    Mutex                       m_mutex;
    sal_uInt32                  m_state;
    Reference<XSimpleRegistry>  m_localReg;
    Reference<XSimpleRegistry>  m_defaultReg;

};

//*************************************************************************
// class NestedKeyImpl the implenetation of interface XRegistryKey
//*************************************************************************
class NestedKeyImpl : public WeakImplHelper1< XRegistryKey >
{
public:
    NestedKeyImpl( NestedRegistryImpl* pDefaultRegistry,
                   Reference<XRegistryKey>& localKey,
                   Reference<XRegistryKey>& defaultKey);

    NestedKeyImpl( const OUString& aKeyName,
                    NestedKeyImpl* pKey);

    ~NestedKeyImpl();

    // XRegistryKey
    virtual OUString SAL_CALL getKeyName() throw(RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  ) throw(RuntimeException);
    virtual RegistryKeyType SAL_CALL getKeyType( const OUString& rKeyName ) throw(InvalidRegistryException, RuntimeException);
    virtual RegistryValueType SAL_CALL getValueType(  ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Int32 SAL_CALL getLongValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException);
    virtual void SAL_CALL setLongValue( sal_Int32 value ) throw(InvalidRegistryException, RuntimeException);
    virtual Sequence< sal_Int32 > SAL_CALL getLongListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException);
    virtual void SAL_CALL setLongListValue( const ::com::sun::star::uno::Sequence< sal_Int32 >& seqValue ) throw(InvalidRegistryException, RuntimeException);
    virtual OUString SAL_CALL getAsciiValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException);
    virtual void SAL_CALL setAsciiValue( const OUString& value ) throw(InvalidRegistryException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getAsciiListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException);
    virtual void SAL_CALL setAsciiListValue( const ::com::sun::star::uno::Sequence< OUString >& seqValue ) throw(InvalidRegistryException, RuntimeException);
    virtual OUString SAL_CALL getStringValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException);
    virtual void SAL_CALL setStringValue( const OUString& value ) throw(InvalidRegistryException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getStringListValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException);
    virtual void SAL_CALL setStringListValue( const ::com::sun::star::uno::Sequence< OUString >& seqValue ) throw(InvalidRegistryException, RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getBinaryValue(  ) throw(InvalidRegistryException, InvalidValueException, RuntimeException);
    virtual void SAL_CALL setBinaryValue( const ::com::sun::star::uno::Sequence< sal_Int8 >& value ) throw(InvalidRegistryException, RuntimeException);
    virtual Reference< XRegistryKey > SAL_CALL openKey( const OUString& aKeyName ) throw(InvalidRegistryException, RuntimeException);
    virtual Reference< XRegistryKey > SAL_CALL createKey( const OUString& aKeyName ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL closeKey(  ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL deleteKey( const OUString& rKeyName ) throw(InvalidRegistryException, RuntimeException);
    virtual Sequence< Reference< XRegistryKey > > SAL_CALL openKeys(  ) throw(InvalidRegistryException, RuntimeException);
    virtual Sequence< OUString > SAL_CALL getKeyNames(  ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL createLink( const OUString& aLinkName, const OUString& aLinkTarget ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL deleteLink( const OUString& rLinkName ) throw(InvalidRegistryException, RuntimeException);
    virtual OUString SAL_CALL getLinkTarget( const OUString& rLinkName ) throw(InvalidRegistryException, RuntimeException);
    virtual OUString SAL_CALL getResolvedName( const OUString& aKeyName ) throw(InvalidRegistryException, RuntimeException);

protected:
    void        computeChanges();
    OUString    computeName(const OUString& name);

    OUString                    m_name;
    sal_uInt32                  m_state;
    NestedRegistryImpl*         m_pRegistry;
    Reference<XRegistryKey>     m_localKey;
    Reference<XRegistryKey>     m_defaultKey;
};


//*************************************************************************
NestedKeyImpl::NestedKeyImpl( NestedRegistryImpl* pDefaultRegistry,
                              Reference<XRegistryKey>& localKey,
                              Reference<XRegistryKey>& defaultKey )
    : m_pRegistry(pDefaultRegistry)
{
    m_pRegistry->acquire();

    m_localKey = localKey;
    m_defaultKey = defaultKey;

    if (m_localKey.is())
    {
        m_name = m_localKey->getKeyName();
    } else
    if (m_defaultKey.is())
    {
        m_name = m_defaultKey->getKeyName();
    }

    m_state = m_pRegistry->m_state;
}

//*************************************************************************
NestedKeyImpl::NestedKeyImpl( const OUString& rKeyName,
                              NestedKeyImpl* pKey)
    : m_pRegistry(pKey->m_pRegistry)
{
    m_pRegistry->acquire();

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
    } else
    if (m_defaultKey.is())
    {
        m_name = m_defaultKey->getKeyName();
    }

    m_state = m_pRegistry->m_state;
}

//*************************************************************************
NestedKeyImpl::~NestedKeyImpl()
{
    if ( m_pRegistry )
        m_pRegistry->release();
}

//*************************************************************************
void NestedKeyImpl::computeChanges()
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( m_state != m_pRegistry->m_state )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());

        Reference<XRegistryKey> tmpKey = rootKey->openKey(m_name);

        if ( tmpKey.is() )
        {
            m_localKey = rootKey->openKey(m_name);
        }

        m_state = m_pRegistry->m_state;
    }
}

//*************************************************************************
// NestedKey_Impl::computeName()
//
OUString NestedKeyImpl::computeName(const OUString& name)
{
    OUString resLocalName, resDefaultName;

    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    try
    {
        if ( m_localKey.is() && m_localKey->isValid() )
        {
            resLocalName = m_localKey->getResolvedName(name);
        } else
        {
            if ( m_defaultKey.is() && m_defaultKey->isValid() )
                return m_defaultKey->getResolvedName(name);
        }

        if ( resLocalName.getLength() > 0 && m_pRegistry->m_defaultReg->isValid() )
        {
            Reference<XRegistryKey> localRoot(m_pRegistry->m_localReg->getRootKey());
            Reference<XRegistryKey> defaultRoot(m_pRegistry->m_defaultReg->getRootKey());

            resDefaultName = defaultRoot->getResolvedName(resLocalName);

            sal_uInt32 count = 100;

            while (resLocalName != resDefaultName && count > 0)
            {
                count--;

                if (resLocalName.getLength() == 0 || resDefaultName.getLength() == 0)
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

//*************************************************************************
OUString SAL_CALL NestedKeyImpl::getKeyName() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    return m_name;
}

//*************************************************************************
sal_Bool SAL_CALL NestedKeyImpl::isReadOnly(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
           return m_localKey->isReadOnly();
    else
        throw InvalidRegistryException();
}

//*************************************************************************
sal_Bool SAL_CALL NestedKeyImpl::isValid(  ) throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    return ((m_localKey.is() && m_localKey->isValid()) ||
            (m_defaultKey.is() && m_defaultKey->isValid()) );
}

//*************************************************************************
RegistryKeyType SAL_CALL NestedKeyImpl::getKeyType( const OUString& rKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getKeyType(rKeyName);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getKeyType(rKeyName);
    }

    return RegistryKeyType_KEY;
}

//*************************************************************************
RegistryValueType SAL_CALL NestedKeyImpl::getValueType(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getValueType();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getValueType();
    }

    return RegistryValueType_NOT_DEFINED;
}

//*************************************************************************
sal_Int32 SAL_CALL NestedKeyImpl::getLongValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getLongValue();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getLongValue();
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::setLongValue( sal_Int32 value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setLongValue(value);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setLongValue(value);
        m_state = m_pRegistry->m_state++;
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
Sequence< sal_Int32 > SAL_CALL NestedKeyImpl::getLongListValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getLongListValue();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getLongListValue();
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::setLongListValue( const Sequence< sal_Int32 >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setLongListValue(seqValue);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setLongListValue(seqValue);
        m_state = m_pRegistry->m_state++;
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
OUString SAL_CALL NestedKeyImpl::getAsciiValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getAsciiValue();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getAsciiValue();
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::setAsciiValue( const OUString& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setAsciiValue(value);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setAsciiValue(value);
        m_state = m_pRegistry->m_state++;
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
Sequence< OUString > SAL_CALL NestedKeyImpl::getAsciiListValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getAsciiListValue();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getAsciiListValue();
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::setAsciiListValue( const Sequence< OUString >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setAsciiListValue(seqValue);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setAsciiListValue(seqValue);
        m_state = m_pRegistry->m_state++;
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
OUString SAL_CALL NestedKeyImpl::getStringValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getStringValue();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getStringValue();
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::setStringValue( const OUString& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setStringValue(value);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setStringValue(value);
        m_state = m_pRegistry->m_state++;
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
Sequence< OUString > SAL_CALL NestedKeyImpl::getStringListValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getStringListValue();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getStringListValue();
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::setStringListValue( const Sequence< OUString >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setStringListValue(seqValue);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setStringListValue(seqValue);
        m_state = m_pRegistry->m_state++;
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
Sequence< sal_Int8 > SAL_CALL NestedKeyImpl::getBinaryValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        return m_localKey->getBinaryValue();
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        return m_defaultKey->getBinaryValue();
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::setBinaryValue( const Sequence< sal_Int8 >& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    computeChanges();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->setBinaryValue(value);
    } else
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
        m_localKey = rootKey->createKey(m_name);
        m_localKey->setBinaryValue(value);
        m_state = m_pRegistry->m_state++;
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL NestedKeyImpl::openKey( const OUString& aKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    OUString resolvedName = computeName(aKeyName);

    if ( resolvedName.getLength() == 0 )
        throw InvalidRegistryException();

    Reference<XRegistryKey> localKey, defaultKey;

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        localKey = m_pRegistry->m_localReg->getRootKey()->openKey(resolvedName);
    }
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        defaultKey = m_pRegistry->m_defaultReg->getRootKey()->openKey(resolvedName);
    }

    if ( localKey.is() || defaultKey.is() )
    {
        return ((XRegistryKey*)new NestedKeyImpl(m_pRegistry, localKey, defaultKey));
    } else
    {
        return Reference<XRegistryKey>();
    }
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL NestedKeyImpl::createKey( const OUString& aKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( (!m_localKey.is() && !m_defaultKey.is()) ||
         (m_localKey.is() && m_localKey->isReadOnly()) )
    {
        throw InvalidRegistryException();
    }

    OUString resolvedName = computeName(aKeyName);

    if ( resolvedName.getLength() == 0 )
        throw InvalidRegistryException();

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        Reference<XRegistryKey> localKey, defaultKey;

        localKey = m_pRegistry->m_localReg->getRootKey()->createKey(resolvedName);
        if ( localKey.is() )
        {
            if ( m_defaultKey.is() && m_defaultKey->isValid() )
            {
                defaultKey = m_pRegistry->m_defaultReg->getRootKey()->openKey(resolvedName);
            }

            m_state = m_pRegistry->m_state++;

            return ((XRegistryKey*)new NestedKeyImpl(m_pRegistry, localKey, defaultKey));
        }
    } else
    {
        Reference<XRegistryKey> localKey, defaultKey;

        if ( m_defaultKey.is() && m_defaultKey->isValid() )
        {
            Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
            m_localKey = rootKey->createKey(m_name);

            localKey = m_pRegistry->m_localReg->getRootKey()->createKey(resolvedName);

            if ( localKey.is() )
            {
                defaultKey = m_pRegistry->m_defaultReg->getRootKey()->openKey(resolvedName);

                m_state = m_pRegistry->m_state++;

                return ((XRegistryKey*)new NestedKeyImpl(m_pRegistry, localKey, defaultKey));
            }
        }
    }

    return Reference<XRegistryKey>();
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::closeKey(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( m_localKey.is() && m_localKey->isValid() )
    {
        m_localKey->closeKey();
    }
    if ( m_defaultKey.is() && m_defaultKey->isValid() )
    {
        m_defaultKey->closeKey();
    }
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::deleteKey( const OUString& rKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( m_localKey.is() && m_localKey->isValid() &&
         !m_localKey->isReadOnly() )
    {
        OUString resolvedName = computeName(rKeyName);

        if ( resolvedName.getLength() == 0 )
        {
            throw InvalidRegistryException();
        }

        m_pRegistry->m_localReg->getRootKey()->deleteKey(resolvedName);
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
Sequence< Reference< XRegistryKey > > SAL_CALL NestedKeyImpl::openKeys(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
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
    sal_Bool                            insert = sal_True;
    OUString                            name;
    sal_Int32                           lastIndex;

    for (i=0; i < local; i++)
    {
        name = localSeq.getConstArray()[i];
        lastIndex = name.lastIndexOf('/');
        name = name.copy(lastIndex);
        retSeq.getArray()[i] =
            (XRegistryKey*)new NestedKeyImpl(name, this);
    }

    sal_uInt32 k = local;
    for (i=0; i < def; i++)
    {
        insert = sal_True;

        for (j=0 ; j < local; j++)
        {
            if ( retSeq.getConstArray()[j]->getKeyName()
                    == defaultSeq.getConstArray()[i] )
            {
                insert = sal_False;
                break;
            }
        }

        if ( insert )
        {
            name = defaultSeq.getConstArray()[i];
            lastIndex = name.lastIndexOf('/');
            name = name.copy(lastIndex);
            retSeq.getArray()[k++] =
                (XRegistryKey*)new NestedKeyImpl(name, this);
        }
    }

    return retSeq;
}

//*************************************************************************
Sequence< OUString > SAL_CALL NestedKeyImpl::getKeyNames(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
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
    sal_Bool            insert = sal_True;

    for (i=0; i < local; i++)
    {
        retSeq.getArray()[i] = localSeq.getConstArray()[i];
    }

    sal_uInt32 k = local;
    for (i=0; i < def; i++)
    {
        insert = sal_True;

        for (j=0 ; j < local; j++)
        {
            if ( retSeq.getConstArray()[j] == defaultSeq.getConstArray()[i] )
            {
                insert = sal_False;
                break;
            }
        }

        if ( insert )
            retSeq.getArray()[k++] = defaultSeq.getConstArray()[i];
    }

    return retSeq;
}

//*************************************************************************
sal_Bool SAL_CALL NestedKeyImpl::createLink( const OUString& aLinkName, const OUString& aLinkTarget )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );

    sal_Bool isCreated = sal_False;
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

        if ( resolvedName.getLength() == 0 )
        {
            throw InvalidRegistryException();
        }

        resolvedName = resolvedName + aLinkName.copy(lastIndex);
    } else
    {
        if ( lastIndex == 0 )
            resolvedName = m_name + aLinkName;
        else
            resolvedName = m_name + OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + aLinkName;
    }

    if ( m_localKey.is() && m_localKey->isValid() )
    {
        isCreated = m_pRegistry->m_localReg->getRootKey()->createLink(resolvedName, aLinkTarget);
    } else
    {
        if ( m_defaultKey.is() && m_defaultKey->isValid() )
        {
            Reference<XRegistryKey> rootKey(m_pRegistry->m_localReg->getRootKey());
            m_localKey = rootKey->createKey(m_name);

            isCreated = m_pRegistry->m_localReg->getRootKey()->createLink(resolvedName, aLinkTarget);
        }
    }

    if ( isCreated )
        m_state = m_pRegistry->m_state++;

    return isCreated;
}

//*************************************************************************
void SAL_CALL NestedKeyImpl::deleteLink( const OUString& rLinkName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
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

        if ( resolvedName.getLength() == 0 )
        {
            throw InvalidRegistryException();
        }

        resolvedName = resolvedName + rLinkName.copy(lastIndex);
    } else
    {
        if ( lastIndex == 0 )
            resolvedName = m_name + rLinkName;
        else
            resolvedName = m_name + OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rLinkName;
    }

    if ( m_localKey.is() && m_localKey->isValid() &&
         !m_localKey->isReadOnly() )
    {
        m_pRegistry->m_localReg->getRootKey()->deleteLink(resolvedName);
    } else
    {
        throw InvalidRegistryException();
    }
}

//*************************************************************************
OUString SAL_CALL NestedKeyImpl::getLinkTarget( const OUString& rLinkName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
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

        if ( resolvedName.getLength() == 0 )
        {
            throw InvalidRegistryException();
        }

        resolvedName = resolvedName + rLinkName.copy(lastIndex);
    } else
    {
        if ( lastIndex == 0 )
            resolvedName = m_name + rLinkName;
        else
            resolvedName = m_name + OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rLinkName;
    }

    OUString linkTarget;
    if ( m_localKey.is() && m_localKey->isValid() )
    {
        try
        {
            linkTarget = m_pRegistry->m_localReg->getRootKey()->getLinkTarget(resolvedName);
            return linkTarget;
        }
        catch(InvalidRegistryException& )
        {
        }
    }

    if ( m_defaultKey.is() && m_defaultKey->isValid() )
        linkTarget = m_pRegistry->m_defaultReg->getRootKey()->getLinkTarget(resolvedName);

    return linkTarget;
}

//*************************************************************************
OUString SAL_CALL NestedKeyImpl::getResolvedName( const OUString& aKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_localKey.is() && !m_defaultKey.is() )
    {
        throw InvalidRegistryException();
    }

    OUString resolvedName = computeName(aKeyName);

    if ( resolvedName.getLength() == 0 )
    {
        throw InvalidRegistryException();
    }

    return resolvedName;
}

//*************************************************************************
//
// DefaultRegistry Implementation
//
//*************************************************************************
NestedRegistryImpl::NestedRegistryImpl( )
    : m_state(0)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}

//*************************************************************************
NestedRegistryImpl::~NestedRegistryImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}


class RegistryEnumueration : public WeakImplHelper1< XEnumeration >
{
public:
    RegistryEnumueration(
        const Reference< XSimpleRegistry > &r1,
        const Reference< XSimpleRegistry > &r2 )
        : m_xReg1( r1 ) , m_xReg2( r2 )
        {}
public:
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (RuntimeException);
    virtual Any SAL_CALL nextElement(  ) throw (NoSuchElementException, WrappedTargetException, RuntimeException);

private:
    Reference< XSimpleRegistry > m_xReg1;
    Reference< XSimpleRegistry > m_xReg2;
};

sal_Bool RegistryEnumueration::hasMoreElements(  ) throw (RuntimeException)
{
    return m_xReg1.is() || m_xReg2.is();
}

Any RegistryEnumueration::nextElement(  )
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
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
        throw NoSuchElementException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "NestedRegistry: no nextElement() !" ) ),Reference< XInterface > () );
    }
    return a;
}


Reference< XEnumeration > NestedRegistryImpl::createEnumeration(  ) throw (RuntimeException)
{
    MutexGuard guard( m_mutex );
    return new RegistryEnumueration( m_localReg, m_defaultReg );
}

Type NestedRegistryImpl::getElementType(  ) throw (RuntimeException)
{
    return getCppuType( &m_localReg );
}

sal_Bool SAL_CALL NestedRegistryImpl::hasElements(  ) throw (RuntimeException)
{
    MutexGuard guard( m_mutex );
    return m_localReg.is() || m_defaultReg.is();
}



//*************************************************************************
OUString SAL_CALL NestedRegistryImpl::getImplementationName(  )
    throw(RuntimeException)
{
    return stoc_bootstrap::defreg_getImplementationName();
}

//*************************************************************************
sal_Bool SAL_CALL NestedRegistryImpl::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//*************************************************************************
Sequence<OUString> SAL_CALL NestedRegistryImpl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return stoc_bootstrap::defreg_getSupportedServiceNames();
}

//*************************************************************************
void SAL_CALL NestedRegistryImpl::initialize( const Sequence< Any >& aArguments )
    throw( Exception, RuntimeException )
{
    Guard< Mutex > aGuard( m_mutex );
    if ( (aArguments.getLength() == 2) &&
         (aArguments[0].getValueType().getTypeClass() == TypeClass_INTERFACE) &&
         (aArguments[1].getValueType().getTypeClass() == TypeClass_INTERFACE) )
    {
        aArguments[0] >>= m_localReg;
        aArguments[1] >>= m_defaultReg;
        if ( m_localReg == m_defaultReg )
            m_defaultReg = Reference< XSimpleRegistry >();
    }
}

//*************************************************************************
OUString SAL_CALL NestedRegistryImpl::getURL() throw(RuntimeException)
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

//*************************************************************************
void SAL_CALL NestedRegistryImpl::open( const OUString&, sal_Bool, sal_Bool )
    throw(InvalidRegistryException, RuntimeException)
{
    throw InvalidRegistryException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("the 'open' method is not specified for a nested registry")),
            Reference< XInterface >() );
}

//*************************************************************************
sal_Bool SAL_CALL NestedRegistryImpl::isValid(  ) throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    try
    {
        if ( (m_localReg.is() && m_localReg->isValid()) ||
             (m_defaultReg.is() && m_defaultReg->isValid()) )
            return sal_True;
    }
    catch(InvalidRegistryException& )
    {
    }

    return sal_False;
}

//*************************************************************************
void SAL_CALL NestedRegistryImpl::close(  )
    throw(InvalidRegistryException, RuntimeException)
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
/*
    throw InvalidRegistryException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("the 'close' method is not specified for a nested registry")),
            Reference< XInterface >() );
*/
}

//*************************************************************************
void SAL_CALL NestedRegistryImpl::destroy(  )
    throw(InvalidRegistryException, RuntimeException)
{
    throw InvalidRegistryException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("the 'destroy' method is not specified for a nested registry")),
            Reference< XInterface >() );
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL NestedRegistryImpl::getRootKey(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Reference<XRegistryKey> tmpKey;

    Guard< Mutex > aGuard( m_mutex );
    if ( m_localReg.is() && m_localReg->isValid() )
    {
        Reference<XRegistryKey> localKey, defaultKey;

        localKey = m_localReg->getRootKey();

        if ( localKey.is() )
        {
            if ( m_defaultReg.is() && m_defaultReg->isValid() )
            {
                defaultKey = m_defaultReg->getRootKey();
            }

            return ((XRegistryKey*)new NestedKeyImpl(this, localKey, defaultKey));
        }
    } else
    {
        throw InvalidRegistryException();
    }

    return Reference<XRegistryKey>();
}

//*************************************************************************
sal_Bool SAL_CALL NestedRegistryImpl::isReadOnly(  )
    throw(InvalidRegistryException, RuntimeException)
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

    return sal_False;
}

//*************************************************************************
void SAL_CALL NestedRegistryImpl::mergeKey( const OUString& aKeyName, const OUString& aUrl )
    throw(InvalidRegistryException, MergeConflictException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_localReg.is() && m_localReg->isValid() )
    {
        m_localReg->mergeKey(aKeyName, aUrl);

        m_state++;
    }
}
} // namespace stco_defreg

namespace stoc_bootstrap
{
//*************************************************************************
Reference<XInterface> SAL_CALL NestedRegistry_CreateInstance( const Reference<XComponentContext>& )
    throw(Exception)
{
    Reference<XInterface>   xRet;
    XSimpleRegistry *pRegistry = (XSimpleRegistry*) new stoc_defreg::NestedRegistryImpl;

    if (pRegistry)
    {
        xRet = Reference<XInterface>::query(pRegistry);
    }

    return xRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
