using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Where an RTF <c>{\shp}</c> goes, what it does to the text around it, and how far its own text sits
/// inside it.
/// </summary>
/// <remarks>
/// <para>
/// Three readings settled here, each measured against 26.2.4.2 on one-shape probe files rather than
/// taken from the specification, because on all three the specification's prose and LibreOffice's
/// dispatch table disagree. The corpus is LibreOffice's own RTF export, so the dispatch table is the
/// only reading that can be right about it.
/// </para>
/// <list type="number">
/// <item><b>The wrap.</b> <c>rtfdispatchvalue.cxx</c>:1222-1247 maps <c>\shpwr</c> 1 to
/// <c>WrapTextMode_NONE</c>, 2 and 4 to <c>PARALLEL</c> and <b>3 and 5 both to <c>THROUGH</c></b>.
/// Three of the five were read wrongly, and 3 is the expensive one — 1341 occurrences in 178 of the
/// 338 converted <c>.rtf</c>, every one of them narrowing lines the reference leaves full width.</item>
/// <item><b>The capture.</b> A top-level <c>shapeType</c> 1 or 202 becomes a
/// <c>com.sun.star.text.TextFrame</c> — a Writer <em>fly</em> — rather than a drawing object
/// (<c>rtfsdrimport.cxx</c>:323-334), so <c>DoNotCaptureDrawObjsOnPage</c> does not exempt it and it
/// is clipped onto its page in both axes.</item>
/// <item><b>The inset.</b> <c>getTextFrameDefaults</c> (<c>rtfsdrimport.cxx</c>:111-124) gives a shape
/// stating no <c>dxTextLeft</c> family 0.1 inch across and 0.05 inch down, so zero is the wrong
/// default.</item>
/// </list>
/// <para>
/// See <c>dotnet/probes/rtf-shape-r73/results.md</c> for the probe files and the numbers.
/// </para>
/// </remarks>
public sealed class RtfShapePlacementTests
{
    /// <summary><c>\shpwr1</c> is <c>WrapTextMode_NONE</c>: no text beside the shape at all.</summary>
    /// <remarks>
    /// Its name in the specification is <em>around</em>, which is what makes it the trap: the reference
    /// starts the paragraph 60 pt lower than for any other value, below the box rather than beside it.
    /// </remarks>
    [Fact]
    public void OneIsNoTextBeside()
        => Frame(wrap: 1).Wrap.ShouldBe(TextWrap.TopAndBottom);

    /// <summary><c>\shpwr2</c> and <c>\shpwr4</c> are both parallel; only 4 marks the wrap tight.</summary>
    [Fact]
    public void TwoAndFourPutTextOnBothSides()
    {
        Frame(wrap: 2).Wrap.ShouldBe(TextWrap.Both);
        Frame(wrap: 4).Wrap.ShouldBe(TextWrap.Both);
    }

    /// <summary><c>\shpwr3</c> and <c>\shpwr5</c> are both through.</summary>
    /// <remarks>
    /// 3 additionally sets <c>wrapNone</c> and 5 does not, which is a difference in what the dmapper is
    /// told about a picture and none at all in the shape's own surround. Reading 3 as an obstacle is
    /// what made 178 of the converted corpus's documents lay out narrower than the reference.
    /// </remarks>
    [Fact]
    public void ThreeAndFiveAreBothThrough()
    {
        Frame(wrap: 3).Wrap.ShouldBe(TextWrap.Through);
        Frame(wrap: 5).Wrap.ShouldBe(TextWrap.Through);
    }

    /// <summary>A shape that states no wrap at all keeps the parallel default.</summary>
    /// <remarks>
    /// Measured, because the reading is not free: LibreOffice never sets <c>Surround</c> for an
    /// unstated <c>\shpwr</c> (<c>rtfsdrimport.cxx</c>:1091 tests its
    /// <c>WrapTextMode_MAKE_FIXED_SIZE</c> sentinel) and the fly keeps its own default. A probe stating
    /// no <c>\shpwr</c> puts the reference's first line at x = 272.1 pt, beside the box, exactly where
    /// <c>\shpwr2</c> does. The field defaulted to 1 here, which is now the one value that would have
    /// pushed the paragraph below the shape instead.
    /// </remarks>
    [Fact]
    public void AnUnstatedWrapIsParallel()
        => Frame(wrap: null).Wrap.ShouldBe(TextWrap.Both);

