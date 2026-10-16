using System.Xml.Linq;
using XRay.Office.Ooxml;
using XRay.Office.TestKit;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Tests <c>fontTable.xml</c>: what it holds, and the one thing layout now asks it.
/// </summary>
/// <remarks>
/// <c>w:rFonts</c> names a family outright, so a paragraph is measured without ever opening the
/// part — which is why it went unconsumed for so long. What the name cannot say is what shape a
/// family <em>nobody has installed</em> is, and that decides which face the fallback lands on.
/// So <c>w:family</c> is now read for real; <c>w:pitch</c>, the embedded-font relationships and
/// PANOSE are still reported rather than acted on, and the pitch's case is measured rather than
/// pending — see <see cref="TheDeclaredPitchIsReadButNotActedOnFromThisPart"/>.
/// </remarks>
public class FontTableTests
{
    private static DocxFile Open(string name)
        => DocxFile.Open(File.OpenRead(Corpus.Require(name)));

    [Fact]
    public void TheTableIsFoundByRelationshipAndItsEntriesRead()
    {
        using DocxFile file = Open("theme-colours.docx");

        WordFont calibri = file.FontTable.Find("Calibri").ShouldNotBeNull();
        calibri.Panose.ShouldBe("020F0502020204030204");
        calibri.Family.ShouldBe("swiss");
        calibri.Pitch.ShouldBe("variable");
        calibri.Charset.ShouldBe("00");
        calibri.IsTrueType.ShouldBeTrue();

        WordFont symbol = file.FontTable.Find("Symbol").ShouldNotBeNull();

        // w:charset="02" is the symbol set, which is the entry that changes how a run's
        // characters are interpreted rather than only how they look.
        symbol.Charset.ShouldBe("02");
        symbol.AlternativeName.ShouldBe("OpenSymbol");
        symbol.IsTrueType.ShouldBeFalse();
    }

    /// <summary>
    /// The lookup tolerates a table that names the same family twice.
    /// </summary>
    /// <remarks>
    /// Not a hypothetical: LibreOffice's own DOCX export writes two <c>Symbol</c> entries into
    /// <c>word-features.docx</c>, one with <c>w:family="roman"</c> and one with
    /// <c>w:family="auto"</c>. A dictionary built with an indexer rather than a guarded add
    /// throws on that file, so a duplicate has to be a first-wins rather than an error.
    /// </remarks>
    [Fact]
    public void ADuplicateFamilyNameKeepsTheFirstEntryRatherThanFailing()
    {
        using DocxFile file = Open("word-features.docx");

        file.FontTable.Fonts.Count(font => font.Name == "Symbol").ShouldBe(2);
        file.FontTable.Find("Symbol").ShouldNotBeNull().Family.ShouldBe("roman");

        // LibreOffice writes an altName for the faces it knows Word will not have, which is the
        // only substitution hint its own export carries — it writes no PANOSE at all.
        file.FontTable.Find("Liberation Serif").ShouldNotBeNull().AlternativeName
            .ShouldBe("Times New Roman");
        file.FontTable.Fonts.ShouldAllBe(font => font.Panose == null);
    }

    /// <summary>
    /// The embedded-font relationships, which are what the part holds that nothing else does.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Read from a fragment rather than from a corpus file, because a document that really
    /// embeds four faces carries four font files and is a megabyte, which is not a thing to keep
    /// in a repository forever. The fragment is the shape LibreOffice's own test data uses
    /// (<c>sw/qa/writerfilter/dmapper/data/subsetted-full-embedded-font.docx</c>).
    /// </para>
    /// <para>
    /// <c>w:subsetted="1"</c> rather than <c>"true"</c> is what real files write, so both spell
    /// the same flag — and the flag is the one that decides whether the embedded face could
    /// serve as a substitute at all, since a subset holds only the glyphs the document uses.
    /// </para>
    /// </remarks>
    [Fact]
    public void EmbeddedFontPartsAreRecordedWithTheirRelationshipKeyAndSubsetFlag()
    {
        XNamespace w = OoxmlNamespaces.WordprocessingML;
        XNamespace r = OoxmlNamespaces.Relationships;

        XElement fonts = new(
            w + "fonts",
            new XElement(
                w + "font",
                new XAttribute(w + "name", "IBM Plex Serif Light"),
                new XElement(w + "family", new XAttribute(w + "val", "roman")),
                new XElement(
                    w + "embedRegular",
                    new XAttribute(r + "id", "rId1"),
                    new XAttribute(w + "subsetted", "1"),
                    new XAttribute(w + "fontKey", "{96649CDE-F9E5-441A-93C3-D1EDFB9F2608}")),
                new XElement(
                    w + "embedBold",
                    new XAttribute(r + "id", "rId2"),
                    new XAttribute(w + "fontKey", "{02014A78-CABC-4EF0-12AC-5CD89AEFDE02}"))));

        WordFontTable table = WordFontTable.Read(fonts);

        table.HasEmbeddedFonts.ShouldBeTrue();

        WordFont font = table.Find("IBM Plex Serif Light").ShouldNotBeNull();
        font.Embedded.Count.ShouldBe(2);

        font.Embedded[0].Style.ShouldBe(WordEmbeddedFontStyle.Regular);
        font.Embedded[0].RelationshipId.ShouldBe("rId1");
        font.Embedded[0].Key.ShouldBe("{96649CDE-F9E5-441A-93C3-D1EDFB9F2608}");
        font.Embedded[0].IsSubsetted.ShouldBeTrue();

        font.Embedded[1].Style.ShouldBe(WordEmbeddedFontStyle.Bold);
        font.Embedded[1].IsSubsetted.ShouldBeFalse();
    }

