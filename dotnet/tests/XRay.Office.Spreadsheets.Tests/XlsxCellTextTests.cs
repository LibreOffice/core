using System.IO.Compression;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Ooxml;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Tests the <c>ST_Xstring</c> rules a SpreadsheetML <c>t</c> element obeys.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation below is a <em>measurement</em> of the installed LibreOffice 26.2.4.2, not a
/// reading of ECMA-376. Authored workbooks holding one string per row were converted with
/// <c>soffice --convert-to pdf</c> and the answer read out of the PDF's text-showing operators —
/// which matters, because the first two cases are indistinguishable in a downscaled raster and
/// the difference between them is 33313 characters on one corpus document.
/// </para>
/// <para>
/// The case that motivated all of this: <c>FY2018_Q4_UAS_Sightings.xlsx</c> carries 4872
/// <c>_x000D_</c> in its shared string table and rendered 304 pages against the reference's 302,
/// because seven glyphs were being drawn where Calc draws nothing.
/// </para>
/// </remarks>
public class XlsxCellTextTests
{
    [Theory]
    // The defect. A carriage return is decoded and then drawn as nothing at all — not as a line
    // break. `soffice` puts "ALPHABRAVO" in ONE Tj at ONE baseline, so the two words are glued.
    [InlineData("ALPHA_x000D_BRAVO", "ALPHABRAVO")]
    // A line feed is the one control that survives, as a break. This is what makes the pairing in
    // the real files — `_x000D_` followed by a real newline — exactly one break rather than two.
    [InlineData("CHARLIE_x000A_DELTA", "CHARLIE\nDELTA")]
    [InlineData("vv_x000D__x000A_ww", "vv\nww")]
    // `_x005F_` is how a producer writes an underscore that would otherwise open an escape.
    // Resuming the scan AFTER the decoded "_" is what leaves "x000D_" as ordinary text; a
    // regular-expression replace over the whole string decodes it a second time and loses it.
    [InlineData("ECHO_x005F_x000D_FOXTROT", "ECHO_x000D_FOXTROT")]
    // Lower-case hex is accepted. Excel writes upper case; `xl/styles.xml` in the corpus writes
    // lower, so a case-sensitive decoder is half a decoder.
    [InlineData("aa_x000d_bb", "aabb")]
    [InlineData("cc_x000a_dd", "cc\ndd")]
    // Not every escape is a control. A space is decoded and kept, and so is any BMP character.
    [InlineData("kk_x0020_ll", "kk ll")]
    [InlineData("qq_x20AC_rr", "qq€rr")]
    // Malformed is not an escape. Three hex digits, or none, and the text stands as written —
    // which is the guard that stops this rule eating "_x" out of an ordinary identifier.
    [InlineData("mm_x00D_nn", "mm_x00D_nn")]
    [InlineData("oo_xZZZZ_pp", "oo_xZZZZ_pp")]
    [InlineData("plain text", "plain text")]
    [InlineData("under_score", "under_score")]
    // A truncated escape at the very end must not read past it.
    [InlineData("tail_x00", "tail_x00")]
    [InlineData("uu_x000d_", "uu")]
    // The other C0 controls the corpus actually contains: U+001E in a syllabus workbook, U+000B
    // in a type-certificate list, U+0002 in a crowdfunding form. All three vanish.
    [InlineData("ee_x001E_ff", "eeff")]
    [InlineData("gg_x000B_hh", "gghh")]
    [InlineData("ii_x0002_jj", "iijj")]
    [InlineData("ss_x0000_tt", "sstt")]
    [InlineData("aa_x001F_bb", "aabb")]
    // U+007F is NOT in the set. LibreOffice draws it, and one corpus workbook carries one.
    [InlineData("aa_x007F_bb", "aabb")]
    public void ATElementDecodesTheWayCalcReadsOne(string raw, string expected)
        => XlsxCellText.Of(raw).ShouldBe(expected);

    [Fact]
    public void TheRuleIsOnTheCharacterAndNotOnHowItWasSpelled()
    {
        // A literal tab in a <t> and `_x0009_` are the same character once the XML is parsed, and
        // LibreOffice draws neither: both authored rows came back as "aabb". A decoder that
        // stripped only what it had itself decoded would be inventing a distinction Calc cannot
        // see — which is why the strip runs over the whole string rather than over the escapes.
        XlsxCellText.Of("aa\tbb").ShouldBe("aabb");
        XlsxCellText.Of("cc_x0009_dd").ShouldBe("ccdd");
        XlsxCellText.Of("aa\tbb").ShouldBe(XlsxCellText.Of("aa_x0009_bb"));
    }

