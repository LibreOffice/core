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

#include <vector>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XRenderable.hpp>
#include <com/sun/star/awt/XToolkit.hpp>

#include <vcl/graph.hxx>

#include <svtools/svtdllapi.h>

namespace com { namespace sun { namespace star {
    namespace drawing {
        class XShapes;
        class XShape;
    }
    namespace frame {
        class XController;
    }
}}}

class SVT_DLLPUBLIC DocumentToGraphicRenderer
{
    const css::uno::Reference<css::lang::XComponent>& mxDocument;

    css::uno::Reference<css::frame::XModel>         mxModel;
    css::uno::Reference<css::frame::XController>    mxController;
    css::uno::Reference<css::view::XRenderable>     mxRenderable;
    css::uno::Reference<css::awt::XToolkit>         mxToolkit;
    css::uno::Any                                   maSelection;
    bool                                            mbIsWriter;
    std::vector<OUString>                           maChapterNames;

    bool hasSelection() const;

    /** Always something even if hasSelection() is false (in which case the
        selection is mxDocument).
     */
    css::uno::Any getSelection() const;

    sal_Int32 getCurrentPageWriter();

public:
    DocumentToGraphicRenderer(const css::uno::Reference<css::lang::XComponent>& xDocument, bool bSelectionOnly);
    ~DocumentToGraphicRenderer();

    sal_Int32 getCurrentPage();
    sal_Int32 getPageCount();
    /**
     * Get list of chapter names for a page, current page is set by
     * renderToGraphic().
     */
    const std::vector<OUString>& getChapterNames() const;

    Size getDocumentSizeInPixels( sal_Int32 nCurrentPage );

    Size getDocumentSizeIn100mm(sal_Int32 nCurrentPage, Point* pDocumentPosition = nullptr);

    Graphic renderToGraphic( sal_Int32 nCurrentPage, Size aDocumentSizePixel,
                            Size aTargetSizePixel, Color aPageColor, bool bExtOutDevData);

    /** Determine whether rxController has a css::view::XSelectionSupplier at
        which either a css::drawing::XShapes or css::drawing::XShape is
        selected. XShapes has precedence over XShape.

        Call only if the SelectionOnly property was set.
     */
    static bool isShapeSelected(
            css::uno::Reference< css::drawing::XShapes > & rxShapes,
            css::uno::Reference< css::drawing::XShape > & rxShape,
            const css::uno::Reference< css::frame::XController > & rxController );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
