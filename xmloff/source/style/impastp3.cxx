/*************************************************************************
 *
 *  $RCSfile: impastp3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:05 $
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

#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#include "impastpl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

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
    for( sal_uInt32 i=0; i < nCount; i++ )
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
