using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The coloured band a highlighted run draws behind itself.
/// </summary>
/// <remarks>
/// <para>
/// Word's highlighter — <c>sprmCHighlight</c> in a DOC, <c>w:highlight</c> in a DOCX, <c>\highlight</c> in
/// an RTF, <c>fo:background-color</c> once LibreOffice has exported any of them to ODF. Every reader read
/// past it and the drawing pass had nowhere to put it, so a highlighted document extracted perfectly and
/// rendered with nothing behind its text. The gate that measures words cannot see that at all:
/// <c>words/batch-001/doc/LENTOBUSSIAIKATAULU.-31.10.-31.12.2022.doc</c> matched 119 words against 119
/// while missing both of its yellow bands.
/// </para>
/// <para>
/// The band is checked at the <em>drawing</em> pass rather than at the reader, because that is where the
/// two halves have to meet: the colour comes from the run and the rectangle from the pen position the tab
/// stops and the justification left it at.
/// </para>
/// </remarks>
public sealed class HighlightTests
{
    private static readonly Length Size = Length.FromPoints(12);

    [Fact]
    public void AHighlightedRunPutsABandBehindItsOwnGlyphs()
    {
        (List<(GlyphRun Run, Colour Colour)> runs, List<(DocRect Area, Colour Colour)> bands) =
            Draw(Highlighted("plain and lit", from: 10, to: 13, Colour.FromRgb(0xFFFF00)));

        bands.Count.ShouldBe(1);
        bands[0].Colour.ShouldBe(Colour.FromRgb(0xFFFF00));

        // The band covers the highlighted run and only it: it starts where that run's glyphs start and is
        // exactly as wide as they are.
        GlyphRun lit = runs[^1].Run;
        bands[0].Area.X.ShouldBe(lit.Origin.X);
        bands[0].Area.Width.ShouldBeGreaterThan(Length.Zero);
        bands[0].Area.Width.ShouldBeLessThan(runs[0].Run.Origin.X + Length.FromPoints(400));
    }

    [Fact]
    public void AnUnhighlightedParagraphDrawsNoBand()
    {
        (_, List<(DocRect Area, Colour Colour)> bands) =
            Draw(Highlighted("plain and lit", from: 10, to: 13, highlight: default));

        bands.ShouldBeEmpty("a transparent highlight is the absence of one, not a colour");
    }

    [Fact]
    public void TheBandStraddlesTheBaselineByTheRunsOwnMetrics()
    {
        (List<(GlyphRun Run, Colour Colour)> runs, List<(DocRect Area, Colour Colour)> bands) =
            Draw(Highlighted("plain and lit", from: 10, to: 13, Colour.FromRgb(0xFFFF00)));

        // Writer takes the rectangle from the baseline less the portion's ascent, with the portion's own
        // height (SwTextPaintInfo::CalcRect) — so the band has ink above the baseline and below it.
        Length baseline = runs[^1].Run.Origin.Y;
        bands[0].Area.Y.ShouldBeLessThan(baseline);
        bands[0].Area.Bottom.ShouldBeGreaterThan(baseline);

        LineMetrics metrics = LineSpacing.Resolve(Face);
        bands[0].Area.Height.ShouldBe(metrics.ScaledLineHeight(Size));
    }

    /// <summary>
    /// <c>w:highlight</c> names a colour from a closed list, and the list is not the obvious one.
    /// </summary>
    /// <remarks>
    /// The values are <c>DomainMapper::getColorFromId</c>'s
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:5198). <c>green</c> is bright green and
    /// <c>darkGreen</c> is the one an eye would call green, which is the pair a plausible guess gets
    /// wrong — and <c>none</c> is a value the schema allows and means no band at all.
    /// </remarks>
    [Theory]
    [InlineData("yellow", 0xFFFF00u)]
    [InlineData("green", 0x00FF00u)]
    [InlineData("darkGreen", 0x008000u)]
    [InlineData("lightGray", 0xC0C0C0u)]
    public void ARunTakesTheColourItsHighlightNames(string name, uint expected)
        => Resolved(name).Highlight.ShouldBe(Colour.FromRgb(expected));

    [Theory]
    [InlineData("none")]
    [InlineData("")]
    public void ARunWithNoHighlightNamesNoColour(string name)
        => Resolved(name).Highlight.ShouldBeNull();

    /// <summary>The character formatting of a run whose <c>w:rPr</c> names a highlight.</summary>
    private static Ooxml.WordTextStyle Resolved(string highlight)
    {
        XNamespace w = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

        XElement properties = new(w + "rPr");
        if (highlight.Length > 0)
        {
            properties.Add(new XElement(w + "highlight", new XAttribute(w + "val", highlight)));
        }

        return Ooxml.WordParagraphFormats.ResolveRun(new Ooxml.WordStyles(), null, properties);
    }

    private static (List<(GlyphRun Run, Colour Colour)>, List<(DocRect Area, Colour Colour)>) Draw(
        PageParagraph paragraph)
    {
        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400));
        List<(DocRect Area, Colour Colour)> bands = [];

        List<(GlyphRun Run, Colour Colour)> runs =
            PageDrawing.RunsIn(area, Line(paragraph), paragraph, bands);

        return (runs, bands);
    }

    /// <summary>A paragraph whose last few characters carry a highlight and whose first do not.</summary>
    private static PageParagraph Highlighted(string text, int from, int to, Colour highlight)
        => new()
        {
            Text = text,
            Face = Face,
            EmSize = Size,
            Runs =
            [
                new PageRun(0, from, Face, Size, Colour: Colour.Black),
                new PageRun(from, to - from, Face, Size, Colour: Colour.Black, Highlight: highlight),
            ],
        };

    private static PlacedLine Line(PageParagraph paragraph)
        => new(
            ParagraphIndex: 0,
            LineIndex: 0,
            Box: new LineBox(
                new TextLine(
                    0, paragraph.Text.Length, paragraph.Text.Length, Length.Zero, EndsParagraph: true),
                Length.Zero,
                Length.Zero,
                Length.FromPoints(14),
                Length.FromPoints(11),
                Length.Zero),
            Top: Length.Zero);

    /// <summary>A real face, since the band's width and height are measurements rather than constants.</summary>
    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
