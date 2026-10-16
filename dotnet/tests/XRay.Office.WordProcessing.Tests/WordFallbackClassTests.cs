using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ww8;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Which DejaVu a word-processing filter falls back to when the named family is not installed.
/// </summary>
/// <remarks>
/// <para>
/// <strong>DejaVu Serif, and the decision belongs to the filter rather than to the resolver.</strong>
/// Measured 2026-08-21 on LibreOffice 26.2.4.2 with 98 authored one-line files plus 28 cross-format
/// ones, each converted by the installed <c>soffice</c> and the drawn face read out of the PDF
/// (<c>probes/words-r54/font-fallback-rule.py</c>,
/// <c>probes/words-r54/cross-format-fallback.py</c>): DOCX, DOC and RTF all answer DejaVu Serif for
/// a family nothing on the machine has, while ODF text, XLSX, PPTX and flat ODS all answer whatever
/// generic fontconfig files the name under. So the same resolver has to give two different answers,
/// and the difference is carried in by the reader as <see cref="FontRequest.DeclaredClass"/>.
/// </para>
/// <para>
/// The reach that makes it worth a test: over all 337 words renderings, 86 disagreed with the
/// reference's embedded font list and <b>32 of them were exactly this</b> — ours DejaVu Sans where
/// the reference has DejaVu Serif, on documents that otherwise pass. The two faces have different
/// advances, so each one is a line-breaking difference as well as a visible one.
/// </para>
/// <para>
/// Three things survive the default, all three asserted below because all three were measured to:
/// a <b>strong metric alias</b> (Arial answers Liberation Sans, Times New Roman answers Liberation
/// Serif, Calibri answers Carlito), a <b>declared swiss</b> family, and an <b>installed</b> family.
/// What does <em>not</em> survive it is fontconfig's own filing — <c>Consolas</c> is
/// <c>monospace</c> to <c>45-latin.conf</c> and <c>fc-match</c> answers DejaVu Sans Mono for it,
/// and 26.2.4.2 through the DOCX filter still draws DejaVu Serif.
/// </para>
/// </remarks>
public sealed class WordFallbackClassTests
{
    // ------------------------------------------------------------------ the mapping itself

    [Theory]
    [InlineData(FontFamilyClass.SansSerif, FontFamilyClass.SansSerif)]
    [InlineData(FontFamilyClass.Serif, FontFamilyClass.Serif)]
    [InlineData(FontFamilyClass.Unknown, FontFamilyClass.Serif)]
    [InlineData(FontFamilyClass.Fixed, FontFamilyClass.Serif)]
    [InlineData(FontFamilyClass.Symbol, FontFamilyClass.Serif)]
    public void OnlyADeclaredSansSerifEscapesTheRomanDefault(
        FontFamilyClass declared, FontFamilyClass expected)
    {
        // `swiss` is the one code that moves the reference; `roman`, `modern`, `script`,
        // `decorative`, `auto` and an absent entry all leave it at Serif, and so does a declared
        // fixed pitch. Measured on four families × eight declarations = 32 authored packages.
        WordFallbackClass.ForDeclared("Aptos", declared).ShouldBe(expected);
    }

    [Theory]
    [InlineData(FontFamilyClass.SansSerif)]
    [InlineData(FontFamilyClass.Serif)]
    [InlineData(FontFamilyClass.Unknown)]
    public void OnlyTheWesternItemCarriesADeclaredClassAtAll(FontFamilyClass declared)
    {
        // `LN_CT_Fonts_ascii` inserts PROP_CHAR_FONT_FAMILY and `LN_CT_Fonts_eastAsia` and
        // `LN_CT_Fonts_cs` insert the name alone
        // (`sw/source/writerfilter/dmapper/DomainMapper.cxx`:436-508), so the CJK and CTL items keep
        // the pool default's family type -- which `OutputDevice::GetDefaultFont` sets to
        // FAMILY_SYSTEM for CJK_TEXT and CTL_TEXT, and which appends no generic at all. Measured on
        // 26.2.4.2 (`probes/fonts-r65/gen-scriptitem.py`): a `w:hint="eastAsia"` run drawing U+2610
        // answers Unifont and a complex-script run drawing U+05D0 answers FreeSans under *every*
        // declared class, including the `swiss` that moves the western row.
        WordFallbackClass.ForScript("Aptos", declared, WriterScript.Asian)
            .ShouldBe(FontFamilyClass.Unknown);
        WordFallbackClass.ForScript("Aptos", declared, WriterScript.Complex)
            .ShouldBe(FontFamilyClass.Unknown);

        // And the western item is unchanged, which is the control.
        WordFallbackClass.ForScript("Aptos", declared, WriterScript.Western)
            .ShouldBe(WordFallbackClass.ForDeclared("Aptos", declared));
    }

