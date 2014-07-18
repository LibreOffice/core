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

#include <rtfdocumentimpl.hxx>

namespace writerfilter
{
namespace rtftok
{
/// Handles the import of drawings using RTF markup.
class RTFSdrImport
{
public:
    RTFSdrImport(RTFDocumentImpl& rImport, css::uno::Reference<css::lang::XComponent> const& xDstDoc);
    virtual ~RTFSdrImport();

    enum ShapeOrPict { SHAPE, PICT };
    void resolve(RTFShape& rShape, bool bClose, ShapeOrPict shapeOrPict);
    void close();
    void append(const OUString& aKey, const OUString& aValue);
    /// Append property on the current parent.
    void appendGroupProperty(const OUString& aKey, const OUString& aValue);
    void resolveDhgt(css::uno::Reference<css::beans::XPropertySet> xPropertySet, sal_Int32 nZOrder, bool bOldStyle);
    void resolveFLine(css::uno::Reference<css::beans::XPropertySet> xPropertySet, sal_Int32 nFLine);
    /**
     * These are the default in Word, but not in Writer.
     *
     * @param bNew if the frame is new-style or old-style.
     */
    std::vector<css::beans::PropertyValue> getTextFrameDefaults(bool bNew);
    /// Push a new group shape to the parent stack.
    void pushParent(css::uno::Reference<css::drawing::XShapes> xParent);
    /// Pop the current group shape from the parent stack.
    void popParent();
    css::uno::Reference<css::drawing::XShape> const& getCurrentShape()
        { return m_xShape; }
    bool isFakePict() { return m_bFakePict; }
private:
    void createShape(const OUString& aService, css::uno::Reference<css::drawing::XShape>& xShape, css::uno::Reference<css::beans::XPropertySet>& xPropertySet);
    void applyProperty(css::uno::Reference<css::drawing::XShape> xShape, const OUString& aKey, const OUString& aValue);
    int initShape(
        css::uno::Reference<css::drawing::XShape> & o_xShape,
        css::uno::Reference<css::beans::XPropertySet> & o_xPropSet,
        bool & o_rIsCustomShape,
        RTFShape const& rShape, bool bClose, ShapeOrPict const shapeOrPict);

    RTFDocumentImpl& m_rImport;
    std::stack< css::uno::Reference<css::drawing::XShapes> > m_aParents;
    css::uno::Reference<css::drawing::XShape> m_xShape;
    /// If m_xShape is imported as a Writer text frame (instead of a drawinglayer rectangle).
    bool m_bTextFrame;
    /// if inside \pict, but actually it's a shape (not a picture)
    bool m_bFakePict;
};
} // namespace rtftok
} // namespace writerfilter

#endif // _RTFSDRIPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
