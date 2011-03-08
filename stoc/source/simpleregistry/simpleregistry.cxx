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
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <registry/registry.hxx>

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace cppu;
using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringToOString;

#define SERVICENAME "com.sun.star.registry.SimpleRegistry"
#define IMPLNAME    "com.sun.star.comp.stoc.SimpleRegistry"

extern rtl_StandardModuleCount g_moduleCount;

namespace stoc_bootstrap
{
Sequence< OUString > simreg_getSupportedServiceNames()
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

OUString simreg_getImplementationName()
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

namespace stoc_simreg {

//*************************************************************************
// class RegistryKeyImpl the implenetation of interface XRegistryKey
//*************************************************************************
class RegistryKeyImpl;

//*************************************************************************
// SimpleRegistryImpl
//*************************************************************************
class SimpleRegistryImpl    : public WeakImplHelper2< XSimpleRegistry, XServiceInfo >
{
public:
    SimpleRegistryImpl( const Registry& rRegistry );

    ~SimpleRegistryImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

    // XSimpleRegistry
    virtual OUString SAL_CALL getURL() throw(RuntimeException);
    virtual void SAL_CALL open( const OUString& rURL, sal_Bool bReadOnly, sal_Bool bCreate ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  ) throw(RuntimeException);
    virtual void SAL_CALL close(  ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL destroy(  ) throw(InvalidRegistryException, RuntimeException);
    virtual Reference< XRegistryKey > SAL_CALL getRootKey(  ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL mergeKey( const OUString& aKeyName, const OUString& aUrl ) throw(InvalidRegistryException, MergeConflictException, RuntimeException);

    friend class RegistryKeyImpl;
protected:
    Mutex       m_mutex;
    OUString    m_url;
    Registry    m_registry;
};


class RegistryKeyImpl : public WeakImplHelper1< XRegistryKey >
{
public:
    RegistryKeyImpl( const RegistryKey& rKey, SimpleRegistryImpl* pRegistry );

    RegistryKeyImpl( const OUString& rKeyName, SimpleRegistryImpl* pRegistry );

    ~RegistryKeyImpl();

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
    OUString            m_name;
    RegistryKey         m_key;
    SimpleRegistryImpl* m_pRegistry;
};

//*************************************************************************
RegistryKeyImpl::RegistryKeyImpl( const RegistryKey& key, SimpleRegistryImpl* pRegistry )
    : m_key(key)
    , m_pRegistry(pRegistry)
{
    m_pRegistry->acquire();
    m_name = m_key.getName();
}

//*************************************************************************
RegistryKeyImpl::RegistryKeyImpl( const OUString& rKeyName,
                                  SimpleRegistryImpl* pRegistry )
    : m_pRegistry(pRegistry)
{
    m_pRegistry->acquire();

    RegistryKey rootKey;
    if (!pRegistry->m_registry.isValid() ||
        pRegistry->m_registry.openRootKey(rootKey))
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        if ( rootKey.openKey(rKeyName, m_key) )
        {
            throw InvalidRegistryException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                (OWeakObject *)this );
        } else
        {
            m_name = rKeyName;
        }
    }
}

//*************************************************************************
RegistryKeyImpl::~RegistryKeyImpl()
{
    m_pRegistry->release();
}

//*************************************************************************
OUString SAL_CALL RegistryKeyImpl::getKeyName() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    return m_name;
}

//*************************************************************************
sal_Bool SAL_CALL RegistryKeyImpl::isReadOnly(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if (m_key.isValid())
    {
        return(m_key.isReadOnly());
    } else
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
sal_Bool SAL_CALL RegistryKeyImpl::isValid(  ) throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    return m_key.isValid();
}

//*************************************************************************
RegistryKeyType SAL_CALL RegistryKeyImpl::getKeyType( const OUString& rKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( m_key.isValid() )
    {
        RegKeyType keyType;
        if ( !m_key.getKeyType(rKeyName, &keyType) )
        {
            switch (keyType)
            {
                case RG_KEYTYPE:
                    return RegistryKeyType_KEY;
                case RG_LINKTYPE:
                    return RegistryKeyType_LINK;
            }
        } else
        {
            throw InvalidRegistryException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                (OWeakObject *)this );
        }
    } else
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    }

    return RegistryKeyType_KEY;
}

