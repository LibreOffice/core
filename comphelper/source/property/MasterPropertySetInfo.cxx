/*************************************************************************
 *
 *  $RCSfile: MasterPropertySetInfo.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2001-07-24 20:37:41 $
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
#ifndef _COMPHELPER_CHAINABLEPROPERTYSETINFO_HXX_
#include <comphelper/MasterPropertySetInfo.hxx>
#endif
#ifndef _COMPHELPER_TYPEGENERATION_HXX_
#include <comphelper/TypeGeneration.hxx>
#endif

using ::rtl::OUString;
using ::comphelper::PropertyInfo;
using ::comphelper::GenerateCppuType;
using ::comphelper::MasterPropertySetInfo;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::XWeak;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::beans::Property;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::beans::UnknownPropertyException;

MasterPropertySetInfo::MasterPropertySetInfo()
    throw()
{
}

MasterPropertySetInfo::MasterPropertySetInfo( PropertyInfo* pMap )
    throw()
{
    add ( pMap );
}

MasterPropertySetInfo::~MasterPropertySetInfo()
    throw()
{
    PropertyDataHash::iterator aEnd = maMap.end(), aIter = maMap.begin();
    while (aIter != aEnd )
    {
        delete (*aIter).second;
        aIter++;
    }
}

//XInterface
Any SAL_CALL MasterPropertySetInfo::queryInterface( const Type& rType )
    throw(RuntimeException)
{
    return ::cppu::queryInterface ( rType                                       ,
                                    // OWeakObject interfaces
                                    reinterpret_cast< XInterface*       > ( this )  ,
                                    static_cast < XWeak*                > ( this )  ,
                                    // my own interfaces
                                    static_cast < XPropertySetInfo*     > ( this ) );
}

void SAL_CALL MasterPropertySetInfo::acquire(  )
    throw()
{
    OWeakObject::acquire();
}
void SAL_CALL MasterPropertySetInfo::release(  )
    throw()
{
    OWeakObject::release();
}

void MasterPropertySetInfo::add( PropertyInfo* pMap, sal_Int32 nCount, sal_uInt8 nMapId )
    throw()
{
    // nCount < 0   => add all
    // nCount == 0  => add nothing
    // nCount > 0   => add at most nCount entries
    if( maProperties.getLength() )
        maProperties.realloc( 0 );

    for ( ; pMap->mpName && ( ( nCount < 0 ) || ( nCount > 0 ) ); --nCount, ++pMap )
    {
        OUString aName( pMap->mpName, pMap->mnNameLen, RTL_TEXTENCODING_ASCII_US );

#ifndef PRODUCT
        PropertyDataHash::iterator aIter = maMap.find( aName );
        if( aIter != maMap.end() )
            OSL_ENSURE( sal_False, "Warning: PropertyInfo added twice, possible error!");
#endif
        maMap[aName] = new PropertyData ( nMapId, pMap );
    }
}

void MasterPropertySetInfo::add( PropertyInfoHash &rHash, sal_uInt8 nMapId )
    throw()
{
    if( maProperties.getLength() )
        maProperties.realloc( 0 );
    PropertyInfoHash::const_iterator aIter = rHash.begin(), aEnd = rHash.end();

    while ( aIter != aEnd )
    {
#ifndef PRODUCT
        PropertyDataHash::iterator aDebugIter = maMap.find( (*aIter).first );
        if( aDebugIter != maMap.end() )
            OSL_ENSURE( sal_False, "Warning: PropertyInfo added twice, possible error!");
#endif
        maMap[(*aIter).first] = new PropertyData ( nMapId, (*aIter).second );
        aIter++;
    }
}

void MasterPropertySetInfo::remove( const rtl::OUString& aName )
    throw()
{
    maMap.erase ( aName );
    if ( maProperties.getLength() )
         maProperties.realloc( 0 );
}

Sequence< ::Property > SAL_CALL MasterPropertySetInfo::getProperties()
    throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 nSize = maMap.size();
    if( maProperties.getLength() != nSize )
    {
        maProperties.realloc ( nSize );
        Property* pProperties = maProperties.getArray();

        PropertyDataHash::iterator aIter = maMap.begin();
        const PropertyDataHash::iterator aEnd = maMap.end();
        for ( ; aIter != aEnd; ++aIter, ++pProperties)
        {
            PropertyInfo* pInfo = (*aIter).second->mpInfo;

            pProperties->Name = OUString( pInfo->mpName, pInfo->mnNameLen, RTL_TEXTENCODING_ASCII_US );
            pProperties->Handle = pInfo->mnHandle;
            const Type* pType = &pProperties->Type;
            GenerateCppuType ( pInfo->meCppuType, pType);
            pProperties->Attributes = pInfo->mnAttributes;
        }
    }
    return maProperties;
}

Property SAL_CALL MasterPropertySetInfo::getPropertyByName( const ::rtl::OUString& rName )
    throw(::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    PropertyDataHash::const_iterator aIter = maMap.find( rName );

    if ( maMap.end() == aIter )
        throw UnknownPropertyException();

    PropertyInfo *pInfo = (*aIter).second->mpInfo;
    Property aProperty;
    aProperty.Name   = OUString( pInfo->mpName, pInfo->mnNameLen, RTL_TEXTENCODING_ASCII_US );
    aProperty.Handle = pInfo->mnHandle;
    const Type* pType = &aProperty.Type;
    GenerateCppuType ( pInfo->meCppuType, pType );
    aProperty.Attributes = pInfo->mnAttributes;
    return aProperty;
}

sal_Bool SAL_CALL MasterPropertySetInfo::hasPropertyByName( const ::rtl::OUString& rName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return static_cast < sal_Bool > ( maMap.find ( rName ) != maMap.end() );
}
