using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.Metafiles;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The device context, object table and clip that WMF, EMF and EMF+ all sit on.
/// </summary>
/// <remarks>
/// Tested on their own rather than only through a WMF, because they are the part with leverage:
/// the EMF reader will drive exactly these types, and a bug in the handle-reuse rule shows up as
/// wrong colours in a file whose records are all read correctly. A test that goes through a
/// whole metafile could not tell the two apart.
/// </remarks>
public class MetafileGroundworkTests
{
    [Fact]
    public void ACreatedObjectTakesTheLowestFreeHandle()
    {
        GraphicsObjectTable table = new();

        table.Add(MetafilePen.Default).ShouldBe(0);
        table.Add(MetafileBrush.Default).ShouldBe(1);
        table.Add(MetafileFont.Default).ShouldBe(2);

        table.Delete(1);

        // Not 3. Real files delete aggressively because GDI limited a device context to a few
        // hundred simultaneous objects, and a decoder that appends instead assigns different
        // handles from the producer from this point on.
        table.Add(MetafilePen.None).ShouldBe(1);
        table.Add(MetafileBrush.None).ShouldBe(3);
    }

    [Fact]
    public void DeletingAHandleTwiceOrOneThatWasNeverCreatedIsHarmless()
    {
        GraphicsObjectTable table = new();
        table.Add(MetafilePen.Default);

        table.Delete(0);
        table.Delete(0);
        table.Delete(9999);
        table.Delete(-1);

        table[0].ShouldBeNull();
        table[9999].ShouldBeNull();
        table.Add(MetafileBrush.Default).ShouldBe(0);
    }

    [Fact]
    public void AStatedHandleGrowsTheTableWithoutShiftingAnything()
    {
        // EMF's create records name their own handle and routinely create object 40 without ever
        // having created 0 to 39.
        GraphicsObjectTable table = new();
        table.Set(40, MetafileBrush.Default);

        table[40].ShouldBe(MetafileBrush.Default);
        table[39].ShouldBeNull();
        table.Add(MetafilePen.Default).ShouldBe(0);
    }

    [Fact]
    public void AHandleBeyondTheSixteenBitLimitIsRefusedRatherThanAllocated()
    {
        // A malformed 32-bit EMF handle would otherwise ask for a table of four billion slots
        // before a single record had drawn anything.
        GraphicsObjectTable table = new();
        table.Set(int.MaxValue, MetafileBrush.Default);

        table.Count.ShouldBe(0);
    }

    [Fact]
    public void SavingAndRestoringCarriesTheWholeContextButNotTheObjects()
    {
        MetafileDeviceContext context = new();
        context.Objects.Add(MetafilePen.Default);
        context.TextColour = Colour.Black;
        context.Mapping.SetMode(MappingMode.Anisotropic);
        context.Mapping.SetWindowExtent(100, 100);

        context.Save();

        context.TextColour = Colour.White;
        context.Mapping.SetWindowExtent(500, 500);
        context.Objects.Add(MetafileBrush.Default);

        context.Restore();

        context.TextColour.ShouldBe(Colour.Black);
        context.Mapping.WindowExtentX.ShouldBe(100);

        // The object created inside the saved block survives: GDI objects belong to the device,
        // not to the saved state, so a RestoreDC does not un-create a brush.
        context.Objects[1].ShouldBe(MetafileBrush.Default);
    }

    [Fact]
    public void RestoreWithAPositiveArgumentNamesAStateRatherThanCountingBack()
    {
        // WMF writes both spellings, sometimes in one file. Treating a positive value as a count
        // discards states a later record still expects.
        MetafileDeviceContext context = new();

        context.TextColour = new Colour(1, 1, 1);
        context.Save();                             // state 0
        context.TextColour = new Colour(2, 2, 2);
        context.Save();                             // state 1
        context.TextColour = new Colour(3, 3, 3);
        context.Save();                             // state 2
        context.TextColour = new Colour(4, 4, 4);

        context.Restore(1);

        context.TextColour.ShouldBe(new Colour(2, 2, 2));
        context.SaveDepth.ShouldBe(1);
    }

    [Fact]
    public void RestoreWithoutASaveIsIgnored()
    {
        MetafileDeviceContext context = new();
        context.TextColour = Colour.White;

        context.Restore();
        context.Restore(-5);
        context.Restore(0);
        context.Restore(3);

        context.TextColour.ShouldBe(Colour.White);
        context.SaveDepth.ShouldBe(0);
    }

