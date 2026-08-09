using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A cell's text may not sit under the half of its border that lies inside the cell.
/// </summary>
/// <remarks>
/// <para>
/// This is the *corrected* form of a rule that was proposed, measured whole and rejected: that Writer
/// takes a cell's border line width off its text width, which would have narrowed every bordered cell
/// in the corpus. It does not. <c>SwCellFrame::Format</c> branches on <c>IsCollapsingBorders()</c>
/// (<c>sw/source/core/layout/tabfrm.cxx</c>:6105–6120) and a Word table always takes the collapsing
/// side, where the inset is <c>rBoxItem.GetDistance()</c> alone — the margin, never the border.
/// </para>
/// <para>
/// What is really there is a floor rather than a charge. Word's border straddles the cell edge, so
/// half of it lies inside the cell, and Word will not run text under that half however small the
/// declared margin is. writerfilter reproduces it at import, in <c>lcl_adjustBorderDistance</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:318–348):
/// </para>
/// <code>
/// pad_l = max(bll/2, cml)
/// pad_r = max(pad_l + blr/2, cml + cmr) - pad_l
/// </code>
/// <para>
/// The two are easy to confuse and produce very different documents, so the observation that separates
/// them is asserted here as well as the rule itself: a cell whose margin already clears half its border
/// — which is nearly every cell, Word's default margin being 108 twips against 5 for half a hairline —
/// keeps exactly the margin it declared, and a *wider* border does not narrow it further.
/// </para>
/// <para>
/// The figures are LibreOffice 24.2.7.2's, on the fixtures <c>dotnet/probes/cell-border-inset.py</c>
/// authors: at a 6 pt border the text inset is 3 pt at declared margins of 0, 1, 2 and 3 pt and then
/// follows the margin at 4, 5 and 6; at a 0.5 pt border it is 0.25 pt at a margin of 0 and the margin
/// everywhere above. The same probe shows an ODF *separating*-border table losing two whole border
/// widths of text area, which is the branch this rule is not.
/// </para>
/// <para>
/// It is an OOXML import adjustment, so it belongs to this reader alone.
/// <c>WW8TabDesc::SetTabBorders</c> (<c>sw/source/filter/ww8/ww8par2.cxx</c>:3020–3042) sets a
/// <c>.doc</c> cell's distance straight from <c>sprmTCellPadding</c> or the band's half-gap with no
/// floor at all, and 66 of the words corpus's 200 documents are <c>.doc</c>.
/// </para>
/// </remarks>
public sealed class CellBorderInsetTests
{
    /// <summary>A margin below half the border is raised to half the border.</summary>
    /// <remarks>
    /// Both sides, because the formula is asymmetric — the right margin is what is left of the wider of
    /// "clear of the right border" and "both declared margins" once the left one is taken — and the two
    /// coincide only when the declared margins are equal, which is the case here and is not the case in
    /// <see cref="TheRightMarginPaysForTheLeftMarginsFloor"/>.
    /// </remarks>
    [Theory]
    [InlineData(48, 0, 60, 60)]      // 6 pt border, no margin at all: half the border on each side
    [InlineData(48, 40, 60, 60)]     // 6 pt border, 2 pt margin: still the border's half
    [InlineData(48, 60, 60, 60)]     // 6 pt border, 3 pt margin: exactly at the floor
    [InlineData(16, 0, 20, 20)]      // 2 pt border, no margin
    [InlineData(4, 0, 5, 5)]         // half-point border, no margin: 5 twips, and not zero
    public void AMarginBelowHalfTheBorderIsRaisedToIt(
        int borderEighths, int marginTwips, int expectedLeft, int expectedRight)
    {
        PageTableCell cell = Cell(borderEighths, marginTwips, marginTwips);

        cell.Padding.Left.ShouldBe(Length.FromTwips(expectedLeft));
        cell.Padding.Right.ShouldBe(Length.FromTwips(expectedRight));
    }

    /// <summary>
    /// A margin that already clears half the border is left exactly as declared, however thick the
    /// border grows.
    /// </summary>
    /// <remarks>
    /// This is the half that separates the floor from the rejected charge: under "the border comes off
    /// the text width" these three would be 108+120, 108+240 and 108+480 twips of inset rather than 108
    /// each time. LibreOffice's own rendering of the same three gives one text-area width, 442.84 pt.
    /// </remarks>
    [Theory]
    [InlineData(4)]
    [InlineData(16)]
    [InlineData(48)]
    public void AMarginThatAlreadyClearsTheBorderIsUntouched(int borderEighths)
    {
        PageTableCell cell = Cell(borderEighths, 108, 108);

        cell.Padding.Left.ShouldBe(Length.FromTwips(108));
        cell.Padding.Right.ShouldBe(Length.FromTwips(108));
    }

