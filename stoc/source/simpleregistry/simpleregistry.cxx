/*************************************************************************
 *
 *  $RCSfile: simpleregistry.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 15:37:29 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
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
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _REGISTRY_REGISTRY_HXX_
#include <registry/registry.hxx>
#endif

#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace cppu;
using namespace osl;
using namespace rtl;

#define SERVICENAME "com.sun.star.registry.SimpleRegistry"
#define IMPLNAME    "com.sun.star.comp.stoc.SimpleRegistry"

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
    SimpleRegistryImpl( const Reference<XMultiServiceFactory> & rXSMgr,
                        const Registry& rRegistry );

    ~SimpleRegistryImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  );

    // XSimpleRegistry
    virtual OUString SAL_CALL getURL() throw(RuntimeException);
    virtual void SAL_CALL open( const OUString& rURL, sal_Bool bReadOnly, sal_Bool bCreate ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  ) throw(RuntimeException);
    virtual void SAL_CALL close(  ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL destroy(  ) throw(InvalidRegistryException, RuntimeException);
    virtual Reference< XRegistryKey > SAL_CALL getRootKey(  ) throw(InvalidRegistryException, RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(InvalidRegistryException, RuntimeException);
    virtual void SAL_CALL mergeKey( const OUString& aKeyName, const OUString& aUrl ) throw(InvalidRegistryException, MergeConflictException, RuntimeException);

    friend RegistryKeyImpl;
protected:
    Mutex       m_mutex;
    OUString    m_url;
    Registry    m_registry;

    Reference<XMultiServiceFactory> m_xSMgr;
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
        throw InvalidRegistryException();
    } else
    {
        if ( rootKey.openKey(rKeyName, m_key) )
        {
            throw InvalidRegistryException();
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
        throw InvalidRegistryException();
    }

    return sal_False;
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
                    break;
                case RG_LINKTYPE:
                    return RegistryKeyType_LINK;
                    break;
            }
        } else
        {
            throw InvalidRegistryException();
        }
    } else
    {
        throw InvalidRegistryException();
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
        throw InvalidRegistryException();
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
        throw InvalidRegistryException();
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

        throw InvalidValueException();
    }

    return 0;
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setLongValue( sal_Int32 value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        if (m_key.setValue(OUString(), RG_VALUETYPE_LONG, &value, sizeof(sal_Int32)))
        {
            throw InvalidValueException();
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
        throw InvalidRegistryException();
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

        throw InvalidValueException();
    }

    return Sequence<sal_Int32>();
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setLongListValue( const Sequence< sal_Int32 >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
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
            throw InvalidValueException();
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
        throw InvalidRegistryException();
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
                    delete(value);
                    throw InvalidValueException();
                } else
                {
                    OUString ret(OStringToOUString(value, RTL_TEXTENCODING_UTF8));
                    delete(value);
                    return ret;
                }
            }
        }

        throw InvalidValueException();
    }

    return OUString();
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setAsciiValue( const OUString& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        OString         sValue = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        sal_uInt32  size = sValue.getLength()+1;
        if ( m_key.setValue(OUString(), RG_VALUETYPE_STRING,
                            (RegValue)(sValue.getStr()), size) )
        {
            throw InvalidValueException();
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
        throw InvalidRegistryException();
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

        throw InvalidValueException();
    }

    return Sequence<OUString>();
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setAsciiListValue( const Sequence< OUString >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
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
            throw InvalidValueException();
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
        throw InvalidRegistryException();
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
                    delete(value);
                    throw InvalidValueException();
                } else
                {
                    OUString ret(value);
                    delete(value);
                    return ret;
                }
            }
        }

        throw InvalidValueException();
    }

    return OUString();
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setStringValue( const OUString& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        sal_uInt32 size = (value.getLength() + 1) * sizeof(sal_Unicode);
        if ( m_key.setValue(OUString(), RG_VALUETYPE_UNICODE,
                            (RegValue)(value.getStr()), size) )
        {
            throw InvalidValueException();
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
        throw InvalidRegistryException();
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

        throw InvalidValueException();
    }

    return Sequence<OUString>();
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setStringListValue( const Sequence< OUString >& seqValue )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
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
            throw InvalidValueException();
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
        throw InvalidRegistryException();
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
                    delete(value);
                    throw InvalidValueException();
                } else
                {
                    Sequence<sal_Int8> seqBytes(value, size);
                    delete(value);
                    return seqBytes;
                }
            }
        }

        throw InvalidValueException();
    }

    return Sequence< sal_Int8 >();
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::setBinaryValue( const Sequence< sal_Int8 >& value )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        sal_uInt32 size = value.getLength();
        if ( m_key.setValue(OUString(), RG_VALUETYPE_BINARY,
                            (RegValue)(value.getConstArray()), size) )
        {
            throw InvalidValueException();
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
        throw InvalidRegistryException();
    } else
    {
        RegError _ret = REG_NO_ERROR;
        if ( _ret = m_key.openKey(aKeyName, newKey) )
        {
            if ( _ret == REG_INVALID_KEY )
                throw InvalidRegistryException();

            return Reference<XRegistryKey>();
        } else
        {
            return ((XRegistryKey*)new RegistryKeyImpl(newKey, m_pRegistry));
        }
    }

    return Reference<XRegistryKey>();
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL RegistryKeyImpl::createKey( const OUString& aKeyName )
    throw(InvalidRegistryException, RuntimeException)
{
    RegistryKey newKey;

    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        RegError _ret = REG_NO_ERROR;
        if ( _ret = m_key.createKey(aKeyName, newKey) )
        {
            if (_ret == REG_INVALID_KEY)
                throw InvalidRegistryException();

            return Reference<XRegistryKey>();
        } else
        {
            return ((XRegistryKey*)new RegistryKeyImpl(newKey, m_pRegistry));
        }
    }

    return Reference<XRegistryKey>();
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

    throw InvalidRegistryException();
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

    throw InvalidRegistryException();
}

//*************************************************************************
Sequence< Reference< XRegistryKey > > SAL_CALL RegistryKeyImpl::openKeys(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        RegistryKeyArray    subKeys;
        RegError            _ret = REG_NO_ERROR;
        if ( _ret = m_key.openSubKeys(OUString(), subKeys) )
        {
            if ( _ret == REG_INVALID_KEY )
                throw InvalidRegistryException();

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

    return Sequence< Reference<XRegistryKey> >();
}

//*************************************************************************
Sequence< OUString > SAL_CALL RegistryKeyImpl::getKeyNames(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        RegistryKeyNames    subKeys;
        RegError            _ret = REG_NO_ERROR;
        if ( _ret = m_key.getKeyNames(OUString(), subKeys) )
        {
            if ( _ret == REG_INVALID_KEY )
                throw InvalidRegistryException();

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

    return Sequence<OUString>();
}

//*************************************************************************
sal_Bool SAL_CALL RegistryKeyImpl::createLink( const OUString& aLinkName, const OUString& aLinkTarget )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        RegError ret = REG_NO_ERROR;

        if ( ret = m_key.createLink(aLinkName, aLinkTarget) )
        {
            if ( ret == REG_DETECT_RECURSION ||
                 ret == REG_INVALID_KEY )
            {
                throw InvalidRegistryException();
            } else
                return sal_False;
        } else
        {
            return sal_True;
        }
    }

    return sal_False;
}

//*************************************************************************
void SAL_CALL RegistryKeyImpl::deleteLink( const OUString& rLinkName )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_pRegistry->m_mutex );
    if ( !m_key.isValid() )
    {
        throw InvalidRegistryException();
    } else
    {
        if ( m_key.deleteLink(rLinkName) )
        {
            throw InvalidRegistryException();
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
        throw InvalidRegistryException();
    } else
    {
        RegError    ret = REG_NO_ERROR;

        if ( ret = m_key.getLinkTarget(rLinkName, linkTarget) )
        {
            throw InvalidRegistryException();
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
        throw InvalidRegistryException();
    } else
    {
        RegError    ret = REG_NO_ERROR;

        if ( ret = m_key.getResolvedKeyName(aKeyName, sal_True, resolvedName) )
        {
            throw InvalidRegistryException();
        }
    }

    return resolvedName;
}

//*************************************************************************
SimpleRegistryImpl::SimpleRegistryImpl( const Reference<XMultiServiceFactory> & rXSMgr,
                                        const Registry& rRegistry )
    : m_xSMgr(rXSMgr)
    , m_registry(rRegistry)
{
}

//*************************************************************************
SimpleRegistryImpl::~SimpleRegistryImpl()
{
}

//*************************************************************************
OUString SAL_CALL SimpleRegistryImpl::getImplementationName(  )
    throw(RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
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
    Guard< Mutex > aGuard( m_mutex );
    return getSupportedServiceNames_Static();
}

//*************************************************************************
Sequence<OUString> SAL_CALL SimpleRegistryImpl::getSupportedServiceNames_Static(  )
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
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
    throw InvalidRegistryException();
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

    throw InvalidRegistryException();
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

    throw InvalidRegistryException();
}

//*************************************************************************
Reference< XRegistryKey > SAL_CALL SimpleRegistryImpl::getRootKey(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
        return ((XRegistryKey*)new RegistryKeyImpl(OUString( RTL_CONSTASCII_USTRINGPARAM("/") ), this));
    else
        throw InvalidRegistryException();

    return Reference< XRegistryKey >();
}

//*************************************************************************
sal_Bool SAL_CALL SimpleRegistryImpl::isReadOnly(  )
    throw(InvalidRegistryException, RuntimeException)
{
    Guard< Mutex > aGuard( m_mutex );
    if ( m_registry.isValid() )
        return m_registry.isReadOnly();
    else
        throw InvalidRegistryException();

    return sal_False;
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
            RegError ret;
            if (ret = m_registry.mergeKey(rootKey, aKeyName, aUrl, sal_False, sal_False))
            {
                if ( ret == REG_MERGE_ERROR )
                    throw MergeConflictException();
                else
                    throw InvalidRegistryException();
            }

            return;
        }
    }

    throw InvalidRegistryException();
}

//*************************************************************************
Reference<XInterface> SAL_CALL SimpleRegistry_CreateInstance( const Reference<XMultiServiceFactory>& rSMgr )
{
    Reference<XInterface>   xRet;
    RegistryLoader          aLoader;

    if ( aLoader.isLoaded() )
    {
        Registry reg(aLoader);

        XSimpleRegistry *pRegistry = (XSimpleRegistry*) new SimpleRegistryImpl(rSMgr, reg);

        if (pRegistry)
        {
            xRet = Reference<XInterface>::query(pRegistry);
        }
    }

    return xRet;
}

}


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
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL =
                ::stoc_simreg::SimpleRegistryImpl::getSupportedServiceNames_Static();
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

    if (rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            ::stoc_simreg::SimpleRegistry_CreateInstance,
            ::stoc_simreg::SimpleRegistryImpl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}



