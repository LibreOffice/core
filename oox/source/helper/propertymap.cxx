/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertymap.cxx,v $
 * $Revision: 1.3 $
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

#include "oox/helper/propertymap.hxx"

#include <string>
#include <stdio.h>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase2.hxx>

using ::rtl::OUString;
using ::osl::MutexGuard;
using ::osl::Mutex;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace oox {

bool PropertyMap::hasProperty( const ::rtl::OUString& rName ) const
{
    return find( rName ) != end();
}

const Any* PropertyMap::getPropertyValue( const ::rtl::OUString& rName ) const
{
    PropertyMapBase::const_iterator aIter( find( rName ) );
    return aIter != end() ? &((*aIter).second) : NULL;
}

void PropertyMap::makeSequence( Sequence< NamedValue >& rSequence ) const
{
    rSequence.realloc( size() );
    NamedValue* pValues = rSequence.getArray();
    const_iterator aIter( begin() );
    const_iterator aEnd( end() );

    for( ; aIter != aEnd; aIter++, pValues++ )
    {
        pValues->Name = (*aIter).first;
        pValues->Value = (*aIter).second;
    }
}


void PropertyMap::makeSequence( Sequence< PropertyValue >& rSequence ) const
{
    rSequence.realloc( size() );
    PropertyValue* pValues = rSequence.getArray();
    const_iterator aIter( begin() );
    const_iterator aEnd( end() );

    for( ; aIter != aEnd; aIter++, pValues++ )
    {
        pValues->Name = (*aIter).first;
        pValues->Value = (*aIter).second;
        pValues->State = PropertyState_DIRECT_VALUE;
    }
}

void PropertyMap::makeSequence( Sequence< OUString >& rNames, Sequence< Any >& rValues ) const
{
    rNames.realloc( size() );
    rValues.realloc( size() );
    OUString* pNames = rNames.getArray();
    Any* pValues = rValues.getArray();
    const_iterator aIter( begin() );
    const_iterator aEnd( end() );

    for( ; aIter != aEnd; aIter++, pNames++, pValues++ )
    {
        *pNames = (*aIter).first;
        *pValues = (*aIter).second;
    }
}


void PropertyMap::dump_debug(const char *pMessage)
{
    const_iterator aIter( begin() );
    const_iterator aEnd( end() );

    if( pMessage != NULL)
    {
        OSL_TRACE("OOX: %s", pMessage);
    }

    if(aIter == aEnd)
    {
        OSL_TRACE("OOX: Properties empty");
        return;
    }

    OSL_TRACE("OOX: Properties");

    for( ; aIter != aEnd; aIter++ )
    {
        std::string value;
        const Any & any = (*aIter).second;
        try {
            char buffer[256];
            if(!any.hasValue() )
            {
                value = "*empty*";
            }
            else if(any.has<OUString>() )
            {
                OUString aStr;
                any >>= aStr;
                value = OUStringToOString( aStr, RTL_TEXTENCODING_ASCII_US ).getStr();
            }
            else if(any.has<sal_Int16>())
            {
                sal_Int16 v = 0;
                any >>= v;
                sprintf(buffer, "%d", (int)v);
                value = buffer;
            }
            else if(any.has<sal_Int32>())
            {
                sal_Int32 v = 0;
                any >>= v;
                sprintf(buffer, "%d", (int)v);
                value = buffer;
            }
            else if(any.has<sal_Bool>())
            {
                sal_Bool v = sal_False;
                any >>= v;
                sprintf(buffer, "%d", (int)v);
                value = buffer;
            }
            else
            {
                value = "contains a: ";
                value += OUStringToOString(any.getValueTypeName(), RTL_TEXTENCODING_ASCII_US ).getStr();
            }
        }
        catch( ... )
        {
            value = "unable to convert from ";
            value += OUStringToOString(any.getValueTypeName(), RTL_TEXTENCODING_ASCII_US ).getStr();
        }
        OSL_TRACE("OOX: -> %s = %s", OUStringToOString( (*aIter).first, RTL_TEXTENCODING_ASCII_US ).getStr(),
                            value.c_str());
    }
}

/** this class implements a generic XPropertySet
    Properties of all names and types can be set and later retrieved
    TODO: move this to comphelper or better find an existing implementation */
class GenericPropertySet : public ::cppu::WeakImplHelper2< XPropertySet, XPropertySetInfo >,
                           private PropertyMapBase,
                           private Mutex
{
public:
    GenericPropertySet();
    GenericPropertySet( const PropertyMapBase& rProperties );

    // XPropertySet
    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);
    virtual Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException);

    // XPropertySetInfo
    virtual Sequence< Property > SAL_CALL getProperties(  ) throw (RuntimeException);
    virtual Property SAL_CALL getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException);
    virtual ::sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw (RuntimeException);
};

GenericPropertySet::GenericPropertySet()
{
}

GenericPropertySet::GenericPropertySet( const PropertyMapBase& rProperties )
: PropertyMapBase( rProperties )
{
}

Reference< XPropertySetInfo > SAL_CALL GenericPropertySet::getPropertySetInfo() throw (RuntimeException)
{
    return this;
}

void SAL_CALL GenericPropertySet::setPropertyValue( const OUString& aPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( *this );
    (*this)[ aPropertyName ] = aValue;
}

Any SAL_CALL GenericPropertySet::getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    iterator aIter( find( PropertyName ) );
    if( aIter == end() )
        throw UnknownPropertyException();

    return (*aIter).second;
}

void SAL_CALL GenericPropertySet::addPropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL GenericPropertySet::removePropertyChangeListener( const OUString& , const Reference< XPropertyChangeListener >&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL GenericPropertySet::addVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}
void SAL_CALL GenericPropertySet::removeVetoableChangeListener( const OUString& , const Reference< XVetoableChangeListener >&  ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException) {}

// XPropertySetInfo
Sequence< Property > SAL_CALL GenericPropertySet::getProperties(  ) throw (RuntimeException)
{
    Sequence< Property > aRet( size() );
    Property* pProperty = aRet.getArray();

    for( iterator aIter = begin(); aIter != end(); aIter++, pProperty++ )
    {
        pProperty->Name = (*aIter).first;
        pProperty->Handle = 0;
        pProperty->Type = (*aIter).second.getValueType();
        pProperty->Attributes = 0;
    }

    return aRet;
}

Property SAL_CALL GenericPropertySet::getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException)
{
    iterator aIter( find( aName ) );
    if( aIter == end() )
        throw UnknownPropertyException();

    Property aProperty;
    aProperty.Name = (*aIter).first;
    aProperty.Handle = 0;
    aProperty.Type = (*aIter).second.getValueType();
    aProperty.Attributes = 0;

    return aProperty;
}

::sal_Bool SAL_CALL GenericPropertySet::hasPropertyByName( const OUString& Name ) throw (RuntimeException)
{
    return find( Name ) != end();
}


Reference< XPropertySet > PropertyMap::makePropertySet() const
{
    Reference< XPropertySet > xSet( new GenericPropertySet(*this) );
    return xSet;
}

}

