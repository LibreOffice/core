/*************************************************************************
 *
 *  $RCSfile: unopropertyarrayhelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
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

sal_Bool UnoPropertyArrayHelper::ImplHasProperty( sal_uInt16 nPropId ) const
{
    if ( ( nPropId >= BASEPROPERTY_FONTDESCRIPTORPART_START ) && ( nPropId <= BASEPROPERTY_FONTDESCRIPTORPART_END ) )
        nPropId = BASEPROPERTY_FONTDESCRIPTOR;

    return maIDs.Get( nPropId ) ? sal_True : sal_False;
}

// ::cppu::IPropertyArrayHelper
sal_Bool UnoPropertyArrayHelper::fillPropertyMembersByHandle( ::rtl::OUString * pPropName, sal_Int16 * pAttributes, sal_Int32 nPropId )
{
    sal_Bool bValid = ImplHasProperty( nPropId );
    if ( bValid )
    {
        if ( pPropName )
            *pPropName = GetPropertyName( nPropId );
        if ( pAttributes )
            *pAttributes = GetPropertyAttribs( nPropId );
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
        sal_uInt32 nId = maIDs.GetObjectKey( s );
        aSortedPropsIds.Insert( 1+GetPropertyOrderNr( nId ), (void*)(sal_uInt32)nId );

        if ( nId == BASEPROPERTY_FONTDESCRIPTOR )
        {
            // Einzelproperties...
            for ( sal_uInt32 i = BASEPROPERTY_FONTDESCRIPTORPART_START; i <= BASEPROPERTY_FONTDESCRIPTORPART_END; i++ )
                aSortedPropsIds.Insert( 1+GetPropertyOrderNr( i ), (void*)(sal_uInt32)i );
        }
    }

    nProps = aSortedPropsIds.Count();   // koennen jetzt mehr sein
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps( nProps );
    ::com::sun::star::beans::Property* pProps = aProps.getArray();

    for ( sal_uInt32 n = 0; n < nProps; n++ )
    {
        sal_uInt16 nId = (sal_uInt16)(sal_uInt32)aSortedPropsIds.GetObject( n );
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


