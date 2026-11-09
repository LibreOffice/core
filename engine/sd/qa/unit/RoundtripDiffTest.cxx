/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* A self-labelling fidelity harness for the pptx filter.
 *
 * For every document of a corpus we take three model dumps:
 *
 *   D  --import-->  A   --export/import-->  B   --export/import-->  C
 *
 * A != B means the export/import pair loses or changes something.  B != C is
 * worse: the input of that second round trip is a file we wrote ourselves, so
 * our export is not even a fixed point of our own import.  Neither comparison
 * needs a reference file or a human, which is what lets this run over hundreds
 * of documents and produce a frequency ranked list of what the filter loses.
 *
 * Driven by the environment:
 *
 *   SD_RT_RUN      run at all; without it, and without SD_RT_CORPUS, this is a no-op
 *   SD_RT_CORPUS   directory of documents to process (default sd/qa/unit/data/pptx)
 *                  A document is read and written in the format its name carries, so a folder
 *                  can hold both the current presentation format and the older binary one
 *   SD_RT_REPORT   report file, appended to and resumable (default sd-roundtrip.log)
 *   SD_RT_LIMIT    stop after that many documents
 *   SD_RT_DUMPDIR  if set, write the A, B and C dumps of every document there
 *   SD_RT_LAYOUT   also lay every page out and keep what it draws: where every piece of text
 *                  and every shape, picture and fill lands, how large it is and in what colour.
 *                  This finds what the model holds correctly but the page no longer draws the
 *                  same way
 */

#include <sal/config.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <string_view>
#include <vector>

#include <o3tl/string_view.hxx>

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XRenderable.hpp>

#include <comphelper/anytostring.hxx>
#include <comphelper/propertyvalue.hxx>
#include <osl/file.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>

using namespace css;
using namespace ::cpo;

namespace
{
/** Longest value we keep; a few properties (bitmaps, geometry) are enormous. */
constexpr sal_Int32 MAX_VALUE_LEN = 400;

/** How many drawn things a page may hold and still have each of them written down. A page that
    draws more than this, such as one carrying a map or another large drawing, is counted by kind
    instead, because naming every piece of it says no more than the count does. */
constexpr size_t MAX_DRAWN_PER_PAGE = 400;

/** How many pieces drawn one after another are taken to be one fill painted in many pieces
    rather than that many things of their own. */
constexpr size_t MIN_REPEATED_PIECES = 16;

/** Two numbers this close count as the same value. The slack is the larger of a part of the
    value and a floor the caller gives, which is what lets a length be compared more loosely than
    a count. */
bool numbersEqual(double fLeft, double fRight, double fFloor)
{
    const double fMagnitude = std::max(std::abs(fLeft), std::abs(fRight));
    return std::abs(fLeft - fRight) <= std::max(fFloor, 0.002 * fMagnitude);
}

/** The floor in hundredths of a millimetre below which a difference in a length of a laid out
    page says nothing. Saving a document rounds every coordinate once, and a tenth of a
    millimetre is finer than anything a reader can see. */
constexpr double LAYOUT_SLACK = 10.0;

/** The slack a value at this key is compared with. */
double slackOf(std::string_view rKey)
{
    if (!rKey.starts_with("layout/"))
        return 1.0;
    const size_t nAt = rKey.rfind('@');
    if (nAt == std::string_view::npos)
        return 1.0;
    const std::string_view aProperty = rKey.substr(nAt + 1);
    if (aProperty == "position" || aProperty == "size" || aProperty == "width"
        || aProperty == "height")
        return LAYOUT_SLACK;
    return 1.0;
}

/** Properties that differ between two saves of the same model for reasons that
    say nothing about fidelity: names we generate ourselves, interop leftovers. */
bool isNoise(const OUString& rName)
{
    static const std::set<OUString> aNoise{
        // Names we generate ourselves when the model is filled in.
        u"FillGradientName"_ustr, u"FillHatchName"_ustr, u"FillBitmapName"_ustr,
        u"FillTransparenceGradientName"_ustr, u"LineDashName"_ustr,
        // Rendered previews, which differ in a byte or two every time they are
        // produced, and a shape identifier that is freshly drawn on every import.
        u"MetaFile"_ustr, u"Preview"_ustr, u"Guid"_ustr,
        // Display strings that only repeat the shape name.
        u"UINameSingular"_ustr, u"UINamePlural"_ustr, u"LinkDisplayName"_ustr,
        // Carried through the round trip untouched by design.
        u"InteropGrabBag"_ustr, u"UserDefinedAttributes"_ustr
    };
    return aNoise.count(rName) != 0;
}

/** Walks a drawing model and writes every reachable property into a flat map
    keyed by the path of the object it belongs to.  Deliberately generic: it asks
    XPropertySetInfo what is there instead of naming properties, so it never
    falls behind the model the way a hand written dumper does. */
class ModelDumper
{
public:
    std::map<OString, OUString> maValues;

    void dump(const uno::Reference<lang::XComponent>& xComponent)
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDrawPages(xComponent, uno::UNO_QUERY);
        if (xDrawPages.is())
            dumpPages(xDrawPages->getDrawPages(), "page");

        uno::Reference<drawing::XMasterPagesSupplier> xMasterPages(xComponent, uno::UNO_QUERY);
        if (xMasterPages.is())
            dumpPages(xMasterPages->getMasterPages(), "master");
    }

private:
    static OString indexed(std::string_view rBase, std::string_view rTag, sal_Int32 nIndex)
    {
        const OString aIndex = "[" + OString::number(nIndex) + "]";
        if (rBase.empty())
            return OString::Concat(rTag) + aIndex;
        return OString::Concat(rBase) + "/" + rTag + aIndex;
    }

    void add(std::string_view rPath, std::string_view rName, const OUString& rValue)
    {
        maValues[OString::Concat(rPath) + "@" + rName]
            = rValue.getLength() > MAX_VALUE_LEN ? rValue.copy(0, MAX_VALUE_LEN) : rValue;
    }

