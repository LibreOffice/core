using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A multi-column section between two continuous breaks shares its text evenly between its columns.
/// </summary>
/// <remarks>
/// <para>
/// The defect these were written against: <c>batch-016/150_5300_13_chg8.doc</c> lays its opening
/// two-column stretch down column one alone, where LibreOffice puts seven lines in column one at
/// x 72.1 and the rest in column two at x 341.8. Writer models the stretch as a
/// <c>SwSectionFrame</c> and searches for the height that holds it
/// (<c>SwLayoutFrame::FormatWidthCols</c>, <c>sw/source/core/layout/wsfrm.cxx</c>:3912); this
/// bisects for the same quantity.
/// </para>
/// <para>
/// Each of these was checked by putting the defect back and watching it fail — see the round's entry in
/// <c>TODO.batches.md</c> for which reintroduction each one catches.
/// </para>
/// </remarks>
public sealed class BalancedColumnTests
{
    /// <summary>A balanced section fills both of its columns instead of only the first.</summary>
    [Fact]
    public void BothColumnsAreUsed()
    {
        LaidOutPage page = Paginate(balances: true)[0];

        Columns(page).ShouldBe([0, 1]);
    }

    /// <summary>An unbalanced section fills column one to the bottom before starting column two.</summary>
    /// <remarks>
    /// The control for the one above and a detector in its own right: a change that balanced *every*
    /// multi-column section would pass <see cref="BothColumnsAreUsed"/> and fail this.
    /// </remarks>
    [Fact]
    public void AnUnbalancedSectionIsNotShared()
    {
        LaidOutPage page = Paginate(balances: false)[0];

        // Six lines is far less than a page, so an unbalanced section never reaches its second column.
        Columns(page).ShouldBe([0]);
    }

    /// <summary>The two columns come out within one line of each other.</summary>
    /// <remarks>
    /// The property that says the height was *searched* rather than merely divided: six lines split as
    /// three and three, not five and one. Counting lines rather than measuring the height keeps the
    /// assertion independent of the face's metrics.
    /// </remarks>
    [Fact]
    public void TheColumnsAreWithinOneLineOfEachOther()
    {
        LaidOutPage page = Paginate(balances: true)[0];

        int first = page.Lines.Count(line =>
            line.Column == 0 && line.ParagraphIndex >= SectionStart && line.ParagraphIndex < SectionEnd);

        int second = page.Lines.Count(line => line.Column == 1);

        Math.Abs(first - second).ShouldBeLessThanOrEqualTo(1);
        (first + second).ShouldBe(SectionLines);
    }

    /// <summary>Both columns of a balanced section start at the same height, not at the page's top.</summary>
    /// <remarks>
    /// A section that opens half way down a page is a box inside the page, so its second column starts
    /// beside its first rather than at the top margin. Resetting the running height to nought at a column
    /// break — right for a page laid out in columns — puts the second column a section's worth too high.
    /// </remarks>
    [Fact]
    public void TheSecondColumnStartsBesideTheFirst()
    {
        LaidOutPage page = Paginate(balances: true)[0];

        Length first = page.Lines
            .Where(line =>
                line.Column == 0 && line.ParagraphIndex >= SectionStart
                && line.ParagraphIndex < SectionEnd)
            .Min(line => line.Top);

        Length second = page.Lines.Where(line => line.Column == 1).Min(line => line.Top);

        second.ShouldBe(first);
        first.ShouldBeGreaterThan(Length.Zero);
    }

    /// <summary>The section that follows starts below the balanced box, not beside it.</summary>
    [Fact]
    public void WhatFollowsStartsBelowTheWholeBox()
    {
        LaidOutPage page = Paginate(balances: true)[0];

        Length deepest = page.Lines
            .Where(line => line.ParagraphIndex >= SectionStart && line.ParagraphIndex < SectionEnd)
            .Max(line => line.Top);

        PlacedLine after = page.Lines.First(line => line.ParagraphIndex >= SectionEnd);

        after.Top.ShouldBeGreaterThan(deepest);
        after.Column.ShouldBe(0);
        after.Columns.ShouldBe(1);
    }

    /// <summary>
    /// The box is tall enough to hold the last paragraph's space-after, not only its lines.
    /// </summary>
    /// <remarks>
    /// The fitting rules count line boxes and a paragraph's space-after is added to the running height
    /// afterwards, so a band chosen from the lines alone ends a paragraph gap too high. Measured at 10 pt
    /// on the corpus document; here the section's last paragraph states 12 pt and the assertion is that
    /// what follows clears the last line by at least that much.
    /// </remarks>
    [Fact]
    public void TheBoxCountsTheLastParagraphsSpaceAfter()
    {
        LaidOutPage page = Paginate(balances: true, spaceAfter: Length.FromPoints(12))[0];

        Length deepestTop = page.Lines
            .Where(line => line.ParagraphIndex >= SectionStart && line.ParagraphIndex < SectionEnd)
            .Max(line => line.Top);

        Length deepestBottom = page.Lines
            .Where(line => line.ParagraphIndex >= SectionStart && line.ParagraphIndex < SectionEnd)
            .Where(line => line.Top == deepestTop)
            .Max(line => line.Top + line.Box.Height);

        PlacedLine after = page.Lines.First(line => line.ParagraphIndex >= SectionEnd);

        (after.Top - deepestBottom).ShouldBeGreaterThanOrEqualTo(Length.FromPoints(11.9));
    }

