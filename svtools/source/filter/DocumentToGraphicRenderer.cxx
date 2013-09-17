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

#include <svtools/DocumentToGraphicRenderer.hxx>

#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <toolkit/helper/vclunohelper.hxx>

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::beans;

DocumentToGraphicRenderer::DocumentToGraphicRenderer( const Reference<XComponent>& rxDocument ) :
    mxDocument(rxDocument),
    mxModel( mxDocument, uno::UNO_QUERY ),
    mxController( mxModel->getCurrentController() ),
    mxRenderable (mxDocument, uno::UNO_QUERY ),
    mxToolkit( VCLUnoHelper::CreateToolkit() )
{
}

DocumentToGraphicRenderer::~DocumentToGraphicRenderer()
{
}

Size DocumentToGraphicRenderer::getDocumentSizeInPixels(sal_Int32 aCurrentPage)
{
    Size aSize100mm = getDocumentSizeIn100mm(aCurrentPage);
    return Size( Application::GetDefaultDevice()->LogicToPixel( aSize100mm, MAP_100TH_MM ) );
}

Size DocumentToGraphicRenderer::getDocumentSizeIn100mm(sal_Int32 aCurrentPage)
{
    Reference< awt::XDevice > xDevice(mxToolkit->createScreenCompatibleDevice( 32, 32 ) );

    uno::Any selection;
    selection <<= mxDocument;

    PropertyValues renderProperties;

    renderProperties.realloc( 3 );
    renderProperties[0].Name = "IsPrinter";
    renderProperties[0].Value <<= sal_True;
    renderProperties[1].Name = "RenderDevice";
    renderProperties[1].Value <<= xDevice;
    renderProperties[2].Name = "View";
    renderProperties[2].Value <<= mxController;

    awt::Size aSize;

    sal_Int32 nPages = mxRenderable->getRendererCount( selection, renderProperties );
    if (nPages >= aCurrentPage)
    {
        Sequence< beans::PropertyValue > aResult = mxRenderable->getRenderer(aCurrentPage - 1, selection, renderProperties );
        for( sal_Int32 nProperty = 0, nPropertyCount = aResult.getLength(); nProperty < nPropertyCount; ++nProperty )
        {
            if ( aResult[ nProperty ].Name == "PageSize" )
            {
                aResult[ nProperty ].Value >>= aSize;
            }
        }
    }

    return Size( aSize.Width, aSize.Height );
}

Graphic DocumentToGraphicRenderer::renderToGraphic(
    sal_Int32 aCurrentPage,
    Size aDocumentSizePixel,
    Size aTargetSizePixel)

{
    if (!mxModel.is() || !mxController.is() || !mxRenderable.is())
        return Graphic();

    Reference< awt::XDevice > xDevice(mxToolkit->createScreenCompatibleDevice( aTargetSizePixel.Width(), aTargetSizePixel.Height() ) );
    if (!xDevice.is())
        return Graphic();

    double fScaleX = aTargetSizePixel.Width()  / (double) aDocumentSizePixel.Width();
    double fScaleY = aTargetSizePixel.Height() / (double) aDocumentSizePixel.Height();

    PropertyValues renderProps;
    renderProps.realloc( 3 );
    renderProps[0].Name = "IsPrinter";
    renderProps[0].Value <<= sal_True;
    renderProps[1].Name = "RenderDevice";
    renderProps[1].Value <<= xDevice;
    renderProps[2].Name = "View";
    renderProps[2].Value <<= mxController;

    GDIMetaFile aMtf;

    OutputDevice* pOutputDev = VCLUnoHelper::GetOutputDevice( xDevice );
    pOutputDev->SetAntialiasing(pOutputDev->GetAntialiasing() | ANTIALIASING_ENABLE_B2DDRAW);
    MapMode mm = pOutputDev->GetMapMode();
    mm.SetScaleX( fScaleX );
    mm.SetScaleY( fScaleY );
    pOutputDev->SetMapMode( mm );

    aMtf.Record( pOutputDev );

    uno::Any aSelection;
    aSelection <<= mxDocument;
    mxRenderable->render(aCurrentPage - 1, aSelection, renderProps );

    aMtf.Stop();
    aMtf.WindStart();
    aMtf.SetPrefSize( aTargetSizePixel );

    return Graphic(aMtf);
}

sal_Int32 DocumentToGraphicRenderer::getCurrentPageWriter()
{
    Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(mxModel->getCurrentController(), UNO_QUERY);
    Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), UNO_QUERY);
    return xCursor->getPage();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