    void dumpPages(const uno::Reference<drawing::XDrawPages>& xPages, const char* pTag)
    {
        if (!xPages.is())
            return;
        for (sal_Int32 i = 0; i < xPages->getCount(); ++i)
        {
            OString aPath = indexed({}, pTag, i);
            uno::Reference<drawing::XDrawPage> xPage(xPages->getDrawPageByIndex(i));
            if (!xPage.is())
                continue;
            dumpProperties(uno::Reference<beans::XPropertySet>(xPage, uno::UNO_QUERY), aPath);
            dumpShapes(xPage, aPath);

            uno::Reference<presentation::XPresentationPage> xPresentationPage(xPage,
                                                                              uno::UNO_QUERY);
            if (xPresentationPage.is())
            {
                uno::Reference<drawing::XDrawPage> xNotes(xPresentationPage->getNotesPage());
                if (xNotes.is())
                {
                    const OString aNotesPath = aPath + "/notes";
                    dumpProperties(uno::Reference<beans::XPropertySet>(xNotes, uno::UNO_QUERY),
                                   aNotesPath);
                    dumpShapes(xNotes, aNotesPath);
                }
            }
        }
    }

    void dumpShapes(const uno::Reference<drawing::XShapes>& xShapes, std::string_view rPath)
    {
        if (!xShapes.is())
            return;
        add(rPath, "ShapeCount", OUString::number(xShapes->getCount()));
        for (sal_Int32 i = 0; i < xShapes->getCount(); ++i)
        {
            uno::Reference<drawing::XShape> xShape(xShapes->getShapeByIndex(i), uno::UNO_QUERY);
            if (!xShape.is())
                continue;
            OString aPath = indexed(rPath, "shape", i);
            add(aPath, "ShapeType", xShape->getShapeType());
            dumpProperties(uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY), aPath);

            uno::Reference<drawing::XShapes> xChildren(xShape, uno::UNO_QUERY);
            if (xChildren.is())
                dumpShapes(xChildren, aPath);
            else
            {
                dumpText(uno::Reference<text::XText>(xShape, uno::UNO_QUERY), aPath);
                dumpTable(uno::Reference<beans::XPropertySet>(xShape, uno::UNO_QUERY), aPath);
            }
        }
    }

    /** A table holds its text in cells of its own, which the text of the shape
        around them does not reach.  Walk the cells, their borders and fills. */
    void dumpTable(const uno::Reference<beans::XPropertySet>& xShapeProps, std::string_view rPath)
    {
        if (!xShapeProps.is())
            return;
        uno::Reference<beans::XPropertySetInfo> xInfo(xShapeProps->getPropertySetInfo());
        if (!xInfo.is() || !xInfo->hasPropertyByName(u"Model"_ustr))
            return;

        uno::Reference<table::XTable> xTable;
        try
        {
            xShapeProps->getPropertyValue(u"Model"_ustr) >>= xTable;
        }
        catch (const uno::Exception&)
        {
            return;
        }
        if (!xTable.is())
            return;

        const sal_Int32 nRows = xTable->getRowCount();
        const sal_Int32 nColumns = xTable->getColumnCount();
        add(rPath, "RowCount", OUString::number(nRows));
        add(rPath, "ColumnCount", OUString::number(nColumns));

        dumpLine(xTable->getRows(), rPath, "row");
        dumpLine(xTable->getColumns(), rPath, "column");

        for (sal_Int32 nRow = 0; nRow < nRows; ++nRow)
        {
            for (sal_Int32 nColumn = 0; nColumn < nColumns; ++nColumn)
            {
                uno::Reference<table::XCell> xCell;
                try
                {
                    xCell = xTable->getCellByPosition(nColumn, nRow);
                }
                catch (const uno::Exception&)
                {
                    continue;
                }
                if (!xCell.is())
                    continue;
                const OString aPath = indexed(indexed(rPath, "row", nRow), "cell", nColumn);
                dumpProperties(uno::Reference<beans::XPropertySet>(xCell, uno::UNO_QUERY), aPath);
                dumpText(uno::Reference<text::XText>(xCell, uno::UNO_QUERY), aPath);
            }
        }
    }

    void dumpLine(const uno::Reference<container::XIndexAccess>& xLines, std::string_view rPath,
                  const char* pTag)
    {
        if (!xLines.is())
            return;
        for (sal_Int32 i = 0; i < xLines->getCount(); ++i)
        {
            uno::Reference<beans::XPropertySet> xLine(xLines->getByIndex(i), uno::UNO_QUERY);
            if (xLine.is())
                dumpProperties(xLine, indexed(rPath, pTag, i));
        }
    }

    /** The hand written XShapeDumper takes shape text as one flat string, which
        makes every loss of character formatting invisible.  Go down to runs. */
    void dumpText(const uno::Reference<text::XText>& xText, std::string_view rPath)
    {
        uno::Reference<container::XEnumerationAccess> xParagraphs(xText, uno::UNO_QUERY);
        if (!xParagraphs.is())
            return;

        uno::Reference<container::XEnumeration> xParagraphEnum;
        try
        {
            xParagraphEnum = xParagraphs->createEnumeration();
        }
        catch (...)
        {
            return;
        }

        sal_Int32 nParagraphCount = 0;
        for (sal_Int32 nParagraph = 0; xParagraphEnum.is() && xParagraphEnum->hasMoreElements();
             ++nParagraph)
        {
            nParagraphCount = nParagraph + 1;
            uno::Reference<beans::XPropertySet> xParagraph(xParagraphEnum->nextElement(),
                                                           uno::UNO_QUERY);
            if (!xParagraph.is())
                continue;
            OString aPath = indexed(rPath, "para", nParagraph);
            dumpProperties(xParagraph, aPath);


            uno::Reference<container::XEnumerationAccess> xRuns(xParagraph, uno::UNO_QUERY);
            if (!xRuns.is())
                continue;
            uno::Reference<container::XEnumeration> xRunEnum = xRuns->createEnumeration();
            sal_Int32 nRunCount = 0;
            for (sal_Int32 nRun = 0; xRunEnum.is() && xRunEnum->hasMoreElements(); ++nRun)
            {
                nRunCount = nRun + 1;
                uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
                if (!xRun.is())
                    continue;
                OString aRunPath = indexed(aPath, "run", nRun);
                add(aRunPath, "Text", xRun->getString());
                dumpProperties(uno::Reference<beans::XPropertySet>(xRun, uno::UNO_QUERY),
                               aRunPath);
            }
            add(aPath, "RunCount", OUString::number(nRunCount));
        }
        add(rPath, "ParagraphCount", OUString::number(nParagraphCount));
    }

    void dumpProperties(const uno::Reference<beans::XPropertySet>& xPropertySet,
                        std::string_view rPath)
    {
        if (!xPropertySet.is())
            return;
        uno::Reference<beans::XPropertySetInfo> xInfo;
        try
        {
            xInfo = xPropertySet->getPropertySetInfo();
        }
        catch (...)
        {
            return;
        }
        if (!xInfo.is())
            return;

        for (const beans::Property& rProperty : xInfo->getProperties())
        {
            if (rProperty.Type.getTypeClass() == cpo::uno::TypeClass_INTERFACE
                || isNoise(rProperty.Name))
                continue;

            cpo::uno::Any aValue;
            try
            {
                aValue = xPropertySet->getPropertyValue(rProperty.Name);
            }
            catch (...)
            {
                continue;
            }
            if (!aValue.hasValue() || aValue.getValueTypeClass() == cpo::uno::TypeClass_INTERFACE)
                continue;

            add(rPath, OUStringToOString(rProperty.Name, RTL_TEXTENCODING_UTF8),
                comphelper::anyToString(aValue));
        }
    }
};

