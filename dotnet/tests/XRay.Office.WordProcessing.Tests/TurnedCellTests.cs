using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A table cell whose text is turned a quarter turn — OOXML's <c>w:textDirection</c>.
/// </summary>
/// <remarks>
/// <para>
/// The rotated row-group label down the side of a table (<c>AIRCRAFT</c>, <c>ENGINES</c>,
/// <c>SPECIALISED SERVICES</c>) is what the property is for, and reading it as upright text is not a
/// cosmetic loss. The paragraph then breaks at the <em>column's</em> width, which for a label column is
/// a few points, so every line holds one glyph and the cell becomes as tall as the label is long. Three
/// corpus documents were failing the gate on exactly that: <c>A1. EASA Form 2.docx</c> at nine pages
/// against seven, and two logbooks whose text layer shattered into 121-124 single-character tokens
/// against the reference's 24-25.
/// </para>
/// <para>
/// Every number asserted here was measured against the installed LibreOffice 26.2.4.2 on generated
/// probes, read out of the PDF's own operators rather than off a raster. The four facts, and what each
/// one refutes:
/// </para>
/// <list type="number">
/// <item>
/// <description>
/// The line breaks at the cell's inner <em>height</em> — frame height less the two half grid lines and
/// less the <em>vertical</em> padding. Horizontal padding does not shorten it. Pinned by a five-twip
/// sweep of <c>w:trHeight</c>: the four-to-five glyph boundary sits at exactly 500 twips in all three of
/// {0.5 pt borders, no borders, 10 pt top and bottom cell margin}, whose frames are 25.5, 25.0 and 45.5
/// pt tall. This refutes "the turn swaps the padding too", which would move the boundary in two of the
/// three.
/// </description>
/// </item>
/// <item>
/// <description>
/// A turned cell contributes <em>nothing</em> to its row's height. Not one line's worth — a row holding
/// only turned cells collapses to zero and LibreOffice draws neither its text nor its borders. This
/// refutes the apparent circularity (the line length is the cell height, the cell height is the tallest
/// cell) and is why the layout can settle in one pass and then place the turned text in a second.
/// </description>
/// </item>
/// <item>
/// <description>
/// A line whose stack offset falls outside the cell is dropped, not clipped: the reference's PDF holds
/// no text-showing operator for it, so it is absent from the text layer as well as from the ink. A 50 pt
/// column whose inner width is 38.7 pt draws four 11.55 pt lines — the fourth overhanging — and not the
/// fifth.
/// </description>
/// </item>
/// <item>
/// <description>
/// <c>w:vAlign</c> aligns the line <em>stack</em> across the cell's width, because that is the same axis
/// in the cell's own frame. Measured at 71.20, 110.00 and 148.80 pt for top, centre and bottom on one
/// fixture.
/// </description>
/// </item>
/// </list>
/// </remarks>
public sealed class TurnedCellTests
{
    /// <summary>
    /// <c>w:textDirection</c> reduces to three answers, and to LibreOffice's three rather than the
    /// specification's six.
    /// </summary>
    /// <remarks>
    /// <c>tbRlV</c> folds onto <c>tbRl</c> and <c>tbLrV</c> is ignored outright — that is what
    /// <c>DomainMapperTableManager.cxx</c>:325-350 does, with the comment "we can't handle these", and
    /// re-measuring against 26.2.4.2 confirms both: <c>tbRlV</c> renders identically to <c>tbRl</c> and
    /// <c>tbLrV</c> identically to no attribute at all. Following the specification instead would turn
    /// text the reference leaves upright.
    /// </remarks>
    [Theory]
    [InlineData("btLr", CellTextDirection.BottomToTopLeftToRight)]
    [InlineData("tbRl", CellTextDirection.TopToBottomRightToLeft)]
    [InlineData("tbRlV", CellTextDirection.TopToBottomRightToLeft)]
    [InlineData("lrTb", CellTextDirection.LeftToRight)]
    [InlineData("lrTbV", CellTextDirection.LeftToRight)]
    [InlineData("tbLrV", CellTextDirection.LeftToRight)]
    [InlineData(null, CellTextDirection.LeftToRight)]
    public void TheReaderMapsEveryTextDirectionTheWayLibreOfficeDoes(
        string? stated, CellTextDirection expected)
    {
        using IDocument document = Open(stated);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PageTable table = pages.Blocks.OfType<PageTable>().Single();

        table.Rows[0].Cells[0].TextDirection.ShouldBe(expected);
    }

