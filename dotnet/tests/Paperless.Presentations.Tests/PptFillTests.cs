using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.Presentations.Layout;
using Paperless.Presentations.MsBinary;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// The five things an Escher <c>fillType</c> can mean, and which way round a shade runs.
/// </summary>
/// <remarks>
/// <para>
/// A binary PowerPoint deck states its whole design in one background shape on its master, and
/// resolving only <c>mso_fillSolid</c> reads that shape's <c>fillColor</c> whatever the fill
/// actually is — which on a picture fill is whatever colour the writer happened to leave in the
/// property. Measured on the corpus: fifteen of the fifty-one PPT decks fill a page background
/// with something that is not a solid colour, and twelve fill a shape with a shade.
/// </para>
/// <para>
/// The subtle part is the colour order. <c>ImportGradientColor</c>
/// (<c>filter/source/msfilter/msdffimp.cxx:2840</c>) builds a parity out of four independent
/// conditions and swaps <c>fillColor</c> with <c>fillBackColor</c> when it comes out odd, so a
/// gradient read without it is not merely mispositioned — it is inside out.
/// </para>
/// </remarks>
public class PptFillTests
{
    private static readonly DocRect Box =
        new(Length.Zero, Length.Zero, Length.FromEmu(1_000_000), Length.FromEmu(500_000));

    // ---------------------------------------------------------------- the resolver itself

    [Fact]
    public void AShapeThatIsNotFilledHasNoPaint()
    {
        Resolve(Table((PptFills.FillType, PptFills.Solid), (EscherPropertyIds.FillColour, 0xFF0000)),
                filled: false)
            .ShouldBeNull();
    }

    [Fact]
    public void ASolidFillIsTheForegroundColour()
    {
        // Escher orders its literal colours blue-green-red, so 0x0000FF is red.
        Resolve(Table((PptFills.FillType, PptFills.Solid), (EscherPropertyIds.FillColour, 0x0000FF)))
            .ShouldBeOfType<SolidPaint>()
            .Colour.ShouldBe(Colour.FromRgb(0xFF0000));
    }

    [Fact]
    public void AFillThatAsksForTheBackgroundPaintsNothing()
    {
        // mso_fillBackground means "show the page through me". The page's own background is
        // already behind the shape, so the shape draws no fill at all.
        Resolve(Table((PptFills.FillType, PptFills.UseBackground),
                      (EscherPropertyIds.FillColour, 0x0000FF)))
            .ShouldBeNull();
    }

    [Fact]
    public void APictureFillIsStretchedOnceAcrossTheBox()
    {
        BitmapPaint paint = Resolve(
                Table((PptFills.FillType, PptFills.Picture), (PptFills.FillBlip, 1)),
                picture: Picture)
            .ShouldBeOfType<BitmapPaint>();

        paint.Stretch.ShouldBeTrue();
        paint.TileSize.ShouldBe(Box.Size);
        paint.Image.ShouldBeSameAs(Picture);
    }

    [Fact]
    public void ABitmapFillWithNoBlipIsNotApproximatedByAColour()
    {
        // Falling back to fillColor here is exactly the defect this suite exists for: it paints
        // a flat rectangle in a colour the file never asks anyone to draw.
        Resolve(Table((PptFills.FillType, PptFills.Picture),
                      (EscherPropertyIds.FillColour, 0x0000FF)))
            .ShouldBeNull();
    }

    [Fact]
    public void ATextureFillTilesAtTheStatedSize()
    {
        BitmapPaint paint = Resolve(
                Table((PptFills.FillType, PptFills.Texture),
                      (PptFills.FillBlip, 1),
                      (PptFills.FillWidth, 90_000),
                      (PptFills.FillHeight, 45_000)),
                picture: Picture)
            .ShouldBeOfType<BitmapPaint>();

        paint.Stretch.ShouldBeFalse();
        paint.TileSize.Width.Emu.ShouldBe(90_000);
        paint.TileSize.Height.Emu.ShouldBe(45_000);
    }

