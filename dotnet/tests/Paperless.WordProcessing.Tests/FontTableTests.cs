using System.Xml.Linq;
using Paperless.Ooxml;
using Paperless.TestKit;
using Paperless.WordProcessing.Ooxml;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests <c>fontTable.xml</c>, which is read and reported rather than acted on.
/// </summary>
/// <remarks>
/// Nothing in layout consumes this: <c>w:rFonts</c> names a family outright, so a paragraph is
/// measured without ever opening the part. What is checked here is that the two things only this
/// part knows come out intact — the embedded-font relationships, and the PANOSE and pitch a
/// substitution would match on.
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
    }
}