    [Fact]
    public void TheFillColourFallsBackToTheBackgroundUntilABrushIsSelected()
    {
        // GDI fills with the background colour until the file selects a brush of its own. A
        // decoder that starts with the default white brush paints white boxes over everything a
        // background-coloured shape was meant to reveal (i57205).
        MetafileDeviceContext context = new() { BackgroundColour = new Colour(9, 9, 9) };

        context.EffectiveBrush.Colour.ShouldBe(new Colour(9, 9, 9));

        context.Brush = new MetafileBrush(BrushStyle.Solid, Colour.White);
        context.IsBrushSelected = true;

        context.EffectiveBrush.Colour.ShouldBe(Colour.White);
    }

    [Fact]
    public void ATransparentBackgroundMeansNoFillUntilABrushIsSelected()
    {
        MetafileDeviceContext context = new() { BackgroundMode = BackgroundMode.Transparent };

        context.EffectiveBrush.IsNull.ShouldBeTrue();
    }

    [Fact]
    public void ArbitraryClipShapesAreAListOfIntersectionsSoTheSinkDoesTheArithmetic()
    {
        MetafileClip clip = new();
        clip.Intersect(GraphicsPath.Rectangle(new DocRect(
            Length.Zero, Length.Zero, Length.FromMillimetres(10), Length.FromMillimetres(10))));
        clip.Intersect(GraphicsPath.Rectangle(new DocRect(
            Length.FromMillimetres(5), Length.Zero, Length.FromMillimetres(10), Length.FromMillimetres(10))));

        clip.Count.ShouldBe(2);

        Recorder recorder = new();
        recorder.Save();
        clip.Apply(recorder);

        // Two clip calls, not one intersected shape: IDrawingSink.ClipPath intersects, so
        // replaying the list *is* the intersection and no path arithmetic is needed.
        recorder.Clips.Count.ShouldBe(2);
    }

    [Fact]
    public void RectangularClipsAreIntersectedExactlyRatherThanStacked()
    {
        MetafileClip clip = new();
        clip.Intersect(new DocRect(Length.Zero, Length.Zero, Length.FromMillimetres(10), Length.FromMillimetres(10)));
        clip.Intersect(new DocRect(Length.FromMillimetres(5), Length.Zero, Length.FromMillimetres(10), Length.FromMillimetres(10)));

        // One rectangle, computed: a rectangle set is closed under intersection, and holding it
        // that way is what lets the *next* record subtract from it exactly.
        clip.Count.ShouldBe(1);

        Recorder recorder = new();
        recorder.Save();
        clip.Apply(recorder);

        recorder.Clips.ShouldHaveSingleItem();
        recorder.Clips[0].X.Millimetres.ShouldBe(5.0, 0.001);
        recorder.Clips[0].Width.Millimetres.ShouldBe(5.0, 0.001);
    }

    [Fact]
    public void ReplacingTheClipDiscardsEverythingBeforeIt()
    {
        MetafileClip clip = new();
        clip.Intersect(new DocRect(Length.Zero, Length.Zero, Length.FromMillimetres(10), Length.FromMillimetres(10)));
        clip.Replace(GraphicsPath.Rectangle(new DocRect(
            Length.Zero, Length.Zero, Length.FromMillimetres(3), Length.FromMillimetres(3))));

        clip.Count.ShouldBe(1);
    }

    [Fact]
    public void AClonedClipIsIndependentAndComparesEqual()
    {
        MetafileClip clip = new();
        clip.Intersect(new DocRect(Length.Zero, Length.Zero, Length.FromMillimetres(10), Length.FromMillimetres(10)));

        MetafileClip copy = clip.Clone();
        copy.SameAs(clip).ShouldBeTrue();

        copy.Intersect(new DocRect(Length.Zero, Length.Zero, Length.FromMillimetres(2), Length.FromMillimetres(2)));

        clip.Count.ShouldBe(1);
        copy.SameAs(clip).ShouldBeFalse();
    }

    [Fact]
    public void AnExclusionIsRecordedAsUnsupportedRatherThanSilentlyDropped()
    {
        MetafileClip clip = new();
        clip.MarkUnsupported();

        clip.HasUnsupportedOperation.ShouldBeTrue();
        clip.Clone().HasUnsupportedOperation.ShouldBeTrue();
    }

    [Fact]
    public void APaletteIndexColourResolvesThroughTheSelectedPalette()
    {
        MetafilePalette palette = new([new Colour(10, 20, 30), new Colour(40, 50, 60)]);

        palette.Resolve(0x01000001).ShouldBe(new Colour(40, 50, 60));

        // Out of range falls back to black rather than throwing: a truncated palette record is
        // the commonest cause and the picture is still worth drawing.
        palette.Resolve(0x01000099).ShouldBe(Colour.Black);

        // Anything without the 0x01 tag is a literal BGR triple.
        palette.Resolve(0x00030201).ShouldBe(new Colour(1, 2, 3));
    }

