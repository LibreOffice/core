using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The <c>iconSet</c> family, most of whose corpus rules are stated where a reader is least
/// likely to look and paint nothing in most of their own buckets.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Every expectation here was read out of 26.2.4.2's own rendering of the same fixture.</strong>
/// Each of the four was converted to PDF and the embedded 16 × 16 images extracted with their
/// soft masks and identified by pixel hash against the reference's own <c>colibre</c> assets;
/// <c>probes/iconset-r103/fixture-reference/painted.txt</c> is the run and
/// <c>fixture-glyphs.txt</c> the identification. The corpus half is in
/// <c>probes/iconset-r103/results.md</c>.
/// </para>
/// <para>
/// Two things these fixtures exist to pin. <strong>Eighteen of the corpus's twenty rules have no
/// main-namespace <c>cfRule</c> at all</strong>, so
/// <c>sheet-cf-icon-set-x14-custom.xlsx</c> states none either; and
/// <strong><c>sheet-cf-icon-set-symbols.xlsx</c> states no optional attribute whatsoever</strong>
/// — no <c>showValue</c>, <c>reverse</c>, <c>custom</c>, <c>cfIcon</c>, <c>gte</c> or extension —
/// because an earlier round on this family shipped an inverted default and caught it only on
/// re-reading, its fixtures having all stated the attribute most real documents omit.
/// </para>
/// </remarks>
public sealed class XlsxIconSetTests
{
    [Fact]
    public void TheBucketIsTheLastThresholdTheValueSatisfies()
    {
        // `sheet-cf-icon-set-symbols.xlsx`: `3Symbols` over 0, 25, 50, 75, 100 with `percent`
        // stops at 0, 33 and 67 and nothing else stated. 26.2.4.2's PDF draws five 11.00 × 11.00
        // images, in order `symbols1-cross`, `symbols1-cross`, `symbols1-exclamation-mark`,
        // `symbols1-check`, `symbols1-check` — so 50 takes the *second* bucket and not the first
        // one it satisfies, which is `GetIconSetInfo` keeping the highest matching index rather
        // than breaking out (`sc/source/core/data/colorscale.cxx`:1205-1216).
        SheetFormatting formatting = Read("sheet-cf-icon-set-symbols.xlsx");

        Glyph(formatting, 0).ShouldBe(SheetIconGlyph.CrossInRed);
        Glyph(formatting, 1).ShouldBe(SheetIconGlyph.CrossInRed);
        Glyph(formatting, 2).ShouldBe(SheetIconGlyph.ExclamationInAmber);
        Glyph(formatting, 3).ShouldBe(SheetIconGlyph.TickInGreen);
        Glyph(formatting, 4).ShouldBe(SheetIconGlyph.TickInGreen);

        // `showValue` defaults to true, and the reference draws all five numbers beside the icons.
        for (int row = 0; row < 5; row++)
        {
            formatting.IconAt(row, 0)!.Value.ShowValue.ShouldBeTrue();
            formatting.HidesValue(row, 0).ShouldBeFalse();
        }
    }

    [Fact]
    public void AnExtensionOnlyRuleIsARuleAndItsNoIconsBucketKeepsItsText()
    {
        // `sheet-cf-icon-set-x14-custom.xlsx` has **no `conditionalFormatting` element at all**:
        // its whole rule is an `x14:cfRule` in the worksheet's `extLst`, carrying its own
        // `xm:sqref`. 26.2.4.2 imports and paints it anyway — "an ext entry does not need to have
        // an existing corresponding entry", `sc/source/filter/oox/extlstcontext.cxx`:165-194 —
        // and its PDF holds **four** images over the five cells, two `shapes-diamond` and two
        // `flags-red`, plus exactly one text span, the `-1` of the cell whose bucket is
        // `NoIcons`.
        SheetFormatting formatting = Read("sheet-cf-icon-set-x14-custom.xlsx");

        // Bucket 0 is `NoIcons`: no icon information at all, so the number survives a
        // `showValue="0"` that removes every other one (`colorscale.cxx`:1236-1239).
        formatting.IconAt(0, 0).ShouldBeNull();
        formatting.HidesValue(0, 0).ShouldBeFalse();

        Glyph(formatting, 1).ShouldBe(SheetIconGlyph.Diamond);
        Glyph(formatting, 2).ShouldBe(SheetIconGlyph.Diamond);
        Glyph(formatting, 3).ShouldBe(SheetIconGlyph.FlagRed);
        Glyph(formatting, 4).ShouldBe(SheetIconGlyph.FlagRed);

        for (int row = 1; row < 5; row++)
        {
            formatting.IconAt(row, 0)!.Value.ShowValue.ShouldBeFalse();
            formatting.HidesValue(row, 0).ShouldBeTrue();
        }
    }