    [Theory]
    // A line feed anywhere in the string changes what the OTHER controls mean, and this is the
    // half of the rule that is easiest to get wrong because the single-line answers look like a
    // general rule and are not. Every row was measured: the same string was put in a cell,
    // converted with `soffice`, and the resulting lines counted.
    //
    // U+000D with no line feed in sight: dropped, exactly as above.
    [InlineData("ALPHA_x000D_BRAVO", "ALPHABRAVO")]
    // The same escape in a string that also holds a line feed: a BREAK, not a drop.
    [InlineData("E_x000D_F_x000A_END", "E\nF\nEND")]
    // CR LF is one break and LF CR is one break. Both orders are real: the corpus writes the
    // second as `&#x0A;&#x0D;`, and reading it as two put a line 8.98 pt low.
    [InlineData("A_x000D__x000A_B_x000A_END", "A\nB\nEND")]
    [InlineData("C_x000A__x000D_D_x000A_END", "C\nD\nEND")]
    // …but a doubled one of the SAME character is two breaks, so the pair rule cannot be
    // "swallow any following line-ending character".
    [InlineData("I_x000D__x000D_J_x000A_END", "I\n\nJ\nEND")]
    [InlineData("G_x000A__x000A_H_x000A_END", "G\n\nH\nEND")]
    [InlineData("K_x000A_L_x000D_END", "K\nL\nEND")]
    // A tab is dropped on a single line and KEPT once there is a line feed, because Calc then
    // lays the cell out through the editing engine and the tab advances to a tab stop. 104 tabs
    // across three corpus workbooks sit in multi-line strings; dropping them glued a bullet to
    // its word and cost 76 words on one document.
    [InlineData("gg_x0009_hh", "gghh")]
    [InlineData("gg_x0009_hh_x000A_ZZ", "gg\thh\nZZ")]
    // The other C0 controls do NOT change with context — dropped either way.
    [InlineData("aa_x001E_bb_x000A_ZZ", "aabb\nZZ")]
    [InlineData("cc_x000B_dd_x000A_ZZ", "ccdd\nZZ")]
    [InlineData("ee_x0002_ff_x000A_ZZ", "eeff\nZZ")]
    [InlineData("kk_x001F_ll_x000A_ZZ", "kkll\nZZ")]
    public void ALineFeedInTheStringChangesWhatTheOtherControlsMean(string raw, string expected)
        => XlsxCellText.Of(raw).ShouldBe(expected);

    [Fact]
    public void AStringWithNothingToDecodeIsReturnedUnchanged()
    {
        // Not a micro-optimisation test: a shared string table runs to tens of thousands of
        // entries and this is the path essentially all of them take.
        const string plain = "Nothing here needs decoding at all";
        XlsxCellText.Of(plain).ShouldBeSameAs(plain);
        XlsxCellText.Of(null).ShouldBe(string.Empty);
        XlsxCellText.Of("").ShouldBe(string.Empty);
    }

    [Fact]
    public void ARichStringDecodesEveryRunAndItsOffsetsFollow()
    {
        // The trap this exists for. `XlsxRichRuns` measures run offsets into the same flattened
        // string `ReadRichString` builds, by walking the same elements — so decoding on one side
        // only leaves every run after the first escape pointing seven characters too far right,
        // and the fix comes back as mis-formatted text instead of as surplus glyphs.
        XElement item = Parse("""
            <si xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
              <r><rPr><b/></rPr><t>ALPHA_x000D_BRAVO</t></r>
              <r><rPr><i/></rPr><t>CHARLIE</t></r>
            </si>
            """);

        XlsxSharedStrings.ReadRichString(item).ShouldBe("ALPHABRAVOCHARLIE");

        IReadOnlyList<XlsxRichRun> runs = XlsxRichRuns.Read(item).ShouldNotBeNull();
        runs.Count.ShouldBe(2);
        runs[0].Start.ShouldBe(0);
        runs[0].Length.ShouldBe(10);
        // 10, not 17. This is the assertion that fails if only one of the two sides is decoded.
        runs[1].Start.ShouldBe(10);
        runs[1].Length.ShouldBe(7);
    }

    [Fact]
    public void APhoneticGuideIsStillDroppedAndTheOffsetsStillAgree()
    {
        // The pre-existing rule and the new one have to hold at once: rPh is skipped, and what
        // survives is decoded.
        XElement item = Parse("""
            <si xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
              <t>ONE_x000D_TWO</t>
              <rPh sb="0" eb="3"><t>reading</t></rPh>
            </si>
            """);

        XlsxSharedStrings.ReadRichString(item).ShouldBe("ONETWO");
    }

