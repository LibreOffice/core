using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// The seven glyphs an <c>iconSet</c> conditional format paints, as vector paths.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why vector paths rather than the reference's own bitmaps.</strong> The reference
/// paints an icon-theme asset: <c>drawIconSets</c> (<c>sc/source/ui/view/output.cxx</c>:960-989,
/// this tree) hands <c>ScIconSetFormat::getBitmap</c> a name and draws the <c>Bitmap</c> it gets
/// back, and the name resolves against whichever icon theme the application is running. Three
/// facts settle the choice for this tree:
/// </para>
/// <list type="number">
///   <item><description>
///     <strong>The corpus reaches seven distinct assets, not twenty-two sets.</strong> Every one
///     of the sixty icons 26.2.4.2 draws over the ten corpus documents that state a rule is one of
///     these seven, because eighteen of the twenty rules are <c>custom="1"</c> and most of their
///     buckets are <c>NoIcons</c>. Seven simple shapes — three flags, a diamond and three discs —
///     is less new surface than a raster pipeline plus an asset set.
///   </description></item>
///   <item><description>
///     <strong>The artwork is already vector upstream.</strong> The <c>colibre</c> assets are
///     authored as SVG (<c>icon-themes/colibre_svg/sc/res/icon-set-*.svg</c>) and the PNGs the
///     application loads are rasterisations of them, so re-authoring the paths reproduces the
///     source of the reference's own bitmap rather than approximating its output. Every
///     coordinate below is that SVG's, converted from its relative spelling to absolute, and
///     every colour is the SVG's own hex.
///   </description></item>
///   <item><description>
///     <strong>An embedded raster would bind us to one theme of one version.</strong> The
///     extracted images do not byte-match any installed <c>images_*.zip</c> entry even at
///     26.2.4.2 — the PDF writer re-encodes them — so a raster copy would be a snapshot with no
///     provenance, and a licensed binary asset in a source tree besides.
///   </description></item>
/// </list>
/// <para>
/// <strong>Which theme, established by measurement rather than assumed.</strong> The seven images
/// were extracted with their soft masks from 26.2.4.2's own PDFs of the seven corpus documents
/// that draw an icon and compared against every <c>icon-set-*</c> entry of all twenty
/// <c>images_*.zip</c> files the reference installs. Each one's nearest match is
/// <c>images_colibre.zip</c>, at a mean absolute channel difference of <strong>0.01 to 0.06 out
/// of 255</strong> — the next theme is two orders of magnitude away. So the reference runs
/// <c>colibre</c> headless, and these are its shapes. <c>probes/iconset-r103/reference-glyphs.txt</c>.
/// </para>
/// <para>
/// <strong>What is not here.</strong> Fifteen of the reference's twenty-two sets name assets no
/// corpus document reaches, and they are recorded as unreached rather than drawn on a guess —
/// arrows, traffic lights, smilies, stars, triangles, pies, bars, boxes and the grey ramps. Two
/// further indices of a set the corpus does reach are unreached the same way
/// (<c>3Signs</c> 1 and 2). See the write-up's table.
/// </para>
/// </remarks>
internal static class SheetIconArtwork
{
    /// <summary>The side of the square the paths below are authored in.</summary>
    /// <remarks>
    /// Every <c>colibre</c> icon-set asset states <c>viewBox="0 0 16 16"</c> and every one the
    /// reference embeds is 16 × 16 pixels, which is also why <c>drawIconSets</c>'s aspect ratio
    /// (<c>output.cxx</c>:982-984) is 1 and the drawn icon is square.
    /// </remarks>
    private const double Box = 16.0;

    /// <summary>The flagpole's grey, and the only colour outside the three-way ramp.</summary>
    private static readonly Colour Pole = new(0x3A, 0x3A, 0x38);

    /// <summary>The white the three disc glyphs' marks are drawn in.</summary>
    private static readonly Colour Mark = new(0xFA, 0xFA, 0xFA);