/** The name under which drawing operations of one kind are counted, or nothing for a kind that
    says little about the page. */
const char* operationGroup(MetaActionType eType)
{
    switch (eType)
    {
        case MetaActionType::TEXT:
        case MetaActionType::TEXTARRAY:
        case MetaActionType::STRETCHTEXT:
            return "text";
        case MetaActionType::POLYLINE:
        case MetaActionType::POLYGON:
        case MetaActionType::POLYPOLYGON:
        case MetaActionType::Transparent:
            return "polygon";
        case MetaActionType::RECT:
        case MetaActionType::ROUNDRECT:
            return "rectangle";
        case MetaActionType::ELLIPSE:
            return "ellipse";
        case MetaActionType::ARC:
        case MetaActionType::PIE:
        case MetaActionType::CHORD:
            return "arc";
        case MetaActionType::LINE:
            return "line";
        case MetaActionType::BMP:
        case MetaActionType::BMPSCALE:
        case MetaActionType::BMPSCALEPART:
        case MetaActionType::BMPEX:
        case MetaActionType::BMPEXSCALE:
        case MetaActionType::BMPEXSCALEPART:
        case MetaActionType::MASK:
        case MetaActionType::MASKSCALE:
        case MetaActionType::MASKSCALEPART:
            return "image";
        case MetaActionType::GRADIENT:
        case MetaActionType::GRADIENTEX:
            return "gradient";
        case MetaActionType::HATCH:
            return "hatch";
        case MetaActionType::FLOATTRANSPARENT:
            return "transparency";
        default:
            return nullptr;
    }
}

/** One drawn thing held back until the things after it say whether it stands on its own or is
    one piece of a fill painted in many pieces. A plain fill is a shape that carries a colour and
    no outline, which is what the bands of a gradient are. */
struct PendingDraw
{
    const char* pGroup;
    tools::Rectangle aArea;
    OUString aFill;
    OUString aLine;
    bool bPlainFill;
};

/** The colours and the font size drawing operations are carried out with at one point of a
    metafile. The whole of it is kept where a push is met and given back at the matching pop,
    which is coarser than the groups a push names and is the same on both sides of a comparison.
    A colour counts as in force only where the operation that set it was switching it on. */
struct PaintState
{
    Color aLineColour = COL_BLACK;
    Color aFillColour = COL_WHITE;
    Color aTextColour = COL_BLACK;
    bool bLineInForce = false;
    bool bFillInForce = false;
    tools::Long nFontHeight = 0;
};

/** A colour as it can stand in a value, in the usual six hexadecimal digits. */
OUString colourText(const Color& rColour) { return "#" + rColour.AsRGBHexString(); }

/** A length rounded to the half millimetre, so that the rounding a save puts on a coordinate
    leaves a drawn thing the same name it had before. */
tools::Long roundedLength(tools::Long nLength)
{
    return (nLength + (nLength < 0 ? -25 : 25)) / 50 * 50;
}

/** The rectangle a drawing operation covers, for an operation that covers an area of the page.
    Gives false for one that leaves no area of its own, among them the operations that only
    change the state the next ones are carried out in. */
