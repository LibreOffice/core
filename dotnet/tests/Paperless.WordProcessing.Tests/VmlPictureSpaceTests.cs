using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A legacy <c>w:pict</c> or an embedded <c>w:object</c> is a picture, and it takes room.
/// </summary>
/// <remarks>
/// <para>
/// Before DrawingML a picture was a VML shape, and Word still writes one for every embedded object and
/// for anything that has ever been through a <c>.doc</c>. It states its size as CSS in the shape's
/// <c>style</c> attribute rather than as a <c>wp:extent</c> in EMUs, which is why a reader written for
/// <c>w:drawing</c> finds nothing to measure and silently makes the picture nought high.
/// </para>
/// <para>
/// Measured on <c>words/batch-010/docx/5709.16 ch.40_mgfinal.docx</c>, whose first table cell holds a
/// <c>Word.Picture.8</c> object 66.6 by 73.8 pt: removing the object from the file moved LibreOffice's
/// own page one up by exactly 60 pt, and with it in place our page one ran 72 pt short of the
/// reference's. LibreOffice reads the same two numbers — <c>oox/source/vml/vmlshape.cxx</c>'s
/// <c>ShapeTypeModel::maWidth</c> and <c>maHeight</c>, decoded by
/// <c>ConversionHelper::decodeMeasureToHmm</c>.
/// </para>
/// <para>
/// The negative case is asserted beside it: a shape stating <c>position:absolute</c> floats, takes no
/// room in the text, and must not push a line down. Across the words track 20 documents carry an inline
/// VML shape and 33 carry only floating ones, so reading a floating shape as inline would be the more
/// common mistake by some margin.
/// </para>
/// </remarks>
public sealed class VmlPictureSpaceTests
{
    /// <summary>
    /// The line grows by exactly the difference between two shapes' stated heights.
    /// </summary>
    /// <remarks>
    /// Stated as a difference rather than as an absolute, because an as-character frame sits on the
    /// baseline and the line is the frame plus the surrounding text's descent — which is right, and is
    /// not what this test is about.
    /// </remarks>
    [Fact]
    public void TheLineGrowsByExactlyTheShapesHeight()
    {
        Length plain = FirstLineHeight(Paragraph(null));
        Length small = FirstLineHeight(Paragraph(Pict("width:40pt;height:50pt")));
        Length large = FirstLineHeight(Paragraph(Pict("width:40pt;height:150pt")));

        small.ShouldBeGreaterThan(plain, "a picture on the line makes the line taller");
        Near(large - small, Length.FromPoints(100));
    }

    /// <summary>
    /// The size is a CSS length, and a bare number is pixels at 96 dpi rather than points.
    /// </summary>
    /// <remarks>
    /// The bare number is the case worth pinning: reading it as points makes a shape written
    /// <c>width:96</c> an inch and a third too small, and VML files written by anything other than Word
    /// use it.
    /// </remarks>
    [Theory]
    [InlineData("1in")]
    [InlineData("96")]
    [InlineData("96px")]
    [InlineData("2.54cm")]
    [InlineData("25.4mm")]
    [InlineData("6pc")]
    public void AShapesSizeIsReadInCssUnits(string height)
    {
        Length stated = FirstLineHeight(Paragraph(Pict($"width:40pt;height:{height}")));
        Length points = FirstLineHeight(Paragraph(Pict("width:40pt;height:72pt")));

        // Both sides collapse to the bare text line when the shape is not read at all, and would then
        // agree for the wrong reason — so the line has to be taller than the text before they are
        // compared. Verified by putting the defect back: without this the six unit cases all pass.
        points.ShouldBeGreaterThan(FirstLineHeight(Paragraph(null)));
        Near(stated, points);
    }

    /// <summary>An embedded object falls back to the size <c>w:dxaOrig</c> states.</summary>
    /// <remarks>
    /// Word writes both, and the twips are what an object whose shape lost its style is left with.
    /// 1485 twips is 74.25 pt, which is the logo on page one of <c>5709.16 ch.40_mgfinal.docx</c>.
    /// </remarks>
    [Fact]
    public void AnObjectWithoutAStyleFallsBackToItsOriginalSize()
    {
        Length original = FirstLineHeight(Paragraph(
            """<w:object w:dxaOrig="1342" w:dyaOrig="1485"><v:shape id="s"/></w:object>"""));
        Length equivalent = FirstLineHeight(Paragraph(Pict("width:67.1pt;height:74.25pt")));

        equivalent.ShouldBeGreaterThan(FirstLineHeight(Paragraph(null)));
        Near(original, equivalent);
    }

    /// <summary>A floating shape takes no room in the line it is anchored in.</summary>
    [Fact]
    public void AFloatingPictDoesNotRaiseTheLine()
    {
        Length plain = FirstLineHeight(Paragraph(null));
        Length floating = FirstLineHeight(Paragraph(
            Pict("position:absolute;left:0;top:0;width:66.6pt;height:200pt")));

        floating.ShouldBe(plain, "a positioned shape is not set in the text");
    }

    /// <summary>Equal to within a twip of rounding.</summary>
    private static void Near(Length measured, Length expected)
        => Math.Abs(measured.Twips - expected.Twips)
            .ShouldBeLessThanOrEqualTo(2L, $"{measured.Points:0.00} pt against {expected.Points:0.00} pt");

    private static string Pict(string style)
        => $"""<w:pict><v:shape id="s" style="{style}"/></w:pict>""";

    private static string Paragraph(string? shape)
        => $"<w:p><w:r><w:t>Alpha</w:t>{shape ?? string.Empty}</w:r></w:p>";

    /// <summary>The height of the first line the body lays out.</summary>
    private static Length FirstLineHeight(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "vml.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Pages[0].Lines[0].Box.Height;
    }

    private static MemoryStream BuildPackage(string body)
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

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:v="urn:schemas-microsoft-com:vml"
                        xmlns:o="urn:schemas-microsoft-com:office:office">
              <w:body>
                {body}
                <w:sectPr>
                  <w:pgSz w:w="11906" w:h="16838"/>
                  <w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
                           w:header="709" w:footer="709" w:gutter="0"/>
                </w:sectPr>
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