    [Fact]
    public void APackageWithNoFontTableGetsAnEmptyOneRatherThanNull()
    {
        WordFontTable.Read(null).Fonts.ShouldBeEmpty();
        WordFontTable.Read(null).HasEmbeddedFonts.ShouldBeFalse();
        WordFontTable.Read(null).Find("Calibri").ShouldBeNull();
        WordFontTable.Read(null).ShapeOf("Calibri").Class.ShouldBe(FontFamilyClass.Unknown);
    }

    [Theory]
    [InlineData("roman", FontFamilyClass.Serif)]
    [InlineData("swiss", FontFamilyClass.SansSerif)]
    [InlineData("modern", FontFamilyClass.Unknown)]
    [InlineData("script", FontFamilyClass.Unknown)]
    [InlineData("decorative", FontFamilyClass.Unknown)]
    [InlineData("auto", FontFamilyClass.Unknown)]
    [InlineData(null, FontFamilyClass.Unknown)]
    public void OnlyRomanAndSwissBecomeAShapeTheResolverActsOn(string? declared, FontFamilyClass expected)
    {
        // Probed against LibreOffice 26.2.4.2 by holding the family name constant and varying only
        // the declaration: roman and swiss each move the fallback, and modern, script, decorative,
        // system and auto each leave it exactly where the undeclared request left it. Mapping
        // "modern" onto a monospaced fallback is the tempting mistake — the name invites it and
        // LibreOffice does not do it, so it would invent a divergence rather than remove one.
        WordFontTable.Read(Table("Some Font", declared, pitch: null))
            .ShapeOf("Some Font").Class.ShouldBe(expected);
    }

    [Theory]
    [InlineData("fixed", FontPitch.Fixed)]
    [InlineData("variable", FontPitch.Variable)]
    [InlineData("default", FontPitch.Unknown)]
    [InlineData(null, FontPitch.Unknown)]
    public void TheDeclaredPitchIsReadButNotActedOnFromThisPart(string? declared, FontPitch expected)
    {
        // Read, and deliberately not passed to the resolver from the DOCX path. LibreOffice's own
        // DOCX filter does not act on it: probed on 26.2.4.2 with a one-run package, `Garamond`
        // declared `swiss` moves the fallback from DejaVu Serif to DejaVu Sans while the same family
        // declared `fixed` leaves it exactly where it was, as does `MS Mincho` declared
        // `modern`+`fixed`. Its *ODF* filter does honour `style:font-pitch` — the same probe run
        // through a `.fodt` answers DejaVu Sans Mono — so the difference is between the two
        // importers, not a property of the resolver. Wiring it here put one corpus document into
        // DejaVu Sans Mono that the reference sets in DejaVu Sans.
        WordFontTable.Read(Table("Some Font", family: null, pitch: declared))
            .ShapeOf("Some Font").Pitch.ShouldBe(expected);
    }

    [Fact]
    public void AFamilyTheTableDoesNotNameHasNoDeclaredShape()
    {
        // The common case for a run: w:rFonts may name a family the table never declared, and the
        // answer to that is "the document said nothing" rather than a guess.
        WordFontTable table = WordFontTable.Read(Table("Some Font", "roman", "fixed"));

        table.ShapeOf("Another Font").ShouldBe(default(DeclaredFontShape));
        table.ShapeOf(null).ShouldBe(default(DeclaredFontShape));
    }

    /// <summary>A one-entry <c>w:fonts</c> root, with the two attributes under test.</summary>
    private static XElement Table(string name, string? family, string? pitch)
    {
        XNamespace w = OoxmlNamespaces.WordprocessingML;
        XElement font = new(w + "font", new XAttribute(w + "name", name));

        if (family is not null)
            font.Add(new XElement(w + "family", new XAttribute(w + "val", family)));

        if (pitch is not null)
            font.Add(new XElement(w + "pitch", new XAttribute(w + "val", pitch)));

        return new XElement(w + "fonts", font);
    }
}