    /// <summary>
    /// The right margin pays for the left one's floor: it is the total the two declared, less whatever
    /// the left side actually took.
    /// </summary>
    /// <remarks>
    /// writerfilter's comment states this outright — "for border widths of 6 pt, left margin 0 mm and
    /// right margin 2 mm, actual left and right margins will (unexpectedly) coincide with inner edges
    /// of cell's borderlines". With a 6 pt border, no left margin and 120 twips of right margin the
    /// left takes 60 and the right is <c>max(60 + 60, 0 + 120) − 60 = 60</c>, so the declared 120
    /// buys no gap on the right at all.
    /// </remarks>
    [Fact]
    public void TheRightMarginPaysForTheLeftMarginsFloor()
    {
        PageTableCell cell = Cell(borderEighths: 48, leftTwips: 0, rightTwips: 120);

        cell.Padding.Left.ShouldBe(Length.FromTwips(60));
        cell.Padding.Right.ShouldBe(Length.FromTwips(60));
    }

    /// <summary>An unbordered cell keeps whatever margin it declared, including none.</summary>
    [Fact]
    public void AnUnborderedCellIsNotInsetAtAll()
    {
        PageTableCell cell = Cell(borderEighths: 0, leftTwips: 0, rightTwips: 0);

        cell.Padding.Left.ShouldBe(Length.Zero);
        cell.Padding.Right.ShouldBe(Length.Zero);
    }

    private static PageTableCell Cell(int borderEighths, int leftTwips, int rightTwips)
    {
        using IDocument document = Open(borderEighths, leftTwips, rightTwips);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Blocks.OfType<PageTable>().Single().Rows[0].Cells[0];
    }

    private static IDocument Open(int borderEighths, int leftTwips, int rightTwips)
    {
        MemoryStream package = BuildPackage(borderEighths, leftTwips, rightTwips);
        using DocumentSource source = DocumentSource.FromStream(package, "cell-border-inset.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(int borderEighths, int leftTwips, int rightTwips)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="settings.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
            </Relationships>
            """;

        // A hand-built DOCX with no settings part does not get LibreOffice's OOXML compatibility
        // defaults, and a fixture minimal enough to be obviously correct is often minimal enough to
        // answer a different question. Measured both ways for this rule and the numbers do not move,
        // but the part is cheap and the trap is not.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        string borders = borderEighths == 0
            ? """
              <w:tblBorders>
                <w:top w:val="none" w:sz="0" w:space="0" w:color="auto"/>
                <w:left w:val="none" w:sz="0" w:space="0" w:color="auto"/>
                <w:bottom w:val="none" w:sz="0" w:space="0" w:color="auto"/>
                <w:right w:val="none" w:sz="0" w:space="0" w:color="auto"/>
              </w:tblBorders>
              """
            : $"""
              <w:tblBorders>
                <w:top w:val="single" w:sz="{borderEighths}" w:space="0" w:color="000000"/>
                <w:left w:val="single" w:sz="{borderEighths}" w:space="0" w:color="000000"/>
                <w:bottom w:val="single" w:sz="{borderEighths}" w:space="0" w:color="000000"/>
                <w:right w:val="single" w:sz="{borderEighths}" w:space="0" w:color="000000"/>
              </w:tblBorders>
              """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblPr>
                    <w:tblW w:w="5000" w:type="dxa"/>
                    <w:tblLayout w:type="fixed"/>
                    {borders}
                    <w:tblCellMar>
                      <w:top w:w="0" w:type="dxa"/>
                      <w:left w:w="{leftTwips}" w:type="dxa"/>
                      <w:bottom w:w="0" w:type="dxa"/>
                      <w:right w:w="{rightTwips}" w:type="dxa"/>
                    </w:tblCellMar>
                  </w:tblPr>
                  <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
                  <w:tr>
                    <w:tc>
                      <w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>cell</w:t></w:r></w:p>
                    </w:tc>
                  </w:tr>
                </w:tbl>
                <w:p><w:r><w:t>after</w:t></w:r></w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", Settings);
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
