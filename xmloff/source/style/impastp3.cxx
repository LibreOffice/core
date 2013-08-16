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

#include "impastpl.hxx"
#include <xmloff/xmlprmap.hxx>

using namespace std;

// Class SvXMLAutoStylePoolParent_Impl
// dtor class SvXMLAutoStylePoolParent_Impl

SvXMLAutoStylePoolParentP_Impl::~SvXMLAutoStylePoolParentP_Impl()
{
    for( size_t i = maPropertiesList.size(); i > 0;  )
        delete maPropertiesList[ --i ];
    maPropertiesList.clear();
}

// Adds a array of XMLPropertyState ( vector< XMLPropertyState > ) to list
// if not added, yet.

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

bool SvXMLAutoStylePoolParentP_Impl::operator< (const SvXMLAutoStylePoolParentP_Impl& rOther) const
{
    return msParent < rOther.msParent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
