using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// The faces shipped beside the library: that they are there, that they are enough on their own,
/// and that they do not displace the machine's.
/// </summary>
/// <remarks>
/// <para>
/// The failure this guards against is the one <c>dotnet/CLAUDE.md</c> records: a container that
/// lost <c>fonts-dejavu-core</c> moved <b>53 of 534 page counts and 426 pages</b> with LibreOffice
/// held constant, and nothing announced it. <c>fc-match</c> could not have, because it never
/// fails — it always returns <em>something</em>, and that something is a face with different
/// advances, so every line breaks somewhere else.
/// </para>
/// <para>
/// The second test is the one that would have caught it. It resolves the four families that decide
/// OOXML metrics against <b>only</b> the shipped directory, so it asserts what a machine with no
/// fonts at all would get.
/// </para>
/// </remarks>
public sealed class BundledFontTests
{
    /// <summary>The faces are deployed beside the assembly and are found.</summary>
    [Fact]
    public void TheShippedFacesAreWhereTheLibraryLooks()
    {
        BundledFonts.Directory.ShouldNotBeNull(
            "the content files are copied to a `fonts` folder beside the assembly");
        Directory.EnumerateFiles(BundledFonts.Directory!, "*.ttf").ShouldNotBeEmpty();
    }

    /// <summary>
    /// The shipped set alone answers every metric-compatible substitution.
    /// </summary>
    /// <remarks>
    /// Resolved against the bundled directory and nothing else, which is a machine with no fonts
    /// installed. Each of these four is a substitution the OOXML corpus depends on: a Calibri
    /// document measured in anything but Carlito paginates differently, and the same for the other
    /// three.
    /// </remarks>
    [Theory]
    [InlineData("Calibri", "Carlito")]
    [InlineData("Cambria", "Caladea")]
    [InlineData("Arial", "Liberation Sans")]
    [InlineData("Times New Roman", "Liberation Serif")]
    [InlineData("Courier New", "Liberation Mono")]
    public void TheShippedSetAloneCoversTheMetricSubstitutions(string asked, string expected)
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build([BundledFonts.Directory!]));

        FontReference resolved = resolver.Resolve(new FontRequest(asked));

        resolved.FaceKey.ShouldNotBeEmpty($"{asked} must resolve with only the shipped faces");
        Path.GetFileName(resolved.FaceKey.Split('#')[0])
            .ShouldStartWith(expected.Replace(" ", string.Empty), Case.Insensitive);
    }

    /// <summary>Bold and italic are there too, not just the roman.</summary>
    /// <remarks>
    /// A family present in one weight only is worse than absent in a specific way: it resolves, so
    /// nothing reports a substitution, and the bold half of every heading is drawn synthetically.
    /// </remarks>
    [Theory]
    [InlineData("Calibri")]
    [InlineData("Cambria")]
    [InlineData("Arial")]
    [InlineData("Times New Roman")]
    public void TheShippedSetCarriesBoldAndItalicToo(string asked)
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build([BundledFonts.Directory!]));

        // A resolved face carries the weight it actually is, not the one that was asked for, so
        // a family shipped in one weight only answers 400 here and the assertion catches it.
        resolver.Resolve(new FontRequest(asked) { Weight = 700 }).Weight.ShouldBe(700);
        resolver.Resolve(new FontRequest(asked) { IsItalic = true }).IsItalic.ShouldBeTrue();
    }

    /// <summary>
    /// An installed face wins over a shipped one, unless the caller asks for the reverse.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The direction is measured rather than chosen. Against LibreOffice 26.2.4.2 from the TDF
    /// tarball — the build these files came from — <c>XRay.Office.Fidelity.Tests</c> failed <b>36</b>
    /// of 552 with the bundle as a fallback and <b>68</b> with it preferred, because LibreOffice
    /// resolves through fontconfig and reads the machine's copies rather than its own shipped
    /// ones. Preferring ours moves away from the reference, not towards it.
    /// </para>
    /// <para>
    /// It comes down to one family: Carlito and Liberation Sans are metrically identical between
    /// the two builds, while Caladea's differ by up to 10% per glyph.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheMachinesOwnFacesComeFirst()
    {
        int bundled = SystemFontIndex.DefaultDirectories
            .ToList()
            .FindIndex(d => string.Equals(d, BundledFonts.Directory, StringComparison.Ordinal));

        bundled.ShouldBeGreaterThan(0, "the shipped faces are searched after the machine's");
        bundled.ShouldBe(SystemFontIndex.DefaultDirectories.Count - 1);
        BundledFonts.Preferred.ShouldBeFalse("preferring them is opt-in");
    }
}
