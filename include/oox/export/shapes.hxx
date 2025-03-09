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

#ifndef INCLUDED_OOX_EXPORT_SHAPES_HXX
#define INCLUDED_OOX_EXPORT_SHAPES_HXX

#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_map>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <oox/dllapi.h>
#include <oox/export/drawingml.hxx>
#include <oox/export/utils.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sax/fshelper.hxx>
#include <tools/fract.hxx>
#include <vcl/mapmod.hxx>

namespace com::sun::star {
namespace beans {
    class XPropertySet;
}
namespace drawing {
    class XShape;
}

namespace embed {
    class XEmbeddedObject;
}
namespace io {
    class XInputStream;
}
namespace uno {
    class XComponentContext;
    class XInterface;
}
}

namespace oox::core {
    class XmlFilterBase;
}

class Graphic;

namespace oox {

OOX_DLLPUBLIC css::uno::Reference<css::io::XInputStream> GetOLEObjectStream(
    css::uno::Reference<css::uno::XComponentContext> const& xContext,
    css::uno::Reference<css::embed::XEmbeddedObject> const& xObj,
    std::u16string_view i_rProgID,
    OUString & o_rMediaType,
    OUString & o_rRelationType,
    OUString & o_rSuffix,
    const char *& o_rpProgID);

}

namespace oox::drawingml {

class OOX_DLLPUBLIC ShapeExport : public DrawingML {

private:
    int m_nEmbeddedObjects;

public:
    typedef std::unordered_map< css::uno::Reference< css::drawing::XShape>, sal_Int32> ShapeHashMap;

protected:
    sal_Int32           mnShapeIdMax;
    bool                mbUserShapes; // for chart's embedded usershapes

    void WriteGraphicObjectShapePart( const css::uno::Reference< css::drawing::XShape >& xShape, const Graphic *pGraphic=nullptr );

    OUString            GetShapeName(const css::uno::Reference< css::drawing::XShape >& xShape);

private:
    sal_Int32           mnXmlNamespace;
    MapMode             maMapModeSrc, maMapModeDest;
    std::shared_ptr<URLTransformer> mpURLTransformer;

    css::awt::Size MapSize( const css::awt::Size& ) const;

    ShapeHashMap maShapeMap;
    ShapeHashMap* mpShapeMap;

public:

    ShapeExport( sal_Int32 nXmlNamespace, ::sax_fastparser::FSHelperPtr pFS,
                 ShapeHashMap* pShapeMap, ::oox::core::XmlFilterBase* pFB,
                 DocumentType eDocumentType = DOCUMENT_PPTX,
                 DMLTextExport* pTextExport = nullptr,
                 bool bUserShapes = false );
    virtual ~ShapeExport() {}

    void SetURLTranslator(const std::shared_ptr<URLTransformer>& pTransformer);

    static bool         NonEmptyText( const css::uno::Reference< css::uno::XInterface >& xIface );
    static bool         IsShapeTypeKnown( const css::uno::Reference< css::drawing::XShape >& xShape );

