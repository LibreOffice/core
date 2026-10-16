using XRay.Office.Core.Geometry;
using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Text that states no colour, on a background that decides what colour it is.
/// </summary>
/// <remarks>
/// <para>
/// OOXML's <c>w:color w:val="auto"</c> and an absent <c>w:color</c> are the same state —
/// LibreOffice's <c>COL_AUTO</c> — and it is not "black". <c>SwDrawTextInfo::ApplyAutoColor</c>
/// (<c>sw/source/core/txtnode/fntcache.cxx</c>:2369) resolves it against whatever brush the frame
/// chain supplies and answers white when the brush is dark. Which is why
/// <c>AFS-050-004-F2_0i.docx</c> page 2 had 305 glyphs in our text layer, at the reference's own
/// positions, painted black on a black banner.
/// </para>
/// <para>
/// Three things this asserts that the colour rule alone does not:
/// </para>
/// <list type="bullet">
/// <item>a run that <em>states</em> a colour is not automatic and never moves — the control;</item>
/// <item>a character highlight is not a brush, in both directions, measured on 26.2.4.2;</item>
/// <item>a paragraph shade beats the cell it sits in, also in both directions.</item>
/// </list>
/// <para>
/// Reintroducing the bug to check these fail: have <c>PageRun.ColourOn</c> ignore its argument, or
/// have <c>PageDrawing.DrawTable</c> pass <c>default</c> instead of the cell's shading, or put
/// <c>?? Colour.Black</c> back into the reader where <c>?? Colour.Transparent</c> now stands.
/// </para>
/// </remarks>
public sealed class AutomaticFontColourTests
{
    private static readonly Colour Black = Colour.FromRgb(0x000000);
    private static readonly Colour White = Colour.FromRgb(0xFFFFFF);
    private static readonly Colour Red = Colour.FromRgb(0xFF0000);

    /// <summary>Every case at once, so one drawing pass covers all of them.</summary>
    [Theory]
    [InlineData("black cell", 0xFFFFFFu)]
    [InlineData("white cell", 0x000000u)]
    [InlineData("solid pattern cell", 0xFFFFFFu)]
    [InlineData("light paragraph in a black cell", 0x000000u)]
    [InlineData("dark paragraph in a white cell", 0xFFFFFFu)]
    [InlineData("yellow highlight in a black cell", 0xFFFFFFu)]
    [InlineData("dark highlight in a white cell", 0x000000u)]
    [InlineData("no table at all", 0x000000u)]
    [InlineData("dark paragraph, no table", 0xFFFFFFu)]
    public void AnUnstatedColourIsResolvedAgainstWhatIsBehindIt(string text, uint expected)
    {
        Painted(text).ShouldBe(Colour.FromRgb(expected));
    }

    /// <summary>
    /// A run that states its own colour keeps it, however dark the cell.
    /// </summary>
    /// <remarks>
    /// The control the whole file needs: a change that reversed <em>everything</em> out of a dark
    /// background would satisfy every row above and fail this one. Measured on 26.2.4.2, which draws
    /// this run red on black.
    /// </remarks>
    [Fact]
    public void AStatedColourIsNotAutomatic()
    {
        Painted("stated red in a black cell").ShouldBe(Red);
    }

    /// <summary>
    /// A <c>w:shd</c> whose <c>w:val</c> is a pattern is painted, and its blend is the reference's.
    /// </summary>
    /// <remarks>
    /// <c>&lt;w:shd w:val="solid" w:color="auto" w:fill="auto"/&gt;</c> is a black cell:
    /// <c>CellColorHandler</c> gives <c>solid</c> a weight of a thousand out of a thousand and
    /// <c>w:color="auto"</c> is black, where <c>w:fill="auto"</c> is white. Reading the fill alone —
    /// which is what stood — painted nothing at all, and is three of the eight rectangles
    /// <c>AFS-050-004-F2_0i</c> page 2 draws.
    /// </remarks>
    [Fact]
    public void APatternedShadingIsBlendedAndPainted()
    {
        List<(DocRect Bounds, Paint Paint)> fills = Fills();

        fills.Select(fill => ((SolidPaint)fill.Paint).Colour)
            .ShouldContain(Black, "the solid-pattern cell paints nothing at all");

        // pct50 of black over white: (0 x 500 + 255 x 500) / 1000, truncated, per channel.
        fills.Select(fill => ((SolidPaint)fill.Paint).Colour)
            .ShouldContain(Colour.FromRgb(0x7F7F7F));
    }

