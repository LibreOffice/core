using System.IO.Compression;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The DOCX family class is inherited, and the family name is not a property of it.
/// </summary>
/// <remarks>
/// <para>
/// Round 54 established that an unrecognised family drawn through a word-processing filter answers
/// DejaVu Serif unless the font table files it <c>swiss</c>, and shipped that as a <em>per-name</em>
/// lookup. It cost a verdict: <c>24-25_FAA_Holdover_Tables.docx</c> went 155 pages to 165, because
/// in that document the reference draws DejaVu <b>Sans</b> for <c>Arial Bold</c>, which its own
/// table files <c>auto</c>.
/// </para>
/// <para>
/// The rule, measured on 26.2.4.2 with 28 authored packages of one paragraph and one run — so the
/// PDF's font list has exactly one entry that can move — in
/// <c>probes/words-r55/family-inheritance.py</c>: <strong>the class is set only where
/// <c>w:rFonts/@w:ascii</c> names a font the table files <c>roman</c> or <c>swiss</c>, and every
/// other way of naming a family leaves whatever an ancestor put there.</strong> It survives
/// <c>w:docDefaults</c>, the <c>w:basedOn</c> chain at any depth, and direct run formatting; it is
/// not set by <c>w:asciiTheme</c>, which supplies the name and never the class; and nothing anywhere
/// stating one leaves it roman.
/// </para>
/// <para>
/// That is <c>DomainMapper::lcl_attribute</c>'s <c>LN_CT_Fonts_ascii</c> arm, which inserts
/// <c>PROP_CHAR_FONT_NAME</c> unconditionally and <c>PROP_CHAR_FONT_FAMILY</c> only when
/// <c>FontTable::getFontEntryByName</c> answers something other than <c>DONTKNOW</c> — and
/// <c>FontTable::lcl_sprm</c> maps only <c>roman</c> and <c>swiss</c>.
/// </para>
/// <para>
/// <c>24-25_FAA_Holdover_Tables.docx</c> is this shape exactly: <c>Normal</c> names <c>Arial</c>,
/// filed <c>swiss</c>, and <c>Heading2</c>, <c>Heading3</c> and <c>Caption</c> are
/// <c>basedOn Normal</c> and name <c>Arial Bold</c>, filed <c>auto</c>. Round 54 recorded style
/// inheritance refuted, on a reading of that document's *whole* embedded font list — which cannot
/// move, because the same table files <c>Century Gothic</c>, <c>Tahoma</c>,
/// <c>Charlotte Sans Book</c> and <c>CWFZGM+Myriad-BoldItalic</c> <c>swiss</c> as well.
/// </para>
/// </remarks>
public sealed class WordInheritedFamilyClassTests
{
    // ------------------------------------------------------------------ the rule, unit level

    /// <summary>The innermost layer that files its own <c>w:ascii</c> name decides, and no other.</summary>
    [Theory]
    // A layer whose ascii name the table files wins, innermost first.
    [InlineData("swiss:Donor", "auto:Target", FontFamilyClass.SansSerif)]
    [InlineData("roman:Donor", "auto:Target", FontFamilyClass.Serif)]
    [InlineData("swiss:Donor", "roman:Target", FontFamilyClass.Serif)]
    [InlineData("roman:Donor", "swiss:Target", FontFamilyClass.SansSerif)]
    // Nothing filed anywhere is Unknown, which `WordFallbackClass` turns into the roman default.
    [InlineData("auto:Donor", "auto:Target", FontFamilyClass.Unknown)]
    [InlineData("", "", FontFamilyClass.Unknown)]
    // `modern`, `script`, `decorative` and a pitch-only entry are all DONTKNOW to the filter, so the
    // outer layer still decides. This is the half round 54's per-name reading gets wrong.
    [InlineData("swiss:Donor", "modern:Target", FontFamilyClass.SansSerif)]
    [InlineData("swiss:Donor", "script:Target", FontFamilyClass.SansSerif)]
    [InlineData("swiss:Donor", "decorative:Target", FontFamilyClass.SansSerif)]
    [InlineData("swiss:Donor", "absent:Target", FontFamilyClass.SansSerif)]
    public void TheInnermostLayerThatFilesItsAsciiNameDecides(
        string outer, string inner, FontFamilyClass expected)
    {
        WordFontTable table = TableOf(outer, inner);

        // Innermost first, which is the order `WordStyles.RunPropertyLayers` returns.
        List<XElement> layers = [RFonts(inner), RFonts(outer)];

        WordParagraphFormats.StatedClass(layers, table).ShouldBe(expected);
    }