bool operationArea(MetaAction& rAction, const OutputDevice& rDevice, tools::Rectangle& rArea)
{
    auto fromPointAndSize = [&rArea](const Point& rPoint, const Size& rSize) {
        rArea = tools::Rectangle(rPoint, rSize);
        rArea.Normalize();
        return true;
    };

    switch (rAction.GetType())
    {
        case MetaActionType::RECT:
            rArea = static_cast<MetaRectAction&>(rAction).GetRect();
            return true;
        case MetaActionType::ROUNDRECT:
            rArea = static_cast<MetaRoundRectAction&>(rAction).GetRect();
            return true;
        case MetaActionType::ELLIPSE:
            rArea = static_cast<MetaEllipseAction&>(rAction).GetRect();
            return true;
        case MetaActionType::ARC:
            rArea = static_cast<MetaArcAction&>(rAction).GetRect();
            return true;
        case MetaActionType::PIE:
            rArea = static_cast<MetaPieAction&>(rAction).GetRect();
            return true;
        case MetaActionType::CHORD:
            rArea = static_cast<MetaChordAction&>(rAction).GetRect();
            return true;
        case MetaActionType::LINE:
        {
            auto& rLine = static_cast<MetaLineAction&>(rAction);
            rArea = tools::Rectangle(rLine.GetStartPoint(), rLine.GetEndPoint());
            rArea.Normalize();
            return true;
        }
        case MetaActionType::POLYLINE:
            rArea = static_cast<MetaPolyLineAction&>(rAction).GetPolygon().GetBoundRect();
            return true;
        case MetaActionType::POLYGON:
            rArea = static_cast<MetaPolygonAction&>(rAction).GetPolygon().GetBoundRect();
            return true;
        case MetaActionType::POLYPOLYGON:
            rArea = static_cast<MetaPolyPolygonAction&>(rAction).GetPolyPolygon().GetBoundRect();
            return true;
        case MetaActionType::Transparent:
            rArea = static_cast<MetaTransparentAction&>(rAction).GetPolyPolygon().GetBoundRect();
            return true;
        case MetaActionType::HATCH:
            rArea = static_cast<MetaHatchAction&>(rAction).GetPolyPolygon().GetBoundRect();
            return true;
        case MetaActionType::GRADIENT:
            rArea = static_cast<MetaGradientAction&>(rAction).GetRect();
            return true;
        case MetaActionType::GRADIENTEX:
            rArea = static_cast<MetaGradientExAction&>(rAction).GetPolyPolygon().GetBoundRect();
            return true;
        case MetaActionType::FLOATTRANSPARENT:
        {
            auto& rFloat = static_cast<MetaFloatTransparentAction&>(rAction);
            return fromPointAndSize(rFloat.GetPoint(), rFloat.GetSize());
        }
        case MetaActionType::BMP:
        {
            auto& rBitmap = static_cast<MetaBmpAction&>(rAction);
            return fromPointAndSize(rBitmap.GetPoint(),
                                    rDevice.PixelToLogic(rBitmap.GetBitmap().GetSizePixel()));
        }
        case MetaActionType::BMPEX:
        {
            auto& rBitmap = static_cast<MetaBmpExAction&>(rAction);
            return fromPointAndSize(rBitmap.GetPoint(),
                                    rDevice.PixelToLogic(rBitmap.GetBitmap().GetSizePixel()));
        }
        case MetaActionType::MASK:
        {
            auto& rBitmap = static_cast<MetaMaskAction&>(rAction);
            return fromPointAndSize(rBitmap.GetPoint(),
                                    rDevice.PixelToLogic(rBitmap.GetBitmap().GetSizePixel()));
        }
        case MetaActionType::BMPSCALE:
        {
            auto& rBitmap = static_cast<MetaBmpScaleAction&>(rAction);
            return fromPointAndSize(rBitmap.GetPoint(), rBitmap.GetSize());
        }
        case MetaActionType::BMPEXSCALE:
        {
            auto& rBitmap = static_cast<MetaBmpExScaleAction&>(rAction);
            return fromPointAndSize(rBitmap.GetPoint(), rBitmap.GetSize());
        }
        case MetaActionType::MASKSCALE:
        {
            auto& rBitmap = static_cast<MetaMaskScaleAction&>(rAction);
            return fromPointAndSize(rBitmap.GetPoint(), rBitmap.GetSize());
        }
        case MetaActionType::BMPSCALEPART:
        {
            auto& rBitmap = static_cast<MetaBmpScalePartAction&>(rAction);
            return fromPointAndSize(rBitmap.GetDestPoint(), rBitmap.GetDestSize());
        }
        case MetaActionType::BMPEXSCALEPART:
        {
            auto& rBitmap = static_cast<MetaBmpExScalePartAction&>(rAction);
            return fromPointAndSize(rBitmap.GetDestPoint(), rBitmap.GetDestSize());
        }
        case MetaActionType::MASKSCALEPART:
        {
            auto& rBitmap = static_cast<MetaMaskScalePartAction&>(rAction);
            return fromPointAndSize(rBitmap.GetDestPoint(), rBitmap.GetDestSize());
        }
        default:
            return false;
    }
}

/** Whether a drawing operation paints a picture of its own, so that the colours in force around
    it say nothing about what it looks like. */
bool paintsItsOwnPicture(MetaActionType eType)
{
    switch (eType)
    {
        case MetaActionType::BMP:
        case MetaActionType::BMPSCALE:
        case MetaActionType::BMPSCALEPART:
        case MetaActionType::BMPEX:
        case MetaActionType::BMPEXSCALE:
        case MetaActionType::BMPEXSCALEPART:
        case MetaActionType::MASK:
        case MetaActionType::MASKSCALE:
        case MetaActionType::MASKSCALEPART:
        case MetaActionType::FLOATTRANSPARENT:
            return true;
        default:
            return false;
    }
}

/** What fills the area a drawing operation covers, or nothing where the operation paints a
    picture of its own. A gradient carries the two colours it runs between. */
OUString fillOfOperation(MetaAction& rAction, const PaintState& rState)
{
    if (paintsItsOwnPicture(rAction.GetType()))
        return OUString();

    switch (rAction.GetType())
    {
        case MetaActionType::GRADIENT:
        {
            const Gradient& rGradient = static_cast<MetaGradientAction&>(rAction).GetGradient();
            return colourText(rGradient.GetStartColor()) + " to "
                   + colourText(rGradient.GetEndColor());
        }
        case MetaActionType::GRADIENTEX:
        {
            const Gradient& rGradient = static_cast<MetaGradientExAction&>(rAction).GetGradient();
            return colourText(rGradient.GetStartColor()) + " to "
                   + colourText(rGradient.GetEndColor());
        }
        default:
            return rState.bFillInForce ? colourText(rState.aFillColour) : u"none"_ustr;
    }
}