    /// <summary>A run naming no family at all is not a run naming a family nobody has.</summary>
    /// <remarks>
    /// The discriminating case, and the one this fix got wrong on its first sweep: a declared class
    /// is consulted in the pre-match step, <em>before</em>
    /// <c>SystemFontResolver.GenericFallbacks</c> gets to separate "no font named" from "a font
    /// nobody has", so handing one over for an empty name bypasses the <c>DefaultFonts</c> answer.
    /// Measured cost of not having this guard: 29 corpus <c>.doc</c> documents moved from Liberation
    /// Serif to DejaVu Serif and 17 verdicts were lost. `.doc` is where it shows because the WW8
    /// reader routinely produces a run with no family, while a DOCX run inherits one from its style.
    /// </remarks>
    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    public void ARunNamingNoFamilyAtAllKeepsTheApplicationDefault(string? familyName)
    {
        WordFallbackClass.ForDeclared(familyName, FontFamilyClass.Unknown)
            .ShouldBe(FontFamilyClass.Unknown);

        SystemFontResolver resolver = new(InstalledIndex());
        LayoutFonts fonts = new(resolver);

        // Liberation Serif, which is what a DOCX whose docDefaults state an empty w:rFonts renders
        // in on 26.2.4.2, and what a flat ODF file declaring no font at all renders in.
        fonts.Reference(familyName, 400, isItalic: false)!.FamilyName
             .ShouldBe("Liberation Serif");
    }

    // ------------------------------------------------------- the DOCX filter, end to end

    [Theory]
    [InlineData("Aptos", null, "DejaVu Serif")]
    [InlineData("Aptos", "roman", "DejaVu Serif")]
    [InlineData("Aptos", "modern", "DejaVu Serif")]
    [InlineData("Aptos", "auto", "DejaVu Serif")]
    [InlineData("Aptos", "swiss", "DejaVu Sans")]
    [InlineData("Candara", null, "DejaVu Serif")]
    [InlineData("Candara", "swiss", "DejaVu Sans")]
    [InlineData("Consolas", null, "DejaVu Serif")]
    [InlineData("Consolas", "swiss", "DejaVu Sans")]
    [InlineData("Garamond", null, "DejaVu Serif")]
    public void AnUnrecognisedFamilyInADocxFallsBackTheWayTheReferenceDoes(
        string family, string? declared, string expected)
    {
        // Candara is filed sans-serif and Consolas monospace by fontconfig, and `fc-match` answers
        // DejaVu Sans and DejaVu Sans Mono for them. The DOCX filter overrides both.
        FamilyDrawnFor(family, declared).ShouldBe(expected);
    }

    [Theory]
    [InlineData("Arial", "Liberation Sans")]
    [InlineData("Times New Roman", "Liberation Serif")]
    [InlineData("Calibri", "Carlito")]
    public void AStrongMetricAliasStillBeatsTheDefault(string family, string expected)
    {
        // The control that says the default is a *fallback* and not a blanket. These three are what
        // makes a DOCX lay out at all without the Microsoft fonts, and the reference answers each of
        // them with its alias whether or not the font table declares anything.
        FamilyDrawnFor(family, declared: null).ShouldBe(expected);
        FamilyDrawnFor(family, declared: "roman").ShouldBe(expected);
    }

    [Fact]
    public void AnInstalledFamilyIsNotSubstitutedAtAll()
    {
        // The other end of the same control: nothing about the default reaches a family that is here.
        FamilyDrawnFor("Liberation Serif", declared: null).ShouldBe("Liberation Serif");
        FamilyDrawnFor("DejaVu Sans", declared: null).ShouldBe("DejaVu Sans");
    }

    // ---------------------------------------------------- the DOC and RTF arm, via LayoutFonts