    [Fact]
    public void ReverseReflectsTheBucketAndGteMakesTheComparisonStrict()
    {
        // `sheet-cf-icon-set-reverse.xlsx`: `3Flags reverse="1"` over 0, 5, 7, 10, 12 with stops
        // at `percent 0`, `num 5 gte="0"` and `num 10`. 26.2.4.2's PDF draws `flags-green`,
        // `flags-green`, `flags-yellow`, `flags-red`, `flags-red`.
        //
        // Both halves are load-bearing and each would be invisible without the other. The second
        // cell holds exactly 5 and takes the *first* bucket, so its stop compares with `>` and not
        // `>=` — `SetCfvoData` moves the entry to `ScConditionMode::Greater` only when a stated
        // `gte` is false (`sc/source/filter/oox/condformatbuffer.cxx`:117-122). And the order is
        // inverted throughout, because `reverse` reflects the index across the entries *after*
        // the search (`sc/source/core/data/colorscale.cxx`:1226-1230).
        SheetFormatting formatting = Read("sheet-cf-icon-set-reverse.xlsx");

        Glyph(formatting, 0).ShouldBe(SheetIconGlyph.FlagGreen);
        Glyph(formatting, 1).ShouldBe(SheetIconGlyph.FlagGreen);
        Glyph(formatting, 2).ShouldBe(SheetIconGlyph.FlagAmber);
        Glyph(formatting, 3).ShouldBe(SheetIconGlyph.FlagRed);
        Glyph(formatting, 4).ShouldBe(SheetIconGlyph.FlagRed);
    }

    [Fact]
    public void AThresholdThatNamesAnotherCellIsResolvedFromIt()
    {
        // `sheet-cf-icon-set-formula.xlsx`: `3Flags` over 1, 2, 3, 4, 5 with stops at `percent 0`
        // and two `num` entries whose `xm:f` is `$A$7`, the second `gte="0"`; A7 holds 3. An
        // `x14:cfvo` whose text does not parse as a number is a formula entry whatever its `type`
        // attribute says — `importFormula` keeps only a parsable one as a value
        // (`sc/source/filter/oox/condformatbuffer.cxx`:424-435) and `ConvertToModel` then
        // overrides the type with `COLORSCALE_FORMULA` (`:329-333`).
        //
        // 26.2.4.2's PDF draws `flags-red`, `flags-red`, `flags-yellow`, `flags-green`,
        // `flags-green`. **An unresolved reference would read as zero**, every value would clear
        // every stop, and all five would take the last bucket — so the fixture discriminates. The
        // corpus's only formula thresholds are the same shape, `$C$11` and `$D$11` in
        // `069_Blue_modern_balance_sheet`, whose two icons are its whole icon ink.
        SheetFormatting formatting = Read("sheet-cf-icon-set-formula.xlsx");

        Glyph(formatting, 0).ShouldBe(SheetIconGlyph.FlagRed);
        Glyph(formatting, 1).ShouldBe(SheetIconGlyph.FlagRed);
        Glyph(formatting, 2).ShouldBe(SheetIconGlyph.FlagAmber);
        Glyph(formatting, 3).ShouldBe(SheetIconGlyph.FlagGreen);
        Glyph(formatting, 4).ShouldBe(SheetIconGlyph.FlagGreen);
    }

    private static SheetIconGlyph Glyph(SheetFormatting formatting, int row)
        => formatting.IconAt(row, 0)!.Value.Glyph;

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
