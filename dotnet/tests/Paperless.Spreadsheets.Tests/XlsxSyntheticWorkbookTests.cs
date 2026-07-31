using System.IO.Compression;
using System.Text;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the XLSX reader against workbooks built byte by byte.
/// </summary>
/// <remarks>
/// LibreOffice writes only one dialect of SpreadsheetML — always shared strings, never inline
/// ones, never a shared formula, never the strict namespaces, always an <c>r</c> on every cell —
/// so a corpus of files it wrote cannot reach most of what the format permits and Excel uses
/// constantly. These workbooks are assembled directly to cover the rest.
/// </remarks>
public class XlsxSyntheticWorkbookTests
{
    private static IDocument Open(byte[] package)
        => new SpreadsheetReader().Read(DocumentSource.FromBytes(package, "synthetic.xlsx"));

    private static ContentTable TableOf(IDocument document, int sheetIndex = 0)
        => document.Content.Children.OfType<ContentSection>()
                   .Where(s => s.Kind == SectionKind.Sheet)
                   .ElementAt(sheetIndex)
                   .Children.OfType<ContentTable>()
                   .Single();

    private static ContentTableCell Cell(IDocument document, int row, int column, int sheetIndex = 0)
        => TableOf(document, sheetIndex).Children.Cast<ContentTableRow>()
                                        .Single(r => r.Index == row)
                                        .Children.Cast<ContentTableCell>()
                                        .Single(c => c.Column == column);

    [Fact]
    public void EachCellTypeIsReadAsItsOwnThing()
    {
        using IDocument document = Open(Workbook.Build(sheet: """
            <sheetData>
              <row r="1">
                <c r="A1" t="s"><v>1</v></c>
                <c r="B1" t="inlineStr"><is><t>written </t></is><is/></c>
                <c r="C1" t="str"><f>UPPER("x")</f><v>X</v></c>
                <c r="D1" t="b"><v>1</v></c>
                <c r="E1" t="e"><v>#NAME?</v></c>
                <c r="F1"><v>1.5</v></c>
              </row>
            </sheetData>
            """));

        // Six values, six different meanings. Treating any of them as the numeric default
        // reads a string as zero.
        Cell(document, 0, 0).Value.ShouldBe("second");
        Cell(document, 0, 1).Value.ShouldBe("written ");
        Cell(document, 0, 2).Value.ShouldBe("X");
        Cell(document, 0, 3).Value.ShouldBe(true);
        Cell(document, 0, 4).Value.ShouldBe(CellError.Name);
        Cell(document, 0, 5).Value.ShouldBe(1.5);
    }

    [Fact]
    public void AnInlineStringKeepsItsRichTextRunsButNotItsPhoneticGuides()
    {
        using IDocument document = Open(Workbook.Build(sheet: """
            <sheetData>
              <row r="1">
                <c r="A1" t="inlineStr">
                  <is><r><t>bold</t></r><r><t xml:space="preserve"> and plain</t></r>
                      <rPh sb="0" eb="4"><t>ボールド</t></rPh></is>
                </c>
              </row>
            </sheetData>
            """));

        // A string split into runs by formatting is one string to a reader. The phonetic guide
        // is furigana shown above the text, not part of it — concatenating it would splice a
        // reading into the middle of the word.
        Cell(document, 0, 0).Value.ShouldBe("bold and plain");
    }

    [Fact]
    public void ACellWithoutAnAddressFollowsTheOneBeforeIt()
    {
        using IDocument document = Open(Workbook.Build(sheet: """
            <sheetData>
              <row><c t="s"><v>0</v></c><c><v>2</v></c></row>
              <row><c><v>3</v></c></row>
            </sheetData>
            """));

        // Both r attributes are optional. Without them a cell simply follows the previous one
        // and a row the previous row, which is how LibreOffice's importer treats it too
        // (sc/source/filter/oox/sheetdatacontext.cxx:347).
        Cell(document, 0, 0).Value.ShouldBe("first");
        Cell(document, 0, 1).Value.ShouldBe(2.0);
        Cell(document, 1, 0).Value.ShouldBe(3.0);
    }

    [Fact]
    public void ASharedFormulaIsReconstructedForEveryCellInItsGroup()
    {
        using IDocument document = Open(Workbook.Build(sheet: """
            <sheetData>
              <row r="1"><c r="C1"><f t="shared" ref="C1:C3" si="0">$A$1+A1*B1</f><v>1</v></c></row>
              <row r="2"><c r="C2"><f t="shared" si="0"/><v>2</v></c></row>
              <row r="3"><c r="C3"><f t="shared" si="0"/><v>3</v></c></row>
            </sheetData>
            """));

        // Excel writes a filled-down column as one shared group: the master carries the text
        // and every other cell carries nothing but the group id. Leaving those empty would
        // drop the formula from the overwhelming majority of cells in a real workbook.
        Cell(document, 0, 2).Formula.ShouldBe("$A$1+A1*B1");
        Cell(document, 1, 2).Formula.ShouldBe("$A$1+A2*B2");
        Cell(document, 2, 2).Formula.ShouldBe("$A$1+A3*B3");
    }

