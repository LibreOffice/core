using System.Xml.Linq;
using XRay.Office.Presentations.Ooxml;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// <c>a:bodyPr/@wrap="none"</c> against the body's autofit, which decides whether it is honoured.
/// </summary>
/// <remarks>
/// <para>
/// <c>wrap="none"</c> does not stand on its own. Measured on LibreOffice 26.2.4.2 with nine
/// authored one-shape decks varying the two axes independently — a 236 pt box holding the
/// 64-character line <c>Free Templates and Infographics for PowerPoint and Google Slides</c> at
/// 22 pt:
/// </para>
/// <list type="table">
///   <listheader><term>wrap</term><description>autofit stated → lines drawn</description></listheader>
///   <item><term>none</term><description>(absent) → 1; noAutofit → 1; spAutoFit → 4; normAutofit → 4</description></item>
///   <item><term>square</term><description>(absent) → 4; noAutofit → 4; spAutoFit → 4; normAutofit → 4</description></item>
/// </list>
/// <para>
/// So a *fitting* autofit beats the wrap and only <c>wrap="none"</c> with <c>noAutofit</c> or with
/// no autofit at all is unbounded. Reading the attribute alone runs the line off the **page**
/// rather than merely off the shape, and everything past the media box is lost from the text
/// layer — on the 2026-08-19 baseline that was 30 of 305 slides renderings against the
/// reference's 9, and <c>Google Slides</c> extracting as <c>Google Slid</c> on a whole template
/// family, which was the entire character difference on 15 of the 28 documents filed as
/// <c>text</c>.
/// </para>
/// </remarks>
public class SlideWrapAutofitTests
{
    private const string Drawing = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>
    /// The measured grid, both axes, exactly as the reference draws it.
    /// </summary>
    [Theory]
    [InlineData("none", null, false)]
    [InlineData("none", "noAutofit", false)]
    [InlineData("none", "spAutoFit", true)]
    [InlineData("none", "normAutofit", true)]
    [InlineData("square", null, true)]
    [InlineData("square", "noAutofit", true)]
    [InlineData("square", "spAutoFit", true)]
    [InlineData("square", "normAutofit", true)]
    [InlineData(null, null, true)]
    public void WrapNoneIsHonouredOnlyWhileTheAutofitLeavesTheShapeAlone(
        string? wrap, string? autofit, bool wraps)
    {
        PptxTextBody.Read(Body(wrap, autofit)).Wraps.ShouldBe(wraps);
    }

    /// <summary>
    /// A fitting autofit inherited from a layout or master beats a slide's own
    /// <c>wrap="none"</c>, because the chain is what resolves — not the nearest element.
    /// </summary>
    /// <remarks>
    /// This is the case a same-element census cannot see, and it is the common shape in the
    /// corpus: PowerPoint writes the wrap where the box is drawn and the autofit where the
    /// placeholder is defined.
    /// </remarks>
    [Fact]
    public void AnInheritedFittingAutofitBeatsTheBodysOwnWrapNone()
    {
        PptxTextBody.Read(
            Body("none", null),
            inheritedBodyProperties: [Properties(null, "spAutoFit")]).Wraps.ShouldBeTrue();
    }

    /// <summary>
    /// An inherited <c>noAutofit</c> leaves <c>wrap="none"</c> standing, which is the other half
    /// of the same question and the case that must keep working.
    /// </summary>
    [Fact]
    public void AnInheritedNoAutofitLeavesWrapNoneStanding()
    {
        PptxTextBody.Read(
            Body("none", null),
            inheritedBodyProperties: [Properties(null, "noAutofit")]).Wraps.ShouldBeFalse();
    }

    private static XElement Body(string? wrap, string? autofit)
        => new(XName.Get("txBody", Drawing), Properties(wrap, autofit), Paragraph());

    private static XElement Properties(string? wrap, string? autofit)
    {
        XElement properties = new(XName.Get("bodyPr", Drawing));

        if (wrap is not null) properties.SetAttributeValue("wrap", wrap);
        if (autofit is not null) properties.Add(new XElement(XName.Get(autofit, Drawing)));

        return properties;
    }

    private static XElement Paragraph()
        => new(
            XName.Get("p", Drawing),
            new XElement(
                XName.Get("r", Drawing),
                new XElement(
                    XName.Get("rPr", Drawing),
                    new XAttribute("sz", "2200"),
                    new XElement(
                        XName.Get("latin", Drawing),
                        new XAttribute("typeface", "Liberation Sans"))),
                new XElement(XName.Get("t", Drawing), "Free Templates and Infographics")));
}
