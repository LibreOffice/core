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
#ifndef INCLUDED_SLIDESHOW_SHAPEIMPORTER_HXX
#define INCLUDED_SLIDESHOW_SHAPEIMPORTER_HXX

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

typedef ::std::vector< ::cppcanvas::PolyPolygonSharedPtr> PolyPolygonVector;
typedef ::boost::shared_ptr< UnoView >      UnoViewSharedPtr;
typedef ::std::vector< UnoViewSharedPtr >   UnoViewVector;

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
    ShapeImporter( const ::com::sun::star::uno::Reference<
                         ::com::sun::star::drawing::XDrawPage >& xPage,
                   const ::com::sun::star::uno::Reference<
                         ::com::sun::star::drawing::XDrawPage >& xActualPage,
                   const ::com::sun::star::uno::Reference<
                         ::com::sun::star::drawing::XDrawPagesSupplier>& xPagesSupplier,
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
private:
    bool isSkip( ::com::sun::star::uno::Reference<
                 ::com::sun::star::beans::XPropertySet> const& xPropSet,
                 ::rtl::OUString const& shapeType,
             ::com::sun::star::uno::Reference<
             ::com::sun::star::drawing::XLayer> const& xLayer);

    ShapeSharedPtr createShape(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape> const& xCurrShape,
        ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> const& xPropSet,
        ::rtl::OUString const& shapeType ) const;

    void importPolygons(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > const& xPropSet) ;

    struct XShapesEntry
    {
        ShapeSharedPtr const mpGroupShape;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShapes> const mxShapes;
        sal_Int32 const mnCount;
        sal_Int32 mnPos;

        explicit XShapesEntry( ShapeSharedPtr const& pGroupShape )
            : mpGroupShape(pGroupShape),
              mxShapes( pGroupShape->getXShape(),
                        ::com::sun::star::uno::UNO_QUERY_THROW ),
              mnCount(mxShapes->getCount()), mnPos(0) {}
        explicit XShapesEntry( ::com::sun::star::uno::Reference<
                      ::com::sun::star::drawing::XShapes> const& xShapes )
            : mpGroupShape(), mxShapes(xShapes),
              mnCount(xShapes->getCount()), mnPos(0) {}
    };
    typedef ::std::stack<XShapesEntry> XShapesStack;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XDrawPage> mxPage;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XDrawPagesSupplier> mxPagesSupplier;
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