    /// <summary>
    /// <c>posrelv</c> 1 is the page and every other value is the anchor paragraph.
    /// </summary>
    /// <remarks>
    /// Measured on five probes whose shape is anchored in the <em>second</em> paragraph, which is what
    /// separates the paragraph from the body area: the reference draws its text at 50.4 pt for 1 — the
    /// page top plus the shape's own 1000 twips — and at 134.0 for the property absent and for 0, 2 and
    /// 3, which is that paragraph's own top plus the same 1000.
    /// </remarks>
    [Fact]
    public void OnlyPosrelvOneIsThePage()
    {
        Frame(wrap: 2, properties: @"{\sp{\sn posrelv}{\sv 1}}")
            .VerticalOrigin.ShouldBe(FrameVerticalOrigin.Page);

        foreach (int value in new[] { 0, 2, 3 })
        {
            Frame(wrap: 2, properties: $@"{{\sp{{\sn posrelv}}{{\sv {value}}}}}")
                .VerticalOrigin.ShouldBe(FrameVerticalOrigin.Paragraph);
        }
    }

    /// <summary>
    /// A shape stating none of the four <c>dxText</c> properties is inset 0.1 inch across and 0.05 inch
    /// down, which is <c>getTextFrameDefaults</c>' own <c>91440 / 360</c> and <c>45720 / 360</c>.
    /// </summary>
    [Fact]
    public void AShapeStatingNoInsetTakesTheTextFrameDefault()
    {
        Margins padding = Frame(wrap: 2).Padding;

        padding.Left.ShouldBe(Length.FromMm100(254));
        padding.Right.ShouldBe(Length.FromMm100(254));
        padding.Top.ShouldBe(Length.FromMm100(127));
        padding.Bottom.ShouldBe(Length.FromMm100(127));
    }

    /// <summary>And one that states them is read in EMUs, which the import divides by 360.</summary>
    /// <remarks>
    /// Zero and absent are two different answers here, exactly as they are for the wrap distance:
    /// LibreOffice's own export writes all four on every shape it emits, so a reader taking the
    /// default when the file states zero is wrong on every file it wrote.
    /// </remarks>
    [Fact]
    public void AStatedInsetIsReadInEmus()
    {
        Margins padding = Frame(
            wrap: 2,
            properties: @"{\sp{\sn dxTextLeft}{\sv 0}}{\sp{\sn dxTextRight}{\sv 0}}"
                + @"{\sp{\sn dyTextTop}{\sv 0}}{\sp{\sn dyTextBottom}{\sv 182880}}").Padding;

        padding.Left.ShouldBe(Length.Zero);
        padding.Right.ShouldBe(Length.Zero);
        padding.Top.ShouldBe(Length.Zero);
        padding.Bottom.ShouldBe(Length.FromMm100(508));
    }

    /// <summary>
    /// <c>posrelh</c> 1 is the page and every other value is the body area, which is what
    /// <c>RelOrientation::FRAME</c> resolves to for a frame anchored in body text.
    /// </summary>
    /// <remarks>
    /// <c>RTFSdrImport::resolve</c> maps only 1 (<c>rtfsdrimport.cxx</c>:696-706) and leaves the rest to
    /// the <c>HoriOrientRelation</c> the text frame was created with, which
    /// <c>getTextFrameDefaults</c> sets to <c>FRAME</c>. Measured over nine files — the property absent
    /// and every value 0 to 7 — on a page with a 2 inch left margin and a 1 inch paragraph indent: the
    /// reference draws the shape's text at 50.1 pt for 1 and at 194.1 pt for all eight others, so the
    /// origin is neither the page nor the indented column but the body area, and <b>0 is not the
    /// margin</b> although the specification names it so.
    /// </remarks>
    [Fact]
    public void OnlyPosrelhOneIsThePage()
    {
        Frame(wrap: 2, properties: @"{\sp{\sn posrelh}{\sv 1}}")
            .HorizontalOrigin.ShouldBe(FrameHorizontalOrigin.Page);

        foreach (int value in new[] { 0, 2, 3, 4, 5, 6, 7 })
        {
            Frame(wrap: 2, properties: $@"{{\sp{{\sn posrelh}}{{\sv {value}}}}}")
                .HorizontalOrigin.ShouldBe(FrameHorizontalOrigin.Column);
        }
    }

    /// <summary>
    /// A shape offered a position past the page's right edge is drawn with its right edge on it.
    /// </summary>
    /// <remarks>
    /// The witness class. <c>026_Unit_Circle_Chart_Four_Quadrants</c> states
    /// <c>\shpleft591\shpright11314</c> against a 3139-twip left margin, so its 10723-twip box is
    /// offered 3730 and would end 2547 twips past an 11906-twip sheet; 26.2.4.2 draws it at
    /// <b>1183</b>, and the paragraph inside it therefore fits where ours ran off the sheet and lost
    /// its last word on every line.
    /// </remarks>
    [Fact]
    public void AShapePastTheRightEdgeIsPulledBackOntoThePage()
    {
        DocRect area = Placed(left: 591, right: 11314);

        area.X.ShouldBe(Length.FromTwips(1183));
        area.Right.ShouldBe(Length.FromTwips(11906));
    }