    [Fact]
    public void SharedFormulaRewritingLeavesNamesAndStringsAlone()
    {
        using IDocument document = Open(Workbook.Build(sheet: """
            <sheetData>
              <row r="1"><c r="B1"><f t="shared" ref="B1:B2" si="0">LOG10(A1)&amp;"see A1"&amp;Tax_2020</f><v>1</v></c></row>
              <row r="2"><c r="B2"><f t="shared" si="0"/><v>2</v></c></row>
            </sheetData>
            """));

        // LOG10 and Tax_2020 both look like a column followed by a row, and the A1 inside the
        // string literal is text. Only the real reference moves.
        Cell(document, 1, 1).Formula.ShouldBe("LOG10(A2)&\"see A1\"&Tax_2020");
    }

    [Fact]
    public void BuiltInNumberFormatIdsAreHonouredWithoutBeingDeclared()
    {
        using IDocument document = Open(Workbook.Build(
            styles: """
                <cellXfs count="3">
                  <xf numFmtId="0"/><xf numFmtId="14"/><xf numFmtId="9"/>
                </cellXfs>
                """,
            sheet: """
                <sheetData>
                  <row r="1">
                    <c r="A1" s="0"><v>46233</v></c>
                    <c r="B1" s="1"><v>46233</v></c>
                    <c r="C1" s="2"><v>0.25</v></c>
                  </row>
                </sheetData>
                """));

        // Ids 0-49 are implicit: a file may use 14 without declaring a numFmt for it, and a
        // reader that only honours the declared ones shows every date as a serial number.
        Cell(document, 0, 0).GetText().ShouldBe("46233");
        Cell(document, 0, 1).GetText().ShouldBe("7/30/2026");
        Cell(document, 0, 1).Value.ShouldBe(new DateTime(2026, 7, 30));
        Cell(document, 0, 2).GetText().ShouldBe("25%");
    }

    [Fact]
    public void The1904EpochShiftsEveryDateInTheWorkbook()
    {
        using IDocument document = Open(Workbook.Build(
            workbookProperties: "<workbookPr date1904=\"1\"/>",
            styles: "<cellXfs count=\"1\"><xf numFmtId=\"14\"/></cellXfs>",
            sheet: """
                <sheetData><row r="1"><c r="A1" s="0"><v>44771</v></c></row></sheetData>
                """));

        // 44771 is 2026-07-30 in the 1904 system and 2022-07-30 in the 1900 one: reading the
        // switch wrong shifts every date in the file by 1462 days, which looks like data
        // corruption rather than a date-system mistake.
        Cell(document, 0, 0).Value.ShouldBe(new DateTime(2026, 7, 30));
    }

    [Fact]
    public void StrictNamespacesReadIdenticallyToTransitionalOnes()
    {
        using IDocument document = Open(Workbook.Build(strict: true, sheet: """
            <sheetData><row r="1"><c r="A1" t="s"><v>0</v></c></row></sheetData>
            """));

        // ISO/IEC 29500 strict names the same elements with different namespace URIs, and real
        // packages use both — sometimes mixed. They are rewritten once, at load.
        Cell(document, 0, 0).Value.ShouldBe("first");
    }

    [Fact]
    public void SheetsAreFoundThroughTheirRelationshipNotTheirPartName()
    {
        using IDocument document = Open(Workbook.Build(
            sheetPartName: "xl/pages/second-sheet.xml",
            sheet: "<sheetData><row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c></row></sheetData>"));

        // The conventional xl/worksheets/sheet1.xml is a convention, not a rule. A producer
        // may name the part anything, and only the r:id says which part is which sheet.
        Cell(document, 0, 0).Value.ShouldBe("first");
    }

    [Fact]
    public void AWorkbookWithNoReadableSheetPartIsAWarningNotAFailure()
    {
        using IDocument document = Open(Workbook.Build(omitSheetPart: true));

        document.Diagnostics.ShouldContain(
            d => d.Code == "PL2141" && d.Severity == DiagnosticSeverity.Warning);
        // The sheet is still there, still named, just empty — partial content beats an
        // exception on a file that violates its own specification.
        document.Content.Children.OfType<ContentSection>()
                .ShouldContain(s => s.Kind == SectionKind.Sheet && s.Name == "Only");
    }

