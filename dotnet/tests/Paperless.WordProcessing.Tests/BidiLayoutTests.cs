using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks that a paragraph's declared direction is read, and that it reaches the drawn line.
/// </summary>
/// <remarks>
/// <para>
/// The four formats spell one property four ways — ODF's <c>style:writing-mode</c>, OOXML's
/// <c>w:bidi</c>, RTF's <c>\rtlpar</c> and WW8's <c>sprmPFBiDi</c> — and two of them spell the
/// alignment that goes with it in terms of <em>page sides</em> rather than reading edges, so the
/// same right-aligned paragraph is <c>fo:text-align="right"</c> in ODF and <c>w:jc w:val="start"</c>
/// in OOXML. Asserting on the resolved paragraphs rather than on a rendering is what makes a
/// failure say which of those two things went wrong; where the line actually lands is compared
/// against LibreOffice separately, in the fidelity tests.
/// </para>
/// <para>
/// The corpus document's right-to-left paragraphs state <c>left</c> and <c>right</c> rather than
/// <c>start</c> and <c>end</c> on purpose — see the comment in <c>bidi-drawing.fodt</c>, which
/// records why a reference rendered by LibreOffice 24.2 needs the physical spellings.
/// </para>
/// <para>
/// Four formats and not five: the same document round-tripped through <c>.odt</c> is deliberately
/// absent. LibreOffice 24.2 <em>rewrites</em> the alignment on the way out — a right-aligned
/// right-to-left paragraph comes back as <c>fo:text-align="end"</c> — because before
/// <c>ParagraphAdjust::START</c> existed it treated ODF's start and end as plain aliases for left
/// and right. Reading that file by the specification, where the two are relative to the writing
/// mode, gives a paragraph aligned the other way; reading it the old way would break every
/// left-to-right document. So the round trip is a version artefact rather than coverage, and the
/// flat ODF document exercises the same reader.
/// </para>
/// </remarks>
public sealed class BidiLayoutTests
{
    /// <summary>The A4 text area the corpus document declares: 21 cm less 2.5 and 2 cm of margin.</summary>
    private static readonly Length TextWidth = Length.FromMillimetres(210 - 25 - 20);

    [Theory]
    [InlineData("bidi-drawing.fodt")]
    [InlineData("bidi-drawing.docx")]
    [InlineData("bidi-drawing.doc")]
    [InlineData("bidi-drawing.rtf")]
    public void ADeclaredWritingModeReachesTheParagraph(string fileName)
    {
        List<PageParagraph> paragraphs = ParagraphsOf(fileName);

        // The three left-to-right paragraphs first, because a reader that answered "right to left"
        // to everything would pass every other assertion here.
        for (int i = 0; i < 3; i++)
        {
            paragraphs[i].Format.IsRightToLeft.ShouldBeFalse(
                $"{fileName}: paragraph {i + 1} is declared left to right");
        }

        for (int i = 3; i < 7; i++)
        {
            paragraphs[i].Format.IsRightToLeft.ShouldBeTrue(
                $"{fileName}: paragraph {i + 1} is declared right to left");
        }
    }

    [Theory]
    [InlineData("bidi-drawing.fodt")]
    [InlineData("bidi-drawing.docx")]
    [InlineData("bidi-drawing.doc")]
    [InlineData("bidi-drawing.rtf")]
    public void APageSideResolvesToTheEdgeItIsInThatDirection(string fileName)
    {
        List<PageParagraph> paragraphs = ParagraphsOf(fileName);

        // Right-aligned and right to left, which is the paragraph's *start* edge.
        paragraphs[3].Format.Alignment.ShouldBe(
            TextAlignment.Start, $"{fileName}: a right-aligned right-to-left paragraph starts there");

        // And left-aligned and right to left, which is its end.
        paragraphs[6].Format.Alignment.ShouldBe(
            TextAlignment.End, $"{fileName}: a left-aligned right-to-left paragraph ends there");

        // The left-to-right ones are unaffected: left is still the start.
        paragraphs[0].Format.Alignment.ShouldBe(TextAlignment.Start);
    }

    [Theory]
    [InlineData("bidi-drawing.fodt")]
    [InlineData("bidi-drawing.docx")]
    [InlineData("bidi-drawing.doc")]
    [InlineData("bidi-drawing.rtf")]
    public void AStartIndentIsMeasuredFromTheRightInARightToLeftParagraph(string fileName)
    {
        List<PageParagraph> paragraphs = ParagraphsOf(fileName);

        // Three centimetres, stated as fo:margin-left and exported as w:ind w:start and \li1701.
        paragraphs[5].Format.StartIndent.Millimetres.ShouldBe(30.0, 0.2, $"{fileName}: the indent");

        // Which the layout puts on the *right*: the line is right-aligned, so its text ends three
        // centimetres short of the right margin and its left edge moves by the same amount.
        LineBox line = FirstLineOf(paragraphs[5]);
        (line.Left + line.Width).Millimetres.ShouldBe(
            (TextWidth - Length.FromMillimetres(30)).Millimetres,
            0.3,
            $"{fileName}: the start indent is at the right margin");
    }