    // ------------------------------------------------------------------- which way a shade runs

    [Fact]
    public void AShadeWithNoAngleAndNoFocusRunsFromTheBackColourAtTheTop()
    {
        // Both swaps fire — a non-negative angle and a zero focus — so the parity is even and the
        // colours stay as stated. A BGradient's *first* stop is its back colour, so fillBackColor
        // is what the top of the box gets. This is what LibreOffice's own PPT export of
        // ppt-page-fill.fodp writes, and its reference rendering draws white at the top.
        GradientPaint paint = Resolve(
                Table((PptFills.FillType, PptFills.ShadeScale),
                      (EscherPropertyIds.FillColour, 0x0000FF),      // red
                      (PptFills.FillBackColour, 0x00FF00)))          // green
            .ShouldBeOfType<GradientPaint>();

        paint.Kind.ShouldBe(GradientKind.Linear);
        paint.Stops[0].Colour.ShouldBe(Colour.FromRgb(0x00FF00));
        paint.Stops[^1].Colour.ShouldBe(Colour.FromRgb(0xFF0000));

        // Angle zero runs straight down the box, so stop 0 is at the top.
        paint.Start.Y.Emu.ShouldBeLessThan(paint.End.Y.Emu);
        paint.Start.X.Emu.ShouldBe(paint.End.X.Emu);
    }

    [Fact]
    public void AFocusAwayFromZeroLeavesTheColoursTheOtherWayRound()
    {
        // Only the angle's swap fires, so the parity is odd and the two colours change places.
        // This is the case the corpus's gradient backgrounds are in — every one of them states a
        // focus of 100 — and getting it wrong renders the deck inside out rather than merely
        // misaligned.
        GradientPaint paint = Resolve(
                Table((PptFills.FillType, PptFills.ShadeScale),
                      (EscherPropertyIds.FillColour, 0x0000FF),
                      (PptFills.FillBackColour, 0x00FF00),
                      (PptFills.FillFocus, 100)))
            .ShouldBeOfType<GradientPaint>();

        paint.Stops[0].Colour.ShouldBe(Colour.FromRgb(0xFF0000));
        paint.Stops[^1].Colour.ShouldBe(Colour.FromRgb(0x00FF00));
    }

    [Fact]
    public void AFocusNearTheMiddleMakesTheShadeAxial()
    {
        // 40 to 60 is LibreOffice's band for "measured from the centre outwards", which it draws
        // as three stops on an ordinary ramp rather than as a kind of its own.
        GradientPaint paint = Resolve(
                Table((PptFills.FillType, PptFills.ShadeScale),
                      (EscherPropertyIds.FillColour, 0x0000FF),
                      (PptFills.FillBackColour, 0x00FF00),
                      (PptFills.FillFocus, 50)))
            .ShouldBeOfType<GradientPaint>();

        paint.Stops.Count.ShouldBe(3);
        paint.Stops[0].Colour.ShouldBe(paint.Stops[^1].Colour);
        paint.Stops[1].Offset.ShouldBe(0.5);
    }

    [Fact]
    public void AHalfTurnTurnsTheRampUpsideDown()
    {
        // fillAngle is 16.16 fixed-point degrees; a negative one is the only way the angle's own
        // swap does *not* fire, which is what LibreOffice's own PPT export writes for a gradient
        // it has rotated.
        GradientPaint paint = Resolve(
                Table((PptFills.FillType, PptFills.ShadeScale),
                      (EscherPropertyIds.FillColour, 0x0000FF),
                      (PptFills.FillBackColour, 0x00FF00),
                      (PptFills.FillAngle, unchecked((uint)(-180 << 16)))))
            .ShouldBeOfType<GradientPaint>();

        // Stop 0 is now at the bottom.
        paint.Start.Y.Emu.ShouldBeGreaterThan(paint.End.Y.Emu);
    }

