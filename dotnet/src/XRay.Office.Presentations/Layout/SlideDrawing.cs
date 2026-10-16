using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Core.Threading;

namespace XRay.Office.Presentations.Layout;

/// <summary>
/// Turns a laid-out slide into drawing commands.
/// </summary>
/// <remarks>
/// <para>
/// Background, then each shape's fill, its outline and its text, in the order the file states
/// its shapes. That order is z-order in both PresentationML and ODF — a shape tree is written
/// back to front — so nothing here sorts anything.
/// </para>
/// <para>
/// Fill before line before text, per shape rather than per slide. A stroke is centred on its own
/// path, so half of it overlaps the fill it belongs to; drawing every fill first and then every
/// outline would put a shape's outline on top of the shape in front of it.
/// </para>
/// </remarks>
public static class SlideDrawing
{
    /// <summary>Draws a slide into a sink.</summary>
    /// <param name="slide">The slide to draw.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public static void Draw(LaidOutSlide slide, IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(slide);
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(slide.Size);
        try
        {
            if (slide.Background is { } background)
            {
                sink.FillPath(
                    GraphicsPath.Rectangle(new DocRect(DocPoint.Origin, slide.Size)), background);
            }

            foreach (PlacedShape shape in slide.Shapes)
            {
                OfficeOperation.ThrowIfCancelled();
                DrawShape(shape, sink);
            }
        }
        finally
        {
            // Always closed, even when a sink throws part way through: a page left open makes the
            // next one nest inside it, turning one bad slide into a broken document.
            sink.EndPage();
        }
    }

    private static void DrawShape(PlacedShape shape, IDrawingSink sink)
    {
        if (shape.Shadow is { } shadow) DrawShadow(shape, shadow, sink);

        if (shape.Fill is { } fill && FillReachesThePage(shape.Picture))
        {
            Fill(shape.FillOutline, fill, sink);

            // A preset's shading faces, in the order it states them. See `PlacedShape.ShadedParts`.
            foreach (SlideShadedPart part in shape.ShadedParts)
            {
                Fill(part.Outline, Brightened(fill, part.Brightness), sink);
            }
        }

        if (shape.Picture is { } picture) DrawPicture(shape, picture, sink);
        if (shape.Line is { } line) sink.StrokePath(shape.StrokeOutline, line);

        if (shape.Text is not { HasInk: true } text) return;

        // An upright shape's runs are already in slide coordinates, so the common case costs no
        // state-stack traffic at all — and, more usefully, its pens land in a backend's output as
        // plain numbers that compare directly against a reference renderer's.
        if (text.IsUpright)
        {
            DrawMarkerPictures(text, sink);
            foreach (PlacedGlyphRun run in text.Runs) DrawRun(run, sink);
            return;
        }

        sink.Save();
        try
        {
            sink.Transform(text.Transform);
            DrawMarkerPictures(text, sink);
            foreach (PlacedGlyphRun run in text.Runs) DrawRun(run, sink);
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>
    /// Draws a body's picture bullets, under the text's own transform and before its glyphs.
    /// </summary>
    /// <remarks>
    /// Before, because <c>Outliner::PaintBullet</c> is called from
    /// <c>ImpEditEngine::Paint</c>'s per-paragraph loop ahead of that paragraph's portions, and
    /// a bullet wide enough to reach under the first line would otherwise cover it. Unclipped:
    /// the shape's own clip is a property of the shape, and a bullet is drawn in the paragraph's
    /// coordinates exactly as its text is.
    /// </remarks>
    private static void DrawMarkerPictures(PlacedText text, IDrawingSink sink)
    {
        if (text.MarkerPictures is not { Count: > 0 } pictures) return;

        foreach (PlacedPicture picture in pictures)
        {
            if (picture.Destination.IsEmpty) continue;

            if (picture.Vector is { } vector && !vector.Value.IsEmpty)
            {
                vector.Value.Draw(sink, picture.Destination);
            }
            else if (picture.Image is { } image)
            {
                sink.DrawImage(image, picture.Destination, picture.Opacity);
            }
        }
    }

    /// <summary>
    /// Fills a shape's outline, expanding a hatch into the commands that draw one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A hatch is the one <see cref="Paint"/> no backend implements, deliberately — see the
    /// note on <see cref="Paint"/> itself. It is expanded here, at the last point that still
    /// knows the shape's own outline, into the background fill and the clipped hairlines
    /// <c>FillHatchPrimitive2D</c> decomposes into, so the sink is handed nothing it did not
    /// already understand.
    /// </para>
    /// <para>
    /// Clipped to the outline rather than intersected with it, because a hatch fills a rounded
    /// rectangle and a chevron as readily as a box and the segments
    /// <see cref="Hatching.Lines"/> produces span the outline's whole bounding box.
    /// </para>
    /// </remarks>
    private static void Fill(GraphicsPath outline, Paint fill, IDrawingSink sink)
    {
        if (fill is not HatchPaint hatch)
        {
            sink.FillPath(outline, fill);
            return;
        }

        if (hatch.Background is { } background)
        {
            sink.FillPath(outline, Paint.Solid(background));
        }

        Stroke pen = new(Paint.Solid(hatch.LineColour), Length.Zero);

        sink.Save();
        try
        {
            sink.ClipPath(outline);

            foreach ((DocPoint from, DocPoint to) in Hatching.Lines(Bounds(outline), hatch))
            {
                sink.StrokePath(new GraphicsPath().MoveTo(from).LineTo(to), pen);
            }
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>The bounding box of a path's own points.</summary>
    /// <remarks>
    /// Control points included, which overstates a curved outline's box slightly and is right
    /// here: the hatch is clipped to the outline, so a box that is too large costs a few
    /// segments the clip discards and a box that is too small leaves a corner unhatched.
    /// </remarks>
    private static DocRect Bounds(GraphicsPath path)
    {
        bool any = false;
        Length left = default, top = default, right = default, bottom = default;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            foreach (DocPoint point in command.Verb == PathVerb.CubicTo
                         ? (ReadOnlySpan<DocPoint>)[command.Point, command.Control1, command.Control2]
                         : (ReadOnlySpan<DocPoint>)[command.Point])
            {
                if (!any)
                {
                    left = right = point.X;
                    top = bottom = point.Y;
                    any = true;
                    continue;
                }

                if (point.X < left) left = point.X;
                if (point.X > right) right = point.X;
                if (point.Y < top) top = point.Y;
                if (point.Y > bottom) bottom = point.Y;
            }
        }

        return any ? DocRect.FromCorners(new DocPoint(left, top), new DocPoint(right, bottom)) : DocRect.Empty;
    }

    /// <summary>
    /// Draws a shape's drop shadow: the shape again, offset, in one colour, behind itself.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The order of the three pieces is the shape's own — fill, then picture, then outline, then
    /// text — because a shadow is the shape's whole decomposition put through a colour
    /// replacement, not a separate drawing
    /// (<c>svx/source/sdr/primitive2d/sdrdecompositiontools.cxx:860-900</c>). With one colour the
    /// order cannot show, but keeping it means the shadow of a shape is derived from the shape
    /// rather than reinvented beside it, and the two cannot drift apart.
    /// </para>
    /// <para>
    /// The offset goes on the state stack rather than into the geometry. A shape's outline is
    /// already in slide coordinates and its text may not be, so translating the two separately
    /// would need the text's own matrix pre-multiplied by hand; one <see cref="IDrawingSink.Transform"/>
    /// outside both is the same translation applied once, and it is what puts the shadow's glyph
    /// runs into the PDF as real text at real positions.
    /// </para>
    /// <para>
    /// <b>A picture casts the silhouette of its own pixels, not of its frame.</b> That is what
    /// LibreOffice casts — the bitmap with every colour replaced and its alpha kept — and it is
    /// expressible here without this layer gaining a codec, because a
    /// <see cref="DuotoneRecolour"/> whose two ends are the same colour <em>is</em> "every colour
    /// replaced by one", and the decoder that carries it out already leaves alpha untouched.
    /// </para>
    /// <para>
    /// This used to be approximated by filling the shape's frame, and only for a picture whose
    /// bytes began with a JPEG start-of-image marker — on the grounds that a JPEG has no alpha, so
    /// its silhouette is its frame, while a PNG's is not and a cut-out logo would gain a black
    /// rectangle behind it. The approximation was exact where it applied and cast no shadow at all
    /// anywhere else. Measured on <c>1-secretariat.PPT</c> page 1, whose winged logo is a PNG: the
    /// reference draws the picture <em>twice</em>, once as itself and once as a flat <c>#808080</c>
    /// copy carrying the same 852-byte soft mask, offset by the shape's 6 pt shadow. Recolouring
    /// the picture reproduces that and retires the JPEG special case, which it subsumes — for an
    /// opaque raster the silhouette and the frame are the same pixels.
    /// </para>
    /// </remarks>
    private static void DrawShadow(PlacedShape shape, SlideShadow shadow, IDrawingSink sink)
    {
        if (shadow.IsInvisible) return;

        bool silhouette = shape.Fill is not null && FillReachesThePage(shape.Picture);
        bool picture = shape.Picture is { Vector: null, Image: not null, Destination.IsEmpty: false };
        bool outline = shape.Line is not null;
        bool text = shadow.CarriesText && shape.Text is { Runs.Count: > 0 };

        if (!silhouette && !picture && !outline && !text) return;

        Paint paint = Paint.Solid(shadow.Colour);
        bool grouped = shadow.Opacity < 1.0;

        sink.Save();
        try
        {
            sink.Transform(AffineTransform.Translation(shadow.OffsetX.Emu, shadow.OffsetY.Emu));
            if (grouped) sink.BeginTransparencyGroup(shadow.Opacity);

            try
            {
                if (silhouette)
                {
                    Paint silhouettePaint =
                        shape.Fill is { } fill ? Recoloured(fill, shadow.Colour) : paint;

                    sink.FillPath(shape.FillOutline, silhouettePaint);

                    // A shading face is part of the silhouette; its own brightness is not, because
                    // a shadow is one colour.
                    foreach (SlideShadedPart part in shape.ShadedParts)
                    {
                        sink.FillPath(part.Outline, silhouettePaint);
                    }
                }

                if (picture) DrawPictureSilhouette(shape, shape.Picture!, shadow.Colour, sink);

                if (shape.Line is { } line)
                {
                    sink.StrokePath(shape.StrokeOutline, line with { Paint = Recoloured(line.Paint, shadow.Colour) });
                }

                if (text) DrawShadowText(shape.Text!, paint, sink);
            }
            finally
            {
                if (grouped) sink.EndTransparencyGroup();
            }
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>
    /// A paint with every colour replaced and every alpha kept.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>BColorModifier_replace</c> (<c>basegfx/source/color/bcolormodifier.cxx</c>),
    /// which is what a <c>ShadowPrimitive2D</c> wraps its children in — and it modifies the
    /// <em>colour</em> of a primitive and not its transparency, which are separate things all the
    /// way down LibreOffice's drawing model. Filling the outline with a flat opaque colour
    /// instead looks like the same thing and is not.
    /// </para>
    /// <para>
    /// Measured, because the difference is a whole slide rather than a detail. Page 34 of
    /// <c>Intersil_Italy_CAN_Bus_Transceiver_Presentation_Final.pptx</c> is covered by a
    /// 10515600 × 3912860 EMU rectangle whose gradient runs from <c>FFC000</c> at zero alpha to
    /// the same at 30%, and which states <c>&lt;a:outerShdw&gt;</c> with no distance and no blur —
    /// so its shadow sits exactly underneath it. Cast as an opaque rectangle at the shadow's 50%,
    /// it tints the entire slide and the page's unaccounted ink goes from 0.18% to 13.52%. Cast
    /// with the gradient's own alpha it is invisible, which is what the reference shows.
    /// </para>
    /// </remarks>
    /// <summary>
    /// A fill taken towards white or black, which is what a shaded subpath is drawn in.
    /// </summary>
    /// <remarks>
    /// <c>EnhancedCustomShape2d::GetColorData</c> (<c>EnhancedCustomShape2d.cxx</c>:1084-1105):
    /// a positive brightness blends towards white as <c>c(1−b) + 255b</c> and a negative one
    /// scales as <c>c(1+b)</c>, both truncated. Only a solid fill is shaded — LibreOffice shades
    /// the colour data of the object, and a gradient, hatch or bitmap face keeps the fill it has,
    /// which is also what every corpus deck's shaded preset carries.
    /// </remarks>
    private static Paint Brightened(Paint paint, double brightness)
    {
        if (paint is not SolidPaint solid || brightness == 0.0) return paint;

        static byte Blend(byte channel, double brightness) => (byte)Math.Clamp(
            brightness >= 0.0
                ? (int)((channel * (1.0 - brightness)) + (brightness * 255.0))
                : (int)(channel * (1.0 + brightness)),
            0,
            255);

        Colour colour = solid.Colour;

        return Paint.Solid(new Colour(
            Blend(colour.R, brightness),
            Blend(colour.G, brightness),
            Blend(colour.B, brightness),
            colour.A));
    }

    private static Paint Recoloured(Paint paint, Colour colour) => paint switch
    {
        SolidPaint solid => Paint.Solid(colour.WithAlpha(solid.Colour.A)),

        GradientPaint gradient => gradient with
        {
            Stops = [.. gradient.Stops.Select(
                stop => stop with { Colour = colour.WithAlpha(stop.Colour.A) })],
        },

        // A hatch's shadow is its background box in the shadow's colour. Drawing the lines again
        // would be more faithful and is not worth a second expansion: a hatch that states no
        // background casts a shadow here that Impress would leave as lines, which no corpus deck
        // asks for — none of the seven carrying an a:pattFill puts a shadow on it.
        // A bitmap or mesh fill has per-pixel alpha this layer cannot see without a codec, so its
        // shadow is the flat colour — right for an opaque one and too solid for a masked one.
        _ => Paint.Solid(colour),
    };

    /// <summary>Draws a shape's text in one colour, in the same place its own text goes.</summary>
    private static void DrawShadowText(PlacedText text, Paint paint, IDrawingSink sink)
    {
        if (text.IsUpright)
        {
            foreach (PlacedGlyphRun run in text.Runs) DrawShadowRun(run, paint, sink);
            return;
        }

        sink.Save();
        try
        {
            sink.Transform(text.Transform);
            foreach (PlacedGlyphRun run in text.Runs) DrawShadowRun(run, paint, sink);
        }
        finally
        {
            sink.Restore();
        }
    }

    private static void DrawShadowRun(in PlacedGlyphRun run, Paint paint, IDrawingSink sink)
    {
        sink.DrawGlyphRun(run.Run, paint);

        if (run.Rules is not { Count: > 0 } rules) return;

        foreach (DocRect rule in rules) sink.FillPath(GraphicsPath.Rectangle(rule), paint);
    }

    /// <summary>
    /// Whether a shape's own fill is drawn at all, given the picture that goes over it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A picture frame's fill is <em>not</em> a backdrop the picture happens to hide. Impress adds
    /// it to the shape's decomposition only when the graphic can be seen through
    /// (<c>svx/source/sdr/primitive2d/sdrgrafprimitive2d.cxx:41-42</c>, "add fill, but only when
    /// graphic is transparent"), and a <em>metafile</em> loaded from a file does not count as
    /// transparent however much of its frame it leaves unpainted. So a plate behind an EMF chart
    /// reaches the page in PowerPoint and does not in Impress.
    /// </para>
    /// <para>
    /// Measured against LibreOffice 24.2.7.2 rather than read out of that source, which predicts
    /// the opposite: its <c>ImpGraphic::isTransparent</c> returns true for everything but an
    /// opaque bitmap. Four renderings of
    /// <c>slides/batch-014/pptx/2014BSA_Sunday_Killion.pptx</c> settle what the binary does. Its
    /// slide 5 is a <c>p:pic</c> over <c>image10.emf</c> stating
    /// <c>&lt;a:solidFill&gt;&lt;a:schemeClr val="tx1"/&gt;</c> — white, under a colour map sending
    /// <c>tx1</c> to a <c>dk1</c> of <c>#FFFFFF</c>. Rendering the deck as found, with that fill
    /// changed to red, and with it replaced by <c>&lt;a:noFill/&gt;</c> gives a <em>byte-identical</em>
    /// page 5 all three times, showing the slide's own background inside the picture frame: no
    /// fill is drawn whatever it says. A fourth, with the EMF swapped for a PNG whose right half
    /// is fully clear, draws the red through that half — so the rule is about the graphic's kind,
    /// not about whether it happens to cover its frame.
    /// </para>
    /// <para>
    /// A shape-level opacity is the other half of Impress's condition
    /// (<c>255 != getGraphicAttr().GetAlpha()</c>) and keeps the fill whatever the picture is.
    /// </para>
    /// <para>
    /// <strong>Only a metafile the package carries as an entry of its own.</strong> The same 892
    /// byte EMF inlined as <c>office:binary-data</c> in a flat ODP has its fill drawn by 24.2.7.2,
    /// and the same document zipped with the metafile under <c>Pictures/</c> does not; the two say
    /// the same thing and differ only in how LibreOffice loads the graphic. A `.ppt` falls on the
    /// inline side of that line and needs to: page 8 of
    /// <c>slides/batch-014/ppt/Thailand17.ppt</c> and page 10 of
    /// <c>slides/batch-010/ppt/W3_Case_Study…Ed.ppt</c> draw a table on a white plate under an
    /// Escher metafile blip, and suppressing it cost 8.44 and 9.33 unaccounted ink.
    /// </para>
    /// <para>
    /// [24.2.7-audit: VERIFIED 2026-08-21, round 56 — both halves hold on 26.2.4.2, and the
    /// second was checked with a <b>discriminating pair</b> rather than a single rendering.
    /// The package-entry half: <c>2014BSA_Sunday_Killion.pptx</c> rendered as found, with the
    /// frame's fill changed to red, and with it replaced by <c>a:noFill</c> gives three
    /// <em>byte-identical</em> page-5 images, so no fill reaches the page whatever the frame
    /// states. The inline half: one authored flat ODP holding that same 306 kB EMF as
    /// <c>office:binary-data</c> under a red-filled frame draws <b>108 304 red pixels</b>, and
    /// the reference's own <c>--convert-to odp</c> of it — the same fill, the same frame, the
    /// bytes moved to <c>Pictures/</c> — draws <b>none</b>. One rendering of either alone says
    /// nothing; the pair is the measurement. <c>probes/slides-r56/audit_picturefill.py</c>.]
    /// </para>
    /// <para>
    /// <strong>The correlation is storage; the cause may not be.</strong> Across five measured
    /// cases every inline metafile keeps its fill and every package entry loses it, which fits
    /// LibreOffice building a <c>GDIMetaFile</c> graphic for the first and a
    /// <c>VectorGraphicData</c> for the second. But those two `.ppt` pages are also pages where
    /// the reference <em>rasterises</em> the metafile — both are on
    /// <c>TODO.raster-ceiling.md</c> — and a raster with a soft mask is transparent by
    /// <c>ImpGraphic::isTransparent</c>'s own rule, which explains them just as well. Separating
    /// the two needs a document whose metafile is rasterised and is <em>not</em> inline, and the
    /// corpus has none whose picture frame states a fill: the four candidates on
    /// <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c> all state <c>a:noFill</c>. Recorded as
    /// unresolved rather than settled.
    /// </para>
    /// </remarks>
    private static bool FillReachesThePage(PlacedPicture? picture)
        => picture is null
           || picture.Opacity < 1.0
           || picture.Destination.IsEmpty
           || picture.IsInline
           || picture.Vector is not { } vector
           || vector.Value.IsEmpty;

    /// <summary>
    /// Whether a picture is one whose silhouette is its whole frame.
    /// </summary>
    /// <remarks>
    /// Decided from the bytes rather than from the declared media type, which office files
    /// mislabel as routinely as they mislabel themselves: <c>FF D8 FF</c> is a JPEG's start-of-image
    /// marker and JPEG has no alpha channel, so every pixel of one is opaque. A decoded image is
    /// taken as shaped, because nothing here can look at its alpha without a codec.
    /// </remarks>
    private static bool IsOpaqueRaster(PlacedPicture? picture)
        => picture is { Vector: null, Image: { } image }
           && !image.IsDecoded
           && image.EncodedBytes.Length >= 3
           && image.EncodedBytes.Span[0] == 0xFF
           && image.EncodedBytes.Span[1] == 0xD8
           && image.EncodedBytes.Span[2] == 0xFF;

    /// <summary>
    /// Draws one run and the rules under and through it.
    /// </summary>
    /// <remarks>
    /// The rules go on after the glyphs and in the run's own colour, which is what every renderer
    /// here does with a decoration: they are filled rectangles rather than strokes so that a
    /// half-point rule stays a half-point rule instead of becoming the device's thinnest line.
    /// </remarks>
    private static void DrawRun(in PlacedGlyphRun run, IDrawingSink sink)
    {
        sink.DrawGlyphRun(run.Run, Paint.Solid(run.Colour));

        if (run.Rules is not { Count: > 0 } rules) return;

        foreach (DocRect rule in rules)
        {
            sink.FillPath(
                new GraphicsPath()
                    .MoveTo(new DocPoint(rule.X, rule.Y))
                    .LineTo(new DocPoint(rule.X + rule.Width, rule.Y))
                    .LineTo(new DocPoint(rule.X + rule.Width, rule.Y + rule.Height))
                    .LineTo(new DocPoint(rule.X, rule.Y + rule.Height))
                    .Close(),
                Paint.Solid(run.Colour));
        }
    }

    /// <summary>
    /// Draws a picture inside the shape that holds it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Always clipped to the outline, for two reasons that both matter. A crop is expressed by
    /// drawing the whole picture into a rectangle larger than the frame, so without a clip a
    /// cropped picture spills across the slide; and a picture frame need not be rectangular —
    /// PowerPoint will put a photograph inside any preset geometry — so the outline is what
    /// bounds it rather than the destination rectangle.
    /// </para>
    /// <para>
    /// Between the fill and the line: the fill is behind a picture with transparency, and the
    /// frame's own border is drawn over the picture's edge, which is what puts a hairline
    /// exactly on the boundary rather than half under it.
    /// </para>
    /// </remarks>
    /// <summary>
    /// Draws a picture as a flat silhouette of one colour, keeping its alpha — the shadow a
    /// picture casts.
    /// </summary>
    /// <remarks>
    /// The same geometry as <see cref="DrawPicture"/>, deliberately: the clip, the destination and
    /// the opacity all come from the same places, so the shadow cannot drift away from the shape
    /// it belongs to. Only the pixels differ, and they differ by a
    /// <see cref="DuotoneRecolour"/> whose two ends are the shadow's colour — which maps every
    /// luminance onto that one colour and leaves the alpha channel alone.
    /// </remarks>
    /// <param name="shape">The shape whose outline clips the picture.</param>
    /// <param name="picture">The picture to cast.</param>
    /// <param name="colour">The shadow's colour.</param>
    /// <param name="sink">Where to draw.</param>
    private static void DrawPictureSilhouette(
        PlacedShape shape, PlacedPicture picture, Colour colour, IDrawingSink sink)
    {
        if (picture.Image is not { } image) return;

        sink.Save();
        try
        {
            sink.ClipPath(shape.Outline);
            sink.DrawImage(
                image with { Duotone = new DuotoneRecolour(colour, colour) },
                picture.Destination,
                picture.Opacity);
        }
        finally
        {
            sink.Restore();
        }
    }

    private static void DrawPicture(PlacedShape shape, PlacedPicture picture, IDrawingSink sink)
    {
        if (picture.Destination.IsEmpty) return;

        sink.Save();
        try
        {
            sink.ClipPath(shape.Outline);

            // The vector wins where a shape has both, which is the DrawingML `svgBlip` case and
            // nothing else; an empty decode falls back to the raster the file put there for it.
            //
            // `VectorImage.Draw` stretches the picture's own *frame* onto the destination and not
            // the extent of its ink, which is the same rule `DrawImage` follows for a raster and
            // is why a crop is expressed here as a larger destination plus a clip rather than as
            // anything the picture knows about.
            if (picture.Vector is { } vector && !vector.Value.IsEmpty)
            {
                vector.Value.Draw(sink, picture.Destination);
            }
            else if (picture.Image is { } image)
            {
                sink.DrawImage(image, picture.Destination, picture.Opacity);
            }
        }
        finally
        {
            sink.Restore();
        }
    }
}
