using System.Buffers.Binary;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.Wmf;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The two record families that carry data rather than geometry: bitmaps and text.
/// </summary>
/// <remarks>
/// Both are where a metafile stops being a list of coordinates. A bitmap record embeds a whole
/// DIB, and a text record embeds eight-bit bytes whose meaning depends on a font selected many
/// records earlier — so both are also where a decoder is most likely to read past the end of a
/// record or to draw something in the wrong place.
/// </remarks>
public class WmfBitmapAndTextTests
{
    private const int Mm = 100;

    [Fact]
    public void AStretchDibPlacesItsBitmapWithoutDecodingIt()
    {
        Recorder recorder = Draw(Square().Raw(WmfFunction.StretchDib, StretchDib(
            sourceWidth: 4, sourceHeight: 4, sourceX: 0, sourceY: 0,
            destX: 2 * Mm, destY: 3 * Mm, destWidth: 20 * Mm, destHeight: 10 * Mm)));

        (RasterImage image, DocRect destination, _) = recorder.Images.ShouldHaveSingleItem();

        destination.X.Millimetres.ShouldBe(2.0, 0.02);
        destination.Y.Millimetres.ShouldBe(3.0, 0.02);
        destination.Width.Millimetres.ShouldBe(20.0, 0.02);
        destination.Height.Millimetres.ShouldBe(10.0, 0.02);

        // Handed on undecoded, which is what keeps a codec out of Paperless.Vector.
        image.IsDecoded.ShouldBeFalse();
        image.EncodedMediaType.ShouldBe("image/bmp");
    }

    [Fact]
    public void ASourceRectangleBecomesAScaleAndAClipRatherThanACrop()
    {
        // Cropping would need a codec. Instead the whole image is placed so that the wanted part
        // lands on the destination, and a clip hides the rest — the same picture, no decode.
        Recorder recorder = Draw(Square().Raw(WmfFunction.StretchDib, StretchDib(
            sourceWidth: 2, sourceHeight: 2, sourceX: 1, sourceY: 1,
            destX: 0, destY: 0, destWidth: 10 * Mm, destHeight: 10 * Mm)));

        DocRect destination = recorder.Images.ShouldHaveSingleItem().Destination;

        // A 2x2 window on a 4x4 bitmap doubles the drawn size, and the one-pixel offset moves it
        // up and left by half the destination.
        destination.Width.Millimetres.ShouldBe(20.0, 0.05);
        destination.X.Millimetres.ShouldBe(-5.0, 0.05);

        recorder.Clips.ShouldNotBeEmpty();
    }