    [Fact]
    public void ASharedStringAndAnInlineStringBothDecodeEndToEnd()
    {
        // End to end through the reader, because the unit above proves the rule and this proves
        // it is actually wired to both of the two ways a cell states text.
        using IDocument document = Open(Package("""
            <sheetData>
              <row r="1">
                <c r="A1" t="s"><v>0</v></c>
                <c r="B1" t="inlineStr"><is><t>INDIA_x000D_JULIET</t></is></c>
                <c r="C1" t="s"><v>1</v></c>
              </row>
            </sheetData>
            """));

        ContentTable table = document.Content.Children.OfType<ContentSection>()
                                     .Single(s => s.Kind == SectionKind.Sheet)
                                     .Children.OfType<ContentTable>().Single();
        ContentTableCell[] cells = table.Children.Cast<ContentTableRow>().Single()
                                        .Children.Cast<ContentTableCell>().ToArray();

        cells[0].Value.ShouldBe("MIAMI FLORIDA");
        cells[1].Value.ShouldBe("INDIAJULIET");
        // The CR/newline pairing the real workbooks are full of: one break, not seven glyphs
        // and a break.
        cells[2].Value.ShouldBe("Summary:\nA320 REPORTED");
    }

    [Fact]
    public void WordProcessingAndDrawingMlTextIsLeftAlone()
    {
        // Not a test of this class — a test of where it is NOT called, which is the thing a
        // future reader is most likely to get wrong. `w:t` and `a:t` are ST_String, and
        // LibreOffice draws all seven characters of `_x000D_` in both: measured with an authored
        // .docx and .pptx, whose PDFs come back reading "ALPHA_x000D_BRAVO" literally.
        //
        // The corpus makes this concrete: 78 of its documents carry `_x0000_` inside a VML
        // o:spid, and decoding those would rewrite shape identifiers into NULs.
        string[] callers = Directory.GetFiles(SourceDirectory(), "*.cs")
                                    .Where(f => File.ReadAllText(f).Contains(
                                        "XlsxCellText.Of", StringComparison.Ordinal))
                                    .Select(Path.GetFileName)
                                    .OrderBy(f => f, StringComparer.Ordinal)
                                    .ToArray()!;

        callers.ShouldBe(["XlsxNoteCaptions.cs", "XlsxRichRuns.cs", "XlsxSharedStrings.cs"]);
    }

    private static string SourceDirectory()
    {
        // Walk up to the repository's dotnet/ directory rather than hard-coding a depth, so the
        // test survives a change of output path.
        DirectoryInfo? at = new(AppContext.BaseDirectory);
        while (at is not null && !Directory.Exists(Path.Combine(at.FullName, "src", "XRay.Office.Spreadsheets")))
            at = at.Parent;

        at.ShouldNotBeNull("could not find the dotnet/ directory above " + AppContext.BaseDirectory);
        return Path.Combine(at.FullName, "src", "XRay.Office.Spreadsheets", "Ooxml");
    }

    private static XElement Parse(string xml)
        => OoxmlXml.TryLoad(new MemoryStream(Encoding.UTF8.GetBytes(xml)), out _)
           ?? throw new InvalidOperationException("the fixture did not parse");

    private static IDocument Open(byte[] package)
        => new SpreadsheetReader().Read(DocumentSource.FromBytes(package, "escapes.xlsx"));

    /// <summary>A minimal workbook whose shared string table holds the escapes under test.</summary>
    private static byte[] Package(string sheet)
    {
        const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
        const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", """
                <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
                  <Default Extension="rels"
                           ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
                  <Override PartName="/xl/workbook.xml"
                            ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
                  <Override PartName="/xl/worksheets/sheet1.xml"
                            ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
                </Types>
                """);

            Write(archive, "_rels/.rels", $"""
                <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
                  <Relationship Id="rId1" Type="{Rns}/officeDocument" Target="xl/workbook.xml"/>
                </Relationships>
                """);

            Write(archive, "xl/_rels/workbook.xml.rels", $"""
                <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
                  <Relationship Id="rId1" Type="{Rns}/worksheet" Target="worksheets/sheet1.xml"/>
                  <Relationship Id="rId2" Type="{Rns}/sharedStrings" Target="sharedStrings.xml"/>
                </Relationships>
                """);

            Write(archive, "xl/workbook.xml", $"""
                <workbook xmlns="{Ns}" xmlns:r="{Rns}">
                  <sheets><sheet name="Only" sheetId="1" r:id="rId1"/></sheets>
                </workbook>
                """);

            // The second entry is the shape the real files take: an escaped carriage return
            // immediately followed by a real newline.
            Write(archive, "xl/sharedStrings.xml", $"""
                <sst xmlns="{Ns}" count="2" uniqueCount="2">
                  <si><t xml:space="preserve">MIAMI_x000D_ FLORIDA</t></si>
                  <si><t xml:space="preserve">Summary:_x000D_
                A320 REPORTED</t></si>
                </sst>
                """);

            Write(archive, "xl/worksheets/sheet1.xml", $"""<worksheet xmlns="{Ns}">{sheet}</worksheet>""");
        }

        return buffer.ToArray();
    }

    private static void Write(ZipArchive archive, string name, string content)
    {
        using StreamWriter writer = new(archive.CreateEntry(name).Open(), new UTF8Encoding(false));
        writer.Write("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
        writer.Write(content);
    }
}
