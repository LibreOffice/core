/*************************************************************************
 *
 *  $RCSfile: propertysetinfo.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-14 15:55:13 $
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

#include "comphelper/propertysetinfo.hxx"

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace comphelper
{
class PropertyMapImpl
{
public:
    PropertyMapImpl() throw();
    virtual ~PropertyMapImpl() throw();

    void add( PropertyMapEntry* pMap ) throw();
    void remove( const OUString& aName ) throw();

    Sequence< Property > getProperties() throw();

    const PropertyMap* getPropertyMap() const throw();

    Property getPropertyByName( const OUString& aName ) throw( UnknownPropertyException );
    sal_Bool hasPropertyByName( const OUString& aName ) throw();

private:
    PropertyMap maPropertyMap;
    Sequence< Property > maProperties;
};
}

PropertyMapImpl::PropertyMapImpl() throw()
{
}

PropertyMapImpl::~PropertyMapImpl() throw()
{
}

void PropertyMapImpl::add( PropertyMapEntry* pMap ) throw()
{
    while( pMap->mpName )
    {
        OUString aName( pMap->mpName, pMap->mnNameLen, RTL_TEXTENCODING_ASCII_US );

#ifndef PRODUCT
        PropertyMap::iterator aIter = maPropertyMap.find( aName );
        if( aIter != maPropertyMap.end() )
        {
            OSL_ENSHURE( sal_False, "Warning: PropertyMapEntry added twice, possible error!");
        }
#endif
        if( NULL == pMap->mpType )
        {
            OSL_ENSHURE( sal_False, "No type in PropertyMapEntry!");
            pMap->mpType = &::getCppuType((const sal_Int32*)0);
        }

        maPropertyMap[aName] = pMap;

        if( maProperties.getLength() )
            maProperties.realloc( 0 );

        pMap = &pMap[1];
    }
}

void PropertyMapImpl::remove( const OUString& aName ) throw()
{
    maPropertyMap.erase( aName );

    if( maProperties.getLength() )
        maProperties.realloc( 0 );
}

Sequence< Property > PropertyMapImpl::getProperties() throw()
{
    // maybe we have to generate the properties after
    // a change in the property map or at first call
    // to getProperties
    if( maProperties.getLength() != (sal_Int32)maPropertyMap.size() )
    {
        maProperties = Sequence< Property >( maPropertyMap.size() );
        Property* pProperties = maProperties.getArray();

        PropertyMap::iterator aIter = maPropertyMap.begin();
        const PropertyMap::iterator aEnd = maPropertyMap.end();
        while( aIter != aEnd )
        {
            PropertyMapEntry* pEntry = (*aIter).second;

            pProperties->Name = OUString( pEntry->mpName, pEntry->mnNameLen, RTL_TEXTENCODING_ASCII_US );
            pProperties->Handle = pEntry->mnHandle;
            pProperties->Type = *pEntry->mpType;
            pProperties->Attributes = pEntry->mnAttributes;

            pProperties++;
            aIter++;
        }
    }

    return maProperties;
}

const PropertyMap* PropertyMapImpl::getPropertyMap() const throw()
{
    return &maPropertyMap;
}

Property PropertyMapImpl::getPropertyByName( const OUString& aName ) throw( UnknownPropertyException )
{
    PropertyMap::iterator aIter = maPropertyMap.find( aName );

    if( maPropertyMap.end() == aIter )
        throw UnknownPropertyException();

    PropertyMapEntry* pEntry = (*aIter).second;

    return Property( aName, pEntry->mnHandle, *pEntry->mpType, pEntry->mnAttributes );
}

sal_Bool PropertyMapImpl::hasPropertyByName( const OUString& aName ) throw()
{
    return maPropertyMap.find( aName ) != maPropertyMap.end();
}

///////////////////////////////////////////////////////////////////////

PropertySetInfo::PropertySetInfo() throw()
{
    mpMap = new PropertyMapImpl();
}

PropertySetInfo::PropertySetInfo( PropertyMapEntry* pMap ) throw()
{
    mpMap = new PropertyMapImpl();
    mpMap->add( pMap );
}

PropertySetInfo::~PropertySetInfo() throw()
{
    delete mpMap;
}

void PropertySetInfo::add( PropertyMapEntry* pMap ) throw()
{
    mpMap->add( pMap );
}

void PropertySetInfo::remove( const rtl::OUString& aName ) throw()
{
    mpMap->remove( aName );
}

Sequence< ::com::sun::star::beans::Property > SAL_CALL PropertySetInfo::getProperties() throw(::com::sun::star::uno::RuntimeException)
{
    return mpMap->getProperties();
}

Property SAL_CALL PropertySetInfo::getPropertyByName( const ::rtl::OUString& aName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    return mpMap->getPropertyByName( aName );
}

sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::RuntimeException)
{
    return mpMap->hasPropertyByName( Name );
}

const PropertyMap* PropertySetInfo::getPropertyMap() const throw()
{
    return mpMap->getPropertyMap();
}
