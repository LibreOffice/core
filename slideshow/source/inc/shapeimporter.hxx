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
#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEIMPORTER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SHAPEIMPORTER_HXX

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XLayer.hpp>
#include "unoviewcontainer.hxx"
#include "unoview.hxx"

#include "shape.hxx"

#include <stack>

namespace slideshow {
namespace internal {

struct SlideShowContext;

typedef std::vector< ::cppcanvas::PolyPolygonSharedPtr> PolyPolygonVector;
typedef std::shared_ptr< UnoView >      UnoViewSharedPtr;
typedef std::vector< UnoViewSharedPtr >   UnoViewVector;

/** This class imports all shapes from a given XShapes object
 */
class ShapeImporter
{
public:
    /** Create shape importer.

        @param xPage
        Page containing the shapes

        @param xActualPage
        Actual page that's imported - if xPage is a master
        page, this argument must refer to the using, i.e the
        page that embeds this specific masterpage. Otherwise,
        this argument is probably equal to xPage.

        @param nOrdNumStart
        Each shape receives a z order number, in order of
        import (which relies on the fact that the API returns
        the shapes in draw order - which it does,
        currently). Since we might mix several pages on screen
        (e.g. master page and foreground page), this value can
        be used as an offset to distinguish those pages.

        @param bConvertingMasterPage
        When true, then the master page is imported. Otherwise, this
        object imports the draw page.
    */
    ShapeImporter( const css::uno::Reference< css::drawing::XDrawPage >& xPage,
                   const css::uno::Reference< css::drawing::XDrawPage >& xActualPage,
                   const css::uno::Reference< css::drawing::XDrawPagesSupplier>& xPagesSupplier,
                   const SlideShowContext&                       rContext,
                   sal_Int32                                     nOrdNumStart,
                   bool                                          bConvertingMasterPage );

    /** This method imports the presentation background shape
     */
    ShapeSharedPtr importBackgroundShape(); // throw (ShapeLoadFailedException)

    /** This method imports presentation-visible shapes (and skips all others).

        @return the generated Shape, or NULL for no more shapes.
    */
    ShapeSharedPtr importShape(); // throw (ConversionFailedException)

    /** Test whether import is done.

        @return true, if all shapes are imported via the
        importShape() call.
    */
    bool isImportDone() const;
    PolyPolygonVector getPolygons();

    double getImportedShapesCount() { return mnAscendingPrio; }
private:
    bool isSkip( css::uno::Reference<css::beans::XPropertySet> const& xPropSet,
                 OUString const& shapeType,
                 css::uno::Reference<css::drawing::XLayer> const& xLayer);

    ShapeSharedPtr createShape(
        css::uno::Reference<css::drawing::XShape> const& xCurrShape,
        css::uno::Reference<css::beans::XPropertySet> const& xPropSet,
        OUString const& shapeType ) const;

    void importPolygons(css::uno::Reference< css::beans::XPropertySet > const& xPropSet) ;

    struct XShapesEntry
    {
        ShapeSharedPtr const mpGroupShape;
        css::uno::Reference<css::drawing::XShapes> const mxShapes;
        sal_Int32 const mnCount;
        sal_Int32 mnPos;

        explicit XShapesEntry( ShapeSharedPtr const& pGroupShape )
            : mpGroupShape(pGroupShape),
              mxShapes( pGroupShape->getXShape(),
                        css::uno::UNO_QUERY_THROW ),
              mnCount(mxShapes->getCount()), mnPos(0) {}
        explicit XShapesEntry( css::uno::Reference<
                      css::drawing::XShapes> const& xShapes )
            : mpGroupShape(), mxShapes(xShapes),
              mnCount(xShapes->getCount()), mnPos(0) {}
    };
    typedef ::std::stack<XShapesEntry> XShapesStack;

    css::uno::Reference<css::drawing::XDrawPage> mxPage;
    css::uno::Reference<css::drawing::XDrawPagesSupplier> mxPagesSupplier;
    const SlideShowContext&                   mrContext;
    PolyPolygonVector                         maPolygons;
    XShapesStack                              maShapesStack;
    double                                    mnAscendingPrio;
    bool                                      mbConvertingMasterPage;
};

} // namespace internal
} // namespace presentation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
