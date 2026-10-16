using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The <c>dataBar</c> family, whose length is not the percentage the specification describes.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Every expectation here was read out of 26.2.4.2's own rendering of the same fixture.</strong>
/// Each of the four was converted twice — <c>--convert-to fods</c> for the rule the reference
/// thinks it imported, and <c>--convert-to pdf</c> for the rectangles it then painted, measured
/// against the cell's own extent. <c>probes/cond-format-r97/results.md</c> records both.
/// </para>
/// <para>
/// The finding the first two fixtures exist for: <strong>whether <c>minLength</c> and
/// <c>maxLength</c> mean anything at all is decided by the <c>x14</c> extension, which does not
/// state them.</strong> <c>DataBarRule</c>'s constructor leaves the axis at <c>databar::NONE</c>
/// (<c>sc/source/filter/oox/condformatbuffer.cxx</c>:351-356) and only
/// <c>ExtCfDataBarRule::finalizeImport</c> moves it, defaulting an extension with no
/// <c>axisPosition</c> to <c>AUTOMATIC</c> (<c>:1630-1643</c>) — and
/// <c>ScDataBarFormat::GetDataBarInfo</c>'s <c>AUTOMATIC</c> arm ignores both lengths
/// (<c>sc/source/core/data/colorscale.cxx</c>:1008-1049). So two files whose main-namespace
/// markup is identical draw 10/30/50/70/90 % and 0/25/50/75/100 %, and it is the second that
/// every corpus rule is.
/// </para>
/// </remarks>
public sealed class XlsxDataBarTests
{
    /// <summary>The bar colour every fixture states, as an <c>rgb</c> so no theme is involved.</summary>
    private const string Bar = "#2E75B6";

    /// <summary>The negative colour the third one states.</summary>
    private const string Negative = "#C00000";

    /// <summary>
    /// <c>COL_LIGHTRED</c>, which a rule stating no negative colour of its own falls back to.
    /// </summary>
    private const string SourceRed = "#FF0000";

    [Fact]
    public void WithNoExtensionTheAxisIsNoneAndTheTwoLengthsDecideTheBar()
    {
        // `sheet-cf-data-bar-lengths.xlsx` states `cfvo min`/`max` over 0…100 and no extension, so
        // it keeps the constructor's `NONE` — 26.2.4.2's own fods writes `axis-position="none"`
        // and `min-length="10" max-length="90"`, the `importAttribs` defaults. Its PDF paints the
        // five cells at 4.82, 14.23, 23.53, 32.88 and 42.29 pt inside one 46.91 pt cell, which is
        // 1 : 3 : 5 : 7 : 9 — the ratio 10/30/50/70/90 gives and the ratio a plain percentage of
        // the range does not.
        SheetFormatting formatting = Read("sheet-cf-data-bar-lengths.xlsx");

        Length(formatting, 0).ShouldBe(10);
        Length(formatting, 1).ShouldBe(30);
        Length(formatting, 2).ShouldBe(50);
        Length(formatting, 3).ShouldBe(70);
        Length(formatting, 4).ShouldBe(90);

        formatting.BarAt(0, 0)!.Value.Zero.ShouldBe(0);
        formatting.BarAt(0, 0)!.Value.Colour.ToString().ShouldBe(Bar);
        formatting.BarAt(0, 0)!.Value.HasAxis.ShouldBeFalse();

        // `mbGradient` defaults true in `ScDataBarFormatData` and only `x14:dataBar/@gradient`
        // changes it, so this rule asks for one; the reference's PDF answers with 209 slices
        // fading from #2e75b6 to white. Painted solid here, with nil corpus reach — see
        // `SheetDataBar.Gradient`.
        formatting.BarAt(0, 0)!.Value.Gradient.ShouldBeTrue();
    }

    [Fact]
    public void WithAnExtensionTheAxisIsAutomaticAndTheTwoLengthsAreIgnored()
    {
        // The same five values and the same main-namespace markup, plus an `x14:dataBar` stating
        // `autoMin`/`autoMax`, `gradient="0"` and no `axisPosition`. 26.2.4.2's fods writes
        // `auto-minimum`/`auto-maximum` — the extension's `cfvo` types replace the main
        // namespace's — and still writes `min-length="10" max-length="90"`, which is the point:
        // the numbers survive the import and no longer reach the bar. Its PDF paints A2…A5 at
        // 11.73, 23.47, 35.18 and 46.91 pt of a 46.91 pt cell, and **paints no bar at all for
        // A1**, whose value sits on the minimum.
        SheetFormatting formatting = Read("sheet-cf-data-bar-auto.xlsx");

        Length(formatting, 0).ShouldBe(0);
        Length(formatting, 1).ShouldBe(25);
        Length(formatting, 2).ShouldBe(50);
        Length(formatting, 3).ShouldBe(75);
        Length(formatting, 4).ShouldBe(100);

        formatting.BarAt(4, 0)!.Value.Zero.ShouldBe(0);
        formatting.BarAt(4, 0)!.Value.Gradient.ShouldBeFalse();
        formatting.BarAt(4, 0)!.Value.Colour.ToString().ShouldBe(Bar);
    }

