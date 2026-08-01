using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The label a numbered or bulleted paragraph draws, and where it lands.
/// </summary>
/// <remarks>
/// <para>
/// All four readers computed the label already — extraction has reported <c>1.</c> and <c>◦</c> since
/// the readers were written — and none of them passed it to layout, so <c>Paginator</c> measured no
/// label and <c>PageDrawing</c> drew none. That is why the same shortfall appeared in six sweep rows
/// across four formats and two packagings: one missing feature, not six defects.
/// </para>
/// <para>
/// Asserted on positions rather than only on presence, because a label at the wrong pen still counts as
/// a word to <c>pdftotext</c> — the measurement that first named this bug could not have told a missing
/// label from a misplaced one, and the numbers below can.
/// </para>
/// </remarks>
public sealed class ListLabelTests
{
    /// <summary>
    /// Every format of the two feature documents draws the labels LibreOffice draws.
    /// </summary>
    /// <remarks>
    /// The labels are the same in all six because the six files are two documents: LibreOffice's own
    /// conversions of one <c>.odt</c> and one <c>.docx</c>. A reader that counted its lists differently
    /// would show up here as one letter or one digit out of step rather than as a missing word.
    /// </remarks>
    [Theory]
    [InlineData("text-features.odt")]
    [InlineData("text-features-flat.fodt")]
    [InlineData("word-features.docx")]
    [InlineData("word-features.dotx")]
    [InlineData("word-features.doc")]
    [InlineData("word-features.rtf")]
    public void TheDocumentsListsAreLabelled(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        List<string> labels =
        [
            .. pages.Paragraphs
                .Where(paragraph => paragraph.Label is { Text.Length: > 0 })
                .Select(paragraph => paragraph.Label!.Text),
        ];

        // Two bullets, a nested bullet, two numbers, two letters and a third number — in that order,
        // and with the continuation paragraph between "2." and "a)" carrying none of its own.
        labels.ShouldBe(["•", "•", "◦", "1.", "2.", "a)", "b)", "3."]);
    }

    /// <summary>
    /// The label hangs where the paragraph's first line would have started, and the text starts after it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The measurement that settles it, taken from LibreOffice's own PDF of <c>text-features.odt</c>: on
    /// a page whose text starts at 56.8 pt, the level-one label sits at 56.8 and its item's text at 74.8,
    /// and the level-two label at 74.8 with its text at 92.8. Eighteen points is the quarter inch the
    /// level's <c>text:min-label-width</c> asks for.
    /// </para>
    /// <para>
    /// Asserted relative to the paragraph's own indents rather than in page coordinates, so the test says
    /// what the rule is: the label's pen is the <em>declared</em> first line's start and the text's pen is
    /// the widened one.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheLabelHangsIntoTheFirstLinesIndent()
    {
        PageParagraph paragraph = Item("1.", start: 18, firstLine: -18);

        paragraph.DeclaredFormat.LineStart(isFirstLine: true).ShouldBe(Length.Zero);
        paragraph.Format.LineStart(isFirstLine: true).ShouldBe(Length.FromPoints(18));
        paragraph.Format.LineStart(isFirstLine: false).ShouldBe(Length.FromPoints(18));
    }

    /// <summary>
    /// A paragraph with no label is left exactly as its reader stated it.
    /// </summary>
    /// <remarks>
    /// The overwhelming majority of paragraphs, so this is the case that must not move: an unlabelled
    /// paragraph's <see cref="PageParagraph.Format"/> has to be the identical instance a reader handed in,
    /// or every hanging indent in every document shifts.
    /// </remarks>
    [Fact]
    public void AnUnlabelledParagraphKeepsItsFormatExactly()
    {
        ParagraphFormat stated = new()
        {
            StartIndent = Length.FromPoints(18),
            FirstLineIndent = Length.FromPoints(-18),
        };

        PageParagraph paragraph = new() { Text = "plain", Face = Face, Format = stated };

        paragraph.Format.ShouldBeSameAs(stated);
        paragraph.Label.ShouldBeNull();
    }

    /// <summary>
    /// A label wider than the room set aside for it pushes the text rather than colliding with it.
    /// </summary>
    /// <remarks>
    /// Writer's floor of <c>m_nFixWidth + m_nMinDist</c> (<c>sw/source/core/text/porfld.cxx:653</c>). The
    /// case is a deep roman numeral in a level that reserved a quarter inch: <c>viii.</c> is wider than
    /// eighteen points at twelve, so the first line's text starts at the label's end instead.
    /// </remarks>
    [Fact]
    public void AnOversizeLabelWidensTheFirstLineFurther()
    {
        PageParagraph narrow = Item("viii.", start: 18, firstLine: -18);
        Length labelWidth = narrow.Label!.Width;

        labelWidth.ShouldBeGreaterThan(Length.FromPoints(18));
        narrow.Format.LineStart(isFirstLine: true).ShouldBe(labelWidth);
    }

