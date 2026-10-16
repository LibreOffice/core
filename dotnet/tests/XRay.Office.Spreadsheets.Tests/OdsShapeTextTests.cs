using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A drawing shape anchored in an ODF cell is in the drawing layer, not in the cell.
/// </summary>
/// <remarks>
/// <para>
/// ODF fastens a cell-anchored object by containment, so a <c>draw:custom-shape</c> is a child of
/// the <c>table:table-cell</c> it belongs to and a walk that reads the cell reads the shape. That
/// is right for extraction — the words are in that cell of that sheet — and wrong for every
/// question the layout asks about a cell, because in Calc the object is an <c>SdrObject</c> on the
/// sheet's draw page and <c>ScColumn::GetOptimalHeight</c> walks the column's own cell storage
/// (<c>sc/source/core/data/column2.cxx</c>:894-949).
/// </para>
/// <para>
/// Reading it as cell text cost two corpus documents their page count once a multi-paragraph cell
/// began sizing its row: <c>EHEST-Pre-departure-checklist</c> went to 27 pages against 24 and
/// <c>SSRO_Quarterly_Statistical_Bulletin_Q3201617_DATA</c> to 11 against 4, and neither holds a
/// multi-paragraph <em>cell</em> at all. <strong>604 <c>draw:custom-shape</c> sit in the cells of
/// 54 of the 307 converted <c>.ods</c>, and 137 of them in 32 documents carry text.</strong>
/// </para>
/// <para>
/// The fixture's own header carries LibreOffice 26.2.4.2's numbers for it, and the variants they
/// were established with.
/// </para>
/// </remarks>
public sealed class OdsShapeTextTests
{
    private static SheetLayout Sheet()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-shape-text.fods"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    private static ContentTableCell CellAt(SheetLayout sheet, int row, int column)
        => sheet.CellAt(row, column).ShouldNotBeNull($"a cell at row {row}, column {column}");

    /// <summary>
    /// The shape's words stay in the cell for extraction, and leave it for the layout.
    /// </summary>
    /// <remarks>
    /// Two answers from one tree: <see cref="ContentNode.GetText"/> is unchanged, so a caller
    /// indexing the sheet still finds the text box's sentences under B2, while
    /// <see cref="ContentTableCell.GetOwnText"/> — which every cell question in the layout now
    /// asks — reports the cell as holding nothing of its own.
    /// </remarks>
    [Fact]
    public void AShapesTextIsInTheCellForExtractionAndOutOfItForTheLayout()
    {
        SheetLayout sheet = Sheet();
        ContentTableCell cell = CellAt(sheet, 1, 1);

        cell.GetText().ShouldContain("Small seven point line");
        cell.GetText().ShouldContain("Bold eleven point line");
        cell.GetOwnText().ShouldBeEmpty("the cell itself holds nothing but the shape");
    }

    /// <summary>The shape's text is kept as a flow of its own rather than as loose paragraphs.</summary>
    /// <remarks>
    /// <see cref="SectionKind.Frame"/> is the marker the word-processing reader already uses for a
    /// text box, and it is what makes the two answers above separable at all.
    /// </remarks>
    [Fact]
    public void AShapesParagraphsAreAFrameSectionInsideTheCell()
    {
        ContentTableCell cell = CellAt(Sheet(), 1, 1);

        ContentSection frame = cell.Children.OfType<ContentSection>().ShouldHaveSingleItem();
        frame.Kind.ShouldBe(SectionKind.Frame);
        frame.Name.ShouldBe("Text Box 1");
        frame.Children.OfType<ContentParagraph>().Count().ShouldBe(5);
        cell.Children.OfType<ContentParagraph>().ShouldBeEmpty();
    }

    /// <summary>
    /// The row the shape is anchored in is no taller for holding it.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 draws the four cell strings 12.784 pt apart — 66.388, 79.173, 91.957 and
    /// 104.741 — so the five-paragraph text box costs row 2 nothing. Measuring the shape as cell
    /// text made that row five lines tall and pushed everything below it down the page, which is
    /// the mechanism behind both regressed documents.
    /// </remarks>
    [Fact]
    public void TheAnchoringRowIsNoTallerForHoldingTheShape()
    {
        SheetLayout sheet = Sheet();

        Length first = sheet.Grid.Rows.SizeAt(0);
        for (int row = 1; row <= 3; row++)
        {
            sheet.Grid.Rows.SizeAt(row).ShouldBe(
                first, $"row {row} is the same height as row 0");
        }
    }

    /// <summary>The shape becomes a drawing carrying its own text.</summary>
    [Fact]
    public void TheShapeIsReadAsADrawing()
    {
        SheetLayout sheet = Sheet();

        SheetDrawing drawing = sheet.Drawings.Items.ShouldHaveSingleItem();
        drawing.From.Column.ShouldBe(1);
        drawing.From.Row.ShouldBe(1);
        drawing.Extent.Width.ShouldBe(Length.FromInches(2.5));
        drawing.Extent.Height.ShouldBe(Length.FromInches(1));

        SheetShapeText text = drawing.Text.ShouldNotBeNull("the shape's text was read");
        text.Paragraphs.Count.ShouldBe(5);
        text.Paragraphs[0].Text.ShouldBe("Small seven point line");
        text.Paragraphs[1].Text.ShouldBe("Bold eleven point line");
        text.Paragraphs[2].Text.ShouldBeEmpty();
        text.Paragraphs[4].Text.ShouldBe("Last");
    }

