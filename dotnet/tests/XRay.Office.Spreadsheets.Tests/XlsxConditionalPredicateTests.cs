using System.IO.Compression;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The five <c>cfRule</c> families that are a predicate on the cell rather than a formula, and the
/// cell a multi-range <c>sqref</c>'s formula is written for.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Every expectation here was read out of 26.2.4.2's own rendering of the same fixture,
/// not out of the specification.</strong> Each fixture under <c>tests/corpus/features/</c> was
/// converted twice: <c>--convert-to fods</c> for the rule the reference thinks it imported, and
/// <c>--convert-to pdf</c> for which cells it then painted, read back as filled rectangles and
/// coloured text spans. `probes/cond-format-r96/results.md` records both.
/// </para>
/// <para>
/// Two of the five diverge from what the specification says, and both divergences are the
/// reference's:
/// </para>
/// <list type="bullet">
/// <item>
/// <description>
/// <c>containsBlanks</c> is not a mode at all. <c>CondFormatRule::finalizeImport</c>
/// (<c>sc/source/filter/oox/condformatbuffer.cxx</c>:860-866) replaces it with the formula
/// <c>LEN(TRIM(#B))=0</c>, so <strong>a cell holding only spaces is blank</strong> — the
/// reference paints <c>sheet-cf-blank-cells.xlsx</c>'s three-space A2 with the blank rule's red.
/// </description>
/// </item>
/// <item>
/// <description>
/// A multi-range <c>sqref</c>'s base cell is <c>ScRangeList::GetTopLeftCorner</c>, which orders
/// addresses <c>(tab, col, row)</c> (<c>sc/inc/address.hxx</c>:396) — the smallest column, ties
/// by row, and <strong>not</strong> the componentwise minimum Excel writes the formula against.
/// The two readings of <c>sheet-cf-multi-range-anchor.xlsx</c> paint opposite cells.
/// </description>
/// </item>
/// </list>
/// <para>
/// The remaining three are the reference's evaluation rather than its import:
/// <c>ScConditionEntry::IsValidStr</c> lowercases both sides of a text comparison
/// (<c>conditio.cxx</c>:1219-1229), the tail comparison of <c>EndsWith</c> is the same
/// transliteration (<c>:1208-1218</c>), and <c>FillCache</c> keys a duplicate on the lowercased
/// string or on the number, never on both, so <c>7</c> and <c>"7"</c> are not duplicates of each
/// other (<c>:804-859</c>).
/// </para>
/// </remarks>
public sealed class XlsxConditionalPredicateTests
{
    /// <summary>The red every fixture's first <c>dxf</c> paints, as its font colour.</summary>
    private const string Red = "#9C0006";

    /// <summary>And as its fill.</summary>
    private const string RedFill = "#FFC7CE";

    /// <summary>The second <c>dxf</c>'s font colour, where a fixture states two.</summary>
    private const string Blue = "#0000FF";

    /// <summary>And its fill.</summary>
    private const string BlueFill = "#CCE5FF";

    [Fact]
    public void ContainsTextIsCaseInsensitiveAndReachesInsideTheString()
    {
        // 26.2.4.2's own PDF of this fixture draws A1 `Open`, A2 `Reopened later`, A3 `OPEN` and
        // A6 `Rope` in #9C0006 over #FFC7CE, and A4 `Closed` and A5 `1924` plain. So the search
        // is not anchored, it ignores case, and a numeric cell is stringified rather than
        // skipped — `IsValid` searches `OUString::number(nArg)`, which for 1924 holds no `ope`.
        Painted painted = Read("sheet-cf-contains-text.xlsx");

        painted.Colour(0).ShouldBe(Red);
        painted.Colour(1).ShouldBe(Red);
        painted.Colour(2).ShouldBe(Red);
        painted.Colour(3).ShouldBeNull();
        painted.Colour(4).ShouldBeNull();
        painted.Colour(5).ShouldBe(Red);

        painted.Fill(0).ShouldBe(RedFill);
        painted.Fill(3).ShouldBeNull();
    }

    [Fact]
    public void EndsWithMatchesOnlyTheTailAndIgnoresCase()
    {
        // The reference paints A1 `Case Closed` and A3 `case CLOSED` and leaves A2
        // `Closed case` and A4 `no` plain — so the needle must sit at the end, a shorter cell
        // cannot match, and the comparison folds case exactly as `containsText` does.
        Painted painted = Read("sheet-cf-ends-with.xlsx");

        painted.Colour(0).ShouldBe(Red);
        painted.Colour(1).ShouldBeNull();
        painted.Colour(2).ShouldBe(Red);
        painted.Colour(3).ShouldBeNull();
    }

    [Fact]
    public void ACellHoldingOnlySpacesIsBlankAndANumberIsNot()
    {
        // The finding this fixture exists for. `containsBlanks` becomes `LEN(TRIM(#B))=0`, so the
        // reference paints A2 — three spaces — and A3 — empty — with the blank rule's red, and
        // gives A1 `text`, A4 `0` and A5 `x` the not-blank rule's blue. A reader testing "the
        // cell states nothing" gets A2 the wrong way round, and one testing "the cell has no
        // text" gets A4 wrong, because TRIM stringifies a number first.
        Painted painted = Read("sheet-cf-blank-cells.xlsx");

        painted.Colour(0).ShouldBe(Blue);
        painted.Colour(1).ShouldBe(Red);
        painted.Colour(2).ShouldBe(Red);
        painted.Colour(3).ShouldBe(Blue);
        painted.Colour(4).ShouldBe(Blue);
    }