//*************************************************************************
RegistryValueType SAL_CALL RegistryKeyImpl::getValueType(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if (!m_key.isValid())
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if (m_key.getValueInfo(OUString(), &type, &size))
        {
            return RegistryValueType_NOT_DEFINED;
        } else
        {
            switch (type)
            {
                case RG_VALUETYPE_LONG:         return RegistryValueType_LONG;
                case RG_VALUETYPE_STRING:       return RegistryValueType_ASCII;
                case RG_VALUETYPE_UNICODE:      return RegistryValueType_STRING;
                case RG_VALUETYPE_BINARY:       return RegistryValueType_BINARY;
                case RG_VALUETYPE_LONGLIST:     return RegistryValueType_LONGLIST;
                case RG_VALUETYPE_STRINGLIST:   return RegistryValueType_ASCIILIST;
                case RG_VALUETYPE_UNICODELIST:  return RegistryValueType_STRINGLIST;
                default:                        return RegistryValueType_NOT_DEFINED;
            }
        }
    }

    return RegistryValueType_NOT_DEFINED;
}

//*************************************************************************
sal_Int32 SAL_CALL RegistryKeyImpl::getLongValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if (!m_key.isValid())
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if ( !m_key.getValueInfo(OUString(), &type, &size) )
        {
            if (type == RG_VALUETYPE_LONG)
            {
                sal_Int32 value;
                if ( !m_key.getValue(OUString(), (RegValue)&value) )
                {
                    return value;
                }
            }
        }

        throw InvalidValueException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setLongValue( sal_Int32 value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        if (m_key.setValue(OUString(), RG_VALUETYPE_LONG, &value, sizeof(sal_Int32)))
        {
            throw InvalidValueException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                (OWeakObject *)this );
        }
    }
}

//*************************************************************************
Sequence< sal_Int32 > SAL_CALL RegistryKeyImpl::getLongListValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if ( !m_key.getValueInfo(OUString(), &type, &size) )
        {
            if (type == RG_VALUETYPE_LONGLIST)
            {
                RegistryValueList<sal_Int32> tmpValue;
                if ( !m_key.getLongListValue(OUString(), tmpValue) )
                {
                    Sequence<sal_Int32> seqValue(size);

                    for (sal_uInt32 i=0; i < size; i++)
                    {
                        seqValue.getArray()[i] = tmpValue.getElement(i);
                    }

                    return seqValue;
                }
            }
        }

        throw InvalidValueException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setLongListValue( const Sequence< sal_Int32 >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        sal_uInt32 length = seqValue.getLength();
        sal_Int32* tmpValue = new sal_Int32[length];

        for (sal_uInt32 i=0; i < length; i++)
        {
            tmpValue[i] = seqValue.getConstArray()[i];
        }

        if ( m_key.setLongListValue(OUString(), tmpValue, length) )
        {
            delete[] tmpValue;
            throw InvalidValueException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                (OWeakObject *)this );
        }

        delete[] tmpValue;
    }
}

//*************************************************************************
OUString SAL_CALL RegistryKeyImpl::getAsciiValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if (!m_key.isValid())
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if ( !m_key.getValueInfo(OUString(), &type, &size) )
        {
            if ( type == RG_VALUETYPE_STRING )
            {
                char* value = new char[size];
                if ( m_key.getValue(OUString(), (RegValue)value) )
                {
                    delete [] value;
                    throw InvalidValueException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                        (OWeakObject *)this );
                } else
                {
                    OUString ret(OStringToOUString(value, RTL_TEXTENCODING_UTF8));
                    delete [] value;
                    return ret;
                }
            }
        }

        throw InvalidValueException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setAsciiValue( const OUString& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        OString         sValue = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        sal_uInt32  size = sValue.getLength()+1;
        if ( m_key.setValue(OUString(), RG_VALUETYPE_STRING,
                            (RegValue)(sValue.getStr()), size) )
        {
            throw InvalidValueException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                (OWeakObject *)this );
        }
    }
}

//*************************************************************************
Sequence< OUString > SAL_CALL RegistryKeyImpl::getAsciiListValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if ( !m_key.getValueInfo(OUString(), &type, &size) )
        {
            if (type == RG_VALUETYPE_STRINGLIST)
            {
                RegistryValueList<char*> tmpValue;
                if ( !m_key.getStringListValue(OUString(), tmpValue) )
                {
                    Sequence<OUString> seqValue(size);

                    for (sal_uInt32 i=0; i < size; i++)
                    {
                        seqValue.getArray()[i] =
                            OStringToOUString(tmpValue.getElement(i), RTL_TEXTENCODING_UTF8);
                    }

                    return seqValue;
                }
            }
        }

        throw InvalidValueException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setAsciiListValue( const Sequence< OUString >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        sal_uInt32  length = seqValue.getLength();
        OString*    pSValue = new OString[length];
        char**      tmpValue = new char*[length];

        for (sal_uInt32 i=0; i < length; i++)
        {
            pSValue[i] = OUStringToOString(seqValue.getConstArray()[i], RTL_TEXTENCODING_UTF8);
            tmpValue[i] = (char*)pSValue[i].getStr();
        }

        if ( m_key.setStringListValue(OUString(), tmpValue, length) )
        {
            delete[] pSValue;
            delete[] tmpValue;
            throw InvalidValueException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                (OWeakObject *)this );
        }

        delete[] pSValue;
        delete[] tmpValue;
    }
}

