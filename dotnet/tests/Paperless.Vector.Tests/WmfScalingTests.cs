using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Vector.Metafiles;
using Paperless.Vector.Wmf;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// How big a WMF says it is, and where its logical coordinates land.
/// </summary>
/// <remarks>
/// The TODO calls mapping-mode and window/viewport arithmetic "the most common source of
/// wrongly-scaled or mirrored output", so these are the tests that matter most. Each one names
/// a millimetre, because a scale error that is a pure factor is invisible in any assertion that
/// compares two decoded values with each other.
/// </remarks>
public class WmfScalingTests
{
    [Fact]
    public void ThePlaceableHeadersResolutionSetsThePhysicalSize()
    {
        // 1440 units to the inch and a 2880-unit window: two inches, 50.8 mm.
        VectorImage image = Decode(new WmfBuilder { UnitsPerInch = 1440, Bounds = (0, 0, 2880, 1440) }
            .WindowOrigin(0, 0)
            .WindowExtent(2880, 1440)
            .Rectangle(0, 0, 2880, 1440));

        image.IntrinsicSize.Width.Millimetres.ShouldBe(50.8, 0.05);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(25.4, 0.05);
    }

    [Fact]
    public void TheViewBoxAndTheIntrinsicSizeAgreeForWmf()
    {
        // They coincide here because every coordinate is mapped to 1/100 mm as it is read, so
        // the display list is already in physical units. Stating it as a test because for EMF
        // they will not, and the seam keeps them apart for that reason.
        VectorImage image = Decode(Square().Rectangle(0, 0, 1000, 1000));

        image.ViewBox.Width.ShouldBe(image.IntrinsicSize.Width);
        image.ViewBox.Height.ShouldBe(image.IntrinsicSize.Height);
        image.ViewBox.X.ShouldBe(Length.Zero);
        image.ViewBox.Y.ShouldBe(Length.Zero);
    }

    [Fact]
    public void AWindowSmallerThanTheViewportMagnifiesEverything()
    {
        // The same rectangle in a window half as wide covers twice the picture.
        Recorder wide = Replay(Square().Rectangle(0, 0, 1270, 1270));
        Recorder narrow = Replay(new WmfBuilder { UnitsPerInch = 2540, Bounds = (0, 0, 2540, 2540) }
            .WindowOrigin(0, 0)
            .WindowExtent(1270, 1270)
            .Rectangle(0, 0, 1270, 1270));

        wide.Fills[0].Bounds.Width.Millimetres.ShouldBe(12.7, 0.05);
        narrow.Fills[0].Bounds.Width.Millimetres.ShouldBe(25.4, 0.05);
    }

    [Fact]
    public void TheWindowOriginMovesTheDrawingRatherThanScalingIt()
    {
        Recorder recorder = Replay(new WmfBuilder { UnitsPerInch = 2540, Bounds = (500, 500, 3040, 3040) }
            .WindowOrigin(500, 500)
            .WindowExtent(2540, 2540)
            .Rectangle(500, 500, 1500, 1500));

        // The window origin is subtracted, so a shape at the window's own corner draws at zero.
        recorder.Fills[0].Bounds.X.Millimetres.ShouldBe(0, 0.02);
        recorder.Fills[0].Bounds.Width.Millimetres.ShouldBe(10.0, 0.02);
    }

    [Theory]
    [InlineData(2, 100.0)]     // MM_LOMETRIC: 0.1 mm units, so 1000 units is 100 mm
    [InlineData(3, 10.0)]      // MM_HIMETRIC: 0.01 mm units
    [InlineData(4, 254.0)]     // MM_LOENGLISH: 0.01 in units
    [InlineData(5, 25.4)]      // MM_HIENGLISH: 0.001 in units
    [InlineData(6, 17.64)]     // MM_TWIPS
    public void EachMetricMappingModeHasItsOwnFixedScale(short mode, double millimetres)
    {
        Recorder recorder = Replay(Square().MapMode(mode).Rectangle(0, 0, 1000, 1000));

        recorder.Fills[0].Bounds.Width.Millimetres.ShouldBe(millimetres, millimetres * 0.001);
    }

    [Theory]
    [InlineData(2)]
    [InlineData(3)]
    [InlineData(4)]
    [InlineData(5)]
    [InlineData(6)]
    public void EveryMetricMappingModeMeasuresUpwards(short mode)
    {
        // GDI's metric modes put y upwards, so a shape below the origin in logical space is
        // above it on the page. Getting this wrong draws the picture upside down, which is the
        // failure the TODO warns about and which no scale assertion would catch.
        Recorder recorder = Replay(Square().MapMode(mode).Rectangle(0, 100, 1000, 1000));

        recorder.Fills[0].Bounds.Bottom.ShouldBeLessThanOrEqualTo(Length.Zero);
    }