    private static readonly Colour RedDark = new(0xD4, 0x23, 0x14);
    private static readonly Colour RedLight = new(0xFF, 0x91, 0x98);
    private static readonly Colour AmberDark = new(0xED, 0x87, 0x33);
    private static readonly Colour AmberLight = new(0xF8, 0xDB, 0x8F);
    private static readonly Colour GreenDark = new(0x30, 0x90, 0x48);
    private static readonly Colour GreenLight = new(0xA1, 0xDD, 0xAA);

    /// <summary>Draws one glyph filling a square box.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="box">Where the glyph goes, in the page's own coordinates.</param>
    /// <param name="glyph">Which glyph.</param>
    public static void Draw(IDrawingSink sink, DocRect box, SheetIconGlyph glyph)
    {
        ArgumentNullException.ThrowIfNull(sink);

        if (box.Width <= Length.Zero || box.Height <= Length.Zero) return;

        Frame frame = new(box);

        switch (glyph)
        {
            case SheetIconGlyph.FlagRed:
                Flag(sink, frame, RedDark, RedLight);
                break;
            case SheetIconGlyph.FlagAmber:
                Flag(sink, frame, AmberDark, AmberLight);
                break;
            case SheetIconGlyph.FlagGreen:
                Flag(sink, frame, GreenDark, GreenLight);
                break;
            case SheetIconGlyph.Diamond:
                Diamond(sink, frame);
                break;
            case SheetIconGlyph.CrossInRed:
                Disc(sink, frame, RedDark, RedLight);
                Cross(sink, frame);
                break;
            case SheetIconGlyph.ExclamationInAmber:
                Disc(sink, frame, AmberDark, AmberLight);
                Exclamation(sink, frame);
                break;
            case SheetIconGlyph.TickInGreen:
                Disc(sink, frame, GreenDark, GreenLight);
                Tick(sink, frame);
                break;
            default:
                // `Unpainted`. The reference draws an asset this tree has no path for; the cell
                // still has an `ScIconSetInfo`, which is why the caller may already have taken
                // its number off the page, but nothing is painted here rather than something
                // guessed. Unreached by the corpus.
                break;
        }
    }

    /// <summary>
    /// <c>icon-set-flags-{red,amber,green}.svg</c>: a rounded pole and a two-tone banner.
    /// </summary>
    /// <remarks>
    /// The three flags are one geometry with three palettes — the assets' path data is identical
    /// byte for byte and only the two <c>fill</c> attributes differ.
    /// </remarks>
    private static void Flag(IDrawingSink sink, Frame frame, Colour dark, Colour light)
    {
        // The pole: `m3 0 c-.554 0-1 .446-1 1 v14 c0 .554.446 1 1 1 s1-.446 1-1 v-14
        // c0-.554-.446-1-1-1z`, with the smooth cubic's implicit first control written out.
        sink.FillPath(
            new GraphicsPath()
                .MoveTo(frame.At(3, 0))
                .CubicTo(frame.At(2.446, 0), frame.At(2, 0.446), frame.At(2, 1))
                .LineTo(frame.At(2, 15))
                .CubicTo(frame.At(2, 15.554), frame.At(2.446, 16), frame.At(3, 16))
                .CubicTo(frame.At(3.554, 16), frame.At(4, 15.554), frame.At(4, 15))
                .LineTo(frame.At(4, 1))
                .CubicTo(frame.At(4, 0.446), frame.At(3.554, 0), frame.At(3, 0))
                .Close(),
            Paint.Solid(Pole));

        // The banner's outer edge, whose top and bottom both sag: `m4 0v8c2.9286006.4434277
        // 8.676455 2.009384 12 0v-8c-3.049936 1.9641378-9.013204.5193558-12 0z`.
        sink.FillPath(
            new GraphicsPath()
                .MoveTo(frame.At(4, 0))
                .LineTo(frame.At(4, 8))
                .CubicTo(frame.At(6.9286006, 8.4434277), frame.At(12.676455, 10.009384), frame.At(16, 8))
                .LineTo(frame.At(16, 0))
                .CubicTo(frame.At(12.950064, 1.9641378), frame.At(6.986796, 0.5193558), frame.At(4, 0))
                .Close(),
            Paint.Solid(dark));

        // Its lighter inside, inset by one unit on each side: `m5 1.1835938v5.96875c2.8741671
        // .5080153 7.454202 1.3390915 10 .1464843v-5.9121093c-1.481628.4974508-3.058766.6797564
        // -4.613281.5390624-1.65566-.1498482-4.0770646-.4810487-5.386719-.7421874z`.
        sink.FillPath(
            new GraphicsPath()
                .MoveTo(frame.At(5, 1.1835938))
                .LineTo(frame.At(5, 7.1523438))
                .CubicTo(frame.At(7.8741671, 7.6603591), frame.At(12.454202, 8.4914353), frame.At(15, 7.2988281))
                .LineTo(frame.At(15, 1.3867188))
                .CubicTo(frame.At(13.518372, 1.8841696), frame.At(11.941234, 2.0664752), frame.At(10.386719, 1.9257812))
                .CubicTo(frame.At(8.731059, 1.7759330), frame.At(6.3096544, 1.4447325), frame.At(5, 1.1835938))
                .Close(),
            Paint.Solid(light));
    }