    [Fact]
    public void ASetDibToDeviceDrawsAtOneToOne()
    {
        // emfio does not implement this record at all, so a document placing a scanned image
        // through it renders blank there.
        byte[] payload = new byte[18 + Dib().Length];
        BinaryPrimitives.WriteUInt16LittleEndian(payload, 0);            // colour usage
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2), 4);  // scan count
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4), 0);  // start scan
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(6), 0);   // source y
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(8), 0);   // source x
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(10), 5 * Mm);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(12), 7 * Mm);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(14), 1 * Mm);   // destination y
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(16), 2 * Mm);   // destination x
        Dib().CopyTo(payload.AsSpan(18));

        Recorder recorder = Draw(Square().Raw((WmfFunction)0x0D33, payload));

        DocRect destination = recorder.Images.ShouldHaveSingleItem().Destination;
        destination.X.Millimetres.ShouldBe(2.0, 0.02);
        destination.Width.Millimetres.ShouldBe(7.0, 0.02);
        destination.Height.Millimetres.ShouldBe(5.0, 0.02);
    }

    [Fact]
    public void ABlitWithNoBitmapAtAllIsReportedRatherThanRead()
    {
        // The two forms of the record are told apart by size alone — there is no flag — and
        // reading the bitmap that is not there is how a decoder walks off the end of a legal
        // file (wmfreader.cxx:919). A META_DIBBITBLT of exactly twelve words is the short form.
        byte[] payload = new byte[18];
        BinaryPrimitives.WriteUInt32LittleEndian(payload, 0x00CC0020);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(10), 5 * Mm);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(12), 5 * Mm);

        VectorImage image = new WmfImageDecoder().Decode(
            Square().Raw((WmfFunction)0x0940, payload).Rectangle(0, 0, Mm, Mm).Build());

        image.Diagnostics.ShouldContain(d => d.Code == "PL6033");
    }

    [Fact]
    public void ExtTextOutHonoursItsDxArrayRatherThanRemeasuring()
    {
        // The DX array is how a metafile records the result of its *own* text layout; a decoder
        // that re-measures substitutes its own and the spacing drifts from what the producer
        // laid out. Four characters at 300 units each: the run is 12 mm wide whatever the face's
        // own advances are.
        GlyphRun run = Run(Square().Font("Liberation Sans", 3 * Mm).Select(0).Raw(
            WmfFunction.ExtTextOut, ExtTextOut("IIII", 1 * Mm, 5 * Mm, [300, 300, 300, 300])));

        Length advance = Length.Zero;
        foreach (PositionedGlyph glyph in run.Glyphs) advance += glyph.Advance;

        advance.Millimetres.ShouldBe(12.0, 0.05);
    }

    [Fact]
    public void ExtTextOutWithoutADxArrayUsesTheFontsOwnAdvances()
    {
        GlyphRun run = Run(Square().Font("Liberation Sans", 3 * Mm).Select(0).Raw(
            WmfFunction.ExtTextOut, ExtTextOut("IIII", 1 * Mm, 5 * Mm, null)));

        Length advance = Length.Zero;
        foreach (PositionedGlyph glyph in run.Glyphs) advance += glyph.Advance;

        // Four narrow letters at 3 mm: nowhere near the 12 mm the previous test's array asked
        // for, which is exactly why honouring the array matters.
        advance.Millimetres.ShouldBeGreaterThan(0.5);
        advance.Millimetres.ShouldBeLessThan(8.0);
    }

    [Fact]
    public void AnOpaqueTextRectangleIsPaintedBeforeTheText()
    {
        byte[] payload = ExtTextOut("Ab", 1 * Mm, 5 * Mm, null, options: 0x0002, rect: (0, 0, 8 * Mm, 6 * Mm));

        Recorder recorder = Draw(Square().Font("Liberation Sans", 3 * Mm).Select(0).Raw(
            WmfFunction.ExtTextOut, payload));

        recorder.Fills.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(8.0, 0.02);
        recorder.Runs.ShouldNotBeEmpty();
    }

    [Fact]
    public void AClippedTextRectangleScopesTheClipToThatRecordOnly()
    {
        byte[] payload = ExtTextOut("Ab", 1 * Mm, 5 * Mm, null, options: 0x0004, rect: (0, 0, 4 * Mm, 6 * Mm));

        Recorder recorder = Draw(Square()
            .Font("Liberation Sans", 3 * Mm)
            .Select(0)
            .Raw(WmfFunction.ExtTextOut, payload)
            .Rectangle(0, 0, 10 * Mm, 10 * Mm));

        // The clip applies to the text and is gone again by the time the rectangle draws, so the
        // rectangle is recorded after a restore rather than inside the text's clip.
        recorder.Clips.Count.ShouldBe(1);
        recorder.Clips[0].Width.Millimetres.ShouldBe(4.0, 0.02);
        recorder.Fills.ShouldHaveSingleItem().Bounds.Width.Millimetres.ShouldBe(10.0, 0.02);
    }

    [Fact]
    public void ARotatedFontRotatesTheRunAboutItsOrigin()
    {
        // A chart's axis labels are the common case, and there is nowhere on a GlyphRun to put a
        // rotation — so it becomes a transform around the run's own origin, which keeps the run
        // one run and lets a PDF backend emit real text.
        Recorder recorder = Draw(Square()
            .Font("Liberation Sans", 3 * Mm, escapement: 900)
            .Select(0)
            .Record(WmfFunction.SetTextAlign, 0x0010)
            .TextOut("Up", 5 * Mm, 5 * Mm));

        recorder.Runs.ShouldHaveSingleItem();

        // The origin is the fixed point of the rotation, so it is unmoved.
        recorder.Runs[0].Origin.X.Millimetres.ShouldBe(5.0, 0.05);
        recorder.Runs[0].Origin.Y.Millimetres.ShouldBe(5.0, 0.05);
        recorder.MaxDepth.ShouldBeGreaterThanOrEqualTo(2);
    }

    [Fact]
    public void AHatchedBrushBecomesStrokedLinesClippedToTheShape()
    {
        // Paint has no hatch kind — Paperless.Core says so and resolves hatches "into a tiled
        // BitmapPaint or an explicit set of stroked lines at read time". Lines, here: a tile
        // would need a rasteriser, and lines stay resolution-independent.
        Recorder recorder = Draw(Square()
            .Record(WmfFunction.SetBkMode, 1)
            .Record(WmfFunction.CreateBrushIndirect, 2, 0x00FF, 0, 4)      // BS_HATCHED, HS_CROSS
            .Select(0)
            .Rectangle(0, 0, 10 * Mm, 10 * Mm));

        // Horizontal and vertical lines at 0.5 mm across 10 mm: about 21 of each.
        recorder.Strokes.Count.ShouldBeGreaterThan(30);
        recorder.Clips.ShouldNotBeEmpty();
        recorder.Fills.ShouldBeEmpty();
    }

    /// <summary>A 4x4 24-bit DIB, which is the smallest thing worth placing.</summary>
    private static byte[] Dib()
    {
        byte[] dib = new byte[40 + (4 * 16)];
        BinaryPrimitives.WriteUInt32LittleEndian(dib, 40);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(4), 4);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(8), 4);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(12), 1);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(14), 24);
        return dib;
    }

    /// <summary>A <c>META_STRETCHDIB</c> payload. [MS-WMF] 2.3.1.6.</summary>
    private static byte[] StretchDib(
        short sourceWidth,
        short sourceHeight,
        short sourceX,
        short sourceY,
        int destX,
        int destY,
        int destWidth,
        int destHeight)
    {
        byte[] dib = Dib();
        byte[] payload = new byte[22 + dib.Length];

        BinaryPrimitives.WriteUInt32LittleEndian(payload, 0x00CC0020);           // SRCCOPY
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4), 0);          // colour usage
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(6), sourceHeight);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(8), sourceWidth);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(10), sourceY);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(12), sourceX);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(14), (short)destHeight);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(16), (short)destWidth);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(18), (short)destY);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(20), (short)destX);
        dib.CopyTo(payload.AsSpan(22));

        return payload;
    }

    private static byte[] ExtTextOut(
        string text,
        int x,
        int y,
        int[]? advances,
        ushort options = 0,
        (int Left, int Top, int Right, int Bottom)? rect = null)
    {
        int stored = (text.Length + 1) & ~1;
        int rectangle = rect is null ? 0 : 8;
        int dx = advances is null ? 0 : advances.Length * 2;

        byte[] payload = new byte[8 + rectangle + stored + dx];

        BinaryPrimitives.WriteInt16LittleEndian(payload, (short)y);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(2), (short)x);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(4), (short)text.Length);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(6), options);

        int offset = 8;
        if (rect is { } r)
        {
            BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(offset), (short)r.Left);
            BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(offset + 2), (short)r.Top);
            BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(offset + 4), (short)r.Right);
            BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(offset + 6), (short)r.Bottom);
            offset += 8;
        }

        for (int i = 0; i < text.Length; i++) payload[offset + i] = (byte)text[i];
        offset += stored;

        if (advances is not null)
        {
            for (int i = 0; i < advances.Length; i++)
            {
                BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(offset + (i * 2)), (short)advances[i]);
            }
        }

        return payload;
    }

    private static WmfBuilder Square()
    {
        WmfBuilder builder = new() { UnitsPerInch = 2540, Bounds = (0, 0, 2540, 2540) };
        return builder.WindowOrigin(0, 0).WindowExtent(2540, 2540);
    }

    /// <summary>The one glyph run a fixture drew, taken from the display list itself.</summary>
    private static GlyphRun Run(WmfBuilder builder)
    {
        VectorImage image = new WmfImageDecoder().Decode(builder.Build());

        foreach (DrawingCommand command in image.Content.Commands)
        {
            if (command is GlyphRunCommand run) return run.Run;
        }

        throw new ShouldAssertException("the fixture drew no glyph run");
    }

    private static Recorder Draw(WmfBuilder builder)
    {
        VectorImage image = new WmfImageDecoder().Decode(builder.Build());
        image.IsEmpty.ShouldBeFalse();

        Recorder recorder = new();
        image.Content.Replay(recorder);
        return recorder;
    }
}
