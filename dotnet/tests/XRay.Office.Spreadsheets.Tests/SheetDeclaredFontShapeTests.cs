using System.Xml.Linq;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.MsBinary;
using XRay.Office.Spreadsheets.Ooxml;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A workbook states a generic family beside every font name, and it decides the fallback.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is not cosmetic and the corpus says so.</strong>
/// <c>2017-04-27-Lease-Transition-Records-Checklist-FINAL-1.xlsx</c> sets its body in
/// <c>Bell MT</c>, which is installed on no Linux box, and declares it <c>family="1"</c> — roman.
/// Measured on 26.2.4.2, LibreOffice renders it in DejaVu Serif; delete just those five attributes
/// from <c>xl/styles.xml</c> and the same binary renders the same workbook in DejaVu Sans. So the
/// name alone does not decide it — <c>fc-match "Bell MT"</c> answers DejaVu Sans — and a reader
/// that drops the declaration renders a serif workbook in a grotesque, which moves every line
/// break and every wrapped row height in it.
/// </para>
/// <para>
/// The codes are the Windows <c>FF_*</c> constants and are shared by SpreadsheetML, XLSB and BIFF:
/// LibreOffice reads the attribute and the record byte into one field against the same
/// <c>OOX_FONTFAMILY_*</c> enumeration (<c>sc/source/filter/oox/stylesbuffer.cxx:110-116</c>,
/// <c>:616</c>, <c>:672</c>). ODF spells the same set as words on <c>style:font-face</c>.
/// </para>
/// <para>
/// Only roman and swiss are carried across, and that is measured rather than assumed. Rendering the
/// same workbook at <c>family="2"</c>, <c>"3"</c> and <c>"5"</c> on 26.2.4.2 gives DejaVu Sans in
/// all three — the undeclared answer — and a flat ODS naming the same absent family gives DejaVu
/// Sans for <c>swiss</c>, <c>modern</c>, <c>decorative</c>, <c>script</c> and <c>system</c> and
/// DejaVu Serif only for <c>roman</c>. Mapping <c>modern</c> onto a monospaced face is the tempting
/// mistake and would invent a divergence.
/// </para>
/// </remarks>
public sealed class SheetDeclaredFontShapeTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    /// <summary>A <c>styleSheet</c> holding one font with the family code given.</summary>
    /// <param name="family">The <c>family</c> element's value, or null to omit the element.</param>
    private static XElement StyleSheet(string? family)
    {
        XElement font = new(
            XName.Get("font", Namespace),
            new XElement(XName.Get("name", Namespace), new XAttribute("val", "Bell MT")),
            new XElement(XName.Get("sz", Namespace), new XAttribute("val", "13")));

        if (family is not null)
        {
            font.Add(new XElement(XName.Get("family", Namespace), new XAttribute("val", family)));
        }

        return new XElement(
            XName.Get("styleSheet", Namespace),
            new XElement(XName.Get("fonts", Namespace), font),
            new XElement(
                XName.Get("cellXfs", Namespace),
                new XElement(
                    XName.Get("xf", Namespace),
                    new XAttribute("fontId", "0"),
                    new XAttribute("applyFont", "1"))));
    }

    private static SheetCellFormat FormatFor(string? family)
    {
        XElement sheet = StyleSheet(family);
        return XlsxCellFormats.Read(sheet, XlsxStyles.Read(sheet), null).Formats[0];
    }

    [Theory]
    [InlineData("1", FontFamilyClass.Serif)]        // FF_ROMAN
    [InlineData("2", FontFamilyClass.SansSerif)]    // FF_SWISS
    [InlineData("3", FontFamilyClass.Unknown)]      // FF_MODERN
    [InlineData("4", FontFamilyClass.Unknown)]      // FF_SCRIPT
    [InlineData("5", FontFamilyClass.Unknown)]      // FF_DECORATIVE
    [InlineData("0", FontFamilyClass.Unknown)]      // FF_DONTCARE
    [InlineData(null, FontFamilyClass.Unknown)]     // the element omitted altogether
    public void TheFamilyElementBecomesAShapeTheResolverActsOn(
        string? family, FontFamilyClass expected)
        => FormatFor(family).DeclaredFontClass.ShouldBe(expected);

    [Fact]
    public void AFamilyElementThatIsNotANumberIsIgnoredRatherThanFailing()
    {
        // Leniency rule 5: a producer writing val="roman" here has written SpreadsheetML that says
        // nothing, not SpreadsheetML that cannot be read.
        FormatFor("roman").DeclaredFontClass.ShouldBe(FontFamilyClass.Unknown);
        FormatFor(string.Empty).DeclaredFontClass.ShouldBe(FontFamilyClass.Unknown);
    }

    [Fact]
    public void TheDeclarationReachesTheFaceTheCellIsActuallySetIn()
    {
        // The whole point, and the only assertion here that depends on the machine: Bell MT is
        // installed nowhere, so the declaration is the only thing that can decide the answer.
        // Guarded rather than skipped, because a box without DejaVu would fail this for a reason
        // that has nothing to do with the code under test — and CLAUDE.md's own warning is that
        // fc-match never fails, it always returns something.
        SheetFace? serif = SheetFonts.For(FormatFor("1"));
        SheetFace? undeclared = SheetFonts.For(FormatFor(null));

        if (serif is null || undeclared is null) return;
        if (undeclared.Value.Face.FamilyName?.StartsWith("DejaVu", StringComparison.Ordinal)
            is not true)
        {
            return;
        }

        serif.Value.Face.FamilyName.ShouldBe("DejaVu Serif");
        undeclared.Value.Face.FamilyName.ShouldBe("DejaVu Sans");
    }

    [Fact]
    public void TheWorkbooksDefaultFontCarriesItToo()
    {
        // A column width is a count of digits of the default font, so the face that font resolves
        // to decides the geometry of every column — not just the ink. The declaration has to reach
        // SheetDefaultFont or a serif workbook is paginated on a grotesque's digit.
        XElement sheet = StyleSheet("1");
        XlsxCellFormatTable table = XlsxCellFormats.Read(sheet, XlsxStyles.Read(sheet), null);

        table.DefaultColumnFont.DeclaredClass.ShouldBe(FontFamilyClass.Serif);
        table.DefaultFont.DeclaredFontClass.ShouldBe(FontFamilyClass.Serif);
    }

    [Fact]
    public void ARunThatRenamesTheFaceDoesNotInheritTheCellsDeclaration()
    {
        // The declaration qualifies the name it was written beside. An rPr naming a different face
        // and saying nothing about its shape has said nothing about *that* face — filing the new
        // name under the old one's family is how a serif cell would drag a sans run into DejaVu
        // Serif. A run that renames nothing is the other case and does keep it.
        XElement sheet = StyleSheet("1");
        XlsxCellFormatTable table = XlsxCellFormats.Read(sheet, XlsxStyles.Read(sheet), null);
        SheetCellFormat cell = table.Formats[0];

        table.Apply(cell, new XlsxRunFont("Arial", null, null, null, null))
             .DeclaredFontClass.ShouldBe(FontFamilyClass.Unknown);

        table.Apply(cell, new XlsxRunFont(null, 9, true, null, null))
             .DeclaredFontClass.ShouldBe(FontFamilyClass.Serif);

        // Named, not positional: this record gained Underline and StruckThrough between the
        // sixth position and here, and a positional 1 silently became an underline style.
        table.Apply(cell, new XlsxRunFont("Bodoni MT", null, null, null, null, DeclaredFamily: 1))
             .DeclaredFontClass.ShouldBe(FontFamilyClass.Serif);
    }

    [Theory]
    [InlineData("roman", FontFamilyClass.Serif)]
    [InlineData("ROMAN", FontFamilyClass.Serif)]
    [InlineData("swiss", FontFamilyClass.SansSerif)]
    [InlineData("modern", FontFamilyClass.Unknown)]
    [InlineData("decorative", FontFamilyClass.Unknown)]
    [InlineData("script", FontFamilyClass.Unknown)]
    [InlineData("system", FontFamilyClass.Unknown)]
    [InlineData(null, FontFamilyClass.Unknown)]
    public void OdfSpellsTheSameCodesAsWords(string? generic, FontFamilyClass expected)
        => SheetDeclaredFonts.FromOdfGeneric(generic).ShouldBe(expected);
}