/** A piece of text as it can stand in a key: one line, printable, and short. */
OString keyText(const OUString& rText)
{
    OUStringBuffer aBuffer(rText.getLength());
    for (sal_Int32 i = 0; i < rText.getLength() && aBuffer.getLength() < 40; ++i)
    {
        const sal_Unicode c = rText[i];
        aBuffer.append(c < 0x20 || c == 0x7f || c == '\t' ? u' ' : c);
    }
    return OUStringToOString(o3tl::trim(aBuffer.makeStringAndClear()), RTL_TEXTENCODING_UTF8);
}

/** Writes what every page draws once it has been laid out.

    Each page is drawn into a metafile, which holds the drawing operations in the order they are
    carried out. A piece of text is keyed by the text itself and anything else by its kind and its
    size, so that a page whose content shifts reads as a move rather than as one thing lost and
    another gained. Each one keeps where it landed, what colour it was drawn in and how large it
    is. Beside them is the number of things of each kind, which is what shows a shape that stopped
    being drawn even though the model still holds it. A fill painted in many pieces counts once,
    as the one thing it is. */
void dumpLayout(std::map<OString, OUString>& rValues,
                const uno::Reference<lang::XComponent>& xComponent)
{
    uno::Reference<view::XRenderable> xRenderable(xComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(xComponent, uno::UNO_QUERY);
    if (!xRenderable.is() || !xModel.is())
        return;

    const uno::Any aSelection(xModel);
    const uno::Sequence<beans::PropertyValue> aNoOptions;
    sal_Int32 nPages = 0;
    try
    {
        nPages = xRenderable->getRendererCount(aSelection, aNoOptions);
    }
    catch (const uno::Exception&)
    {
        return;
    }

    for (sal_Int32 nPage = 0; nPage < nPages; ++nPage)
    {
        ScopedVclPtrInstance<VirtualDevice> pDevice;
        pDevice->EnableOutput(false);
        pDevice->SetMapMode(MapMode(MapUnit::Map100thMM));

        GDIMetaFile aMetaFile;
        aMetaFile.SetPrefMapMode(pDevice->GetMapMode());
        aMetaFile.Record(pDevice);

        rtl::Reference<VCLXDevice> xDevice(new VCLXDevice);
        xDevice->SetOutputDevice(pDevice);
        const uno::Sequence<beans::PropertyValue> aOptions{ comphelper::makePropertyValue(
            u"RenderDevice"_ustr, uno::Reference<awt::XDevice>(xDevice)) };

        try
        {
            xRenderable->render(nPage, aSelection, aOptions);
        }
        catch (const uno::Exception&)
        {
        }
        aMetaFile.Stop();

        const OString aPagePath = "layout/page[" + OString::number(nPage) + "]";
        std::map<OString, sal_Int32> aSeen;
        std::map<OString, sal_Int32> aCounts;
        PaintState aState;
        std::vector<PaintState> aStateStack;
        std::vector<PendingDraw> aRun;
        std::map<OString, OUString> aDrawnValues;
        size_t nDrawn = 0;

        // Writes one drawn thing. Its name is its kind and its size rounded to the half
        // millimetre, which is what lets the same thing be found again on the other side of a
        // round trip once it has moved.
        auto recordDrawn = [&](std::string_view rGroup, const tools::Rectangle& rArea,
                               const OUString& rFill, const OUString& rLine) {
            ++aCounts[OString(rGroup)];
            ++nDrawn;
            if (nDrawn > MAX_DRAWN_PER_PAGE)
                return;

            const OString aSubject = OString::Concat(rGroup) + "["
                                     + OString::number(roundedLength(rArea.GetWidth())) + "x"
                                     + OString::number(roundedLength(rArea.GetHeight())) + "]";
            const OString aPath
                = aPagePath + "/" + aSubject + "[" + OString::number(aSeen[aSubject]++) + "]";
            aDrawnValues[aPath + "@position"]
                = OUString::number(rArea.Left()) + "," + OUString::number(rArea.Top());
            aDrawnValues[aPath + "@size"]
                = OUString::number(rArea.GetWidth()) + "x" + OUString::number(rArea.GetHeight());
            if (!rFill.isEmpty())
                aDrawnValues[aPath + "@fill"] = rFill;
            if (!rLine.isEmpty())
                aDrawnValues[aPath + "@line"] = rLine;
        };

        // Writes the things held back so far. A long enough run of plain fills in changing
        // colours is the bands one gradient is painted in, and a long enough run of pieces all
        // of one size is one fill laid out as a pattern. Either goes down as the area the whole
        // run covers and how many pieces it took.
        auto flushRun = [&]() {
            if (aRun.empty())
                return;

            const bool bLongEnough = aRun.size() >= MIN_REPEATED_PIECES;
            bool bAllPlainFills = bLongEnough;
            bool bAllOneSize = bLongEnough;
            for (const auto& rPiece : aRun)
            {
                bAllPlainFills = bAllPlainFills && rPiece.bPlainFill;
                bAllOneSize = bAllOneSize
                              && roundedLength(rPiece.aArea.GetWidth())
                                     == roundedLength(aRun.front().aArea.GetWidth())
                              && roundedLength(rPiece.aArea.GetHeight())
                                     == roundedLength(aRun.front().aArea.GetHeight());
            }
            const bool bBands = bAllPlainFills && aRun.front().aFill != aRun.back().aFill;

            if (bBands || bAllOneSize)
            {
                tools::Rectangle aWhole = aRun.front().aArea;
                for (const auto& rPiece : aRun)
                    aWhole.Union(rPiece.aArea);

                OString aGroup("gradient band"_ostr);
                if (!bBands)
                    aGroup = OString::Concat("repeated ") + aRun.front().pGroup;
                const OUString aFill
                    = aRun.front().aFill == aRun.back().aFill
                          ? aRun.front().aFill
                          : aRun.front().aFill + " to " + aRun.back().aFill;
                recordDrawn(aGroup, aWhole, aFill, aRun.front().aLine);
            }
            else
            {
                for (const auto& rPiece : aRun)
                    recordDrawn(rPiece.pGroup, rPiece.aArea, rPiece.aFill, rPiece.aLine);
            }
            aRun.clear();
        };

        for (size_t nAction = 0; nAction < aMetaFile.GetActionSize(); ++nAction)
        {
            MetaAction* pAction = aMetaFile.GetAction(nAction);
            if (!pAction)
                continue;

            switch (pAction->GetType())
            {
                case MetaActionType::PUSH:
                    aStateStack.push_back(aState);
                    continue;
                case MetaActionType::POP:
                    if (!aStateStack.empty())
                    {
                        aState = aStateStack.back();
                        aStateStack.pop_back();
                    }
                    continue;
                case MetaActionType::LINECOLOR:
                {
                    auto* pColour = static_cast<MetaLineColorAction*>(pAction);
                    aState.bLineInForce = pColour->IsSetting();
                    aState.aLineColour = pColour->GetColor();
                    continue;
                }
                case MetaActionType::FILLCOLOR:
                {
                    auto* pColour = static_cast<MetaFillColorAction*>(pAction);
                    aState.bFillInForce = pColour->IsSetting();
                    aState.aFillColour = pColour->GetColor();
                    continue;
                }
                case MetaActionType::TEXTCOLOR:
                    aState.aTextColour = static_cast<MetaTextColorAction*>(pAction)->GetColor();
                    continue;
                case MetaActionType::FONT:
                    aState.nFontHeight
                        = static_cast<MetaFontAction*>(pAction)->GetFont().GetFontHeight();
                    continue;
                default:
                    break;
            }

            const char* pGroup = operationGroup(pAction->GetType());
            if (!pGroup)
                continue;
            if (std::string_view(pGroup) == "text")
                ++aCounts["text"_ostr];

            OUString aText;
            Point aPoint;
            double fWidth = -1;
            switch (pAction->GetType())
            {
                case MetaActionType::TEXT:
                {
                    auto* pText = static_cast<MetaTextAction*>(pAction);
                    aText = pText->GetText().copy(pText->GetIndex(), pText->GetLen());
                    aPoint = pText->GetPoint();
                    break;
                }
                case MetaActionType::TEXTARRAY:
                {
                    auto* pText = static_cast<MetaTextArrayAction*>(pAction);
                    aText = pText->GetText().copy(pText->GetIndex(), pText->GetLen());
                    aPoint = pText->GetPoint();
                    if (!pText->GetDXArray().empty())
                        fWidth = pText->GetDXArray().back();
                    break;
                }
                case MetaActionType::STRETCHTEXT:
                {
                    auto* pText = static_cast<MetaStretchTextAction*>(pAction);
                    aText = pText->GetText().copy(pText->GetIndex(), pText->GetLen());
                    aPoint = pText->GetPoint();
                    fWidth = pText->GetWidth();
                    break;
                }
                default:
                {
                    tools::Rectangle aArea;
                    if (!operationArea(*pAction, *pDevice, aArea))
                    {
                        flushRun();
                        continue;
                    }

                    if (!aRun.empty() && std::string_view(aRun.front().pGroup) != pGroup)
                        flushRun();

                    OUString aLine;
                    if (!paintsItsOwnPicture(pAction->GetType()))
                        aLine = aState.bLineInForce ? colourText(aState.aLineColour)
                                                    : u"none"_ustr;
                    const bool bPlainFill
                        = aState.bFillInForce && !aState.bLineInForce
                          && (pAction->GetType() == MetaActionType::POLYGON
                              || pAction->GetType() == MetaActionType::POLYPOLYGON);
                    aRun.push_back({ pGroup, aArea, fillOfOperation(*pAction, aState), aLine,
                                     bPlainFill });
                    continue;
                }
            }

            flushRun();

            const OString aKey = keyText(aText);
            if (aKey.isEmpty())
                continue;
            const OString aSubject = "text[" + aKey + "]";
            const OString aPath
                = aPagePath + "/" + aSubject + "[" + OString::number(aSeen[aSubject]++) + "]";
            rValues[aPath + "@position"]
                = OUString::number(aPoint.X()) + "," + OUString::number(aPoint.Y());
            rValues[aPath + "@colour"] = colourText(aState.aTextColour);
            rValues[aPath + "@height"] = OUString::number(aState.nFontHeight);
            if (fWidth >= 0)
                rValues[aPath + "@width"] = OUString::number(static_cast<sal_Int32>(fWidth));
        }

        flushRun();

        if (nDrawn <= MAX_DRAWN_PER_PAGE)
            rValues.insert(aDrawnValues.begin(), aDrawnValues.end());

        for (const auto& rCount : aCounts)
            rValues[aPagePath + "@count[" + rCount.first + "]"]
                = OUString::number(rCount.second);
    }
}

enum class DiffKind
{
    Lost,
    Added,
    Changed
};

struct Diff
{
    DiffKind eKind;
    OString aKey;
    OUString aBefore;
    OUString aAfter;
};

/** Compares two values number aware, so that a position expressed in EMU and
    converted back through 1/100 mm does not show up as a difference. */
bool valuesEqual(const OUString& rLeft, const OUString& rRight, double fFloor)
{
    if (rLeft == rRight)
        return true;

    sal_Int32 nLeft = 0, nRight = 0;
    const sal_Int32 nLeftLen = rLeft.getLength(), nRightLen = rRight.getLength();

    auto isNumberStart = [](const OUString& rText, sal_Int32 nPos) {
        sal_Unicode c = rText[nPos];
        if (rtl::isAsciiDigit(c))
            return true;
        return (c == '-' || c == '+') && nPos + 1 < rText.getLength()
               && rtl::isAsciiDigit(rText[nPos + 1]);
    };
    auto readNumber = [](const OUString& rText, sal_Int32& rPos) {
        sal_Int32 nStart = rPos;
        if (rText[rPos] == '-' || rText[rPos] == '+')
            ++rPos;
        while (rPos < rText.getLength()
               && (rtl::isAsciiDigit(rText[rPos]) || rText[rPos] == '.'))
            ++rPos;
        return o3tl::toDouble(rText.subView(nStart, rPos - nStart));
    };

    while (nLeft < nLeftLen && nRight < nRightLen)
    {
        if (isNumberStart(rLeft, nLeft) && isNumberStart(rRight, nRight))
        {
            double fLeft = readNumber(rLeft, nLeft);
            double fRight = readNumber(rRight, nRight);
            if (!numbersEqual(fLeft, fRight, fFloor))
                return false;
        }
        else if (rLeft[nLeft] == rRight[nRight])
        {
            ++nLeft;
            ++nRight;
        }
        else
            return false;
    }
    return nLeft == nLeftLen && nRight == nRightLen;
}

/** Says whether a difference at this key tells anything about the filter.

    Three kinds do not. A shape states the character and paragraph properties of the text it holds
    as a single value, which means nothing when the text carries several, and the text itself is
    compared run by run anyway. A run of no characters is formatted in a way nobody can see. And
    the text of a placeholder nobody has typed into is the prompt the application supplies, which
    is never written to a file, so the two sides hold different prompts in different sizes. */
bool tellsSomething(const std::map<OString, OUString>& rBefore,
                    const std::map<OString, OUString>& rAfter, std::string_view rKey)
{
    const size_t nAt = rKey.rfind('@');
    if (nAt == std::string_view::npos)
        return true;
    const OString aPath(rKey.substr(0, nAt));
    const OString aProperty(rKey.substr(nAt + 1));

    auto isEmptyOnEitherSide = [&rBefore, &rAfter](const OString& rValueKey, const char* pEmpty) {
        for (const auto& rSide : { std::cref(rBefore), std::cref(rAfter) })
        {
            const auto it = rSide.get().find(rValueKey);
            if (it != rSide.get().end() && it->second == OUString::createFromAscii(pEmpty))
                return true;
        }
        return false;
    };

    if (aPath.indexOf("/para[") < 0
        && (aProperty.startsWith("Char") || aProperty.startsWith("Para")))
        return false;

    if (aPath.indexOf("/run[") >= 0 && aProperty != "Text"
        && isEmptyOnEitherSide(aPath + "@Text", ""))
        return false;

    const sal_Int32 nParagraph = aPath.indexOf("/para[");
    if (nParagraph >= 0
        && isEmptyOnEitherSide(OString::Concat(aPath.subView(0, nParagraph))
                                   + "@IsEmptyPresentationObject",
                               "true"))
        return false;

    // Once the two sides hold a different number of paragraphs or runs, each index below that
    // point names a different piece of text and every property of it reads as changed.
    auto differsBetweenSides = [&rBefore, &rAfter](const OString& rCountKey) {
        const auto aFirst = rBefore.find(rCountKey);
        const auto aSecond = rAfter.find(rCountKey);
        if (aFirst == rBefore.end() || aSecond == rAfter.end())
            return false;
        return aFirst->second != aSecond->second;
    };
    if (nParagraph >= 0
        && differsBetweenSides(OString::Concat(aPath.subView(0, nParagraph))
                               + "@ParagraphCount"))
        return false;
    const sal_Int32 nRun = aPath.indexOf("/run[");
    if (nRun >= 0
        && differsBetweenSides(OString::Concat(aPath.subView(0, nRun)) + "@RunCount"))
        return false;

    return true;
}

std::vector<Diff> compare(const std::map<OString, OUString>& rBefore,
                          const std::map<OString, OUString>& rAfter)
{
    std::vector<Diff> aDiffs;
    for (const auto& rEntry : rBefore)
    {
        if (!tellsSomething(rBefore, rAfter, rEntry.first))
            continue;
        auto it = rAfter.find(rEntry.first);
        if (it == rAfter.end())
            aDiffs.push_back({ DiffKind::Lost, rEntry.first, rEntry.second, OUString() });
        else if (!valuesEqual(rEntry.second, it->second, slackOf(rEntry.first)))
            aDiffs.push_back({ DiffKind::Changed, rEntry.first, rEntry.second, it->second });
    }
    for (const auto& rEntry : rAfter)
    {
        if (rBefore.find(rEntry.first) == rBefore.end()
            && tellsSomething(rBefore, rAfter, rEntry.first))
            aDiffs.push_back({ DiffKind::Added, rEntry.first, OUString(), rEntry.second });
    }
    return aDiffs;
}

const char* kindName(DiffKind eKind)
{
    switch (eKind)
    {
        case DiffKind::Lost:
            return "LOST";
        case DiffKind::Added:
            return "ADDED";
        case DiffKind::Changed:
            return "CHANGED";
    }
    return "?";
}

/** The report is tab separated, so values must not carry tabs or newlines. */
OString flatten(std::u16string_view rText)
{
    OUStringBuffer aBuffer(rText);
    for (sal_Int32 i = 0; i < aBuffer.getLength(); ++i)
    {
        if (aBuffer[i] == '\t' || aBuffer[i] == '\n' || aBuffer[i] == '\r')
            aBuffer[i] = ' ';
    }
    return OUStringToOString(aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
}

OString environment(const char* pName, const char* pDefault)
{
    const char* pValue = std::getenv(pName);
    return OString(pValue && *pValue ? pValue : pDefault);
}
}

class SdRoundtripDiffTest : public UnoApiTest
{
public:
    SdRoundtripDiffTest()
        : UnoApiTest(u"/sd/qa/unit/data/"_ustr)
    {
    }

    void testCorpus();

    CPPUNIT_TEST_SUITE(SdRoundtripDiffTest);
    CPPUNIT_TEST(testCorpus);
    CPPUNIT_TEST_SUITE_END();

private:
    std::vector<OUString> collectCorpus(const OUString& rDirectoryUrl);
    void writeDump(std::string_view rDirectory, std::string_view rName, std::string_view rStage,
                   const std::map<OString, OUString>& rValues);
};

std::vector<OUString> SdRoundtripDiffTest::collectCorpus(const OUString& rDirectoryUrl)
{
    std::vector<OUString> aFiles;
    osl::Directory aDirectory(rDirectoryUrl);
    if (aDirectory.open() != osl::FileBase::E_None)
        return aFiles;

    osl::DirectoryItem aItem;
    while (aDirectory.getNextItem(aItem) == osl::FileBase::E_None)
    {
        osl::FileStatus aStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName
                                | osl_FileStatus_Mask_FileURL);
        if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
            continue;
        if (aStatus.getFileType() != osl::FileStatus::Regular)
            continue;
        const OUString aFileName = aStatus.getFileName();
        if (!aFileName.endsWithIgnoreAsciiCase(u".pptx")
            && !aFileName.endsWithIgnoreAsciiCase(u".ppt"))
            continue;
        aFiles.push_back(aStatus.getFileURL());
    }
    std::sort(aFiles.begin(), aFiles.end());
    return aFiles;
}

