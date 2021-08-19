/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_OOX_EXPORT_DMLPRESETSHAPEXPORT_HXX
#define INCLUDED_OOX_EXPORT_DMLPRESETSHAPEXPORT_HXX

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <string_view>

#include <oox/export/drawingml.hxx>

namespace com::sun::star::beans
{
struct PropertyValue;
}

namespace com::sun::star::drawing
{
class XShape;
struct EnhancedCustomShapeAdjustmentValue;
}

namespace oox::core
{
class XmlFilterBase;
}

namespace oox::drawingml
{
/// Class for exporting the custom shapes to OOXML preset ones, if possible.
/// This functionality needed for keeping the information for the office programs
/// about the shape type, and geometry data. Before these shapes were exported
/// with custom geometry, and they kept their geometry but has no information
/// about the shape itself. This lead to lost textbox size/position/padding for
/// example.
class DMLPresetShapeExporter
{
private:
    // the shape to export
    css::uno::Reference<css::drawing::XShape> m_xShape;
    // the DMLwriter
    DrawingML* m_pDMLexporter;
    // the type of the custom shape (diamond/rectangle/circle/triangle...)
    OUString m_sPresetShapeType;
    // True if the shape has points where its geometry can be modified
    bool m_bHasHandleValues;
    // The first the x the second the y coordinate, of flipping
    std::pair<bool, bool> m_bIsFlipped;

    // Custom Shape Geometry information for export:

    // The adjusting values stored in this sequence:
    css::uno::Sequence<css::drawing::EnhancedCustomShapeAdjustmentValue> m_AdjustmentValues;
    // Shapes what have adjusting points, the range of these points
    // and the index of the value stored in this sequence:
    css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>> m_HandleValues;

    //TODO:
    //css::awt::Rectangle m_ViewBox;
    //css::uno::Sequence<css::beans::PropertyValue> m_Path;
    //css::uno::Sequence<OUString> m_Equations;

public:
    DMLPresetShapeExporter() = delete;
    ~DMLPresetShapeExporter();

    DMLPresetShapeExporter(DrawingML* pDMLExporter,
                           css::uno::Reference<css::drawing::XShape> xShape);

    // Writes the preset shape to the xml
    bool WriteShape();

private:
    struct AdjustmentPointValueBase
    {
        std::optional<double> nMaxVal;
        std::optional<double> nMinVal;
        std::optional<double> nCurrVal;
    };

    typedef AdjustmentPointValueBase RadiusAdjustmentValue;
    typedef AdjustmentPointValueBase AngleAdjustmentValue;
    typedef AdjustmentPointValueBase XAdjustmentValue;
    typedef AdjustmentPointValueBase YAdjustmentValue;

    // Returns true, if the shape has adjusting points
    bool HasHandleValue() const;

    // Returns true if the shape flipped.
    bool IsXFlipped() const { return m_bIsFlipped.first; };
    bool IsYFlipped() const { return m_bIsFlipped.second; };

    // Returns with the shape type, like triangle for example
    const OUString& GetShapeType() const;
    // Returns with the handle points
    const css::uno::Sequence<css::uno::Sequence<css::beans::PropertyValue>>&
    GetHandleValues() const;
    // Returns with the adjustment values
    const css::uno::Sequence<css::drawing::EnhancedCustomShapeAdjustmentValue>&
    GetAdjustmentValues() const;
    // Returns with the raw value of the given property of the shape geometry.
    css::uno::Any GetHandleValueOfModificationPoint(sal_Int32 nPoint, std::u16string_view sType);
    // Returns with the appropriate value of the handle point.
    RadiusAdjustmentValue GetAdjustmentPointRadiusValue(sal_Int32 nPoint);
    AngleAdjustmentValue GetAdjustmentPointAngleValue(sal_Int32 nPoint);
    XAdjustmentValue GetAdjustmentPointXValue(sal_Int32 nPoint);
    YAdjustmentValue GetAdjustmentPointYValue(sal_Int32 nPoint);

    // Writes one adjustment point.
    bool WriteAV(const OUString& sValName, const OUString& sVal);
    // Opens/Closes the AVlist tag.
    bool StartAVListWriting();
    bool EndAVListWriting();

    // Finds the given value in the sequence
    static css::uno::Any FindHandleValue(css::uno::Sequence<css::beans::PropertyValue> aValues,
                                         std::u16string_view sKey);
    // Writes and converts the adjustment points from sdr to ooxml ones per shape type.
    bool WriteShapeWithAVlist();

}; // end of DMLPresetShapeExporter class

} // end of namespace oox::drawingml

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