    /// <summary>
    /// A list tab carries the text to the level's own stop.
    /// </summary>
    /// <remarks>
    /// The label-alignment mode ODF writes for Writer and OOXML writes in <c>w:tab w:val="num"</c>. The
    /// stop wins over the hanging indent when it is further out, which is what a level whose
    /// <c>fo:text-indent</c> and <c>text:list-tab-stop-position</c> disagree asks for.
    /// </remarks>
    [Fact]
    public void AListTabTakesTheTextToItsStop()
    {
        PageParagraph paragraph = Item("1.", start: 18, firstLine: -18, label => label with
        {
            Follow = LabelFollow.ListTab,
            TabStop = Length.FromPoints(36),
        });

        paragraph.Format.LineStart(isFirstLine: true).ShouldBe(Length.FromPoints(36));
    }

    /// <summary>
    /// The label is drawn, once, at the head of the paragraph's first line and nowhere else.
    /// </summary>
    /// <remarks>
    /// Drawing is where the bug was visible, so this checks the drawing rather than the model: the
    /// paragraph's own glyphs start at the widened indent, and one further run sits an advance to the left
    /// of them. A second line of the same paragraph draws no label at all — a wrapped item is numbered
    /// once.
    /// </remarks>
    [Fact]
    public void TheLabelIsDrawnAtTheHeadOfTheFirstLineOnly()
    {
        PageParagraph paragraph = Item("1.", start: 18, firstLine: -18);

        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400));
        PlacedLine first = Line(paragraph, lineIndex: 0);
        PlacedLine second = Line(paragraph, lineIndex: 1);

        List<(GlyphRun Run, Colour Colour)> drawn = PageDrawing.RunsIn(area, first, paragraph);

        drawn.Count.ShouldBe(2);
        drawn[0].Run.Origin.X.ShouldBe(Length.Zero);
        drawn[1].Run.Origin.X.ShouldBe(Length.FromPoints(18));

        PageDrawing.RunsIn(area, second, paragraph).Count.ShouldBe(1);
    }

    /// <summary>
    /// An item with no words still draws its number.
    /// </summary>
    /// <remarks>
    /// What a list being typed into looks like, and the reason the label is emitted before the empty-line
    /// exit rather than after it. LibreOffice draws the number for an empty item too.
    /// </remarks>
    [Fact]
    public void AnEmptyItemStillDrawsItsLabel()
    {
        PageParagraph paragraph = Item("1.", start: 18, firstLine: -18, text: "");

        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400));

        PageDrawing.RunsIn(area, Line(paragraph, lineIndex: 0), paragraph).Count.ShouldBe(1);
    }

    /// <summary>A paragraph that is a list item, with the indents its level gave it.</summary>
    private static PageParagraph Item(
        string label,
        double start,
        double firstLine,
        Func<PageLabel, PageLabel>? adjust = null,
        string text = "an item")
    {
        PageLabel measured = PageLabel.Measured(label, Face, Length.FromPoints(12))
            with { Follow = LabelFollow.Nothing };

        return new PageParagraph
        {
            Text = text,
            Face = Face,
            EmSize = Length.FromPoints(12),
            Format = new ParagraphFormat
            {
                StartIndent = Length.FromPoints(start),
                FirstLineIndent = Length.FromPoints(firstLine),
            },
            Label = adjust is null ? measured : adjust(measured),
        };
    }

    /// <summary>One line of a paragraph, placed at the top of its area.</summary>
    /// <remarks>
    /// Built by hand rather than paginated, because what is under test is where a line's runs go given
    /// its box — and <see cref="LineBox.Left"/> is exactly the number the paginator would have taken from
    /// <see cref="ParagraphFormat.LineStart"/>.
    /// </remarks>
    private static PlacedLine Line(PageParagraph paragraph, int lineIndex)
        => new(
            ParagraphIndex: 0,
            LineIndex: lineIndex,
            Box: new LineBox(
                new TextLine(
                    0, paragraph.Text.Length, paragraph.Text.Length, Length.Zero, EndsParagraph: true),
                paragraph.Format.LineStart(isFirstLine: lineIndex == 0),
                Length.Zero,
                Length.FromPoints(14),
                Length.FromPoints(11),
                Length.Zero),
            Top: Length.Zero);

    /// <summary>
    /// A real face, since the label's advance is a measurement rather than a constant.
    /// </summary>
    /// <remarks>
    /// Through the resolver rather than by finding a file, so the test runs on a machine with a different
    /// set of fonts installed: what matters is that the advance is a real one, not which face produced it.
    /// </remarks>
    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
