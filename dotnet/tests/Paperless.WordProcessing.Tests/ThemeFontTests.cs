using System.Xml.Linq;
using Paperless.Ooxml.DrawingML;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests the indirection a <c>w:rFonts</c> may use instead of naming a family:
/// <c>w:asciiTheme="minorHAnsi"</c> and its seven companions.
/// </summary>
/// <remarks>
/// <para>
/// This is not a rare corner. Word writes the indirect form for every run of every document
/// saved from an unmodified Office template, so a reader that ignores it never sees the theme's
/// face at all and falls back to whatever <c>w:docDefaults</c> names — which in a converted
/// document is usually Times New Roman where the theme says Calibri.
/// </para>
/// <para>
/// The cost is not that the shapes are wrong. It is that the two substitutes have different
/// vertical metrics: Liberation Serif's line box is 2268/2048 of the em and Carlito's is
/// 2500/2048, so believing the wrong one makes every line six per cent short. Six per cent
/// compounds down a page into one extra line, and down a hundred and fifty pages into a
/// document that is fifty pages shorter than the reference while containing the same words.
/// </para>
/// <para>
/// The mapping is LibreOffice's <c>resolveMajorMinorTypeFace</c>
/// (<c>sw/source/writerfilter/dmapper/ThemeHandler.cxx</c>:323); the precedence — the indirect
/// attribute beating the direct one beside it — is <c>DomainMapper::lcl_attribute</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:453), which says so in a comment.
/// </para>
/// </remarks>
public class ThemeFontTests
{
    private static readonly XNamespace W =
        "http://schemas.openxmlformats.org/wordprocessingml/2006/main";

    private static readonly XNamespace A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>
    /// A theme whose six faces are all different, so a mapping that reaches the wrong one says so.
    /// </summary>
    private static DrawingTheme Theme() => DrawingTheme.Read(
        new XElement(A + "theme",
            new XElement(A + "themeElements",
                new XElement(A + "fontScheme",
                    new XElement(A + "majorFont",
                        new XElement(A + "latin", new XAttribute("typeface", "Cambria")),
                        new XElement(A + "ea", new XAttribute("typeface", "MajorAsian")),
                        new XElement(A + "cs", new XAttribute("typeface", "MajorComplex"))),
                    new XElement(A + "minorFont",
                        new XElement(A + "latin", new XAttribute("typeface", "Calibri")),
                        new XElement(A + "ea", new XAttribute("typeface", "MinorAsian")),
                        new XElement(A + "cs", new XAttribute("typeface", "MinorComplex")))))))
        .ShouldNotBeNull();

    /// <summary>
    /// Styles whose defaults name a family outright, which is what the indirection has to beat.
    /// </summary>
    private static WordStyles Styles()
    {
        WordStyles styles = new();
        styles.Add(new XElement(W + "styles",
            new XElement(W + "docDefaults",
                new XElement(W + "rPrDefault",
                    new XElement(W + "rPr",
                        new XElement(W + "rFonts",
                            new XAttribute(W + "ascii", "Times New Roman"),
                            new XAttribute(W + "hAnsi", "Times New Roman")))))));
        return styles;
    }

    /// <summary>The Latin family a run's <c>w:rFonts</c> attributes resolve to.</summary>
    private static string? Family(DrawingTheme? theme, params (string Name, string Value)[] attributes)
    {
        XElement fonts = new(W + "rFonts");
        foreach ((string name, string value) in attributes)
        {
            fonts.SetAttributeValue(W + name, value);
        }

        return WordCharacterFormat
            .Resolve(Styles(), new XElement(W + "rPr", fonts), paragraphStyleId: null, theme)
            .FontName;
    }

    [Fact]
    public void AThemeReferenceReachesTheThemesFaceRatherThanTheDocumentDefault()
    {
        Family(Theme(), ("asciiTheme", "minorHAnsi")).ShouldBe("Calibri");
        Family(Theme(), ("asciiTheme", "majorHAnsi")).ShouldBe("Cambria");
    }