    private static Colour Painted(string text)
    {
        RecordingDrawingSink sink = new();
        using IPaginatedDocument document = Document();
        WordProcessingPages pages = (WordProcessingPages)document.Layout();
        foreach (LaidOutPage page in pages.Pages) PageDrawing.Draw(page, pages.Blocks, sink);

        List<DrawnGlyphRun> drawn =
        [
            .. sink.Pages.SelectMany(page => page.Runs).Where(run => run.Run.Text == text),
        ];

        drawn.Count.ShouldBe(1, $"'{text}' was drawn {drawn.Count} times");
        return ((SolidPaint)drawn[0].Paint).Colour;
    }

    private static List<(DocRect Bounds, Paint Paint)> Fills()
    {
        PlacedDrawingSink sink = new();
        using IPaginatedDocument document = Document();
        WordProcessingPages pages = (WordProcessingPages)document.Layout();
        foreach (LaidOutPage page in pages.Pages) PageDrawing.Draw(page, pages.Blocks, sink);
        return sink.Fills;
    }

    private static IPaginatedDocument Document()
    {
        MemoryStream package = BuildPackage();
        DocumentSource source = DocumentSource.FromStream(package, "auto-colour.docx");
        return (IPaginatedDocument)new WordProcessingReader().Read(source);
    }

    /// <summary>One DOCX carrying every background the rule distinguishes.</summary>
    private static MemoryStream BuildPackage()
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
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
              <Relationship Id="rId1" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
            </Relationships>
            """;

        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault><w:rPr>
                  <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
                  <w:sz w:val="24"/>
                </w:rPr></w:rPrDefault>
              </w:docDefaults>
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
            </w:styles>
            """;

        string document =
            $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {Cell("black cell", Shade("clear", "auto", "000000"))}
                {Cell("white cell", Shade("clear", "auto", "FFFFFF"))}
                {Cell("solid pattern cell", Shade("solid", "auto", "auto"))}
                {Cell("half pattern cell", Shade("pct50", "auto", "auto"))}
                {Cell("light paragraph in a black cell", Shade("clear", "auto", "000000"),
                      paragraph: Shade("clear", "auto", "FFFFFF"))}
                {Cell("dark paragraph in a white cell", Shade("clear", "auto", "FFFFFF"),
                      paragraph: Shade("clear", "auto", "000000"))}
                {Cell("yellow highlight in a black cell", Shade("clear", "auto", "000000"),
                      run: "<w:highlight w:val=\"yellow\"/>")}
                {Cell("dark highlight in a white cell", Shade("clear", "auto", "FFFFFF"),
                      run: "<w:highlight w:val=\"darkBlue\"/>")}
                {Cell("stated red in a black cell", Shade("clear", "auto", "000000"),
                      run: "<w:color w:val=\"FF0000\"/>")}
                <w:p><w:r><w:t>no table at all</w:t></w:r></w:p>
                <w:p><w:pPr>{Shade("clear", "auto", "000000")}</w:pPr>
                  <w:r><w:t>dark paragraph, no table</w:t></w:r></w:p>
                <w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/styles.xml", Styles);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static string Shade(string val, string colour, string fill)
            => $"<w:shd w:val=\"{val}\" w:color=\"{colour}\" w:fill=\"{fill}\"/>";

        static string Cell(string text, string shade, string paragraph = "", string run = "")
            => $"""
               <w:tbl><w:tblPr><w:tblW w:w="9000" w:type="dxa"/></w:tblPr>
                 <w:tblGrid><w:gridCol w:w="9000"/></w:tblGrid>
                 <w:tr><w:tc><w:tcPr><w:tcW w:w="9000" w:type="dxa"/>{shade}</w:tcPr>
                   <w:p><w:pPr>{paragraph}</w:pPr>
                     <w:r><w:rPr>{run}</w:rPr><w:t>{text}</w:t></w:r></w:p>
                 </w:tc></w:tr></w:tbl>
               """;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