    /// <summary>
    /// A section whose trailing space cannot fit in <em>any</em> band still ends the search.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The bisection has two verdicts and only one of them can stop: <c>Fits</c> settles once the bounds
    /// meet, and <c>TooShort</c> has nothing to settle, because "too short" is never an answer on its own.
    /// So a trial at the tallest band there is — the section's whole remaining height — that still comes
    /// out too short hands back the same candidate for ever, restoring the fill to the section's first
    /// block each time round. It is a hang rather than a slow path, and nothing above the paginator can
    /// see it: no page is emitted, so <c>MaxPages</c> never bites.
    /// </para>
    /// <para>
    /// Reachable whenever the last paragraph's space-after exceeds what is left of the body, which is why
    /// it stayed latent — <c>150_5300_13_chg10.doc</c> paginated in ten seconds until a running head
    /// shortened its body, and then never finished at all.
    /// </para>
    /// </remarks>
    [Fact]
    public async Task ASectionOverhangingEveryBandStillEndsTheSearch()
    {
        Task<List<LaidOutPage>> run = Task.Run(
            () => Paginate(balances: true, spaceAfter: Length.FromPoints(1000)));

        Task first = await Task.WhenAny(run, Task.Delay(TimeSpan.FromSeconds(30), TestContext.Current.CancellationToken));

        first.ShouldBeSameAs(
            run, "the search has to end even when no band holds the section's trailing space");

        LaidOutPage page = (await run)[0];

        // And it ends by keeping the trial's placement rather than by dropping the section. Which column
        // the lines land in is deliberately not asserted: no trial ever fitted, so there is no balanced
        // placement to keep, and filling in order is what an unbalanced section does anyway.
        page.Lines.Count(line =>
                line.ParagraphIndex >= SectionStart && line.ParagraphIndex < SectionEnd)
            .ShouldBe(SectionLines);
    }

    /// <summary>A line records the columns it was placed in, not the ones the page ends up with.</summary>
    /// <remarks>
    /// One page holds all three sections here, and the page is written when the last of them is current.
    /// Reading the count off the page put "two columns" on the single-column paragraphs above and below
    /// the stretch, which draws a full-width paragraph into half a column.
    /// </remarks>
    [Fact]
    public void EachLineCarriesItsOwnColumnCount()
    {
        LaidOutPage page = Paginate(balances: true)[0];

        page.Lines.Where(line => line.ParagraphIndex < SectionStart)
            .ShouldAllBe(line => line.Columns == 1);

        page.Lines.Where(line =>
                line.ParagraphIndex >= SectionStart && line.ParagraphIndex < SectionEnd)
            .ShouldAllBe(line => line.Columns == 2);

        page.Lines.Where(line => line.ParagraphIndex >= SectionEnd)
            .ShouldAllBe(line => line.Columns == 1);
    }

    /// <summary>The two columns are drawn side by side, at the width the gap leaves them.</summary>
    [Fact]
    public void TheColumnsAreDrawnSideBySide()
    {
        LaidOutPage page = Paginate(balances: true)[0];

        PlacedLine first = page.Lines.First(line => line.Column == 0 && line.Columns == 2);
        PlacedLine second = page.Lines.First(line => line.Column == 1);

        DocRect left = page.ColumnArea(first);
        DocRect right = page.ColumnArea(second);

        left.Width.ShouldBe(right.Width);
        (right.X - left.X - left.Width).ShouldBe(Gap);

        // And a single-column line still spans the whole measure.
        page.ColumnArea(page.Lines[0]).Width.ShouldBe(page.BodyArea.Width);
    }

    /// <summary>Where the balanced section's blocks start and end, as block indices.</summary>
    private const int SectionStart = 2;

    /// <inheritdoc cref="SectionStart"/>
    private const int SectionEnd = SectionStart + SectionLines;

    /// <summary>How many one-line paragraphs the balanced section holds.</summary>
    private const int SectionLines = 6;

    private static Length Gap { get; } = Length.FromTwips(708);

    /// <summary>
    /// Three sections on one page: a single-column opening, a two-column stretch, a single-column close.
    /// </summary>
    private static List<LaidOutPage> Paginate(bool balances, Length spaceAfter = default)
    {
        List<PageBlock> blocks = [];

        for (int i = 0; i < SectionStart; i++) blocks.Add(Paragraph($"Opening {i}", 0));

        for (int i = 0; i < SectionLines; i++)
        {
            blocks.Add(Paragraph(
                $"Stretch {i}", 1, i == SectionLines - 1 ? spaceAfter : default));
        }

        blocks.Add(Paragraph("Closing", 2));

        PageGeometry oneColumn = Geometry;
        PageGeometry twoColumns = Geometry with { Columns = 2, ColumnGap = Gap };

        return new Paginator(PaginationOptions.Word).Paginate(
            blocks,
            [
                new PaginatedSection(new WritingSection { Page = oneColumn }),
                new PaginatedSection(new WritingSection
                {
                    Page = twoColumns,
                    Break = SectionBreak.Continuous,
                    BalancesColumns = balances,
                }),
                new PaginatedSection(new WritingSection
                {
                    Page = oneColumn,
                    Break = SectionBreak.Continuous,
                }),
            ]);
    }

    private static PageParagraph Paragraph(string text, int section, Length spaceAfter = default)
        => new()
        {
            Text = text,
            Face = Face,
            EmSize = Length.FromPoints(11),
            SectionIndex = section,
            Format = ParagraphFormat.Default with { SpaceAfter = spaceAfter },
        };

    /// <summary>Which columns the page's lines landed in, in order and without repeats.</summary>
    private static int[] Columns(LaidOutPage page)
        => [.. page.Lines.Select(line => line.Column).Distinct().Order()];

    /// <summary>An A4 page with an inch of margin.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
