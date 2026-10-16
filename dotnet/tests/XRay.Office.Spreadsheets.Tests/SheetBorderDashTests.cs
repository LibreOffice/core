using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What dash array a patterned cell border is stroked with: an absolute table, scaled by the
/// sheet's print scale and by nothing else.
/// </summary>
/// <remarks>
/// <para>
/// **Every number below is read out of LibreOffice 26.2.4.2's own PDF**, not out of the C++
/// tree, which is 27.2.0.0.alpha0+ here and not the reference binary's source. The fixtures are
/// round 115's — one border style per cell, thirteen cells, differing only in the
/// <c>pageSetup/@scale</c> the sheet states.
/// </para>
/// <para>
/// The seat is O59. A border's dashing is <c>svtools::GetLineDashing(type, PatternScale() × 10)</c>
/// over a table of small integers, and a Calc cell's pattern scale is the twips-to-1/100 mm
/// factor — so one table unit is ten twips, half a point, and has nothing to do with how thick
/// the line is. This tree derived the array from the line's own width, which made a <c>medium</c>
/// dotted border's dots three and a half times the reference's.
/// </para>
/// </remarks>
public sealed class SheetBorderDashTests
{
    private static IReadOnlyList<DrawnStroke> Strokes(string fixture)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(fixture));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return [.. sink.Pages.SelectMany(page => page.StrokedPaths)];
    }

    /// <summary>Every distinct dash array on the fixture, in points.</summary>
    private static IReadOnlyList<double[]> Dashes(string fixture)
        => [.. Strokes(fixture)
            .Where(stroke => stroke.Stroke.DashPattern is { Count: > 0 })
            .Select(stroke => stroke.Stroke.DashPattern!
                .Select(length => Math.Round(length.Points, 3)).ToArray())
            .Distinct(new SequenceComparer())
            .OrderBy(dash => dash.Length)
            .ThenBy(dash => dash[0])];

    private sealed class SequenceComparer : IEqualityComparer<double[]>
    {
        public bool Equals(double[]? a, double[]? b) => a is not null && b is not null && a.SequenceEqual(b);

        public int GetHashCode(double[] value)
        {
            HashCode hash = new();
            foreach (double item in value) hash.Add(item);
            return hash.ToHashCode();
        }
    }

    private static bool Matches(double[] dash, params double[] expected)
        => dash.Length == expected.Length
           && dash.Zip(expected).All(pair => Math.Abs(pair.First - pair.Second) < 0.01);

    [Fact]
    public void TheFivePatternsAreTheirAbsoluteTables()
    {
        // 26.2.4.2 on the thirteen-style fixture: [0.49999 0.99998] dotted, [2.99995 0.99998]
        // fine-dashed, [7.99987 2.49996] dashed, and the dash-dot pair adding 2.5/2.5 terms to
        // the last. Ten twips a unit, over GetDashing's 1/2, 6/2, 16/5, 16/5/5/5 and
        // 16/5/5/5/5/5.
        IReadOnlyList<double[]> dashes = Dashes("sheet-border-widths.xlsx");

        dashes.ShouldContain(dash => Matches(dash, 0.5, 1.0));
        dashes.ShouldContain(dash => Matches(dash, 3.0, 1.0));
        dashes.ShouldContain(dash => Matches(dash, 8.0, 2.5));
        dashes.ShouldContain(dash => Matches(dash, 8.0, 2.5, 2.5, 2.5));
        dashes.ShouldContain(dash => Matches(dash, 8.0, 2.5, 2.5, 2.5, 2.5, 2.5));
    }

    [Fact]
    public void AThinAndAMediumDottedBorderDrawTheSameArray()
    {
        // The whole of O59: the array does not scale with the width. The fixture holds a
        // `dotted` (15 twips) and a `mediumDashDot`-family set (35 twips), and 26.2.4.2 draws
        // one [0.49999 0.99998] and one [7.99987 …] — never a [0.75 0.75] or a [1.75 1.75].
        IReadOnlyList<double[]> dashes = Dashes("sheet-border-widths.xlsx");

        dashes.Count(dash => Matches(dash, 0.5, 1.0)).ShouldBe(1);
        dashes.ShouldNotContain(dash => Matches(dash, 0.75, 0.75));
        dashes.ShouldNotContain(dash => Matches(dash, 1.75, 1.75));
    }

    [Fact]
    public void EveryDashTakesThePrintScale()
    {
        // The same thirteen cells at `pageSetup scale="42"`. 26.2.4.2 writes [0.21001 0.42002]
        // for the dotted rule and scales the other four by the same 0.42.
        IReadOnlyList<double[]> dashes = Dashes("sheet-border-widths-scaled.xlsx");

        dashes.ShouldContain(dash => Matches(dash, 0.21, 0.42));
        dashes.ShouldContain(dash => Matches(dash, 1.26, 0.42));
        dashes.ShouldContain(dash => Matches(dash, 3.36, 1.05));
        dashes.ShouldNotContain(dash => Matches(dash, 0.5, 1.0));
    }

    [Fact]
    public void ABiffPatternedBorderDrawsTheSameArrays()
    {
        // The dash table is svtools' and is reached from the frame-border primitive, so it is
        // downstream of which filter read the file. 26.2.4.2's own .xls of the fixture draws the
        // identical arrays.
        IReadOnlyList<double[]> dashes = Dashes("sheet-border-widths.xls");

        dashes.ShouldContain(dash => Matches(dash, 0.5, 1.0));
        dashes.ShouldContain(dash => Matches(dash, 3.0, 1.0));
        dashes.ShouldContain(dash => Matches(dash, 8.0, 2.5));
    }
}