    [Fact]
    public void APenStyleBecomesADashPatternWithTheRightDutyCycle()
    {
        Length dot = Length.FromMillimetres(1);

        MetafilePen.Dashes(PenStyle.Solid, dot).ShouldBeNull();
        MetafilePen.Dashes(PenStyle.Dot, dot)!.Count.ShouldBe(2);
        MetafilePen.Dashes(PenStyle.Dash, dot)![0].Millimetres.ShouldBe(3.0, 0.001);
        MetafilePen.Dashes(PenStyle.DashDot, dot)!.Count.ShouldBe(4);
        MetafilePen.Dashes(PenStyle.DashDotDot, dot)!.Count.ShouldBe(6);
    }

    [Fact]
    public void AnArcsEndsComeFromTheEllipsesParameterNotTheRaysAngle()
    {
        // GDI names an arc's ends by points on a ray from the centre, and for a non-circular
        // ellipse the ray's angle is not the parametric angle where it crosses. Using the ray's
        // angle puts the end of every non-circular arc in the wrong place.
        DocRect rect = new(Length.Zero, Length.Zero, Length.FromMillimetres(40), Length.FromMillimetres(10));

        // A ray at 45 degrees from the centre of a 4:1 ellipse.
        DocPoint start = new(Length.FromMillimetres(30), Length.FromMillimetres(-5));
        DocPoint end = new(Length.FromMillimetres(0), Length.FromMillimetres(5));

        GraphicsPath path = MetafileGeometry.Arc(rect, start, end, MetafileGeometry.ArcClosure.Open);

        // The arc starts where the ray crosses the ellipse, which for a 4:1 ellipse at a
        // 45-degree ray is at parameter atan2(20, 5) = 1.3258 rad — well round from 45 degrees.
        double x = path.Commands[0].Point.X.Millimetres;
        double y = path.Commands[0].Point.Y.Millimetres;

        x.ShouldBe(20 + (20 * Math.Cos(Math.Atan2(20, 5))), 0.01);
        y.ShouldBe(5 - (5 * Math.Sin(Math.Atan2(20, 5))), 0.01);
    }

    [Fact]
    public void ADibIsMeasuredAndRewrappedWithoutDecodingAPixel()
    {
        // A 2x2 24-bit DIB: a 40-byte header and two 8-byte rows (six bytes of pixels padded to
        // a four-byte boundary).
        byte[] dib = new byte[40 + 16];
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(dib, 40);
        System.Buffers.Binary.BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(4), 2);
        System.Buffers.Binary.BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(8), 2);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(12), 1);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(14), 24);

        DeviceIndependentBitmap.Result result = DeviceIndependentBitmap.Read(dib).ShouldNotBeNull();

        result.Width.ShouldBe(2);
        result.Height.ShouldBe(2);
        result.Length.ShouldBe(56);

        // Still encoded: nothing in this library has looked at a pixel, which is the whole point
        // of RasterImage.Encoded and the reason Paperless.Vector needs no codec.
        result.Image.IsDecoded.ShouldBeFalse();
        result.Image.EncodedMediaType.ShouldBe("image/bmp");
        result.Image.EncodedBytes.Span[0].ShouldBe((byte)'B');
        result.Image.EncodedBytes.Span[1].ShouldBe((byte)'M');
    }

    [Fact]
    public void ADibWithAnImplausibleHeaderIsRefused()
    {
        DeviceIndependentBitmap.Read([]).ShouldBeNull();
        DeviceIndependentBitmap.Read([1, 2, 3, 4, 5, 6, 7, 8]).ShouldBeNull();

        byte[] huge = new byte[40];
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32LittleEndian(huge, 40);
        System.Buffers.Binary.BinaryPrimitives.WriteInt32LittleEndian(huge.AsSpan(4), 1 << 20);
        System.Buffers.Binary.BinaryPrimitives.WriteInt32LittleEndian(huge.AsSpan(8), 1 << 20);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt16LittleEndian(huge.AsSpan(14), 32);

        DeviceIndependentBitmap.Read(huge).ShouldBeNull();
    }

    [Fact]
    public void ACountBeyondTheBytesAvailableIsImplausible()
    {
        MetafileBudget.Plausible(10, 4, 40).ShouldBeTrue();
        MetafileBudget.Plausible(11, 4, 40).ShouldBeFalse();
        MetafileBudget.Plausible(-1, 4, 40).ShouldBeFalse();
        MetafileBudget.Plausible(int.MaxValue, 4, 40).ShouldBeFalse();
    }
}
