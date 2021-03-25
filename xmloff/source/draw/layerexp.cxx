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


#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlexp.hxx>
#include "layerexp.hxx"
#include <tools/diagnose_ex.h>

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

    static const OUStringLiteral strName( u"Name" );
    static const OUStringLiteral strTitle( u"Title" );
    static const OUStringLiteral strDescription( u"Description" );
    static const OUStringLiteral strIsVisible( u"IsVisible");
    static const OUStringLiteral strIsPrintable( u"IsPrintable");
    static const OUStringLiteral strIsLocked( u"IsLocked" );

    OUString sTmp;


    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_LAYER_SET, true, true );

    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        try
        {
            Reference< XPropertySet> xLayer( xLayerManager->getByIndex( nIndex ), UNO_QUERY_THROW );
            xLayer->getPropertyValue( strName ) >>= sTmp;
            if(!sTmp.isEmpty())
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, sTmp );

            bool bTmpVisible( true );
            bool bTmpPrintable( true );
            xLayer->getPropertyValue( strIsVisible) >>= bTmpVisible;
            xLayer->getPropertyValue( strIsPrintable) >>= bTmpPrintable;
            // only write non-default values, default is "always"
            if ( bTmpVisible )
            {
                if ( !bTmpPrintable )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY, OUString("screen") );
            }
            else
            {
                if ( bTmpPrintable)
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY, OUString("printer") );
                else
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY, OUString("none") );
            }

            bool bTmpLocked( false );
            xLayer->getPropertyValue( strIsLocked ) >>= bTmpLocked;
            // only write non-default value, default is "false"
            if ( bTmpLocked )
            {
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_PROTECTED, OUString("true") );
            }

            SvXMLElementExport aEle( rExport, XML_NAMESPACE_DRAW, XML_LAYER, true, true );

            // title property (as <svg:title> element)
            xLayer->getPropertyValue(strTitle) >>= sTmp;
            if(!sTmp.isEmpty())
            {
                SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SVG, XML_TITLE, true, false);
                rExport.Characters(sTmp);
            }

            // description property (as <svg:desc> element)
            xLayer->getPropertyValue(strDescription) >>= sTmp;
            if(!sTmp.isEmpty())
            {
                SvXMLElementExport aDesc(rExport, XML_NAMESPACE_SVG, XML_DESC, true, false);
                rExport.Characters(sTmp);
            }
        }
        catch( Exception& )
        {
            TOOLS_WARN_EXCEPTION("xmloff.draw", "exception caught during export of one layer!");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
