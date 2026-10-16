using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// That Calc's 720 dpi reference device reaches a drawn cell.
/// </summary>
/// <remarks>
/// <para>
/// The arithmetic is tested in <c>XRay.Office.Text.Tests.ApplicationGridTests</c> against 468
/// (face, size) pairs read out of LibreOffice's own PDFs. This is the wiring, and it is asserted
/// on the baselines of a cell read from a package rather than on a resolved <c>LineMetrics</c>: a
/// constructed metric would agree with whatever grid it was constructed with, and the thing that
/// can break is <c>SheetFonts</c> handing out a grid-less one.
/// </para>
/// <para>
/// <b>720 dpi and not the 8640 the C++ names.</b> <c>ScDocument::GetVirtualDevice_100th_mm</c>
/// really is <c>RefDevMode::MSO1</c>, and a printed cell is not formatted against it —
/// <c>ScOutputData</c> formats against the output device, which on a PDF export is the writer's
/// own reference device at 720 dpi. Measured, the two score 92 of 273 and 273 of 273.
/// </para>
/// </remarks>
public sealed class SheetReferenceDeviceTests
{
    private static string Sheet(double points) => $"""
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document
         xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
         xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
         xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
         xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"
         xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
         xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
         office:version="1.3"
         office:mimetype="application/vnd.oasis.opendocument.spreadsheet">
        <office:font-face-decls>
         <style:font-face style:name="Liberation Sans" svg:font-family="'Liberation Sans'"/>
        </office:font-face-decls>
        <office:automatic-styles>
         <style:style style:name="co1" style:family="table-column">
          <style:table-column-properties style:column-width="18cm"/></style:style>
         <style:style style:name="ro1" style:family="table-row">
          <style:table-row-properties style:row-height="6cm" style:use-optimal-row-height="false"/>
         </style:style>
         <style:style style:name="ce1" style:family="table-cell">
          <style:table-cell-properties style:vertical-align="top" fo:padding="0cm"
            fo:wrap-option="wrap"/>
          <style:text-properties style:font-name="Liberation Sans" fo:font-size="{points:0.#}pt"
            style:font-name-asian="Liberation Sans" style:font-size-asian="{points:0.#}pt"/>
         </style:style>
        </office:automatic-styles>
        <office:body><office:spreadsheet>
         <table:table table:name="S">
          <table:table-column table:style-name="co1"/>
          <table:table-row table:style-name="ro1">
           <table:table-cell table:style-name="ce1" office:value-type="string">
            <text:p>Hxy one</text:p><text:p>Hxy two</text:p><text:p>Hxy three</text:p>
           </table:table-cell>
          </table:table-row>
         </table:table>
        </office:spreadsheet></office:body>
        </office:document>
        """;

    private static List<long> Baselines(double points)
    {
        using IDocument read = new SpreadsheetReader().Read(
            DocumentSource.FromBytes(Encoding.UTF8.GetBytes(Sheet(points)), "cell.fods"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)((IPaginatedDocument)read).Layout()).Pages[0].Draw(sink);

        return [.. sink.Pages[0].Runs
            .Where(run => run.Text.StartsWith("Hxy", StringComparison.Ordinal))
            .Select(run => run.Origin.Y.Mm100)
            .Distinct()
            .Order()];
    }

    [Theory]
    // LibreOffice's own PDF, `probes/refdev-01/probe-calc.py`: a multi-paragraph cell at these
    // sizes puts consecutive baselines exactly this far apart, in whole 1/100 mm.
    [InlineData(10.0, 395)]
    [InlineData(13.5, 533)]
    [InlineData(18.0, 709)]
    [InlineData(20.5, 808)]
    [InlineData(24.0, 946)]
    public void AMultiLineCellsBaselinesLandWhereLibreOfficePutsThem(double points, long pitch)
    {
        List<long> baselines = Baselines(points);

        baselines.Count.ShouldBe(3);
        for (int i = 1; i < baselines.Count; i++)
        {
            (baselines[i] - baselines[i - 1]).ShouldBe(pitch);
        }
    }

    [Fact]
    public void ExactScalingIsNotWhatLibreOfficeDraws()
    {
        // 18 pt Liberation Sans is 1854 + 434 units over a 2048-unit em; scaled exactly on a
        // 635-unit em that is 709.4, and it happens to round to the 709 the device gives. 24 pt is
        // where the two part company: exact scaling gives 945.9 → 946 and… so does the device. The
        // honest separator is 10 pt, where exact scaling gives 394 and Calc draws 395.
        Math.Round((1854 + 434) * 353.0 / 2048).ShouldBe(394);

        List<long> baselines = Baselines(10.0);
        (baselines[1] - baselines[0]).ShouldBe(395);
    }
}
