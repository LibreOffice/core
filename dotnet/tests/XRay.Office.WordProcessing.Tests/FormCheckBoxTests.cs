using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The square a legacy <c>FORMCHECKBOX</c> puts on the line, and the room it takes there.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwFieldFormCheckboxPortion::Format</c> makes the portion a square of
/// <c>rInf.GetTextHeight()</c> with the line's own ascent, and <c>SwTextPaintInfo::DrawCheckBox</c>
/// strokes that rectangle deflated by 25 twips a side, in black with no fill, crossed when ticked.
/// </para>
/// <para>
/// <strong>The standing record said the size "would not pin (9.0…15.9 pt, not following
/// <c>w:checkBox/w:size</c>)" and left 675 fields in 12 documents undrawn.</strong> It pins exactly
/// and <c>w:checkBox/w:size</c> is inert — the range was a range of font sizes. The figures asserted
/// below are 26.2.4.2's own, read out of its PDFs by <c>probes/words-r56/formcheckbox.py</c> with a
/// duplicate-input control that agreed to the digit.
/// </para>
/// </remarks>
public sealed class FormCheckBoxTests
{
    /// <summary>The reference's own square, at the sizes it was measured at.</summary>
    /// <remarks>
    /// The page's left margin is 1440 twips, so the reference draws the box's left edge at
    /// 72 + 1.25 = 73.25 pt at every size — the inset, and nothing else, separates the two.
    /// </remarks>
    [Theory]
    [InlineData(16, 6.70)]
    [InlineData(24, 11.30)]
    [InlineData(48, 25.10)]
    [InlineData(80, 43.50)]
    public void TheDrawnSquareIsTheReferencesOwn(int halfPoints, double side)
    {
        DocRect box = Stroked(Package(halfPoints: halfPoints)).ShouldHaveSingleItem();

        box.Width.Points.ShouldBe(side, 0.02);
        box.Height.Points.ShouldBe(side, 0.02);
        box.X.Points.ShouldBe(73.25, 0.02);
    }

    /// <summary>
    /// The size follows the <em>face</em> at one stated point size, which is what says it is the line's
    /// text height rather than anything the field or the run declares.
    /// </summary>
    [Theory]
    [InlineData("Liberation Serif", 11.30)]
    [InlineData("Liberation Mono", 11.10)]
    [InlineData("DejaVu Sans", 11.50)]
    [InlineData("Carlito", 12.15)]
    public void TheSquareFollowsTheFaceAtOneSize(string family, double side)
        => Stroked(Package(family: family)).ShouldHaveSingleItem()
            .Width.Points.ShouldBe(side, 0.02);

    /// <summary><c>w:checkBox/w:size</c> is parsed by nothing and changes nothing.</summary>
    /// <remarks>
    /// The premise of the old record, and it is false: four fixtures stating 5, 10, 20 and 40 pt all
    /// draw the run's own 11.30 on 26.2.4.2. 109 of the corpus's 675 boxes state one.
    /// </remarks>
    [Theory]
    [InlineData(10)]
    [InlineData(20)]
    [InlineData(40)]
    [InlineData(80)]
    public void AStatedCheckBoxSizeIsInert(int stated)
        => Stroked(Package(boxSize: stated)).ShouldHaveSingleItem()
            .Width.Points.ShouldBe(11.30, 0.02);

    /// <summary>A ticked box is crossed corner to corner, an unticked one is not.</summary>
    [Fact]
    public void OnlyATickedBoxIsCrossed()
    {
        Lines(Package(checkedState: true)).Count.ShouldBe(2);
        Lines(Package()).ShouldBeEmpty();
    }

    /// <summary><c>w:checked</c> is the state and <c>w:default</c> is only what it reverts to.</summary>
    [Fact]
    public void TheCurrentStateWinsOverTheDefault()
    {
        Lines(Package(checkedState: false, current: true)).Count.ShouldBe(2);
        Lines(Package(checkedState: true, current: false)).ShouldBeEmpty();
    }

    /// <summary>
    /// The box takes room on the line, which is the half of this that moves a line break.
    /// </summary>
    /// <remarks>
    /// Drawing the square and not reserving its width would be no better than what was there before:
    /// 675 of these across 12 corpus documents were taking no room at all, so every line holding one
    /// was laid out narrower than the reference lays it out. The reserved width is the <em>portion's</em>
    /// square — the undeflated one — because the inset is what the page shows and not what the line pays.
    /// </remarks>
    [Fact]
    public void TheBoxReservesItsWholeSquareOnTheLine()
    {
        PageParagraph paragraph = Paragraph(Package());

        InlineObject inline = paragraph.InlineObjects.ShouldHaveSingleItem();
        inline.Width.Points.ShouldBe(13.80, 0.02, "the portion is the text height, not the drawn box");
        inline.Width.ShouldBe(inline.Height);
        inline.Offset.ShouldBe(0);
    }

