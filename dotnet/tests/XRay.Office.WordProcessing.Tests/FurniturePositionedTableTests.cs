using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A positioned table in a running head, which is a frame rather than a block in the flow.
/// </summary>
/// <remarks>
/// <para>
/// Writer's DOCX importer turns a <c>w:tblpPr</c> table into a fly holding a table — visible in
/// <c>--convert-to fodt</c> as a <c>draw:frame</c> whose style carries <c>w:bottomFromText</c> as its
/// <c>fo:margin-bottom</c> — and in a header the anchor paragraph's text does not move out of its way.
/// So the head's height is <c>max(in-flow content height, frame bottom + the frame's lower spacing)</c>
/// rather than the two stacked.
/// </para>
/// <para>
/// Measured on the installed 26.2.4.2 by perturbing that flat XML and re-rendering
/// <c>words/batch-010/docx/5709.16 ch.40_mgfinal.docx</c>: the body's first line moves one for one with
/// the frame's lower spacing (0 → 114.54 pt, 403 twips → 134.69 pt, 1 in → 186.54 pt), does not move
/// with the frame's <em>upper</em> spacing, and does not move when the anchor paragraph grows from 8 pt
/// to 20 pt — only once the paragraph is taller than the frame does it decide. Text put into that
/// paragraph draws at the very top of the header, overlapping the frame.
/// </para>
/// <para>
/// Stacking the two instead made that document's head 10.95 pt short on all 31 pages — the table's
/// height plus a 9.20 pt empty paragraph where Writer takes the table's height plus its 20.15 pt lower
/// spacing — which is exactly the page it was losing. It renders 32 of 32 now.
/// </para>
/// <para>
/// Each of these was verified by putting the defect back, and the split is exact. Reverting
/// <c>FlowLayouter</c> and <c>PageFurnitureSet</c> alone fails the three that are about the head's
/// layout — <see cref="APositionedTableInARunningHeadDoesNotStackWithTheParagraphBesideIt"/>,
/// <see cref="ATallParagraphBesideAPositionedTableDecidesTheHeadsHeight"/> and
/// <see cref="TheParagraphBesideAPositionedTableStartsAtTheTopOfTheHead"/>. Reverting the reader as
/// well fails <see cref="ThePositionedTablesLowerSpacingIsRead"/> and
/// <see cref="APositionedTableNeedNotStateASpacing"/> on top of them, five of eight. The three that
/// never fail are the controls, which is their point.
/// </para>
/// </remarks>
public sealed class FurniturePositionedTableTests
{
    /// <summary>
    /// A running head's height is the positioned table's reach, not the table plus what follows it.
    /// </summary>
    [Fact]
    public void APositionedTableInARunningHeadDoesNotStackWithTheParagraphBesideIt()
    {
        PlacedFlow head = Head(Table(positioned: true, lower: Lower), ShortParagraph);

        head.Advance.ShouldBe(RowHeight + Lower);
    }

    /// <summary>
    /// And a paragraph taller than the table decides instead, which is the other half of the
    /// <c>max</c>: the table is out of the flow, not ignored by it.
    /// </summary>
    [Fact]
    public void ATallParagraphBesideAPositionedTableDecidesTheHeadsHeight()
    {
        PlacedFlow head = Head(Table(positioned: true, lower: Lower), TallParagraph);

        head.Advance.ShouldBeGreaterThan(RowHeight + Lower);
        head.Advance.ShouldBe(Head(TallParagraph).Advance);
    }

    /// <summary>
    /// The paragraph beside a positioned table starts at the top of the head rather than below it.
    /// </summary>
    /// <remarks>
    /// The measurement this comes from is the anchor text drawn at <c>yMin</c> 36.26 pt in a header
    /// whose own top is 36.06 — overlapping the frame. A stacking layouter puts it a whole table lower,
    /// so this pins the placement as well as the height.
    /// </remarks>
    [Fact]
    public void TheParagraphBesideAPositionedTableStartsAtTheTopOfTheHead()
    {
        PlacedFlow head = Head(Table(positioned: true, lower: Lower), ShortParagraph);

        head.Lines.Count.ShouldBe(1);
        head.Lines[0].Top.ShouldBe(Length.Zero);
    }

    /// <summary>The control: an ordinary table in a running head still stacks.</summary>
    /// <remarks>
    /// Here so that the change cannot be mistaken for having taken every table out of the flow, and so
    /// that a rewrite which loses the distinction fails something.
    /// </remarks>
    [Fact]
    public void AnOrdinaryTableInARunningHeadStillStacks()
    {
        PlacedFlow head = Head(Table(positioned: false, lower: Lower), ShortParagraph);

        head.Advance.ShouldBe(RowHeight + Head(ShortParagraph).Advance);
        head.Lines[0].Top.ShouldBe(RowHeight);
    }

