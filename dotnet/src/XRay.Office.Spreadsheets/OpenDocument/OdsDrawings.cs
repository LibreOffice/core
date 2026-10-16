using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Vector;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// The drawings anchored on one ODF sheet: any <c>draw:</c> element inside a
/// <c>table:table-cell</c> or a <c>table:shapes</c>.
/// </summary>
/// <remarks>
/// <para>
/// ODF anchors a drawing by <em>containment</em> where SpreadsheetML anchors it by address: the
/// frame is a child of the cell it is fastened to, and <c>svg:x</c> and <c>svg:y</c> are measured
/// from that cell's top-left corner. So the walk that finds the drawings is the same walk that
/// counts the columns, and the repeat counts have to be honoured on the way or every frame after
/// a repeated column lands in the wrong place.
/// </para>
/// <para>
/// <strong>The end cell wins over the stated size, and that is measurable.</strong> A frame may
/// carry <c>table:end-cell-address</c> with <c>table:end-x</c> and <c>table:end-y</c> as well as
/// <c>svg:width</c> and <c>svg:height</c>; the first is ODF's "resize with the cells" anchor,
/// Calc's <c>ScDrawObjData</c>. When the two disagree Calc believes the end cell and rewrites the
/// lengths: a hand-written frame stating <c>svg:width="1.28in"</c> and ending at C3 is saved back
/// as <c>svg:width="1.3201in"</c>, which is the two columns it spans less its own start offset.
/// So a frame with an end address is read as a two-cell anchor and one without it as a one-cell
/// anchor, which is exactly the distinction the attribute exists to make.
/// </para>
/// <para>
/// <strong>Two ways to hold the bytes, and a flat file needs the second.</strong> A packaged ODS
/// writes <c>xlink:href="Pictures/…"</c>; a flat one writes the bytes inline as base64 in
/// <c>office:binary-data</c>, because there is no package to put them in. Both are read, and
/// neither is decoded — <see cref="RasterImage.Encoded"/> carries the file's own bytes to whichever
/// backend wants pixels.
/// </para>
/// </remarks>
internal static class OdsDrawings
{
    /// <summary>How many repeats of one row or column element are walked into.</summary>
    /// <remarks>The same cap the format readers use, and for the same reason.</remarks>
    private const int MaxRepeat = 4096;

    /// <summary>Reads the drawings anchored on one sheet.</summary>
    /// <param name="file">The document, for its package.</param>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static SheetDrawings Read(OdfFile file, XElement table)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(table);

        List<SheetDrawing> drawings = [];
        int row = 0;

        ReadSheetShapes(file, table, drawings);

        foreach (XElement rowElement in Rows(table))
        {
            int repeat = Repeat(rowElement, "number-rows-repeated");
            int span = Math.Min(repeat, MaxRepeat);

            for (int at = 0; at < span && row < SheetAddress.MaxRow; at++, row++)
            {
                // Only the first copy of a repeated row carries its drawings, which is what the
                // extraction path does with the same attribute: a row repeated a million times is
                // the sheet's padding rather than a million pictures.
                if (at == 0) ReadCells(file, rowElement, row, drawings);
            }

            if (repeat > span) row += repeat - span;
        }

