using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

using RowSlice = XRay.Office.WordProcessing.Layout.TableLayouter.RowSlice;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a table row's two halves are charged for when a page break falls inside one.
/// </summary>
/// <remarks>
/// <para>
/// A part of a split row spans from the top of the block its first line is in to the top of the block
/// its last line is followed by — not from line top to line bottom. The two are the same in the middle
/// of a paragraph and differ at either end of one, and the difference is a paragraph's
/// <c>w:spacing w:before</c> and <c>w:after</c>: the follow part re-applies the space-before of the
/// paragraph it resumes, and a part whose last line completes a paragraph is charged that paragraph's
/// space-after.
/// </para>
/// <para>
/// Measured rather than reasoned, on sixteen authored probes rendered through LibreOffice 26.2.4.2 —
/// <c>dotnet/probes/words-regress-01/probe-rowsplit-spacing.py</c> sweeps the two spacings
/// independently over a row cut across a page and <c>probe-rowsplit-paras.py</c> moves the cut to a
/// paragraph boundary. In every one of them the reference's follow part is
/// <c>before + remaining lines + after + border</c>, so a split row's parts add up to <em>more</em>
/// than the unsplit row by exactly its space-before. Writer reaches this through
/// <c>SwFlowFrame::CalcUpperSpace</c> under <c>AddParaSpacingToTableCells</c>, the same compatibility
/// setting that already makes <see cref="PlacedFlow.Advance"/> charge a cell for its last paragraph's
/// space-after.
/// </para>
/// <para>
/// Nothing here hard-codes a font metric. The line height is <em>derived</em> — read back off the
/// flow the layouter produced — so the assertions state the rule and not this machine's Liberation
/// Serif, and go on meaning the same thing if the metrics move.
/// </para>
/// </remarks>
public sealed class TableRowSplitSpacingTests
{
    private static readonly Length Before = Length.FromPoints(6);
    private static readonly Length After = Length.FromPoints(3);

    /// <summary>
    /// The follow part carries the space-before of the paragraph it resumes, so the row's two parts
    /// add up to more than the row.
    /// </summary>
    [Fact]
    public void TheFollowPartOfASplitParagraphCarriesItsSpaceBefore()
    {
        PageTable table = OneParagraph(Before, After);
        (Length row, Length line, int count) = Measure(table);
        (RowSlice first, RowSlice follow) = SplitBeforeLastLine(table);

        // Every line but the last, and the space above them — but no space-after: the paragraph is
        // not finished on this part.
        first.Height.ShouldBe(Before + (line * (count - 1)));

        // The last line and the space below it — and the space above it a second time.
        follow.Height.ShouldBe(Before + line + After);

        row.ShouldBe(Before + (line * count) + After, "the unsplit row is the two ends and the lines");
    }

    /// <summary>
    /// The same row stated as a sum: the parts exceed the whole by one space-before.
    /// </summary>
    /// <remarks>
    /// The form the defect was found in. Our follow part was measured from its first <em>line</em>, so
    /// the parts added up to exactly the row and the row came out a space-before short on every page
    /// after the first — 1.00 pt a page on <c>Sample_SQMS_Program.docx</c>, which is a page over sixty
    /// of them.
    /// </remarks>
    [Fact]
    public void ASplitRowsPartsExceedTheRowByItsSpaceBefore()
    {
        PageTable table = OneParagraph(Before, After);
        (Length row, _, _) = Measure(table);
        (RowSlice first, RowSlice follow) = SplitBeforeLastLine(table);

        (first.Height + follow.Height).ShouldBe(row + Before);
    }

    /// <summary>
    /// The control: with no space-before there is nothing to re-apply, and the parts add up to the row.
    /// </summary>
    [Fact]
    public void WithNoSpaceBeforeThePartsAddUpToTheRowExactly()
    {
        PageTable table = OneParagraph(Length.Zero, After);
        (Length row, _, _) = Measure(table);
        (RowSlice first, RowSlice follow) = SplitBeforeLastLine(table);

        (first.Height + follow.Height).ShouldBe(row);
    }

