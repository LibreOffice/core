using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A DOCX <c>REF</c> field draws the result the producer cached, as Word does — and 26.2.4.2
/// recomputes it from the bookmark.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The two disagree about <em>when</em> a field is evaluated, not about what it evaluates
/// to.</strong> Word does not update a <c>REF</c> when a document is opened or printed: the reader
/// sees what was last computed, and only F9 or an explicit *update fields before printing* changes
/// it. LibreOffice recomputes every one on load — a <c>REF</c> naming a bookmark becomes a
/// <c>SwGetRefField</c> with <c>ReferenceFieldSource::BOOKMARK</c>
/// (<c>dmapper/DomainMapper_Impl.cxx</c>:8541-8635) and <c>UpdateField</c> reads the bookmark's own
/// text through <c>FindAnchor</c> (<c>reffld.cxx</c>:1559-1591, whose <b>−1</b> for two halves in
/// different nodes <c>:603-607</c> reads as <em>to the end of the paragraph</em>), filtered by
/// <c>FilterText</c> (<c>:461-489</c>).
/// </para>
/// <para>
/// This tree follows Word, so the first group below asserts the cache. The second pins the
/// reference's rule through <c>DocxReferenceFields.Resolve</c>, because the measurement was the
/// expensive half of the round that made it and it should not have to be made twice: every value
/// there is 26.2.4.2's own rendering of the committed fixture
/// <c>tests/corpus/features/words-reference-field.docx</c>
/// (<c>probes/docxref-r158/reference-arms.txt</c>), one arm per paragraph, each field caching a
/// deliberately wrong result so that what the reference draws is its own answer.
/// </para>
/// <para>
/// What following Word costs is in <c>dotnet/TODO.word-parity.md</c>. It is visible on
/// <c>FAA 2025-26 Holdover Tables.docx</c>, where 26.2.4.2 draws the nonsense its own rule produces
/// — <c>The list of fluids (Tables Table 55, Table 56, Table 57 and Table 58)</c> against the file's
/// cached <c>(Tables 55, 56, 57 and 58)</c> — and one note's expansion is a whole page of that
/// document.
/// </para>
/// </remarks>
public sealed class DocxReferenceFieldTests
{
    private const string Fixture = "words-reference-field.docx";

    /// <summary>
    /// A bookmark covering more than the cache does not change what is drawn.
    /// </summary>
    /// <remarks>
    /// Arm A: the mark covers <c>Table 7</c> across two runs and the field cached <c>7</c>. Word
    /// draws <c>7</c> until the field is updated; 26.2.4.2 draws <c>Table 7</c>.
    /// </remarks>
    [Fact]
    public void TheCachedResultIsDrawn() => Arm("A more than the cache").ShouldBe("7 .");

    /// <summary>And a deliberately stale cache is drawn stale.</summary>
    /// <remarks>
    /// Arm B, whose bookmark ends in another paragraph. The reference draws <c>half one</c>; the
    /// file says <c>stale</c> and so do we.
    /// </remarks>
    [Fact]
    public void AStaleCacheIsNotCorrected() => Arm("B into the next paragraph").ShouldBe("stale .");

    /// <summary>
    /// The compact <c>w:fldSimple</c> spelling is the same field and is treated the same way.
    /// </summary>
    /// <remarks>Arm L, which states its instruction as an attribute and its cache as its children.</remarks>
    [Fact]
    public void TheCompactFormIsDrawnFromItsCacheToo()
        => Arm("L the compact form").ShouldBe("stale .");

    /// <summary>
    /// The reference's rule, pinned against its own rendering of the same fixture.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Six expansions and three declines. The declines are this reader's policy rather than the
    /// reference's behaviour and are the same with the switch either way: a collapsed bookmark and
    /// a name no mark holds would <em>erase</em> what the file cached (26.2.4.2 draws nothing and
    /// <c>Error: Reference source not found</c> respectively), and a <c>REF</c> whose cached result
    /// is itself a field is left alone because the reference draws <b>both</b> values.
    /// </para>
    /// <para>
    /// Through <c>Resolve</c> rather than through the layout, because the switch is an environment
    /// variable and every other test in the process would see it.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheReferenceExpandsEachBookmarkAsMeasured()
    {
        Dictionary<string, string> expansions = DocxReferenceFields.Resolve(Body())!;

        expansions["A_more"].ShouldBe("Table 7");
        expansions["B_span"].ShouldBe("half one", "to the end of the start paragraph");
        expansions["E_break"].ShouldBe("Line one Line two", "a w:br reaches the field as a space");
        expansions["F_seq"].ShouldBe("Figure 1", "a field inside the bookmark contributes its value");
        expansions["G_hyphen"].ShouldBe("softhyphen and non-breaking", "FilterText's two hyphens");
        expansions["H_tab"].ShouldBe("before after", "and a w:tab is a space too");

        // A collapsed bookmark expands to nothing in a DOCX whatever it is named -- the #i81002#
        // cross-reference branch is unreachable from this importer, measured over four names -- and
        // an empty expansion is declined where it is used rather than here, because erasing what
        // the file cached is the one substitution that cannot be checked against the page.
        expansions["C_point"].ShouldBe("");
        expansions["__RefHeading__5"].ShouldBe("");

        // A name no mark holds is not recorded at all: 26.2.4.2 draws
        // `Error: Reference source not found` and our own bookmark table is what the lookup missed.
        expansions.ShouldNotContainKey("Z_missing");
    }

    /// <summary>The instruction reader, on which arguments name a bookmark at all.</summary>
    [Fact]
    public void TheInstructionReaderNamesTheBookmarkOnlyWhenTheTextIsWhatIsQuoted()
    {
        FieldInstructions.ReferenceBookmark(" REF _Ref1 \\h ").ShouldBe("_Ref1");
        FieldInstructions.ReferenceBookmark(" REF _Ref1 \\h \\* MERGEFORMAT ").ShouldBe("_Ref1");
        FieldInstructions.ReferenceBookmark(" REF \"a name\" \\h ").ShouldBe("a name");
        FieldInstructions.ReferenceBookmark(" REF _Ref1 \\p ").ShouldBeNull();
        FieldInstructions.ReferenceBookmark(" REF _Ref1 \\n ").ShouldBeNull();
        FieldInstructions.ReferenceBookmark(" PAGEREF _Ref1 \\h ").ShouldBeNull();
        FieldInstructions.ReferenceBookmark(" REF ").ShouldBeNull();
    }

    private static XElement Body()
    {
        using ZipArchive zip = ZipFile.OpenRead(Corpus.Require(Fixture));
        using Stream stream = zip.GetEntry("word/document.xml")!.Open();

        return Word.Child(XElement.Load(stream), "body")!;
    }

    /// <summary>What the fixture's arm of that label lays out, after the label and its colon.</summary>
    private static string Arm(string label)
    {
        string line = Lines().FirstOrDefault(text => text.StartsWith(label + ": ", StringComparison.Ordinal))
                      ?? throw new InvalidOperationException($"the fixture states no arm '{label}'");

        return line[(label.Length + 2)..];
    }

    private static List<string> Lines()
    {
        using DocumentSource source = DocumentSource.FromFile(Corpus.Require(Fixture));
        using IDocument document = new WordProcessingReader().Read(source);

        var pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return [.. pages.Paragraphs.Select(paragraph => paragraph.Text)];
    }
}
