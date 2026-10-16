using System.Xml.Linq;
using XRay.Office.Presentations.OpenDocument;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// ODF's own spelling of WordArt, which is the model's native one.
/// </summary>
/// <remarks>
/// <para>
/// A <c>draw:enhanced-geometry</c> states the three things the OOXML filter has to derive:
/// <c>draw:type</c> is the LibreOffice Fontwork type itself rather than a <c>prst</c> to map,
/// <c>draw:modifiers</c> is already in the 21600 viewbox the geometry tables use, and
/// <c>draw:text-path-scale</c> is <c>TextPathScaleX</c> outright.
/// <c>xmloff/source/draw/ximpcustomshape.cxx:1136-1150</c> is the reference's reader.
/// </para>
/// <para>
/// <strong>Zero corpus reach, and that is measured rather than assumed.</strong> The 945-document
/// corpus holds 272 <c>docx</c>, 251 <c>pptx</c>, 241 <c>xlsx</c>, 66 <c>doc</c>, 64 <c>xls</c> and
/// 51 <c>ppt</c> — and <b>not one ODF file of any kind</b>. So the gate cannot see this path at
/// all, and what it is measured against is a deck LibreOffice 26.2.4.2 converted from
/// <c>FAAAIandtheArtandScienceofV&amp;Vfinal.pptx</c>, whose eight arch labels are genuine ODF
/// fontwork markup. See <c>dotnet/probes/odf-fontwork/results.md</c>.
/// </para>
/// </remarks>
public sealed class OdfFontworkTests
{
    private const string Draw = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";

    private static XElement Geometry(string attributes)
        => XElement.Parse(
            $"<draw:enhanced-geometry xmlns:draw=\"{Draw}\" {attributes}/>");

    /// <summary>The arch label a converted deck writes, read whole.</summary>
    [Fact]
    public void AnOdfTextPathStatesItsTypeItsModifiersAndItsScale()
    {
        OdfFontwork.Warp warp = OdfFontwork.Read(Geometry(
            "draw:text-path=\"true\" draw:text-path-mode=\"path\" draw:text-path-scale=\"shape\""
            + " draw:type=\"fontwork-arch-down-curve\" draw:modifiers=\"0\"")).ShouldNotBeNull();

        warp.FontworkType.ShouldBe("fontwork-arch-down-curve");
        warp.Adjustments.ShouldBe([0.0]);
        warp.KeepsFontSize.ShouldBeTrue("draw:text-path-scale=\"shape\" is TextPathScaleX");
    }

    /// <summary>
    /// <c>draw:text-path-scale="path"</c> is the other value, and it is what 36 of the 40 warps get.
    /// </summary>
    /// <remarks>
    /// Measured on the converted fixture: LibreOffice writes <c>shape</c> on exactly
    /// <c>fontwork-arch-up-curve</c>, <c>fontwork-arch-down-curve</c>, <c>fontwork-circle-curve</c>
    /// and <c>fontwork-open-circle-curve</c> — the same four the DrawingML side derives it for from
    /// <c>fontworkhelpers.cxx:173-179</c>, which is an independent corroboration of that list.
    /// </remarks>
    [Fact]
    public void APathScaledWarpDoesNotKeepItsFontSize()
        => OdfFontwork.Read(Geometry(
                "draw:text-path=\"true\" draw:text-path-scale=\"path\" draw:type=\"fontwork-wave\""))
            .ShouldNotBeNull().KeepsFontSize.ShouldBeFalse();

    /// <summary>Several modifiers, which is what a pour shape's angle and radius are.</summary>
    [Fact]
    public void ModifiersAreSpaceSeparatedAndInWordArtUnits()
        => OdfFontwork.Read(Geometry(
                "draw:text-path=\"true\" draw:type=\"fontwork-arch-up-pour\""
                + " draw:modifiers=\"180 5400\""))
            .ShouldNotBeNull().Adjustments.ShouldBe([180.0, 5400.0]);

    /// <summary>An absent <c>draw:modifiers</c> leaves the preset's own defaults in force.</summary>
    [Fact]
    public void NoModifiersMeansThePresetsOwnDefaults()
        => OdfFontwork.Read(Geometry(
                "draw:text-path=\"true\" draw:type=\"fontwork-wave\""))
            .ShouldNotBeNull().Adjustments.ShouldBeNull();

    /// <summary>A shape that is not in text-path mode is not Fontwork, whatever its type says.</summary>
    [Theory]
    [InlineData("draw:type=\"fontwork-wave\"")]
    [InlineData("draw:text-path=\"false\" draw:type=\"fontwork-wave\"")]
    public void AShapeThatIsNotInTextPathModeIsNotFontwork(string attributes)
        => OdfFontwork.Read(Geometry(attributes)).ShouldBeNull();

    /// <summary>
    /// And neither is one whose <c>draw:type</c> names no preset.
    /// </summary>
    /// <remarks>
    /// <c>non-primitive</c> is what LibreOffice writes for a shape a user drew rather than chose,
    /// and a <c>draw:custom-shape</c> may carry a <c>draw:text-path</c> beside one. Its geometry is
    /// in its <c>draw:enhanced-path</c>, which this reader does not consult, so answering "not
    /// Fontwork" leaves it to the ordinary custom-shape path rather than warping it wrongly.
    /// </remarks>
    [Theory]
    [InlineData("non-primitive")]
    [InlineData("ooxml-rect")]
    public void AShapeWhoseTypeNamesNoPresetIsNotFontwork(string type)
        => OdfFontwork.Read(Geometry($"draw:text-path=\"true\" draw:type=\"{type}\"")).ShouldBeNull();

    /// <summary>Nothing at all is not Fontwork either.</summary>
    [Fact]
    public void AShapeWithNoEnhancedGeometryIsNotFontwork()
        => OdfFontwork.Read(null).ShouldBeNull();
}