    [Fact]
    public void ANegativeRangePutsTheAxisInsideTheCellAndPaintsLeftOfItInItsOwnColour()
    {
        // `cfvo num -100`/`num 100` with an `x14:negativeFillColor`, over −100…100. The zero is
        // `-100 × nMin/(nMax−nMin)` = 50, and the reference's PDF draws the axis at x 77.33 in a
        // cell running 53.89…100.80 — its exact midpoint — with the two negative bars reaching
        // left of it in #c00000 and the two positive ones right of it in #2e75b6. A3, at zero,
        // draws neither a bar nor an axis, because `drawDataBars` returns on a zero length before
        // it reaches the axis at all.
        SheetFormatting formatting = Read("sheet-cf-data-bar-negative.xlsx");

        Length(formatting, 0).ShouldBe(-100);
        Length(formatting, 1).ShouldBe(-50);
        Length(formatting, 2).ShouldBe(0);
        Length(formatting, 3).ShouldBe(50);
        Length(formatting, 4).ShouldBe(100);

        for (int row = 0; row < 5; row++)
        {
            formatting.BarAt(row, 0)!.Value.Zero.ShouldBe(50);
            formatting.BarAt(row, 0)!.Value.HasAxis.ShouldBeTrue();
        }

        formatting.BarAt(0, 0)!.Value.Colour.ToString().ShouldBe(Negative);
        formatting.BarAt(1, 0)!.Value.Colour.ToString().ShouldBe(Negative);
        formatting.BarAt(3, 0)!.Value.Colour.ToString().ShouldBe(Bar);
        formatting.BarAt(4, 0)!.Value.Colour.ToString().ShouldBe(Bar);
    }

    [Fact]
    public void WithNoNegativeFillColourANegativeValueTakesTheSourcesOwnRedAndNotTheBarsColour()
    {
        // The one-attribute control for the fixture above: the same rule over the same −100…100,
        // differing only in that its extension states **no** `x14:negativeFillColor`.
        //
        // `mbNeg` does not mean "a negative colour was stated". It is `ScDataBarFormatData`'s own
        // default of **true** (`sc/inc/colorscale.hxx`:107); the OOXML importer's only assignment
        // sets it true again beside `mxNegativeColor` (`condformatbuffer.cxx`:1658-1664), and the
        // single place in `sc/` that clears it is the **ODF** importer
        // (`sc/source/filter/xml/xmlcondformat.cxx`:483). So the `COL_LIGHTRED` fallback at
        // `sc/source/core/data/colorscale.cxx`:1082 is not unreachable from SpreadsheetML — it is
        // the usual answer there, taken by every bar whose extension omits the element.
        //
        // Measured twice at 26.2.4.2, both on this fixture: `--convert-to fods` writes
        // `calcext:negative-color="#ff0000"`, as it does for eight of the corpus's nine rules; and
        // `--convert-to pdf` paints A1 and A2 `#ff0000` at exactly the geometry the `#c00000`
        // fixture paints them. `probes/cond-format-r97/fixture-reference/painted.txt`.
        SheetFormatting formatting = Read("sheet-cf-data-bar-default-negative.xlsx");

        formatting.BarAt(0, 0)!.Value.Colour.ToString().ShouldBe(SourceRed);
        formatting.BarAt(1, 0)!.Value.Colour.ToString().ShouldBe(SourceRed);

        // The positives are untouched, so this is the negative arm alone and not a palette fault.
        formatting.BarAt(3, 0)!.Value.Colour.ToString().ShouldBe(Bar);
        formatting.BarAt(4, 0)!.Value.Colour.ToString().ShouldBe(Bar);

        // And the geometry is the stated-colour fixture's, which is what makes the pair a control.
        Length(formatting, 0).ShouldBe(-100);
        Length(formatting, 4).ShouldBe(100);
        formatting.BarAt(0, 0)!.Value.Zero.ShouldBe(50);
    }

    [Fact]
    public void ShowValueOffTakesTheCellsOwnNumberOffThePage()
    {
        // 26.2.4.2's PDF of `sheet-cf-data-bar-only.xlsx` holds five bars and **no text-showing
        // operator anywhere** — the numbers 20…100 are not drawn behind the bars, they are not
        // drawn at all. `ScOutputData::DrawStrings` clears `bDoCell` for the cell
        // (`sc/source/ui/view/output2.cxx`:1691-1697), after the row's height is settled.
        SheetFormatting formatting = Read("sheet-cf-data-bar-only.xlsx");

        for (int row = 0; row < 5; row++)
        {
            formatting.BarAt(row, 0)!.Value.ShowValue.ShouldBeFalse();
            formatting.HidesValue(row, 0).ShouldBeTrue();
        }

        // And the neighbouring fixture, whose rule states no `showValue`, keeps its numbers.
        SheetFormatting shown = Read("sheet-cf-data-bar-lengths.xlsx");
        shown.HidesValue(0, 0).ShouldBeFalse();
    }

    private static double Length(SheetFormatting formatting, int row)
        => formatting.BarAt(row, 0)!.Value.Length;

    /// <summary>Reads one fixture's conditional formats straight out of the package.</summary>
    private static SheetFormatting Read(string fixture)
    {
        using ZipArchive archive = ZipFile.OpenRead(Corpus.Require(fixture));

        return XlsxCellDecoration.Read(
            Part(archive, "xl/styles.xml"),
            null,
            Part(archive, "xl/worksheets/sheet1.xml"),
            XlsxSharedStrings.Read(Part(archive, "xl/sharedStrings.xml")),
            out _);
    }

    private static XElement? Part(ZipArchive archive, string name)
    {
        ZipArchiveEntry? entry = archive.GetEntry(name);
        if (entry is null) return null;

        using Stream stream = entry.Open();
        return XElement.Load(stream);
    }
}