    [Fact]
    public void ADuplicateIsKeyedOnTheLoweredStringOrOnTheNumberAndNeverOnBoth()
    {
        // The reference paints A1 `alpha`, A2 `alpha`, A3 `ALPHA`, A5 `7` and A6 `7`, and leaves
        // A4 `bravo` and A7 — the *string* `7` — plain. So the cache folds case, and the number
        // 7 and the text 7 are different keys: `FillCache` puts one in `maValues` and the other
        // in `maStrings` and `IsDuplicate` only ever consults the map its own cell belongs to.
        Painted painted = Read("sheet-cf-duplicate-values.xlsx");

        painted.Colour(0).ShouldBe(Red);
        painted.Colour(1).ShouldBe(Red);
        painted.Colour(2).ShouldBe(Red);
        painted.Colour(3).ShouldBeNull();
        painted.Colour(4).ShouldBe(Red);
        painted.Colour(5).ShouldBe(Red);
        painted.Colour(6).ShouldBeNull();
    }

    [Fact]
    public void AMultiRangeSqrefIsAnchoredOnItsSmallestColumnAndNotOnItsSmallestRow()
    {
        // `$B1="hit"` on `B3 A5`, whose componentwise minimum is A3 and whose top-left corner is
        // A5. The reference's `--convert-to fods` states `base-cell-address="Rules.A5"`, and its
        // PDF paints A5 and not B3 — with A5 as the base the shift is −4 rows, so A5 tests B1
        // (`hit`) and B3 tests row −1, which resolves to `#REF!`. Anchoring on A3 instead paints
        // exactly the other cell, so this assertion distinguishes the two readings rather than
        // merely exercising one.
        Painted painted = Read("sheet-cf-multi-range-anchor.xlsx");

        painted.Fill(4, column: 0).ShouldBe(RedFill);
        painted.Fill(2, column: 1).ShouldBeNull();
    }

    [Fact]
    public void AConditionComparesTheStoredStringAndNotTheDrawnOne()
    {
        // `sheet-cf-stored-vs-drawn.xlsx` puts a leading `_x0009_` on A1, A5 and A6 and on C1. The
        // tab is a control character the drawing layer elides, so 26.2.4.2 draws A1 as `alpha` and
        // A5/A6 as `bravo` — the same glyphs as their untabbed neighbours — while comparing them
        // as different strings: its PDF paints A2 and A3 (`alpha`) and A5/A6 (`\talpha`... rather
        // `\tbravo`) and leaves A1 and A4 plain, so the two tabbed `bravo`s are duplicates of each
        // other and of nothing else. `RichStringPortion::setText` decodes the `_xHHHH_` escapes
        // and stops (`sc/source/filter/oox/richstring.cxx`:60-63), so the control character is
        // still in the cell when `FillCache` keys on it.
        Painted painted = Read("sheet-cf-stored-vs-drawn.xlsx");

        painted.Fill(0).ShouldBeNull();
        painted.Fill(1).ShouldBe(RedFill);
        painted.Fill(2).ShouldBe(RedFill);
        painted.Fill(3).ShouldBeNull();
        painted.Fill(4).ShouldBe(RedFill);
        painted.Fill(5).ShouldBe(RedFill);

        // And the same string reaches the blank rule, whose replacement formula is
        // `LEN(TRIM(#B))=0`: `TRIM` takes spaces and not tabs, so the reference gives C1 — a lone
        // tab — the *not*-blank rule's blue and C2 — three spaces — the blank rule's red. A reader
        // comparing the drawn text sees C1 as empty and gets it exactly the wrong way round.
        painted.Fill(0, column: 2).ShouldBe(BlueFill);
        painted.Fill(1, column: 2).ShouldBe(RedFill);
        painted.Fill(2, column: 2).ShouldBe(BlueFill);
    }

    /// <summary>What one fixture's rules paint, by position.</summary>
    private sealed record Painted(
        SheetFormatting Formatting,
        Dictionary<(int Row, int Column), SheetConditionalText> Text)
    {
        /// <summary>The font colour a rule sets on one cell of column A, or null for none.</summary>
        /// <param name="row">The zero-based row.</param>
        /// <param name="column">The zero-based column, column A by default.</param>
        public string? Colour(int row, int column = 0)
            => Text.TryGetValue((row, column), out SheetConditionalText found)
               && found.Colour is { } colour
                ? colour.ToString()
                : null;

        /// <summary>The fill a rule sets on one cell, or null for none.</summary>
        /// <param name="row">The zero-based row.</param>
        /// <param name="column">The zero-based column, column A by default.</param>
        public string? Fill(int row, int column = 0)
            => Formatting.At(row, column).Background?.ToString();
    }

    /// <summary>
    /// Reads one fixture's conditional formats straight out of the package.
    /// </summary>
    /// <remarks>
    /// The three parts by name rather than through the workbook's relationships, because each
    /// fixture holds exactly one worksheet and the point of the file is the rule inside it.
    /// </remarks>
    private static Painted Read(string fixture)
    {
        using ZipArchive archive = ZipFile.OpenRead(Corpus.Require(fixture));

        XlsxSharedStrings shared = XlsxSharedStrings.Read(Part(archive, "xl/sharedStrings.xml"));
        SheetFormatting formatting = XlsxCellDecoration.Read(
            Part(archive, "xl/styles.xml"),
            null,
            Part(archive, "xl/worksheets/sheet1.xml"),
            shared,
            out Dictionary<(int Row, int Column), SheetConditionalText> text);

        return new Painted(formatting, text);
    }

    private static XElement? Part(ZipArchive archive, string name)
    {
        ZipArchiveEntry? entry = archive.GetEntry(name);
        if (entry is null) return null;

        using Stream stream = entry.Open();
        return XElement.Load(stream);
    }
}