void SdRoundtripDiffTest::writeDump(std::string_view rDirectory, std::string_view rName,
                                    std::string_view rStage,
                                    const std::map<OString, OUString>& rValues)
{
    if (rDirectory.empty())
        return;
    const OString aPath = OString::Concat(rDirectory) + "/" + rName + "." + rStage + ".dump";
    std::ofstream aStream(aPath.getStr());
    for (const auto& rEntry : rValues)
        aStream << rEntry.first << "\t" << flatten(rEntry.second) << "\n";
}

void SdRoundtripDiffTest::testCorpus()
{
    // A corpus run takes hours, so it never happens as part of make check.
    if (!std::getenv("SD_RT_RUN") && !std::getenv("SD_RT_CORPUS"))
        return;

    skipValidation();

    OUString aCorpusUrl;
    OString aCorpus = environment("SD_RT_CORPUS", "");
    if (aCorpus.isEmpty())
        aCorpusUrl = createFileURL(u"pptx/");
    else
        osl::FileBase::getFileURLFromSystemPath(
            OStringToOUString(aCorpus, RTL_TEXTENCODING_UTF8), aCorpusUrl);

    const OString aReportPath = environment("SD_RT_REPORT", "sd-roundtrip.log");
    const OString aDumpDirectory = environment("SD_RT_DUMPDIR", "");
    const sal_Int32 nLimit = environment("SD_RT_LIMIT", "0").toInt32();
    const bool bLayout = std::getenv("SD_RT_LAYOUT") != nullptr;

    // Resume: anything already reported on is not done again, so that the run
    // can be restarted after a document takes the process down with it.
    std::set<OString> aDone;
    {
        std::ifstream aStream(aReportPath.getStr());
        std::string aLine;
        while (std::getline(aStream, aLine))
        {
            if (aLine.compare(0, 5, "FILE\t") == 0)
            {
                std::string aRest = aLine.substr(5);
                aDone.insert(OString(aRest.substr(0, aRest.find('\t')).c_str()));
            }
        }
    }

    std::vector<OUString> aFiles = collectCorpus(aCorpusUrl);
    CPPUNIT_ASSERT_MESSAGE("no documents in the corpus", !aFiles.empty());

    std::ofstream aReport(aReportPath.getStr(), std::ios::app);
    sal_Int32 nProcessed = 0;

    for (const OUString& rUrl : aFiles)
    {
        const OString aName
            = OUStringToOString(rUrl.subView(rUrl.lastIndexOf('/') + 1), RTL_TEXTENCODING_UTF8);
        if (aDone.count(aName))
            continue;
        if (nLimit && nProcessed >= nLimit)
            break;
        ++nProcessed;

        // Named before the work starts: if the process dies on this document,
        // the report still says which one it was.
        aReport << "START\t" << aName << "\n";
        aReport.flush();

        ModelDumper aFirst, aSecond, aThird;
        try
        {
            // A document is written in the format it was read in, so that each round trip
            // measures one pair of filters rather than two.
            const TestFilter eFilter
                = rUrl.endsWithIgnoreAsciiCase(u".ppt") ? TestFilter::PPT : TestFilter::PPTX;

            loadFromURL(rUrl);
            aFirst.dump(mxComponent);
            if (bLayout)
                dumpLayout(aFirst.maValues, mxComponent);
            saveAndReload(eFilter);
            aSecond.dump(mxComponent);
            if (bLayout)
                dumpLayout(aSecond.maValues, mxComponent);
            saveAndReload(eFilter);
            aThird.dump(mxComponent);
            if (bLayout)
                dumpLayout(aThird.maValues, mxComponent);
        }
        catch (const std::exception& rException)
        {
            aReport << "FILE\t" << aName << "\tERROR\t" << rException.what() << "\n";
            aReport.flush();
            continue;
        }
        catch (...)
        {
            aReport << "FILE\t" << aName << "\tERROR\tunknown exception\n";
            aReport.flush();
            continue;
        }

        writeDump(aDumpDirectory, aName, "a", aFirst.maValues);
        writeDump(aDumpDirectory, aName, "b", aSecond.maValues);
        writeDump(aDumpDirectory, aName, "c", aThird.maValues);

        const std::vector<Diff> aLossy = compare(aFirst.maValues, aSecond.maValues);
        const std::vector<Diff> aUnstable = compare(aSecond.maValues, aThird.maValues);

        aReport << "FILE\t" << aName << "\tOK\t" << aFirst.maValues.size() << "\t"
                << aLossy.size() << "\t" << aUnstable.size() << "\n";
        for (const Diff& rDiff : aLossy)
            aReport << "D1\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        for (const Diff& rDiff : aUnstable)
            aReport << "D2\t" << aName << "\t" << kindName(rDiff.eKind) << "\t" << rDiff.aKey
                    << "\t" << flatten(rDiff.aBefore) << "\t" << flatten(rDiff.aAfter) << "\n";
        aReport.flush();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdRoundtripDiffTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
