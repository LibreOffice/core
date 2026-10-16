using System.IO.Compression;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Formats;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The pictures a worksheet's legacy VML drawing carries, which Calc draws and the DrawingML
/// drawing part beside them does not reach.
/// </summary>
/// <remarks>
/// <para>
/// See <see cref="XlsxLegacyPictures"/> for the mechanism and the probe. In short: Excel writes a
/// camera-tool picture as an <c>xdr:pic</c> inside <c>mc:Choice Requires="a14"</c> <em>and</em> as
/// a <c>v:shape</c> in the legacy VML, <c>oox</c> honours no <c>a14</c> choice, and so the VML one
/// is the only one Calc sees. Unwrapping the <c>mc:AlternateContent</c> on
/// <c>013_Contextures_chart_sample</c> makes 26.2.4.2 draw the picture twice, which is what
/// identifies the two as separate objects rather than two spellings of one.
/// </para>
/// <para>
/// The workbooks are written here rather than kept as fixtures because what is under test is the
/// anchor and the part selection, and one fixture can only carry one spelling of either.
/// </para>
/// </remarks>
public sealed class XlsxLegacyPictureTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string Rns = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    /// <summary>A one-pixel PNG, so the reader has real bytes to decide a media kind from.</summary>
    private static readonly byte[] Png = Convert.FromBase64String(
        "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==");

    private const string PictureShape = """
        <v:shape id="Picture 4" o:spid="_x0000_s2111" type="#_x0000_t75"
                 style='position:absolute;margin-left:48pt;margin-top:84pt;width:326.25pt;height:221.25pt;visibility:visible'>
          <v:imagedata o:relid="rId1" o:title=""/>
          <x:ClientData ObjectType="Pict">
            <x:SizeWithCells/>
            <x:Anchor>1, 0, 7, 0, 6, 96, 21, 48</x:Anchor>
            <x:CF>Pict</x:CF>
            <x:Camera/>
          </x:ClientData>
        </v:shape>
        """;

    /// <summary>
    /// Reads one sheet's legacy pictures out of a workbook written to these arguments.
    /// </summary>
    /// <param name="shapes">The VML the legacy drawing part holds.</param>
    /// <param name="element">
    /// The worksheet element that names the VML part — <c>legacyDrawing</c> for a sheet object and
    /// <c>legacyDrawingHF</c> for a header or footer image, which share a relationship type.
    /// </param>
    /// <param name="hasImagePart">Whether the image the VML names exists in the package.</param>
    private static List<SheetDrawing> Read(
        string shapes, string element = "legacyDrawing", bool hasImagePart = true)
    {
        MemoryStream stream = new(Package(shapes, element, hasImagePart));
        using XlsxFile file = XlsxFile.Open(stream);
        XElement? worksheet = file.LoadSheet(file.Sheets[0]);

        return XlsxLegacyPictures.Read(file.Package, file.Sheets[0].PartName, worksheet);
    }

    private static byte[] Package(string shapes, string element, bool hasImagePart)
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml",
                """<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">"""
                + """<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>"""
                + """<Default Extension="vml" ContentType="application/vnd.openxmlformats-officedocument.vmlDrawing"/>"""
                + """<Default Extension="png" ContentType="image/png"/>"""
                + """<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>"""
                + """<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>"""
                + "</Types>");

            Write(archive, "_rels/.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/officeDocument" Target="xl/workbook.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/_rels/workbook.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/worksheet" Target="/xl/worksheets/sheet1.xml"/>"""
                + "</Relationships>");

            Write(archive, "xl/workbook.xml",
                $"""<workbook xmlns="{Ns}" xmlns:r="{Rns}"><sheets><sheet name="One" sheetId="1" r:id="rId1"/></sheets></workbook>""");

            // `rId1` on the *sheet* names the VML part and `rId1` on the *VML part* names the
            // image. Deliberately the same id for two different targets: resolving a picture's
            // `o:relid` against the sheet finds a VML part where an image belongs, which is the
            // mistake this shares with the DrawingML reader beside it.
            Write(archive, "xl/worksheets/_rels/sheet1.xml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + $"""<Relationship Id="rId1" Type="{Rns}/vmlDrawing" Target="/xl/drawings/vmlDrawing1.vml"/>"""
                + "</Relationships>");

            Write(archive, "xl/drawings/_rels/vmlDrawing1.vml.rels",
                """<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">"""
                + (hasImagePart
                    ? $"""<Relationship Id="rId1" Type="{Rns}/image" Target="/xl/media/image1.png"/>"""
                    : string.Empty)
                + "</Relationships>");

            Write(archive, "xl/worksheets/sheet1.xml",
                $"""<worksheet xmlns="{Ns}" xmlns:r="{Rns}"><sheetData/><{element} r:id="rId1"/></worksheet>""");

            Write(archive, "xl/drawings/vmlDrawing1.vml",
                """<xml xmlns:v="urn:schemas-microsoft-com:vml" xmlns:o="urn:schemas-microsoft-com:office:office" xmlns:x="urn:schemas-microsoft-com:office:excel">"""
                + shapes + "</xml>");

            if (hasImagePart)
            {
                ZipArchiveEntry image = archive.CreateEntry("xl/media/image1.png");
                using Stream content = image.Open();
                content.Write(Png, 0, Png.Length);
            }
        }

        return buffer.ToArray();
    }

    private static void Write(ZipArchive archive, string name, string content)
    {
        ZipArchiveEntry entry = archive.CreateEntry(name);
        using StreamWriter writer = new(entry.Open(), new UTF8Encoding(false));
        writer.Write(content);
    }

    [Fact]
    public void APictureOnTheLegacyDrawingIsRead()
    {
        List<SheetDrawing> drawings = Read(PictureShape);

        drawings.Count.ShouldBe(1);
        drawings[0].Image.ShouldNotBeNull();
        drawings[0].Name.ShouldBe("Picture 4");
    }

    /// <summary>
    /// The anchor is the client anchor's two corners, and the kind is the two-cell one.
    /// </summary>
    /// <remarks>
    /// <c>ShapeAnchor::calcAnchorRectEmu</c> gives <c>ANCHOR_VML</c> and <c>ANCHOR_TWOCELL</c> the
    /// same case label for both position and size, so a VML anchor resizes with its cells and needs
    /// no kind of its own.
    /// </remarks>
    [Fact]
    public void TheAnchorIsTheClientAnchorsTwoCorners()
    {
        SheetDrawing picture = Read(PictureShape)[0];

        picture.Anchor.ShouldBe(SheetAnchorKind.TwoCell);
        picture.From.Column.ShouldBe(1);
        picture.From.Row.ShouldBe(7);
        picture.To.Column.ShouldBe(6);
        picture.To.Row.ShouldBe(21);
    }

    /// <summary>
    /// An <c>x:Anchor</c>'s offsets are screen pixels, where a DrawingML anchor's are EMUs.
    /// </summary>
    /// <remarks>
    /// <c>ShapeAnchor::importVmlAnchor</c> sets <c>CellAnchorType::Pixel</c> and
    /// <c>calcCellAnchorEmu</c> scales through <c>Unit::ScreenX</c>, which is 96 dpi headless. The
    /// fixture states 96 and 48, so the two offsets must be exactly one inch and half an inch —
    /// read as EMUs they would be a ten-thousandth of a point and the picture would sit on its
    /// cell's corner.
    /// </remarks>
    [Fact]
    public void TheAnchorsOffsetsAreScreenPixels()
    {
        SheetDrawing picture = Read(PictureShape)[0];

        picture.To.ColumnOffset.Points.ShouldBe(72, 0.0001);
        picture.To.RowOffset.Points.ShouldBe(36, 0.0001);
    }

    /// <summary>
    /// A header or footer image is not an object on the sheet, even though its part is reached by
    /// the same relationship type.
    /// </summary>
    /// <remarks>
    /// The regression this exists for is measured: keying on the <c>vmlDrawing</c> relationship
    /// type rather than on the worksheet's <c>&lt;legacyDrawing&gt;</c> element draws
    /// <c>PBN Matrix NAAs (V01)</c>'s 24 header watermarks, and one each on
    /// <c>UAE Type Accepted Aircraft Models</c> and <c>Application_Compliance_Checklist</c>, as
    /// pictures on the grid. All three match the reference today.
    /// </remarks>
    [Fact]
    public void AHeaderOrFooterDrawingIsNotAnObjectOnTheSheet()
        => Read(PictureShape, element: "legacyDrawingHF").ShouldBeEmpty();

    /// <summary>
    /// A comment's shape is not a picture. <c>VmlDrawing::isShapeSupported</c> excludes exactly
    /// <c>XML_Note</c> and nothing else, because the comment machinery draws those.
    /// </summary>
    [Fact]
    public void ANoteShapeIsNotReadHere()
        => Read(PictureShape.Replace("ObjectType=\"Pict\"", "ObjectType=\"Note\"",
                                     StringComparison.Ordinal)).ShouldBeEmpty();

    /// <summary>
    /// A hidden shape reaches no page. <c>vmlshape.cxx:897-901</c> sets <c>Printable</c> false as
    /// well as <c>Visible</c> false, so it is out of the print area too.
    /// </summary>
    [Fact]
    public void AHiddenShapeIsNotDrawn()
        => Read(PictureShape.Replace("visibility:visible", "visibility:hidden",
                                     StringComparison.Ordinal)).ShouldBeEmpty();

    /// <summary>A shape carrying no <c>v:imagedata</c> is not a picture.</summary>
    /// <remarks>
    /// This is where the legacy form controls fall out — a Button or a Scroll Bar is a VML shape
    /// with client data and no image, and LibreOffice rebuilds it as an OLE form control. The
    /// corpus holds one, hidden, in <c>015_Free_Gantt_Chart_Template</c>.
    /// <para>
    /// <strong>A drift guard, not a detector, and measured as one.</strong> Making the
    /// <c>v:imagedata</c> requirement inert leaves this test passing, because a shape with no
    /// image names no relationship and is dropped a few lines later by the guard that
    /// <see cref="APictureWhoseImageIsMissingIsDropped"/> covers. It is kept because the two
    /// guards say different things — "a control is not a picture" and "a picture whose bytes are
    /// gone is not anchored" — and only one of them survives if the reader ever grows a control
    /// arm.
    /// </para>
    /// </remarks>
    [Fact]
    public void AShapeWithNoImageDataIsNotAPicture()
    {
        List<SheetDrawing> drawings = Read("""
            <v:shape id="Scroll Bar 46" style='position:absolute;visibility:visible'>
              <x:ClientData ObjectType="Scroll"><x:Anchor>1, 0, 7, 0, 6, 0, 21, 0</x:Anchor></x:ClientData>
            </v:shape>
            """);

        drawings.ShouldBeEmpty();
    }

    /// <summary>
    /// A shape with no client anchor is skipped, and this pins the gap rather than the behaviour.
    /// </summary>
    /// <remarks>
    /// <c>ShapeBase::calcShapeRectangle</c> falls back to the CSS rectangle there, and 26.2.4.2
    /// does too: with <c>013_Contextures_chart_sample</c>'s <c>x:Anchor</c> deleted the reference
    /// draws the picture at <c>margin-left:48pt</c> and 326.25pt wide, moving its first label from
    /// x = 133.8 to 112.6. No corpus worksheet reaches that arm, so it is recorded rather than
    /// written — and this test is what will fail loudly if someone writes it.
    /// </remarks>
    [Fact]
    public void AShapeWithNoClientAnchorIsSkipped()
    {
        List<SheetDrawing> drawings = Read("""
            <v:shape id="Picture 4" type="#_x0000_t75"
                     style='position:absolute;margin-left:48pt;margin-top:84pt;width:326.25pt;height:221.25pt'>
              <v:imagedata o:relid="rId1" o:title=""/>
            </v:shape>
            """);

        drawings.ShouldBeEmpty();
    }

    /// <summary>
    /// A picture whose image part is missing is dropped rather than anchored empty.
    /// </summary>
    /// <remarks>
    /// The same rule the DrawingML reader follows, and it matters for the print area: an anchored
    /// nothing still stretches the printed block over its cells.
    /// </remarks>
    [Fact]
    public void APictureWhoseImageIsMissingIsDropped()
        => Read(PictureShape, hasImagePart: false).ShouldBeEmpty();

    /// <summary>A sheet naming no legacy drawing reads nothing, and opens no part looking.</summary>
    [Fact]
    public void AWorksheetWithNoLegacyDrawingReadsNothing()
        => Read(PictureShape, element: "pageMargins").ShouldBeEmpty();

    /// <summary>
    /// The legacy picture reaches the sheet's drawings, and does so beside whatever the DrawingML
    /// part put there rather than instead of it.
    /// </summary>
    /// <remarks>
    /// The wiring test, and it is here because the reader can be entirely right and never be
    /// called: every test above drives <see cref="XlsxLegacyPictures.Read"/> directly, so
    /// replacing the call site in <c>XlsxReader</c> with an empty list breaks none of them.
    /// Measured that way through <c>verify-test.sh</c> before this was written.
    /// </remarks>
    [Fact]
    public void ALegacyPictureReachesTheSheetsDrawings()
    {
        using DocumentSource source = DocumentSource.FromBytes(
            Package(PictureShape, "legacyDrawing", hasImagePart: true), "legacy.xlsx");
        using OoxmlSpreadsheetDocument document =
            XlsxReader.Read(source, DocumentFormat.Xlsx);

        document.Sheets[0].Drawings.Items.Count(d => d.Name == "Picture 4").ShouldBe(1);
    }
}
