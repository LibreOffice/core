using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A DOC section can state a width for each of its columns instead of asking for equal ones.
/// </summary>
/// <remarks>
/// <para>
/// <c>sprmSFEvenlySpaced</c> defaults to on, and when it is off Word writes a <c>sprmSDxaColWidth</c> per
/// column and a <c>sprmSDxaColSpacing</c> after every column but the last. LibreOffice reads exactly that
/// (<c>SwWW8ImplReader::SetCols</c>, <c>sw/source/filter/ww8/ww8par6.cxx</c>:994-1032) and applies it to a
/// <c>SwFormatCol</c> whose columns are no longer <c>Ortho</c> (:449-466).
/// </para>
/// <para>
/// Measured on <c>absrc-pac-01-info-note-en.doc</c>, whose second section states 2880 and 5760 twips
/// either side of a 720-twip gap: read as even columns its two halves are 216 pt each, and LibreOffice
/// draws them 144 pt and 288 pt. Its centred <c>INFORMATION HIGHLIGHTS</c> sits at x 311.00 in the
/// reference and the even reading predicts 94.86 — a hundred and sixteen points of column, not a rounding.
/// </para>
/// </remarks>
public sealed class DocUnevenColumnTests
{
    /// <summary>The stated widths are read, in the order the section names them.</summary>
    [Fact]
    public void AnUnevenlySpacedSectionStatesEachColumnsWidth()
    {
        WritingSection section = Ww8SectionTable.ReadProperties(UnevenTwoColumns);

        section.Page.Columns.ShouldBe(2);
        section.Page.ColumnRuler.ShouldNotBeNull();
        section.Page.ColumnRuler!.Widths.ShouldBe([Length.FromTwips(2880), Length.FromTwips(5760)]);
        section.Page.ColumnRuler!.Gaps.ShouldBe([Length.FromTwips(720)]);
    }

    /// <summary>A section that does not say otherwise still asks for equal columns.</summary>
    /// <remarks>
    /// The control, and the reason the widths are collected whether the flag is set or not: a section can
    /// carry a stale set left by an earlier edit, and reading those would re-cut a page Word lays out even.
    /// </remarks>
    [Fact]
    public void AnEvenlySpacedSectionCarriesNoRuler()
    {
        byte[] evenly = [.. UnevenTwoColumns, .. EvenlySpaced(on: true)];

        Ww8SectionTable.ReadProperties(evenly).Page.ColumnRuler.ShouldBeNull();
    }

    /// <summary>A section missing one of its widths falls back to even columns.</summary>
    /// <remarks>
    /// LibreOffice substitutes a plain inch for a missing <c>sprmSDxaColWidth</c>, which is unrelated to
    /// the measure and worse than even columns on every page size but one. A half-filled ruler is
    /// therefore no ruler at all.
    /// </remarks>
    [Fact]
    public void ASectionMissingAWidthFallsBackToEvenColumns()
    {
        byte[] partial =
        [
            .. TopMarginOnly, .. ColumnCount(2), .. EvenlySpaced(on: false),
            .. ColumnWidth(0, 2880), .. ColumnSpacing(0, 720),
        ];

        Ww8SectionTable.ReadProperties(partial).Page.ColumnRuler.ShouldBeNull();
    }

    /// <summary>The columns are drawn at the widths and offsets the section states.</summary>
    /// <remarks>
    /// US Letter with an inch of margin leaves 9360 twips, which is exactly what 2880 + 720 + 5760 comes
    /// to — so this asserts the stated numbers rather than an apportioning of them.
    /// </remarks>
    [Fact]
    public void TheColumnsAreDrawnAtTheStatedWidths()
    {
        PageGeometry page = Uneven();

        page.TextWidth.ShouldBe(Length.FromTwips(9360));
        page.ColumnArea(0).Width.ShouldBe(Length.FromTwips(2880));
        page.ColumnArea(1).Width.ShouldBe(Length.FromTwips(5760));

        page.ColumnArea(0).X.ShouldBe(page.Margins.Left);
        page.ColumnArea(1).X.ShouldBe(page.Margins.Left + Length.FromTwips(2880 + 720));
    }

    /// <summary>Line breaking follows the column, not the section's first column.</summary>
    [Fact]
    public void EachColumnBreaksAtItsOwnWidth()
    {
        PageGeometry page = Uneven();

        page.ColumnWidthAt(0).ShouldBe(Length.FromTwips(2880));
        page.ColumnWidthAt(1).ShouldBe(Length.FromTwips(5760));
    }

    /// <summary>A right-to-left section puts its first stated column on the right.</summary>
    /// <remarks>
    /// The ruler is read leading-edge-first like everything else, so reversing the index is all the
    /// direction does — the widths themselves are not mirrored.
    /// </remarks>
    [Fact]
    public void ARightToLeftSectionStartsWithTheRightmostColumn()
    {
        PageGeometry page = Uneven() with { IsRightToLeft = true };

        page.ColumnArea(0).Width.ShouldBe(Length.FromTwips(5760));
        page.ColumnArea(0).X.ShouldBe(page.Margins.Left + Length.FromTwips(2880 + 720));
        page.ColumnArea(1).X.ShouldBe(page.Margins.Left);
    }

