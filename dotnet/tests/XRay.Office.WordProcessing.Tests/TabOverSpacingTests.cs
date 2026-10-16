using System.Globalization;
using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A right stop is clamped at the text frame's right edge, not at the line's — so a paragraph's own
/// right indent does not pull it in, and the tab may reach past that indent without breaking the line.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwTabPortion::PostFormat</c> (<c>sw/source/core/text/txttab.cxx</c>:503) clamps at
/// <c>rInf.GetTextFrame()-&gt;getFrameArea().Right()</c> under <c>TabOverSpacing</c>, which
/// <c>WriterFilter.cxx</c>:325 sets for every writerfilter document, and only at <c>rInf.Width()</c> —
/// the line's own width, indents taken out — for a document carrying neither compatibility flag.
/// Clamping at the line's edge instead puts a contents entry's page number short by the paragraph's
/// right indent: measured against the reference on the corpus, 18.09 and 18.10 pt on the two
/// <c>mcar</c> revisions (<c>toc 4</c>, <c>w:right="360"</c>) and 28.45 pt on <c>EHEST-SMS</c>
/// (<c>toc 2</c>, <c>w:right="1134"</c>, stop 566 twips inside the frame).
/// </para>
/// <para>
/// The fixture is that shape at its smallest: US Letter with 1440-twip margins, so the frame is 9360
/// twips wide, a right dotted stop declared at the frame's own edge, and a 994-twip right indent
/// between the two. The entry's number therefore belongs 994 twips past the line's right edge and 0
/// past the frame's. Its title is longer than the 720-twip hanging indent on purpose: a tab still
/// inside that indent advances to the indent itself whatever stops the paragraph declares, which is a
/// different rule and would measure nothing about this one.
/// </para>
/// </remarks>
public sealed class TabOverSpacingTests
{
    /// <summary>The frame's right edge on the fixture: the left margin plus the text width.</summary>
    private static readonly Length FrameRight = Length.FromTwips(1440 + 9360);

    /// <summary>
    /// The number at a stop declared inside the paragraph's right indent is drawn at the stop.
    /// </summary>
    /// <remarks>
    /// Asserted on the drawn run's right edge rather than on the tab's width, because that edge is what
    /// the reference's own PDF can be measured at and is what the corpus figures above are.
    /// </remarks>
    [Fact]
    public void AStopInsideTheParagraphsRightIndentIsHonouredWhereItWasDeclared()
    {
        DrawnGlyphRun number = NumberOn(stop: 9360, rightIndent: 994);

        (number.Origin.X + number.Width).Points.ShouldBe(FrameRight.Points, 0.05);
    }

    /// <summary>
    /// The entry stays on one line, though its number sits past the line's own right edge.
    /// </summary>
    /// <remarks>
    /// The other half of the rule, and the half without which the first would be a regression. Writer
    /// fits the text after a right stop while the tab is still one twip wide — <c>PreFormat</c> only
    /// records the stop as pending — and settles the tab's width afterwards in <c>PostFormat</c>. A
    /// filler that counted the settled width against the line's own would break here, and a contents
    /// entry would come out as four lines: its number, its title, its leader dots and its page.
    /// </remarks>
    [Fact]
    public void AnEntryWhoseNumberSitsInsideTheRightIndentStaysOnOneLine()
    {
        Baselines(stop: 9360, rightIndent: 994).Count.ShouldBe(1);
    }

    /// <summary>A stop declared past the frame's right edge is still pulled back to it.</summary>
    /// <remarks>
    /// The clamp is loosened by this rule, not removed. Writer's own bound is an absolute page
    /// coordinate compared against a line-relative stop, so it lets such a stop run on into the page's
    /// right margin; the bound here is the frame's edge, which stops it at the margin instead. That
    /// remaining band is bounded by how far past the frame the stop was declared — a great deal less
    /// than the indent this replaced.
    /// </remarks>
    [Fact]
    public void AStopPastTheFramesRightEdgeIsPulledBackToIt()
    {
        DrawnGlyphRun number = NumberOn(stop: 11000, rightIndent: 994);

        (number.Origin.X + number.Width).Points.ShouldBe(FrameRight.Points, 0.05);
    }

    /// <summary>
    /// A paragraph with no right indent is unaffected, which is the control on the other three.
    /// </summary>
    [Fact]
    public void AParagraphWithNoRightIndentPutsItsNumberInTheSamePlace()
    {
        DrawnGlyphRun number = NumberOn(stop: 9360, rightIndent: 0);

        (number.Origin.X + number.Width).Points.ShouldBe(FrameRight.Points, 0.05);
    }

    /// <summary>The run holding the entry's page number: the last one drawn on the line.</summary>
    private static DrawnGlyphRun NumberOn(int stop, int rightIndent)
    {
        DrawnPage page = Drawn(stop, rightIndent);

        return page.Runs.Single(run => run.Text == "7");
    }

    /// <summary>The distinct baselines the entry was drawn on, which is how many lines it took.</summary>
    private static HashSet<long> Baselines(int stop, int rightIndent)
        => [.. Drawn(stop, rightIndent).Runs.Select(run => run.Origin.Y.Emu)];

    private static DrawnPage Drawn(int stop, int rightIndent)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromStream(BuildPackage(stop, rightIndent), "contents.docx"))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages.Count.ShouldBe(1);
            pages[0].Draw(sink);
        }

        return sink.Pages.Single();
    }

    private static MemoryStream BuildPackage(int stop, int rightIndent)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        string document = string.Format(
            CultureInfo.InvariantCulture,
            """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr>
                    <w:tabs><w:tab w:val="right" w:leader="dot" w:pos="{0}"/></w:tabs>
                    <w:ind w:left="720" w:right="{1}" w:hanging="720"/>
                  </w:pPr>
                  <w:r><w:t>Chapter 1 - Definitions</w:t></w:r>
                  <w:r><w:tab/><w:t>7</w:t></w:r>
                </w:p>
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1080" w:right="1440" w:bottom="1080" w:left="1440"
                           w:header="432" w:footer="432" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """,
            stop,
            rightIndent);

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
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
