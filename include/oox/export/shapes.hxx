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

#include <oox/dllapi.h>
#include <com/sun/star/uno/XReference.hpp>
#include <oox/export/drawingml.hxx>
#include <sax/fshelper.hxx>
#include <vcl/mapmod.hxx>
#include <tools/fract.hxx>
#include <unordered_map>

namespace com { namespace sun { namespace star {
namespace beans {
    class XPropertySet;
}
namespace drawing {
    class XShape;
    class XShapes;
}
}}}

namespace oox { namespace drawingml {

class OOX_DLLPUBLIC URLTransformer
{
public:
    virtual ~URLTransformer();

    virtual OUString getTransformedString(const OUString& rURL) const;

    virtual bool isExternalURL(const OUString& rURL) const;
};

class OOX_DLLPUBLIC ShapeExport : public DrawingML {

private:
    static int mnEmbeddeDocumentCounter;
    struct ShapeCheck
    {
        bool operator()( const css::uno::Reference< css::drawing::XShape>& s1, const css::uno::Reference< css::drawing::XShape>& s2 ) const
        {
            return s1 == s2;
        }
    };

    struct ShapeHash
    {
        size_t operator()( const css::uno::Reference < css::drawing::XShape > ) const;
    };

public:
    typedef std::unordered_map< css::uno::Reference< css::drawing::XShape>, sal_Int32, ShapeHash, ShapeCheck> ShapeHashMap;

protected:
    sal_Int32           mnShapeIdMax, mnPictureIdMax;

    void WriteGraphicObjectShapePart( css::uno::Reference< css::drawing::XShape > xShape, const Graphic *pGraphic=nullptr );

private:
    sal_Int32           mnXmlNamespace;
    Fraction            maFraction;
    MapMode             maMapModeSrc, maMapModeDest;
    std::shared_ptr<URLTransformer> mpURLTransformer;

    css::awt::Size MapSize( const css::awt::Size& ) const;

    ShapeHashMap maShapeMap;
    ShapeHashMap* mpShapeMap;
    OUString m_presetWarp;

public:

    ShapeExport( sal_Int32 nXmlNamespace, ::sax_fastparser::FSHelperPtr pFS, ShapeHashMap* pShapeMap = nullptr, ::oox::core::XmlFilterBase* pFB = nullptr, DocumentType eDocumentType = DOCUMENT_PPTX, DMLTextExport* pTextExport = nullptr );
    virtual ~ShapeExport() {}

    void SetURLTranslator(std::shared_ptr<URLTransformer> pTransformer);

    static bool         NonEmptyText( css::uno::Reference< css::uno::XInterface > xIface );

    ShapeExport&
                        WriteBezierShape( css::uno::Reference< css::drawing::XShape > xShape, bool bClosed );
    ShapeExport&
                        WriteClosedBezierShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteConnectorShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteCustomShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteEllipseShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteGraphicObjectShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteGroupShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteLineShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteNonVisualDrawingProperties( css::uno::Reference< css::drawing::XShape > xShape, const char* sName );
    virtual ShapeExport&
                        WriteNonVisualProperties( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteOpenBezierShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteRectangleShape( css::uno::Reference< css::drawing::XShape > xShape );

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
     *   <tr><td><tt>com.sun.star.drawing.ClosedBezierShape</tt></td>    <td>ShapeExport::WriteClosedBezierShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.CustomShape</tt></td>          <td>ShapeExport::WriteCustomShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.EllipseShape</tt></td>         <td>ShapeExport::WriteEllipseShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.GraphicObjectShape</tt></td>   <td>ShapeExport::WriteGraphicObjectShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.LineShape</tt></td>            <td>ShapeExport::WriteLineShape</td></tr>
     *   <tr><td><tt>com.sun.star.drawing.OpenBezierShape</tt></td>      <td>ShapeExport::WriteOpenBezierShape</td></tr>
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
    ShapeExport&       WriteShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&       WriteTextBox( css::uno::Reference< css::uno::XInterface > xIface, sal_Int32 nXmlNamespace );
    virtual ShapeExport&
                        WriteTextShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteTableShape( css::uno::Reference< css::drawing::XShape > xShape );
    ShapeExport&
                        WriteOLE2Shape( css::uno::Reference< css::drawing::XShape > xShape );
    virtual ShapeExport&
                        WriteUnknownShape( css::uno::Reference< css::drawing::XShape > xShape );

    void WriteTable( css::uno::Reference< css::drawing::XShape > rXShape );

    void WriteTableCellProperties(css::uno::Reference< css::beans::XPropertySet > rXPropSet);

    void WriteTableCellBorders(css::uno::Reference< css::beans::XPropertySet > rXPropSet);

    sal_Int32 GetNewShapeID( const css::uno::Reference< css::drawing::XShape > rShape );
    sal_Int32 GetNewShapeID( const css::uno::Reference< css::drawing::XShape > rShape, ::oox::core::XmlFilterBase* pFB );
    sal_Int32 GetShapeID( const css::uno::Reference< css::drawing::XShape > rShape );
    static sal_Int32 GetShapeID( const css::uno::Reference< css::drawing::XShape > rShape, ShapeHashMap* pShapeMap );
};

}}

#endif // INCLUDED_OOX_EXPORT_SHAPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
