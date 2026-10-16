using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell font's <c>&lt;color theme="n"/&gt;</c> is resolved against the <em>workbook's</em>
/// theme, not against the standard Office palette.
/// </summary>
/// <remarks>
/// <para>
/// Fills, borders and conditional formats already went through <see cref="XlsxPalette"/> and so
/// through the theme part; fonts had a hard-coded twelve-colour table of their own, which is
/// right for a workbook that has not been re-themed and wrong by a hue for one that has.
/// <strong>102 of the corpus's 947 documents re-theme a slot their fonts actually name</strong> —
/// so this was not one document's defect. Two of them were read blind by reviewers who had never
/// seen the file: <c>053_Personal_asset_inventory_5446d84b.xlsx</c>, whose 48 pt heading we drew
/// in <c>#4472C4</c> against 26.2.4.2's <c>#177185</c>, and
/// <c>070_Equipment_inventory_list_Use_this_template_fd524c8a.xlsx</c>, whose title we drew olive
/// where the reference draws steel blue.
/// </para>
/// <para>
/// The slot order is SpreadsheetML's and light and dark are swapped against the scheme's own
/// element order — slot 0 is <c>lt1</c> and slot 1 is <c>dk1</c>. That is why the first case
/// below asserts a background/text pair rather than only an accent: getting the pair the wrong
/// way round paints every default-coloured cell white on white, and an accent-only test cannot
/// see it.
/// </para>
/// </remarks>
public sealed class SheetThemeFontColourTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    [Theory]
    [InlineData(0, 0xFAFAFAu)]   // lt1  — the light background, first in SpreadsheetML's order
    [InlineData(1, 0x101010u)]   // dk1  — the dark text, second
    [InlineData(2, 0xF0F0F0u)]   // lt2
    [InlineData(3, 0x202020u)]   // dk2
    [InlineData(4, 0x177185u)]   // accent1 — the slot 053's heading names
    [InlineData(9, 0x639FCCu)]   // accent6 — the slot 070's title names
    [InlineData(10, 0x6B9F25u)]  // hlink
    public void AFontsThemeSlotComesFromTheWorkbooksOwnScheme(int slot, uint expected)
    {
        SheetCellFormat format = FormatFor($"""<color theme="{slot}"/>""");
        format.Colour.ShouldBe(Colour.FromRgb(expected));
    }

    [Fact]
    public void AWorkbookWithNoThemePartStillReadsItsOtherColours()
    {
        // The theme part is genuinely optional, and a missing one must not take the rgb and
        // indexed routes down with it.
        XElement styleSheet = StyleSheet("""<color rgb="FF336699"/>""");
        XlsxCellFormats.Read(styleSheet, XlsxStyles.Read(styleSheet), null)
            .Formats[0].Colour.ShouldBe(Colour.FromRgb(0x336699));
    }

    [Fact]
    public void ATintIsAppliedToTheSchemeColourAndNotToTheDefaultOne()
    {
        // A tint is a luminance modulation on whatever the slot resolved to, so reading the
        // slot from the wrong palette moves the tinted answer too — which is how the defect
        // stayed invisible on the many cells that carry `theme="1" tint="…"` over a scheme
        // whose dk1 is black anyway.
        SheetCellFormat plain = FormatFor("""<color theme="4"/>""");
        SheetCellFormat lightened = FormatFor("""<color theme="4" tint="0.5"/>""");

        plain.Colour.ShouldBe(Colour.FromRgb(0x177185));
        lightened.Colour.ShouldNotBe(plain.Colour);

        // Lightening towards white raises every channel above the untinted slot's.
        lightened.Colour.R.ShouldBeGreaterThan(plain.Colour.R);
        lightened.Colour.G.ShouldBeGreaterThan(plain.Colour.G);
        lightened.Colour.B.ShouldBeGreaterThan(plain.Colour.B);
    }

    private static SheetCellFormat FormatFor(string colour)
    {
        XElement styleSheet = StyleSheet(colour);
        return XlsxCellFormats.Read(styleSheet, XlsxStyles.Read(styleSheet), Theme).Formats[0];
    }

    /// <summary>A colour scheme that differs from the standard Office one in every slot used.</summary>
    private static XElement Theme => XElement.Parse(
        $"""
        <theme xmlns="{A}"><themeElements><clrScheme name="Probe">
        <dk1><srgbClr val="101010"/></dk1>
        <lt1><srgbClr val="FAFAFA"/></lt1>
        <dk2><srgbClr val="202020"/></dk2>
        <lt2><srgbClr val="F0F0F0"/></lt2>
        <accent1><srgbClr val="177185"/></accent1>
        <accent2><srgbClr val="4FB4C8"/></accent2>
        <accent3><srgbClr val="E6640F"/></accent3>
        <accent4><srgbClr val="9CBC00"/></accent4>
        <accent5><srgbClr val="5D6C38"/></accent5>
        <accent6><srgbClr val="639FCC"/></accent6>
        <hlink><srgbClr val="6B9F25"/></hlink>
        <folHlink><srgbClr val="E8630E"/></folHlink>
        </clrScheme></themeElements></theme>
        """);

    private static XElement StyleSheet(string colour) => XElement.Parse(
        $"""
        <styleSheet xmlns="{Ns}">
        <fonts count="1"><font><sz val="11"/>{colour}<name val="Liberation Sans"/></font></fonts>
        <cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
        <cellXfs count="1">
        <xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0" applyFont="1"/></cellXfs>
        </styleSheet>
        """);
}
