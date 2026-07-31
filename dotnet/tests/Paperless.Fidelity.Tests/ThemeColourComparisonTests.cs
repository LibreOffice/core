using System.Xml.Linq;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks the DrawingML colour transform chain against the colours LibreOffice actually paints.
/// </summary>
/// <remarks>
/// <para>
/// The chain is <c>Paperless.Ooxml</c>'s and is shared by all three families, so an error in it
/// is an error in every themed shape of every document at once — which is why it is verified by
/// measurement rather than by reading the specification. The specification is in fact the wrong
/// authority here: it describes <c>a:shade</c> as a plain multiply, and LibreOffice applies it
/// in a gamma-decoded space, so following the specification would leave every shaded fill
/// perceptibly too dark while still looking like a plausible colour.
/// </para>
/// <para>
/// The measurement runs through shape fills rather than through run colours, and that is not an
/// arbitrary choice. LibreOffice does not resolve a <c>w:color w:themeColor</c> at all: its
/// importer copies the cached <c>w:val</c> straight into the character colour and keeps the
/// theme reference only for round-tripping
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:2676). Measured on this document —
/// whose five themed runs deliberately state the reference three different ways — LibreOffice
/// paints the run with a cached <c>w:val</c> in the theme colour and the four without one in
/// <b>black</b>. So a run colour cannot be used to check theme resolution against LibreOffice,
/// while a shape fill goes through <c>oox/source/drawingml/color.cxx</c> and can.
/// </para>
/// </remarks>
public sealed class ThemeColourComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-theme").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    [Fact]
    public void EveryThemedShapeFillIsTheColourLibreOfficePaints()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("theme-colours.docx");

        using FileStream stream = File.OpenRead(path);
        using DocxFile file = DocxFile.Open(stream);

        DrawingTheme theme = file.Theme.ShouldNotBeNull();

        List<DrawingColour> fills = [.. ShapeFills(file.Document)];
        fills.Count.ShouldBe(12);

        // The shapes are inline, one per paragraph, so LibreOffice paints them top to bottom in
        // document order. The runs above them draw glyphs rather than rectangles, so the only
        // filled rectangles on the page are the shapes themselves.
        List<PdfFill> painted =
            [.. PdfFills.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                        .OrderBy(fill => fill.Top)];

        painted.Count.ShouldBe(fills.Count);

        for (int i = 0; i < fills.Count; i++)
        {
            Core.Graphics.Colour resolved = fills[i].Resolve(theme).ShouldNotBeNull();
            uint got = ((uint)resolved.R << 16) | ((uint)resolved.G << 8) | resolved.B;

            got.ShouldBe(
                painted[i].Colour,
                $"shape {i} resolved to #{got:X6} where LibreOffice painted #{painted[i].Colour:X6}");
        }
    }

    /// <summary>
    /// The transform chain is not commutative, and this is the pair that proves it.
    /// </summary>
    /// <remarks>
    /// Shapes 9 and 10 carry the same two transforms on the same scheme colour in opposite
    /// orders — <c>lumMod 50%</c> then <c>shade 60%</c>, against <c>shade 60%</c> then
    /// <c>lumMod 50%</c>. They come out different because the two work in different spaces: one
    /// scales luminance in HSL and the other scales the components in gamma-decoded RGB.
    /// Measured, LibreOffice paints #3F4E20 for the first and #3E4A23 for the second.
    /// </remarks>
    [Fact]
    public void TheSameTwoTransformsInTheOtherOrderGiveADifferentColour()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("theme-colours.docx"));
        using DocxFile file = DocxFile.Open(stream);

        List<DrawingColour> fills = [.. ShapeFills(file.Document)];

        Core.Graphics.Colour first = fills[9].Resolve(file.Theme).ShouldNotBeNull();
        Core.Graphics.Colour second = fills[10].Resolve(file.Theme).ShouldNotBeNull();

        first.ToString().ShouldBe("#3F4E20");
        second.ToString().ShouldBe("#3E4A23");
    }

    private static IEnumerable<DrawingColour> ShapeFills(XElement document)
        => document
            .Descendants(XName.Get("solidFill", OoxmlNamespaces.DrawingML))
            .Select(fill => DrawingColour.Read(fill.Elements().FirstOrDefault()))
            .OfType<DrawingColour>();
}