    [Fact]
    public void AMalformedSheetPartLosesThatSheetAndNothingElse()
    {
        byte[] package = Workbook.Build(sheet: "<sheetData><row r=\"1\"><c r=\"A1\"><v>1</v>");
        using IDocument document = Open(package);

        document.Diagnostics.ShouldContain(d => d.Code == "PL2140");
        TableOf(document).Children.ShouldBeEmpty();
    }

    [Fact]
    public void APackageWithNoWorkbookPartIsRejected()
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            Workbook.Write(archive, "[Content_Types].xml", """
                <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
                  <Override PartName="/xl/nothing.xml"
                            ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
                </Types>
                """);
            Workbook.Write(archive, "xl/nothing.xml", "not xml at all");
        }

        // Identification gets as far as the content type, so this arrives at the reader as an
        // XLSX and has to fail with something a caller can act on.
        Should.Throw<Exception>(() => Open(buffer.ToArray()))
              .ShouldBeAssignableTo<MalformedDocumentException>();
    }

    [Fact]
    public void ARowGapInsideTheUsedRangeSurvivesAndAHugeOneIsCapped()
    {
        using IDocument document = Open(Workbook.Build(sheet: """
            <sheetData>
              <row r="1"><c r="A1" t="s"><v>0</v></c></row>
              <row r="4"><c r="A4" t="s"><v>1</v></c></row>
              <row r="200000"><c r="A200000" t="s"><v>0</v></c></row>
            </sheetData>
            """));

        ContentTable table = TableOf(document);

        // Rows 2 and 3 are blank lines a reader sees; the two hundred thousand after row 4 are
        // a gap, not a layout, and materialising them would cost more than the file does.
        table.Children.Cast<ContentTableRow>().Select(r => r.Index).Take(4)
             .ShouldBe([0, 1, 2, 3]);
        table.Children.Count.ShouldBe(4 + 4096 + 1);
        document.Diagnostics.ShouldContain(d => d.Code == "PL2145");
    }

    /// <summary>Builds a minimal but valid SpreadsheetML package.</summary>
    private static class Workbook
    {
        private const string Transitional = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
        private const string Strict = "http://purl.oclc.org/ooxml/spreadsheetml/main";
        private const string RelationshipsTransitional =
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
        private const string RelationshipsStrict = "http://purl.oclc.org/ooxml/officeDocument/relationships";

        public static byte[] Build(
            string sheet = "<sheetData/>",
            string styles = "<cellXfs count=\"1\"><xf numFmtId=\"0\"/></cellXfs>",
            string workbookProperties = "",
            string sheetPartName = "xl/worksheets/sheet1.xml",
            bool strict = false,
            bool omitSheetPart = false)
        {
            string ns = strict ? Strict : Transitional;
            string rns = strict ? RelationshipsStrict : RelationshipsTransitional;

            MemoryStream buffer = new();
            using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
            {
                Write(archive, "[Content_Types].xml", $"""
                    <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
                      <Default Extension="rels"
                               ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
                      <Override PartName="/xl/workbook.xml"
                                ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>
                      <Override PartName="/{sheetPartName}"
                                ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>
                    </Types>
                    """);

                Write(archive, "_rels/.rels", """
                    <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
                      <Relationship Id="rId1"
                        Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"
                        Target="xl/workbook.xml"/>
                    </Relationships>
                    """);

                Write(archive, "xl/_rels/workbook.xml.rels", $"""
                    <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
                      <Relationship Id="rId1"
                        Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet"
                        Target="/{sheetPartName}"/>
                      <Relationship Id="rId2"
                        Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings"
                        Target="sharedStrings.xml"/>
                      <Relationship Id="rId3"
                        Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"
                        Target="styles.xml"/>
                    </Relationships>
                    """);

                Write(archive, "xl/workbook.xml", $"""
                    <workbook xmlns="{ns}" xmlns:r="{rns}">
                      {workbookProperties}
                      <sheets><sheet name="Only" sheetId="1" r:id="rId1"/></sheets>
                    </workbook>
                    """);

                Write(archive, "xl/sharedStrings.xml", $"""
                    <sst xmlns="{ns}" count="2" uniqueCount="2">
                      <si><t>first</t></si><si><t>second</t></si>
                    </sst>
                    """);

                Write(archive, "xl/styles.xml", $"""
                    <styleSheet xmlns="{ns}">{styles}</styleSheet>
                    """);

                if (!omitSheetPart)
                {
                    Write(archive, sheetPartName, $"""
                        <worksheet xmlns="{ns}" xmlns:r="{rns}">{sheet}</worksheet>
                        """);
                }
            }
            return buffer.ToArray();
        }

        public static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            byte[] bytes = Encoding.UTF8.GetBytes(content);
            entry.Write(bytes, 0, bytes.Length);
        }
    }
}