    /// <summary>
    /// A turned cell adds nothing at all to its row's height.
    /// </summary>
    /// <remarks>
    /// The label here is long enough that reading it upright would make the cell many lines tall, which is
    /// exactly the defect: at the 500-twip column width the upright reading gives eight single-glyph lines
    /// and a row about 90 pt tall. The row must be as tall as its ordinary neighbour and no taller.
    /// </remarks>
    [Fact]
    public void ATurnedCellDoesNotMakeItsRowTaller()
    {
        Length upright = RowHeight(Table(CellTextDirection.LeftToRight, "AIRCRAFT"));
        Length turned = RowHeight(Table(CellTextDirection.BottomToTopLeftToRight, "AIRCRAFT"));
        Length alone = RowHeight(Table(CellTextDirection.BottomToTopLeftToRight, ""));

        turned.ShouldBe(alone, "the turned cell charges the row exactly what an empty cell charges it");
        upright.ShouldBeGreaterThan(turned, "read upright the same label is many single-glyph lines tall");
    }

    /// <summary>
    /// A row holding nothing but turned cells collapses, and its text is not drawn.
    /// </summary>
    /// <remarks>
    /// The sharpest statement of the rule, and the one that is easiest to get wrong by charging the row
    /// "at least one line". LibreOffice draws no text and no borders for such a row at all.
    /// </remarks>
    [Fact]
    public void ARowOfOnlyTurnedCellsHasNoHeightAndNoText()
    {
        PageTable table = new()
        {
            ColumnWidths = [Length.FromTwips(2000)],
            Rows =
            [
                new PageTableRow
                {
                    Cells =
                    [
                        new PageTableCell
                        {
                            Padding = CellPadding.Word,
                            TextDirection = CellTextDirection.BottomToTopLeftToRight,
                            Blocks = [Paragraph("ROTATED LABEL TEXT")],
                        },
                    ],
                },
            ],
        };

        (List<PlacedTableCell> cells, List<Length> heights) =
            TableLayouter.LayOut(table, DocPoint.Origin);

        heights[0].ShouldBe(Length.Zero);
        cells[0].Content.ShouldBeNull("no line fits in a cell of no height, so none is drawn");
    }