    /// <summary>
    /// Widths stated against a wider measure than the section's own are apportioned to it.
    /// </summary>
    /// <remarks>
    /// Writer's own behaviour: <c>SwFormatCol</c> holds wish widths and divides the frame it is given
    /// between them, leaving the gaps alone. Here the same one-to-two split is asked for on a measure
    /// 1440 twips narrower: the gap keeps its 720, and the 7200 twips left over divide one to two into
    /// 2400 and 4800.
    /// </remarks>
    [Fact]
    public void StatedWidthsAreApportionedToTheMeasureTheSectionActuallyHas()
    {
        PageGeometry page = Uneven() with
        {
            Margins = PageGeometry.Letter.Margins with { Right = Length.FromTwips(1440 * 2) },
        };

        page.TextWidth.ShouldBe(Length.FromTwips(7920));
        page.ColumnArea(0).Width.ShouldBe(Length.FromTwips(2400));
        page.ColumnArea(1).Width.ShouldBe(Length.FromTwips(4800));
        (page.ColumnArea(1).X - page.ColumnArea(0).Right).ShouldBe(Length.FromTwips(720));
    }

    /// <summary>A ruler naming more columns than the section has is ignored rather than trusted.</summary>
    [Fact]
    public void ARulerThatDisagreesWithTheColumnCountIsIgnored()
    {
        PageGeometry page = Uneven() with { Columns = 3 };

        page.Ruler.ShouldBeNull();
        page.ColumnArea(0).Width.ShouldBe(page.ColumnArea(1).Width);
    }

    /// <summary>A page carries the ruler through to whoever draws it.</summary>
    [Fact]
    public void APageDrawsItsColumnsFromTheRuler()
    {
        PageGeometry geometry = Uneven();

        Layout.LaidOutPage page = new()
        {
            Index = 0,
            Number = 1,
            Size = geometry.Size,
            BodyArea = geometry.TextArea,
            ColumnCount = 2,
            ColumnGap = Length.FromTwips(720),
            ColumnRuler = geometry.Ruler,
            Lines = [],
        };

        page.ColumnArea(0).Width.ShouldBe(Length.FromTwips(2880));
        page.ColumnArea(1).X.ShouldBe(
            page.BodyArea.X + Length.FromTwips(2880 + 720));
    }

    /// <summary>
    /// A paragraph that runs out of one column finishes in the next, whatever width that one is.
    /// </summary>
    /// <remarks>
    /// The trap in breaking each column at its own width: the blocks are broken up front and a
    /// part-placed paragraph is carried across on a line index that counts into <em>that</em> list of
    /// lines. Handing back a list broken to the next column's width indexes a different line, or — when
    /// the wider column needed fewer of them — none at all, which is an
    /// <c>ArgumentOutOfRangeException</c> out of the paginator rather than a misplaced line. Caught on
    /// <c>150_5300_13_chg8.doc</c>, whose opening stretch states 2880 and 5760 twips and whose first
    /// paragraph crosses between them.
    /// </remarks>
    [Fact]
    public void AParagraphCrossingBetweenColumnsOfDifferentWidthsIsStillPlaced()
    {
        // The narrow column holds far more lines than the whole paragraph needs in the wide one, which is
        // the arrangement that turns a stale line index into an overrun rather than a misplaced line.
        PageGeometry geometry = Uneven();

        PageParagraph paragraph = new()
        {
            Text = string.Join(' ', Enumerable.Repeat("column crossing paragraph text", 60)),
            Face = Face,
            EmSize = Length.FromPoints(11),
        };

        List<Layout.LaidOutPage> pages = new Paginator(PaginationOptions.Word).Paginate(
            [paragraph],
            [new PaginatedSection(new WritingSection { Page = geometry })]);

        pages.ShouldNotBeEmpty();
        pages[0].Lines.Select(line => line.Column).Distinct().Count().ShouldBe(2);

        // Every line of the paragraph is placed exactly once, wherever the columns cut it.
        pages.SelectMany(page => page.Lines).Select(line => line.LineIndex).Distinct().Count()
            .ShouldBe(pages.Sum(page => page.Lines.Count));
    }

    /// <summary>US Letter, one-inch margins, and the two columns the corpus document states.</summary>
    private static PageGeometry Uneven()
        => PageGeometry.Letter with
        {
            Columns = 2,
            ColumnGap = Length.FromTwips(720),
            ColumnRuler = Ww8SectionTable.ReadProperties(UnevenTwoColumns).Page.ColumnRuler,
        };

    /// <summary>Two columns of 2880 and 5760 twips either side of a 720-twip gap.</summary>
    private static byte[] UnevenTwoColumns =>
    [
        .. TopMarginOnly, .. ColumnCount(2), .. EvenlySpaced(on: false),
        .. ColumnWidth(0, 2880), .. ColumnSpacing(0, 720), .. ColumnWidth(1, 5760),
    ];

    /// <summary><c>sprmSDyaTop</c> at Word's own inch, so the grpprl starts on a sprm this file can see.</summary>
    private static byte[] TopMarginOnly => [0x23, 0x90, 0xA0, 0x05];

    /// <summary><c>sprmSCcolumns</c>, whose operand is the count less one.</summary>
    private static byte[] ColumnCount(int columns)
        => [0x0B, 0x50, (byte)(columns - 1), 0x00];

    /// <summary><c>sprmSFEvenlySpaced</c>, one byte.</summary>
    private static byte[] EvenlySpaced(bool on) => [0x05, 0x30, on ? (byte)1 : (byte)0];

    /// <summary><c>sprmSDxaColWidth</c>: the column index, then two bytes of twips.</summary>
    private static byte[] ColumnWidth(int column, int twips)
        => [0x03, 0xF2, (byte)column, (byte)(twips & 0xFF), (byte)(twips >> 8)];

    /// <summary><c>sprmSDxaColSpacing</c>, in the same shape.</summary>
    private static byte[] ColumnSpacing(int column, int twips)
        => [0x04, 0xF2, (byte)column, (byte)(twips & 0xFF), (byte)(twips >> 8)];

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
