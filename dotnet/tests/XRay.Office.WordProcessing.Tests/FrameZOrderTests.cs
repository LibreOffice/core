using System.Xml.Linq;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>wp:anchor</c>'s place in the stack is the <c>relativeHeight</c> it declares, not the order
/// the anchors happen to appear in.
/// </summary>
/// <remarks>
/// <para>
/// The defect these were written against does not present as a z-order fault. It presents as
/// <em>missing content</em>: the renderer draws the text, correctly positioned, and then paints a
/// shape declared later in the document over it, so the page loses material that is in the content
/// stream all along. Every pixel metric reports it as content missing, which is a different bug in a
/// different place, and five separate readings in this repository's parity catalogue were one
/// instance of it.
/// </para>
/// <para>
/// Measured over those five corpus documents: every one declares <c>relativeHeight</c> on every
/// anchor — 20 to 44 of them each — and <strong>not one</strong> is in document order.
/// <c>045_Visual_Product_Roadmap</c> shows <c>2021</c> at content-stream offset 2473 and fills its
/// black box at 4180, over the top; <c>060_Human_Body_Concept_Map</c> draws the entire page and then
/// the grey ground across all of it, which is why it rendered as one blank grey sheet.
/// </para>
/// </remarks>
public sealed class FrameZOrderTests
{
    /// <summary>The declared height is read, and it is read as unsigned.</summary>
    /// <remarks>
    /// <c>ST_RelativeHeight</c> is a 32-bit unsigned value and real files use the top of the range —
    /// the corpus templates sit around 251 660 000. That fits a signed <c>int</c>, but the type does
    /// not, and a reader that narrows it silently turns the highest shapes on a page into the lowest.
    /// The maximum is asserted for that reason rather than for completeness.
    /// </remarks>
    [Theory]
    [InlineData("251659264", 251659264L)]
    [InlineData("251707392", 251707392L)]
    [InlineData("4294967295", 4294967295L)]
    [InlineData("0", 0L)]
    public void TheAnchorsDeclaredHeightIsRead(string declared, long expected) =>
        Frame(declared).ZOrder.ShouldBe(expected);

    /// <summary>An anchor that declares nothing sorts below every anchor that does.</summary>
    /// <remarks>
    /// Zero rather than a sentinel, so the ordering needs no special case: it is where an undeclared
    /// shape belongs, and because the sort is stable such shapes keep document order among
    /// themselves — which is what this code did before the height was read at all.
    /// </remarks>
    [Fact]
    public void AnAnchorWithNoDeclaredHeightIsZero() =>
        Frame(null).ZOrder.ShouldBe(0L);

    /// <summary>A value that is not a number is zero rather than an exception.</summary>
    /// <remarks>
    /// Real files violate their own schema constantly, and a malformed z order is not a reason to
    /// refuse a document — it is a reason to fall back to document order for that one shape.
    /// </remarks>
    [Theory]
    [InlineData("")]
    [InlineData("high")]
    [InlineData("-1")]
    [InlineData("251659264.5")]
    [InlineData("99999999999999999999")]
    public void AMalformedHeightIsZero(string declared) =>
        Frame(declared).ZOrder.ShouldBe(0L);

    private static PageFrame Frame(string? relativeHeight)
    {
        string attribute = relativeHeight is null
            ? string.Empty
            : $""" relativeHeight="{relativeHeight}" """;

        XElement drawing = XElement.Parse(
            $"""
            <w:drawing xmlns:w="{W}" xmlns:wp="{Wp}" xmlns:a="{A}" xmlns:wps="{Wps}">
              <wp:anchor{attribute}>
                <wp:extent cx="914400" cy="457200"/>
                <wp:wrapNone/>
                <a:graphic><a:graphicData><wps:wsp/></a:graphicData></a:graphic>
              </wp:anchor>
            </w:drawing>
            """);

        return DocxFrames
            .ReadAll(drawing, null, anchorOffset: 0, pictures: null,
                     new DocxFrameContext(null, InHeaderFooter: false, CompatibilityMode: 15))
            .ShouldHaveSingleItem();
    }

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Wps = "http://schemas.microsoft.com/office/word/2010/wordprocessingShape";
}