    [Fact]
    public void TheEightThemeNamesReachTheSixFacesTheSchemeDeclares()
    {
        DrawingFontScheme scheme = Theme().Fonts.ShouldNotBeNull();

        // The scheme holds one Latin face, so the ASCII and high-ANSI names are the same face.
        WordParagraphFormats.ThemeFace(scheme, "majorAscii").ShouldBe("Cambria");
        WordParagraphFormats.ThemeFace(scheme, "majorHAnsi").ShouldBe("Cambria");
        WordParagraphFormats.ThemeFace(scheme, "majorEastAsia").ShouldBe("MajorAsian");
        WordParagraphFormats.ThemeFace(scheme, "majorBidi").ShouldBe("MajorComplex");
        WordParagraphFormats.ThemeFace(scheme, "minorAscii").ShouldBe("Calibri");
        WordParagraphFormats.ThemeFace(scheme, "minorHAnsi").ShouldBe("Calibri");
        WordParagraphFormats.ThemeFace(scheme, "minorEastAsia").ShouldBe("MinorAsian");
        WordParagraphFormats.ThemeFace(scheme, "minorBidi").ShouldBe("MinorComplex");

        // Not a theme name at all, and not a family name either — it must not be mistaken for one.
        WordParagraphFormats.ThemeFace(scheme, "somethingElse").ShouldBeNull();
    }

    /// <summary>
    /// Word writes both attributes, and the indirect one is what it obeys.
    /// </summary>
    /// <remarks>
    /// The direct attribute holds the face the theme had when the document was last saved, which
    /// is stale as soon as the theme changes. Preferring it renders the document as it was rather
    /// than as it is, and does so silently.
    /// </remarks>
    [Fact]
    public void TheIndirectAttributeBeatsTheDirectOneBesideIt()
        => Family(Theme(), ("ascii", "Verdana"), ("asciiTheme", "minorHAnsi")).ShouldBe("Calibri");

    /// <summary>
    /// A package with no theme part falls back to the direct attribute rather than to nothing.
    /// </summary>
    /// <remarks>
    /// Which is why the indirection is followed first but not exclusively: a document may name a
    /// theme face and ship no theme part, and the stale direct attribute beside it is then the
    /// only family anyone knows.
    /// </remarks>
    [Fact]
    public void WithNoThemeTheDirectAttributeIsStillUsed()
        => Family(theme: null, ("ascii", "Verdana"), ("asciiTheme", "minorHAnsi")).ShouldBe("Verdana");

    /// <summary>
    /// Each of <c>w:rFonts</c>' four families is inherited in its own right.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A run naming only a complex-script face keeps its style's Latin one. This is the shape Word
    /// writes constantly — <c>&lt;w:rFonts w:cs="Arial"/&gt;</c> beside a <c>w:szCs</c>, on a run of
    /// ordinary Latin text — and it appears in three quarters of the corpus's DOCX files. Treating
    /// the innermost element as the whole answer leaves such a run with no Latin family at all, and
    /// the search then falls through to the complex-script one, setting Latin text in it.
    /// </para>
    /// <para>
    /// LibreOffice gets this by construction: its importer maps each attribute to a property of its
    /// own (<c>DomainMapper::lcl_attribute</c>'s separate <c>LN_CT_Fonts_*</c> cases), so nothing is
    /// ever inherited as a group.
    /// </para>
    /// </remarks>
    [Fact]
    public void EachOfTheFourFamiliesIsInheritedOnItsOwn()
    {
        // Only a complex-script face here, so the document default's Latin one still applies.
        Family(theme: null, ("cs", "Arial")).ShouldBe("Times New Roman");

        // And with a theme, the layer below resolves through it rather than being skipped.
        Family(Theme(), ("cs", "Arial")).ShouldBe("Times New Roman");
    }

    /// <summary>
    /// A theme reference that resolves to nothing falls through to the layer below, not to the
    /// complex-script face beside it.
    /// </summary>
    [Fact]
    public void AnUnresolvableThemeReferenceFallsThroughToTheNextLayer()
        => Family(theme: null, ("asciiTheme", "minorHAnsi")).ShouldBe("Times New Roman");
}
