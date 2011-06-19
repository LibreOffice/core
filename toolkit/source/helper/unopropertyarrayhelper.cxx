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
#include "precompiled_toolkit.hxx"

#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>

//  ----------------------------------------------------
//  class UnoPropertyArrayHelper
//  ----------------------------------------------------

UnoPropertyArrayHelper::UnoPropertyArrayHelper( const ::com::sun::star::uno::Sequence<sal_Int32>& rIDs )
{
    sal_Int32 nIDs = rIDs.getLength();
    const sal_Int32* pIDs = rIDs.getConstArray();
    for ( sal_Int32 n = 0; n < nIDs; n++ )
        maIDs.Insert( pIDs[n], (void*)1L );
}

UnoPropertyArrayHelper::UnoPropertyArrayHelper( const std::list< sal_uInt16 > &rIDs )
{
    std::list< sal_uInt16 >::const_iterator iter;
    for( iter = rIDs.begin(); iter != rIDs.end(); ++iter)
      maIDs.Insert( *iter, (void*)1L);
}

sal_Bool UnoPropertyArrayHelper::ImplHasProperty( sal_uInt16 nPropId ) const
{
    if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
        nPropId = BASEPROPERTY_FONTDESCRIPTOR;

    return maIDs.Get( nPropId ) ? sal_True : sal_False;
}

// ::cppu::IPropertyArrayHelper
sal_Bool UnoPropertyArrayHelper::fillPropertyMembersByHandle( ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nPropId )
{
    sal_uInt16 id = sal::static_int_cast< sal_uInt16 >(nPropId);
    sal_Bool bValid = ImplHasProperty( id );
    if ( bValid )
    {
        if ( pPropName )
            *pPropName = GetPropertyName( id );
        if ( pAttributes )
            *pAttributes = GetPropertyAttribs( id );
    }
    return bValid;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > UnoPropertyArrayHelper::getProperties()
{
    // Sortiert nach Namen...

    Table aSortedPropsIds;
    sal_uInt32 nProps = maIDs.Count();
    for ( sal_uInt32 s = 0; s < nProps; s++ )
    {
        sal_uInt16 nId = sal::static_int_cast< sal_uInt16 >(
            maIDs.GetObjectKey( s ));
        aSortedPropsIds.Insert( 1+GetPropertyOrderNr( nId ), (void*)(sal_uIntPtr)nId );

        if ( nId == BASEPROPERTY_FONTDESCRIPTOR )
        {
            // Einzelproperties...
            for ( sal_uInt16 i = BASEPROPERTY_FONTDESCRIPTORPART_START; i <= BASEPROPERTY_FONTDESCRIPTORPART_END; i++ )
                aSortedPropsIds.Insert( 1+GetPropertyOrderNr( i ), (void*)(sal_uIntPtr)i );
        }
    }

    nProps = aSortedPropsIds.Count();   // koennen jetzt mehr sein
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps( nProps );
    ::com::sun::star::beans::Property* pProps = aProps.getArray();

    for ( sal_uInt32 n = 0; n < nProps; n++ )
    {
        sal_uInt16 nId = (sal_uInt16)(sal_uLong)aSortedPropsIds.GetObject( n );
        pProps[n].Name = GetPropertyName( nId );
        pProps[n].Handle = nId;
        pProps[n].Type = *GetPropertyType( nId );
        pProps[n].Attributes = GetPropertyAttribs( nId );
    }

    return aProps;
}

::com::sun::star::beans::Property UnoPropertyArrayHelper::getPropertyByName(const ::rtl::OUString& rPropertyName) throw (::com::sun::star::beans::UnknownPropertyException)
{
    ::com::sun::star::beans::Property aProp;
    sal_uInt16 nId = GetPropertyId( rPropertyName );
    if ( ImplHasProperty( nId ) )
    {
        aProp.Name = rPropertyName;
        aProp.Handle = -1;
        aProp.Type = *GetPropertyType( nId );
        aProp.Attributes = GetPropertyAttribs( nId );
    }

    return aProp;
}

sal_Bool UnoPropertyArrayHelper::hasPropertyByName(const ::rtl::OUString& rPropertyName)
{
    return ImplHasProperty( GetPropertyId( rPropertyName ) );
}

sal_Int32 UnoPropertyArrayHelper::getHandleByName( const ::rtl::OUString & rPropertyName )
{
    sal_Int32 nId = (sal_Int32 ) GetPropertyId( rPropertyName );
    return nId ? nId : (-1);
}

sal_Int32 UnoPropertyArrayHelper::fillHandles( sal_Int32* pHandles, const ::com::sun::star::uno::Sequence< ::rtl::OUString > & rPropNames )
{
    const ::rtl::OUString* pNames = rPropNames.getConstArray();
    sal_Int32 nValues = rPropNames.getLength();
    sal_Int32 nValidHandles = 0;

    for ( sal_Int32 n = 0; n < nValues; n++ )
    {
        sal_uInt16 nPropId = GetPropertyId( pNames[n] );
        if ( nPropId && ImplHasProperty( nPropId ) )
        {
            pHandles[n] = nPropId;
            nValidHandles++;
        }
        else
        {
            pHandles[n] = -1;
        }
    }
    return nValidHandles;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