    /// <summary>
    /// A run takes its size, face and weight from its own <c>text:span</c>.
    /// </summary>
    /// <remarks>
    /// The reference draws the first line at 7 pt in LiberationSerif and the second at 11 pt in
    /// LiberationSans-Bold, which is the two spans' own <c>fo:font-size</c>, <c>fo:font-family</c>
    /// and <c>fo:font-weight</c>.
    /// </remarks>
    [Fact]
    public void ARunTakesItsFaceSizeAndWeightFromItsOwnSpan()
    {
        SheetShapeText text = Sheet().Drawings.Items[0].Text.ShouldNotBeNull();

        SheetShapeRun small = text.Paragraphs[0].Runs.ShouldHaveSingleItem();
        small.Size.ShouldBe(Length.FromPoints(7));
        small.Family.ShouldBe("Liberation Serif");
        small.Bold.ShouldBeFalse();

        SheetShapeRun bold = text.Paragraphs[1].Runs.ShouldHaveSingleItem();
        bold.Size.ShouldBe(Length.FromPoints(11));
        bold.Family.ShouldBe("Liberation Sans");
        bold.Bold.ShouldBeTrue();
    }

    /// <summary>
    /// Neither the graphic style's text properties nor <c>draw:text-style-name</c> reaches a run.
    /// </summary>
    /// <remarks>
    /// The fixture's graphic style states <c>fo:font-size="18pt"</c> and its
    /// <c>draw:text-style-name</c> names a centred paragraph style, and 26.2.4.2 honours neither:
    /// the paragraph that names no style of its own is drawn left-aligned at 11.99 pt in Liberation
    /// Serif. Established by four one-attribute variants — see the fixture's header — so this
    /// assertion pins a refutation rather than a convenience.
    /// </remarks>
    [Fact]
    public void ARunInheritsNeitherTheGraphicStyleNorTheShapesTextStyle()
    {
        SheetShapeText text = Sheet().Drawings.Items[0].Text.ShouldNotBeNull();
        SheetShapeParagraph plain = text.Paragraphs[3];

        plain.Alignment.ShouldBe(
            SheetShapeAlignment.Left, "draw:text-style-name's fo:text-align does not apply");

        foreach (SheetShapeRun run in plain.Runs)
        {
            run.Size.ShouldBe(
                SheetShapeText.DefaultSize, "not the graphic style's 18 pt");
            run.Family.ShouldBeNull("so the painter uses the drawing layer's own default face");
        }
    }

    /// <summary>An empty paragraph keeps its own span's size rather than the shape's default.</summary>
    /// <remarks>
    /// LibreOffice writes a blank line as <c>&lt;text:p&gt;&lt;text:span
    /// text:style-name="T15"/&gt;&lt;/text:p&gt;</c> and the EditEngine measures it from the
    /// character attributes at the paragraph's own position
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:1896-1902), so the blank line here is 7 pt tall
    /// and not 12.
    /// </remarks>
    [Fact]
    public void AnEmptyParagraphKeepsItsOwnSpansSize()
    {
        SheetShapeText text = Sheet().Drawings.Items[0].Text.ShouldNotBeNull();

        SheetShapeRun blank = text.Paragraphs[2].Runs.ShouldHaveSingleItem();
        blank.Text.ShouldBeEmpty();
        blank.Size.ShouldBe(Length.FromPoints(7));
        blank.Family.ShouldBe("Liberation Serif");
    }

    /// <summary>
    /// A <c>text:s</c> is that many spaces and a <c>text:tab</c> draws no glyph.
    /// </summary>
    /// <remarks>
    /// Read character by character out of 26.2.4.2's PDF, the fourth paragraph's three
    /// <c>text:s</c> spaces are in the ink and the tab puts nothing in the text layer at all —
    /// <c>the</c> ends at 244.642 and <c>shape</c> begins at 245.735, which is 1.09 pt of advance
    /// and no character.
    /// </remarks>
    [Fact]
    public void SpacesAreKeptAndATabDrawsNothing()
    {
        SheetShapeText text = Sheet().Drawings.Items[0].Text.ShouldNotBeNull();

        text.Paragraphs[3].Text.ShouldBe("Inherits   theshape");
    }

    /// <summary>The box properties do come from the graphic style.</summary>
    /// <remarks>
    /// <para>
    /// The four <c>fo:padding-*</c> are the insets — the reference draws the shape's first glyph
    /// at x 184.507, which is cell B2's left edge at 170.079 plus <c>svg:x</c> and
    /// <c>fo:padding-left</c> of a tenth of an inch each.
    /// </para>
    /// <para>
    /// <c>style:overflow-behavior="clip"</c> is ODF's spelling of DrawingML's
    /// <c>vertOverflow="clip"</c> and maps to the same UNO property
    /// (<c>xmloff/source/draw/sdpropls.cxx</c>:159 through
    /// <c>xmloff/source/style/prhdlfac.cxx</c>:483-487).
    /// </para>
    /// </remarks>
    [Fact]
    public void TheBoxPropertiesComeFromTheGraphicStyle()
    {
        SheetShapeText text = Sheet().Drawings.Items[0].Text.ShouldNotBeNull();

        text.LeftInset.ShouldBe(Length.FromInches(0.1));
        text.RightInset.ShouldBe(Length.FromInches(0.1));
        text.TopInset.ShouldBe(Length.FromInches(0.05));
        text.BottomInset.ShouldBe(Length.FromInches(0.05));
        text.Wraps.ShouldBeTrue();
        text.Anchor.ShouldBe(SheetShapeAnchor.Top);
        text.ClipsVerticalOverflow.ShouldBeTrue();
        text.Preset.ShouldBe("rect", "draw:type=\"ooxml-rect\" less its prefix");
    }
}