//*************************************************************************
OUString SAL_CALL RegistryKeyImpl::getStringValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if ( !m_key.getValueInfo(OUString(), &type, &size) )
        {
            if (type == RG_VALUETYPE_UNICODE)
            {
                sal_Unicode* value = new sal_Unicode[size];
                if ( m_key.getValue(OUString(), (RegValue)value) )
                {
                    delete [] value;
                    throw InvalidValueException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                        (OWeakObject *)this );
                } else
                {
                    OUString ret(value);
                    delete [] value;
                    return ret;
                }
            }
        }

        throw InvalidValueException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setStringValue( const OUString& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        sal_uInt32 size = (value.getLength() + 1) * sizeof(sal_Unicode);
        if ( m_key.setValue(OUString(), RG_VALUETYPE_UNICODE,
                            (RegValue)(value.getStr()), size) )
        {
            throw InvalidValueException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                (OWeakObject *)this );
        }
    }
}

//*************************************************************************
Sequence< OUString > SAL_CALL RegistryKeyImpl::getStringListValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if ( !m_key.getValueInfo(OUString(), &type, &size) )
        {
            if (type == RG_VALUETYPE_UNICODELIST)
            {
                RegistryValueList<sal_Unicode*> tmpValue;
                if ( !m_key.getUnicodeListValue(OUString(), tmpValue) )
                {
                    Sequence<OUString> seqValue(size);

                    for (sal_uInt32 i=0; i < size; i++)
                    {
                        seqValue.getArray()[i] = OUString(tmpValue.getElement(i));
                    }

                    return seqValue;
                }
            }
        }

        throw InvalidValueException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setStringListValue( const Sequence< OUString >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        sal_uInt32  length = seqValue.getLength();
        sal_Unicode**   tmpValue = new sal_Unicode*[length];

        for (sal_uInt32 i=0; i < length; i++)
        {
            tmpValue[i] = (sal_Unicode*)seqValue.getConstArray()[i].getStr();
        }

        if (m_key.setUnicodeListValue(OUString(), tmpValue, length))
        {
            delete[] tmpValue;
            throw InvalidValueException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                (OWeakObject *)this );
        }

        delete[] tmpValue;
    }
}

//*************************************************************************
Sequence< sal_Int8 > SAL_CALL RegistryKeyImpl::getBinaryValue(  )
    throw(InvalidRegistryException, InvalidValueException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegValueType    type;
        sal_uInt32      size;

        if ( !m_key.getValueInfo(OUString(), &type, &size) )
        {
            if (type == RG_VALUETYPE_BINARY)
            {
                sal_Int8* value = new sal_Int8[size];
                if (m_key.getValue(OUString(), (RegValue)value))
                {
                    delete [] value;
                    throw InvalidValueException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                        (OWeakObject *)this );
                } else
                {
                    Sequence<sal_Int8> seqBytes(value, size);
                    delete [] value;
                    return seqBytes;
                }
            }
        }

        throw InvalidValueException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setBinaryValue( const Sequence< sal_Int8 >& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        sal_uInt32 size = value.getLength();
        if ( m_key.setValue(OUString(), RG_VALUETYPE_BINARY,
                            (RegValue)(value.getConstArray()), size) )
        {
            throw InvalidValueException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidValueException") ),
                (OWeakObject *)this );
        }
    }
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL RegistryKeyImpl::openKey( const OUString& aKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    RegistryKey newKey;

    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegError _ret = m_key.openKey(aKeyName, newKey);
        if ( _ret )
        {
            if ( _ret == REG_INVALID_KEY )
            {
                throw InvalidRegistryException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                    (OWeakObject *)this );
            }

            return Reference<XRegistryKey>();
        } else
        {
            return ((XRegistryKey*)new RegistryKeyImpl(newKey, m_pRegistry));
        }
    }
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL RegistryKeyImpl::createKey( const OUString& aKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    RegistryKey newKey;

    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegError _ret = m_key.createKey(aKeyName, newKey);
        if ( _ret )
        {
            if (_ret == REG_INVALID_KEY)
            {
                throw InvalidRegistryException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                    (OWeakObject *)this );
            }

            return Reference<XRegistryKey>();
        } else
        {
            return ((XRegistryKey*)new RegistryKeyImpl(newKey, m_pRegistry));
        }
    }
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::closeKey(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( m_key.isValid() )
    {
        if ( !m_key.closeKey() )
            return;
    }

    throw InvalidRegistryException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
        (OWeakObject *)this );
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::deleteKey( const OUString& rKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( m_key.isValid() )
    {
        if ( !m_key.deleteKey(rKeyName) )
            return;
    }

    throw InvalidRegistryException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
        (OWeakObject *)this );
}