    [Fact]
    public void AShapeShadedFillConvergesOnTheCentreOfTheBox()
    {
        GradientPaint paint = Resolve(
                Table((PptFills.FillType, PptFills.ShadeShape),
                      (EscherPropertyIds.FillColour, 0x0000FF),
                      (PptFills.FillBackColour, 0x00FF00)))
            .ShouldBeOfType<GradientPaint>();

        paint.Kind.ShouldBe(GradientKind.Rectangular);
        paint.Start.X.Emu.ShouldBe(Box.Width.Emu / 2);
        paint.Start.Y.Emu.ShouldBe(Box.Height.Emu / 2);
    }

    // ---------------------------------------------------------------------- end to end

    /// <summary>
    /// <c>ppt-page-fill.ppt</c> is <c>ppt-page-fill.fodp</c> converted by LibreOffice: one page
    /// shaded white at the top to <c>#3366CC</c> at the bottom, carrying one rectangle shaded
    /// green at the top to red at the bottom. Those two directions are LibreOffice 24.2.7.2's own
    /// rendering of the converted file, not the ODF it came from — the export turns the shape's
    /// stated quarter-turn into a half-turn, and the reference follows the export.
    /// </summary>
    [Fact]
    public void APageShadedInTheFileIsShadedOnThePage()
    {
        LaidOutSlide slide = FirstSlide("ppt-page-fill.ppt");

        GradientPaint background = slide.Background.ShouldBeOfType<GradientPaint>();
        background.Stops[0].Colour.ShouldBe(Colour.White);
        background.Stops[^1].Colour.ShouldBe(Colour.FromRgb(0x3366CC));
        background.Start.Y.Emu.ShouldBeLessThan(background.End.Y.Emu);
    }

    [Fact]
    public void AShapeShadedInTheFileIsShadedOnThePage()
    {
        LaidOutSlide slide = FirstSlide("ppt-page-fill.ppt");

        GradientPaint fill = slide.Shapes
            .Select(shape => shape.Fill)
            .OfType<GradientPaint>()
            .ShouldHaveSingleItem();

        // Red at the bottom, green at the top: stop 0 is the red one and it sits lower.
        fill.Stops[0].Colour.ShouldBe(Colour.FromRgb(0xFF0000));
        fill.Stops[^1].Colour.ShouldBe(Colour.FromRgb(0x00FF00));
        fill.Start.Y.Emu.ShouldBeGreaterThan(fill.End.Y.Emu);
    }

    // ------------------------------------------------------------------------ opacity

    /// <summary>
    /// A shape saying nothing about opacity is opaque, not invisible.
    /// </summary>
    /// <remarks>
    /// <c>ApplyFillAttributes</c> initialises <c>dTrans</c> to 1.0 and only overwrites it inside
    /// <c>IsProperty(DFF_Prop_fillOpacity)</c> (<c>msdffimp.cxx:1365-1376</c>), so the absent
    /// case is the whole corpus and getting it wrong empties every page.
    /// </remarks>
    [Fact]
    public void AFillStatingNoOpacityIsOpaque()
    {
        Resolve(Table((PptFills.FillType, PptFills.Solid),
                      (EscherPropertyIds.FillColour, 0x0000FF)))
            .ShouldBeOfType<SolidPaint>()
            .Colour.A.ShouldBe((byte)255);
    }

    [Fact]
    public void ASolidFillCarriesItsOpacity()
    {
        // 0x8000 is a half in 16.16 fixed point.
        Resolve(Table((PptFills.FillType, PptFills.Solid),
                      (EscherPropertyIds.FillColour, 0x0000FF),
                      (PptFills.FillOpacity, 0x8000)))
            .ShouldBeOfType<SolidPaint>()
            .Colour.ShouldBe(Colour.FromRgb(0xFF0000).WithAlpha(128));
    }

