using XRay.Office.Core.Threading;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Itemisation;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Shaping;
using XRay.Office.Vector;
using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// Turns laid-out pages into drawing commands.
/// </summary>
/// <remarks>
/// <para>
/// One line becomes one glyph run, positioned at its baseline. Positioned rather than measured again:
/// layout already committed to these advances when it decided where the lines broke, so re-deriving the
/// positions here would risk output that disagrees with the breaks around it — which is exactly what
/// <see cref="GlyphRun"/>'s own contract says a backend must not do.
/// </para>
/// <para>
/// The glyphs come from shaping the line's own characters, and shaping the line rather than the paragraph
/// is a deliberate small inaccuracy: a kern pair straddling a line break does not apply, which is right,
/// but a contextual form that depended on a following character now sees the line's end instead. For the
/// Latin text this can currently measure, the two are the same.
/// </para>
/// </remarks>
public static class PageDrawing
{
    /// <summary>
    /// Draws a page into a sink: its header, its body and its footer.
    /// </summary>
    /// <remarks>
    /// The header first and the footer last, which is reading order and also the order a backend would
    /// prefer — nothing here overlaps, so the order is a convention rather than a correctness matter, but a
    /// recorded display list reads far better when it matches the page. The footnotes come after the body
    /// they belong to and before the footer, which is where they sit on the sheet, with their separator rule
    /// immediately before them.
    /// <para>
    /// The floating frames come after the body, which is paint order rather than reading order and is the
    /// one place the two differ: a frame with a background is opaque, and the text it displaced has
    /// already been shortened to keep clear of it — so a frame drawn first would be painted over by
    /// whatever ran under it.
    /// </para>
    /// <para>
    /// <strong>Except the ones the document puts behind its text</strong>, which go first —
    /// <see cref="PageFrame.BehindText"/>. Writer keeps these on the <em>hell</em> layer and paints that
    /// layer before the text; a letterhead or a watermark is the whole of the case, and it is not a
    /// nuance. Measured on <c>info-bulletin-601.doc</c>, whose every page carries one full-page opaque
    /// raster anchored in the header story: emitted after the text it covers the text, and the document
    /// renders as five blank sheets while its extractable word count reads 1298 of 1302 — a defect no
    /// gate column can see, because the words are all still in the PDF's text layer underneath.
    /// </para>
    /// <para>
    /// The footer stays last, and that is deliberate rather than left over: it is what kept the page
    /// numbers legible on that document while everything above them was buried.
    /// </para>
    /// </remarks>
    /// <param name="page">The page to draw.</param>
    /// <param name="blocks">The blocks the page's body lines index into.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public static void Draw(
        LaidOutPage page, IReadOnlyList<PageBlock> blocks, IDrawingSink sink)
    {
        OfficeOperation.ThrowIfCancelled();
        ArgumentNullException.ThrowIfNull(page);
        ArgumentNullException.ThrowIfNull(blocks);
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(page.Size);
        try
        {
            DrawPageBorder(page, sink);

            foreach (PlacedFrame frame in Stacked(page.Frames, behind: true))
            {
                DrawFrame(frame, sink);
            }

            DrawFlow(page.Header, sink);
            DrawBody(page, blocks, sink);
            DrawLineNumbers(page, sink);
            foreach (PlacedTable table in page.Tables) DrawTable(table, sink);
            DrawSeparator(page.NoteSeparator, sink);
            DrawFlow(page.Notes, sink);

            foreach (PlacedFrame frame in Stacked(page.Frames, behind: false))
            {
                DrawFrame(frame, sink);
            }

            DrawFlow(page.Footer, sink);
        }
        finally
        {
            // Always closed, even if a sink throws part way through: a page left open would make the
            // next one nest inside it, turning one bad page into a broken document.
            sink.EndPage();
        }
    }

    /// <summary>
    /// Draws the section's page border and its shadow, if it has one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This was not drawn at all until round 67</strong>, and no gate column could see it:
    /// a border adds no words and no pages, so all seven corpus documents that declare one pass the
    /// word gate while missing a 4.5 pt rectangle round the whole page.
    /// <c>Case-Study-Heathrow-Airport.docx</c> is the witness — 11.55 of its 14.70 <c>|ink|%</c>
    /// against 26.2.4.2 is this one rectangle, which put it at the head of the words track's
    /// per-page ink ranking.
    /// </para>
    /// <para>
    /// The geometry is measured off 26.2.4.2's own PDF of that document rather than derived from
    /// the specification. A4 595.304 x 841.89, <c>w:sz="36"</c> (4.5 pt), <c>w:space="15"</c>,
    /// <c>w:offsetFrom="page"</c>, <c>w:shadow="1"</c>:
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// four strokes of <c>4.5 w</c> at <c>0.2235 0.3961 0.2 RG</c>, centrelines at 17.25 from the
    /// left and top — that is <c>space + width/2</c> — and at 573.60 and 21.69, which are
    /// <c>space + width/2</c> from the far edges <em>plus the shadow's own width</em>;
    /// </description></item>
    /// <item><description>
    /// each stroke spanning the rectangle's full outer extent in its own direction, so the corners
    /// overlap rather than mitre;
    /// </description></item>
    /// <item><description>
    /// the shadow as two black rectangles offset down and right by its width —
    /// <c>19.4 15.039 560.85 4.45 re f*</c> and <c>575.8 19.439 4.45 803.05 re f*</c>.
    /// </description></item>
    /// </list>
    /// <para>
    /// So the shadow <em>shrinks the box</em> rather than hanging off the paper, which is the one
    /// thing about it that cannot be guessed. Its width is taken as the border's own; the reference
    /// draws 4.45 against a stated 4.5, a twentieth of a point that no comparison resolves.
    /// </para>
    /// <para>
    /// Drawn first, under everything including a behind-text frame, and <strong><c>w:zOrder</c> is
    /// deliberately not honoured</strong>. It reaches <c>writerfilter</c>'s token table as
    /// <c>ooxml:CT_PageBorders_zOrder</c> and <c>PageBordersHandler::lcl_attribute</c> then reads
    /// only <c>display</c> and <c>offsetFrom</c>
    /// (<c>sw/source/writerfilter/dmapper/PageBordersHandler.cxx</c>:44-80), so LibreOffice never
    /// puts a page border in front of the text. Measured as well as read: a fixture declaring
    /// <c>w:zOrder="front"</c> renders on 26.2.4.2 stroke for stroke like the same fixture without
    /// it. Honouring the attribute would move us away from the reference, not towards it.
    /// </para>
    /// <para>
    /// An <em>art</em> border — one of the 165 <c>w:val</c> values naming a repeating device rather
    /// than a line — draws nothing, which is also the reference's behaviour rather than a gap:
    /// <see cref="BorderRules.WordStyleOf"/> answers 0 for a token Writer's own map does not know
    /// and <see cref="BorderRules.FromWord"/> answers null for that, exactly as
    /// <c>lcl_convertBorderStyleFromToken</c> returns <c>none</c>. Fixtures declaring
    /// <c>apples</c> and <c>basicWhiteDashes</c> render on 26.2.4.2 with no stroke, no image and
    /// the text in the same place as an unbordered page.
    /// </para>
    /// </remarks>
    private static void DrawPageBorder(LaidOutPage page, IDrawingSink sink)
    {
        if (page.Borders is not { } border) return;

        DocRect outer = border.Outer;

        if (border.Shadow > Length.Zero)
        {
            Fill(
                new DocRect(outer.X + border.Shadow, outer.Bottom, outer.Width, border.Shadow),
                Colour.Black,
                sink);
            Fill(
                new DocRect(outer.Right, outer.Y + border.Shadow, border.Shadow, outer.Height),
                Colour.Black,
                sink);
        }

        StrokeSide(border.Top, sink,
            new DocPoint(outer.X, outer.Y + border.Top.Width / 2),
            new DocPoint(outer.Right, outer.Y + border.Top.Width / 2));
        StrokeSide(border.Bottom, sink,
            new DocPoint(outer.X, outer.Bottom - border.Bottom.Width / 2),
            new DocPoint(outer.Right, outer.Bottom - border.Bottom.Width / 2));
        StrokeSide(border.Left, sink,
            new DocPoint(outer.X + border.Left.Width / 2, outer.Y),
            new DocPoint(outer.X + border.Left.Width / 2, outer.Bottom));
        StrokeSide(border.Right, sink,
            new DocPoint(outer.Right - border.Right.Width / 2, outer.Y),
            new DocPoint(outer.Right - border.Right.Width / 2, outer.Bottom));
    }

    /// <summary>One side of a page border, or nothing when the side draws none.</summary>
    private static void StrokeSide(PageBorderSide side, IDrawingSink sink, DocPoint from, DocPoint to)
    {
        if (!side.Draws) return;

        sink.StrokePath(
            new GraphicsPath().MoveTo(from).LineTo(to),
            new Stroke(Paint.Solid(side.Colour), side.Width));
    }

    /// <summary>
    /// One side of the page's frames, back to front by the z order their anchors declare.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Document order is not paint order. A <c>wp:anchor</c> carries its own <c>relativeHeight</c>,
    /// and a file is free to declare a background last and a caption first — which is exactly what
    /// the corpus's templates do: of the five documents where this was measured, all five declare
    /// <c>relativeHeight</c> on every anchor and not one of them is in document order.
    /// </para>
    /// <para>
    /// Painting in document order there does not look like a z-order fault. It looks like missing
    /// content: the text is drawn, correctly positioned, and a shape declared later covers it, so
    /// every pixel metric reports the page as having lost it. Five separate readings in this
    /// repository's parity catalogue were one instance of this.
    /// </para>
    /// <para>
    /// <see cref="Enumerable.OrderBy{TSource, TKey}(IEnumerable{TSource}, Func{TSource, TKey})"/> is
    /// a stable sort, which is the property that matters as much as the ordering itself: frames whose
    /// anchors declare no <c>relativeHeight</c> all compare equal at zero and therefore keep document
    /// order among themselves, which is both Word's own tie-break and exactly what this code did
    /// before the z order was read at all.
    /// </para>
    /// </remarks>
    private static IEnumerable<PlacedFrame> Stacked(
        IReadOnlyList<PlacedFrame> frames, bool behind) =>
        frames.Where(frame => frame.Frame.BehindText == behind)
              .OrderBy(frame => frame.Frame.ZOrder);

    /// <summary>
    /// Draws the body's lines, each relative to the rectangle of the column it landed in.
    /// </summary>
    /// <remarks>
    /// Grouped by column rather than looked up per line, because the rectangle is the same for every line of
    /// a column and computing it per line would divide the body's width by the column count once per line of
    /// the page. Single-column text — which is nearly everything — takes one group and one lookup.
    /// </remarks>
    private static void DrawBody(
        LaidOutPage page,
        IReadOnlyList<PageBlock> blocks,
        IDrawingSink sink,
        Colour background = default)
    {
        if (page.ColumnCount <= 1
            && page.Lines.All(line => line.Columns <= 1 && page.BodyAreaOf(line) == page.BodyArea))
        {
            DrawLines(page.BodyArea, page.Lines, blocks, sink, background);
            return;
        }

        // Grouped by the *line's* own band rather than by the page's column index, because one page can
        // carry sections that disagree about how many columns there are — see `PlacedLine.Columns` — and
        // about the text area those columns divide, which is what the last two members are: a text
        // section's own indents make a full-measure line above it and one below it different bands.
        foreach (IGrouping<(int Columns, int Column, Length Left, Length Width), PlacedLine> band in
                 page.Lines.GroupBy(
                     line => (line.Columns, line.Column, line.BodyLeft, line.BodyWidth)))
        {
            DrawLines(page.ColumnArea(band.First()), [.. band], blocks, sink, background);
        }
    }

    /// <summary>
    /// Draws a page's margin line numbers, right-aligned against the text edge.
    /// </summary>
    /// <remarks>
    /// Right-aligned here rather than in <see cref="LineNumbering"/> because aligning means measuring, and
    /// measuring means shaping: the model carries the right edge and this shapes the digits once, to draw
    /// them and to know how far left of that edge to start. Measured on the reference, whose one-, two-
    /// and three-digit numbers sit at three different left edges and one right one.
    /// </remarks>
    private static void DrawLineNumbers(LaidOutPage page, IDrawingSink sink)
    {
        if (page.LineNumbers.Count == 0) return;
        if (page.Numbering is not { } numbering) return;

        FontReference font = numbering.Font ?? Reference(numbering.Face);

        foreach (PageLineNumber mark in page.LineNumbers)
        {
            ShapedText shaped = TextShaper.Default.Shape(numbering.Face, mark.Text, numbering.Shaping);
            Length width = shaped.Width(numbering.EmSize);

            sink.DrawGlyphRun(
                Build(
                    shaped,
                    mark.Text,
                    numbering.EmSize,
                    font,
                    new DocPoint(mark.RightBaseline.X - width, mark.RightBaseline.Y),
                    Length.Zero),
                Paint.Solid(Colour.Black));
        }
    }

    /// <summary>
    /// Draws the rule above a page's notes.
    /// </summary>
    /// <remarks>
    /// Filled rather than stroked, which is what LibreOffice's own PDF export does: it writes the separator as
    /// a closed rectangular path and fills it, so its thickness is the rectangle's height rather than a pen
    /// width. Matching that is not pedantry — a stroke is centred on its path, so the same coordinates stroked
    /// would put half the rule's thickness on the wrong side of the line.
    /// </remarks>
    private static void DrawSeparator(DocRect? separator, IDrawingSink sink)
    {
        if (separator is { } rule) Fill(rule, Colour.Black, sink);
    }

    /// <summary>Draws a flow — a header, a footer or a cell — which is lines in their own rectangle.</summary>
    /// <param name="flow">The flow, or null when there is none.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="background">
    /// What is painted behind this flow, for a run whose colour is automatic — see
    /// <see cref="Automatic"/>. Transparent means the page, which is never dark.
    /// </param>
    private static void DrawFlow(PlacedFlow? flow, IDrawingSink sink, Colour background = default)
    {
        if (flow is null || flow.IsEmpty) return;

        DrawLines(flow.Area, flow.Lines, flow.Blocks, sink, background);
        foreach (PlacedTable table in flow.Tables) DrawTable(table, sink, background);
    }

    /// <summary>
    /// Draws a floating frame: its background, its own text, and its border.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Background, then content, then border — the order a table cell is drawn in and for the same reason:
    /// a border runs through the centre of its own line, so half of it overlaps whatever is inside.
    /// </para>
    /// <para>
    /// A picture goes between the two, for the same reason its own text would: it covers the background
    /// and the border is drawn over its edge. The image is handed over exactly as the reader found it —
    /// still the file's own bytes — because the backend is what has a codec, and a PDF backend given a
    /// JPEG passes it through to <c>DCTDecode</c> without ever decoding it. A frame the document called a
    /// picture whose bytes could not be found draws as it always did: its background and its border, and
    /// a hole where the pixels would have gone.
    /// </para>
    /// <para>
    /// <strong>A vector picture is stretched onto the frame by its own view box, not by its ink.</strong>
    /// <c>VectorImage.Draw</c> maps the picture's whole frame onto the destination and clips to it, which
    /// is the mapping LibreOffice uses for a <c>Graphic</c> on an <c>SdrObject</c>'s logic rectangle. This
    /// is the one call a reader gets wrong first: taking the extent of what the picture actually paints
    /// instead makes a logo with margins come out several times too large and clipped, which reads as a
    /// mapping bug in the decoder and is not one.
    /// </para>
    /// <para>
    /// The vector wins over the raster where a frame has both, which happens only for a DrawingML
    /// <c>svgBlip</c>. A decode that comes back empty falls through to the raster, which is what that
    /// fallback is written into the file for.
    /// </para>
    /// <para>
    /// <strong>A chart wins over both, and that ordering is the whole of the ODT case.</strong> ODF
    /// stores a chart as a <c>draw:object</c> followed by a <c>draw:image</c> of it — a picture of the
    /// chart for a reader that cannot embed one — so a frame holding a chart usually holds a replacement
    /// picture too. Every one LibreOffice writes is a <c>VCLMTF</c> StarView metafile, which nothing
    /// here decodes, so the fall-through costs nothing today; drawing the composed chart is still the
    /// right answer whatever the fallback turns out to be, because it is live geometry rather than a
    /// snapshot taken at some other size.
    /// </para>
    /// </remarks>
    private static void DrawFrame(PlacedFrame frame, IDrawingSink sink)
    {
        // A turned shape is the same drawing in a turned space, so the fill, the outline and a
        // picture all go through one transform rather than each being rotated in its own terms.
        // `a:xfrm/@rot` turns the shape about the centre of its stated rectangle, which is what the
        // translate-rotate-translate composition below says.
        //
        // The text goes through its own, because `wps:bodyPr/@rot` states the text's angle rather
        // than an addition to the shape's — see `PageFrame.TextRotationDegrees` for the census that
        // establishes it and the reference rendering that confirms it. The two are usually equal and
        // are usually both zero.
        AffineTransform? shape = Turn(frame.Ink, frame.Frame.RotationDegrees);
        AffineTransform? text = Turn(frame.Area, frame.Frame.TextRotationDegrees);

        (GraphicsPath? filled, GraphicsPath? stroked) = Outlines(frame);

        Turned(sink, shape);

        // A gradient before a colour, because the two are never both set and a gradient is the
        // one that needs the placed rectangle: it is carried unplaced on the frame and becomes a
        // paint here, against the area the layout engine settled on.
        if (frame.Frame.Gradient is { } ramp)
        {
            GradientPaint paint = ramp.Paint(frame.Ink);
            if (filled is null) Fill(frame.Ink, paint, sink);
            else sink.FillPath(filled, paint);
        }
        else if (frame.Frame.Fill is { } fill)
        {
            if (filled is null) Fill(frame.Ink, fill, sink);
            else sink.FillPath(filled, Paint.Solid(fill));
        }

        if (frame.Frame.Chart is { } chart)
            FrameChart.Draw(sink, chart, frame.Ink, frame.Frame.ChartFontFamily);
        else if (frame.Frame.Vector is { } vector && !vector.Value.IsEmpty)
            DrawPicture(sink, frame, filled, vector, null);
        else if (frame.Frame.Image is { } image) DrawPicture(sink, frame, filled, null, image);

        Upright(sink, shape);

        // The frame's own fill *is* the background an automatic font colour resolves against, and the
        // reason it was not, for four rounds, is that the two witnesses against it were misread.
        //
        // Round 59 passed the fill, measured 383 glyphs turning white that the reference draws black
        // across `docs-quality-MA.IMS.00001-…docx` page 9 (`#0070C0`, WCAG 39) and
        // `069_Work_Breakdown_Structure_Template_Professional…` (`#8496B0`, WCAG 76), and removed the
        // arm — concluding that such a shape's text must belong to the drawing layer and never reach
        // `ApplyAutoColor`. Round 62 then established the opposite on four inverted arms of `012`.
        // **Both measurements are right and neither hypothesis is**: both witnesses state a
        // transparency, `ApplyAutoColor` asks `getAverageColor` and not the fill, and blended toward
        // white those two fills are luminance 106 and 172. See `AutomaticColour`, which carries the
        // three-colour bracket that pins the blend.
        //
        // A frame stating no fill still resolves to black here rather than continuing to its
        // *anchor's* background, which is the other limb of round 62's rule and the one `012`'s white
        // title needs. The anchor is not reachable from here — frames are drawn from a per-page list —
        // so that limb is still open.
        // A fixed-height body whose text does not fit is CLIPPED to its text area, and the clip is
        // what the reference's text layer is missing rather than the text itself.
        //
        // `EditEngine` paints under a scissor whenever the content range leaves the clip range —
        // `if (!aClipRange.isInside(aContentRange))`, `editeng/source/editeng/impedit3.cxx`:4174-4181
        // — so an overflowing shape's spill is drawn and then masked away. Ours was painted through.
        //
        // [bin] Measured on eight corpus diagram templates against 26.2.4.2. Our text is
        // character-for-character identical to the reference's on all eight: read with `pdftotext`,
        // which ignores clip paths, the alphanumeric multisets match exactly and the word multisets
        // match on seven of eight. Read with MuPDF, which honours them, the reference is 18 to 102
        // characters shorter — and those characters are precisely the caps and ascenders poking above
        // a clip edge, at the same font, size and x to 0.1 pt. That is a mask, not a layout
        // difference, and it is why this reads to a text-extraction gate as *we draw too much*.
        //
        // The rectangle is measured rather than assumed, on a 100 x 24 pt box at three inset pairs:
        // the VERTICAL insets apply and the HORIZONTAL ones do not, so it is the shape's full width
        // between the text area's top and bottom. It is emitted outside `Turned`/`Upright` because
        // the reference's scissor is axis-aligned in page space even for turned text.
        //
        // Conditional on purpose, for the reason `DrawPicture` gives below: an unconditional clip
        // would be a visual no-op that changed the bytes of every rendering carrying a shape. A body
        // that fits is not clipped and neither is an autofitting one — `a:spAutoFit` measured as
        // never clipped, `a:normAutofit` measured as not shrinking and behaving as `a:noAutofit`.
        DocRect inside = frame.Area.Deflate(frame.Frame.Padding);
        bool clips = frame.Frame.HasFixedHeight
                     && frame.Content is { } overflowing
                     && FlowLayouter.Extent(overflowing) > inside.Height;

        if (clips)
        {
            sink.Save();
            sink.ClipPath(GraphicsPath.Rectangle(
                new DocRect(frame.Area.X, inside.Y, frame.Area.Width, inside.Height)));
        }

        try
        {
            Turned(sink, text);
            DrawFlow(frame.Content, sink, frame.Frame.Fill ?? default);
            Upright(sink, text);
        }
        finally
        {
            if (clips) sink.Restore();
        }

        if (frame.Frame.BorderColour is not { } colour) return;
        if (frame.Frame.BorderWidth <= Length.Zero) return;

        Turned(sink, shape);
        try
        {
            DrawBorder(frame, stroked, colour, sink);
        }
        finally
        {
            Upright(sink, shape);
        }
    }

    /// <summary>
    /// The rotation a frame is drawn through at a given angle, or null when it is square to the page.
    /// </summary>
    /// <remarks>
    /// Null rather than the identity so that the overwhelming majority of frames pay neither a
    /// <c>Save</c>/<c>Restore</c> pair nor a transform in the output: a PDF content stream gains
    /// three operators per turned shape and none per upright one.
    /// </remarks>
    private static AffineTransform? Turn(DocRect area, double degrees)
    {
        if (degrees == 0) return null;

        double cx = area.X.Emu + (area.Width.Emu / 2.0);
        double cy = area.Y.Emu + (area.Height.Emu / 2.0);

        return AffineTransform.Concat(
            AffineTransform.Concat(
                AffineTransform.Translation(-cx, -cy),
                AffineTransform.Rotation(degrees * Math.PI / 180.0)),
            AffineTransform.Translation(cx, cy));
    }

    /// <summary>Enters a turned space, or does nothing when there is no turn.</summary>
    private static void Turned(IDrawingSink sink, AffineTransform? turn)
    {
        if (turn is not { } transform) return;

        sink.Save();
        sink.Transform(transform);
    }

    /// <summary>Leaves it again.</summary>
    private static void Upright(IDrawingSink sink, AffineTransform? turn)
    {
        if (turn is not null) sink.Restore();
    }

    /// <summary>The frame's outline, stroked where the shape's own geometry says.</summary>
    private static void DrawBorder(
        PlacedFrame frame, GraphicsPath? outline, Colour colour, IDrawingSink sink)
    {
        // The dash pattern is expanded here rather than at the reader because it is a function of the
        // pen width and the cap as well as of the preset's name, and only the width is settled by the
        // time the frame is built. `capExtendsDash` is the round/square case, where MSO measures the
        // cap inside the ink and LibreOffice shortens the ink to compensate.
        Stroke stroke = new(
            Paint.Solid(colour),
            frame.Frame.BorderWidth,
            Cap: frame.Frame.BorderCap,
            DashPattern: DashPresets.Pattern(
                frame.Frame.BorderDash,
                frame.Frame.BorderWidth,
                frame.Frame.BorderCap is not LineCap.Butt));
        DocRect area = frame.Ink;

        // The border is stroked inside the room the frame took, where it says so. See
        // PageFrame.BorderInset -- a form checkbox is the only thing that does.
        if (frame.Frame.BorderInset > Length.Zero)
        {
            Length inset = frame.Frame.BorderInset;
            area = new DocRect(
                area.X + inset, area.Y + inset,
                Length.Max(Length.Zero, area.Width - inset - inset),
                Length.Max(Length.Zero, area.Height - inset - inset));
        }

        // A line shape's outline is its diagonal rather than its rectangle: corner to opposite corner,
        // which is the two-point path `ImportShape` builds for it, with the mirror flags choosing which
        // pair of corners. Drawing the box instead puts three sides on the page that are not in the file.
        // The shape's own geometry, when it declares one. Before the line shape below, because a
        // preset never reaches here -- `line` and `straightConnector1` are excluded at the reader.
        if (outline is not null)
        {
            Arrowed(outline, frame, stroke, sink);
            return;
        }

        if (frame.Frame.IsLine)
        {
            DocPoint near = new(area.X, frame.Frame.IsLineMirrored ? area.Bottom : area.Y);
            DocPoint far = new(area.Right, frame.Frame.IsLineMirrored ? area.Y : area.Bottom);

            // Which end the line starts at, which is invisible until it carries an arrowhead —
            // see PageFrame.IsLineReversed.
            (DocPoint from, DocPoint to) =
                frame.Frame.IsLineReversed ? (far, near) : (near, far);

            Arrowed(new GraphicsPath().MoveTo(from).LineTo(to), frame, stroke, sink);
            return;
        }

        sink.StrokePath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.X, area.Y))
                .LineTo(new DocPoint(area.Right, area.Y))
                .LineTo(new DocPoint(area.Right, area.Bottom))
                .LineTo(new DocPoint(area.X, area.Bottom))
                .Close(),
            stroke);

        if (!frame.Frame.IsCrossed) return;

        // Both diagonals of the same rectangle, which is what a *checked* box is.
        sink.StrokePath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.X, area.Y))
                .LineTo(new DocPoint(area.Right, area.Bottom)),
            stroke);
        sink.StrokePath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.Right, area.Y))
                .LineTo(new DocPoint(area.X, area.Bottom)),
            stroke);
    }

    /// <summary>
    /// Strokes a path with whichever arrowheads its shape declares, shortened to make room.
    /// </summary>
    /// <remarks>
    /// <para>
    /// An arrowhead is a filled polygon beside the shaft rather than a property of the pen, so
    /// there is nothing for a backend to know about: <see cref="LineEnds.Apply"/> returns the
    /// shortened line and one closed path per end, filled with the line's own paint. It is
    /// LibreOffice's own decomposition, at the same layer —
    /// <c>PolygonStrokeArrowPrimitive2D</c> becomes a stroke and up to two filled polygons.
    /// </para>
    /// <para>
    /// Called for every stroked shaft, including ones that carry no marker, because
    /// <see cref="LineEnds.Apply"/> hands a path straight back when neither end names a shape and
    /// when the path is not an open polyline. The rectangle border below therefore needs no test
    /// of its own — a closed path is left exactly as it was.
    /// </para>
    /// <para>
    /// The corpus has <b>608 line ends across 38 <c>docx</c></b> — 353 tails and 255 heads, with
    /// 208 of them in one integrated-management-system manual. Every one drew as a plain line,
    /// which on a flowchart is the difference between a diagram and a set of boxes joined by
    /// sticks.
    /// </para>
    /// </remarks>
    private static void Arrowed(
        GraphicsPath path, PlacedFrame frame, Stroke stroke, IDrawingSink sink)
    {
        if (frame.Frame.HeadEnd.Type is null && frame.Frame.TailEnd.Type is null)
        {
            sink.StrokePath(path, stroke);
            return;
        }

        (GraphicsPath shaft, List<GraphicsPath> markers) =
            LineEnds.Apply(path, stroke, frame.Frame.HeadEnd, frame.Frame.TailEnd);

        sink.StrokePath(shaft, stroke);
        foreach (GraphicsPath marker in markers) sink.FillPath(marker, stroke.Paint);
    }

    /// <summary>
    /// Draws a frame's picture into its area, cropped when the file says so.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A crop is a larger destination plus a clip, and until this round the word path had
    /// neither.</strong> The fractions say how much of the picture each edge throws away, so the
    /// surviving part is what fills the frame and the whole of it is correspondingly bigger; the
    /// clip to the frame is what turns that into a crop rather than into a picture drawn over the
    /// text on either side of it. Adding the rectangle without the clip would be strictly worse
    /// than doing nothing.
    /// </para>
    /// <para>
    /// <strong>And a picture frame need not be rectangular.</strong> A <c>pic:pic</c> carries an
    /// <c>a:prstGeom</c> in its own <c>pic:spPr</c> exactly as a <c>wps:wsp</c> does, and
    /// LibreOffice fills that geometry with the bitmap rather than the bounding box — <c>oox</c>
    /// gives such a picture a <c>com.sun.star.drawing.CustomShape</c> service with a bitmap fill
    /// (<c>Shape::createAndInsert</c>, <c>oox/source/drawingml/shape.cxx</c>), and 26.2.4.2's own
    /// PDF emits the preset's outline as a clip path immediately before the image operator. So the
    /// outline bounds the picture whenever the shape states one, whether or not anything is
    /// cropped. The slide side has clipped to the outline all along; only this one did not.
    /// </para>
    /// <para>
    /// Clipped nowhere else. Both backends already confine a picture to the rectangle they are
    /// given — a raster is stretched onto exactly it, and <c>VectorImage.Draw</c> clips to its
    /// destination — so an unconditional clip would be a no-op that changed the bytes of every
    /// rendering carrying a picture.
    /// </para>
    /// <para>
    /// The border is drawn after this and is deliberately outside the clip: it belongs to the
    /// frame rather than to the picture, and half a hairline of it falls on the boundary the clip
    /// would have cut away.
    /// </para>
    /// </remarks>
    /// <param name="sink">Where to draw.</param>
    /// <param name="frame">The frame holding the picture.</param>
    /// <param name="outline">
    /// The shape's own filled geometry, placed in the page, or null when it states none and its
    /// rectangle is its outline.
    /// </param>
    /// <param name="vector">The vector picture, where the frame has one.</param>
    /// <param name="image">The raster picture, where the frame has one.</param>
    private static void DrawPicture(
        IDrawingSink sink,
        PlacedFrame frame,
        GraphicsPath? outline,
        Lazy<VectorImage>? vector,
        RasterImage? image)
    {
        DocRect destination = frame.Frame.Crop.Apply(frame.Ink);

        if (destination == frame.Ink && outline is null)
        {
            PaintPicture(sink, frame.Ink, vector, image);
            return;
        }

        sink.Save();
        try
        {
            sink.ClipPath(outline ?? GraphicsPath.Rectangle(frame.Ink));
            PaintPicture(sink, destination, vector, image);
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>Puts whichever of the two pictures a frame has into a rectangle.</summary>
    private static void PaintPicture(
        IDrawingSink sink, DocRect where, Lazy<VectorImage>? vector, RasterImage? image)
    {
        if (vector is not null) vector.Value.Draw(sink, where);
        else if (image is not null) sink.DrawImage(image, where);
    }

    /// <summary>
    /// Draws a table, which is its cells' text.
    /// </summary>
    /// <remarks>
    /// Shading behind the text and borders over it, which is paint order rather than preference: a border
    /// runs through the centre of a grid line, so half its width overlaps the cells either side of it.
    /// </remarks>
    private static void DrawTable(PlacedTable table, IDrawingSink sink, Colour background = default)
    {
        // Every shade before any text, rather than each cell's shade before its own text: a shade is opaque,
        // and a cell whose content overflows into its neighbour would otherwise have that overflow painted
        // over by the neighbour's fill.
        foreach (PlacedTableCell cell in table.Cells)
        {
            OfficeOperation.Poll();
            if (cell.Cell.Shading is { } colour) Fill(cell.Area, colour, sink);
        }

        foreach (PlacedTableCell cell in table.Cells)
        {
            // The cell's own fill where it has one, and whatever was behind the table where it has
            // not: `SwFrame::GetBackgroundBrush` walks up the frame chain until something answers,
            // so a cell in a shaded frame is on the frame's colour rather than on the page's.
            DrawCellContent(cell, sink, cell.Cell.Shading ?? background);
        }

        DrawBorders(table, sink);
    }

    /// <summary>
    /// Draws one cell's text, turning it first when the cell says so.
    /// </summary>
    /// <remarks>
    /// The whole of the turned case, and deliberately so: the flow underneath is an ordinary upright one
    /// in its own coordinates, so every backend draws it through the code it already had and only the
    /// transform is new. LibreOffice's own PDF does the same thing — a <c>0 1 -1 0 x y</c> text matrix
    /// and an otherwise unremarkable run — which is why the turned text stays real text in the output
    /// rather than becoming a picture of itself.
    /// </remarks>
    private static void DrawCellContent(
        PlacedTableCell cell, IDrawingSink sink, Colour background = default)
    {
        if (cell.ContentTransform is not { } onto)
        {
            DrawFlow(cell.Content, sink, background);
            return;
        }

        if (cell.Content is null) return;

        sink.Save();
        try
        {
            sink.Transform(onto);
            DrawFlow(cell.Content, sink, background);
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>
    /// Draws a table's borders, consolidated the way LibreOffice consolidates them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One stroke per <em>grid line</em> rather than four round each cell, which is measured rather than
    /// chosen: LibreOffice writes five horizontals for a four-row table and one vertical per column boundary,
    /// each as a single <c>m … l S</c>. Drawing twelve short segments instead would be right on the page and
    /// incomparable against the reference.
    /// </para>
    /// <para>
    /// Two details of the geometry, both measured. A grid line runs through the <em>centre</em> of the border,
    /// and every stroke <strong>overshoots by half its own width at both ends</strong> — on a table spanning
    /// 56.7 to 538.6 pt the horizontals run 56.45 to 538.85. So the overshoot is what makes two perpendicular
    /// borders meet at a corner rather than leaving a notch.
    /// </para>
    /// <para>
    /// The overshoot is Writer's rule and not Word's, which is what
    /// <see cref="PageTable.JoinsBordersLikeWord"/> switches: a Word table shortens an interior line by the
    /// <em>full</em> width of the outer line it meets, so the outline owns the corner outright. Measured, the
    /// same table read from DOC or DOCX runs its middle horizontals 56.95 to 538.35 where the ODF one runs
    /// them 56.45 to 538.85.
    /// </para>
    /// <para>
    /// Horizontals before verticals, which matters only for which one wins a join and is free to match.
    /// </para>
    /// </remarks>
    private static void DrawBorders(PlacedTable table, IDrawingSink sink)
    {
        // Collected per grid line and merged, so that a run of cells agreeing about an edge becomes one
        // stroke. Keyed on the line's own coordinate rounded to a twip, because two cells' shared edge is
        // computed from two different rectangles and can differ in the last EMU.
        List<Edge> edges = WithSpans(Edges(table), table.Table.JoinsBordersLikeWord);

        foreach (Edge edge in edges)
        {
            // The stated width is the whole rule's, so a double's two strokes are drawn inside it — one
            // against each edge of the band. See `BorderRules`.
            BorderBands bands = edge.Border.Bands;
            IReadOnlyList<Length>? dashes = BorderRules.Dashes(edge.Border.Line);
            Length half = edge.Border.Width / 2;

            // A horizontal rule meets a vertical one, which is CENTRED on its line, so it overshoots by
            // half a rule at each end. A vertical's own span is already final — `WithSpans` settles it,
            // because a horizontal band HANGS DOWNWARDS and the two ends are therefore not symmetrical.
            Length from = edge.IsHorizontal ? edge.From - half : edge.From;
            Length to = edge.IsHorizontal ? edge.To + half : edge.To;

            // Where this band's top edge sits, relative to the grid line. A vertical border is CENTRED
            // on its line and a horizontal one HANGS DOWNWARDS FROM IT: [src] `SwTabFramePainter::Insert`
            // (`sw/source/core/layout/paintfrm.cxx`:3061-3064) sets `RefMode::Begin` on a cell's two
            // horizontal borders -- "drawn below the reference points", `mfRefModeOffset = +width/2` --
            // and `Centered` on its two vertical ones. So every band on one horizontal line begins at the
            // line, whatever its own width, and no reference to the widest of them is needed: the layout
            // puts the grid line ON the row boundary and `TableLayouter.TopBand` charges the row below for
            // the whole band. [bin] `probes/tablerow-r146/results.md` §3.1 -- three columns stating
            // 0.5/3.0/1.5 pt across one boundary draw 95.001..95.501, 95.001..98.001 and 95.001..96.501.
            //
            // The two are written as separate expressions rather than as one parameterised by `top`,
            // and that is not tidiness: `Length`'s division ROUNDS (`operator /`, `Length.cs`:140), so
            // `half - Inner/2` and `-half + Width - Inner/2` — the same arithmetic re-associated —
            // differ by one EMU whenever the width is an odd number of them. Written as one expression
            // this round moved a doubled *vertical* border on `150_5300_13_chg12.doc` by 0.0001 pt,
            // which it has no business touching at all; the confinement sweep is what caught it.
            if (edge.IsHorizontal)
            {
                Rule(bands.Outer / 2, bands.Outer);
                if (bands.HasTwoRules) Rule(edge.Border.Width - (bands.Inner / 2), bands.Inner);
            }
            else
            {
                Rule((bands.Outer / 2) - half, bands.Outer);
                if (bands.HasTwoRules) Rule(half - (bands.Inner / 2), bands.Inner);
            }

            // `offset` is where the stroke's own centre sits relative to the grid line.
            void Rule(Length offset, Length thick)
            {
                if (thick <= Length.Zero) return;

                Length at = edge.At + offset;
                Stroke stroke = new(
                    Paint.Solid(edge.Border.Colour), thick, DashPattern: dashes);

                GraphicsPath path = edge.IsHorizontal
                    ? new GraphicsPath()
                        .MoveTo(new DocPoint(from, at))
                        .LineTo(new DocPoint(to, at))
                    : new GraphicsPath()
                        .MoveTo(new DocPoint(at, from))
                        .LineTo(new DocPoint(at, to));

                sink.StrokePath(path, stroke);
            }
        }
    }

    /// <summary>One consolidated grid line: where it sits, how far it runs, and its border.</summary>
    /// <param name="IsHorizontal">True when it runs across the page.</param>
    /// <param name="At">Where it sits on the other axis.</param>
    /// <param name="From">Where it starts along its own axis.</param>
    /// <param name="To">Where it ends.</param>
    /// <param name="Border">Its width and colour.</param>
    /// <param name="IsOuter">
    /// True when it is part of the table's outline rather than of its grid, which only Word's join rule
    /// cares about: the outline keeps its full length and the inner lines give way to it.
    /// </param>
    private readonly record struct Edge(
        bool IsHorizontal, Length At, Length From, Length To, TableBorder Border, bool IsOuter = false);

    /// <summary>
    /// Every grid line's final span: how far a vertical reaches into the bands it meets, and Word's join
    /// rule, by which an inner line gives way to the outline.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The two ends of a vertical are not symmetrical, because a horizontal band hangs downwards
    /// from its line rather than being centred on it</strong> ([src] <c>SwTabFramePainter::Insert</c>,
    /// <c>paintfrm.cxx</c>:3061-3064, <c>RefMode::Begin</c> on the horizontals and <c>Centered</c> on the
    /// verticals). At its top the vertical already starts on the boundary, which is where that band
    /// begins, so it reaches nothing; at its bottom it has to cross the whole band or the corner is left
    /// open. Under the centred model this tree used until round 149 the two ends were one expression,
    /// <c>± width/2</c>, which agrees for as long as every rule in the table has one width.
    /// </para>
    /// <para>
    /// Word's join is then subtracted from both ends, by the <em>full</em> width of the outer line rather
    /// than half of it — ported from <c>SwTabFramePainter::FindStylesForLine</c>, which adjusts an inner
    /// entry's start and end for every outer entry it meets and does it before the half-width overshoot
    /// is added. For a vertical that is what makes it stop at the table's outline instead of crossing it,
    /// while still crossing every <em>interior</em> band on the way.
    /// </para>
    /// <para>
    /// [bin] Measured on <c>table-borders.docx</c> against 26.2.4.2, whose three verticals run
    /// <c>70.201..161.901</c> (outer: the first band's top to the last band's bottom),
    /// <c>70.701..161.401</c> (interior, full height: below the outer top band, stopping at the top of
    /// the outer bottom one) and <c>70.701..142.501</c> (interior, ending on an <em>interior</em>
    /// boundary, which it crosses). Getting any one of the three wrong is a whole rule width, and all
    /// three are reproduced.
    /// </para>
    /// </remarks>
    /// <param name="edges">The merged grid lines.</param>
    /// <param name="joinsLikeWord">Whether the table applies Word's join rule; see <c>PageTable</c>.</param>
    private static List<Edge> WithSpans(List<Edge> edges, bool joinsLikeWord)
    {
        // The widest band at each horizontal line, and the widest OUTER band — the second is what an
        // inner line has to give way to, since only the outline clears a corner.
        Dictionary<long, Length> band = [];
        Dictionary<(bool, long), Length> outline = [];

        foreach (Edge edge in edges)
        {
            if (edge.IsHorizontal)
            {
                long line = edge.At.Twips;
                if (!band.TryGetValue(line, out Length seen) || edge.Border.Width > seen)
                    band[line] = edge.Border.Width;
            }

            if (!edge.IsOuter) continue;

            (bool, long) key = (edge.IsHorizontal, edge.At.Twips);
            if (!outline.TryGetValue(key, out Length width) || edge.Border.Width > width)
                outline[key] = edge.Border.Width;
        }

        List<Edge> spanned = new(edges.Count);
        foreach (Edge edge in edges)
        {
            // A vertical crosses the band at its lower boundary. Its upper end needs nothing: the band
            // there begins on the boundary it already starts at.
            Length to = edge.IsHorizontal
                ? edge.To
                : edge.To + band.GetValueOrDefault(edge.To.Twips, Length.Zero);

            if (edge.IsOuter || !joinsLikeWord)
            {
                spanned.Add(edge with { To = to });
                continue;
            }

            spanned.Add(edge with
            {
                From = edge.From + Meeting(edge, edge.From),
                To = to - Meeting(edge, edge.To),
            });
        }

        return spanned;

        Length Meeting(Edge edge, Length end)
            => outline.TryGetValue((!edge.IsHorizontal, end.Twips), out Length width)
                ? width
                : Length.Zero;
    }

    /// <summary>
    /// A table's grid lines, merged along each line where consecutive cells agree.
    /// </summary>
    /// <remarks>
    /// Horizontals first and then verticals, each built by grouping the cells' edges on the line they sit on
    /// and joining the runs that touch. A vertical therefore stops where its boundary stops, which is what
    /// LibreOffice does: a table whose last row spans two columns leaves that column's stroke short.
    /// </remarks>
    private static List<Edge> Edges(PlacedTable table)
    {
        List<Edge> loose = [];
        int columns = table.Table.ColumnWidths.Count;

        IReadOnlyList<PageTableRow> shared = table.Table.RowsWithSharedEdges;

        foreach (PlacedTableCell cell in table.Cells)
        {
            // The shared-edge borders rather than the cell's own, which is what closes the hole a page
            // break leaves: a row stating `w:top w:val="nil"` under a row stating a bottom border draws
            // nothing of its own, and where the two are on the same page the row above's bottom covers
            // the line, so the hole appears only at the top of a table's continuation page. See
            // `PageTable.RowsWithSharedEdges`. The row *heights* are deliberately not on it — the same
            // probe says they should be, and the corpus says our height model has other errors that
            // change interacts with; `probes/wordstable-r131/results.md` §4.
            CellBorders borders = Shared(shared, cell);
            DocRect area = cell.Area;

            // Which of a cell's edges belong to the table's outline, taken from where the cell sits
            // rather than from where its rectangle lands: a row whose cells are short of the grid still
            // has a last cell, and its right edge is still the outline.
            bool first = cell.Row <= table.FirstRow;
            bool last = cell.Row + Math.Max(1, cell.Cell.RowSpan) >= table.RowEnd;

            if (!borders.Top.IsNone)
                loose.Add(new Edge(true, area.Y, area.X, area.Right, borders.Top, first));
            if (!borders.Bottom.IsNone)
                loose.Add(new Edge(true, area.Bottom, area.X, area.Right, borders.Bottom, last));
            if (!borders.Left.IsNone)
                loose.Add(new Edge(false, area.X, area.Y, area.Bottom, borders.Left,
                    cell.Cell.Column == 0));
            if (!borders.Right.IsNone)
                loose.Add(new Edge(false, area.Right, area.Y, area.Bottom, borders.Right,
                    cell.Cell.ColumnEnd >= columns));
        }

        List<Edge> merged = [];

        foreach (bool horizontal in (bool[])[true, false])
        {
            IEnumerable<IGrouping<(long, Length, Colour), Edge>> lines = loose
                .Where(edge => edge.IsHorizontal == horizontal)
                .GroupBy(edge => (edge.At.Twips, edge.Border.Width, edge.Border.Colour));

            foreach (IGrouping<(long, Length, Colour), Edge> line in lines)
            {
                foreach (Edge run in Merge([.. line.OrderBy(edge => edge.From.Emu)]))
                {
                    merged.Add(run);
                }
            }
        }

        return merged;
    }

    /// <summary>One placed cell's borders, with the horizontal edges it shares resolved.</summary>
    /// <remarks>
    /// Matched on the row the cell starts in and the column it starts at, which is how a placed cell
    /// names itself; a cell the resolution did not touch comes back as the row stated it.
    /// </remarks>
    private static CellBorders Shared(IReadOnlyList<PageTableRow> rows, PlacedTableCell cell)
    {
        if (cell.Row < 0 || cell.Row >= rows.Count) return cell.Cell.Borders;

        foreach (PageTableCell other in rows[cell.Row].Cells)
        {
            if (other.Column == cell.Cell.Column) return other.Borders;
        }

        return cell.Cell.Borders;
    }

    /// <summary>Joins the runs along one grid line that touch or overlap.</summary>
    /// <remarks>
    /// Touching counts, and has to: two cells side by side produce two separate edges that meet exactly at
    /// the boundary between them, and a reference that wrote one stroke across both would disagree with two.
    /// </remarks>
    private static List<Edge> Merge(List<Edge> sorted)
    {
        List<Edge> runs = [];

        foreach (Edge edge in sorted)
        {
            if (runs.Count > 0 && edge.From <= runs[^1].To)
            {
                // Outer wins over inner across a merge, which matters for a run that starts as the
                // outline of one row and continues as the grid line of the next.
                runs[^1] = runs[^1] with
                {
                    To = edge.To > runs[^1].To ? edge.To : runs[^1].To,
                    IsOuter = runs[^1].IsOuter || edge.IsOuter,
                };
                continue;
            }

            runs.Add(edge);
        }

        return runs;
    }

    /// <summary>
    /// The two paths a frame's own geometry draws, placed in the page: what is filled and what is
    /// stroked. Nulls when the shape states no geometry and the caller should paint its rectangle.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Word documents declare shape geometry and this side used to ignore it.</strong> The
    /// same <c>spPr</c> was already being read for fill and outline, so every anchored shape was
    /// painted as its bounding rectangle whatever it asked for: a timeline's milestone circles came
    /// out as squares, a roadmap's chevrons as bars. The catalogue was never the gap — all 187
    /// presets are in <c>PresetShapeGeometry.txt</c> and the slide side has resolved them all along.
    /// </para>
    /// <para>
    /// <strong>Two paths and not one, because a subpath states whether it is filled and whether it
    /// is stroked.</strong> Every connector — <c>bentConnector1</c> to <c>5</c>, the curved ones —
    /// is a single open subpath declaring <c>fill="none"</c>, and a shape carrying one still takes
    /// a fill from its <c>a:fillRef</c>. Filling the whole outline of one draws a solid blob where
    /// the file states a line, which is what this did while it returned a single path.
    /// </para>
    /// <para>
    /// An <c>a:custGeom</c> arrives already resolved, on the frame, because its guide formulae need
    /// the shape's extent and the reader has it; a preset is a name, so it is cheapest evaluated
    /// once here with the placed rectangle in hand. Either way the result is translated rather than
    /// transformed — the geometry is in the shape's own coordinates with its origin at the top left,
    /// and any rotation is applied by the caller around the whole drawing.
    /// </para>
    /// <para>
    /// A preset the catalogue does not know returns nulls and the caller paints the rectangle,
    /// which is what LibreOffice falls back to as well.
    /// </para>
    /// </remarks>
    private static (GraphicsPath? Fill, GraphicsPath? Stroke) Outlines(PlacedFrame frame)
    {
        DocRect area = frame.Ink;
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return (null, null);

        if (frame.Frame.FillOutline is { } custom)
        {
            return (Placed(custom, area), Placed(frame.Frame.StrokeOutline, area));
        }

        if (frame.Frame.Preset is not { Length: > 0 } preset) return (null, null);

        if (CustomShapeGeometry.Preset(
                preset, new DocSize(area.Width, area.Height), frame.Frame.Adjustments)
            is not { } geometry)
        {
            return (null, null);
        }

        return (Placed(geometry.FillOutline, area), Placed(geometry.StrokeOutline, area));
    }

    /// <summary>A path in shape coordinates, moved to where the frame was placed.</summary>
    private static GraphicsPath? Placed(GraphicsPath? path, DocRect area)
    {
        if (path is null) return null;

        GraphicsPath placed = new();
        foreach (PathCommand command in path.Commands)
        {
            switch (command.Verb)
            {
                case PathVerb.MoveTo: placed.MoveTo(Shift(command.Point)); break;
                case PathVerb.LineTo: placed.LineTo(Shift(command.Point)); break;
                case PathVerb.CubicTo:
                    placed.CubicTo(
                        Shift(command.Control1), Shift(command.Control2), Shift(command.Point));
                    break;
                case PathVerb.Close: placed.Close(); break;
                default: break;
            }
        }

        return placed;

        DocPoint Shift(DocPoint point) => new(area.X + point.X, area.Y + point.Y);
    }

    /// <summary>Fills a rectangle, which is what a shade and a rule both are.</summary>
    private static void Fill(DocRect area, Colour colour, IDrawingSink sink)
        => Fill(area, Paint.Solid(colour), sink);

    /// <summary>
    /// The same, with any paint: the sink has no rectangle of its own, so both go through a path.
    /// </summary>
    private static void Fill(DocRect area, Paint paint, IDrawingSink sink)
    {
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return;

        GraphicsPath path = new GraphicsPath()
            .MoveTo(new DocPoint(area.X, area.Y))
            .LineTo(new DocPoint(area.Right, area.Y))
            .LineTo(new DocPoint(area.Right, area.Bottom))
            .LineTo(new DocPoint(area.X, area.Bottom))
            .Close();

        sink.FillPath(path, paint);
    }

    /// <summary>
    /// Draws a set of placed lines relative to an area.
    /// </summary>
    /// <remarks>
    /// One path for the body, the header and the footer, because they differ only in which rectangle their
    /// coordinates are relative to. A header drawn by its own code would be the second place tabs and
    /// per-run formatting had to be applied, and the two would drift.
    /// </remarks>
    private static void DrawLines(
        DocRect area,
        IReadOnlyList<PlacedLine> lines,
        IReadOnlyList<PageBlock> blocks,
        IDrawingSink sink,
        Colour background = default)
    {
        DrawParagraphShading(area, lines, blocks, sink);
        DrawParagraphBorders(area, lines, blocks, sink);

        foreach (PlacedLine line in lines)
        {
            OfficeOperation.Poll();
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= blocks.Count) continue;
            if (blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;

            List<(DocRect Area, Colour Colour)> highlights = [];
            List<(DocRect Area, Colour Colour)> rules = [];
            List<(DocRect Area, string Link)>? links = paragraph.HasLinks ? [] : null;
            List<(GlyphRun Run, Colour Colour)> runs =
                RunsIn(area, line, paragraph, highlights, rules,
                       paragraph.Shading ?? background, links);

            if (paragraph.Anchors.Count > 0 || paragraph.OutlineLevel > 0) Declare(area, line, paragraph, sink);

            // Every band on the line before any of its glyphs, not band-then-glyphs run by run: two
            // adjacent highlighted runs overlap by a fraction of a point where one's advance ends and the
            // next begins, and painting a band after its neighbour's text has been drawn clips the text.
            foreach ((DocRect band, Colour colour) in highlights) Fill(band, colour, sink);

            foreach ((GlyphRun run, Colour colour) in runs)
            {
                sink.DrawGlyphRun(run, Paint.Solid(colour));
            }

            // After the glyphs, which is the order every other layer here draws a decoration in and the
            // order Writer paints one: a strikethrough belongs over the letters it crosses out, and an
            // underline that a descender interrupts is what a font's own offset already expresses.
            foreach ((DocRect rule, Colour colour) in rules) Fill(rule, colour, sink);

            // Last: a link paints nothing, so where it falls in the order is only a matter of tidiness.
            if (links is not null)
            {
                foreach ((DocRect clickable, string link) in links) AddLink(clickable, link, sink);
            }
        }
    }

    /// <summary>
    /// Tells the sink about the named places and the outline entry a line carries.
    /// </summary>
    /// <remarks>
    /// A bookmark is declared on the line its offset falls in — the last line, for one at the very end of
    /// the text — at the line's own left edge and top, which is where a viewer scrolls to. A heading is
    /// declared once, on its first line.
    /// </remarks>
    private static void Declare(DocRect area, PlacedLine line, PageParagraph paragraph, IDrawingSink sink)
    {
        DocPoint at = new(area.X + line.Box.Left, area.Y + line.Top);
        int start = line.Box.Line.Start;
        int end = line.Box.Line.End;
        bool last = end >= paragraph.Text.Length;

        foreach (PageAnchor anchor in paragraph.Anchors)
        {
            bool here = (anchor.Offset >= start && (anchor.Offset < end || (last && anchor.Offset >= end)))
                        || (line.StartsParagraph && anchor.Offset < start);
            if (here && anchor.Name.Length > 0) sink.AddDestination(anchor.Name, at);
        }

        if (line.StartsParagraph && paragraph.OutlineLevel > 0)
        {
            string title = string.Join(' ', paragraph.Text
                .Split([' ', '\t', '\n', '\r', '\u2028', '\u00A0'], StringSplitOptions.RemoveEmptyEntries));
            if (title.Length > 0) sink.AddOutlineEntry(paragraph.OutlineLevel, title, at);
        }
    }

    /// <summary>A link to a bookmark when the target begins <c>#</c>, and to an address otherwise.</summary>
    private static void AddLink(DocRect area, string link, IDrawingSink sink)
    {
        if (link.Length > 1 && link[0] == '#') sink.AddInternalLink(area, link[1..]);
        else if (link.Length > 0) sink.AddLink(area, link);
    }

    /// <summary>
    /// Fills the background behind each shaded paragraph, before any of the text is drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Every shade before any glyph, for the reason a table's cells are drawn that way: a fill is opaque,
    /// and a shaded paragraph drawn after its neighbour would paint over the descenders hanging into it.
    /// </para>
    /// <para>
    /// <strong>The rectangle is the paragraph's print area, not its frame.</strong> Writer paints a text
    /// frame's background over <c>lcl_CalcBorderRect</c>'s rectangle —
    /// <c>getFramePrintArea() + getFrameArea().Pos()</c>, <c>sw/source/core/layout/paintfrm.cxx:1265</c> —
    /// so the fill spans the indents rather than the whole column, and it stops at the first and last
    /// line rather than covering the space before and after the paragraph. Measured on a shaded
    /// paragraph indented 720 twips with 400 twips of spacing either side, LibreOffice fills exactly the
    /// indented line stack and leaves both spacings white.
    /// </para>
    /// <para>
    /// The one exception is the join, and it is why a run of same-coloured headings reads as one bar
    /// rather than as stripes: when the previous frame carries the same background, the rectangle's top
    /// is pulled up to the frame's top — <c>aRect.Top( getFrameArea().Top() )</c>,
    /// <c>paintfrm.cxx:7033</c> — which is where the paragraph before it stopped filling. So the space
    /// between two identically shaded paragraphs is filled and the space between two differently shaded
    /// ones is not, both of which are measurable and neither of which follows from the other.
    /// </para>
    /// </remarks>
    private static void DrawParagraphShading(
        DocRect area,
        IReadOnlyList<PlacedLine> lines,
        IReadOnlyList<PageBlock> blocks,
        IDrawingSink sink)
    {
        // The run being accumulated: one or more consecutive paragraphs that agree about their colour and
        // their edges, and so become a single rectangle. Emitting one per paragraph would be the same
        // coverage and not the same picture — two abutting fills leave a blended seam a rasteriser cannot
        // avoid, which reads as a pale rule across a shaded heading.
        Colour? colour = null;
        DocRect run = default;
        int last = -2;

        int index = -1;
        Length top = Length.Zero;
        Length bottom = Length.Zero;

        void Emit()
        {
            if (colour is { } fill) Fill(run, fill, sink);
            colour = null;
        }

        void Flush()
        {
            if (index < 0 || blocks[index] is not PageParagraph paragraph) return;
            if (paragraph.Shading is not { } fill)
            {
                Emit();
                last = -2;
                return;
            }

            DocRect next = ShadeArea(area, paragraph, top, bottom);

            // Joined when the paragraph immediately before was filled the same way: the rectangle grows
            // downwards over whatever sat between the two, which is the space one's spacing-after and the
            // other's spacing-before left blank.
            if (colour == fill && last == index - 1 && next.X == run.X && next.Width == run.Width)
            {
                run = new DocRect(run.X, run.Y, run.Width, next.Bottom - run.Y);
            }
            else
            {
                Emit();
                colour = fill;
                run = next;
            }

            last = index;
        }

        foreach (PlacedLine line in lines)
        {
            OfficeOperation.Poll();
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= blocks.Count) continue;

            if (line.ParagraphIndex != index)
            {
                Flush();
                index = line.ParagraphIndex;
                top = line.Top;
            }

            bottom = line.Top + line.Box.Height;
        }

        Flush();
        Emit();
    }

    /// <summary>
    /// Draws the rules round each bordered paragraph, after its shading and before its text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Per paragraph rather than per run of joined paragraphs, because the reader has already resolved the
    /// join — the lower of two identically bordered paragraphs carries no top border and the upper none at
    /// the bottom, so their left and right rules meet as two abutting segments and the box reads as one.
    /// That is what LibreOffice's own PDF holds: a two-paragraph box emits its verticals as
    /// <c>691.45-707.30</c> and <c>675.55-691.45</c> rather than as one stroke.
    /// </para>
    /// <para>
    /// The geometry is measured rather than assumed, and two parts of it are counter-intuitive. The rule
    /// sits at the <em>outer</em> edge with <c>w:space</c> between it and the text, so a rule's near edge
    /// is a distance from the text and its far edge is the frame; and the vertical rules sit
    /// <em>outside</em> the print area, which means a bordered paragraph can draw into the page margin and
    /// its lines break exactly where an unbordered one's would.
    /// </para>
    /// </remarks>
    private static void DrawParagraphBorders(
        DocRect area,
        IReadOnlyList<PlacedLine> lines,
        IReadOnlyList<PageBlock> blocks,
        IDrawingSink sink)
    {
        int index = -1;
        Length top = Length.Zero;
        Length bottom = Length.Zero;

        // Where the box the paragraph above opened stopped, and which paragraph that was. A joined
        // paragraph starts its own box there rather than at its own text, so the side rules run across
        // whatever `w:spacing` stands between the two — which is what LibreOffice draws and what a box
        // per paragraph gets visibly wrong on a spaced list.
        int joined = -2;
        Length joinedAt = Length.Zero;

        void Flush()
        {
            if (index < 0 || blocks[index] is not PageParagraph paragraph) return;
            if (paragraph.Borders is not { Draws: true } borders) return;

            DocRect text = ShadeArea(area, paragraph, top, bottom);

            Length left = text.X - (borders.Left?.Allowance ?? Length.Zero);
            Length right = text.Right + (borders.Right?.Allowance ?? Length.Zero);
            Length above = borders.JoinsAbove && joined == index - 1 && joinedAt < text.Y
                ? joinedAt
                : text.Y - borders.Above;
            Length below = text.Bottom + borders.Below;

            joined = index;
            joinedAt = below;

            if (borders.Top is { Draws: true } rule)
            {
                DrawRule(
                    new DocRect(left, text.Y - rule.Space - rule.Width, right - left, rule.Width),
                    rule, across: true, outerLast: false, sink);
            }

            if (borders.Bottom is { Draws: true } under)
            {
                DrawRule(
                    new DocRect(left, text.Bottom + under.Space, right - left, under.Width),
                    under, across: true, outerLast: true, sink);
            }

            if (borders.Left is { Draws: true } start)
            {
                DrawRule(
                    new DocRect(left, above, start.Width, below - above),
                    start, across: false, outerLast: false, sink);
            }

            if (borders.Right is { Draws: true } end)
            {
                DrawRule(
                    new DocRect(right - end.Width, above, end.Width, below - above),
                    end, across: false, outerLast: true, sink);
            }
        }

        foreach (PlacedLine line in lines)
        {
            OfficeOperation.Poll();
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= blocks.Count) continue;

            if (line.ParagraphIndex != index)
            {
                Flush();
                index = line.ParagraphIndex;
                top = line.Top;
            }

            bottom = line.Top + line.Box.Height;
        }

        Flush();
    }

    /// <summary>
    /// Fills one paragraph border's band with the strokes its line style actually draws.
    /// </summary>
    /// <param name="band">The whole band the border covers, the second rule and the gap included.</param>
    /// <param name="rule">The border.</param>
    /// <param name="across">True for a top or bottom rule, false for a side one.</param>
    /// <param name="outerLast">
    /// True when the band's <em>far</em> edge is the one away from the text — a bottom or a right rule.
    /// It decides which end of the band the outer stroke sits at, which is visible the moment the two
    /// strokes differ in width: an <c>outset</c> puts its thin rule outside on all four sides.
    /// </param>
    /// <param name="sink">Where to draw.</param>
    /// <remarks>
    /// The strokes are <em>filled</em> rather than stroked, as the single rule always was, and a dashed
    /// one is filled a dash at a time. A stroked path would put the pen's centre on the band's edge and
    /// need every rectangle here recomputed round it; the dash lengths are the pen's own multiples
    /// either way. See <see cref="BorderRules"/>.
    /// </remarks>
    private static void DrawRule(
        DocRect band, ParagraphBorder rule, bool across, bool outerLast, IDrawingSink sink)
    {
        BorderBands bands = rule.Bands;
        IReadOnlyList<Length>? dashes = BorderRules.Dashes(rule.Line);
        Length span = across ? band.Height : band.Width;

        Piece(outerLast ? span - bands.Outer : Length.Zero, bands.Outer);

        if (bands.HasTwoRules)
        {
            Piece(outerLast ? Length.Zero : bands.Outer + bands.Gap, bands.Inner);
        }

        void Piece(Length at, Length thick)
        {
            if (thick <= Length.Zero) return;

            DocRect stroke = across
                ? new DocRect(band.X, band.Y + at, band.Width, thick)
                : new DocRect(band.X + at, band.Y, thick, band.Height);

            if (dashes is null || dashes.Count == 0)
            {
                Fill(stroke, rule.Colour, sink);
                return;
            }

            Length along = across ? stroke.Width : stroke.Height;
            Length drawn = Length.Zero;

            for (int i = 0; drawn < along; i += 2)
            {
                Length ink = dashes[i % dashes.Count];
                Length gap = dashes[(i + 1) % dashes.Count];
                if (ink + gap <= Length.Zero) return;

                Length here = Length.Min(ink, along - drawn);
                if (here > Length.Zero)
                {
                    Fill(
                        across
                            ? new DocRect(stroke.X + drawn, stroke.Y, here, stroke.Height)
                            : new DocRect(stroke.X, stroke.Y + drawn, stroke.Width, here),
                        rule.Colour, sink);
                }

                drawn += ink + gap;
            }
        }
    }

    /// <summary>The rectangle a paragraph's shading fills, in the coordinates of the area holding it.</summary>
    /// <remarks>
    /// The indents narrow it from both sides, and which side each one is on depends on the paragraph's
    /// direction — a right-to-left paragraph's start indent is its right edge. The first-line indent
    /// deliberately does not narrow it: it moves one line's text, not the paragraph's print area.
    /// </remarks>
    private static DocRect ShadeArea(
        DocRect area, PageParagraph paragraph, Length top, Length bottom)
    {
        ParagraphFormat format = paragraph.DeclaredFormat;
        Length before = format.IsRightToLeft ? format.EndIndent : format.StartIndent;
        Length after = format.IsRightToLeft ? format.StartIndent : format.EndIndent;

        Length left = area.X + Length.Max(before, Length.Zero);
        Length right = area.Right - Length.Max(after, Length.Zero);

        return new DocRect(left, area.Y + top, right - left, bottom - top);
    }

    /// <summary>
    /// The glyph runs one line draws: one per formatting change, and one per tab.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A paragraph with uniform formatting and no tabs draws one run per line, which is the common case and
    /// the cheap one. Formatting splits it further — a bold phrase crossing a line break becomes two runs,
    /// one on each line, because a glyph run is one font at one size at one position and a line break is a
    /// position — and so do tabs, because the text after a tab starts at a stop rather than where the text
    /// before it ended.
    /// </para>
    /// <para>
    /// Within a stretch the pen advances across the runs rather than restarting per run, so the second run
    /// of a stretch starts where the first ended. Measuring each from zero would stack them all at the
    /// margin.
    /// </para>
    /// </remarks>
    public static List<(GlyphRun Run, Colour Colour)> RunsFor(
        LaidOutPage page, PlacedLine line, PageParagraph paragraph)
    {
        ArgumentNullException.ThrowIfNull(page);
        return RunsIn(page.ColumnArea(line), line, paragraph);
    }

    /// <summary>The glyph runs one line draws, relative to whichever area it belongs to.</summary>
    /// <param name="area">The rectangle the line's coordinates are relative to.</param>
    /// <param name="line">The line.</param>
    /// <param name="paragraph">Its paragraph.</param>
    /// <param name="highlights">
    /// Collects the coloured band behind each highlighted run, or null when the caller wants only the
    /// glyphs. Out of this walk rather than a second one, because the band's left edge and width are the
    /// pen positions the tab stops and the justification decided here — recomputing them elsewhere would
    /// be a second place for that arithmetic to be got right.
    /// </param>
    /// <param name="rules">
    /// Collects the underline and strikethrough rectangles each decorated run asks for, or null when the
    /// caller wants only the glyphs. Out of this walk for the same reason the bands are: a rule spans the
    /// advance the pen just measured, and its offset and thickness come from the face this walk resolved.
    /// </param>
    /// <param name="background">
    /// What is painted behind this line — the paragraph's own shade, else its cell's, else its
    /// frame's, else the page — for a run whose colour is <em>automatic</em>. Transparent means the
    /// page, which is never dark, so the default preserves what a caller with no background to
    /// offer used to get. See <see cref="Automatic"/>.
    /// </param>
    /// <param name="links">
    /// Receives the clickable area of each run that is a link, with where it leads, when not null. A
    /// link paints nothing, so it is not among the glyph runs returned.
    /// </param>
    public static List<(GlyphRun Run, Colour Colour)> RunsIn(
        DocRect area,
        PlacedLine line,
        PageParagraph paragraph,
        List<(DocRect Area, Colour Colour)>? highlights = null,
        List<(DocRect Area, Colour Colour)>? rules = null,
        Colour background = default,
        List<(DocRect Area, string Link)>? links = null)
    {
        ArgumentNullException.ThrowIfNull(paragraph);

        List<(GlyphRun, Colour)> runs = [];

        int start = line.Box.Line.Start;
        int end = Math.Min(line.Box.Line.VisibleEnd, paragraph.Text.Length);

        Length lineLeft = area.X + line.Box.Left;
        Length baseline = area.Y + line.Baseline;

        // Before the text and before the empty-line exit, because an item with no words still has a
        // number: an empty list paragraph draws its label and nothing else, which is what LibreOffice
        // does and what a list being typed into looks like.
        if (line.StartsParagraph && paragraph.Label is { Text.Length: > 0 } label)
        {
            ShapedText shapedLabel = TextShaper.Default.Shape(label.Face, label.Text, label.Shaping);
            if (shapedLabel.Glyphs.Count > 0)
            {
                runs.Add((
                    Build(
                        shapedLabel,
                        label.Text,
                        label.EmSize,
                        label.Font ?? Reference(paragraph, label.Face),
                        new DocPoint(lineLeft - paragraph.LabelAdvance, baseline),
                        Length.Zero),
                    label.Colour.A == 0 ? Automatic(background) : label.Colour));
            }
        }

        if (end <= start) return runs;

        List<TabbedSegment> stretches =
            Stretches(paragraph, start, end, line.StartsParagraph, area.Width);

        for (int index = 0; index < stretches.Count; index++)
        {
            TabbedSegment segment = stretches[index];

            // Before the emptiness test: a tab followed by nothing still draws its leader, which is what
            // a table-of-contents line whose page number sits on the next line looks like.
            if (Leader(paragraph, segment, lineLeft, baseline, background) is { } filled)
            {
                runs.Add(filled);
            }

            // Before the emptiness test for the same reason the leader is: a tab carries the decoration
            // of the font at the tab whether or not anything follows it. See `TabRule`.
            if (rules is not null) TabRule(paragraph, segment, lineLeft, baseline, rules, background);

            if (segment.IsEmpty) continue;

            // The justification belongs to the last stretch alone. A tab is a fixed portion whose glue is
            // nought, so the stretch it closes is stretched by nothing and only the last one reaches the
            // right margin's glue — see `ParagraphLayouter.Justification`, which counts the same blanks.
            Length spaceAdd = index == stretches.Count - 1 ? line.Box.SpaceAdd : Length.Zero;

            Length pen = lineLeft + segment.Left;

            // The as-character objects on this stretch, in position order, consumed as the pen reaches
            // them. An object contributes no glyphs — the frame is drawn separately, by `DrawFrame` at
            // the rectangle `FrameLayout` hung it at — so all the text pass owes it is the room it takes.
            int nextObject = 0;
            List<InlineObject> onStretch = paragraph.HasInlineObjects
                ? [.. paragraph.InlineObjects
                    .Where(one => one.Offset >= segment.Start && one.Offset < segment.End)
                    .OrderBy(one => one.Offset)]
                : [];

            foreach (PageRun run in InVisualOrder(paragraph, segment.Start, segment.End))
            {
                while (nextObject < onStretch.Count && onStretch[nextObject].Offset <= run.Start)
                {
                    pen += onStretch[nextObject].Width;
                    nextObject++;
                }

                // The script-change gap, charged before the run that follows it and from the size of
                // the run that ends at it — the same rule and the same one place `MeasuredParagraph`
                // added it to the prefix table, so the pen and the line break agree.
                if (paragraph.HasScriptSpace && run.Start > segment.Start
                    && ScriptSpacing.Opens(paragraph.Text, run.Start))
                {
                    pen += ScriptSpacing.GapFor(SizeEndingAt(paragraph, run.Start));
                }

                string text = paragraph.Text[run.Start..run.End];
                ShapedText shaped = TextShaper.Default.Shape(run.Face, text, run.EffectiveShaping);
                if (shaped.Glyphs.Count == 0) continue;

                // A raised run draws above the baseline and advances along it unchanged, which is why the
                // rise moves the origin rather than the glyphs: the pen below has to carry on from where an
                // unraised run would have left it.
                GlyphRun glyphRun = Build(
                    shaped,
                    text,
                    run.EmSize,
                    run.Font ?? Reference(paragraph, run.Face),
                    new DocPoint(pen, baseline - run.Rise),
                    spaceAdd,
                    run.Tracking,
                    run.WidthScale);

                runs.Add((glyphRun, run.ColourOn(background)));

                // The pen carries the justification with it, or the second run on a stretched line would
                // start where the first would have ended unjustified and overlap the words before it.
                Length extent = Extent(glyphRun);

                if (highlights is not null && run.IsHighlighted)
                {
                    highlights.Add((Band(paragraph, run, pen, extent, baseline), run.Highlight));
                }

                if (rules is not null && run.IsDecorated)
                {
                    Rules(run, pen, extent, baseline, rules, background);
                }

                // The clickable area of a link is the band the run's own face and size give it, the same
                // rectangle a highlight would take, so the pointer changes where the text is.
                if (links is not null && run.Link is { Length: > 0 } link
                    && Band(paragraph, run, pen, extent, baseline) is { IsEmpty: false } clickable)
                {
                    links.Add((clickable, link));
                }

                pen += extent;
            }
        }

        return runs;
    }

    /// <summary>The em size of whatever run ends at a position.</summary>
    /// <remarks>
    /// `MeasuredParagraph` asks the same question of its own run list, and both have to answer the
    /// same way or the pen pays a different gap from the one the line was broken on. A uniform
    /// paragraph has no runs and answers with the paragraph's own size, which is what it is set in.
    /// </remarks>
    private static Length SizeEndingAt(PageParagraph paragraph, int position)
    {
        foreach (PageRun run in paragraph.Runs)
        {
            if (position - 1 >= run.Start && position - 1 < run.End) return run.EmSize;
        }

        return paragraph.EmSize;
    }

    /// <summary>
    /// The coloured band behind one highlighted run.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer paints a character background over the <em>portion's</em> box rather than the line's:
    /// <c>SwTextPaintInfo::CalcRect</c> (<c>sw/source/core/text/inftxt.cxx</c>) takes the rectangle from
    /// the baseline less the portion's ascent, with the portion's own height. So the band follows the
    /// run's face and size, not the tallest thing on the line — which is what stops a highlighted word in
    /// a footnote-sized face from being given a band as tall as the heading beside it, and what stops a
    /// double-spaced paragraph from being highlighted across the whole of its leading.
    /// </para>
    /// <para>
    /// The metrics are resolved through the same <see cref="LineSpacing"/> call and the same device grid
    /// the measurement used, so the band's height is the height layout gave the run rather than a second
    /// opinion about it.
    /// </para>
    /// </remarks>
    private static DocRect Band(
        PageParagraph paragraph, PageRun run, Length pen, Length extent, Length baseline)
    {
        LineMetrics metrics = LineSpacing.Resolve(
            run.Face, paragraph.Metrics, WriterLineBox.LeadingAboveText);
        Length size = run.MetricEmSize > Length.Zero ? run.MetricEmSize : run.EmSize;

        Length ascent = metrics.ScaledAscent(size);
        Length height = metrics.ScaledLineHeight(size);
        if (height <= Length.Zero) return default;

        // The rise moves the band with the text: a highlighted superscript is banded where it is drawn.
        return new DocRect(pen, baseline - run.Rise - ascent, extent, height);
    }

    /// <summary>
    /// The rules drawn under and through one decorated run.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A decoration is not shaped. It is a rectangle the output device fills across the run's advance,
    /// which is why it can be added here without any measurement changing: the glyphs had that advance
    /// already. The offset and the thickness are the face's own <c>post</c> and <c>OS/2</c> numbers
    /// through <see cref="LineSpacing.ResolveDecorations(OpenTypeFace, LineMetrics)"/> — the same call
    /// the slide and the cell layers make, including its refusal to believe the three Liberation faces'
    /// <c>post</c> tables, which is LibreOffice's own shipped <c>FontsDontUseUnderlineMetrics</c> and
    /// matters here more than anywhere: those three are what a corpus set in Arial, Times New Roman and
    /// Courier New actually resolves to.
    /// </para>
    /// <para>
    /// No <see cref="MetricGrid"/> is passed, unlike <see cref="Band"/>, and deliberately: a grid
    /// quantises the <em>scaling</em> of a metric onto device pixels and this resolution reads design
    /// units, so a printer-metrics document would get an identical answer from a grid it had to be
    /// threaded here to supply.
    /// </para>
    /// <para>
    /// Per run rather than per line, which is what makes an underlined phrase inside a plain sentence
    /// underline only itself. Two adjacent underlined runs abut, since each spans exactly the advance
    /// the pen charged it — there is no gap to bridge and no overlap to double-darken.
    /// </para>
    /// </remarks>
    private static void Rules(
        PageRun run,
        Length pen,
        Length extent,
        Length baseline,
        List<(DocRect Area, Colour Colour)> rules,
        Colour background = default)
    {
        if (run.EmSize <= Length.Zero || extent <= Length.Zero) return;

        LineMetrics line = LineSpacing.Resolve(run.Face);

        // A rule's weight AND its offset are whole numbers of the PDF writer's own 720 dpi pixels
        // rounded to whole units of the map mode the page is painted in, which on a Writer page is
        // the TWIP. See `LineSpacing.ResolveRuleWidths` and `MetricGrid.WriterTextLine`.
        LineSpacing.RuleWidths widths =
            LineSpacing.ResolveRuleWidths(run.Face, line, run.EmSize, MetricGrid.WriterTextLine);

        // The rise carries the rules with the text, exactly as it carries the band: a struck-through
        // superscript is struck where it is drawn rather than where it would have sat unraised.
        Length baselineOfRun = baseline - run.Rise;

        if (run.Underline == TextUnderline.SingleLine)
        {
            Length thickness = widths.Underline;
            if (thickness > Length.Zero)
            {
                rules.Add((
                    new DocRect(
                        pen, baselineOfRun + widths.UnderlineOffset, extent, thickness),
                    run.ColourOn(background)));
            }
        }
        else if (run.Underline == TextUnderline.BoldLine)
        {
            Length thickness = widths.BoldUnderline;
            if (thickness > Length.Zero)
            {
                rules.Add((
                    new DocRect(
                        pen, baselineOfRun + widths.BoldUnderlineOffset, extent, thickness),
                    run.ColourOn(background)));
            }
        }
        else if (run.Underline == TextUnderline.DoubleLine)
        {
            // Both the thickness AND the two offsets come off the device here, where a single
            // underline still takes its offset from the design units: a double underline's lines
            // straddle where a single one would sit, their separation is floored on the device
            // rather than scaled with the em, and the second carries the writer's extra thickness.
            // See `LineSpacing.RuleWidths.DoubleUnderlineSecond`.
            Length thickness = widths.DoubleUnderline;
            if (thickness > Length.Zero)
            {
                rules.Add((
                    new DocRect(
                        pen, baselineOfRun + widths.DoubleUnderlineFirst, extent, thickness),
                    run.ColourOn(background)));
                rules.Add((
                    new DocRect(
                        pen, baselineOfRun + widths.DoubleUnderlineSecond, extent, thickness),
                    run.ColourOn(background)));
            }
        }

        if (run.IsStruckThrough)
        {
            Length thickness = widths.Strikeout;
            if (thickness > Length.Zero)
            {
                rules.Add((
                    new DocRect(
                        pen, baselineOfRun + widths.StrikeoutOffset, extent, thickness),
                    run.ColourOn(background)));
            }
        }
    }

    /// <summary>
    /// The rule a decorated tab carries across the blank it advanced over, if it carries one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>Writer underlines the line, not the runs.</b> A tab is painted as a run of literal blanks in
    /// the font at the tab itself, so whatever line that font carries is drawn across it —
    /// <c>SwTabPortion::Paint</c> (<c>sw/source/core/text/txttab.cxx</c>:626-641), under the comment
    /// <c>// Tabs should be underlined at once</c>. The switch is not "is it underlined":
    /// <c>SwFntObj</c>'s constructor sets <c>m_bPaintBlank</c> from
    /// <c>(underline || overline || strikeout) &amp;&amp; !IsWordLineMode()</c>
    /// (<c>sw/source/core/txtnode/fntcache.cxx</c>:106-109), so a strikethrough spans a tab exactly as
    /// an underline does. This model carries no overline, so two of the three arms are drawn here and
    /// the third has nothing to draw.
    /// </para>
    /// <para>
    /// <b>The rule spans the tab's whole width, and that is not what counting the blanks would give.</b>
    /// The count is <c>Width() / GetTextSize(' ')</c>, an integer division of two whole-twip lengths,
    /// and <em>n</em> blanks at their own advances would stop up to one space short of the stop. They do
    /// not, because the call passes <c>bKern = true</c>: <c>SwTextPaintInfo::DrawText_</c> turns that
    /// into <c>aDrawInf.SetKern(rPor.Width())</c> and routes it to <c>SwSubFont::DrawStretchText_</c>
    /// (<c>inftxt.cxx</c>:798-808, <c>swfont.cxx</c>:1289-1345), which calls
    /// <c>OutputDevice::DrawStretchText(aPos, rInf.GetWidth(), …)</c> — so the blanks are set onto the
    /// portion's own width and the rule reaches the stop. It is the same <c>bKern</c>, and the same
    /// compression, that <see cref="Leader"/> already models for a dot leader.
    /// </para>
    /// <para>
    /// <b>This is the arm a fix taken from one witness gets wrong, so it is measured against its
    /// alternative rather than merely agreed with.</b> On five Liberation Mono tabs at 10 pt whose
    /// residue modulo the 120-twip blank is 80, 20, 100, 100 and 100 twips — where the two readings are
    /// <b>1.00 to 5.00 pt apart</b> — 26.2.4.2 draws 16.00, 19.00, 23.00, 29.00 and 35.00 pt of cover,
    /// which is the tab's full width at <b>5 of 5</b> and the truncated reading at 0 of 5.
    /// </para>
    /// <para>
    /// <b>So the count decides one thing only: whether anything is drawn at all.</b> A tab narrower than
    /// one blank gives <c>nChar == 0</c>, an empty string, and <c>DrawText_</c> returns on
    /// <c>!nLength</c> — no rule. Measured at 26.2.4.2 rather than inferred, and the cliff falls exactly
    /// where a whole-twip blank puts it: in Liberation Mono at 10 pt, whose blank is 120 twips to the
    /// twip, tabs of 30, 90 and 119 twips carry no rule and 121, 180 and 361 carry one. And the width is
    /// <em>truncated</em> to the twip rather than rounded, which Liberation Sans at 10 pt separates —
    /// its blank is 55.566 twips, and a tab of exactly 55 twips draws a rule, which rounding to 56 would
    /// refuse. <c>probes/wordsdec-r126</c>.
    /// </para>
    /// <para>
    /// <b>What is deliberately not modelled is the switch.</b> <c>IsWordLineMode()</c> — Word's
    /// <c>w:u w:val="words"</c>, RTF's <c>\ulw</c>, <c>sprmCKul</c> operand 2 and ODF's
    /// <c>style:text-{underline,overline,line-through}-mode="skip-white-space"</c> — turns the whole of
    /// this off, and no corpus document states it in any of the four spellings. See the round's
    /// write-up for the census and the twelve authored rows that measure the cost of declining it.
    /// </para>
    /// </remarks>
    private static void TabRule(
        PageParagraph paragraph,
        TabbedSegment segment,
        Length lineLeft,
        Length baseline,
        List<(DocRect Area, Colour Colour)> rules,
        Colour background = default)
    {
        if (segment.GapWidth <= Length.Zero) return;

        // The font in effect AT the tab, which is what `rInf.GetFont()` means there — the same
        // position `Leader` reads, and for the same reason.
        PageRun at = RunAt(paragraph, segment.Start - 1);
        if (!at.IsDecorated) return;

        Length blank = TextShaper.Default
            .Shape(at.Face, " ", at.EffectiveShaping)
            .Width(at.EmSize);
        if (blank <= Length.Zero) return;

        Length unit = Length.FromTwips(blank.Emu / Length.EmuPerTwip);
        if (unit <= Length.Zero || segment.GapWidth.Emu < unit.Emu) return;

        Rules(at, lineLeft + segment.GapLeft, segment.GapWidth, baseline, rules, background);
    }

    /// <summary>
    /// How many fill characters one tab may draw, however small the face and however wide the blank.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input, in the same spirit as <see cref="TabRuler.MaxSegments"/>. A page-wide
    /// blank filled at a plausible size holds a few hundred dots; a document declaring a one-EMU face
    /// would ask for billions, and each one costs a glyph.
    /// </remarks>
    private const int MaxLeaderCharacters = 4096;

    /// <summary>
    /// The run of fill characters a tab draws across the blank it advanced over, if it has one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The dot leader of a table of contents, and the port of <c>SwTabPortion::Paint</c>
    /// (<c>sw/source/core/text/txttab.cxx:648-659</c>): the blank's width divided by one fill character's,
    /// truncated, so the fill never runs past the stop it stops at. Underscore takes one extra, because
    /// its glyph spans its whole advance and a truncated run of them shows the rounding as visible gaps —
    /// Writer makes the same exception and for the same reason.
    /// </para>
    /// <para>
    /// Drawn in the face in effect <em>at the tab</em> rather than the one after it, which is what
    /// <c>rInf.GetFont()</c> means at that point in Writer. A contents line whose title is bold and whose
    /// page number is not would otherwise draw bold dots between them.
    /// </para>
    /// <para>
    /// This paints inside space the tab had already reserved, so it moves no line break and no page
    /// break: a paragraph measures exactly as it did before the leader existed.
    /// </para>
    /// <para>
    /// <b>The fill character's width is a whole twip, and that is not a rounding nicety — it is worth
    /// one or two dots on every contents line.</b> <c>nCharWidth</c> is a <c>SwTwips</c> and so is the
    /// portion's <c>Width()</c>, so Writer's count is an integer division of two integer-twip lengths.
    /// Carlito's full stop at 12 pt is 60.586 twips; Writer measures 60 and fits 134 dots into the
    /// 8051-twip blank of <c>system_design__technical_architecture_template.docx</c>, where dividing by
    /// the exact advance fits 132. The two missing dots leave a 2.51 pt hole in front of the page
    /// number, and poppler — which is what the corpus gate scores — reads a hole that wide as a word
    /// break, so <c>Revision History………4</c> extracts as three tokens against the reference's two. A
    /// leader that stops a character short of its stop and a spurious extracted word are therefore the
    /// same defect measured two ways — but <em>only</em> this one. A leader can also stop short because
    /// the stop itself is in the wrong place, which is the <c>TabOverSpacing</c> clamp recorded in
    /// <c>TODO.batches.md</c> and is not this: measured over the 28 corpus documents that draw a dot
    /// leader, the page number's right edge agrees with the reference to 0.10 pt on 25 of them and is
    /// 18 to 28 pt short on the other three.
    /// </para>
    /// <para>
    /// <b>The count is taken at the whole twip and the fill is then drawn back down onto the blank</b>,
    /// which is the <c>bKern</c> argument <c>SwTabPortion::Paint</c> passes to
    /// <c>SwTextPaintInfo::DrawText</c>: it selects <c>SwFont::DrawStretchText_</c>, so the run is laid
    /// out against the portion's width rather than its own. Counting at 60 twips and setting at 60.586
    /// would put 134 dots into 132 dots' worth of blank, so the surplus is taken back off every
    /// character and the last one lands at the stop.
    /// </para>
    /// <para>
    /// <b>It compresses and it does not expand</b>, and that asymmetry is measured rather than assumed
    /// — VCL's <c>GenericSalLayout::Justify</c> spreads a widening across the blanks in the string, and
    /// a run of dots has none. Both halves are visible in the reference's own PDFs. In
    /// <c>system_design__technical_architecture_template.docx</c>, where Carlito's 60.586 twips
    /// truncates, the per-dot advance comes out 3.0002, 3.0121 or 3.0240 pt line by line — never the
    /// font's own 3.0293 — and each run ends flush against its page number. In
    /// <c>Agile_Arc_SysDes.docx</c>, where Liberation Serif's full stop is 55 twips exactly and nothing
    /// truncates, the dots are written as one unadjusted show at their natural width and stop 2.05 to
    /// 2.35 pt short of the number, which is just the division's remainder left where it fell.
    /// </para>
    /// <para>
    /// The extracted word count turns on that trailing gap rather than on the dot count, because
    /// poppler starts a new word at a gap of about a tenth of the em: a leader ending 1.5 pt short of a
    /// 12 pt page number extracts as a separate token and one ending 0.3 pt short does not. Correcting
    /// the count alone moved 24 of one document's 33 contents lines and left its token count exactly
    /// where it was; the two together took it to the reference's, word for word.
    /// </para>
    /// </remarks>
    private static (GlyphRun Run, Colour Colour)? Leader(
        PageParagraph paragraph,
        TabbedSegment segment,
        Length lineLeft,
        Length baseline,
        Colour background = default)
    {
        if (!segment.HasLeader) return null;

        PageRun at = RunAt(paragraph, segment.Start - 1);

        Length exact = TextShaper.Default
            .Shape(at.Face, segment.Leader.ToString(), at.EffectiveShaping)
            .Width(at.EmSize);
        if (exact <= Length.Zero) return null;

        // Truncated, not rounded: 60.586 twips has to become 60 for the reference's 134 dots, and
        // rounding it to 61 fits 131.
        Length unit = Length.FromTwips(exact.Emu / Length.EmuPerTwip);
        if (unit <= Length.Zero) return null;

        long count = segment.GapWidth.Emu / unit.Emu;
        if (segment.Leader == '_') count++;
        if (count <= 0) return null;

        // Compressed to fit, never expanded to fill, and the clamp is not symmetry for its own sake —
        // it is what the reference does, measured. A count taken at the truncated width can ask for
        // more room than the blank has, and then the run is squeezed onto it; when it asks for less,
        // the remainder is simply left. `MaxLeaderCharacters` is excluded because a clamped count is a
        // guess at what a hostile document meant, and dividing the blank by it would set one character
        // per page rather than refuse.
        Length pitch = count <= MaxLeaderCharacters
            ? Length.FromEmu(Math.Min(exact.Emu, segment.GapWidth.Emu / count))
            : exact;

        string fill = new(segment.Leader, (int)Math.Min(count, MaxLeaderCharacters));
        ShapedText shaped = TextShaper.Default.Shape(at.Face, fill, at.EffectiveShaping);
        if (shaped.Glyphs.Count == 0) return null;

        return (
            Build(
                shaped,
                fill,
                at.EmSize,
                at.Font ?? Reference(paragraph, at.Face),
                new DocPoint(lineLeft + segment.GapLeft, baseline - at.Rise),
                Length.Zero,
                pitch - exact),
            at.ColourOn(background));
    }

    /// <summary>
    /// What an <em>automatic</em> font colour resolves to over a given background.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwDrawTextInfo::ApplyAutoColor</c> (<c>sw/source/core/txtnode/fntcache.cxx</c>:2369) asks
    /// the frame chain for a background brush and answers <c>COL_WHITE</c> when the brush is dark
    /// and <c>COL_BLACK</c> otherwise. With no brush at all it falls back to the application's
    /// document colour, which is white — so a transparent background here means black, which is
    /// what every caller got before a background existed to pass.
    /// </para>
    /// <para>
    /// <strong>A character highlight is not a brush.</strong> Measured, and in both directions: a
    /// yellow <c>w:highlight</c> on a run in a black cell is drawn <em>white</em>, and a
    /// <c>darkBlue</c> highlight in a white cell is drawn <em>black</em>
    /// (<c>probes/words-r59/autocolour.py</c>, cases <c>B/cell-dark-run-highlight-light</c> and
    /// <c>B/cell-light-run-highlight-dark</c>). The brush the function asks for is the font's
    /// <em>back colour</em>, which is <c>RES_CHRATR_BACKGROUND</c> — character shading — and
    /// <c>w:highlight</c> is <c>RES_CHRATR_HIGHLIGHT</c>, a different item entirely. Reading the
    /// highlight as the background is the obvious wrong answer and both those cases refuse it.
    /// </para>
    /// </remarks>
    /// <param name="background">The brush behind the run, or transparent for none.</param>
    private static Colour Automatic(Colour background) => AutomaticColour.Over(background);

    /// <summary>
    /// The formatting run covering a character, or the paragraph's own formatting where none does.
    /// </summary>
    /// <remarks>
    /// Asked for the tab character itself, which sits at the end of the stretch before the one the stop
    /// placed — so a position before the paragraph's first character, or past its last, falls back rather
    /// than failing.
    /// </remarks>
    private static PageRun RunAt(PageParagraph paragraph, int at)
    {
        if (paragraph.HasRuns)
        {
            foreach (PageRun run in paragraph.Runs)
            {
                if (at >= run.Start && at < run.End) return run;
            }
        }

        return new PageRun(
            Math.Max(at, 0),
            0,
            paragraph.Face,
            paragraph.EmSize,
            paragraph.Font,
            paragraph.Colour,
            paragraph.Shaping);
    }

    /// <summary>
    /// The stretches a line is divided into by its tabs, each placed at its stop.
    /// </summary>
    /// <remarks>
    /// One stretch covering the whole line when there is no tab, which is nearly always — and it goes
    /// through the same code path so that a tabbed line and an untabbed one cannot drift apart. The
    /// measurement handed to the ruler is the same one the layout used, so the stops land in the same
    /// places here as they did when the line's width was decided.
    /// </remarks>
    /// <param name="paragraph">The paragraph the line belongs to.</param>
    /// <param name="start">Where the line's text starts.</param>
    /// <param name="end">Where its visible text ends.</param>
    /// <param name="isFirstLine">True for the paragraph's own first line.</param>
    /// <param name="areaWidth">
    /// How wide the rectangle holding the line is — the column for a body paragraph, the cell's inner
    /// width for one in a table — or null when the caller does not have it. It is the frame edge a right
    /// stop declared past the margin is pulled back to, and the paragraph's right indent is deliberately
    /// not taken out of it: Writer clamps at the frame's edge, so a stop inside the indent stands. The
    /// layout resolved it the same way, so omitting it here would draw a leader running off the page that
    /// the line was never measured to have. See <c>TabRuler.Place</c>.
    /// </param>
    private static List<TabbedSegment> Stretches(
        PageParagraph paragraph, int start, int end, bool isFirstLine, Length? areaWidth = null)
    {
        if (!TabRuler.HasTab(paragraph.Text, start, end))
        {
            return [new TabbedSegment(start, end, Length.Zero, Length.Zero)];
        }

        return TabRuler.Segments(
            paragraph.Text,
            start,
            end,
            paragraph.Format,
            (from, to) => WidthBetween(paragraph, from, to),
            isFirstLine,
            paragraph.Format.ClampsTabsAtLineEdge && areaWidth is { } width
                ? width - paragraph.Format.TabOrigin
                : null);
    }

    /// <summary>
    /// The formatting runs covering a stretch, clipped to it, in order.
    /// </summary>
    /// <remarks>
    /// One synthetic run for a uniform paragraph, so the drawing loop does not need two shapes. Ordered by
    /// position rather than trusted to arrive that way: a run list out of order would draw the line's words
    /// in the wrong places, and the readers build it from four different formats.
    /// </remarks>
    private static List<PageRun> RunsIn(PageParagraph paragraph, int start, int end)
    {
        if (!paragraph.HasRuns)
        {
            return ByScriptSpace(paragraph, ByFace(
                paragraph,
                AroundObjects(
                    paragraph,
                    [
                        new PageRun(
                            start,
                            end - start,
                            paragraph.Face,
                            paragraph.EmSize,
                            paragraph.Font,
                            paragraph.Colour,
                            paragraph.Shaping,
                            Tracking: paragraph.Tracking,

                            // And the paragraph's own character width, which is tracking's twin
                            // here as everywhere else: this is the SECOND fallback a uniform
                            // paragraph goes through — `PageParagraph.Measure` builds the other —
                            // and carrying the scale in one of the two is worse than carrying it
                            // in neither, because the line would then break at the squeezed width
                            // and be drawn at the unsqueezed one.
                            WidthPerCent: paragraph.WidthPerCent,
                            Item: paragraph.Item),
                    ])));
        }

        List<PageRun> clipped = [];
        foreach (PageRun run in paragraph.Runs.OrderBy(run => run.Start))
        {
            int from = Math.Max(run.Start, start);
            int to = Math.Min(run.End, end);
            if (to <= from) continue;

            clipped.Add(run with { Start = from, Length = to - from });
        }

        return ByScriptSpace(paragraph, ByFace(paragraph, AroundObjects(paragraph, clipped)));
    }

    /// <summary>
    /// The runs cut again wherever the run's own face has no glyph, as the measurement cut them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same <see cref="FontItemiser.Split"/> <see cref="MeasuredParagraph"/> runs, and it has to be
    /// the same one for the same reason the object cut does: the line's break was decided against the
    /// fallback face's advances, so drawing the stretch in the primary face would put the glyphs at
    /// widths the layout never measured — on top of drawing the wrong glyphs.
    /// </para>
    /// <para>
    /// A paragraph whose faces cover all of its text comes back through here untouched. The itemiser
    /// returns one non-fallback piece per run in that case, which is checked for explicitly rather
    /// than rebuilt: a run split at a boundary it does not need loses its shaping context and
    /// measures very slightly wide, which is enough to move a line break.
    /// </para>
    /// <para>
    /// A fallback piece drops the run's <see cref="PageRun.Font"/>, because that reference names the
    /// face the reader resolved and this piece is drawn in a different one. The caller derives a
    /// reference from the face itself when there is none, which is what puts the right font program
    /// in the PDF.
    /// </para>
    /// </remarks>
    /// <summary>
    /// The runs cut again at every script change that opens a gap, as the measurement cut them.
    /// </summary>
    /// <remarks>
    /// The pen adds <see cref="ScriptSpacing"/>'s gap when it reaches a boundary, and it can only do
    /// that between two runs — so a run spanning one has to be cut, exactly as a run spanning an
    /// as-character object is. Ordinarily the fallback split has already cut here, because the two
    /// scripts are rarely in one face; this is what makes the pen right when they are.
    /// </remarks>
    private static List<PageRun> ByScriptSpace(PageParagraph paragraph, List<PageRun> runs)
    {
        if (!paragraph.HasScriptSpace) return runs;

        List<PageRun> split = [];

        foreach (PageRun run in runs)
        {
            int from = run.Start;

            for (int at = run.Start + 1; at < run.End; at++)
            {
                if (!ScriptSpacing.Opens(paragraph.Text, at)) continue;

                split.Add(run with { Start = from, Length = at - from });
                from = at;
            }

            split.Add(from == run.Start ? run : run with { Start = from, Length = run.End - from });
        }

        return split;
    }

    private static List<PageRun> ByFace(PageParagraph paragraph, List<PageRun> runs)
    {
        if (paragraph.Fallback is not { } fallback) return runs;

        List<PageRun> split = [];

        foreach (PageRun run in runs)
        {
            List<FaceRun> faces = FontItemiser.Split(
                paragraph.Text, run.Start, run.Length, run.Face, fallback, item: run.Item);

            if (faces.Count == 1 && !faces[0].IsFallback)
            {
                split.Add(run);
                continue;
            }

            foreach (FaceRun face in faces)
            {
                split.Add(face.IsFallback
                    ? run with
                    {
                        Start = face.Start,
                        Length = face.Length,
                        Face = face.Face,
                        Font = fallback.ReferenceFor(face.Face, AsksForItalic(run)),
                    }
                    : run with { Start = face.Start, Length = face.Length });
            }
        }

        return split;
    }

    /// <summary>Whether a run asked for italic, however it was answered.</summary>
    /// <remarks>
    /// Two states, not one. A run whose family has an italic installed is answered with that face
    /// and <see cref="OpenTypeFace.IsItalic"/> records it; a run whose family has none is answered
    /// with the upright face and a <see cref="FontReference.SyntheticOblique"/> instead. Both asked
    /// for italic, and 26.2.4.2 shears a fallback face for both — measured on
    /// <c>probes/words-r58/fallback-oblique.py</c>'s <c>cjk-italic</c> and <c>cjk-italic-none</c>,
    /// which differ by exactly that and agree at six sheared glyphs in every one of six formats.
    /// Reading only the face would lose the second, which is the arm that reaches a document naming
    /// a family nobody has installed — and those are the documents that fall back in the first
    /// place.
    /// </remarks>
    private static bool AsksForItalic(PageRun run)
        => run.Face.IsItalic || (run.Font?.SyntheticOblique ?? false);

    /// <summary>
    /// The runs cut at every as-character object's boundary, so the pen has somewhere to jump.
    /// </summary>
    /// <remarks>
    /// The same cut <see cref="MeasuredParagraph"/> makes before it shapes, and it has to be the same one:
    /// the pen advances by what it draws, so text after a picture starts where the run before the picture
    /// ended plus the picture's width — and a run drawn across the boundary would draw the whole sentence
    /// from one origin and put the words after the picture underneath it.
    /// </remarks>
    private static List<PageRun> AroundObjects(PageParagraph paragraph, List<PageRun> runs)
    {
        if (!paragraph.HasInlineObjects) return runs;

        List<PageRun> cut = [];

        foreach (PageRun run in runs)
        {
            int at = run.Start;

            foreach (InlineObject one in paragraph.InlineObjects.OrderBy(one => one.Offset))
            {
                if (one.Offset <= at || one.Offset >= run.End) continue;

                cut.Add(run with { Start = at, Length = one.Offset - at });
                at = one.Offset;
            }

            if (at < run.End) cut.Add(run with { Start = at, Length = run.End - at });
        }

        return cut;
    }

    /// <summary>
    /// The runs a stretch draws, in the order they are drawn left to right.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Rule L2 over the runs, which is the whole of what makes a mixed-direction line readable: the
    /// runs are stored in logical order and drawn in visual order, so a Hebrew phrase between two
    /// English ones is drawn in the middle and its own words run the other way. Writer arrives at
    /// the same place by a different route — it keeps the portions logical and jumps the pen by the
    /// whole width of a bidi portion whose direction differs from its surroundings
    /// (<c>SwTextPainter::PaintMultiPortion</c>, <c>sw/source/core/text/pormulti.cxx:1630</c>) —
    /// and the result is identical, because both are rule L2.
    /// </para>
    /// <para>
    /// The split is the itemiser's, not a second one: the same sub-runs the paragraph was
    /// <em>measured</em> against, so a line's drawn width is the width its break was decided with.
    /// Each piece is told its script and its direction, which is not decoration — a shaper handed a
    /// Hebrew run without them lays its glyphs out left to right and the word comes out reversed.
    /// </para>
    /// <para>
    /// A paragraph with nothing right-to-left in it never gets here:
    /// <see cref="TextItemiser.MayReorder"/> is checked first, and the runs are returned exactly as
    /// they were. That matters more than it
    /// looks — a run split at a boundary it does not need loses its shaping context and measures
    /// very slightly wide, which is enough to move a line break.
    /// </para>
    /// </remarks>
    private static List<PageRun> InVisualOrder(PageParagraph paragraph, int start, int end)
    {
        List<(PageRun Run, byte Level)> pieces = Pieces(paragraph, start, end);
        if (pieces.Count == 0) return RunsIn(paragraph, start, end);

        TextItemiser.ReorderVisually(pieces, piece => piece.Level);
        return [.. pieces.Select(piece => piece.Run)];
    }

    /// <summary>
    /// A stretch cut into the pieces one direction and one script each, in logical order.
    /// </summary>
    /// <remarks>
    /// Empty when the paragraph cannot reorder, which is how both callers say "use the runs as they
    /// are" without either of them repeating the test.
    /// </remarks>
    private static List<(PageRun Run, byte Level)> Pieces(
        PageParagraph paragraph, int start, int end)
    {
        BidiDirection direction = paragraph.BaseDirection;
        if (!TextItemiser.MayReorder(paragraph.Text, direction)) return [];

        List<TextItem> items = TextItemiser.Itemise(paragraph.Text, direction);
        List<(PageRun, byte)> pieces = [];

        foreach (PageRun run in RunsIn(paragraph, start, end))
        {
            foreach (TextItem item in items)
            {
                int from = Math.Max(run.Start, item.Start);
                int to = Math.Min(run.End, item.End);
                if (to <= from) continue;

                pieces.Add((
                    run with
                    {
                        Start = from,
                        Length = to - from,
                        Shaping = run.Shaping with
                        {
                            Script = item.Script,
                            RightToLeft = item.IsRightToLeft,
                        },
                    },
                    item.Level));
            }
        }

        return pieces;
    }

    /// <summary>
    /// How far along a line a character position sits, measured from where the line's text starts.
    /// </summary>
    /// <remarks>
    /// <para>
    /// What an as-character anchor needs: an inline picture hangs at the position its anchor character
    /// occupies, and that position is a sum of glyph advances rather than anything pagination recorded.
    /// Answered through the same stretches and the same shaping the line will be <em>drawn</em> with, so
    /// that the picture cannot land somewhere the words around it disagree with — which is the failure
    /// re-measuring in a second way would produce, and it would be invisible in every document without a
    /// tab in it.
    /// </para>
    /// <para>
    /// A position on a later stretch of a tabbed line carries that stretch's own start, so a picture after
    /// a tab hangs at the stop rather than where the text before the tab ended.
    /// </para>
    /// </remarks>
    /// <param name="paragraph">The paragraph the line belongs to.</param>
    /// <param name="line">The line, whose own range bounds the answer.</param>
    /// <param name="at">The character position, as an index into the paragraph's text.</param>
    internal static Length OffsetOnLine(PageParagraph paragraph, PlacedLine line, int at)
    {
        ArgumentNullException.ThrowIfNull(paragraph);

        int start = line.Box.Line.Start;
        int end = Math.Min(line.Box.Line.End, paragraph.Text.Length);
        int position = Math.Clamp(at, start, end);

        foreach (TabbedSegment segment in Stretches(paragraph, start, end, line.StartsParagraph))
        {
            if (position > segment.End) continue;

            return segment.Left + WidthBetween(paragraph, segment.Start, position);
        }

        return Length.Zero;
    }

    /// <summary>
    /// The width of a range of a paragraph's text, in whichever faces cover it.
    /// </summary>
    /// <remarks>
    /// Shaped here rather than taken from the layout, because what reaches a page is a
    /// <see cref="PageParagraph"/> and its line boxes — not the measured paragraph the layout built. The
    /// two agree because both shape the same text in the same faces with the same options; the cost is one
    /// extra shaping pass per tabbed stretch, and only tabbed stretches ask.
    /// </remarks>
    private static Length WidthBetween(PageParagraph paragraph, int from, int to)
    {
        Length total = Length.Zero;

        // The as-character objects the range crosses, on the same half-open rule the prefix table uses —
        // an object at `from` belongs to this range and one at `to` to the next. So the tab stop a line
        // with a picture before it reaches is the stop the layout measured, and the picture's own left
        // edge, which is asked for as the width up to its own offset, does not include itself.
        foreach (InlineObject one in paragraph.InlineObjects)
        {
            if (one.Offset >= from && one.Offset < to) total += one.Width;
        }

        // The same pieces the stretch will be drawn in, unordered — a width is a sum and does not
        // care which way round they go, but it does care that they were shaped the same way, or a
        // tab in a mixed-direction line would advance to a stop the text does not reach.
        List<(PageRun Run, byte Level)> pieces = Pieces(paragraph, from, to);

        foreach (PageRun run in pieces.Count > 0
                     ? pieces.Select(piece => piece.Run)
                     : RunsIn(paragraph, from, to))
        {
            string text = paragraph.Text[run.Start..run.End];
            total += TextShaper.Default.Shape(run.Face, text, run.EffectiveShaping).Width(run.EmSize);

            // One tracking unit per character, which is exactly what the prefix table charges across a
            // range: it puts the gap *before* each character, so a range of n of them carries n. Any other
            // count here would put a tab stop somewhere the layout did not measure.
            if (run.Tracking != Length.Zero) total += run.Tracking * run.Length;
        }

        return total;
    }

    /// <summary>
    /// Builds a glyph run from a shaped stretch of text at an origin.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each glyph's offset is relative to the run's origin, and the pen accumulates across them — which is
    /// what makes a run one draw call rather than one per glyph. The vertical offset is negated because a
    /// shaper's is up-positive and document space is down-positive; getting that backwards puts every
    /// accent below the letter it belongs to.
    /// </para>
    /// <para>
    /// Justification lands here, on the advance of each blank, which is where Writer puts it too: its kern
    /// array adds the space to the blank's own entry (<c>SwFntObj::DrawText</c>) rather than shifting the
    /// words. That keeps a run one draw call and keeps the glyph positions self-consistent, so a backend
    /// that re-measured would still agree with the line's extent.
    /// </para>
    /// </remarks>
    private static GlyphRun Build(
        ShapedText shaped,
        string text,
        Length emSize,
        FontReference font,
        DocPoint origin,
        Length spaceAdd,
        Length tracking = default,
        double widthScale = 1.0)
    {
        List<PositionedGlyph> glyphs = new(shaped.Glyphs.Count);
        List<int> clusters = new(shaped.Glyphs.Count);

        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            // `w:rPr/w:w` multiplies the face's own advance and nothing else. The two distances added
            // below it are page-space distances that the character width does not reach: tracking is a
            // gap between glyphs — measured unscaled on the probe, where a run at 50 per cent with a
            // 40-twip spacing comes to 41.958 + 14 x 2 pt — and a justification share is decided after
            // the line's width is known.
            Length advance = Squeezed(shaped.Scale(glyph.Advance, emSize), widthScale);

            // Every glyph carries its tracking, the run's last included, so the text after a tracked
            // run starts one tracking unit further on.
            //
            // This used to exempt the last glyph, citing `SvxFont::QuickGetTextSize`
            // (`editeng/source/items/svxfont.cxx`:481-500) — "adds one per advance and then takes the
            // trailing one back off". That is a text-*size* query, and `editeng` is Draw and Impress's
            // engine rather than Writer's, so it was worth measuring what the reference actually draws.
            // Against a tracked run of three characters followed by an untracked one at a declared
            // 2.25 pt (`probes/tracking-trailing-gap/`):
            //
            //     Writer   (w:spacing)    A->A 2.20, 2.24    A->BBB 2.29
            //     Impress  (a:rPr/@spc)   A->A 2.24, 2.26    A->ABB 2.29
            //
            // The gap into the untracked run is the same as the gaps inside the tracked one, in both
            // engines — so the last glyph carries its tracking and the two families agree.
            //
            // **The asymmetry this leaves is LibreOffice's own and is deliberate.** The measurement
            // side stays at n − 1 — `MeasuredParagraph`'s prefix table and the tests in
            // `CharacterTrackingTests`, which cite `QuickGetTextSize` for exactly that count — because
            // that citation is about a text *size* query and is right for what it describes. The pen
            // is a different question and the probe measures it directly. So LibreOffice measures a
            // tracked run one unit narrower than it draws it, and reproducing that means reproducing
            // both numbers rather than picking one and making them agree.
            //
            // Making the measurement charge n as well was tried and reverted: it breaks the five
            // `CharacterTrackingTests` that pin the editeng count, and it moves line breaking for
            // every tracked run in the corpus to buy internal tidiness the reference does not have.
            //
            // Visible on `OM template for non-complex NCC operators`, whose header is `Rev.` + a space
            // run + `X` + `of` with no space between the last two: the `X` run's 45 twentieths open a
            // 2.25 pt gap the reference draws, so `pdftotext` reads `X of` there and read `Xof` here.
            if (tracking != Length.Zero) advance += tracking;

            // A blank on a justified line is wider than the font says. Tested on the character the
            // cluster names rather than on the glyph id, because a glyph id means nothing without the
            // face and the cluster is what the shaper guarantees.
            if (spaceAdd != Length.Zero
                && glyph.Cluster >= 0
                && glyph.Cluster < text.Length
                && text[glyph.Cluster] == ' ')
            {
                advance += spaceAdd;
            }

            glyphs.Add(new PositionedGlyph(
                glyph.GlyphId,
                new DocPoint(
                    pen + Squeezed(shaped.Scale(glyph.OffsetX, emSize), widthScale),
                    -shaped.Scale(glyph.OffsetY, emSize)),
                advance));

            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = font,
            FontSize = emSize,
            Origin = origin,
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,

            // The positions above are already squeezed; this is what tells a backend to squeeze the
            // glyphs themselves. See GlyphRun.WidthScale.
            WidthScale = widthScale,
        };
    }

    /// <summary>One advance under the run's character width, or as it is when the run states none.</summary>
    private static Length Squeezed(Length advance, double widthScale)
        => widthScale == 1.0 ? advance : Length.FromEmu((long)Math.Round(advance.Emu * widthScale));

    /// <summary>How far a run's pen travels: the sum of its advances, justification included.</summary>
    private static Length Extent(GlyphRun run)
    {
        Length total = Length.Zero;
        foreach (PositionedGlyph glyph in run.Glyphs) total += glyph.Advance;
        return total;
    }

    /// <summary>
    /// A reference for a paragraph whose font was not resolved through a resolver.
    /// </summary>
    /// <remarks>
    /// Hand-built input — a test, or a caller driving the paginator directly — has a face but no
    /// reference. Naming the face's own family is enough for a backend to group runs by font, and it
    /// records no substitution because none was made.
    /// </remarks>
    /// <summary>
    /// A reference for a face the run did not name, preferring one that can be embedded.
    /// </summary>
    /// <remarks>
    /// A reader stores a <see cref="PageRun.Font"/> for the runs it resolved and leaves it null for
    /// the faces the layout supplied — a list label, a tab leader, a paragraph with no runs. The
    /// name-only reference below is enough to draw with and not enough to embed, because a PDF
    /// writer loads the font program through the face key: measured on
    /// <c>Annex-10-to-the-Aircraft-Maintenance-Specialist-Certification-Rule-GCAA.docx</c>, which
    /// announces <c>DejaVuSans</c> unembedded beside four faces it embeds, and fails the corpus
    /// gate for it. The resolver that loaded the face can still name the file it came from.
    /// </remarks>
    private static FontReference Reference(PageParagraph paragraph, Text.Fonts.OpenTypeFace face)
        => paragraph.Fallback?.ReferenceFor(face) ?? Reference(face);

    private static FontReference Reference(Text.Fonts.OpenTypeFace face) => new()
    {
        FamilyName = face.FamilyName ?? string.Empty,
        Weight = face.Weight,
        IsItalic = face.IsItalic,
        FaceKey = face.FamilyName ?? string.Empty,
    };
}
