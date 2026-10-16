using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Rendering.Pdf;
using XRay.Office.Rendering.Raster;
using Shouldly;
using SkiaSharp;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// A PDF is written to the stream as it is produced, and a page image as it is encoded.
/// </summary>
/// <remarks>
/// What is promised is that a large document does not have to exist in memory twice. The test for it is
/// not a memory measurement, which would be a test of the garbage collector, but an observation of
/// ordering: when the second page begins to be drawn, the first page's content is already in the
/// destination.
/// </remarks>
public sealed class PdfStreamingTests
{
    [Fact]
    public void EachPagesContentIsInTheDestinationBeforeTheNextPageIsDrawn()
    {
        using RecordingStream destination = new();
        List<long> lengthAtStart = [];

        IPage[] pages =
        [
            .. Enumerable.Range(0, 4).Select(i => (IPage)new ProbePage(i, () => lengthAtStart.Add(destination.Length))),
        ];

        new PdfRenderer().Render(new DrawnPages(pages), destination);

        lengthAtStart.Count.ShouldBe(4);
        lengthAtStart[0].ShouldBeGreaterThan(0, "the header is written before anything is drawn");
        for (int i = 1; i < lengthAtStart.Count; i++)
        {
            lengthAtStart[i].ShouldBeGreaterThan(lengthAtStart[i - 1], "the previous page's content stream was written at once");
        }
    }

    [Fact]
    public void ADestinationThatCannotSeekOrReadWorks()
    {
        using ForwardOnlyStream destination = new();

        new PdfRenderer().Render(
            new DrawnPages(new DrawnPage(DrawnPage.A4, sink => sink.FillPath(
                GraphicsPath.Rectangle(new DocRect(Length.FromInches(1), Length.FromInches(1), Length.FromInches(1), Length.FromInches(1))),
                Paint.Solid(Colour.Black)))),
            destination);

        byte[] bytes = destination.ToArray();
        PdfFile pdf = PdfFile.Parse(bytes);
        pdf.Text.ShouldStartWith("%PDF-1.7");
        pdf.Text.ShouldEndWith("%%EOF\n");

        // The offsets the cross-reference table states are where the objects really are, which is what
        // the writer could get wrong by counting bytes instead of asking a stream it cannot ask.
        string text = pdf.Text;
        int xref = text.LastIndexOf("\nxref\n", StringComparison.Ordinal) + 1;
        int startxref = int.Parse(text[(text.LastIndexOf("startxref\n", StringComparison.Ordinal) + "startxref\n".Length)..].Split('\n')[0], System.Globalization.CultureInfo.InvariantCulture);
        startxref.ShouldBe(xref);

        string[] entries = text[xref..].Split('\n').Skip(2).TakeWhile(l => l.Length == 20 || l.Length == 19).ToArray();
        for (int i = 1; i < entries.Length; i++)
        {
            int offset = int.Parse(entries[i][..10], System.Globalization.CultureInfo.InvariantCulture);
            text[offset..].ShouldStartWith($"{i} 0 obj\n");
        }
    }

    [Fact]
    public void ThePngIsEncodedStraightIntoTheDestinationAndIsAValidImage()
    {
        using ForwardOnlyStream destination = new();
        new RasterRenderer(new RasterRenderOptions { Dpi = 72 }).Render(
            new DrawnPage(new DocSize(Length.FromPoints(100), Length.FromPoints(50)), sink => { }),
            destination, RasterFormat.Png);

        using SKBitmap decoded = SKBitmap.Decode(destination.ToArray());
        decoded.Width.ShouldBe(100);
        decoded.Height.ShouldBe(50);

        using ForwardOnlyStream jpeg = new();
        new RasterRenderer(new RasterRenderOptions { Dpi = 72 }).Render(
            new DrawnPage(new DocSize(Length.FromPoints(100), Length.FromPoints(50)), sink => { }),
            jpeg, RasterFormat.Jpeg);
        SKBitmap.Decode(jpeg.ToArray()).Width.ShouldBe(100);
    }

    [Fact]
    public void RenderPagesWritesOneStreamPerPageAndHandsEachBackToTheCaller()
    {
        List<MemoryStream> streams = [];
        DocSize size = new(Length.FromPoints(60), Length.FromPoints(40));

        new RasterRenderer(new RasterRenderOptions { Dpi = 72 }).RenderPages(
            [new DrawnPage(size, sink => { }), new DrawnPage(size, sink => { }) { Index = 1 }],
            page =>
            {
                MemoryStream next = new();
                streams.Add(next);
                return next;
            },
            RasterFormat.Png);

        streams.Count.ShouldBe(2);
        streams.ShouldAllBe(s => s.Length > 0 && s.CanWrite, "the caller's streams are left open");
        streams.ForEach(s => SKBitmap.Decode(s.ToArray()).Width.ShouldBe(60));
    }

    // ------------------------------------------------------------------------------------------------

    private sealed class ProbePage(int index, Action atStart) : IPage
    {
        public int Index => index;
        public DocSize Size => DrawnPage.A4;
        public string? Label => null;

        public void Draw(IDrawingSink sink)
        {
            atStart();
            sink.BeginPage(Size);
            sink.FillPath(
                GraphicsPath.Rectangle(new DocRect(Length.FromInches(1), Length.FromInches(1), Length.FromInches(2), Length.FromInches(2))),
                Paint.Solid(Colour.Black));
            sink.EndPage();
        }
    }

    /// <summary>A memory stream that can be asked how much it holds while it is being written.</summary>
    private sealed class RecordingStream : MemoryStream;

    /// <summary>A stream that can only be written forwards and cannot be read back or measured.</summary>
    private sealed class ForwardOnlyStream : Stream
    {
        private readonly MemoryStream _inner = new();

        public override bool CanRead => false;
        public override bool CanSeek => false;
        public override bool CanWrite => true;
        public override long Length => throw new NotSupportedException();

        public override long Position
        {
            get => throw new NotSupportedException();
            set => throw new NotSupportedException();
        }

        public byte[] ToArray() => _inner.ToArray();

        public override void Write(byte[] buffer, int offset, int count) => _inner.Write(buffer, offset, count);

        public override void Flush()
        {
        }

        public override int Read(byte[] buffer, int offset, int count) => throw new NotSupportedException();

        public override long Seek(long offset, SeekOrigin origin) => throw new NotSupportedException();

        public override void SetLength(long value) => throw new NotSupportedException();

        protected override void Dispose(bool disposing)
        {
            if (disposing) _inner.Dispose();
            base.Dispose(disposing);
        }
    }
}
