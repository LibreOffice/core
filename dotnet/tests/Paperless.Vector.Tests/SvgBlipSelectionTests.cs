using System.Xml.Linq;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// Choosing the vector rendering of a DrawingML picture over the raster fallback beside it.
/// </summary>
/// <remarks>
/// <para>
/// The markup below is not invented. It is what LibreOffice 24.2 wrote when an ODT holding a
/// 1 395-byte <c>Pictures/logo.svg</c> was converted to <c>.docx</c>: a 39 497-byte
/// <c>word/media/image1.png</c> on <c>r:embed</c>, and the original SVG on an
/// <c>asvg:svgBlip</c> inside the <c>{96DAC541-…}</c> extension. PowerPoint 365 writes the
/// same shape. Preferring the second is what turns a soft 96-dpi bitmap into exact geometry
/// at print resolution.
/// </para>
/// </remarks>
public sealed class SvgBlipSelectionTests
{
    private const string DrawingML = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Relationships = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>The 2010 <c>useLocalDpi</c> extension, which sits in the same list.</summary>
    private const string Other = "{28A0092B-C50C-407E-A947-70E740481C1C}";

    [Fact]
    public void AnSvgBlipIsPreferredOverTheRasterFallback()
    {
        BlipReference.Choice choice = BlipReference.Choose(Parse($"""
            <a:blip xmlns:a="{DrawingML}" xmlns:r="{Relationships}" r:embed="rId2">
              <a:extLst>
                <a:ext uri="{BlipReference.SvgExtensionUri}">
                  <asvg:svgBlip xmlns:asvg="{BlipReference.SvgBlipNamespace}" r:embed="rId3"/>
                </a:ext>
              </a:extLst>
            </a:blip>
            """));

        choice.RelationshipId.ShouldBe("rId3");
        choice.IsVector.ShouldBeTrue();

        // The raster is still reported, because a vector that fails to decode should fall
        // back to it rather than leaving a blank frame.
        choice.FallbackRelationshipId.ShouldBe("rId2");
    }

    [Fact]
    public void APlainBlipReportsItsRaster()
    {
        BlipReference.Choice choice = BlipReference.Choose(Parse($"""
            <a:blip xmlns:a="{DrawingML}" xmlns:r="{Relationships}" r:embed="rId7"/>
            """));

        choice.RelationshipId.ShouldBe("rId7");
        choice.IsVector.ShouldBeFalse();
        choice.FallbackRelationshipId.ShouldBeNull();
    }

    [Fact]
    public void AnExtensionWithAnotherUriIsIgnored()
    {
        // a:extLst is open-ended: the 2010 extensions and several others live beside this
        // one, so matching on the element name alone would pick up the wrong relationship.
        BlipReference.Choice choice = BlipReference.Choose(Parse($"""
            <a:blip xmlns:a="{DrawingML}" xmlns:r="{Relationships}" r:embed="rId2">
              <a:extLst>
                <a:ext uri="{Other}">
                  <a14:useLocalDpi xmlns:a14="http://schemas.microsoft.com/office/drawing/2010/main" val="0"/>
                </a:ext>
              </a:extLst>
            </a:blip>
            """));

        choice.RelationshipId.ShouldBe("rId2");
        choice.IsVector.ShouldBeFalse();
    }

    [Fact]
    public void ALinkedPictureNamesNothingToFetch()
    {
        // r:link points outside the package. A document reader that followed it would be an
        // exfiltration channel, so it is not read at all.
        BlipReference.Choice choice = BlipReference.Choose(Parse($"""
            <a:blip xmlns:a="{DrawingML}" xmlns:r="{Relationships}" r:link="rId9"/>
            """));

        choice.RelationshipId.ShouldBeNull();
        choice.IsVector.ShouldBeFalse();
    }

    [Fact]
    public void NoBlipMeansNoChoice()
    {
        BlipReference.Choice choice = BlipReference.Choose(null);

        choice.RelationshipId.ShouldBeNull();
        choice.IsVector.ShouldBeFalse();
    }

    [Fact]
    public void TheSvgMediaTypeIsRecognisedIncludingWithParameters()
    {
        VectorImages.IsVectorMediaType("image/svg+xml").ShouldBeTrue();
        VectorImages.IsVectorMediaType("image/svg+xml; charset=utf-8").ShouldBeTrue();
        VectorImages.IsVectorMediaType("image/png").ShouldBeFalse();
        VectorImages.IsVectorMediaType(null).ShouldBeFalse();
    }

    private static XElement Parse(string xml) => XElement.Parse(xml);
}