    /// <summary>A theme font supplies the name and never the class.</summary>
    /// <remarks>
    /// Measured both ways round: a theme font the table files <c>swiss</c> still comes out DejaVu
    /// <b>Serif</b> under a roman ancestor, and a theme font under a <c>swiss</c> ancestor comes out
    /// DejaVu Sans. `LN_CT_Fonts_asciiTheme` never touches `PROP_CHAR_FONT_FAMILY`. This is the
    /// mechanism behind the other direction of the corpus's disagreement — `ESPN-R - MCF - RA - Ed1`
    /// names `Calibri Light` through `w:majorHAnsi` and its table files that name `swiss`.
    /// </remarks>
    [Fact]
    public void AThemeFontTakesTheAncestorsClassAndNotItsOwnEntry()
    {
        WordFontTable table = TableOf("swiss:Donor", "swiss:Themed");

        XElement themed = new(
            Word.Name("rFonts"), new XAttribute(Word.Name("asciiTheme"), "minorHAnsi"));

        WordParagraphFormats.StatedClass([themed, RFonts("swiss:Donor")], table)
            .ShouldBe(FontFamilyClass.SansSerif);
        WordParagraphFormats.StatedClass([themed, RFonts("roman:Donor")], TableOf("roman:Donor", "swiss:Themed"))
            .ShouldBe(FontFamilyClass.Serif);

        // And on its own it states nothing at all, so the roman default stands.
        WordParagraphFormats.StatedClass([themed], table).ShouldBe(FontFamilyClass.Unknown);
    }

    /// <summary>Only the ascii slot carries the class.</summary>
    /// <remarks>
    /// <c>LN_CT_Fonts_hAnsi</c> is <c>break; //unsupported</c> in <c>DomainMapper</c>, and the
    /// <c>cs</c> and <c>eastAsia</c> arms set only a symbol charset. Measured: a run stating nothing
    /// but <c>w:hAnsi</c> draws the docDefaults' family, not its own.
    /// </remarks>
    [Fact]
    public void OnlyTheAsciiSlotStatesAClass()
    {
        WordFontTable table = TableOf("swiss:Donor", "swiss:Target");

        foreach (string slot in new[] { "hAnsi", "cs", "eastAsia" })
        {
            XElement fonts = new(
                Word.Name("rFonts"), new XAttribute(Word.Name(slot), "Target"));

            WordParagraphFormats.StatedClass([fonts], table).ShouldBe(FontFamilyClass.Unknown);
        }
    }

    /// <summary>With no table there is nothing to ask, and the roman default is the whole answer.</summary>
    [Fact]
    public void NoFontTableStatesNothing()
        => WordParagraphFormats.StatedClass([RFonts("swiss:Donor")], null)
            .ShouldBe(FontFamilyClass.Unknown);

    // ------------------------------------------------------------- the same rule, end to end

    /// <summary>
    /// The shape that cost round 54 a verdict: a style <c>basedOn</c> one that names a filed family.
    /// </summary>
    [Theory]
    [InlineData("swiss", "DejaVu Sans")]
    [InlineData("roman", "DejaVu Serif")]
    public void AStyleInheritsItsBaseStylesClassThroughAnUnfiledName(string donor, string expected)
        => DrawnFamily(
            normalFont: "Donor", normalClass: donor,
            derivedFont: "Target", derivedClass: "auto",
            useDerivedStyle: true).ShouldBe(expected);