    /// <summary>
    /// <c>icon-set-shapes-diamond.svg</c>: a square on its point, outlined by a second one.
    /// </summary>
    /// <remarks>
    /// The asset draws the outline as an even-odd pair of diamonds rather than as a stroke, and
    /// the inner one's <c>1.4140625</c> is <c>2 − √2 ⁄ 2</c> rounded to the SVG's own precision —
    /// a one-unit inset measured perpendicular to the edge. Two fills reproduce it exactly and
    /// need no fill rule.
    /// </remarks>
    private static void Diamond(IDrawingSink sink, Frame frame)
    {
        sink.FillPath(
            new GraphicsPath()
                .MoveTo(frame.At(8, 0)).LineTo(frame.At(16, 8))
                .LineTo(frame.At(8, 16)).LineTo(frame.At(0, 8)).Close(),
            Paint.Solid(RedDark));

        sink.FillPath(
            new GraphicsPath()
                .MoveTo(frame.At(8, 1.4140625)).LineTo(frame.At(14.585938, 8))
                .LineTo(frame.At(8, 14.585938)).LineTo(frame.At(1.4140625, 8)).Close(),
            Paint.Solid(RedLight));
    }

    /// <summary>The disc the three <c>symbols1</c> glyphs sit on: <c>r=8</c> over <c>r=7</c>.</summary>
    private static void Disc(IDrawingSink sink, Frame frame, Colour dark, Colour light)
    {
        sink.FillPath(Circle(frame, 8, 8, 8), Paint.Solid(dark));
        sink.FillPath(Circle(frame, 8, 8, 7), Paint.Solid(light));
    }

    /// <summary>
    /// <c>icon-set-symbols1-check.svg</c>'s tick, which overshoots the disc at its long end.
    /// </summary>
    /// <remarks>
    /// <c>m2.9743327 8.6823554 4.017513 3.3539126 8.0350293-10.061733</c> at
    /// <c>stroke-width="1.927546"</c> with round caps and joins. Its far end lands at
    /// <c>(15.03, 1.97)</c>, outside the <c>r=7</c> inner disc and just inside the box, which is
    /// why the rendered glyph's tick appears to run off the top right of the circle.
    /// </remarks>
    private static void Tick(IDrawingSink sink, Frame frame)
        => sink.StrokePath(
            new GraphicsPath()
                .MoveTo(frame.At(2.9743327, 8.6823554))
                .LineTo(frame.At(6.9918457, 12.0362680))
                .LineTo(frame.At(15.0268750, 1.9745350)),
            new Stroke(Paint.Solid(Mark), frame.Units(1.927546), LineCap.Round, LineJoin.Round));

