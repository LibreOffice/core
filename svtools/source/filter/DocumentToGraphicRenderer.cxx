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

#include <tools/fract.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <toolkit/helper/vclunohelper.hxx>

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::beans;

DocumentToGraphicRenderer::DocumentToGraphicRenderer( const Reference<XComponent>& rxDocument, bool bSelectionOnly ) :
    mxDocument(rxDocument),
    mxModel( mxDocument, uno::UNO_QUERY ),
    mxController( mxModel->getCurrentController() ),
    mxRenderable (mxDocument, uno::UNO_QUERY ),
    mxToolkit( VCLUnoHelper::CreateToolkit() ),
    mbSelectionOnly( bSelectionOnly ),
    mbIsWriter( false )
{
    try
    {
        uno::Reference< lang::XServiceInfo > xServiceInfo( mxDocument, uno::UNO_QUERY);
        if (xServiceInfo.is())
        {
            if (xServiceInfo->supportsService("com.sun.star.text.TextDocument"))
                mbIsWriter = true;
        }
    }
    catch (const uno::Exception&)
    {
    }

    if (!(mbSelectionOnly && mxController.is()))
        return;

    try
    {
        uno::Reference< view::XSelectionSupplier > xSelSup( mxController, uno::UNO_QUERY);
        if (xSelSup.is())
        {
            uno::Any aViewSelection( xSelSup->getSelection());
            if (aViewSelection.hasValue())
            {
                /* FIXME: Writer always has a selection even if nothing is
                 * selected, but passing a selection to
                 * XRenderable::render() it always renders an empty page.
                 * So disable the selection already here. The current page
                 * the cursor is on is rendered. */
                if (!mbIsWriter)
                    maSelection = aViewSelection;
            }
        }
    }
    catch (const uno::Exception&)
    {
    }
}

DocumentToGraphicRenderer::~DocumentToGraphicRenderer()
{
}

Size DocumentToGraphicRenderer::getDocumentSizeInPixels(sal_Int32 nCurrentPage)
{
    Size aSize100mm = getDocumentSizeIn100mm(nCurrentPage);
    return Application::GetDefaultDevice()->LogicToPixel(aSize100mm, MapMode(MapUnit::Map100thMM));
}

bool DocumentToGraphicRenderer::hasSelection() const
{
    return maSelection.hasValue();
}

uno::Any DocumentToGraphicRenderer::getSelection() const
{
    uno::Any aSelection;
    if (hasSelection())
        aSelection = maSelection;
    else
        aSelection <<= mxDocument;  // default: render whole document
    return aSelection;
}

Size DocumentToGraphicRenderer::getDocumentSizeIn100mm(sal_Int32 nCurrentPage)
{
    Reference< awt::XDevice > xDevice(mxToolkit->createScreenCompatibleDevice( 32, 32 ) );

    uno::Any selection( getSelection());

    PropertyValues renderProperties;

    renderProperties.realloc( 4 );
    renderProperties[0].Name = "IsPrinter";
    renderProperties[0].Value <<= true;
    renderProperties[1].Name = "RenderDevice";
    renderProperties[1].Value <<= xDevice;
    renderProperties[2].Name = "View";
    renderProperties[2].Value <<= mxController;
    renderProperties[3].Name = "RenderToGraphic";
    renderProperties[3].Value <<= true;

    awt::Size aSize;

    sal_Int32 nPages = mxRenderable->getRendererCount( selection, renderProperties );
    if (nPages >= nCurrentPage)
    {
        Sequence< beans::PropertyValue > aResult = mxRenderable->getRenderer(nCurrentPage - 1, selection, renderProperties );
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
    sal_Int32 nCurrentPage,
    Size aDocumentSizePixel,
    Size aTargetSizePixel,
    Color aPageColor)

{
    if (!mxModel.is() || !mxController.is() || !mxRenderable.is())
        return Graphic();

    Reference< awt::XDevice > xDevice(mxToolkit->createScreenCompatibleDevice( aTargetSizePixel.Width(), aTargetSizePixel.Height() ) );
    if (!xDevice.is())
        return Graphic();

    assert( aDocumentSizePixel.Width() > 0 && aDocumentSizePixel.Height() > 0 &&
            aTargetSizePixel.Width() > 0 && aTargetSizePixel.Height() > 0);

    double fScaleX = aTargetSizePixel.Width()  / (double) aDocumentSizePixel.Width();
    double fScaleY = aTargetSizePixel.Height() / (double) aDocumentSizePixel.Height();

    PropertyValues renderProps;
    renderProps.realloc( 4 );
    renderProps[0].Name = "IsPrinter";
    renderProps[0].Value <<= true;
    renderProps[1].Name = "RenderDevice";
    renderProps[1].Value <<= xDevice;
    renderProps[2].Name = "View";
    renderProps[2].Value <<= mxController;
    renderProps[3].Name = "RenderToGraphic";
    renderProps[3].Value <<= true;

    GDIMetaFile aMtf;

    OutputDevice* pOutputDev = VCLUnoHelper::GetOutputDevice( xDevice );
    pOutputDev->SetAntialiasing(pOutputDev->GetAntialiasing() | AntialiasingFlags::EnableB2dDraw);
    MapMode mm = pOutputDev->GetMapMode();
    mm.SetScaleX( Fraction(fScaleX) );
    mm.SetScaleY( Fraction(fScaleY) );
    pOutputDev->SetMapMode( mm );

    aMtf.Record( pOutputDev );

    if (aPageColor != Color(COL_TRANSPARENT))
    {
        pOutputDev->SetBackground(Wallpaper(aPageColor));
        pOutputDev->Erase();
    }

    uno::Any aSelection( getSelection());
    mxRenderable->render(nCurrentPage - 1, aSelection, renderProps );

    aMtf.Stop();
    aMtf.WindStart();
    aMtf.SetPrefSize( aTargetSizePixel );

    return Graphic(aMtf);
}

sal_Int32 DocumentToGraphicRenderer::getCurrentPage()
{
    if (hasSelection())
        return 1;

    if (mbIsWriter)
        return getCurrentPageWriter();

    /* TODO: other application specific page detection? */
    return 1;
}

sal_Int32 DocumentToGraphicRenderer::getCurrentPageWriter()
{
    Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(mxModel->getCurrentController(), UNO_QUERY);
    if (!xTextViewCursorSupplier.is())
        return 1;
    Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), UNO_QUERY);
    return xCursor.is() ? xCursor->getPage() : 1;
}

// static
bool DocumentToGraphicRenderer::isShapeSelected(
        css::uno::Reference< css::drawing::XShapes > & rxShapes,
        css::uno::Reference< css::drawing::XShape > & rxShape,
        const css::uno::Reference< css::frame::XController > & rxController )
{
    bool bShape = false;
    if (rxController.is())
    {
        uno::Reference< view::XSelectionSupplier > xSelectionSupplier( rxController, uno::UNO_QUERY);
        if (xSelectionSupplier.is())
        {
            uno::Any aAny( xSelectionSupplier->getSelection());
            if (aAny >>= rxShapes)
                bShape = true;
            else if (aAny >>= rxShape)
                bShape = true;
        }
    }
    return bShape;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
