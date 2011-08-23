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


#ifndef _COM_SUN_STAR_DRAWING_XLAYERSUPPLIER_HPP_ 
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#endif




#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif



#ifndef _XMLOFF_LAYEREXP_HXX
#include "layerexp.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::binfilter::xmloff::token;

void SdXMLayerExporter::exportLayer( SvXMLExport& rExport )
{
    Reference< XLayerSupplier > xLayerSupplier( rExport.GetModel(), UNO_QUERY );
    if( !xLayerSupplier.is() )
        return;

    Reference< XIndexAccess > xLayerManager( xLayerSupplier->getLayerManager(), UNO_QUERY );
    if( !xLayerManager.is() )
        return;

    const sal_Int32 nCount = xLayerManager->getCount();
    if( nCount == 0 )
        return;

    Reference< XPropertySet> xLayer;
    const OUString strName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );

    OUStringBuffer sTmp;
    OUString aName;

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_LAYER_SET, sal_True, sal_True );

    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        xLayerManager->getByIndex( nIndex ) >>= xLayer;

        if( xLayer.is() )
        {
            if( xLayer->getPropertyValue( strName ) >>= aName )
            {
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aName );
            }

            SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_LAYER, sal_True, sal_True );
        }	  
    }
}
}//end of namespace binfilter
