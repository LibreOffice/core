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
namespace binfilter {

using namespace std;
using namespace rtl;

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
    while( maPropertiesList.Count() )
        delete maPropertiesList.Remove( maPropertiesList.Count() -1 );
}

///////////////////////////////////////////////////////////////////////////////
//
// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.
//

sal_Bool SvXMLAutoStylePoolParentP_Impl::Add( XMLFamilyData_Impl* pFamilyData, const vector< XMLPropertyState >& rProperties, OUString& rName )
{
    sal_Bool bAdded = sal_False;

    sal_Int32 nProperties = rProperties.size();
    sal_uInt32 nCount = maPropertiesList.Count();
    SvXMLAutoStylePoolPropertiesP_Impl *pProperties = 0;
    sal_uInt32 i;

    for( i=0; i < nCount; i++ )
    {
        SvXMLAutoStylePoolPropertiesP_Impl *pIS = maPropertiesList.GetObject( i );
        if( nProperties > pIS->GetProperties().size() ) 
        {
            continue;
        }
        else if( nProperties < pIS->GetProperties().size() )
        {
            break;
        }
        else if( pFamilyData->mxMapper->Equals( pIS->GetProperties(), rProperties ) ) 
        {
            pProperties = pIS;
            break;
        }
    }
    
    if( !pProperties )
    {
        pProperties = new SvXMLAutoStylePoolPropertiesP_Impl( pFamilyData, rProperties );
        maPropertiesList.Insert( pProperties, i );
        bAdded = sal_True;
    }
    
    rName = pProperties->GetName();

    return bAdded;
}

///////////////////////////////////////////////////////////////////////////////
//
// Search for a array of XMLPropertyState ( vector< XMLPropertyState > ) in list
//

OUString SvXMLAutoStylePoolParentP_Impl::Find( const XMLFamilyData_Impl* pFamilyData, const vector< XMLPropertyState >& rProperties ) const
{
    OUString sName;
    sal_uInt16 nItems = rProperties.size();
    sal_uInt32 nCount = maPropertiesList.Count();
    for( sal_uInt32 i=0; i < nCount; i++ )
    {
        SvXMLAutoStylePoolPropertiesP_Impl *pIS = maPropertiesList.GetObject( i );
        if( nItems > pIS->GetProperties().size() ) 
        {
            continue;
        }
        else if( nItems < pIS->GetProperties().size() )
        {
            break;
        }
        else if( pFamilyData->mxMapper->Equals( pIS->GetProperties(), rProperties ) ) 
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
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
