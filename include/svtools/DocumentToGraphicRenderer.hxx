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


#ifndef INCLUDED_SVTOOLS_DOCUMENTTOGRAPHICRENDERER_HXX
#define INCLUDED_SVTOOLS_DOCUMENTTOGRAPHICRENDERER_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/awt/XToolkit.hpp>

#include <vcl/graph.hxx>

#include <svtools/svtdllapi.h>

class SVT_DLLPUBLIC DocumentToGraphicRenderer
{
    const css::uno::Reference<css::lang::XComponent>& mxDocument;

    css::uno::Reference<css::frame::XModel>        mxModel;
    css::uno::Reference<css::frame::XController>   mxController;
    css::uno::Reference<css::view::XRenderable>    mxRenderable;
    css::uno::Reference<css::awt::XToolkit>        mxToolkit;

public:
    DocumentToGraphicRenderer(const css::uno::Reference<css::lang::XComponent>& xDocument);
    ~DocumentToGraphicRenderer();

    sal_Int32 getCurrentPageWriter( );

    Size getDocumentSizeInPixels( sal_Int32 aCurrentPage );

    Size getDocumentSizeIn100mm( sal_Int32 aCurrentPage );

    Graphic renderToGraphic(sal_Int32 aCurrentPage, Size aDocumentSizePixel,
                            Size aTargetSizePixel, Color aPageColor = COL_TRANSPARENT);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
