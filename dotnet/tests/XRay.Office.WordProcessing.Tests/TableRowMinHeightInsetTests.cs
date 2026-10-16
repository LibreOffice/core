using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a row's declared height is a floor <em>on</em> — the content, or the whole row.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice answers differently depending on where the table came from.
/// <c>DocumentSettingId::MIN_ROW_HEIGHT_INCL_BORDER</c> — its own comment for it is "handle MS Word
/// 'atLeast' oddities" — is set by the DOC, DOCX and RTF filters (<c>ww8par.cxx</c>:1966,
/// <c>DomainMapper.cxx</c>:156) and never by the ODF one. With it on,
/// <c>lcl_CalcMinRowHeight</c> (<c>sw/source/core/layout/tabfrm.cxx</c>:5087-5097) raises the stated
/// height by the top border and both cell margins before applying it, and
/// <c>lcl_GetFixedRowHeight</c> (:5058) raises an <c>exact</c> height by the bottom margin alone —
/// <em>"MS Word also adds the bottom border padding (but not the bottom border line)"</em>.
/// </para>
/// <para>
/// Measured on <c>FAA 2025-26 Holdover Tables.docx</c>, whose Table 54 rows state
/// <c>w:trHeight w:val="397"</c> and a <c>w:tcMar</c> of top 23, bottom 0 under a <c>w:sz="8"</c> grid:
/// 397 + 20 + 23 = 440 twips = 22.00 pt, which is the reference's row pitch exactly. Reading the floor
/// as covering the margins gives 417 twips, and eleven rows of the difference is 12.7 pt — enough to let
/// a three-line note onto a page the reference had to break before.
/// </para>
/// <para>
/// The ODF half is asserted too, because it is the half that fails silently: the two flags are one line
/// apart in the readers and a rule applied to all five formats fixed the <c>table-exact-row</c> fidelity
/// fixture's three Word forms while breaking its two ODF ones.
/// </para>
/// </remarks>
public sealed class TableRowMinHeightInsetTests
{
    private static readonly Length Floor = Length.FromTwips(1000);
    private static readonly Length TopMargin = Length.FromTwips(120);
    private static readonly Length BottomMargin = Length.FromTwips(80);

    /// <summary>
    /// A Word table's <c>atLeast</c> floor is raised by both cell margins before it is applied.
    /// </summary>
    [Fact]
    public void AWordRowsFloorIsRaisedByItsCellMargins()
    {
        Length bare = Height(Table(margins: false, word: true));
        Length inset = Height(Table(margins: true, word: true));

        (inset - bare).ShouldBe(TopMargin + BottomMargin,
            "the floor is a floor on the content, so the margins go on top of it");
    }

    /// <summary>An ODF table's is not: the floor covers the whole row, margins included.</summary>
    [Fact]
    public void AnOdfRowsFloorCoversItsCellMargins()
    {
        Length bare = Height(Table(margins: false, word: false));
        Length inset = Height(Table(margins: true, word: false));

        inset.ShouldBe(bare, "MIN_ROW_HEIGHT_INCL_BORDER is a Word-filter setting");
    }

    /// <summary>
    /// An <c>exact</c> Word row takes the bottom margin and not the top one.
    /// </summary>
    /// <remarks>
    /// The asymmetry is <c>lcl_GetFixedRowHeight</c>'s and is worth pinning because it looks like an
    /// oversight: the obvious symmetry — both margins, as the <c>atLeast</c> branch takes — is wrong.
    /// </remarks>
    [Fact]
    public void AnExactWordRowTakesOnlyTheBottomMargin()
    {
        Length bare = Height(Table(margins: false, word: true, exact: true));
        Length inset = Height(Table(margins: true, word: true, exact: true));

        (inset - bare).ShouldBe(BottomMargin);
    }

    /// <summary>A floor lower than the content still loses to it, margins or no margins.</summary>
    /// <remarks>
    /// The raising must not turn into a second floor of its own: a row whose text already needs more than
    /// the stated height is as tall as its text, which is what <c>atLeast</c> means.
    /// </remarks>
    [Fact]
    public void ContentTallerThanTheRaisedFloorStillWins()
    {
        Length tall = Height(Table(margins: true, word: true, floor: Length.Zero));
        Length floored = Height(Table(margins: true, word: true, floor: Length.FromTwips(1)));

        floored.ShouldBe(tall, "a floor under the content decides nothing");
    }

    private static Length Height(PageTable table)
        => TableLayouter.LayOut(table, new DocPoint(Length.Zero, Length.Zero)).RowHeights[0];

    private static PageTable Table(bool margins, bool word, bool exact = false, Length? floor = null)
        => new()
        {
            ColumnWidths = [Length.FromTwips(4000)],
            MinHeightIncludesInsets = word,
            Rows =
            [
                new PageTableRow
                {
                    MinHeight = floor ?? Floor,
                    HasExactHeight = exact,
                    Cells =
                    [
                        new PageTableCell
                        {
                            Padding = margins
                                ? new CellPadding(Length.Zero, Length.Zero, TopMargin, BottomMargin)
                                : new CellPadding(
                                    Length.Zero, Length.Zero, Length.Zero, Length.Zero),
                            Borders = CellBorders.Uniform(
                                new TableBorder(Length.FromPoints(1), Colour.Black)),
                            Blocks = [Paragraph("row")],
                        },
                    ],
                },
            ],
        };

    private static PageParagraph Paragraph(string text) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        Format = ParagraphFormat.Default,
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