    private static List<DocRect> Stroked(MemoryStream package)
    {
        Sink sink = Draw(package);
        return sink.Rectangles;
    }

    private static List<(DocPoint From, DocPoint To)> Lines(MemoryStream package)
        => Draw(package).Lines;

    private static Sink Draw(MemoryStream package)
    {
        using DocumentSource source = DocumentSource.FromStream(package, "checkbox.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        Sink sink = new();
        foreach (LaidOutPage page in pages.Pages) PageDrawing.Draw(page, pages.Blocks, sink);
        return sink;
    }

    private static PageParagraph Paragraph(MemoryStream package)
    {
        using DocumentSource source = DocumentSource.FromStream(package, "checkbox.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageParagraph>().First(block => block.Text.Length > 0);
    }

    /// <summary>Collects the stroked geometry and ignores everything else.</summary>
    private sealed class Sink : IDrawingSink
    {
        public List<DocRect> Rectangles { get; } = [];

        public List<(DocPoint From, DocPoint To)> Lines { get; } = [];

        public void StrokePath(GraphicsPath path, Stroke stroke)
        {
            ArgumentNullException.ThrowIfNull(path);

            List<DocPoint> points =
            [
                .. path.Commands
                    .Where(command => command.Verb is PathVerb.MoveTo or PathVerb.LineTo)
                    .Select(command => command.Point),
            ];

            if (points.Count == 4)
            {
                Rectangles.Add(Bounds(points));
                return;
            }

            if (points.Count == 2) Lines.Add((points[0], points[1]));
        }

        private static DocRect Bounds(List<DocPoint> points)
        {
            Length left = points.Min(p => p.X);
            Length top = points.Min(p => p.Y);
            return new DocRect(
                left, top, points.Max(p => p.X) - left, points.Max(p => p.Y) - top);
        }

        public void DrawGlyphRun(GlyphRun run, Paint paint) { }

        public void BeginPage(DocSize size) { }

        public void EndPage() { }

        public void Save() { }

        public void Restore() { }

        public void Transform(AffineTransform transform) { }

        public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) { }

        public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero) { }

        public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0) { }

        public void BeginTransparencyGroup(double opacity) { }

        public void EndTransparencyGroup() { }
    }

    private static MemoryStream Package(
        int halfPoints = 24,
        string family = "Liberation Serif",
        int? boxSize = null,
        bool checkedState = false,
        bool? current = null)
    {
        const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
        const string PkgR = "http://schemas.openxmlformats.org/package/2006/relationships";
        const string R = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

        string box = "<w:checkBox>"
            + (boxSize is { } stated ? $"<w:size w:val=\"{stated}\"/>" : "<w:sizeAuto/>")
            + (current is { } state ? $"<w:checked w:val=\"{(state ? 1 : 0)}\"/>" : string.Empty)
            + $"<w:default w:val=\"{(checkedState ? 1 : 0)}\"/></w:checkBox>";

        string runProperties =
            $"<w:rPr><w:rFonts w:ascii=\"{family}\" w:hAnsi=\"{family}\"/>"
            + $"<w:sz w:val=\"{halfPoints}\"/></w:rPr>";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="{W}"><w:body>
              <w:p>
                <w:r>{runProperties}<w:fldChar w:fldCharType="begin"><w:ffData>
                  <w:name w:val="Check1"/><w:enabled/>{box}
                </w:ffData></w:fldChar></w:r>
                <w:r>{runProperties}<w:instrText xml:space="preserve"> FORMCHECKBOX </w:instrText></w:r>
                <w:r>{runProperties}<w:fldChar w:fldCharType="end"/></w:r>
                <w:r>{runProperties}<w:t xml:space="preserve">Hx</w:t></w:r>
              </w:p>
              <w:sectPr><w:pgSz w:w="11906" w:h="16838"/>
                <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/></w:sectPr>
            </w:body></w:document>
            """;

        string styles = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="{W}"><w:docDefaults><w:rPrDefault><w:rPr>
              <w:rFonts w:ascii="{family}" w:hAnsi="{family}"/><w:sz w:val="{halfPoints}"/>
            </w:rPr></w:rPrDefault><w:pPrDefault/></w:docDefaults></w:styles>
            """;

        string types = """
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

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", types);
            Write(archive, "_rels/.rels", $"""
                <?xml version="1.0" encoding="UTF-8"?>
                <Relationships xmlns="{PkgR}">
                  <Relationship Id="rId1" Target="word/document.xml" Type="{R}/officeDocument"/>
                </Relationships>
                """);
            Write(archive, "word/_rels/document.xml.rels", $"""
                <?xml version="1.0" encoding="UTF-8"?>
                <Relationships xmlns="{PkgR}">
                  <Relationship Id="rId8" Target="styles.xml" Type="{R}/styles"/>
                </Relationships>
                """);
            Write(archive, "word/document.xml", document);
            Write(archive, "word/styles.xml", styles);
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
