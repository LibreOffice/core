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

#include "impastpl.hxx"
#include <xmloff/xmlprmap.hxx>

using namespace std;

using ::rtl::OUString;

//#############################################################################
//
// Class SvXMLAutoStylePoolParent_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// dtor class SvXMLAutoStylePoolParent_Impl
//

SvXMLAutoStylePoolParentP_Impl::~SvXMLAutoStylePoolParentP_Impl()
{
    for( size_t i = maPropertiesList.size(); i > 0;  )
        delete maPropertiesList[ --i ];
    maPropertiesList.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.
//

sal_Bool SvXMLAutoStylePoolParentP_Impl::Add( XMLFamilyData_Impl& rFamilyData, const vector< XMLPropertyState >& rProperties, OUString& rName, bool bDontSeek )
{
    sal_Bool bAdded = sal_False;
    SvXMLAutoStylePoolPropertiesP_Impl *pProperties = 0;
    size_t i = 0;
    sal_Int32 nProperties = rProperties.size();
    size_t nCount = maPropertiesList.size();

    for( i = 0; i < nCount; i++ )
    {
        SvXMLAutoStylePoolPropertiesP_Impl *pIS = maPropertiesList[ i ];
        if( nProperties > (sal_Int32)pIS->GetProperties().size() )
        {
            continue;
        }
        else if( nProperties < (sal_Int32)pIS->GetProperties().size() )
        {
            break;
        }
        else if( !bDontSeek && rFamilyData.mxMapper->Equals( pIS->GetProperties(), rProperties ) )
        {
            pProperties = pIS;
            break;
        }
    }

    if( !pProperties )
    {
        pProperties = new SvXMLAutoStylePoolPropertiesP_Impl( rFamilyData, rProperties );
        SvXMLAutoStylePoolPropertiesPList_Impl::iterator it = maPropertiesList.begin();
        ::std::advance( it, i );
        maPropertiesList.insert( it, pProperties );
        bAdded = sal_True;
    }

    rName = pProperties->GetName();

    return bAdded;
}

///////////////////////////////////////////////////////////////////////////////
//
// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) with a given name.
// If the name exists already, nothing is done. If a style with a different name and
// the same properties exists, a new one is added (like with bDontSeek).
//

sal_Bool SvXMLAutoStylePoolParentP_Impl::AddNamed( XMLFamilyData_Impl& rFamilyData, const vector< XMLPropertyState >& rProperties, const OUString& rName )
{
    sal_Bool bAdded = sal_False;
    size_t i = 0;
    sal_Int32 nProperties = rProperties.size();
    size_t nCount = maPropertiesList.size();

    for( i = 0; i < nCount; i++ )
    {
        SvXMLAutoStylePoolPropertiesP_Impl *pIS = maPropertiesList[ i ];
        if( nProperties > (sal_Int32)pIS->GetProperties().size() )
        {
            continue;
        }
        else if( nProperties < (sal_Int32)pIS->GetProperties().size() )
        {
            break;
        }
    }

    if(rFamilyData.mpNameList->find(rName) == rFamilyData.mpNameList->end())
    {
        SvXMLAutoStylePoolPropertiesP_Impl* pProperties =
                new SvXMLAutoStylePoolPropertiesP_Impl( rFamilyData, rProperties );
        // ignore the generated name
        pProperties->SetName( rName );
        SvXMLAutoStylePoolPropertiesPList_Impl::iterator it = maPropertiesList.begin();
        ::std::advance( it, i );
        maPropertiesList.insert( it, pProperties );
        bAdded = sal_True;
    }

    return bAdded;
}

///////////////////////////////////////////////////////////////////////////////
//
// Search for a array of XMLPropertyState ( vector< XMLPropertyState > ) in list
//

OUString SvXMLAutoStylePoolParentP_Impl::Find( const XMLFamilyData_Impl& rFamilyData, const vector< XMLPropertyState >& rProperties ) const
{
    OUString sName;
    vector< XMLPropertyState>::size_type nItems = rProperties.size();
    size_t nCount = maPropertiesList.size();
    for( size_t i = 0; i < nCount; i++ )
    {
        SvXMLAutoStylePoolPropertiesP_Impl *pIS = maPropertiesList[ i ];
        if( nItems > pIS->GetProperties().size() )
        {
            continue;
        }
        else if( nItems < pIS->GetProperties().size() )
        {
            break;
        }
        else if( rFamilyData.mxMapper->Equals( pIS->GetProperties(), rProperties ) )
        {
            sName = pIS->GetName();
            break;
        }
    }

    return sName;
}

///////////////////////////////////////////////////////////////////////////////
//
// Sort-function for sorted list of SvXMLAutoStylePoolParent_Impl-elements
//

int SvXMLAutoStylePoolParentPCmp_Impl( const SvXMLAutoStylePoolParentP_Impl& r1,
                                       const SvXMLAutoStylePoolParentP_Impl& r2)
{
    return (int)r1.GetParent().compareTo( r2.GetParent() );
}

///////////////////////////////////////////////////////////////////////////////
//
// Implementation of sorted list of SvXMLAutoStylePoolParent_Impl-elements
//

IMPL_CONTAINER_SORT( SvXMLAutoStylePoolParentsP_Impl,
                     SvXMLAutoStylePoolParentP_Impl,
                     SvXMLAutoStylePoolParentPCmp_Impl )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