    /// <summary>The two arms that reach <see cref="LayoutFonts"/> take <em>different</em> defaults.</summary>
    /// <remarks>
    /// <para>
    /// Round 54 recorded them as the same and it was wrong, because its DOC probe was a DOCX round
    /// trip through LibreOffice and the DOCX <em>import</em> had already applied the roman default —
    /// so the <c>.doc</c> it measured declared <c>ff=roman</c>. A flat ODF file exported to Word 97
    /// has no such default to bake in, and nine of them
    /// (<c>probes/words-r55/doc-family-code.py</c>) say that through the DOC filter <b>only
    /// <c>ff=roman</c> draws DejaVu Serif</b>: no code at all, <c>modern</c> and <c>decorative</c>
    /// all reach fontconfig's own generic.
    /// </para>
    /// <para>
    /// RTF is the arm that keeps the roman default, and for a reason that unifies the three rather
    /// than adding a rule: its filter never sets the family at all — <c>\fnil</c>, <c>\froman</c>,
    /// <c>\fswiss</c> and <c>\fmodern</c> are all inert — so Writer's roman pool default stands.
    /// The DOCX filter leaves an inherited value whose floor is that same default. The WW8 filter is
    /// the only one of the three that writes <c>FAMILY_DONTKNOW</c> onto the item.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheRtfArmTakesTheRomanDefaultAndTheDocArmDoesNot()
    {
        SystemFontIndex index = InstalledIndex();

        // RTF reaches LayoutFonts with no font table at all, so there is nothing to read and the
        // roman default is the whole answer.
        LayoutFonts rtf = new(new SystemFontResolver(index));
        rtf.Reference("Aptos", 400, isItalic: false)!.FamilyName.ShouldBe("DejaVu Serif");
        rtf.Reference("Consolas", 400, isItalic: false)!.FamilyName.ShouldBe("DejaVu Serif");

        // DOC carries the FFN's family code and it is the whole answer, `Unknown` included.
        LayoutFonts swiss = new(new SystemFontResolver(index))
        {
            DeclaredShapes = _ => new DeclaredFontShape(FontFamilyClass.SansSerif),
        };
        swiss.Reference("Aptos", 400, isItalic: false)!.FamilyName.ShouldBe("DejaVu Sans");

        LayoutFonts roman = new(new SystemFontResolver(index))
        {
            DeclaredShapes = _ => new DeclaredFontShape(FontFamilyClass.Serif),
        };
        roman.Reference("Aptos", 400, isItalic: false)!.FamilyName.ShouldBe("DejaVu Serif");

        // `ff` 0, 6 and 7 are FAMILY_DONTKNOW, which appends no generic and lands on fontconfig's
        // own answer — DejaVu Sans for `Aptos`, DejaVu Sans **Mono** for `Consolas`. This is the
        // case round 54 could not measure and got backwards.
        LayoutFonts undeclared = new(new SystemFontResolver(index))
        {
            DeclaredShapes = _ => default,
        };
        undeclared.Reference("Aptos", 400, isItalic: false)!.FamilyName.ShouldBe("DejaVu Sans");
        undeclared.Reference("Consolas", 400, isItalic: false)!.FamilyName.ShouldBe("DejaVu Sans Mono");

        // And the guard survives it: a run naming no family at all still reaches DefaultFonts.
        undeclared.Reference(null, 400, isItalic: false)!.FamilyName.ShouldBe("Liberation Serif");
        undeclared.Reference("", 400, isItalic: false)!.FamilyName.ShouldBe("Liberation Serif");
    }

    /// <summary>
    /// The WW8 reader overrides the <c>FFN</c>'s code for fourteen name prefixes, and the DOCX one
    /// does not.
    /// </summary>
    /// <remarks>
    /// <c>SwWW8ImplReader::GetFontParams</c>'s own reason is that the code "might be set wrong when
    /// Doc was not created by Winword but by third party program". Measured: a flat ODF declaring no
    /// generic, exported to Word 97 and back, draws <c>Garamond</c> in DejaVu Serif and the otherwise
    /// identical <c>Aptos</c> in DejaVu Sans; <c>Univers</c> and <c>Helvetica</c> both draw Sans.
    /// </remarks>
    [Theory]
    [InlineData("Garamond", FontFamilyClass.Serif)]
    [InlineData("CG Times", FontFamilyClass.Serif)]
    [InlineData("Times New Roman Bold", FontFamilyClass.Serif)]
    [InlineData("Helvetica", FontFamilyClass.SansSerif)]
    [InlineData("Helv", FontFamilyClass.SansSerif)]
    [InlineData("Univers", FontFamilyClass.SansSerif)]
    [InlineData("Lucida Sans Unicode", FontFamilyClass.SansSerif)]
    // No prefix matches, so the entry's own code — nothing — stands.
    [InlineData("Aptos", FontFamilyClass.Unknown)]
    [InlineData("Candara", FontFamilyClass.Unknown)]
    public void TheWw8ReaderOverridesTheFfnCodeByName(string family, FontFamilyClass expected)
        => Ww8FontTable.Empty.ShapeOf(family).Class.ShouldBe(expected);

    // ------------------------------------------------------------------- the reach control