    ShapeExport&
                        WritePolyPolygonShape( const css::uno::Reference< css::drawing::XShape >& xShape, bool bClosed );
    ShapeExport&
                        WriteClosedPolyPolygonShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteConnectorShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteCustomShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteEllipseShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteGraphicObjectShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteGroupShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteLineShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteNonVisualDrawingProperties( const css::uno::Reference< css::drawing::XShape >& xShape, const char* sName );
    virtual ShapeExport&
                        WriteNonVisualProperties( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteOpenPolyPolygonShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteRectangleShape( const css::uno::Reference< css::drawing::XShape >& xShape );

    /**
     * Write the DrawingML for a particular shape.
     *
     * <p>This is the member function you want.  It performs the type lookup and
     * invokes the appropriate corresponding Write*() method for the specific
     * type.</p>
     *
     * <p>To write an XShape, XShape::getShapeType() is called to determine
     * the shape type, and the corresponding method in this table is
     * invoked:</p>
     *
     * <table>
     *   <tr><th>Shape Type</th><th>Method</th></tr>
     *   <tr><td><tt>com.sun.star.drawing.ClosedBezierShape</tt></td>    <td>ShapeExport::WriteClosedPolyPolygonShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.CustomShape</tt></td>          <td>ShapeExport::WriteCustomShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.EllipseShape</tt></td>         <td>ShapeExport::WriteEllipseShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.GraphicObjectShape</tt></td>   <td>ShapeExport::WriteGraphicObjectShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.LineShape</tt></td>            <td>ShapeExport::WriteLineShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.OpenBezierShape</tt></td>      <td>ShapeExport::WriteOpenPolyPolygonShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.PolyPolygonShape</tt></td>      <td>ShapeExport::WriteClosedPolyPolygonShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.PolyLineShape</tt></td>      <td>ShapeExport::WriteOpenPolyPolygonShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.RectangleShape</tt></td>       <td>ShapeExport::WriteRectangleShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.TableShape</tt></td>           <td>ShapeExport::WriteTableShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.TextShape</tt></td>            <td>ShapeExport::WriteTextShape</td></tr>
     *   <tr><td><tt>com.sun.star.presentation.DateTimeShape</tt></td>   <td>ShapeExport::WriteTextShape</td></tr>
     *   <tr><td><tt>com.sun.star.presentation.FooterShape</tt></td>     <td>ShapeExport::WriteTextShape</td></tr>
     *   <tr><td><tt>com.sun.star.presentation.HeaderShape</tt></td>     <td>ShapeExport::WriteTextShape</td></tr>
     *   <tr><td><tt>com.sun.star.presentation.NotesShape</tt></td>      <td>ShapeExport::WriteTextShape</td></tr>
     *   <tr><td><tt>com.sun.star.presentation.OutlinerShape</tt></td>   <td>ShapeExport::WriteTextShape</td></tr>
     *   <tr><td><tt>com.sun.star.presentation.SlideNumberShape</tt></td><td>ShapeExport::WriteTextShape</td></tr>
     *   <tr><td><tt>com.sun.star.presentation.TitleTextShape</tt></td>  <td>ShapeExport::WriteTextShape</td></tr>
     * </table>
     *
     * <p>If the shape type is not recognized, then
     * <tt>ShapeExport::WriteUnknownShape</tt> is called.</p>
     *
     * @param xShape    The shape to export as DrawingML.
     * @return   <tt>*this</tt>
     */
    ShapeExport&       WriteShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&       WriteTextBox( const css::uno::Reference< css::uno::XInterface >& xIface, sal_Int32 nXmlNamespace, bool bWritePropertiesAsLstStyles = false );
    virtual ShapeExport&
                        WriteTextShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    ShapeExport&
                        WriteTableShape( const css::uno::Reference< css::drawing::XShape >& xShape );
    void                WriteMathShape(css::uno::Reference<css::drawing::XShape> const& xShape);
    ShapeExport&
                        WriteOLE2Shape( const css::uno::Reference< css::drawing::XShape >& xShape );
    virtual ShapeExport&
                        WriteUnknownShape( const css::uno::Reference< css::drawing::XShape >& xShape );

    void WriteTable( const css::uno::Reference< css::drawing::XShape >& rXShape );

    void WriteTableCellProperties(const css::uno::Reference< css::beans::XPropertySet >& rXPropSet);

    void WriteBorderLine(const sal_Int32 XML_line, const css::table::BorderLine2& rBorderLine);
    void WriteTableCellBorders(const css::uno::Reference< css::beans::XPropertySet >& rXPropSet);

    sal_Int32 GetNewShapeID( const css::uno::Reference< css::drawing::XShape >& rShape );
    sal_Int32 GetNewShapeID( const css::uno::Reference< css::drawing::XShape >& rShape, ::oox::core::XmlFilterBase* pFB );
    sal_Int32 GetShapeID( const css::uno::Reference< css::drawing::XShape >& rShape );
    static sal_Int32 GetShapeID( const css::uno::Reference< css::drawing::XShape >& rShape, ShapeHashMap* pShapeMap );
};

}

#endif // INCLUDED_OOX_EXPORT_SHAPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