/// <summary>
/// The BIFF <c>FONT</c> record's family byte, which is the same declaration in binary.
/// </summary>
/// <remarks>
/// Separate from the SpreadsheetML tests because the risk is different: there the declaration is an
/// attribute that is either present or not, and here it is one byte at one offset in a fixed-layout
/// record. It sits between the underline byte and the character set, and every neighbour of it is
/// skipped rather than read — so nothing else in the suite would notice the offset being wrong by
/// one, and a wrong offset reads the character set or the underline as a family and files half the
/// workbook's fonts under roman.
/// </remarks>
public sealed class XlsDeclaredFontShapeTests
{
    /// <summary>A globals stream holding one <c>FONT</c> record and one empty worksheet.</summary>
    private static SheetDefaultFont? DefaultFontOf(byte family)
    {
        List<byte> globals =
            [.. BiffChartFixture.Record(BiffChartFixture.Bof, [0x00, 0x06, 0x05, 0x00, 0, 0, 0, 0])];
        globals.AddRange(BiffChartFixture.FontRecord("Bell MT", family));

        byte[] sheet =
        [
            .. BiffChartFixture.Record(BiffChartFixture.Bof, [0x00, 0x06, 0x10, 0x00, 0, 0, 0, 0]),
            .. BiffChartFixture.Record(
                BiffChartFixture.Dimensions, [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0]),
            .. BiffChartFixture.Record(BiffChartFixture.Eof, []),
        ];

        int directory = BiffChartFixture.BoundSheetRecord(0, worksheet: true).Length + 4;
        globals.AddRange(
            BiffChartFixture.BoundSheetRecord(globals.Count + directory, worksheet: true));
        globals.AddRange(BiffChartFixture.Record(BiffChartFixture.Eof, []));
        globals.AddRange(sheet);

        List<Diagnostic> diagnostics = [];
        XlsWorkbookReader reader = new([.. globals], diagnostics);
        reader.Read();

        return reader.Layouts.Single().Grid.ColumnDigits?.Font;
    }

    [Theory]
    [InlineData(0, FontFamilyClass.Unknown)]
    [InlineData(1, FontFamilyClass.Serif)]
    [InlineData(2, FontFamilyClass.SansSerif)]
    [InlineData(3, FontFamilyClass.Unknown)]
    [InlineData(5, FontFamilyClass.Unknown)]
    public void TheFamilyByteIsReadFromItsOwnOffset(byte family, FontFamilyClass expected)
    {
        SheetDefaultFont? font = DefaultFontOf(family);

        font.ShouldNotBeNull();
        font.Family.ShouldBe("Bell MT");
        font.DeclaredClass.ShouldBe(expected);
    }
}