    /// <summary>
    /// A part is charged the space-after of a paragraph it finishes, even though more of the cell
    /// follows.
    /// </summary>
    /// <remarks>
    /// The other end of the same rule, and the one that decides where the cut may fall: measured on
    /// <c>probe-rowsplit-paras.py</c>, two paragraphs of one line each cut between them give the
    /// reference two parts of <c>before + line + after</c>, not a first part that stops at its line.
    /// </remarks>
    [Fact]
    public void APartThatFinishesAParagraphIsChargedItsSpaceAfter()
    {
        PageTable table = TwoParagraphs(Before, After);
        (Length row, _, int count) = Measure(table);
        count.ShouldBe(2, "one line each");

        // One line's height, taken from a paragraph that wraps — `Measure` reports the block-to-block
        // advance here, which is the very spacing under test.
        Length line = Measure(OneParagraph(Before, After)).Line;

        (RowSlice first, RowSlice follow) = SplitBeforeLastLine(table);

        first.Height.ShouldBe(Before + line + After, "the first paragraph is finished on this part");
        follow.Height.ShouldBe(Before + line + After, "and the second is a whole paragraph of its own");
        (first.Height + follow.Height).ShouldBe(row, "neither end is counted twice here");
    }

    /// <summary>
    /// The cut still falls on a line boundary and no line is lost or drawn twice.
    /// </summary>
    /// <remarks>
    /// A guard on the two rules above rather than a rule of its own: charging a part for spacing it
    /// does not draw is right, and charging it for a line it does not draw would not be.
    /// </remarks>
    [Fact]
    public void NoLineIsLostOrDrawnTwiceAcrossTheCut()
    {
        PageTable table = OneParagraph(Before, After);
        (_, _, int count) = Measure(table);
        (RowSlice first, RowSlice follow) = SplitBeforeLastLine(table);

        Lines(first).ShouldBe(count - 1);
        Lines(follow).ShouldBe(1);
        follow.IsComplete.ShouldBeTrue("the second part holds everything that was left");
    }

    private static int Lines(RowSlice slice)
        => slice.Cells.Sum(cell => cell.Content?.Lines.Count ?? 0);

    /// <summary>A cell's line height and its row's, as the layouter actually produced them.</summary>
    /// <remarks>
    /// Read off the placed flow rather than computed from a font, so this test states the rule and not
    /// this machine's Liberation Serif — and so it keeps meaning the same thing if the metrics move.
    /// </remarks>
    private static (Length Row, Length Line, int Count) Measure(PageTable table)
    {
        (List<PlacedTableCell> cells, List<Length> heights) =
            TableLayouter.LayOut(table, new DocPoint(Length.Zero, Length.Zero));

        PlacedFlow flow = cells[0].Content!;
        Length line = flow.Lines.Count > 1
            ? flow.Lines[1].Top - flow.Lines[0].Top
            : flow.Lines[0].Box.Height;

        return (heights[0], line, flow.Lines.Count);
    }

    /// <summary>Cuts a row so that its first part holds all but the last line.</summary>
    /// <remarks>
    /// The room is searched for rather than computed, because computing it would need the very rule
    /// under test. Stepping down from the whole row's height, the first room that leaves exactly one
    /// line over is the cut this test means.
    /// </remarks>
    private static (RowSlice First, RowSlice Follow) SplitBeforeLastLine(PageTable table)
    {
        (Length row, Length line, int count) = Measure(table);
        count.ShouldBeGreaterThan(1, "a row of one line has nothing to split");

        (List<PlacedTableCell> cells, _) =
            TableLayouter.LayOut(table, new DocPoint(Length.Zero, Length.Zero));

        for (Length room = row; room > line; room -= Length.FromTwips(5))
        {
            if (TableLayouter.SliceRow(table.Rows[0], cells, Length.Zero, room) is not { } first)
                continue;
            if (Lines(first) != count - 1) continue;

            RowSlice? follow = TableLayouter.SliceRow(
                table.Rows[0], cells, first.Cut, Length.FromPoints(10_000));
            follow.ShouldNotBeNull("the rest of the row has to be placeable");

            return (first, follow.Value);
        }

        throw new InvalidOperationException("no room leaves exactly one line over");
    }

