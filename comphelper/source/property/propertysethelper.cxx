/*************************************************************************
 *
 *  $RCSfile: propertysethelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 13:32:35 $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "comphelper/propertysetinfo.hxx"
#include "comphelper/propertysethelper.hxx"

///////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace comphelper
{
class PropertySetHelperImpl
{
public:
    PropertyMapEntry* find( const OUString& aName ) const throw();

    PropertySetInfo* mpInfo;
};
}

PropertyMapEntry* PropertySetHelperImpl::find( const OUString& aName ) const throw()
{
    PropertyMap::const_iterator aIter = mpInfo->getPropertyMap()->find( aName );

    if( mpInfo->getPropertyMap()->end() != aIter )
    {
        return (*aIter).second;
    }
    else
    {
        return NULL;
    }
}

///////////////////////////////////////////////////////////////////////

PropertySetHelper::PropertySetHelper( comphelper::PropertySetInfo* pInfo ) throw()
{
    mp = new PropertySetHelperImpl;
    mp->mpInfo = pInfo;
    pInfo->acquire();
}

PropertySetHelper::~PropertySetHelper() throw()
{
    mp->mpInfo->release();
    delete mp;
}

// XPropertySet
Reference< XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo(  ) throw(RuntimeException)
{
    return mp->mpInfo;
}

void SAL_CALL PropertySetHelper::setPropertyValue( const ::rtl::OUString& aPropertyName, const Any& aValue ) throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    PropertyMapEntry* aEntries[2];
    aEntries[0] = mp->find( aPropertyName );

    if( NULL == aEntries[0] )
        throw UnknownPropertyException();

    aEntries[1] = NULL;

    _setPropertyValues( (const PropertyMapEntry**)aEntries, &aValue );
}

Any SAL_CALL PropertySetHelper::getPropertyValue( const ::rtl::OUString& PropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyMapEntry* aEntries[2];
    aEntries[0] = mp->find( PropertyName );

    if( NULL == aEntries[0] )
        throw UnknownPropertyException();

    aEntries[1] = NULL;

    Any aAny;
    _getPropertyValues( (const PropertyMapEntry**)aEntries, &aAny );

    return aAny;
}

void SAL_CALL PropertySetHelper::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // todo
}

// XMultiPropertySet
void SAL_CALL PropertySetHelper::setPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    const sal_Int32 nCount = aPropertyNames.getLength();

    if( nCount != aValues.getLength() )
        throw IllegalArgumentException();

    if( nCount )
    {
        PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];
        const OUString* pNames = aPropertyNames.getConstArray();

        sal_Bool bUnknown = sal_False;
        sal_Int32 n;
        for( n = 0; !bUnknown && ( n < nCount ); n++, pNames++ )
        {
            pEntries[n] = mp->find( *pNames );
            bUnknown = NULL == pEntries[n];
        }

        if( !bUnknown )
            _setPropertyValues( (const PropertyMapEntry**)pEntries, aValues.getConstArray() );

        delete pEntries;

        if( bUnknown )
            throw UnknownPropertyException();
    }
}

Sequence< Any > SAL_CALL PropertySetHelper::getPropertyValues( const Sequence< ::rtl::OUString >& aPropertyNames ) throw(RuntimeException)
{
    const sal_Int32 nCount = aPropertyNames.getLength();

    Sequence< Any > aValues;
    if( nCount )
    {
        PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];
        const OUString* pNames = aPropertyNames.getConstArray();

        sal_Bool bUnknown = sal_False;
        sal_Int32 n;
        for( n = 0; !bUnknown && ( n < nCount ); n++, pNames++ )
        {
            pEntries[n] = mp->find( *pNames );
            bUnknown = NULL == pEntries[n];
        }

        if( !bUnknown )
            _getPropertyValues( (const PropertyMapEntry**)pEntries, aValues.getArray() );

        delete pEntries;

        if( bUnknown )
            throw UnknownPropertyException();
    }

    return aValues;
}

void SAL_CALL PropertySetHelper::addPropertiesChangeListener( const Sequence< ::rtl::OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    // todo
}

void SAL_CALL PropertySetHelper::firePropertiesChangeEvent( const Sequence< ::rtl::OUString >& aPropertyNames, const Reference< XPropertiesChangeListener >& xListener ) throw(RuntimeException)
{
    // todo
}

// XPropertyState
PropertyState SAL_CALL PropertySetHelper::getPropertyState( const ::rtl::OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    PropertyMapEntry* aEntries[2];

    aEntries[0] = mp->find( PropertyName );
    if( aEntries[0] == NULL )
        throw UnknownPropertyException();

    aEntries[1] = NULL;

    PropertyState aState;
    _getPropertyStates( (const PropertyMapEntry**)aEntries, &aState );

    return aState;
}

Sequence< PropertyState > SAL_CALL PropertySetHelper::getPropertyStates( const Sequence< ::rtl::OUString >& aPropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    const sal_Int32 nCount = aPropertyName.getLength();

    Sequence< PropertyState > aStates( nCount );

    if( nCount )
    {
        const OUString* pNames = aPropertyName.getConstArray();

        sal_Bool bUnknown = sal_False;

        PropertyMapEntry** pEntries = new PropertyMapEntry*[nCount+1];

        sal_Int32 n;
        for( n = 0; !bUnknown && (n < nCount); n++, pNames++ )
        {
            pEntries[n] = mp->find( *pNames );
            bUnknown = NULL == pEntries[n];
        }

        pEntries[nCount] = NULL;

        if( !bUnknown )
            _getPropertyStates( (const PropertyMapEntry**)pEntries, aStates.getArray() );

        delete pEntries;

        if( bUnknown )
            throw UnknownPropertyException();
    }

    return aStates;
}

void SAL_CALL PropertySetHelper::setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(UnknownPropertyException, RuntimeException)
{
    PropertyMapEntry *pEntry  = mp->find( PropertyName );
    if( NULL == pEntry )
        throw UnknownPropertyException();

    _setPropertyToDefault( pEntry );
}

Any SAL_CALL PropertySetHelper::getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    PropertyMapEntry* pEntry = mp->find( aPropertyName );
    if( NULL == pEntry )
        throw UnknownPropertyException();

    return _getPropertyDefault( pEntry );
}

void PropertySetHelper::_getPropertyStates( const comphelper::PropertyMapEntry** ppEntries, PropertyState* pStates ) throw(UnknownPropertyException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");
}

void PropertySetHelper::_setPropertyToDefault( const comphelper::PropertyMapEntry* pEntry )  throw(UnknownPropertyException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");
}

Any PropertySetHelper::_getPropertyDefault( const comphelper::PropertyMapEntry* pEntry ) throw(UnknownPropertyException, WrappedTargetException )
{
    OSL_ENSURE( sal_False, "you have to implement this yourself!");

    Any aAny;
    return aAny;
}
