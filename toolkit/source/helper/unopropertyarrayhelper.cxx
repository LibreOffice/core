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


#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/property.hxx>
#include <map>

//  ----------------------------------------------------
//  class UnoPropertyArrayHelper
//  ----------------------------------------------------

UnoPropertyArrayHelper::UnoPropertyArrayHelper( const ::com::sun::star::uno::Sequence<sal_Int32>& rIDs )
{
    sal_Int32 nIDs = rIDs.getLength();
    const sal_Int32* pIDs = rIDs.getConstArray();
    for ( sal_Int32 n = 0; n < nIDs; n++ )
        maIDs.insert( pIDs[n] );
}

UnoPropertyArrayHelper::UnoPropertyArrayHelper( const std::list< sal_uInt16 > &rIDs )
{
    std::list< sal_uInt16 >::const_iterator iter;
    for( iter = rIDs.begin(); iter != rIDs.end(); ++iter)
      maIDs.insert( *iter );
}

sal_Bool UnoPropertyArrayHelper::ImplHasProperty( sal_uInt16 nPropId ) const
{
    if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
        nPropId = BASEPROPERTY_FONTDESCRIPTOR;

    return maIDs.find( nPropId ) != maIDs.end() ? sal_True : sal_False;
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

    std::map<sal_Int32, sal_uInt16> aSortedPropsIds;
    for( std::set<sal_Int32>::const_iterator it =  maIDs.begin(); it != maIDs.end(); ++it)
    {
        sal_uInt16 nId = sal::static_int_cast< sal_uInt16 >(*it);
        aSortedPropsIds[ 1+GetPropertyOrderNr( nId ) ] = nId;

        if ( nId == BASEPROPERTY_FONTDESCRIPTOR )
        {
            // Einzelproperties...
            for ( sal_uInt16 i = BASEPROPERTY_FONTDESCRIPTORPART_START; i <= BASEPROPERTY_FONTDESCRIPTORPART_END; i++ )
                aSortedPropsIds[ 1+GetPropertyOrderNr( i ) ]  = i;
        }
    }

    sal_uInt32 nProps = aSortedPropsIds.size();   // koennen jetzt mehr sein
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps( nProps );
    ::com::sun::star::beans::Property* pProps = aProps.getArray();

    std::map<sal_Int32, sal_uInt16>::const_iterator it = aSortedPropsIds.begin();
    for ( sal_uInt32 n = 0; n < nProps; n++, ++it )
    {
        sal_uInt16 nId = it->second;
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
