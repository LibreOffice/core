using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Vector.Tests;

/// <summary>
/// Where an <c>ExtCreateFontIndirectW</c>'s face name ends.
/// </summary>
/// <remarks>
/// <para>
/// <c>lfFaceName</c> is a fixed 32-code-unit field and the name ends at its first NUL. Whether
/// the rest of the field is zeroed is the producer's business, and plenty do not zero it: the
/// EMF chart in <c>2014BSA_Sunday_Killion.pptx</c> writes <c>Times New Roman\0\0</c> and then
/// twelve code units of stack rubbish (<c>7f 13 65 43 18 ee a8 08 …</c>).
/// </para>
/// <para>
/// A reader that <em>skips</em> the NULs rather than stopping at one asks for the concatenation,
/// which no substitution table recognises, so it falls through to the generic sans — and every
/// label of that chart came out in the wrong face at the wrong widths. It had been invisible for
/// thirty rounds because a wrong face and a right face both drew upright; it surfaced only when
/// this stack learned to synthesise an oblique, at which point the wrong face, having no italic,
/// began to lean where the reference does not.
/// </para>
/// </remarks>
public class EmfFaceNameTests
{
    private const int Mm = 100;

    /// <summary>Rubbish of the shape a real producer leaves behind the terminator.</summary>
    private static readonly ushort[] Rubbish =
        [0x137F, 0x4365, 0xEE18, 0x08A8, 0xE258, 0x001B, 0x76FD, 0x7392];

    [Fact]
    public void TheFaceNameEndsAtItsFirstNulAndTheRestOfTheFieldIsIgnored()
    {
        Recorder recorder = Draw(Page()
            .Font(1, "Liberation Serif", -300, faceTail: Rubbish)
            .Select(1)
            .Text(0, 10 * Mm, "Ag"));

        recorder.GlyphRuns.ShouldHaveSingleItem()
            .Font.RequestedFamily.ShouldBe("Liberation Serif");
    }

    [Fact]
    public void AZeroedFieldReadsTheSameName()
    {
        // The control: the same record with the tail zeroed, which is the case that always
        // worked. The two must agree, or the fix has moved the answer for well-formed files.
        Recorder clean = Draw(Page()
            .Font(1, "Liberation Serif", -300)
            .Select(1)
            .Text(0, 10 * Mm, "Ag"));

        Recorder dirty = Draw(Page()
            .Font(1, "Liberation Serif", -300, faceTail: Rubbish)
            .Select(1)
            .Text(0, 10 * Mm, "Ag"));

        dirty.GlyphRuns[0].Font.RequestedFamily
            .ShouldBe(clean.GlyphRuns[0].Font.RequestedFamily);

        // And the same face was resolved, not merely the same name asked for — which is the
        // half that decides what reaches the page.
        dirty.GlyphRuns[0].Font.FamilyName.ShouldBe(clean.GlyphRuns[0].Font.FamilyName);
        dirty.Runs[0].Origin.X.Millimetres.ShouldBe(clean.Runs[0].Origin.X.Millimetres, 0.01);
    }

    [Fact]
    public void ANameThatFillsTheWholeFieldIsNotTruncated()
    {
        // The other end of the same rule: 32 code units with no terminator at all. Stopping at
        // the first NUL must not turn into stopping early.
        string full = new('x', 31);

        Draw(Page().Font(1, full, -300).Select(1).Text(0, 10 * Mm, "A"))
            .GlyphRuns.ShouldHaveSingleItem().Font.RequestedFamily.ShouldBe(full);
    }

    private static EmfBuilder Page() => new();

    private static Recorder Draw(EmfBuilder builder)
    {
        Recorder recorder = new();
        VectorImage image = builder.Decode();
        image.Draw(recorder, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return recorder;
    }
}