    /// <summary><c>icon-set-symbols1-cross.svg</c>'s two bars, drawn corner to corner.</summary>
    private static void Cross(IDrawingSink sink, Frame frame)
    {
        Stroke stroke = new(Paint.Solid(Mark), frame.Units(2), LineCap.Round, LineJoin.Round);

        sink.StrokePath(new GraphicsPath().MoveTo(frame.At(4, 12)).LineTo(frame.At(12, 4)), stroke);
        sink.StrokePath(new GraphicsPath().MoveTo(frame.At(12, 12)).LineTo(frame.At(4, 4)), stroke);
    }

    /// <summary>
    /// <c>icon-set-symbols1-exclamation-mark.svg</c>'s bar and dot.
    /// </summary>
    /// <remarks>
    /// The asset spells them as two rounded rectangles, <c>x=7 y=2 w=2 h=9 ry=1</c> and
    /// <c>x=7 y=12 w=2 h=2 ry=1</c>. A width-2 radius on a width-2 rectangle is a capsule, so the
    /// first is a round-capped stroke down its own centre line and the second is a circle — the
    /// same outline with two fewer path elements.
    /// </remarks>
    private static void Exclamation(IDrawingSink sink, Frame frame)
    {
        sink.StrokePath(
            new GraphicsPath().MoveTo(frame.At(8, 3)).LineTo(frame.At(8, 10)),
            new Stroke(Paint.Solid(Mark), frame.Units(2), LineCap.Round, LineJoin.Round));

        sink.FillPath(Circle(frame, 8, 13, 1), Paint.Solid(Mark));
    }

    /// <summary>A circle as four cubics, which is what every path sink here can take.</summary>
    private static GraphicsPath Circle(Frame frame, double centreX, double centreY, double radius)
    {
        // The usual circular-arc constant: 4/3 · (√2 − 1).
        const double Kappa = 0.5522847498307936;
        double k = radius * Kappa;

        return new GraphicsPath()
            .MoveTo(frame.At(centreX, centreY - radius))
            .CubicTo(
                frame.At(centreX + k, centreY - radius),
                frame.At(centreX + radius, centreY - k),
                frame.At(centreX + radius, centreY))
            .CubicTo(
                frame.At(centreX + radius, centreY + k),
                frame.At(centreX + k, centreY + radius),
                frame.At(centreX, centreY + radius))
            .CubicTo(
                frame.At(centreX - k, centreY + radius),
                frame.At(centreX - radius, centreY + k),
                frame.At(centreX - radius, centreY))
            .CubicTo(
                frame.At(centreX - radius, centreY - k),
                frame.At(centreX - k, centreY - radius),
                frame.At(centreX, centreY - radius))
            .Close();
    }

    /// <summary>Maps the assets' own 16 × 16 space onto the box the icon is drawn in.</summary>
    private readonly struct Frame(DocRect box)
    {
        private readonly double _scaleX = box.Width.Emu / Box;
        private readonly double _scaleY = box.Height.Emu / Box;

        /// <summary>One point of the asset's coordinate space, on the page.</summary>
        public DocPoint At(double x, double y)
            => new(box.X + Length.FromEmu((long)Math.Round(x * _scaleX)),
                   box.Y + Length.FromEmu((long)Math.Round(y * _scaleY)));

        /// <summary>A length of the asset's coordinate space, on the page.</summary>
        /// <remarks>
        /// Along the narrower axis, so a stroke never grows past the box; the two scales are
        /// equal in practice because <c>drawIconSets</c> keeps the bitmap's own 1:1 ratio.
        /// </remarks>
        public Length Units(double value)
            => Length.FromEmu((long)Math.Round(value * Math.Min(_scaleX, _scaleY)));
    }
}