//*************************************************************************
Sequence< Reference< XRegistryKey > > SAL_CALL RegistryKeyImpl::openKeys(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegistryKeyArray    subKeys;
        RegError            _ret = REG_NO_ERROR;
        if ( (_ret = m_key.openSubKeys(OUString(), subKeys)) )
        {
            if ( _ret == REG_INVALID_KEY )
            {
                throw InvalidRegistryException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                    (OWeakObject *)this );
            }

            return Sequence< Reference<XRegistryKey> >();
        } else
        {
            sal_uInt32                          length = subKeys.getLength();
            Sequence< Reference<XRegistryKey> > seqKeys(length);

            for (sal_uInt32 i=0; i < length; i++)
            {
                seqKeys.getArray()[i] =
                    (XRegistryKey*) new RegistryKeyImpl(subKeys.getElement(i), m_pRegistry);
            }
            return seqKeys;
        }
    }
}

//*************************************************************************
Sequence< OUString > SAL_CALL RegistryKeyImpl::getKeyNames(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegistryKeyNames    subKeys;
        RegError            _ret = REG_NO_ERROR;
        if ( (_ret = m_key.getKeyNames(OUString(), subKeys)) )
        {
            if ( _ret == REG_INVALID_KEY )
            {
                throw InvalidRegistryException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                    (OWeakObject *)this );
            }

            return Sequence<OUString>();
        } else
        {
            sal_uInt32          length = subKeys.getLength();
            Sequence<OUString>  seqKeys(length);

            for (sal_uInt32 i=0; i < length; i++)
            {
                seqKeys.getArray()[i] = subKeys.getElement(i);
            }
            return seqKeys;
        }
    }
}

//*************************************************************************
sal_Bool SAL_CALL RegistryKeyImpl::createLink( const OUString& aLinkName, const OUString& aLinkTarget )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegError ret = m_key.createLink(aLinkName, aLinkTarget);
        if ( ret )
        {
            if ( ret == REG_DETECT_RECURSION ||
                 ret == REG_INVALID_KEY )
            {
                throw InvalidRegistryException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                    (OWeakObject *)this );
            } else
            {
                return sal_False;
            }
        }
    }
    return sal_True;
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::deleteLink( const OUString& rLinkName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        if ( m_key.deleteLink(rLinkName) )
        {
            throw InvalidRegistryException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                (OWeakObject *)this );
        }
    }
}

//*************************************************************************
OUString SAL_CALL RegistryKeyImpl::getLinkTarget( const OUString& rLinkName )
    throw(InvalidRegistryException, RuntimeException)
{
    OUString linkTarget;

    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegError ret = m_key.getLinkTarget(rLinkName, linkTarget);
        if ( ret )
        {
            throw InvalidRegistryException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                (OWeakObject *)this );
        }
    }

    return linkTarget;
}

//*************************************************************************
OUString SAL_CALL RegistryKeyImpl::getResolvedName( const OUString& aKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    OUString resolvedName;

    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    } else
    {
        RegError ret = m_key.getResolvedKeyName(
            aKeyName, sal_True, resolvedName);
        if ( ret )
        {
            throw InvalidRegistryException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                (OWeakObject *)this );
        }
    }

    return resolvedName;
}

//*************************************************************************
SimpleRegistryImpl::SimpleRegistryImpl( const Registry& rRegistry )
    : m_registry(rRegistry)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}

//*************************************************************************
SimpleRegistryImpl::~SimpleRegistryImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

//*************************************************************************
OUString SAL_CALL SimpleRegistryImpl::getImplementationName(  )
    throw(RuntimeException)
{
    return stoc_bootstrap::simreg_getImplementationName();
}

