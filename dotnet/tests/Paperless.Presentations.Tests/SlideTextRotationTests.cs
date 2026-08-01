using System.Globalization;
using System.Xml.Linq;
using Paperless.Presentations.Layout;
using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Text turned inside a shape that has not moved — <c>a:bodyPr/@rot</c>, and what a SmartArt
/// <c>autoTxRot</c> resolves to.
/// </summary>
/// <remarks>
/// <para>
/// It is not the shape's rotation. A cycle turns each of its nodes to face along the ring and then
/// counter-turns the labels so they stay readable; the node keeps its rotation and the label gets
/// its own. LibreOffice keeps the two apart the same way, as <c>TextPreRotateAngle</c> beside
/// <c>RotateAngle</c>, and the <c>tx</c> algorithm writes only the first
/// (<c>oox/source/drawingml/diagram/diagramlayoutatoms.cxx:1730-1760</c>).
/// </para>
/// <para>
/// Measured on <c>sd/qa/unit/data/pptx/smartart-autoTxRot.pptx</c>, which carries the same
/// five-node cycle three times over — once with the attribute absent, so <c>upr</c>; once with
/// <c>none</c>; once with <c>grav</c>. Its 48 labels now agree with LibreOffice's on pen, em size
/// and angle to 0.100 pt. Before, all 48 were drawn upright while the reference wrote a rotation
/// matrix for 41 of them — and a run-count comparison could not see it, because a rotated run is
/// a <c>Tm</c> where an upright one is a <c>Td</c>.
/// </para>
/// </remarks>
public class SlideTextRotationTests
{
    private const string Drawing = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>Sixtieth-thousandths of a degree, as everywhere else in DrawingML.</summary>
    private const int PerDegree = 60000;

    /// <summary>
    /// The angle is read clockwise in sixtieth-thousandths of a degree, like every other one.
    /// </summary>
    /// <remarks>
    /// Clockwise is worth pinning: ODF's rotation runs the other way, and a body read
    /// anticlockwise puts a quarter-turned label where a three-quarter-turned one belongs — which
    /// looks like a placement bug rather than a sign error.
    /// </remarks>
    [Theory]
    [InlineData(0, 0.0)]
    [InlineData(90 * PerDegree, Math.PI / 2)]
    [InlineData(180 * PerDegree, Math.PI)]
    [InlineData(-90 * PerDegree, -Math.PI / 2)]
    public void TheBodyRotationIsReadInRadiansClockwise(int units, double expected)
    {
        PptxTextBody.Read(Body(units)).Rotation.ShouldBe(expected, 1e-9);
    }

    /// <summary>A body stating no rotation is upright, which is the overwhelming majority.</summary>
    [Fact]
    public void ABodyWithNoRotationIsUpright()
    {
        PptxTextBody.Read(new XElement(
            XName.Get("txBody", Drawing),
            new XElement(XName.Get("bodyPr", Drawing)),
            Paragraph())).Rotation.ShouldBe(0);
    }

    private static XElement Body(int rotationUnits)
    {
        XElement properties = new(XName.Get("bodyPr", Drawing));

        if (rotationUnits != 0)
        {
            properties.SetAttributeValue(
                "rot", rotationUnits.ToString(CultureInfo.InvariantCulture));
        }

        return new XElement(XName.Get("txBody", Drawing), properties, Paragraph());
    }

    private static XElement Paragraph()
        => new(
            XName.Get("p", Drawing),
            new XElement(
                XName.Get("r", Drawing),
                new XElement(
                    XName.Get("rPr", Drawing),
                    new XAttribute("sz", "1800"),
                    new XElement(
                        XName.Get("latin", Drawing),
                        new XAttribute("typeface", "Liberation Sans"))),
                new XElement(XName.Get("t", Drawing), "Node")));
}
