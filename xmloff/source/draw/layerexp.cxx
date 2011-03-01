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
#include "precompiled_xmloff.hxx"

#include <tools/debug.hxx>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/xmltoken.hxx>
#include "xmlnmspe.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/nmspmap.hxx>
#include "layerexp.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::xmloff::token;

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

    const OUString strName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );
    const OUString strTitle( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
    const OUString strDescription( RTL_CONSTASCII_USTRINGPARAM( "Description" ) );

    OUString sTmp;

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_LAYER_SET, sal_True, sal_True );

    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        try
        {
            Reference< XPropertySet> xLayer( xLayerManager->getByIndex( nIndex ), UNO_QUERY_THROW );
            xLayer->getPropertyValue( strName ) >>= sTmp;
            if(sTmp.getLength())
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, sTmp );

            SvXMLElementExport aEle( rExport, XML_NAMESPACE_DRAW, XML_LAYER, sal_True, sal_True );

            // title property (as <svg:title> element)
            xLayer->getPropertyValue(strTitle) >>= sTmp;
            if(sTmp.getLength())
            {
                SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SVG, XML_TITLE, sal_True, sal_False);
                rExport.Characters(sTmp);
            }

            // description property (as <svg:desc> element)
            xLayer->getPropertyValue(strDescription) >>= sTmp;
            if(sTmp.getLength() > 0)
            {
                SvXMLElementExport aDesc(rExport, XML_NAMESPACE_SVG, XML_DESC, sal_True, sal_False);
                rExport.Characters(sTmp);
            }
        }
        catch( Exception& )
        {
            OSL_FAIL("SdXMLayerExporter::exportLayer(), exception caught during export of one layer!");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