//*************************************************************************
sal_Bool SAL_CALL SimpleRegistryImpl::supportsService( const OUString& ServiceName )
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
Sequence<OUString> SAL_CALL SimpleRegistryImpl::getSupportedServiceNames(  )
    throw(RuntimeException)
{
    return stoc_bootstrap::simreg_getSupportedServiceNames();
}

//*************************************************************************
OUString SAL_CALL SimpleRegistryImpl::getURL() throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return m_url;
}

//*************************************************************************
void SAL_CALL SimpleRegistryImpl::open( const OUString& rURL, sal_Bool bReadOnly, sal_Bool bCreate )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
    {
        m_registry.close();
    }

    RegAccessMode accessMode = REG_READWRITE;

    if ( bReadOnly )
        accessMode = REG_READONLY;

    if ( !m_registry.open(rURL, accessMode) )
    {
        m_url = rURL;
        return;
    }

    if ( bCreate )
    {
        if ( !m_registry.create(rURL) )
        {
            m_url = rURL;
            return;
        }
    }

    m_url = OUString();

    OUStringBuffer reason( 128 );
    reason.appendAscii( RTL_CONSTASCII_STRINGPARAM("Couldn't ") );
    if( bCreate )
    {
        reason.appendAscii( RTL_CONSTASCII_STRINGPARAM("create") );
    }
    else
    {
        reason.appendAscii( RTL_CONSTASCII_STRINGPARAM("open") );
    }
    reason.appendAscii( RTL_CONSTASCII_STRINGPARAM(" registry ") );
    reason.append( rURL );
    if( bReadOnly )
    {
        reason.appendAscii( RTL_CONSTASCII_STRINGPARAM(" for reading") );
    }
    else
    {
        reason.appendAscii( RTL_CONSTASCII_STRINGPARAM(" for writing" ) );
    }
    throw InvalidRegistryException( reason.makeStringAndClear() , Reference< XInterface >() );
}

//*************************************************************************
sal_Bool SAL_CALL SimpleRegistryImpl::isValid(  ) throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return m_registry.isValid();
}

//*************************************************************************
void SAL_CALL SimpleRegistryImpl::close(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
    {
        if ( !m_registry.close() )
        {
            m_url = OUString();
            return;
        }
    }

    throw InvalidRegistryException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
        (OWeakObject *)this );
}

//*************************************************************************
void SAL_CALL SimpleRegistryImpl::destroy(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
    {
        if ( !m_registry.destroy(OUString()) )
        {
            m_url = OUString();
            return;
        }
    }

    throw InvalidRegistryException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
        (OWeakObject *)this );
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL SimpleRegistryImpl::getRootKey(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
        return ((XRegistryKey*)new RegistryKeyImpl(OUString( RTL_CONSTASCII_USTRINGPARAM("/") ), this));
    else
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
sal_Bool SAL_CALL SimpleRegistryImpl::isReadOnly(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
        return m_registry.isReadOnly();
    else
    {
        throw InvalidRegistryException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
            (OWeakObject *)this );
    }
}

//*************************************************************************
void SAL_CALL SimpleRegistryImpl::mergeKey( const OUString& aKeyName, const OUString& aUrl )
    throw(InvalidRegistryException, MergeConflictException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
    {
        RegistryKey rootKey;
        if ( !m_registry.openRootKey(rootKey) )
        {
            RegError ret = m_registry.mergeKey(rootKey, aKeyName, aUrl, sal_False, sal_False);
            if (ret)
            {
                if ( ret == REG_MERGE_CONFLICT )
                    return;
                if ( ret == REG_MERGE_ERROR )
                {
                    throw MergeConflictException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("MergeConflictException") ),
                        (OWeakObject *)this );
                }
                else
                {
                    throw InvalidRegistryException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
                        (OWeakObject *)this );
                }
            }

            return;
        }
    }

    throw InvalidRegistryException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("InvalidRegistryException") ),
        (OWeakObject *)this );
}
}

namespace stoc_bootstrap
{
//*************************************************************************
Reference<XInterface> SAL_CALL SimpleRegistry_CreateInstance( const Reference<XComponentContext>& )
{
    Reference<XInterface>   xRet;

    Registry reg;

    XSimpleRegistry *pRegistry = (XSimpleRegistry*) new stoc_simreg::SimpleRegistryImpl(reg);

    if (pRegistry)
    {
        xRet = Reference<XInterface>::query(pRegistry);
    }

    return xRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