    [Fact]
    public void AShadeCarriesAnOpacityAtEachEnd()
    {
        // 0xC28F and 0x428F are what LibreOffice's own PPT export writes for a 25%-to-75%
        // ODF transparency gradient, and are the values in ppt-fill-opacity.ppt below.
        IReadOnlyList<GradientStop> stops = Resolve(Table(
                (PptFills.FillType, PptFills.Shade),
                (PptFills.FillOpacity, 0xC28F),
                (EscherPropertyIds.FillColour, 0x0000FF),
                (PptFills.FillBackColour, 0x00FF00),
                (PptFills.FillBackOpacity, 0x428F)))
            .ShouldBeOfType<GradientPaint>()
            .Stops;

        stops.Select(stop => stop.Colour.A).ShouldBe([(byte)67, (byte)194]);
    }

    /// <summary>
    /// <c>ppt-fill-opacity.ppt</c> is <c>ppt-fill-opacity.fodp</c> converted by LibreOffice:
    /// a #0066ff rectangle at 50% opacity, an opaque one beside it, and a red-to-green shade
    /// under a 25%-to-75% transparency ramp.
    /// </summary>
    /// <remarks>
    /// The expectations are read out of LibreOffice's own reading of the converted file rather
    /// than out of the ODF that produced it. Round-tripping it back to flat ODF gives
    /// <c>draw:opacity="50%"</c> on the first rectangle, nothing on the second, and
    /// <c>&lt;draw:opacity … draw:start="26%" draw:end="76%"/&gt;</c> on the shade — the 25 and
    /// 75 come back as 26 and 76 because the export quantises them to 16.16 and the import
    /// rounds them back, which is exactly the sort of thing that makes the source document the
    /// wrong authority.
    /// </remarks>
    [Fact]
    public void OpacityStatedInTheFileReachesThePage()
    {
        LaidOutSlide slide = FirstSlide("ppt-fill-opacity.ppt");

        List<byte> solids = slide.Shapes
            .Select(shape => shape.Fill)
            .OfType<SolidPaint>()
            .Where(paint => paint.Colour.WithAlpha(255) == Colour.FromRgb(0x0066FF))
            .Select(paint => paint.Colour.A)
            .Order()
            .ToList();

        solids.ShouldBe([(byte)128, (byte)255]);

        slide.Shapes
            .Select(shape => shape.Fill)
            .OfType<GradientPaint>()
            .ShouldHaveSingleItem()
            .Stops.Select(stop => stop.Colour.A)
            .Order()
            .ShouldBe([(byte)67, (byte)194]);
    }

    // -------------------------------------------------- colours named indirectly

    /// <summary>
    /// A colour word whose top byte carries <c>0x10</c> names another property and a function.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>0x104301F0</c> is the second colour of the master background of
    /// <c>slides/batch-014/ppt/ws_prod-g-doc-Events-2008-February-5-NATO-activities.ppt</c>:
    /// index <c>0xF0</c> is "use <c>fillColor</c>", function bits <c>1</c> are "darken by the
    /// parameter", and the parameter is <c>0x43</c>. Over that deck's <c>fillColor</c> of
    /// <c>0x00771531</c> — <c>#311577</c>, since Escher orders its literals blue-green-red —
    /// each channel becomes <c>67 × c ÷ 256</c>, which is <c>#0C051F</c>.
    /// </para>
    /// <para>
    /// That is exactly what LibreOffice 24.2.7.2's flat-ODF export of the deck states:
    /// <c>draw:start-color="#0c051f" draw:end-color="#311577"</c>. Answering null instead —
    /// which the whole <c>0x10</c> family used to do, on the grounds that a headless renderer has
    /// no desktop theme — fell back to white and drew all fourteen of its pages as a pale
    /// gradient where the reference draws a near-black one.
    /// </para>
    /// </remarks>
    [Fact]
    public void AColourNamingAnotherPropertyIsResolvedAndThenDarkened()
    {
        GradientPaint fill = Resolve(Table(
                (PptFills.FillType, PptFills.ShadeScale),
                (EscherPropertyIds.FillColour, 0x00771531),
                (PptFills.FillBackColour, 0x104301F0)))
            .ShouldBeOfType<GradientPaint>();

        Colour[] colours = [.. fill.Stops.Select(stop => stop.Colour)];
        colours.ShouldContain(Colour.FromRgb(0x0C051F));
        colours.ShouldContain(Colour.FromRgb(0x311577));
    }