    /// <summary>The other control: the body still stacks a positioned table.</summary>
    /// <remarks>
    /// The body is deliberately outside this rule — there Writer's fly does wrap the anchor's text, so
    /// the text goes below the table, which is what stacking already approximates. 21 corpus documents
    /// hold a positioned table in the body against 4 in a header or foot, so this is the larger half of
    /// the corpus and it is untouched.
    /// </remarks>
    [Fact]
    public void TheBodyStillStacksAPositionedTable()
    {
        List<LaidOutPage> pages = new Paginator(PaginationOptions.Word).Paginate(
            [Table(positioned: true, lower: Lower), ShortParagraph],
            new WritingSection { Page = Geometry });

        // The paragraph sits below the table rather than beside it. Measured against the table the page
        // actually placed, since what the body makes of an exact row height is not this test's subject —
        // and in page coordinates, because a line's own top is relative to the body area and a placed
        // table's rectangle is not.
        pages[0].Lines.Count.ShouldBe(1);
        pages[0].Tables.Count.ShouldBe(1);
        (pages[0].BodyArea.Top + pages[0].Lines[0].Top)
            .ShouldBeGreaterThanOrEqualTo(pages[0].Tables[0].Area.Bottom);
    }

    /// <summary><c>w:tblpPr</c> makes a table positioned, and <c>w:bottomFromText</c> is its spacing.</summary>
    [Fact]
    public void ThePositionedTablesLowerSpacingIsRead()
    {
        PageTable table = ReadTable(
            """<w:tblpPr w:leftFromText="187" w:bottomFromText="403" w:vertAnchor="text" w:tblpY="1"/>""");

        table.IsPositioned.ShouldBeTrue();
        table.LowerSpacing.ShouldBe(Length.FromTwips(403));
    }

    /// <summary>
    /// A positioned table stating no <c>w:bottomFromText</c> is still positioned, with no spacing.
    /// </summary>
    /// <remarks>
    /// Not a corner: two of the corpus's four running heads holding a positioned table state no
    /// <c>w:tblpXSpec</c> and no <c>w:bottomFromText</c> either, so reading the flag off the alignment
    /// or off the spacing would have missed both.
    /// </remarks>
    [Fact]
    public void APositionedTableNeedNotStateASpacing()
    {
        PageTable table = ReadTable("""<w:tblpPr w:vertAnchor="page" w:horzAnchor="page" w:tblpX="712"/>""");

        table.IsPositioned.ShouldBeTrue();
        table.HorizontalPosition.ShouldBeNull();
        table.LowerSpacing.ShouldBe(Length.Zero);
    }

    /// <summary>And an ordinary table is not positioned.</summary>
    [Fact]
    public void AnOrdinaryTableIsNotPositioned()
    {
        PageTable table = ReadTable("");

        table.IsPositioned.ShouldBeFalse();
        table.LowerSpacing.ShouldBe(Length.Zero);
    }

    /// <summary>A one-row, one-cell table of an exact height, positioned or not.</summary>
    private static PageTable Table(bool positioned, Length lower) => new()
    {
        ColumnWidths = [Length.FromTwips(6000)],
        IsPositioned = positioned,
        LowerSpacing = lower,
        Rows =
        [
            new PageTableRow
            {
                Cells = [new PageTableCell { Blocks = [] }],
                MinHeight = RowHeight,
                HasExactHeight = true,
            },
        ],
    };

    /// <summary>The blocks laid out as a running head, at the geometry's header area.</summary>
    private static PlacedFlow Head(params PageBlock[] blocks)
    {
        PageFurnitureSet furniture = new(
            new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = blocks,
            });

        PlacedFlow? head = furniture.Header(
            new WritingSection { Page = Geometry }, Geometry, pageNumber: 1, isFirstPageOfSection: true);

        head.ShouldNotBeNull();
        return head!;
    }

    /// <summary>The one table of a <c>w:body</c> holding a table with the given <c>w:tblPr</c> children.</summary>
    private static PageTable ReadTable(string tableProperties)
    {
        const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

        XElement body = XElement.Parse($"""
            <w:body xmlns:w="{W}">
              <w:tbl>
                <w:tblPr><w:tblW w:w="6000" w:type="dxa"/>{tableProperties}</w:tblPr>
                <w:tblGrid><w:gridCol w:w="6000"/></w:tblGrid>
                <w:tr><w:tc><w:p><w:r><w:t>cell</w:t></w:r></w:p></w:tc></w:tr>
              </w:tbl>
            </w:body>
            """);

        return new DocxLayoutSource(new WordStyles()).Read(body).OfType<PageTable>().Single();
    }

    /// <summary>Shorter than the table, so the table decides the head's height.</summary>
    private static PageParagraph ShortParagraph => Paragraph("head", Length.FromPoints(8));

    /// <summary>Taller than the table, so the paragraph does.</summary>
    private static PageParagraph TallParagraph => Paragraph("head", Length.FromPoints(60));

    private static PageParagraph Paragraph(string text, Length size) => new()
    {
        Text = text,
        Face = Face,
        EmSize = size,
    };

    /// <summary>Half an inch, comfortably taller than an 8 pt line and shorter than a 60 pt one.</summary>
    private static Length RowHeight { get; } = Length.FromInches(0.5);

    /// <summary>The corpus document's own <c>w:bottomFromText</c>, 403 twips.</summary>
    private static Length Lower { get; } = Length.FromTwips(403);

    /// <summary>Letter, with the head half an inch into a one inch top margin.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(12240), Length.FromTwips(15840)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