    /// <summary>
    /// The shared resolver is unchanged, which is what keeps slides, sheets and ODF out of it.
    /// </summary>
    /// <remarks>
    /// This is the test that would have caught the tempting version of this fix. Putting the roman
    /// default in <c>SystemFontResolver.GenericFallbacks</c> reads as a one-line change and is wrong
    /// three ways over: authored PPTX, XLSX and flat ODS files answer DejaVu Sans, DejaVu Sans and
    /// DejaVu Sans Mono for these same three families — <c>fc-match</c>'s own column — and over all
    /// 302 slides and 307 sheets renderings compared against the reference's font lists, <b>zero</b>
    /// documents show ours DejaVu Sans against the reference's DejaVu Serif.
    /// </remarks>
    [Theory]
    [InlineData("Aptos", "DejaVu Sans")]
    [InlineData("Candara", "DejaVu Sans")]
    [InlineData("Consolas", "DejaVu Sans Mono")]
    [InlineData("Garamond", "DejaVu Serif")]
    public void ARequestThatDeclaresNothingStillGetsFontconfigsGeneric(string family, string expected)
    {
        SystemFontResolver resolver = new(InstalledIndex());

        resolver.Resolve(new FontRequest(family)).FamilyName.ShouldBe(expected);
    }

    // ------------------------------------------------------------------------- the harness

    private static SystemFontIndex InstalledIndex()
    {
        SystemFontIndex index = SystemFontIndex.Build();
        Assert.SkipWhen(index.FamilyCount == 0, "no fonts are installed; see check-env.sh");
        Assert.SkipUnless(index.Has("DejaVu Serif") && index.Has("DejaVu Sans"),
                          "fonts-dejavu-core is not installed; see check-env.sh");
        return index;
    }

    /// <summary>The family actually drawn for a one-run package naming <paramref name="family"/>.</summary>
    /// <param name="family">What <c>w:rFonts</c> asks for.</param>
    /// <param name="declared">
    /// The <c>w:family</c> its <c>fontTable.xml</c> entry states, or null for a package whose font
    /// table does not mention it — which is the case the roman default exists for.
    /// </param>
    private static string FamilyDrawnFor(string family, string? declared)
    {
        InstalledIndex();

        using DocumentSource source =
            DocumentSource.FromStream(BuildPackage(family, declared), "fallback.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PageParagraph paragraph = pages.Paragraphs.First(p => p.Text.StartsWith("Hand", StringComparison.Ordinal));

        // The paragraph's own face when the run does not differ from it — a run is only emitted
        // where it does, so asking `Runs[0]` alone finds nothing precisely when the answer is the
        // one the paragraph mark also resolved to.
        return (paragraph.Runs.Count > 0 ? paragraph.Runs[0].Font : paragraph.Font)?.FamilyName
               ?? throw new InvalidOperationException("the run resolved to no face at all");
    }

    private static MemoryStream BuildPackage(string family, string? declared)
    {
        const string RelationshipsNamespace =
            "http://schemas.openxmlformats.org/package/2006/relationships";
        const string OfficeRelationships =
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

        string contentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
              <Override PartName="/word/fontTable.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"/>
            </Types>
            """;

        string rootRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="{RelationshipsNamespace}">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="{OfficeRelationships}/officeDocument"/>
            </Relationships>
            """;

        string documentRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="{RelationshipsNamespace}">
              <Relationship Id="rId1" Target="settings.xml" Type="{OfficeRelationships}/settings"/>
              <Relationship Id="rId2" Target="fontTable.xml" Type="{OfficeRelationships}/fontTable"/>
            </Relationships>
            """;

        // Carried for the reason DocxColumnGapTests carries it: a hand-built DOCX without a settings
        // part misses LibreOffice's OOXML compatibility defaults and answers a different question.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        // The entry names a *different* family when `declared` is null, so the part is present and
        // parsed either way and the two cases differ only in whether this family is in it.
        string entry = declared is null
            ? """<w:font w:name="Some Other Family"><w:family w:val="swiss"/></w:font>"""
            : $"""<w:font w:name="{family}"><w:family w:val="{declared}"/></w:font>""";

        string fontTable = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:fonts xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              {entry}
            </w:fonts>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr><w:rPr><w:rFonts w:ascii="{family}" w:hAnsi="{family}"/></w:rPr></w:pPr>
                  <w:r>
                    <w:rPr><w:rFonts w:ascii="{family}" w:hAnsi="{family}"/></w:rPr>
                    <w:t>Handgloves quick brown fox 12345</w:t>
                  </w:r>
                </w:p>
                <w:sectPr>
                  <w:pgSz w:w="11906" w:h="16838"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="720" w:footer="720" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", contentTypes);
            Write(archive, "_rels/.rels", rootRelationships);
            Write(archive, "word/_rels/document.xml.rels", documentRelationships);
            Write(archive, "word/settings.xml", Settings);
            Write(archive, "word/fontTable.xml", fontTable);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
