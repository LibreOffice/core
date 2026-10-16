using XRay.Office.Core.Units;
using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A VML text box insets its text and keeps its stated height, and neither was read.
/// </summary>
/// <remarks>
/// <para>
/// <c>DocxVmlFrames</c> set <c>PageFrame.Padding</c> from a stub whose two branches were the same
/// expression — <c>box is null ? default : default</c> — and never set
/// <c>PageFrame.HasFixedHeight</c> at all, so every VML text box in the corpus was laid out with
/// its text against its own edge and grew to hold whatever it was given. The DrawingML reader
/// beside it has had both since it was written.
/// </para>
/// <para>
/// <strong>Reach is 15 documents, and the census that says 131 is counting markup rather than
/// boxes.</strong> 131 of the 272 corpus DOCX state a <c>v:textbox</c> and there are 1638 of them —
/// but <strong>1472 are the <c>mc:Fallback</c> half of an <c>mc:AlternateContent</c></strong> whose
/// <c>mc:Choice</c> carries the same shape as DrawingML, so the reader takes the <c>w:drawing</c>
/// and never looks at the VML. 166 boxes in 15 documents are reachable, 14 renderings move, and
/// <strong>1467 of the 1638 state no <c>inset</c> at all</strong> — so the commonest case is the one
/// the defaults decide. 75 state <c>mso-fit-shape-to-text</c> and none states
/// <c>insetmode="auto"</c>.
/// </para>
/// <para>
/// <strong>What it buys: one gate verdict, and it lands exactly.</strong>
/// <c>068_Work_Breakdown_Structure_Template_Green_Theme</c> drew 493 alphanumeric characters
/// against 26.2.4.2's 475 and now draws 475, going <c>glyphs</c> to <c>match</c>; two more
/// truncate to the reference's count as well (185 → 179 and 255 → 246). Summed unsigned ink over
/// the 14 movers 4.14 → 4.04, four better and two worse by 0.01 and 0.03, and the other 117
/// documents are byte-identical. Those Work Breakdown templates carry a far larger *placement*
/// defect on top of this, which this does not touch.
/// </para>
/// <para>
/// <strong>Confirmed twice against 26.2.4.2</strong>, on ten one-attribute fixtures
/// (<c>probes/vmlinset-r171</c>): its own <c>--convert-to fodt</c> states the four
/// <c>fo:padding-*</c> these expect, and its rendering of the same ten puts the first line where
/// they say — left within 0.10 pt, which is the two writers' constant text-origin offset, and top
/// exactly. A fixture has to declare the <c>_x0000_t202</c> shapetype Word writes: without it the
/// shape resolves to a geometry-less <c>draw:custom-shape</c> rather than to a text frame, whose
/// padding never reaches the page, and all ten arms then render identically.
/// </para>
/// </remarks>
public sealed class VmlTextBoxInsetTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string V = "urn:schemas-microsoft-com:vml";

    private static PageFrame Box(string textbox, string style = "")
    {
        XElement pict = XElement.Parse(
            $"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\">"
            + $"<v:shape style=\"position:absolute;margin-left:36pt;margin-top:72pt;"
            + $"width:200pt;height:60pt{style}\">{textbox}</v:shape></w:pict>");

        return DocxVmlFrames.ReadAll(pict, 0, null, _ => []).ShouldHaveSingleItem();
    }

    /// <summary>A box stating no inset takes ECMA-376's, which is not zero.</summary>
    [Fact]
    public void ABoxStatingNoInsetTakesTheDefault()
    {
        PageFrame frame = Box("<v:textbox><w:txbxContent/></v:textbox>");

        frame.Padding.Left.Points.ShouldBe(7.2, 0.01);
        frame.Padding.Right.Points.ShouldBe(7.2, 0.01);
        frame.Padding.Top.Points.ShouldBe(3.6, 0.01);
        frame.Padding.Bottom.Points.ShouldBe(3.6, 0.01);
    }

    /// <summary>And a box stating zero really means zero — the opposite answer, not the same one.</summary>
    [Fact]
    public void AStatedZeroIsNotTheDefault()
    {
        PageFrame frame = Box("<v:textbox inset=\"0,0,0,0\"><w:txbxContent/></v:textbox>");

        frame.Padding.Left.ShouldBe(Length.Zero);
        frame.Padding.Top.ShouldBe(Length.Zero);
        frame.Padding.Right.ShouldBe(Length.Zero);
        frame.Padding.Bottom.ShouldBe(Length.Zero);
    }

    /// <summary>A value left out of the list takes that side's default, not the previous side's.</summary>
    [Fact]
    public void AnOmittedSideTakesItsOwnDefault()
    {
        PageFrame frame = Box("<v:textbox inset=\",.6mm,,.6mm\"><w:txbxContent/></v:textbox>");

        frame.Padding.Left.Points.ShouldBe(7.2, 0.01);
        frame.Padding.Right.Points.ShouldBe(7.2, 0.01);
        frame.Padding.Top.Points.ShouldBe(1.70, 0.01);
        frame.Padding.Bottom.Points.ShouldBe(1.70, 0.01);
    }

    /// <summary>So a one-value inset moves the left side alone.</summary>
    [Fact]
    public void AOneValueInsetMovesTheLeftSideOnly()
    {
        PageFrame frame = Box("<v:textbox inset=\"4mm\"><w:txbxContent/></v:textbox>");

        frame.Padding.Left.Points.ShouldBe(11.34, 0.01);
        frame.Padding.Top.Points.ShouldBe(3.6, 0.01);
        frame.Padding.Right.Points.ShouldBe(7.2, 0.01);
        frame.Padding.Bottom.Points.ShouldBe(3.6, 0.01);
    }

    /// <summary>
    /// <c>insetmode="auto"</c> takes none of it and keeps Writer's own 1.5 mm.
    /// </summary>
    /// <remarks>
    /// The whole inset block is guarded on the mode not being <c>auto</c>
    /// (<c>oox/source/vml/vmltextboxcontext.cxx</c>:185), so such a box sets no border distance and
    /// inherits the <c>Frame</c> style's padding — which 26.2.4.2 exports as
    /// <c>fo:padding="0.0591in"</c> and draws 4.25 pt in. No corpus document states the mode.
    /// </remarks>
    [Fact]
    public void TheAutomaticInsetModeKeepsWritersOwn()
    {
        PageFrame frame = Box("<v:textbox insetmode=\"auto\" inset=\"4mm,4mm,4mm,4mm\">"
                              + "<w:txbxContent/></v:textbox>");

        frame.Padding.Left.Points.ShouldBe(4.25, 0.01);
        frame.Padding.Bottom.Points.ShouldBe(4.25, 0.01);
    }

    /// <summary>Any other mode is read as a stated one, so its inset applies.</summary>
    [Fact]
    public void AnyOtherInsetModeReadsTheInset()
    {
        PageFrame frame = Box("<v:textbox insetmode=\"custom\"><w:txbxContent/></v:textbox>");

        frame.Padding.Left.Points.ShouldBe(7.2, 0.01);
    }

    /// <summary>A box keeps its declared height, so its text is cut at it.</summary>
    [Fact]
    public void ABoxKeepsItsHeight()
        => Box("<v:textbox><w:txbxContent/></v:textbox>").HasFixedHeight.ShouldBeTrue();

    /// <summary><c>mso-fit-shape-to-text</c> on the box makes the height grow instead.</summary>
    [Fact]
    public void TheBoxesOwnFitStyleMakesItGrow()
        => Box("<v:textbox style=\"mso-fit-shape-to-text:t\"><w:txbxContent/></v:textbox>")
            .HasFixedHeight.ShouldBeFalse();

    /// <summary>And so does the same declaration on the shape, which is where Word writes it.</summary>
    [Fact]
    public void TheShapesFitStyleMakesItGrowToo()
        => Box("<v:textbox><w:txbxContent/></v:textbox>", ";mso-fit-shape-to-text:t")
            .HasFixedHeight.ShouldBeFalse();

    /// <summary>A shape carrying no text box has neither a padding nor a height to keep.</summary>
    [Fact]
    public void AShapeWithoutATextBoxIsUntouched()
    {
        PageFrame frame = Box(string.Empty);

        frame.Padding.ShouldBe(default);
        frame.HasFixedHeight.ShouldBeFalse();
    }
}