    /// <summary>
    /// A collapsing document's follow part is charged the paragraph's <em>own</em> space-before,
    /// not the gap the flow was laid out with.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two differ only when the paragraph above states a space-after — which is why every test
    /// above, laid out with collapsing off, could not see this. With collapsing on, the paragraph
    /// that opens the follow part was laid out with <c>max(before − after, 0)</c> above it, and
    /// re-applying <em>that</em> gives it nothing at all when the two figures are equal, which is
    /// the commonest shape a Word style has.
    /// </para>
    /// <para>
    /// Writer never collapses here, because it never finds a frame to collapse against: the
    /// paragraph above is in the master cell frame on the previous page, so
    /// <c>SwFlowFrame::CalcUpperSpace</c> falls to its no-previous-frame branch and takes
    /// <c>pAttrs-&gt;GetULSpace().GetUpper()</c> whole
    /// (<c>sw/source/core/layout/flowfrm.cxx:1744-1748</c>). Measured against both installed
    /// references in <c>dotnet/probes/words-clip-rowsplit/</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void ACollapsingDocumentsFollowPartStillCarriesTheWholeSpaceBefore()
    {
        // Equal figures, so collapsing takes the space-before away entirely inside the flow.
        PageTable table = TwoParagraphs(Before, Before);

        (List<PlacedTableCell> cells, List<Length> heights) =
            TableLayouter.LayOut(
                table, new DocPoint(Length.Zero, Length.Zero), collapsesSpacing: true);

        PlacedFlow flow = cells[0].Content!;
        flow.Lines.Count.ShouldBe(2, "one line each");
        flow.Lines[1].UpperSpace.ShouldBe(
            Length.Zero, "collapsing took the second paragraph's space-before away");

        Length line = flow.Lines[1].Top - flow.Lines[0].Top;
        RowSlice follow = FollowPart(table, cells, heights[0], line);

        follow.Height.ShouldBe(
            Before + line, "the follow part re-applies the whole space-before above its line");
    }

    /// <summary>The second part of a row cut so that its first part holds all but the last line.</summary>
    private static RowSlice FollowPart(
        PageTable table, List<PlacedTableCell> cells, Length row, Length line)
    {
        for (Length room = row; room > line; room -= Length.FromTwips(5))
        {
            if (TableLayouter.SliceRow(table.Rows[0], cells, Length.Zero, room) is not { } first)
                continue;
            if (Lines(first) != 1) continue;

            RowSlice? follow = TableLayouter.SliceRow(
                table.Rows[0], cells, first.Cut, Length.FromPoints(10_000));
            follow.ShouldNotBeNull("the rest of the row has to be placeable");

            return follow.Value;
        }

        throw new InvalidOperationException("no room leaves exactly one line over");
    }

    /// <summary>One paragraph long enough to wrap several times in the column.</summary>
    private static PageTable OneParagraph(Length before, Length after)
        => Table([Paragraph(
            string.Join(" ", Enumerable.Repeat("alpha bravo charlie delta", 6)), before, after)]);

    /// <summary>Two paragraphs of one line each, so a cut between them is a cut between blocks.</summary>
    private static PageTable TwoParagraphs(Length before, Length after)
        => Table([Paragraph("alpha", before, after), Paragraph("bravo", before, after)]);

    private static PageTable Table(List<PageParagraph> paragraphs) => new()
    {
        ColumnWidths = [Length.FromTwips(4000)],
        Rows = [new PageTableRow { Cells = [new PageTableCell { Blocks = [.. paragraphs] }] }],
    };

    private static PageParagraph Paragraph(string text, Length before, Length after) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        Format = ParagraphFormat.Default with { SpaceBefore = before, SpaceAfter = after },
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