        return drawings.Count == 0 ? SheetDrawings.Empty : new SheetDrawings(drawings);
    }

    /// <summary>
    /// The drawings fastened to the sheet rather than to a cell: <c>table:shapes</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF has two containers for a sheet's drawings and they mean different anchors.
    /// A <c>draw:frame</c> inside a <c>table:table-cell</c> is anchored to that cell and its
    /// <c>svg:x</c>/<c>svg:y</c> are measured from the cell's corner; a <c>draw:frame</c> inside
    /// <c>table:shapes</c> — the table's own first child, before the columns — is anchored to the
    /// <em>sheet</em>, and its <c>svg:x</c>/<c>svg:y</c> are measured from the table's origin.
    /// LibreOffice keeps the same distinction on the way back out: <c>ScXMLExport</c> writes a
    /// page-anchored object into <c>table:shapes</c> and a cell-anchored one into the cell
    /// (<c>sc/source/filter/xml/xmlexprt.cxx</c>, <c>WriteShapes</c>), and its importer routes the
    /// container through <c>ScXMLTableShapeContext</c> with no cell address at all.
    /// </para>
    /// <para>
    /// Walking only the cells is what made this invisible: every ODS LibreOffice writes from a
    /// chart pasted onto a cell puts the frame in the cell, so the whole corpus took the first
    /// route. <c>chart2/qa/extras/data/ods/tdf166428_Low_High_StockChart_LO248.ods</c> takes the
    /// second, and its stock chart was read into a correct <c>ChartPlot</c> that then had nowhere
    /// to go — 24 words rendered against LibreOffice's 60, all of the difference being the chart.
    /// </para>
    /// </remarks>
    private static void ReadSheetShapes(OdfFile file, XElement table, List<SheetDrawing> drawings)
    {
        XElement? shapes = table.Element(XName.Get("shapes", OdfNamespaces.Table));
        if (shapes is null) return;

        foreach (XElement shape in Shapes(shapes))
        {
            if (Read(file, shape, 0, 0, sheetAnchored: true) is { } drawing) drawings.Add(drawing);
        }
    }

    private static void ReadCells(
        OdfFile file, XElement rowElement, int row, List<SheetDrawing> drawings)
    {
        int column = 0;

        foreach (XElement cell in rowElement.Elements())
        {
            if (cell.Name.NamespaceName != OdfNamespaces.Table) continue;
            if (cell.Name.LocalName is not ("table-cell" or "covered-table-cell")) continue;

            int repeat = Repeat(cell, "number-columns-repeated");

            foreach (XElement shape in Shapes(cell))
            {
                if (Read(file, shape, row, column) is { } drawing) drawings.Add(drawing);
            }

            column += Math.Min(repeat, MaxRepeat);
            if (repeat > MaxRepeat) column += repeat - MaxRepeat;
            if (column >= SheetAddress.MaxColumn) break;
        }
    }

    /// <summary>
    /// The drawing elements a cell or a <c>table:shapes</c> holds, however they are wrapped.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Two wrappers stand between a cell and its pictures, and reading only the cell's own
    /// children misses both.</strong> A <c>draw:g</c> is a group — a sheet's watermark is usually
    /// one — and a <c>draw:a</c> is a hyperlink around a shape, which ODF states as a wrapper
    /// element rather than as an attribute (ODF 1.3 §10.4.14). Both are transparent to placement:
    /// neither carries a rectangle of its own, and a child's <c>svg:x</c> or <c>draw:transform</c>
    /// is measured from the same origin whether or not it is inside one.
    /// </para>
    /// <para>
    /// Censused over the 307 converted <c>.ods</c>: <strong>89 frames sit inside a
    /// <c>draw:g</c></strong> — 72 of them in <c>SIL_TDB648.ods</c>, whose 74 frames are all but
    /// two of the pictures that make it 88 pages against our 60 — and <strong>33 inside a
    /// <c>draw:a</c></strong>, across thirteen documents. So this is where that document's missing
    /// pictures went, and the <c>draw:transform</c> below is the second gate rather than the first:
    /// a grouped frame stating a plain <c>svg:x</c> was missed too.
    /// </para>
    /// <para>
    /// The group is <em>not</em> read as a drawing of its own. ODF gives a group no rectangle: its
    /// children carry absolute coordinates in the anchor's own space, so each child is its own
    /// one-cell-anchored drawing and the union of them is what
    /// <see cref="Layout.SheetDrawingArea"/> and <see cref="Layout.SheetEmptyPages"/> already take.
    /// The <c>table:end-cell-address</c> LibreOffice writes on the group is its cached bounding
    /// anchor for that same union.
    /// </para>
    /// <para>
    /// <strong>Every drawing element is yielded, not only <c>draw:frame</c>.</strong> A frame is
    /// ODF's wrapper for an <em>embedded</em> thing — a picture, a chart, an OLE object — and a
    /// shape drawn on the sheet is not embedded in anything: a text box is a
    /// <c>draw:custom-shape</c>, and rectangles, ellipses, lines, connectors and form controls are
    /// each their own element. Calc reads all of them through one shape context
    /// (<c>ScXMLTableRowCellContext</c> hands any <c>draw:</c> child to
    /// <c>XMLShapeImportHelper</c>) and prints them all through <c>PrintDrawingLayer</c>.
    /// Censused over the 307 converted <c>.ods</c>: <strong>604 <c>draw:custom-shape</c> in 54
    /// documents</strong> sit in cells, of which 137 in 32 documents carry text, against 495
    /// frames of which none does. <see cref="Read(OdfFile, XElement, int, int, bool)"/> decides
    /// what each of them is worth drawing.
    /// </para>
    /// </remarks>
    private static IEnumerable<XElement> Shapes(XElement container)
    {
        foreach (XElement child in container.Elements())
        {
            if (child.Name.NamespaceName != OdfNamespaces.Draw) continue;

            if (child.Name.LocalName is "g" or "a")
            {
                foreach (XElement nested in Shapes(child)) yield return nested;
            }
            else
            {
                yield return child;
            }
        }
    }

    /// <summary>
    /// Reads one drawing element into the drawing it puts on the sheet, or null when it puts
    /// nothing there.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Four kinds of element reach this: a frame holding a picture, a frame holding an embedded
    /// object, a shape carrying text, and — since round 84 — a shape carrying nothing but a fill
    /// or an outline. That last one is the largest of the four: censused over the 307 converted
    /// <c>.ods</c>, <b>444 custom shapes in 26 documents carry ink and no text</b>, against 198
    /// that carry both.
    /// </para>
    /// <para>
    /// <strong>Answering null for one of those cost a page count as well as a picture</strong>,
    /// because a drawing reaches the printed block through a different route from every cell
    /// question: <c>ScDrawLayer::GetPrintArea</c> takes the bounding rectangle of <em>every</em>
    /// object on the sheet's draw page — its only exclusion is the hidden-comment layer, and the
    /// line above the layer test reads <c>//TODO: test Flags (hidden?)</c>
    /// (<c>sc/source/core/data/drwlayer.cxx</c>:1397-1414) — and
    /// <c>ScDocument::GetPrintArea</c> maxes that into the cells' answer
    /// (<c>documen2.cxx</c>:644-664). So a shape the reader drops narrows the print area of every
    /// sheet carrying one, which is what <see cref="Layout.SheetDrawingArea"/> already says and
    /// this reader was quietly defeating.
    /// </para>
    /// <para>
    /// A shape carrying <em>none</em> of the four is still answered null, and that remains a
    /// deliberate under-count of the same rule: a <c>draw:control</c>, and any shape whose style
    /// states <c>draw:fill="none"</c> and <c>draw:stroke="none"</c>, is an object on Calc's draw
    /// page and widens its block. Counting them is a change of a different size — 78 controls and
    /// 630 unfilled shapes in the column — and it is measured rather than assumed in
    /// <c>probes/sheet-fill-r84</c>.
    /// </para>
    /// </remarks>
    /// <param name="file">The document, for its package and its styles.</param>
    /// <param name="frame">The <c>draw:</c> element.</param>
    /// <param name="row">The zero-based row of the anchoring cell.</param>
    /// <param name="column">Its zero-based column.</param>
    /// <param name="sheetAnchored">True for a <c>table:shapes</c> child, which names no cell.</param>
    private static SheetDrawing? Read(
        OdfFile file, XElement frame, int row, int column, bool sheetAnchored = false)
    {
        XElement? image = frame.Element(XName.Get("image", OdfNamespaces.Draw));
        XElement? objectFrame = frame.Element(XName.Get("object", OdfNamespaces.Draw));

        Length width =
            OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "width")) ?? Length.Zero;
        Length height =
            OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "height")) ?? Length.Zero;

        Length x = OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "x")) ?? Length.Zero;
        Length y = OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "y")) ?? Length.Zero;

        // A turned frame states no coordinate pair at all; its rectangle is the transform's.
        Turned? turned = Placement(frame, width, height);
        if (turned is { } placed)
        {
            x = placed.X;
            y = placed.Y;
            width = placed.Width;
            height = placed.Height;
        }

        // And three kinds state no rectangle at all: their geometry is two points.
        bool segment = false;
        if (turned is null && Endpoints(frame) is { } pair)
        {
            segment = true;
            x = pair.X;
            y = pair.Y;
            width = pair.Width;
            height = pair.Height;
        }

        // Read before the branches below because a shape's ink is the same question whatever
        // else it holds, and because the fill of a frame that turns out to hold a picture is
        // simply unused rather than wrong.
        // **A segment's ink is deliberately not taken**, because the only outline this can paint
        // is the box's. <see cref="Layout.SheetShapeInk"/> falls back to the rectangle for a shape
        // that names no preset, and a rectangle is not a line: the corpus's 30 elbow connectors
        // state their real path as an `svg:d` — `M1238 6853v1508h808v1507` on
        // `016_Free_Organizational_Chart`'s `Elbow Connector 46` — which is a shape this reader
        // has no parser for. Painting them is left; what is not left is the *extent*, which is
        // what a print area is made of. The two straight-segment kinds the corpus does hold are
        // axis-aligned, so their boxes are degenerate and `SheetShapeInk.Draw` returns on them
        // anyway.
        OdsShapeInk.Ink ink = segment ? default : OdsShapeInk.Read(file.Styles, frame);

        SheetDrawing drawing = new()
        {
            Anchor = sheetAnchored ? SheetAnchorKind.Absolute : SheetAnchorKind.OneCell,
            From = new SheetCellPoint(column, x, row, y),
            Position = new DocPoint(x, y),
            Extent = new DocSize(width, height),
            Name = Attribute(frame, OdfNamespaces.Draw, "name"),
            Description = Description(frame),
        };

        // A sheet-anchored frame states no end cell — it is not fastened to the grid at all — so
        // the two-cell reading below is not even attempted for one.
        //
        // **Nor is it attempted for a shape whose rectangle came from a `draw:transform`, because
        // the two are the same rectangle stated twice and combining them is a union of both.**
        // A turned shape states no `svg:x`, so its top-left corner is the transform's — which may
        // be well to the *left* of the anchor cell — while `table:end-cell-address` is Calc's
        // cached far corner for the same object. Taking one from each gives a box neither of them
        // describes. Measured on `Foreign_SA-CAT-I_and_CAT-II-III_Pub_0.ods`, whose `TextBox 1` is
        // turned a half turn and states an end cell of AS3: the columns to AS are 47.73 inches
        // where the shape's own `svg:width` is 23.66, and the union widened the printed block far
        // enough to cost **four pages against 26.2.4.2's sixteen**. 26.2.4.2 prints sixteen with
        // the end cell present and sixteen with it deleted, so the end cell decides nothing there.
        //
        // The end cell does decide the box of a shape that states `svg:x`, and that is measured
        // rather than assumed — `probes/ods-draw-r82/endcell.py` puts a right-aligned text box
        // 1 inch wide at A2 with an end cell of E2 and 26.2.4.2 draws its line at x 289.644
        // against 73.644 without the end cell, which is the four inches the end cell states.
        // **Reach: 2 shapes in 2 of the 307 converted `.ods` state both.**
        //
        // **A segment is excluded for the same reason, and it is not a fine judgement**: Calc
        // writes a connector's cached end anchor as the cell the object is *anchored in*, not as
        // the far corner of the line. `017_Timeline_Templates`' `Straight Connector 2` sits in A1
        // and runs to `svg:y2="15.7957in"`, and its `table:end-cell-address` is `A1` with an
        // `end-y` of `0.0394in` — so taking the end cell collapses a fifteen-inch line to a point
        // and the print area never grows.
        if (!sheetAnchored
            && turned is null
            && !segment
            && EndCell(Attribute(frame, OdfNamespaces.Table, "end-cell-address")) is { } end)
        {
            drawing = drawing with
            {
                Anchor = SheetAnchorKind.TwoCell,
                To = new SheetCellPoint(
                    end.Column,
                    OdfValue.ParseLength(Attribute(frame, OdfNamespaces.Table, "end-x")) ?? Length.Zero,
                    end.Row,
                    OdfValue.ParseLength(Attribute(frame, OdfNamespaces.Table, "end-y")) ?? Length.Zero),
            };
        }

        // An embedded object is a chart, a formula or another document. The flag is set for all of
        // them, so that "there is something here" stays distinguishable from "there is nothing
        // here"; only a chart of a drawable kind also carries a model.
        //
        // **The object is looked at before the picture, and the order is what makes a sheet's
        // chart draw.** A frame holding an object carries a *replacement* picture beside it —
        // `draw:image xlink:href="./ObjectReplacements/Object 1"` — which is what an application
        // that cannot open the object shows instead. Reading the picture first found one on every
        // chart in every ODS LibreOffice has ever written, recorded the frame as a plain picture,
        // and then painted nothing, because all 82 of those streams are `VCLMTF` and no decoder
        // here reads StarView metafiles. A deck never hit it: an ODP frame carries the object
        // alone.
        if (objectFrame is not null)
        {
            SheetDrawing chart = drawing with { IsChart = true, Chart = Plot(file, objectFrame) };

            // A chart of a kind the engine does not draw falls back to the replacement picture,
            // which is better than nothing wherever a backend can decode one.
            if (chart.Chart is not null || image is null) return chart;

            (RasterImage? fallback, Lazy<VectorImage>? drawn) = Load(file, image);
            return chart with { Image = fallback, Vector = drawn };
        }

        // A shape that embeds nothing may still carry text, and a text box is exactly that: an
        // `mso-spt202` or `ooxml-rect` custom shape whose `text:p` children are its body. That
        // text is the shape's, not the anchoring cell's — see OdsShapeText.
        //
        // And it may carry ink instead of, or as well as, that text: 444 of the column's custom
        // shapes are inked and textless.
        //
        // **A shape with neither is still a drawing**, and answering null for one was costing a
        // page. Every object on Calc's draw page widens the printed block —
        // `ScDrawLayer::GetPrintArea` skips only `SC_LAYER_HIDDEN`
        // (`sc/source/core/data/drwlayer.cxx`:1397-1414) — so a rectangle stating
        // `draw:fill="none" draw:stroke="none"` past the last cell keeps a page alive with
        // nothing on it. Measured on `features/sheet-shape-ink.ods`, whose fifth shape is exactly
        // that: 26.2.4.2 prints **two** pages, the second of them empty, and this reader printed
        // one until it stopped dropping the shape. The SpreadsheetML reader has always kept
        // one — an `xdr:sp` produces a drawing whatever it holds — which is why the same fixture
        // as `.xlsx` was already two pages of two.
        if (image is null)
        {
            SheetShapeText? shapeText = OdsShapeText.Read(file.Styles, frame);

            // **A turned shape's ink goes on a part, for the reason a turned picture's does.**
            // `Placement` answers the *bounding* box of the transform, which is what the print
            // area and the empty-page test want and what a rotated star does not fill: painting
            // the geometry into it draws a star as much as 20% too large and squared up.
            // `SheetDrawingPart` is the only thing that carries an angle, so the shape's own
            // untuned rectangle is centred in the box and turned there — the same arithmetic the
            // picture branch below does. **Reach: 255 turned `draw:custom-shape` in 8 of the 307
            // converted `.ods`**, 242 of them the reward-chart template's own stamps.
            if (turned is { Degrees: not 0 } inked && ink.HasInk)
            {
                return drawing with
                {
                    Text = shapeText,
                    Parts =
                    [
                        new SheetDrawingPart(
                            Fraction(inked.Width - inked.Shape.Width, 2 * inked.Width),
                            Fraction(inked.Height - inked.Shape.Height, 2 * inked.Height),
                            Fraction(inked.Shape.Width, inked.Width),
                            Fraction(inked.Shape.Height, inked.Height),
                            inked.Degrees)
                        {
                            Fill = ink.Fill,
                            Stroke = ink.Stroke,
                            StrokeWidth = ink.StrokeWidth,
                            Preset = ink.Preset,
                        },
                    ],
                };
            }

            return drawing with
            {
                Text = shapeText,
                Fill = ink.Fill,
                Stroke = ink.Stroke,
                StrokeWidth = ink.StrokeWidth,
                Preset = ink.Preset,
            };
        }

        (RasterImage? raster, Lazy<VectorImage>? vector) = Load(file, image);

        // A turned picture is carried as a part rather than on the drawing, because a part is the
        // one thing that has an angle: the drawing's own rectangle is the *bounding* box, which is
        // what the print area and the empty-page test want, and the part inside it is the picture's
        // own box, which is what gets painted and turned. See <see cref="SheetDrawingPart"/>.
        if (turned is { Degrees: not 0 } box && (raster is not null || vector is not null))
        {
            return drawing with
            {
                Parts =
                [
                    new SheetDrawingPart(
                        Fraction(box.Width - box.Shape.Width, 2 * box.Width),
                        Fraction(box.Height - box.Shape.Height, 2 * box.Height),
                        Fraction(box.Shape.Width, box.Width),
                        Fraction(box.Shape.Height, box.Height),
                        box.Degrees)
                    {
                        Image = raster,
                        Vector = vector,
                    },
                ],
            };
        }

        return drawing with { Image = raster, Vector = vector };
    }

    /// <summary>The rectangle a <c>draw:transform</c> puts a frame in, and how far it is turned.</summary>
    /// <param name="X">The bounding box's left edge, in the anchor's own space.</param>
    /// <param name="Y">Its top edge.</param>
    /// <param name="Width">The bounding box's width.</param>
    /// <param name="Height">Its height.</param>
    /// <param name="Degrees">How far the frame is turned clockwise, in degrees.</param>
    /// <param name="Shape">The frame's own untuned size, which the box encloses.</param>
    private readonly record struct Turned(
        Length X, Length Y, Length Width, Length Height, double Degrees, DocSize Shape);

    /// <summary>
    /// Where a <c>draw:transform</c> puts a frame that states no <c>svg:x</c> or <c>svg:y</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice writes a turned or sheared object with a transform instead of a coordinate pair
    /// — <c>skewX (0.0222) rotate (0.4824) translate (0.4622in 2.9827in)</c> is
    /// <c>SIL_TDB648.ods</c>'s watermark — and leaves <c>svg:width</c> and <c>svg:height</c> on the
    /// element as the shape's own, untuned size. So a reader that takes the two lengths and no
    /// transform places every such frame on the anchor cell's corner, and one that takes neither
    /// drops it entirely.
    /// </para>
    /// <para>
    /// <strong>What is returned is the bounding box, because that is the rectangle Calc's two
    /// page-deciding questions ask for.</strong> <c>ScDrawLayer::GetPrintArea</c> widens the
    /// printed block to cover every object and <c>ScDocument::HasAnyDraw</c> keeps a page an object
    /// overlaps; both go through <c>GetCurrentBoundRect</c>, which is the turned box rather than
    /// the shape's own. The picture inside it keeps its own size and angle as a
    /// <see cref="SheetDrawingPart"/> — the same shape the SpreadsheetML reader already produces
    /// for a grouped, turned watermark, and the reason <see cref="SheetDrawingBounds"/> exists.
    /// </para>
    /// <para>
    /// Checked against 26.2.4.2's own PDF of <c>SIL_TDB648.ods</c>: its page 4 draws three copies
    /// of that watermark 439.2 × 283.3 pt, against the 440.7 × 282.5 this computes from a
    /// 455.5 × 80.3 pt frame turned 27.64°, and the vertical distance between two of them is
    /// 325.4 pt against the 325.4 pt the two <c>translate</c> pairs state.
    /// </para>
    /// </remarks>
    private static Turned? Placement(XElement frame, Length width, Length height)
    {
        if (Attribute(frame, OdfNamespaces.SvgCompatible, "x") is not null) return null;
        if (OdfTransform.Read(frame) is not { } transform) return null;

        (double x, double y) = (transform.E, transform.F);
        double right = (width.Emu * transform.A) + x;
        double bottom = (width.Emu * transform.B) + y;
        double downX = (height.Emu * transform.C) + x;
        double downY = (height.Emu * transform.D) + y;
        double farX = (width.Emu * transform.A) + (height.Emu * transform.C) + x;
        double farY = (width.Emu * transform.B) + (height.Emu * transform.D) + y;

        double minX = Math.Min(Math.Min(x, right), Math.Min(downX, farX));
        double maxX = Math.Max(Math.Max(x, right), Math.Max(downX, farX));
        double minY = Math.Min(Math.Min(y, bottom), Math.Min(downY, farY));
        double maxY = Math.Max(Math.Max(y, bottom), Math.Max(downY, farY));

        // The angle the matrix turns through, taken from the image of the x axis. A shear leaves
        // that vector alone, so this is the rotation and not the whole transform — which is what a
        // part wants, its box being axis-aligned before it is turned.
        double degrees = Math.Atan2(transform.B, transform.A) * 180.0 / Math.PI;

        return new Turned(
            Length.FromEmu((long)Math.Round(minX)),
            Length.FromEmu((long)Math.Round(minY)),
            Length.FromEmu((long)Math.Round(maxX - minX)),
            Length.FromEmu((long)Math.Round(maxY - minY)),
            degrees,
            new DocSize(width, height));
    }

    /// <summary>One length as a fraction of another, or zero when the second is degenerate.</summary>
    private static double Fraction(Length part, Length whole)
        => whole.Emu == 0 ? 0 : part.Emu / (double)whole.Emu;


    /// <summary>
    /// The chart a <c>draw:object</c> holds, or null when it holds something else.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One reader for both shapes an embedded chart takes — a packaged sheet's
    /// <c>Object 1/content.xml</c> reached by <c>xlink:href</c>, and a flat sheet's inlined
    /// <c>office:document</c> — because <see cref="OdfChart.Locate"/> already hides the
    /// difference. This is the same call the ODP layout makes, which is what the move of
    /// <see cref="OdfChartPlot"/> down into <c>XRay.Office.OpenDocument</c> bought: before it, a
    /// sheet would have needed a second copy of the reader.
    /// </para>
    /// <para>
    /// The styles are the chart sub-document's own — <c>ch1</c>, <c>ch2</c>, … in its own
    /// <c>office:automatic-styles</c> — and not the workbook's, so they are read from whichever
    /// root the chart was found under.
    /// </para>
    /// </remarks>
    private static ChartPlot? Plot(OdfFile file, XElement objectFrame)
    {
        if (OdfChart.Locate(objectFrame, file) is not { } chart) return null;

        return OdfChartPlot.Read(chart, new OdfChartStyles(chart.AncestorsAndSelf().Last()));
    }

    /// <summary>
    /// A <c>draw:image</c>'s picture, told apart into a raster and a metafile.
    /// </summary>
    /// <remarks>
    /// <c>draw:mime-type</c> is passed on as a hint and the bytes decide, because ODF's own exporters
    /// disagree with themselves: LibreOffice writes <c>image/x-emf</c> for a file it stored under a
    /// <c>.emf</c> name and <c>image/x-wmf</c> for one it stored under <c>.wmf</c>, and neither name
    /// nor type tells an EMF+ from the EMF that carries it. A vector is left undecoded until something
    /// draws it; see <c>SheetDrawing.Vector</c> for what that costs otherwise.
    /// </remarks>
    private static (RasterImage? Raster, Lazy<VectorImage>? Vector) Load(OdfFile file, XElement image)
    {
        string? mediaType = Attribute(image, OdfNamespaces.Draw, "mime-type");

        if (Attribute(image, OdfNamespaces.XLink, "href") is { Length: > 0 } href)
        {
            // A path with a scheme points outside the package; XRay.Office does not fetch those.
            if (href.Contains("://", StringComparison.Ordinal)) return default;

            string part = href.StartsWith("./", StringComparison.Ordinal) ? href[2..] : href;
            using Stream? content = file.OpenPart(part);
            if (content is null) return default;

            using MemoryStream buffer = new();
            content.CopyTo(buffer);
            return buffer.Length == 0 ? default : Drawable(buffer.ToArray(), mediaType);
        }

        XElement? data = image.Element(XName.Get("binary-data", OdfNamespaces.Office));
        if (data is null) return default;

        try
        {
            byte[] bytes = Convert.FromBase64String(data.Value);
            return bytes.Length == 0 ? default : Drawable(bytes, mediaType);
        }
        catch (FormatException)
        {
            // Base64 a writer mangled is a picture that cannot be drawn, not a document that
            // cannot be read.
            return default;
        }
    }

    /// <summary>Which of the two kinds some picture bytes are.</summary>
    private static (RasterImage? Raster, Lazy<VectorImage>? Vector) Drawable(
        ReadOnlyMemory<byte> bytes, string? mediaType)
        => VectorImages.For(bytes.Span) is not null
            ? (null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)))
            : (RasterImage.Encoded(bytes, mediaType), null);

    /// <summary>
    /// A <c>table:end-cell-address</c>: a sheet name, a dot, and an A1 reference.
    /// </summary>
    /// <remarks>
    /// The sheet name is dropped rather than checked. A frame that names another sheet is not
    /// something any writer produces, and honouring it would mean placing a picture on a sheet it
    /// is not stored in; taking the address as this sheet's is the lenient reading and the one
    /// that keeps the picture. The name may itself contain dots when it is quoted, so the split is
    /// on the <em>last</em> one.
    /// </remarks>
    private static (int Column, int Row)? EndCell(string? address)
    {
        if (string.IsNullOrWhiteSpace(address)) return null;

        string reference = address[(address.LastIndexOf('.') + 1)..].Replace("$", string.Empty,
            StringComparison.Ordinal);

        int at = 0;
        int column = 0;
        while (at < reference.Length && char.IsAsciiLetter(reference[at]))
        {
            column = (column * 26) + (char.ToUpperInvariant(reference[at]) - 'A' + 1);
            at++;
        }

        if (at == 0 || at >= reference.Length) return null;
        if (!int.TryParse(reference[at..], NumberStyles.Integer, CultureInfo.InvariantCulture,
                          out int row) || row <= 0)
        {
            return null;
        }

        return (column - 1, row - 1);
    }

    /// <summary>
    /// The rectangle of a shape whose geometry is a pair of points rather than a box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>draw:line</c>, <c>draw:connector</c> and <c>draw:measure</c> state no
    /// <c>svg:width</c>, no <c>svg:height</c> and no <c>svg:x</c>: they state
    /// <c>svg:x1</c>, <c>svg:y1</c>, <c>svg:x2</c> and <c>svg:y2</c>, and their rectangle is the
    /// box those two points bound. <c>SdXMLLineShapeContext::startFastElement</c>
    /// (<c>xmloff/source/draw/ximpshap.cxx</c>:1045-1097) says so in as many words — it takes the
    /// smaller of each coordinate as the shape's position, puts the pair in the polygon and leaves
    /// the size at 1 × 1, so the object's own bounding rectangle is the polygon's — and
    /// <c>SdXMLConnectorShapeContext</c> (<c>:1963-2035</c>) sets <c>StartPosition</c> and
    /// <c>EndPosition</c> from the same four attributes.
    /// </para>
    /// <para>
    /// <strong>A reader that looks only for <c>svg:width</c> gives every one of them a zero-sized
    /// box, and a zero-sized box widens no print area.</strong> That is one page of
    /// <c>017_Timeline_Templates_for_Excel_b88faee6</c>: its <c>Straight Connector 2</c> is the
    /// timeline's spine, anchored in A1 and running from <c>svg:y1="0.6965in"</c> to
    /// <c>svg:y2="15.7957in"</c>, well below the last cell of a sheet whose data ends at row 66.
    /// 26.2.4.2 prints that sheet on two pages with the second carrying only the line's tail, and
    /// deleting the one <c>draw:connector</c> from the file makes the reference itself print one —
    /// three pages against two for the workbook, which is exactly the gap this closes. The
    /// <c>.xlsx</c> twin never had it, because a <c>twoCellAnchor</c> states the same connector's
    /// end as row 78 and the SpreadsheetML reader has always taken it.
    /// <c>probes/ods-residue-r95/variants-ods.py</c>.
    /// </para>
    /// <para>
    /// Reach over the 307 converted <c>.ods</c>: <strong>3 documents, 41 <c>draw:line</c>,
    /// 31 <c>draw:connector</c>, no <c>draw:measure</c></strong>, of which 28 are diagonal and all
    /// 72 state a stroke. <c>probes/ods-residue-r95/lineshape-census.py</c>.
    /// </para>
    /// </remarks>
    /// <param name="frame">The <c>draw:</c> element.</param>
    private static (Length X, Length Y, Length Width, Length Height)? Endpoints(XElement frame)
    {
        if (frame.Name.LocalName is not ("line" or "connector" or "measure")) return null;

        Length? x1 = OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "x1"));
        Length? y1 = OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "y1"));
        Length? x2 = OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "x2"));
        Length? y2 = OdfValue.ParseLength(Attribute(frame, OdfNamespaces.SvgCompatible, "y2"));

        if (x1 is not { } left || y1 is not { } top
            || x2 is not { } right || y2 is not { } bottom)
        {
            return null;
        }

        return (Length.FromEmu(Math.Min(left.Emu, right.Emu)),
                Length.FromEmu(Math.Min(top.Emu, bottom.Emu)),
                Length.FromEmu(Math.Abs(right.Emu - left.Emu)),
                Length.FromEmu(Math.Abs(bottom.Emu - top.Emu)));
    }

    /// <summary>The frame's description, which ODF writes as a child element rather than an attribute.</summary>
    private static string? Description(XElement frame)
        => frame.Element(XName.Get("desc", OdfNamespaces.SvgCompatible))?.Value
           ?? frame.Element(XName.Get("title", OdfNamespaces.SvgCompatible))?.Value;

    private static IEnumerable<XElement> Rows(XElement table)
    {
        foreach (XElement child in table.Elements())
        {
            if (child.Name.NamespaceName != OdfNamespaces.Table) continue;

            if (child.Name.LocalName == "table-row")
            {
                yield return child;
            }
            else if (child.Name.LocalName is "table-header-rows" or "table-row-group")
            {
                foreach (XElement nested in Rows(child)) yield return nested;
            }
        }
    }

    private static string? Attribute(XElement element, string ns, string name)
        => element.Attribute(XName.Get(name, ns))?.Value;

    private static int Repeat(XElement element, string name)
        => element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value is { } value
           && int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int count)
           && count > 0
            ? count
            : 1;
}
