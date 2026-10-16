using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A floating VML shape's <c>z-index</c> decides which layer it paints on and where in that layer's
/// stack it sits.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Nothing read it, so every VML shape painted in document order in front of the text.</strong>
/// That does not present as a z-order fault. It presents as missing content: on
/// <c>JEMIT_Template.docx</c> the journal's own title sits in a <c>v:shape</c> declaring
/// <c>z-index:251659264</c> and the grey masthead band is an ordinary <c>wp:inline</c> picture
/// declared after it, so the picture painted over the title and the band came out empty. The words
/// were in the PDF's text layer the whole time — <c>pdftotext</c> reads all four of them — which is
/// exactly the class of defect no gate column can see.
/// </para>
/// <para>
/// The other direction is a watermark: Word writes one as a <c>v:shape</c> with a <em>negative</em>
/// <c>z-index</c> in a header, which is the hell layer.
/// </para>
/// </remarks>
public sealed class VmlZIndexTests
{
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string V = "urn:schemas-microsoft-com:vml";

    /// <summary>2^32, the offset that lifts a <c>z-index</c> clear of every <c>relativeHeight</c>.</summary>
    private const long Base = 4294967296L;

    private static XElement Pict(string inner)
        => XElement.Parse($"<w:pict xmlns:w=\"{W}\" xmlns:v=\"{V}\">{inner}</w:pict>");

    private static PageFrame Shape(string style)
        => DocxVmlFrames.ReadAll(Pict($"<v:rect style=\"{style}\"/>"), 0, null)
            .ShouldHaveSingleItem();

    /// <summary>A positive <c>z-index</c> is in front of the text, at its own height.</summary>
    [Fact]
    public void APositiveZIndexIsInFrontOfTheText()
    {
        PageFrame frame = Shape(
            "position:absolute;margin-left:0;margin-top:0;width:100pt;height:50pt;z-index:251659264");

        frame.BehindText.ShouldBeFalse();
        frame.ZOrder.ShouldBe(Base + 251659264L);
    }

    /// <summary>
    /// A negative <c>z-index</c> is the hell layer, which is how Word writes a watermark.
    /// </summary>
    /// <remarks>
    /// <c>DomainMapper_Impl.cxx:5157</c> — <c>PROP_OPAQUE</c> is set to <c>zOrder &gt;= 0</c> and
    /// nothing else, so the sign alone decides the layer.
    /// </remarks>
    [Fact]
    public void ANegativeZIndexIsBehindTheText()
    {
        PageFrame frame = Shape(
            "position:absolute;margin-left:0;margin-top:0;width:100pt;height:50pt;z-index:-251655168");

        frame.BehindText.ShouldBeTrue();
        frame.ZOrder.ShouldBe(Base - 251655168L);
    }

    /// <summary>
    /// A shape declaring a <c>z-index</c> outranks every DrawingML <c>relativeHeight</c>, whatever the
    /// two numbers are.
    /// </summary>
    /// <remarks>
    /// <c>GraphicZOrderHelper::adjustRelativeHeight</c>, <c>GraphicHelpers.cxx:286</c>: "in general,
    /// all z-index-defined shapes appear on top of relativeHeight graphics regardless of the value".
    /// The corpus's two families sit in the same numeric neighbourhood — both around 251 million — so
    /// a reader that compared them directly would interleave them.
    /// </remarks>
    [Fact]
    public void AZIndexOutranksTheWholeRelativeHeightRange()
    {
        Shape("position:absolute;margin-left:0;margin-top:0;width:10pt;height:10pt;z-index:1")
            .ZOrder.ShouldBeGreaterThan(uint.MaxValue);
    }

    /// <summary>A shape declaring no <c>z-index</c> keeps document order, in front of the text.</summary>
    [Fact]
    public void NoZIndexIsZeroAndInFront()
    {
        PageFrame frame = Shape(
            "position:absolute;margin-left:0;margin-top:0;width:100pt;height:50pt");

        frame.BehindText.ShouldBeFalse();
        frame.ZOrder.ShouldBe(0L);
    }

    /// <summary>A malformed value is document order rather than an exception.</summary>
    [Theory]
    [InlineData("")]
    [InlineData("auto")]
    [InlineData("251659264.5")]
    public void AMalformedZIndexIsDocumentOrder(string declared)
    {
        PageFrame frame = Shape(
            $"position:absolute;margin-left:0;margin-top:0;width:100pt;height:50pt;z-index:{declared}");

        frame.BehindText.ShouldBeFalse();
        frame.ZOrder.ShouldBe(0L);
    }

    /// <summary>Every member of a group paints on the layer the group itself declares.</summary>
    /// <remarks>
    /// A member states no <c>z-index</c> of its own — it is positioned in the group's coordinate space
    /// and stacked with it — so reading the attribute per shape would put a watermark group's contents
    /// back in front of the text.
    /// </remarks>
    [Fact]
    public void AGroupsMembersInheritItsLayer()
    {
        List<PageFrame> frames = DocxVmlFrames.ReadAll(
            Pict("<v:group style=\"position:absolute;margin-left:0;margin-top:0;"
                 + "width:100pt;height:100pt;z-index:-251655168\" coordsize=\"1000,1000\">"
                 + "<v:rect style=\"position:absolute;left:0;top:0;width:100;height:100\"/>"
                 + "<v:rect style=\"position:absolute;left:500;top:500;width:100;height:100\"/>"
                 + "</v:group>"),
            0, null);

        frames.Count.ShouldBe(2);
        frames.ShouldAllBe(frame => frame.BehindText);
        frames.ShouldAllBe(frame => frame.ZOrder == Base - 251655168L);
    }
}
