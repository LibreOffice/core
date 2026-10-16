using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A floating VML shape is drawn — and its text box with it — while still reserving no line room.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Two halves that must be kept apart.</strong> Reserving room for a <c>position:absolute</c>
/// shape is wrong: that is what added seven pages to <c>33004.docx</c> and got an earlier attempt
/// reverted. But returning nothing for one is equally wrong, and dropped its text box with it —
/// seven corpus documents rendered completely blank. <c>TextWrap.Through</c> says both at once.
/// </para>
/// <para>
/// <strong>Measured on LibreOffice 26.2.4.2.</strong>
/// <c>069_Work_Breakdown_Structure_Template_Professional_Format.docx</c> holds 49 VML shapes, every
/// one <c>position:absolute</c>, 24 carrying a <c>v:textbox</c>. We extracted 0 words against the
/// reference's 119; after this, 110. The remaining 9 are the reference splitting <c>SUBTASK</c>
/// into <c>SU</c> and <c>BTASK</c> nine times — its own per-glyph tokenisation, not missing text.
/// Across <c>words/chartset-005…014</c> the gate went 69 of 100 to 74; the original 200-document
/// words batches did not move at all.
/// </para>
/// <para>
/// <strong>18 of those 24 text boxes are inside a <c>v:group</c></strong>, whose members state
/// <c>left</c>/<c>top</c>/<c>width</c>/<c>height</c> as bare numbers in the group's own
/// <c>coordsize</c> space. Reading them as points puts every one metres off the page.
/// </para>
/// </remarks>
public sealed class VmlFloatingShapeTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string V = "urn:schemas-microsoft-com:vml";

    private static XElement Pict(string inner)
        => XElement.Parse($"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\">{inner}</w:pict>");

    /// <summary>A floating shape with a text box is drawn, and makes no room on any line.</summary>
    [Fact]
    public void AFloatingTextBoxIsDrawnAndReservesNothing()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:rect style=\"position:absolute;margin-left:36pt;margin-top:72pt;"
                 + "width:144pt;height:36pt\"><v:textbox><w:txbxContent/></v:textbox></v:rect>"),
            0,
            null,
            _ => []);

        PageFrame frame = frames.ShouldHaveSingleItem();
        frame.Wrap.ShouldBe(TextWrap.Through, "a floating shape makes no room on a line");
        frame.Anchor.ShouldBe(FrameAnchor.Paragraph);
        frame.IsImage.ShouldBeFalse("a shape carrying a text box is not a picture");
        frame.Size.Width.Points.ShouldBe(144, 0.01);
        frame.Size.Height.Points.ShouldBe(36, 0.01);
        frame.HorizontalOffset.Points.ShouldBe(36, 0.01);
        frame.VerticalOffset.Points.ShouldBe(72, 0.01);
    }

    /// <summary>The control: an inline shape still reserves its declared box.</summary>
    [Fact]
    public void AnInlineShapeStillReservesItsBox()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:shape style=\"width:425pt;height:190pt\"/>"), 0, null);

        PageFrame frame = frames.ShouldHaveSingleItem();
        frame.Anchor.ShouldBe(FrameAnchor.AsCharacter);
        frame.Size.Height.Points.ShouldBe(190, 0.01);
    }

    /// <summary>Every shape of a <c>w:pict</c> is read, not just the first.</summary>
    [Fact]
    public void EveryShapeIsRead()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:rect style=\"position:absolute;margin-left:0;margin-top:0;width:10pt;height:10pt\"/>"
                 + "<v:rect style=\"position:absolute;margin-left:20pt;margin-top:0;width:10pt;height:10pt\"/>"
                 + "<v:rect style=\"position:absolute;margin-left:40pt;margin-top:0;width:10pt;height:10pt\"/>"),
            0, null);

        frames.Count.ShouldBe(3);
        frames.Select(f => f.HorizontalOffset.Points).ShouldBe([0, 20, 40], 0.01);
    }

    /// <summary>
    /// A group's member is mapped out of the group's coordinate space into real units.
    /// </summary>
    /// <remarks>
    /// The group is 200 pt wide over a 21600-unit space, so a member at <c>left:5400</c> — a quarter
    /// of the way across — sits 50 pt in, plus the group's own 36 pt offset.
    /// </remarks>
    [Fact]
    public void AGroupMemberIsMappedOutOfTheGroupsCoordinateSpace()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:group style=\"position:absolute;margin-left:36pt;margin-top:72pt;"
                 + "width:200pt;height:100pt\" coordsize=\"21600,21600\">"
                 + "<v:rect style=\"position:absolute;left:5400;top:10800;width:5400;height:2160\">"
                 + "<v:textbox><w:txbxContent/></v:textbox></v:rect>"
                 + "</v:group>"),
            0, null, _ => []);

        PageFrame frame = frames.ShouldHaveSingleItem();
        frame.HorizontalOffset.Points.ShouldBe(36 + 50, 0.01);
        frame.VerticalOffset.Points.ShouldBe(72 + 50, 0.01);
        frame.Size.Width.Points.ShouldBe(50, 0.01);
        frame.Size.Height.Points.ShouldBe(10, 0.01);
        frame.Wrap.ShouldBe(TextWrap.Through);
    }

    /// <summary>
    /// A <c>v:group</c> inside a <c>v:group</c> is walked, and its members are mapped through both
    /// coordinate spaces.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This used to be a <c>continue</c> — <c>if (member.Name.LocalName is "group") continue;</c>,
    /// commented "nested groups: not measured, not guessed" — so a nested group and its entire subtree
    /// were discarded.
    /// </para>
    /// <para>
    /// The arithmetic here composes twice. The outer group is 200 pt wide over a 21600-unit space; the
    /// inner group sits at <c>left:5400</c> and is <c>width:10800</c>, so it occupies 50 pt to 150 pt.
    /// It declares its own 1000-unit space, so its member at <c>left:500</c>, <c>width:250</c> sits half
    /// way across it and is a quarter of it wide: 36 + 50 + 50 = 136 pt, 25 pt wide.
    /// </para>
    /// <para>
    /// Measured on <c>068_Work_Breakdown_Structure_Template_Green_Theme</c>, whose five phase groups each
    /// hold a connector and a nested group of seven task boxes: 19 words against the reference's 86
    /// before, 86 after, and the words inside the nested groups number exactly 67.
    /// </para>
    /// </remarks>
    [Fact]
    public void ANestedGroupIsWalkedThroughBothCoordinateSpaces()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:group style=\"position:absolute;margin-left:36pt;margin-top:72pt;"
                 + "width:200pt;height:100pt\" coordsize=\"21600,21600\">"
                 + "<v:group style=\"position:absolute;left:5400;top:0;width:10800;height:21600\""
                 + " coordsize=\"1000,1000\">"
                 + "<v:rect style=\"position:absolute;left:500;top:0;width:250;height:500\">"
                 + "<v:textbox><w:txbxContent/></v:textbox></v:rect>"
                 + "</v:group>"
                 + "</v:group>"),
            0, null, _ => []);

        PageFrame frame = frames.ShouldHaveSingleItem();
        frame.HorizontalOffset.Points.ShouldBe(36 + 50 + 50, 0.01);
        frame.VerticalOffset.Points.ShouldBe(72, 0.01);
        frame.Size.Width.Points.ShouldBe(25, 0.01);
        frame.Size.Height.Points.ShouldBe(50, 0.01);
        frame.Wrap.ShouldBe(TextWrap.Through);
    }

    /// <summary>
    /// A group's own members and a nested group's are both read, and the flat one is not disturbed.
    /// </summary>
    [Fact]
    public void AGroupsOwnMembersAreReadBesideItsNestedGroups()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:group style=\"position:absolute;margin-left:0;margin-top:0;"
                 + "width:100pt;height:100pt\" coordsize=\"1000,1000\">"
                 + "<v:rect style=\"position:absolute;left:0;top:0;width:100;height:100\"/>"
                 + "<v:group style=\"position:absolute;left:500;top:500;width:500;height:500\""
                 + " coordsize=\"100,100\">"
                 + "<v:rect style=\"position:absolute;left:0;top:0;width:50;height:50\"/>"
                 + "<v:rect style=\"position:absolute;left:50;top:50;width:50;height:50\"/>"
                 + "</v:group>"
                 + "</v:group>"),
            0, null, _ => []);

        frames.Count.ShouldBe(3, "the group\u2019s own member plus both of the nested group\u2019s");
        frames.Select(f => f.HorizontalOffset.Points).ShouldBe([0, 50, 75], 0.01);
        frames.Select(f => f.Size.Width.Points).ShouldBe([10, 25, 25], 0.01);
    }

    /// <summary>
    /// <c>coordorigin</c> shifts the child space, and a member is placed relative to it.
    /// </summary>
    [Fact]
    public void CoordOriginShiftsTheChildSpace()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:group style=\"position:absolute;margin-left:0;margin-top:0;"
                 + "width:100pt;height:100pt\" coordsize=\"1000,1000\" coordorigin=\"500,500\">"
                 + "<v:rect style=\"position:absolute;left:500;top:500;width:100;height:100\"/>"
                 + "</v:group>"),
            0, null);

        PageFrame frame = frames.ShouldHaveSingleItem();
        frame.HorizontalOffset.Points.ShouldBe(0, 0.01, "left equals the origin, so it sits at the group's edge");
        frame.VerticalOffset.Points.ShouldBe(0, 0.01);
    }

    /// <summary>A zero-area shape is skipped: VML writes width:0 for a bare connector rule.</summary>
    [Fact]
    public void AZeroAreaShapeIsSkipped()
    {
        DocxVmlFrames.ReadAll(
            Pict("<v:shape style=\"position:absolute;margin-left:10pt;margin-top:10pt;"
                 + "width:0;height:12.75pt\"/>"), 0, null).ShouldBeEmpty();
    }

    /// <summary>
    /// <c>v:shapetype</c> is not a shape — taking the first VML element finds no size and
    /// silently reserves nothing.
    /// </summary>
    [Fact]
    public void AShapeTypeIsNotTakenForTheShape()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:shapetype id=\"t\"/><v:shape style=\"width:100pt;height:50pt\"/>"), 0, null);

        frames.ShouldHaveSingleItem().Size.Width.Points.ShouldBe(100, 0.01);
    }
}