    [Theory]
    [InlineData("bidi-drawing.fodt")]
    [InlineData("bidi-drawing.docx")]
    [InlineData("bidi-drawing.doc")]
    [InlineData("bidi-drawing.rtf")]
    public void ARightToLeftParagraphStartsAtTheRightEdge(string fileName)
    {
        List<PageParagraph> paragraphs = ParagraphsOf(fileName);

        LineBox started = FirstLineOf(paragraphs[3]);
        (started.Left + started.Width).Millimetres.ShouldBe(
            TextWidth.Millimetres, 0.3, $"{fileName}: a start-aligned line ends at the right margin");

        started.Left.ShouldBeGreaterThan(
            Length.Zero, $"{fileName}: and so does not begin at the left one");

        // The end-aligned one is the mirror of it, and is what says the mirror is a mirror rather
        // than a right-alignment applied twice.
        FirstLineOf(paragraphs[6]).Left.ShouldBe(
            Length.Zero, $"{fileName}: an end-aligned right-to-left line starts at the left margin");
    }

    [Theory]
    [InlineData("bidi-drawing.fodt")]
    [InlineData("bidi-drawing.docx")]
    [InlineData("bidi-drawing.doc")]
    [InlineData("bidi-drawing.rtf")]
    public void AMixedLineIsDrawnInVisualOrder(string fileName)
    {
        List<PageParagraph> paragraphs = ParagraphsOf(fileName);

        // "Start שלום 123 עולם end.": the Hebrew and the number between them reverse as a block, so
        // the drawn text is not the paragraph's text.
        List<(GlyphRun Run, Colour Colour)> runs = DrawnRunsOf(paragraphs[2]);

        runs.Count.ShouldBeGreaterThan(1, $"{fileName}: a mixed line draws more than one run");

        string drawn = string.Concat(runs.Select(run => run.Run.Text));
        drawn.ShouldNotBe(
            paragraphs[2].Text,
            $"{fileName}: the runs are drawn in logical order, so nothing was reordered");

        // Every character still gets drawn exactly once, which is what tells a reordering from a
        // reader that dropped a run: the drawn text is a permutation of the paragraph's.
        drawn.Order().ShouldBe(paragraphs[2].Text.Order());

        // And the pens rise strictly, which is what "visual order" means once the runs are placed.
        for (int i = 1; i < runs.Count; i++)
        {
            runs[i].Run.Origin.X.ShouldBeGreaterThan(
                runs[i - 1].Run.Origin.X, $"{fileName}: run {i + 1} is left of the one before it");
        }
    }

    [Theory]
    [InlineData("bidi-drawing.fodt")]
    [InlineData("bidi-drawing.docx")]
    [InlineData("bidi-drawing.doc")]
    [InlineData("bidi-drawing.rtf")]
    public void ALatinParagraphIsStillDrawnAsOneRun(string fileName)
    {
        // The regression that matters. A paragraph with nothing right-to-left in it must not be cut
        // into sub-runs it does not need: each boundary loses the shaping context across it, so the
        // line measures very slightly wide — enough, on a full line, to move a break. One run, and
        // its text is the paragraph's own with nothing reordered.
        PageParagraph latin = ParagraphsOf(fileName)[0];

        List<(GlyphRun Run, Colour Colour)> runs = DrawnRunsOf(latin);

        runs.Count.ShouldBe(1, $"{fileName}: a Latin line draws one run");
        runs[0].Run.Text.ShouldBe(latin.Text);
    }

    /// <summary>The corpus document's body paragraphs, in order.</summary>
    private static List<PageParagraph> ParagraphsOf(string fileName)
    {
        string path = Corpus.Require(fileName);

        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        List<PageParagraph> paragraphs = [.. pages.Paragraphs.Where(p => p.Text.Length > 0)];
        paragraphs.Count.ShouldBe(7, $"{fileName}: the document's paragraphs");

        return paragraphs;
    }

    /// <summary>The first line of a paragraph laid out on its own, at the document's text width.</summary>
    /// <remarks>
    /// Laid out here rather than read off the page so that the assertion is about the paragraph's own
    /// properties: a line taken from the page would also carry whatever the page's geometry did to it,
    /// and the two would be indistinguishable in a failure.
    /// </remarks>
    private static LineBox FirstLineOf(PageParagraph paragraph)
    {
        ParagraphLayouter layouter = new(paragraph.Face);

        LaidOutParagraph laid = layouter.Layout(
            paragraph.Text,
            paragraph.Format,
            paragraph.EmSize,
            TextWidth,
            paragraph.Language,
            follows: null,
            paragraph.Shaping);

        return laid.Lines[0];
    }

    /// <summary>The glyph runs a paragraph's first line draws, in the order they are drawn.</summary>
    private static List<(GlyphRun Run, Colour Colour)> DrawnRunsOf(PageParagraph paragraph)
    {
        LineBox box = FirstLineOf(paragraph);

        return PageDrawing.RunsIn(
            new DocRect(Length.Zero, Length.Zero, TextWidth, Length.FromMillimetres(100)),
            new PlacedLine(0, 0, box, Length.Zero),
            paragraph);
    }
}