    /// <summary>And direct run formatting inherits it too, which is where round 54 looked.</summary>
    /// <remarks>
    /// Its authored probe put the unfiled name on the run rather than on a style and read DejaVu
    /// Serif. It does not reproduce: 26.2.4.2 answers DejaVu Sans for exactly this package.
    /// </remarks>
    [Theory]
    [InlineData("swiss", "DejaVu Sans")]
    [InlineData("roman", "DejaVu Serif")]
    public void DirectRunFormattingInheritsTheStylesClass(string donor, string expected)
        => DrawnFamily(
            normalFont: "Donor", normalClass: donor,
            derivedFont: "Target", derivedClass: "auto",
            useDerivedStyle: false).ShouldBe(expected);

    /// <summary>The document defaults are a layer like any other.</summary>
    [Theory]
    [InlineData("swiss", "DejaVu Sans")]
    [InlineData("roman", "DejaVu Serif")]
    public void TheDocumentDefaultsDonateTheClass(string donor, string expected)
        => DrawnFamily(
            normalFont: null, normalClass: null,
            derivedFont: "Target", derivedClass: "auto",
            useDerivedStyle: false, defaultFont: "Donor", defaultClass: donor).ShouldBe(expected);

    /// <summary>An entry of its own beats an ancestor, in both directions.</summary>
    [Theory]
    [InlineData("swiss", "roman", "DejaVu Serif")]
    [InlineData("roman", "swiss", "DejaVu Sans")]
    public void TheRunsOwnFiledEntryBeatsTheAncestor(string donor, string own, string expected)
        => DrawnFamily(
            normalFont: "Donor", normalClass: donor,
            derivedFont: "Target", derivedClass: own,
            useDerivedStyle: true).ShouldBe(expected);

    /// <summary>The control: nothing anywhere states a class, and the roman default stands.</summary>
    /// <remarks>
    /// Round 54's whole result, restated here so that a change to the inheritance cannot quietly
    /// take the default with it. This is a control, not a detector.
    /// </remarks>
    [Fact]
    public void NothingStatingAClassIsStillTheRomanDefault()
        => DrawnFamily(
            normalFont: "Donor", normalClass: "auto",
            derivedFont: "Target", derivedClass: "auto",
            useDerivedStyle: true).ShouldBe("DejaVu Serif");

    /// <summary>The other control: an installed family never reaches any of this.</summary>
    [Fact]
    public void AnInstalledFamilyIgnoresTheInheritedClass()
        => DrawnFamily(
            normalFont: "Donor", normalClass: "swiss",
            derivedFont: "Liberation Serif", derivedClass: "auto",
            useDerivedStyle: true).ShouldBe("Liberation Serif");

    // ------------------------------------------------------------------------- the harness

    /// <summary>A <c>w:rFonts</c> naming one family in its ascii slot, or nothing.</summary>
    private static XElement RFonts(string spec)
        => spec.Length == 0
            ? new XElement(Word.Name("rFonts"))
            : new XElement(Word.Name("rFonts"), new XAttribute(Word.Name("ascii"), Name(spec)));

    private static string Name(string spec) => spec.Split(':')[1];

    /// <summary>A font table from <c>class:name</c> specs; <c>absent:</c> leaves the name out.</summary>
    private static WordFontTable TableOf(params string[] specs)
    {
        XElement fonts = new(Word.Name("fonts"));

        foreach (string spec in specs)
        {
            if (spec.Length == 0 || spec.StartsWith("absent:", StringComparison.Ordinal)) continue;

            fonts.Add(new XElement(
                Word.Name("font"),
                new XAttribute(Word.Name("name"), Name(spec)),
                new XElement(Word.Name("family"),
                             new XAttribute(Word.Name("val"), spec.Split(':')[0]))));
        }

        return WordFontTable.Read(fonts);
    }