    [Fact]
    public void ABareMetafileIsMeasuredFromWhatItDraws()
    {
        // No placeable header at all: the size has to come from the records. The window records
        // are the first thing tried, so this file states none and is measured from its geometry.
        WmfBuilder builder = new() { Placeable = false };
        VectorImage image = Decode(builder.Rectangle(0, 0, 300, 200).Rectangle(100, 100, 500, 400));

        image.IsEmpty.ShouldBeFalse();

        // 501 units wide at the assumed 96 units to the inch: the +1 is GDI's inclusive
        // rectangle, and 96 is what LibreOffice assumes when a file states nothing.
        image.IntrinsicSize.Width.Millimetres.ShouldBe(501.0 / 96 * 25.4, 0.2);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(401.0 / 96 * 25.4, 0.2);
    }

    [Fact]
    public void ABareMetafileTooWideToBeCredibleIsScaledDown()
    {
        // 4000 logical units at 96 to the inch would be a metre across. LibreOffice raises the
        // assumed resolution instead, so the picture stays a plausible size.
        WmfBuilder builder = new() { Placeable = false };
        VectorImage image = Decode(builder.Rectangle(0, 0, 4000, 2000));

        image.IntrinsicSize.Width.Millimetres.ShouldBeLessThan(300);
        image.IntrinsicSize.Width.Millimetres.ShouldBeGreaterThan(200);
    }

    [Fact]
    public void AnAnisotropicPlaceableIgnoresAResolutionLargerThanItsWindow()
    {
        // Undocumented, and load-bearing: honouring 1440 units to the inch for a 100-unit window
        // would make the picture 1.8 mm across. Other office suites ignore it, and so does
        // LibreOffice (wmfreader.cxx:2142-2156).
        VectorImage image = Decode(new WmfBuilder { UnitsPerInch = 1440, Bounds = (0, 0, 100, 100) }
            .WindowOrigin(0, 0)
            .WindowExtent(100, 100)
            .Rectangle(0, 0, 100, 100));

        image.IntrinsicSize.Width.Millimetres.ShouldBe(25.4, 0.1);
    }

    [Fact]
    public void ScaleWindowExtHalvesTheCoordinateSpaceAndDoublesTheDrawing()
    {
        // ScaleWindowExt takes numerator/denominator pairs as y-denominator, y-numerator,
        // x-denominator, x-numerator.
        Recorder recorder = Replay(Square()
            .Record(WmfFunction.Rectangle, 1000, 1000, 0, 0)
            .Record((WmfFunction)0x0410, 2, 1, 2, 1)
            .Record(WmfFunction.Rectangle, 1000, 1000, 0, 0));

        recorder.Fills.Count.ShouldBe(2);
        recorder.Fills[1].Bounds.Width.Millimetres.ShouldBe(recorder.Fills[0].Bounds.Width.Millimetres * 2, 0.02);
    }

    [Fact]
    public void MappingIsExactlyLibreOfficesArithmetic()
    {
        // The mapping is testable on its own, which is the point of keeping it out of the reader.
        MetafileMapping mapping = new();
        mapping.SetMode(MappingMode.Anisotropic);
        mapping.SetWindowExtent(1000, 500);
        mapping.SetViewportExtent(10000, 10000);
        mapping.WindowOriginX = 100;
        mapping.WindowOriginY = 50;

        (double x, double y) = mapping.MapPointMm100(600, 300);

        // (600-100)/1000*10000 = 5000, and (300-50)/500*10000 = 5000: the axes scale
        // independently, which is what anisotropic means and what makes it the mode that
        // stretches pictures when it is mishandled.
        x.ShouldBe(5000, 0.001);
        y.ShouldBe(5000, 0.001);
    }

    private static WmfBuilder Square()
        => new WmfBuilder { UnitsPerInch = 2540, Bounds = (0, 0, 2540, 2540) }
            .WindowOrigin(0, 0)
            .WindowExtent(2540, 2540);

    private static VectorImage Decode(WmfBuilder builder) => new WmfImageDecoder().Decode(builder.Build());

    private static Recorder Replay(WmfBuilder builder)
    {
        VectorImage image = Decode(builder);
        image.IsEmpty.ShouldBeFalse();

        Recorder recorder = new();
        image.Content.Replay(recorder);
        return recorder;
    }
}