    /// <summary>
    /// The turned text breaks at the cell's inner height, and it is the same inner box an upright cell uses.
    /// </summary>
    /// <remarks>
    /// Asserted as a monotonic response rather than as one number so that it cannot be satisfied by a
    /// constant: a taller row must give the label longer lines and so fewer of them, and a row tall enough
    /// must give it exactly one. The horizontal padding is deliberately large here — if it were subtracted
    /// from the line length, as it would be under a "swap everything" reading of the turn, the tall case
    /// would still wrap.
    /// </remarks>
    [Fact]
    public void TheLineBreaksAtTheCellsInnerHeight()
    {
        int Lines(int rowTwips)
        {
            PageTable table = Table(
                CellTextDirection.BottomToTopLeftToRight,
                "AIRCRAFT",
                rowHeight: Length.FromTwips(rowTwips),
                padding: new CellPadding(Length.FromTwips(400), Length.FromTwips(400),
                                         Length.Zero, Length.Zero),
                labelColumn: Length.FromTwips(3000));

            (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(table, DocPoint.Origin);
            cells[0].Content.ShouldNotBeNull();
            return cells[0].Content!.Lines.Count;
        }

        // A taller row is a longer line, so fewer of them — which is the whole claim, and it is a claim a
        // reading that broke at the column's width could not satisfy at all.
        Lines(300).ShouldBeGreaterThan(Lines(900));
        Lines(900).ShouldBeGreaterThan(Lines(4000));

        // And a row taller than the label is long gives it exactly one line, horizontal padding of
        // 400 twips a side notwithstanding: the turn does not swap the padding.
        Lines(4000).ShouldBe(1);
    }

    /// <summary>
    /// The vertical padding shortens the line and the horizontal padding does not.
    /// </summary>
    /// <remarks>
    /// The single measurement that separates the implemented rule from the plausible wrong one. Pinned
    /// against 26.2.4.2 by a five-twip sweep: 10 pt of top and bottom cell margin moved the four-to-five
    /// glyph boundary not at all when the row grew to keep the same inner height, while the same margin
    /// applied to the line's own direction would have shortened it by 20 pt.
    /// </remarks>
    [Fact]
    public void OnlyTheVerticalPaddingShortensTheLine()
    {
        int Lines(CellPadding padding)
        {
            PageTable table = Table(
                CellTextDirection.BottomToTopLeftToRight,
                "AIRCRAFT",
                rowHeight: Length.FromTwips(1100),
                padding: padding,
                labelColumn: Length.FromTwips(3000));

            (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(table, DocPoint.Origin);
            return cells[0].Content!.Lines.Count;
        }

        CellPadding none = new(Length.Zero, Length.Zero, Length.Zero, Length.Zero);
        CellPadding sides = new(Length.FromTwips(500), Length.FromTwips(500),
                                Length.Zero, Length.Zero);
        CellPadding ends = new(Length.Zero, Length.Zero,
                               Length.FromTwips(500), Length.FromTwips(500));

        Lines(sides).ShouldBe(Lines(none), "padding across the stack cannot shorten the line");
        Lines(ends).ShouldBeGreaterThan(Lines(none), "padding along the line does shorten it");
    }

    /// <summary>
    /// A line that would start outside the cell is not laid down at all.
    /// </summary>
    /// <remarks>
    /// Dropped rather than clipped, which is the half that matters for the word gate: a clipped line is
    /// still in the text layer and still counts. The column here is one line thick, so a label needing two
    /// lines must come back with one.
    /// </remarks>
    [Fact]
    public void ALineStartingPastTheCellsEdgeIsNotDrawn()
    {
        PageTable table = Table(
            CellTextDirection.BottomToTopLeftToRight,
            "AIRCRAFT AND ENGINES AND COMPONENTS",
            rowHeight: Length.FromTwips(400),
            labelColumn: Length.FromTwips(300));

        (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(table, DocPoint.Origin);

        // The label needs several lines at this row height, and the column has room for the start of one.
        cells[0].Content.ShouldNotBeNull();
        cells[0].Content!.Lines.Count.ShouldBe(1, "only one line thickness fits across a 300-twip column");

        // The rest are gone rather than merely out of sight: a clipped line would still be in the text
        // layer, which is the half the word gate scores.
        PageTable roomier = Table(
            CellTextDirection.BottomToTopLeftToRight,
            "AIRCRAFT AND ENGINES AND COMPONENTS",
            rowHeight: Length.FromTwips(400),
            labelColumn: Length.FromTwips(3000));

        TableLayouter.LayOut(roomier, DocPoint.Origin).Cells[0]
            .Content!.Lines.Count.ShouldBeGreaterThan(1, "the same label does need more than one line");
    }

    /// <summary>
    /// The turn is carried as a transform, and it maps the flow's own coordinates onto the page.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The whole of the drawing contract, asserted on the two basis vectors so that neither a wrong sign
    /// nor a swapped axis can pass. For <c>btLr</c> the text advances <em>up</em> the page and the lines
    /// stack <em>rightwards</em>, so a point one unit along the line goes one unit up and a point one unit
    /// down the stack goes one unit right.
    /// </para>
    /// <para>
    /// The origin is the bottom-left of the cell's inner box, because that is the end the text runs from.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheTransformTurnsTheFlowAQuarterTurnAnticlockwise()
    {
        PageTable table = Table(
            CellTextDirection.BottomToTopLeftToRight, "AIRCRAFT", rowHeight: Length.FromTwips(2000));

        (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(table, DocPoint.Origin);

        PlacedTableCell turned = cells[0];
        turned.ContentTransform.ShouldNotBeNull();

        AffineTransform onto = turned.ContentTransform!.Value;

        // One unit along the text goes one unit up the page; one unit down the stack goes one right.
        DocPoint along = Map(onto, Length.FromPoints(10), Length.Zero);
        DocPoint across = Map(onto, Length.Zero, Length.FromPoints(10));
        DocPoint origin = Map(onto, Length.Zero, Length.Zero);

        (origin.Y - along.Y).ShouldBe(Length.FromPoints(10));
        along.X.ShouldBe(origin.X);
        (across.X - origin.X).ShouldBe(Length.FromPoints(10));
        across.Y.ShouldBe(origin.Y);

        // And the origin is the bottom-left of the inner box.
        origin.X.ShouldBe(turned.Area.X + CellPadding.Word.Left);
        origin.Y.ShouldBe(turned.Area.Bottom);
    }

    /// <summary>
    /// The clockwise direction turns the other way, from the opposite corner.
    /// </summary>
    /// <remarks>
    /// <c>tbRl</c>: glyphs run down the page and lines stack leftwards, so the flow starts at the cell's
    /// top-right. Nothing in the sample corpus states it — 111 occurrences across ten documents are all
    /// <c>btLr</c> — but the mapping is measured, and asserting it is what stops the two from being
    /// implemented as one.
    /// </remarks>
    [Fact]
    public void TheClockwiseDirectionTurnsFromTheOppositeCorner()
    {
        PageTable table = Table(
            CellTextDirection.TopToBottomRightToLeft, "AIRCRAFT", rowHeight: Length.FromTwips(2000));

        (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(table, DocPoint.Origin);

        AffineTransform onto = cells[0].ContentTransform!.Value;

        DocPoint origin = Map(onto, Length.Zero, Length.Zero);
        DocPoint along = Map(onto, Length.FromPoints(10), Length.Zero);
        DocPoint across = Map(onto, Length.Zero, Length.FromPoints(10));

        (along.Y - origin.Y).ShouldBe(Length.FromPoints(10), "glyphs run down the page");
        (origin.X - across.X).ShouldBe(Length.FromPoints(10), "lines stack leftwards");
        origin.X.ShouldBe(cells[0].Area.Right - CellPadding.Word.Right);
        origin.Y.ShouldBe(cells[0].Area.Y);
    }

    /// <summary>
    /// <c>w:vAlign</c> moves the line stack across the cell's width rather than down its height.
    /// </summary>
    /// <remarks>
    /// The property keeps its name because that is what every format spells <c>vAlign</c>, and because it
    /// is the same axis in the cell's own frame — but on the page it is horizontal, and a reader that
    /// applied it vertically would leave a centred label hard against the cell's left edge.
    /// </remarks>
    [Fact]
    public void VerticalAlignmentPlacesTheStackAcrossTheCell()
    {
        Length Origin(VerticalTextAlignment alignment)
        {
            PageTable table = Table(
                CellTextDirection.BottomToTopLeftToRight,
                "AIRCRAFT",
                rowHeight: Length.FromTwips(2000),
                labelColumn: Length.FromTwips(3000),
                alignment: alignment);

            (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(table, DocPoint.Origin);
            return Map(cells[0].ContentTransform!.Value, Length.Zero, Length.Zero).X;
        }

        Length top = Origin(VerticalTextAlignment.Top);
        Length middle = Origin(VerticalTextAlignment.Middle);
        Length bottom = Origin(VerticalTextAlignment.Bottom);

        top.ShouldBeLessThan(middle);
        middle.ShouldBeLessThan(bottom);

        // Centred means centred: the two gaps either side of the stack are equal, to the EMU that an
        // odd total leaves over.
        Math.Abs((middle - top).Emu - (bottom - middle).Emu).ShouldBeLessThanOrEqualTo(2L);
    }

    /// <summary>
    /// Moving a placed table moves a turned cell's text with it, exactly once.
    /// </summary>
    /// <remarks>
    /// A turned cell's flow is in the cell's own coordinates, so the shift belongs to the transform and
    /// not to the flow. Shifting both would move the label twice and put it off the page; shifting
    /// neither would leave it wherever the pre-layout pass put it, which is the page's top-left corner.
    /// </remarks>
    [Fact]
    public void OffsettingATableMovesTheTurnedTextOnce()
    {
        PageTable table = Table(
            CellTextDirection.BottomToTopLeftToRight, "AIRCRAFT", rowHeight: Length.FromTwips(2000));

        (List<PlacedTableCell> cells, _) = TableLayouter.LayOut(table, DocPoint.Origin);

        Length dx = Length.FromPoints(40);
        Length dy = Length.FromPoints(90);
        List<PlacedTableCell> moved = TableLayouter.Offset(cells, dx, dy);

        DocPoint before = Map(cells[0].ContentTransform!.Value, Length.Zero, Length.Zero);
        DocPoint after = Map(moved[0].ContentTransform!.Value, Length.Zero, Length.Zero);

        (after.X - before.X).ShouldBe(dx);
        (after.Y - before.Y).ShouldBe(dy);
        moved[0].Content!.Area.ShouldBe(cells[0].Content!.Area, "the flow itself does not move");
    }

    private static DocPoint Map(AffineTransform onto, Length x, Length y)
        => new(
            Length.FromEmu((long)Math.Round((onto.A * x.Emu) + (onto.C * y.Emu) + onto.E)),
            Length.FromEmu((long)Math.Round((onto.B * x.Emu) + (onto.D * y.Emu) + onto.F)));

    private static Length RowHeight(PageTable table)
        => TableLayouter.LayOut(table, DocPoint.Origin).RowHeights[0];

    /// <summary>A label cell beside an ordinary one, so the row has a height of its own.</summary>
    private static PageTable Table(
        CellTextDirection direction,
        string label,
        Length? rowHeight = null,
        CellPadding? padding = null,
        Length? labelColumn = null,
        VerticalTextAlignment alignment = VerticalTextAlignment.Top)
        => new()
        {
            ColumnWidths = [labelColumn ?? Length.FromTwips(500), Length.FromTwips(6000)],
            Rows =
            [
                new PageTableRow
                {
                    MinHeight = rowHeight ?? Length.Zero,
                    Cells =
                    [
                        new PageTableCell
                        {
                            Column = 0,
                            Padding = padding ?? CellPadding.Word,
                            TextDirection = direction,
                            VerticalAlignment = alignment,
                            Blocks = label.Length == 0 ? [] : [Paragraph(label)],
                        },
                        new PageTableCell
                        {
                            Column = 1,
                            Padding = CellPadding.Word,
                            Blocks = [Paragraph("body")],
                        },
                    ],
                },
            ],
        };

    private static IDocument Open(string? direction)
    {
        MemoryStream package = BuildPackage(direction);
        using DocumentSource source = DocumentSource.FromStream(package, "turned-cell.docx");
        return new WordProcessingReader().Read(source);
    }

    /// <summary>
    /// A one-cell table stating (or not stating) <c>w:textDirection</c>, and a second ordinary cell so
    /// the row has a height that does not depend on the answer.
    /// </summary>
    private static MemoryStream BuildPackage(string? direction)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        string stated = direction is null
            ? string.Empty
            : $"""<w:textDirection w:val="{direction}"/>""";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblPr><w:tblW w:w="8000" w:type="dxa"/><w:tblLayout w:type="fixed"/></w:tblPr>
                  <w:tblGrid><w:gridCol w:w="500"/><w:gridCol w:w="7500"/></w:tblGrid>
                  <w:tr>
                    <w:tc><w:tcPr><w:tcW w:w="500" w:type="dxa"/>{stated}</w:tcPr>
                      <w:p><w:r><w:t>AIRCRAFT</w:t></w:r></w:p></w:tc>
                    <w:tc><w:tcPr><w:tcW w:w="7500" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>body</w:t></w:r></w:p></w:tc>
                  </w:tr>
                </w:tbl>
                <w:p/>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }

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