    private static string DrawnFamily(
        string? normalFont, string? normalClass, string derivedFont, string derivedClass,
        bool useDerivedStyle, string? defaultFont = null, string? defaultClass = null)
    {
        SystemFontIndex index = SystemFontIndex.Build();
        Assert.SkipWhen(index.FamilyCount == 0, "no fonts are installed; see check-env.sh");
        Assert.SkipUnless(index.Has("DejaVu Serif") && index.Has("DejaVu Sans"),
                          "fonts-dejavu-core is not installed; see check-env.sh");

        using DocumentSource source = DocumentSource.FromStream(
            BuildPackage(normalFont, normalClass, derivedFont, derivedClass, useDerivedStyle,
                         defaultFont, defaultClass),
            "inherited.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PageParagraph paragraph =
            pages.Paragraphs.First(p => p.Text.StartsWith("Hand", StringComparison.Ordinal));

        return (paragraph.Runs.Count > 0 ? paragraph.Runs[0].Font : paragraph.Font)?.FamilyName
               ?? throw new InvalidOperationException("the run resolved to no face at all");
    }

    private static MemoryStream BuildPackage(
        string? normalFont, string? normalClass, string derivedFont, string derivedClass,
        bool useDerivedStyle, string? defaultFont, string? defaultClass)
    {
        const string Pkg = "http://schemas.openxmlformats.org/package/2006/relationships";
        const string Off = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

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
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
              <Override PartName="/word/fontTable.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"/>
            </Types>
            """;

        string rootRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="{Pkg}">
              <Relationship Id="rId1" Target="word/document.xml" Type="{Off}/officeDocument"/>
            </Relationships>
            """;

        string documentRelationships = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="{Pkg}">
              <Relationship Id="rId1" Target="settings.xml" Type="{Off}/settings"/>
              <Relationship Id="rId2" Target="fontTable.xml" Type="{Off}/fontTable"/>
              <Relationship Id="rId3" Target="styles.xml" Type="{Off}/styles"/>
            </Relationships>
            """;

        // Carried for the reason `WordFallbackClassTests` carries it: a hand-built DOCX with no
        // settings part misses LibreOffice's OOXML compatibility defaults.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        string Entry(string? name, string? declared)
            => name is null || declared is null or "absent"
                ? string.Empty
                : $"""<w:font w:name="{name}"><w:family w:val="{declared}"/></w:font>""";

        string fontTable = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:fonts xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              {Entry(normalFont, normalClass)}
              {Entry(defaultFont, defaultClass)}
              {Entry(derivedFont, derivedClass)}
            </w:fonts>
            """;

        string Fonts(string? name)
            => name is null ? string.Empty : $"""<w:rFonts w:ascii="{name}" w:hAnsi="{name}"/>""";

        string styles = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault><w:rPr>{Fonts(defaultFont)}</w:rPr></w:rPrDefault>
                <w:pPrDefault/>
              </w:docDefaults>
              <w:style w:type="paragraph" w:styleId="Normal">
                <w:name w:val="Normal"/>
                <w:rPr>{Fonts(normalFont)}</w:rPr>
              </w:style>
              <w:style w:type="paragraph" w:styleId="Derived">
                <w:name w:val="Derived"/>
                <w:basedOn w:val="Normal"/>
                <w:rPr>{Fonts(derivedFont)}</w:rPr>
              </w:style>
            </w:styles>
            """;

        // Either the derived *style* names the target family, or the paragraph uses `Normal` and the
        // run names it directly. Both were measured and both inherit.
        string paragraphProperties = useDerivedStyle
            ? """<w:pPr><w:pStyle w:val="Derived"/></w:pPr>"""
            : """<w:pPr><w:pStyle w:val="Normal"/></w:pPr>""";
        string runProperties = useDerivedStyle ? string.Empty : $"<w:rPr>{Fonts(derivedFont)}</w:rPr>";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  {paragraphProperties}
                  <w:r>{runProperties}<w:t>Handgloves quick brown fox 12345</w:t></w:r>
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
            Write(archive, "word/styles.xml", styles);
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
