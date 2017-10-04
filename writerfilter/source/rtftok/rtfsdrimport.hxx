/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFSDRIMPORT_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFSDRIMPORT_HXX

#include <stack>

#include <com/sun/star/drawing/XShapes.hpp>

#include <dmapper/GraphicZOrderHelper.hxx>
#include "rtfdocumentimpl.hxx"

namespace writerfilter
{
namespace rtftok
{
/// Handles the import of drawings using RTF markup.
class RTFSdrImport final
{
public:
    RTFSdrImport(RTFDocumentImpl& rDocument, css::uno::Reference<css::lang::XComponent> const& xDstDoc);
    ~RTFSdrImport();

    enum ShapeOrPict { SHAPE, PICT };
    void resolve(RTFShape& rShape, bool bClose, ShapeOrPict shapeOrPict);
    void close();
    void append(const OUString& aKey, const OUString& aValue);
    /// Append property on the current parent.
    void appendGroupProperty(const OUString& aKey, const OUString& aValue);
    void resolveDhgt(css::uno::Reference<css::beans::XPropertySet> const& xPropertySet, sal_Int32 nZOrder, bool bOldStyle);
    /// Set line color and line width on the shape, using the relevant API depending on if the shape is a text frame or not.
    static void resolveLineColorAndWidth(bool bTextFrame, const css::uno::Reference<css::beans::XPropertySet>& xPropertySet, css::uno::Any const& rLineColor, css::uno::Any const& rLineWidth);
    static void resolveFLine(css::uno::Reference<css::beans::XPropertySet> const& xPropertySet, sal_Int32 nFLine);
    /**
     * These are the default in Word, but not in Writer.
     *
     * @param bNew if the frame is new-style or old-style.
     */
    static std::vector<css::beans::PropertyValue> getTextFrameDefaults(bool bNew);
    /// Push a new group shape to the parent stack.
    void pushParent(css::uno::Reference<css::drawing::XShapes> const& xParent);
    /// Pop the current group shape from the parent stack.
    void popParent();
    css::uno::Reference<css::drawing::XShape> const& getCurrentShape()
    {
        return m_xShape;
    }
    bool isFakePict()
    {
        return m_bFakePict;
    }
private:
    void createShape(const OUString& rService, css::uno::Reference<css::drawing::XShape>& xShape, css::uno::Reference<css::beans::XPropertySet>& xPropertySet);
    void applyProperty(css::uno::Reference<css::drawing::XShape> const& xShape, const OUString& aKey, const OUString& aValue);
    int initShape(css::uno::Reference<css::drawing::XShape>& o_xShape,
                  css::uno::Reference<css::beans::XPropertySet>& o_xPropSet,
                  bool& o_rIsCustomShape,
                  RTFShape const& rShape, bool bClose, ShapeOrPict shapeOrPict);

    RTFDocumentImpl& m_rImport;
    std::stack< css::uno::Reference<css::drawing::XShapes> > m_aParents;
    css::uno::Reference<css::drawing::XShape> m_xShape;
    /// If m_xShape is imported as a Writer text frame (instead of a drawinglayer rectangle).
    bool m_bTextFrame;
    /// If m_xShape is imported as a Writer text graphic object (instead of a drawinglayer shape).
    bool m_bTextGraphicObject;
    /// if inside \pict, but actually it's a shape (not a picture)
    bool m_bFakePict;
    std::stack<writerfilter::dmapper::GraphicZOrderHelper> m_aGraphicZOrderHelpers;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFSDRIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
