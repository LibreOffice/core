/*************************************************************************
 *
 *  $RCSfile: XMLPageExport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-11 07:49:38 $
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

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLPAGEEXPORT_HXX
#include "XMLPageExport.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

void XMLPageExport::collectPageMasterAutoStyle(
            const Reference < XPropertySet > & rPropSet )
{
}

void XMLPageExport::exportMasterPageContent(
                const Reference < XPropertySet > & rPropSet,
                sal_Bool bAutoStyles )
{
}

sal_Bool XMLPageExport::exportStyle(
            const Reference< XStyle >& rStyle,
            sal_Bool bAutoStyles )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    Any aAny;

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        if( !*(sal_Bool *)aAny.getValue() )
            return sal_False;
    }

    if( bAutoStyles )
    {
        collectPageMasterAutoStyle( xPropSet );
        exportMasterPageContent( xPropSet, sal_True );
    }
    else
    {
        OUString sName( rStyle->getName() );
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_name, sName );

        aAny = xPropSet->getPropertyValue( sFollowStyle );
        OUString sNextName;
        aAny >>= sNextName;
        if( sName != sNextName )
        {
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_next_style_name,
                          sNextName );
        }
//      OUString sPageMaster = GetExport().GetAutoStylePool()->Find(
//                                          XML_STYLE_FAMILY_PAGE_MASTER,
//                                          xPropSet );
//      if( sPageMaster.getLength() )
//          GetExport().AddAttribute( XML_NAMESPACE_STYLE,
//                                    sXML_page_master_name,
//                                    sPageMaster );

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                  sXML_master_page, sal_True, sal_True );

        exportMasterPageContent( xPropSet, sal_False );
    }

    return sal_True;
}

XMLPageExport::XMLPageExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sFollowStyle( RTL_CONSTASCII_USTRINGPARAM( "FollowStyle" ) )
{
    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(),
                                                       UNO_QUERY );
    DBG_ASSERT( xFamiliesSupp.is(),
                "No XStyleFamiliesSupplier from XModel for export!" );
    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        DBG_ASSERT( xFamiliesSupp.is(),
                    "getStyleFamilies() from XModel failed for export!" );
        if( xFamilies.is() )
        {
            const OUString aPageStyleName(
                        RTL_CONSTASCII_USTRINGPARAM( "PageStyles" ));

            if( xFamilies->hasByName( aPageStyleName ) )
            {
                Reference < XNameContainer > xStyleCont;
                xFamilies->getByName( aPageStyleName ) >>= xStyleCont;

                xPageStyles = Reference< XIndexAccess >( xStyleCont,
                                                          UNO_QUERY );

                DBG_ASSERT( xPageStyles.is(),
                            "Page Styles not found for export!" );
            }
        }
    }
}

XMLPageExport::~XMLPageExport()
{
}

void XMLPageExport::exportStyles( sal_Bool bUsed, sal_Bool bAutoStyles )
{
    if( xPageStyles.is() )
    {
        const sal_Int32 nStyles = xPageStyles->getCount();

        for( sal_Int32 i=0; i < nStyles; i++ )
        {
            Reference< XStyle > xStyle;
            xPageStyles->getByIndex( i ) >>= xStyle;

            if( !bUsed || xStyle->isInUse() )
                exportStyle( xStyle, bAutoStyles );
        }
    }
}