    [Fact]
    public void AColourNamingADesktopColourIsStillLeftUnresolved()
    {
        // Index 0x12 is mso_syscolorWindow, which a headless renderer has no way to answer.
        // Nothing in the 51-deck ppt corpus uses one; leaving it null keeps the shape unfilled
        // rather than inventing a colour no file states.
        Resolve(Table(
                (PptFills.FillType, PptFills.Solid),
                (EscherPropertyIds.FillColour, 0x10000012)))
            .ShouldBeNull();
    }

    /// <summary>
    /// The same indirection end to end, on a file LibreOffice has been asked to read.
    /// </summary>
    /// <remarks>
    /// <c>ppt-derived-colour.ppt</c> is <c>shape-geometry-ppt.ppt</c> with <strong>four bytes
    /// changed</strong>: one slide shape's <c>fillColor</c>, which was the literal
    /// <c>0x000000C0</c>, is now <c>0x104301F5</c> — "take <c>fillBackColor</c> and darken it by
    /// 67/256". That shape's <c>fillBackColor</c> is <c>0x00FFFF3F</c>, which is <c>#3FFFFF</c>,
    /// and darkening it gives <c>#104242</c>.
    /// <para>
    /// LibreOffice 24.2.7.2's flat-ODF export of the patched file carries
    /// <c>draw:fill-color="#104242"</c> and its export of the unpatched one carries no such
    /// colour anywhere, which is what makes this a test of the reference's reading rather than
    /// of my arithmetic. A hand-written binary fixture could not have settled that.
    /// </para>
    /// </remarks>
    [Fact]
    public void AShapeInAFileTakesTheColourTheReferenceGivesIt()
    {
        FirstSlide("ppt-derived-colour.ppt").Shapes
            .Select(shape => shape.Fill)
            .OfType<SolidPaint>()
            .Select(paint => paint.Colour)
            .ShouldContain(Colour.FromRgb(0x104242));
    }

    [Fact]
    public void AColourNamingItselfDoesNotRecurse()
    {
        // 0xF5 is "use fillBackColor", stated *as* fillBackColor. LibreOffice guards the same
        // loop; without a guard this is a stack overflow on a malformed file rather than a
        // wrong colour.
        Resolve(Table(
                (PptFills.FillType, PptFills.Shade),
                (EscherPropertyIds.FillColour, 0x00000000),
                (PptFills.FillBackColour, 0x100001F5)))
            .ShouldBeOfType<GradientPaint>()
            .Stops.Select(stop => stop.Colour)
            .ShouldContain(Colour.White);
    }

    // ------------------------------------------------------------------------- fixtures

    /// <summary>A one-pixel PNG: enough for a fill to have something to draw.</summary>
    private static readonly RasterImage Picture = RasterImage.Encoded(
        new byte[]
        {
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
            0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
            0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
            0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xDE,
        },
        "image/png");

    private static Paint? Resolve(
        EscherPropertyTable properties, bool filled = true, RasterImage? picture = null)
        => PptFills.Resolve(
            properties, filled, PptColourScheme.Default, Box, AffineTransform.Identity, picture);

    private static EscherPropertyTable Table(params (ushort Id, uint Value)[] entries)
    {
        byte[] content = new byte[entries.Length * 6];
        for (int i = 0; i < entries.Length; i++)
        {
            content[i * 6] = (byte)entries[i].Id;
            content[(i * 6) + 1] = (byte)(entries[i].Id >> 8);
            content[(i * 6) + 2] = (byte)entries[i].Value;
            content[(i * 6) + 3] = (byte)(entries[i].Value >> 8);
            content[(i * 6) + 4] = (byte)(entries[i].Value >> 16);
            content[(i * 6) + 5] = (byte)(entries[i].Value >> 24);
        }

        return EscherPropertyTable.Read(content, entries.Length);
    }

    private static LaidOutSlide FirstSlide(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[0];
    }
}
