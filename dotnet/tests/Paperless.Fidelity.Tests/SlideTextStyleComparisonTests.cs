using System.Xml.Linq;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Ooxml;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks where the shape's own text style sits in a slide's character-inheritance chain,
/// against the colours and faces LibreOffice actually draws with.
/// </summary>
/// <remarks>
/// <para>
/// The shape's text style is the rung with no element inside the text body: it comes from
/// <c>p:style/a:fontRef</c>, it carries a colour and a typeface, and it sits <em>between</em>
/// everything the shape inherits and everything the body states
/// (<c>oox/source/drawingml/textparagraph.cxx</c>:52-67). Both extremes give the right answer on
/// a shape that states only one of the two, which is most shapes, so a document that states two
/// is the only thing that can tell a correct implementation from a plausible one — and that is
/// what <c>deck-text-style.pptx</c> is.
/// </para>
/// <para>
/// The measurement runs through the PDF's own fill colour rather than through the content tree,
/// because a <c>ContentRun</c> carries no colour: what is being checked is not what the slide
/// says but what colour it comes out. <c>PdfTextRuns</c> reads it from the graphics state, which
/// is the only place a reference render states a character colour at all.
/// </para>
/// </remarks>
public sealed class SlideTextStyleComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-slide-style").FullName;

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
    public void EveryRunResolvesToTheColourLibreOfficeDrawsItIn()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("deck-text-style.pptx");

        List<DrawingCharacterStyle> resolved = Resolved(path);
        List<PdfTextRun> drawn = Drawn(path);

        resolved.Count.ShouldBe(7);
        drawn.Count.ShouldBe(resolved.Count);

        // Shape 6 is the one deliberate divergence and is checked on its own below; shape 7 is
        // the placeholder whose style is *not* inherited, and states no colour at all.
        resolved[6].Colour.ShouldBeNull();
        drawn[6].Colour.ShouldBe(0x000000u);

        for (int i = 0; i < 5; i++)
        {
            uint mine = Rgb(resolved[i]);
            mine.ShouldBe(
                drawn[i].Colour,
                $"shape {i + 1} resolved to #{mine:X6} where LibreOffice drew #{drawn[i].Colour:X6}");
        }
    }

    /// <summary>
    /// The five shapes, named for which layer is supposed to win.
    /// </summary>
    /// <remarks>
    /// Stated as literals as well as compared with the reference, so that a change to the chain
    /// says <em>which</em> rung moved rather than only that something did.
    /// </remarks>
    [Fact]
    public void TheChainRunsRunOverParagraphOverBodyOverShapeStyleOverMaster()
    {
        List<DrawingCharacterStyle> resolved = Resolved(Corpus.Require("deck-text-style.pptx"));

        // 1: only the shape style states a colour, so accent1 straight from the theme.
        Rgb(resolved[0]).ShouldBe(0x4F81BDu);

        // 2: the body's a:lstStyle states one too, and beats the shape style.
        Rgb(resolved[1]).ShouldBe(0x00B050u);

        // 3: a different scheme slot, to show the first case is not a coincidence of ordering.
        Rgb(resolved[2]).ShouldBe(0x9BBB59u);

        // 4: the run's own a:rPr, which beats everything.
        Rgb(resolved[3]).ShouldBe(0xFF7F00u);

        // 5: a transform on the fontRef's colour — accent2 at lumMod 75%, which is the same
        // arithmetic a themed shape fill goes through and not a second implementation of it.
        Rgb(resolved[4]).ShouldBe(0x953735u);
    }

    /// <summary>
    /// LibreOffice never applies a master's <c>p:otherStyle</c>, and this is why.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SlidePersist::createXShapes</c> pushes the master's text styles into Impress's style
    /// families with <c>for (int i = 0; i &lt; 4; i++)</c> and a switch whose <c>case 4</c> is
    /// the standard style — the one <c>p:otherStyle</c> is parsed into
    /// (<c>oox/source/ppt/slidepersist.cxx</c>:315). The loop stops one short of it, so the
    /// style is read, stored, and never used; <c>case 5</c>, the subtitle style, is equally
    /// unreachable.
    /// </para>
    /// <para>
    /// Measured: shape 6 of the deck is a plain text box with no <c>p:style</c>, and the master's
    /// <c>p:otherStyle</c> states magenta for every level. LibreOffice draws it <b>black</b>.
    /// Paperless resolves the magenta, which is what the file says and what PowerPoint shows.
    /// The divergence is deliberate and it is not new here — the same fallback already decides
    /// the bullet of every non-placeholder shape in the deck — but it had never been measured.
    /// </para>
    /// </remarks>
    [Fact]
    public void AMastersOtherStyleReachesParagraphsThatLibreOfficeLeavesAtTheDefault()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("deck-text-style.pptx");

        Rgb(Resolved(path)[5]).ShouldBe(0xFF00FFu);
        Drawn(path)[5].Colour.ShouldBe(0x000000u);
    }

    /// <summary>
    /// The typeface half of the same rung: <c>a:fontRef idx="minor"</c> is the theme's minor face.
    /// </summary>
    /// <remarks>
    /// Checked against the PDF's font <em>resources</em> rather than against a family name,
    /// because resolving a resource to a base font needs real object parsing and the assertion
    /// does not need it: the five styled shapes must share one resource and the unstyled one must
    /// not. Measured, the file's two embedded faces are <c>Carlito</c> — the metric-compatible
    /// substitute for the theme's Calibri — and <c>LiberationSans</c>, which is Impress's own
    /// default and what a shape with no <c>p:style</c> falls back to.
    /// </remarks>
    [Fact]
    public void AFontReferenceResolvesToTheThemesMinorFace()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("deck-text-style.pptx");

        List<DrawingCharacterStyle> resolved = Resolved(path);
        for (int i = 0; i < 5; i++) resolved[i].LatinTypeface.ShouldBe("Calibri");

        // No p:style at all, so no theme face — and nothing invented in its place. The seventh
        // is a placeholder whose *layout* placeholder has one, which it does not inherit.
        resolved[5].LatinTypeface.ShouldBeNull();
        resolved[6].LatinTypeface.ShouldBeNull();

        List<PdfTextRun> drawn = Drawn(path);
        drawn.Take(5).Select(run => run.FontResource).Distinct().Count().ShouldBe(1);
        drawn[5].FontResource.ShouldNotBe(drawn[0].FontResource);
        drawn[6].FontResource.ShouldBe(drawn[5].FontResource);
    }

    /// <summary>Each shape's first run, resolved through the whole chain, in document order.</summary>
    private static List<DrawingCharacterStyle> Resolved(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using PptxFile file = PptxFile.Open(stream);

        PptxSlide slide = file.Slides[0];
        PptxTextStyles styles = new(
            slide.Layout, slide.Master, file.DefaultTextStyle, isNotesPage: false,
            theme: slide.Theme);

        List<DrawingCharacterStyle> resolved = [];

        foreach (XElement shape in Ppt.Children(slide.ShapeTree!, "sp"))
        {
            DrawingTextOptions options = styles.For(shape, _ => null);

            XElement body = Ppt.Child(shape, "txBody")!;
            XElement paragraph = Drawing.Children(body, "p").First();
            XElement run = Drawing.Children(paragraph, "r").First();

            resolved.Add(DrawingCharacterStyle.Resolve(
                Drawing.Child(run, "rPr"),
                Drawing.Child(paragraph, "pPr"),
                Drawing.Child(body, "lstStyle"),
                level: 0,
                options));
        }

        return resolved;
    }

    /// <summary>LibreOffice's runs on the first page, top to bottom.</summary>
    private List<PdfTextRun> Drawn(string path)
        => [.. PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
            .Where(run => run.PageIndex == 0 && run.GlyphCount > 0)
            .OrderBy(run => run.Y)];

    private static uint Rgb(DrawingCharacterStyle style)
    {
        Core.Graphics.Colour colour = style.Colour.ShouldNotBeNull();
        return ((uint)colour.R << 16) | ((uint)colour.G << 8) | colour.B;
    }
}
