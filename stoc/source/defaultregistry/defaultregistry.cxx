/*************************************************************************
 *
 *  $RCSfile: defaultregistry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:32:37 $
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_HXX_
#include <osl/diagnose.h>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _REGISTRY_REGISTRY_HXX_
#include <registry/registry.hxx>
#endif

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace cppu;
using namespace vos;
using namespace osl;
using namespace rtl;


#define NESTED_SERVICENAME "com.sun.star.registry.NestedRegistry"
#define NESTED_IMPLNAME    "com.sun.star.comp.stoc.NestedRegistry"

namespace stoc_defreg
{

//*************************************************************************
// NestedRegistryImpl
//*************************************************************************
class NestedKeyImpl;

class NestedRegistryImpl    : public WeakAggImplHelper3 < XSimpleRegistry, XInitialization, XServiceInfo >
{
public:
    NestedRegistryImpl( const Reference<XMultiServiceFactory> & rXSMgr );

    NestedRegistryImpl( const Reference<XMultiServiceFactory> & rXSMgr,
                        Reference<XSimpleRegistry>& localReg,
                         Reference<XSimpleRegistry>& systemReg );

    ~NestedRegistryImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static OUString SAL_CALL getImplementationName_Static(  );
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

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

    friend NestedKeyImpl;
protected:
    Mutex                       m_mutex;
    sal_uInt32                  m_state;
    Reference<XSimpleRegistry>  m_localReg;
    Reference<XSimpleRegistry>  m_defaultReg;

private:
    Reference<XMultiServiceFactory> m_xSMgr;
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

    return sal_False;
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

    return 0;
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

    return Sequence<sal_Int32>();
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

    return OUString();
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

    return Sequence<OUString>();
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

    return OUString();
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

    return Sequence<OUString>();
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

    return Sequence<sal_Int8>();
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

    sal_Bool isCreated = False;
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
NestedRegistryImpl::NestedRegistryImpl( const Reference<XMultiServiceFactory> & rSMgr )
    : m_state(0)
    , m_xSMgr(rSMgr)
{
}

NestedRegistryImpl::NestedRegistryImpl( const Reference<XMultiServiceFactory> & rSMgr,
                                          Reference<XSimpleRegistry>& localReg,
                                          Reference<XSimpleRegistry>& defaultReg)
    : m_state(0)
    , m_localReg(localReg)
    , m_defaultReg(defaultReg)
    , m_xSMgr(rSMgr)
{
}

//*************************************************************************
NestedRegistryImpl::~NestedRegistryImpl()
{
}

//*************************************************************************
OUString SAL_CALL NestedRegistryImpl::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString::createFromAscii( NESTED_IMPLNAME );
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
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL NestedRegistryImpl::getSupportedServiceNames_Static(  )
{
    OUString aStr( OUString::createFromAscii( NESTED_SERVICENAME ) );
    return Sequence< OUString >( &aStr, 1 );
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
void SAL_CALL NestedRegistryImpl::open( const OUString& rURL, sal_Bool bReadOnly, sal_Bool bCreate )
    throw(InvalidRegistryException, RuntimeException)
{
    throw InvalidRegistryException();
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
    throw InvalidRegistryException();
}

//*************************************************************************
void SAL_CALL NestedRegistryImpl::destroy(  )
    throw(InvalidRegistryException, RuntimeException)
{
    throw InvalidRegistryException();
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

//*************************************************************************
Reference<XInterface> SAL_CALL NestedRegistry_CreateInstance( const Reference<XMultiServiceFactory>& rSMgr )
    throw(Exception)
{
    Reference<XInterface>   xRet;
    XSimpleRegistry *pRegistry = (XSimpleRegistry*) new NestedRegistryImpl(rSMgr);

    if (pRegistry)
    {
        xRet = Reference<XInterface>::query(pRegistry);
    }

    return xRet;
}

} // namespace stco_defreg

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            // NestedRegistry
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii( "/" NESTED_IMPLNAME "/UNO/SERVICES" ) ) );

            Sequence< OUString > & rSNL = ::stoc_defreg::NestedRegistryImpl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    if ( rtl_str_compare( pImplName, NESTED_IMPLNAME ) == 0 )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            ::stoc_defreg::NestedRegistry_CreateInstance,
            ::stoc_defreg::NestedRegistryImpl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