    /// <summary>And one offered a position left of the sheet is pushed back onto it.</summary>
    [Fact]
    public void AShapeLeftOfThePageIsPushedBackOntoIt()
        => Placed(left: -4000, right: 1000).X.ShouldBe(Length.Zero);

    /// <summary>The vertical half, which the RTF reader did not apply at all before this.</summary>
    /// <remarks>
    /// It was switched off on the grounds that <c>WriterFilter.cxx</c>:332 sets
    /// <c>DoNotCaptureDrawObjsOnPage</c> for RTF as well as for DOCX. It does; the shapes are captured
    /// anyway, because they are flies and not drawing objects.
    /// </remarks>
    [Fact]
    public void AShapeBelowThePageIsPulledUpOntoIt()
    {
        DocRect area = Placed(left: 591, right: 5591, top: 18000, bottom: 18600);

        area.Bottom.ShouldBe(Length.FromTwips(16838));
    }

    /// <summary>A shape that fits is not moved in either direction.</summary>
    [Fact]
    public void AShapeThatFitsIsLeftWhereItIs()
    {
        DocRect area = Placed(left: 591, right: 5591);

        area.X.ShouldBe(Length.FromTwips(3139 + 591));
        area.Y.ShouldBe(Length.FromTwips(1440 + 1000));
    }

    /// <summary>
    /// A table inside <c>{\shptxt}</c> is the shape's content, not something to drop.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice's RTF export writes a Writer text frame's content verbatim, so a boxed table comes
    /// back as <c>{\shptxt\trowd…\cellx…\intbl…\row}</c> — and the paragraphs inside it are
    /// <c>\intbl</c>, so they go to the cell being built rather than to the frame's paragraph list.
    /// <c>FinishTable</c> then had nowhere to put the finished table, because its list of destinations
    /// named the body, a note and the furniture and not the frame, so the whole table went out with
    /// nothing left in its place.
    /// </para>
    /// <para>
    /// Six of the converted corpus's <c>.rtf</c> carry one — <c>043_Visual_Product_Roadmap</c> has four
    /// and drew 6 of the reference's 184 words, <c>019_Project_Timeline</c> five and drew 16 of 98 —
    /// and it is invisible to extraction, which walks the content tree and reads every cell.
    /// </para>
    /// </remarks>
    [Fact]
    public void ATableInsideAShapeIsTheShapesOwnContent()
    {
        PageFrame frame = Frame(
            wrap: 2,
            text: @"\trowd\cellx3000\cellx6000\pard\intbl ALPHA\cell\pard\intbl BETA\cell\row");

        PageTable table = frame.Blocks.OfType<PageTable>().ShouldHaveSingleItem();
        table.Rows.ShouldHaveSingleItem().Cells.Count.ShouldBe(2);
    }

    /// <summary>The frame the one shape of a probe file produced, before it is placed.</summary>
    private static PageFrame Frame(int? wrap, string properties = "", string? text = null)
        => Pages(wrap, properties, 591, 1000, 5591, 1600, text)
            .Paragraphs.SelectMany(paragraph => paragraph.Frames)
            .ShouldHaveSingleItem();

    /// <summary>Where that shape ended up on the page.</summary>
    private static DocRect Placed(
        int left, int right, int top = 1000, int bottom = 1600)
        => Pages(2, string.Empty, left, top, right, bottom)
            .Pages[0].Frames.ShouldHaveSingleItem().Area;

    /// <summary>
    /// One shape on an A4 page with a 3139-twip left margin, which is the witness's own geometry.
    /// </summary>
    private static WordProcessingPages Pages(
        int? wrap, string properties, int left, int top, int right, int bottom, string? text = null)
    {
        string rtf =
            @"{\rtf1\ansi\deff0{\fonttbl{\f0\froman Liberation Serif;}}"
            + @"\paperw11906\paperh16838\margl3139\margr3281\margt1440\margb1440\sectd"
            + @"\pard\plain\f0\fs20 MARKER"
            + @"{\shp{\*\shpinst"
            + $@"\shpleft{left}\shptop{top}\shpright{right}\shpbottom{bottom}"
            + (wrap is null ? string.Empty : $@"\shpwr{wrap}")
            + @"\shpbxignore\shpbyignore\shpz1"
            + @"{\sp{\sn shapeType}{\sv 1}}"
            + properties
            + @"{\shptxt" + (text ?? @"\pard\plain\f0\fs20 ZZQ\par") + "}}}"
            + @"\par}";

        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(rtf)), "shape.rtf");
        using IDocument document = new WordProcessingReader().Read(source);

        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }
}
