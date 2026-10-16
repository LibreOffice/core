using System.Globalization;

using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// How thick 26.2.4.2 draws an underline, a double underline and a strikethrough.
/// </summary>
/// <remarks>
/// <para>
/// <b>Every expectation here was read off the reference's own PDF</b>, not computed from the rule
/// under test. <c>probes/quantise-r120/</c> authors one line per (face, size, kind) three times
/// over — as <c>.fodt</c>, <c>.fods</c> and <c>.fodp</c>, so the module is a variable — renders
/// each with 26.2.4.2 twice (C11) and reads the drawn thickness back out of the content stream.
/// The two tables below are those measurements: 270 rules from Calc in whole hundredths of a
/// millimetre and 270 from Writer in whole twips. The Impress leg agrees with the Calc one on all
/// 270 and is not repeated.
/// </para>
/// <para>
/// The rule that reproduces them is <see cref="LineSpacing.ResolveRuleWidths(OpenTypeFace,
/// LineMetrics, Length, MetricGrid)"/>, and what makes it a rule rather than a fit is that it has
/// no free parameter and covers both branches of <c>FontMetricData::ImplInitTextLineSize</c>: the
/// three Liberation faces take the descent branch (Mono's descent trips #i55341's clamp), and
/// Carlito, Caladea and DejaVu Sans take the HarfBuzz one.
/// </para>
/// <para>
/// A face's own numbers are read from the installed file rather than restated, because the point
/// of the table is the <em>quantisation</em> and a restated metric would let a wrong one pass.
/// </para>
/// </remarks>
public class RuleWidthTests
{
    /// <summary>Measured off 26.2.4.2's PDF of `sweep-fods.fods`: whole hundredths of a millimetre.</summary>
    /// <remarks>Each cell is <c>size=single/double/strikethrough</c>.</remarks>
    private static readonly string[] Calc =
    [
        "Liberation Sans: 6=11/7/11 7=14/7/14 8=14/11/14 9=18/11/18 10=18/11/18 11=21/14/21 12=21/14/21 14=28/18/28 16=32/18/32 18=35/21/35 20=39/25/39 24=46/28/46 28=53/32/53 36=67/42/67 48=92/56/92",
        "Liberation Serif: 6=11/7/11 7=14/7/14 8=14/11/14 9=18/11/18 10=21/14/21 11=21/14/21 12=25/14/25 14=28/18/28 16=32/21/32 18=35/21/35 20=39/25/39 24=46/28/46 28=53/35/53 36=71/42/71 48=92/60/92",
        "Liberation Mono: 6=14/11/14 7=18/11/18 8=21/14/21 9=21/14/21 10=25/14/25 11=28/18/28 12=28/18/28 14=35/21/35 16=39/25/39 18=46/28/46 20=49/32/49 24=60/39/60 28=67/42/67 36=88/56/88 48=116/74/116",
        "Carlito: 6=21/14/14 7=25/18/18 8=28/21/21 9=32/21/21 10=35/25/25 11=39/25/28 12=42/28/28 14=49/32/35 16=56/39/39 18=64/42/42 20=67/46/49 24=81/56/56 28=95/64/67 36=123/81/85 48=162/109/113",
        "Caladea: 6=11/7/11 7=14/11/14 8=14/11/14 9=18/11/18 10=18/14/18 11=21/14/21 12=21/14/21 14=28/18/28 16=28/21/28 18=32/21/32 20=35/25/35 24=42/28/42 28=53/35/53 36=64/42/64 48=85/56/85",
        "DejaVu Sans: 6=11/7/11 7=14/11/14 8=14/11/14 9=14/11/18 10=18/11/18 11=18/14/21 12=21/14/21 14=25/18/25 16=28/18/28 18=28/21/32 20=32/21/35 24=39/28/42 28=46/32/49 36=56/39/64 48=78/53/85",
    ];

    /// <summary>The same grid measured off `sweep-fodt.fodt`: whole twips.</summary>
    /// <remarks>
    /// A twip is exactly two of the writer's 720 dpi pixels, so every one of these is even and a
    /// Writer rule is a whole tenth of a point. That the unit differs from Calc's is the half of
    /// this the source does not say — see <see cref="MetricGrid.TextLine"/>.
    /// </remarks>
    private static readonly string[] Writer =
    [
        "Liberation Sans: 6=6/4/6 7=8/4/8 8=8/6/8 9=10/6/10 10=10/6/10 11=12/8/12 12=12/8/12 14=16/10/16 16=18/10/18 18=20/12/20 20=22/14/22 24=26/16/26 28=30/18/30 36=38/24/38 48=52/32/52",
        "Liberation Serif: 6=6/4/6 7=8/4/8 8=8/6/8 9=10/6/10 10=12/8/12 11=12/8/12 12=14/8/14 14=16/10/16 16=18/12/18 18=20/12/20 20=22/14/22 24=26/16/26 28=30/20/30 36=40/24/40 48=52/34/52",
        "Liberation Mono: 6=8/6/8 7=10/6/10 8=12/8/12 9=12/8/12 10=14/8/14 11=16/10/16 12=16/10/16 14=20/12/20 16=22/14/22 18=26/16/26 20=28/18/28 24=34/22/34 28=38/24/38 36=50/32/50 48=66/42/66",
        "Carlito: 6=12/8/8 7=14/10/10 8=16/12/12 9=18/12/12 10=20/14/14 11=22/14/16 12=24/16/16 14=28/18/20 16=32/22/22 18=36/24/24 20=38/26/28 24=46/32/32 28=54/36/38 36=70/46/48 48=92/62/64",
        "Caladea: 6=6/4/6 7=8/6/8 8=8/6/8 9=10/6/10 10=10/8/10 11=12/8/12 12=12/8/12 14=16/10/16 16=16/12/16 18=18/12/18 20=20/14/20 24=24/16/24 28=30/20/30 36=36/24/36 48=48/32/48",
        "DejaVu Sans: 6=6/4/6 7=8/6/8 8=8/6/8 9=8/6/10 10=10/6/10 11=10/8/12 12=12/8/12 14=14/10/14 16=16/10/16 18=16/12/18 20=18/12/20 24=22/16/24 28=26/18/28 36=32/22/36 48=44/30/48",
    ];

    /// <summary>
    /// Where the reference puts each rule, measured off its own PDFs of <c>probes/dblunder-r123</c>'s
    /// position probe. Each cell is <c>size=underline/double1,double2/strikethrough</c>, in whole
    /// hundredths of a millimetre below the baseline, and each number is the STROKE CENTRE — which
    /// is what a PDF segment states and what this reads back.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The offsets are a separate measurement from the thicknesses above and they were not taken
    /// until round 123. Round 120 quantised the thickness and left the offset a fraction of the em,
    /// on the strength of a <b>0.02 pt</b> agreement measured on one document's strikethroughs; over
    /// these 126 rules a single underline was <b>0.067 pt</b> out on average and <b>0.139 pt</b> at
    /// worst, so the figure that justified skipping it was not representative of the class.
    /// </para>
    /// <para>
    /// The sizes here are seven rather than the fifteen above, and they reach down to 6 pt on
    /// purpose: the descent branch floors the gap between a double underline's two lines at
    /// <c>1 + DPIY/150</c> device pixels, which is <b>five</b> at 720 dpi, and that floor binds only
    /// where the double underline is thinner than five pixels. A probe of large text cannot see the
    /// term at all.
    /// </para>
    /// </remarks>
    private static readonly string[] CalcOffsets =
    [
        "Liberation Sans: 6=25/14,46/-53 8=32/18,57/-74 10=39/25,64/-92 12=46/32,78/-109 16=64/46,99/-148 24=92/64,148/-222 48=183/127,296/-445",
        "Liberation Serif: 6=25/14,46/-53 8=32/18,57/-74 10=42/28,74/-92 12=49/35,81/-109 16=64/42,106/-145 24=95/67,151/-219 48=187/127,307/-441",
        "Liberation Mono: 6=35/21,60/-49 8=46/32,78/-64 10=56/42,88/-81 12=67/49,103/-99 16=88/64,138/-131 24=131/92,208/-198 48=258/183,406/-395",
        "Carlito: 6=25/11,53/-46 8=32/14,77/-60 10=39/14,89/-78 12=46/18,102/-92 16=60/21,138/-123 24=85/32,201/-183 48=169/60,388/-367",
        "Caladea: 6=21/14,35/-53 8=28/18,50/-71 10=35/25,67/-92 12=42/28,70/-106 16=56/39,102/-141 24=85/56,141/-215 48=169/113,282/-430",
        "DejaVu Sans: 6=11/4,25/-49 8=14/4,36/-64 10=14/4,36/-81 12=21/7,49/-99 16=28/7,60/-131 24=35/14,99/-198 48=74/21,180/-395",
    ];

    /// <summary>The same 126 rules on a word-processing page, in whole twips.</summary>
    private static readonly string[] WriterOffsets =
    [
        "Liberation Sans: 6=14/8,26/-30 8=18/10,32/-42 10=22/14,36/-52 12=26/18,44/-62 16=36/26,56/-84 24=52/36,84/-126 48=104/72,168/-252",
        "Liberation Serif: 6=14/8,26/-30 8=18/10,32/-42 10=24/16,42/-52 12=28/20,46/-62 16=36/24,60/-82 24=54/38,86/-124 48=106/72,174/-250",
        "Liberation Mono: 6=20/12,34/-28 8=26/18,44/-36 10=32/24,50/-46 12=38/28,58/-56 16=50/36,78/-74 24=74/52,118/-112 48=146/104,230/-224",
        "Carlito: 6=14/6,30/-26 8=18/8,44/-34 10=22/8,50/-44 12=26/10,58/-52 16=34/12,78/-70 24=48/18,114/-104 48=96/34,220/-208",
        "Caladea: 6=12/8,20/-30 8=16/10,28/-40 10=20/14,38/-52 12=24/16,40/-60 16=32/22,58/-80 24=48/32,80/-122 48=96/64,160/-244",
        "DejaVu Sans: 6=6/2,14/-28 8=8/2,20/-36 10=8/2,20/-46 12=12/4,28/-56 16=16/4,34/-74 24=20/8,56/-112 48=42/12,102/-224",
    ];

    private static readonly Dictionary<string, string> Files = new()
    {
        ["Liberation Sans"] = "LiberationSans-Regular.ttf",
        ["Liberation Serif"] = "LiberationSerif-Regular.ttf",
        ["Liberation Mono"] = "LiberationMono-Regular.ttf",
        ["Carlito"] = "Carlito-Regular.ttf",
        ["Caladea"] = "Caladea-Regular.ttf",
        ["DejaVu Sans"] = "DejaVuSans.ttf",
    };

    private static readonly string[] SearchDirectories =
    [
        "/usr/share/fonts/truetype/crosextra",
        "/usr/share/fonts/truetype/liberation",
        "/usr/share/fonts/truetype/liberation2",
        "/usr/share/fonts/truetype/dejavu",
        "/usr/share/fonts",
    ];

    private static string? Find(string fileName)
    {
        foreach (string directory in SearchDirectories)
        {
            if (!Directory.Exists(directory)) continue;

            string direct = Path.Combine(directory, fileName);
            if (File.Exists(direct)) return direct;

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }
        return null;
    }

    private static OpenTypeFace Require(string family)
    {
        string? path = Find(Files[family]);
        Assert.SkipWhen(path is null, $"{Files[family]} is not installed; see check-env.sh");

        OpenTypeFace? face = OpenTypeFace.ReadFile(path!);
        face.ShouldNotBeNull($"{path} should be readable as a font");
        return face!;
    }

    private static IEnumerable<(string Family, int Size, int[] Units)> Table(string[] rows)
    {
        foreach (string row in rows)
        {
            string[] halves = row.Split(": ", 2);
            foreach (string cell in halves[1].Split(' '))
            {
                string[] sides = cell.Split('=');
                yield return (halves[0],
                              int.Parse(sides[0], CultureInfo.InvariantCulture),
                              sides[1].Split('/', ',')
                                      .Select(v => int.Parse(v, CultureInfo.InvariantCulture))
                                      .ToArray());
            }
        }
    }

    private static void Check(string[] rows, MetricGrid grid, Func<long, Length> unit)
    {
        int cases = 0;

        foreach ((string family, int size, int[] want) in Table(rows))
        {
            OpenTypeFace face = Require(family);
            LineSpacing.RuleWidths got = LineSpacing.ResolveRuleWidths(
                face, LineSpacing.Resolve(face), Length.FromPoints(size), grid);

            string where = $"{family} at {size} pt";
            got.Underline.ShouldBe(unit(want[0]), $"{where}, single underline");
            got.DoubleUnderline.ShouldBe(unit(want[1]), $"{where}, double underline");
            got.Strikeout.ShouldBe(unit(want[2]), $"{where}, strikethrough");
            cases++;
        }

        cases.ShouldBe(90, "six faces at fifteen sizes");
    }

    /// <summary>
    /// The same shape of check on the OFFSETS, comparing stroke centres.
    /// </summary>
    /// <remarks>
    /// This model fills a rule where the reference strokes one (C16), so what it holds is a top
    /// edge; half the drawn thickness turns that back into the centre a PDF segment states. The
    /// half is a half <em>logical unit</em> and not a whole one, which is why the comparison is on
    /// twice the value rather than on the value.
    /// </remarks>
    private static void CheckOffsets(string[] rows, MetricGrid grid, Func<long, Length> unit)
    {
        int cases = 0;

        foreach ((string family, int size, int[] want) in Table(rows))
        {
            OpenTypeFace face = Require(family);
            LineSpacing.RuleWidths got = LineSpacing.ResolveRuleWidths(
                face, LineSpacing.Resolve(face), Length.FromPoints(size), grid);

            string where = $"{family} at {size} pt";

            Centre(got.UnderlineOffset, got.Underline).ShouldBe(unit(want[0]), $"{where}, underline");
            Centre(got.DoubleUnderlineFirst, got.DoubleUnderline)
                .ShouldBe(unit(want[1]), $"{where}, first line of a double underline");
            Centre(got.DoubleUnderlineSecond, got.DoubleUnderline)
                .ShouldBe(unit(want[2]), $"{where}, second line of a double underline");
            Centre(got.StrikeoutOffset, got.Strikeout)
                .ShouldBe(unit(want[3]), $"{where}, strikethrough");
            cases++;
        }

        cases.ShouldBe(42, "six faces at seven sizes");
    }

    private static Length Centre(Length top, Length thickness) => top + (thickness / 2);

    /// <summary>
    /// A BOLD underline is a third size, and both corpus witnesses are pinned here.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>((descent × 50) + 50) / 100</c> against the single's <c>× 25</c>
    /// (<c>vcl/source/font/fontmetric.cxx</c>:289), with a guard bumping it by one where the two
    /// come out equal (:290-291). The numbers below were read off 26.2.4.2's own banked renderings
    /// of the only two corpus documents that ask for one — `License App Instructions 2-22.docx`
    /// draws Liberation Serif at 12 pt with a 1.3 pt rule whose stroke centre is 1.4 pt below the
    /// baseline, and `OM template for non-complex NCC operators_August 2016.docx` draws Liberation
    /// Sans at 17 pt with a 1.8 pt rule centred at 1.9.
    /// </para>
    /// <para>
    /// Both are on a Writer page, so the unit is the twip. The single underline of the same face
    /// and size is 14 and 18 twips, so the bold is not a rounding away from it in either case.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("Liberation Serif", 12, 26, 28, 14)]
    [InlineData("Liberation Sans", 17, 36, 38, 18)]
    public void ABoldUnderlineIsItsOwnThicknessAndItsOwnOffset(
        string family, int size, int thickness, int centre, int ordinary)
    {
        OpenTypeFace face = Require(family);
        LineSpacing.RuleWidths got = LineSpacing.ResolveRuleWidths(
            face, LineSpacing.Resolve(face), Length.FromPoints(size), MetricGrid.WriterTextLine);

        got.BoldUnderline.ShouldBe(Length.FromTwips(thickness));
        Centre(got.BoldUnderlineOffset, got.BoldUnderline).ShouldBe(Length.FromTwips(centre));
        got.Underline.ShouldBe(Length.FromTwips(ordinary));
    }

    /// <summary>Every measured Calc offset, in the whole hundredth of a millimetre it was drawn at.</summary>
    [Fact]
    public void EveryMeasuredCalcRuleSitsWhereTheDeviceChainPutsIt()
        => CheckOffsets(CalcOffsets, MetricGrid.TextLine, u => Length.FromMm100(u));

    /// <summary>And the same on a word-processing page, where the unit is the twip.</summary>
    [Fact]
    public void EveryMeasuredWriterRuleSitsWhereTheDeviceChainPutsIt()
        => CheckOffsets(WriterOffsets, MetricGrid.WriterTextLine, u => Length.FromTwips(u));

    /// <summary>
    /// Every one of the 90 (face, size) pairs Calc draws, in the whole hundredth of a millimetre
    /// the reference drew it at.
    /// </summary>
    [Fact]
    public void EveryMeasuredCalcRuleIsAWholeHundredthOfAMillimetreAndTheRuleFindsIt()
        => Check(Calc, MetricGrid.TextLine, u => Length.FromMm100(u));

    /// <summary>
    /// The same 90 pairs on a word-processing page, where the map unit is the twip and the answers
    /// are therefore different numbers of the same 720 dpi pixels.
    /// </summary>
    [Fact]
    public void EveryMeasuredWriterRuleIsAWholeTwipAndTheRuleFindsIt()
        => Check(Writer, MetricGrid.WriterTextLine, u => Length.FromTwips(u));

    /// <summary>
    /// The two grids really do disagree, so the test above is not passing on a coincidence.
    /// </summary>
    /// <remarks>
    /// 21 hundredths of a millimetre is 0.5953 pt and 12 twips is 0.6 pt: the same six pixels
    /// rounded onto two units. If the words path took Calc's grid this would be the error it made.
    /// </remarks>
    [Fact]
    public void TheSamePixelCountComesOutDifferentlyOnTheTwoGrids()
    {
        OpenTypeFace face = Require("Liberation Sans");
        LineMetrics line = LineSpacing.Resolve(face);
        Length size = Length.FromPoints(12);

        LineSpacing.ResolveRuleWidths(face, line, size, MetricGrid.TextLine)
                   .Underline.ShouldBe(Length.FromMm100(21));
        LineSpacing.ResolveRuleWidths(face, line, size, MetricGrid.WriterTextLine)
                   .Underline.ShouldBe(Length.FromTwips(12));
    }

    /// <summary>
    /// The seven pairs O64 was opened on, restated as the (face, size, kind) they came from.
    /// </summary>
    /// <remarks>
    /// Round 118 measured seven thicknesses over four spreadsheets and recorded them as 0.51,
    /// 0.595, 0.51, 0.595, 1.389, 0.794 and 0.397 pt, which are 18, 21, 18, 21, 49, 28 and 14
    /// hundredths of a millimetre. Four of the seven are the two rows above; the two Carlito ones
    /// and the double are the sizes those documents happen to use, so they are asserted here in
    /// the form the register states them.
    /// </remarks>
    [Theory]
    [InlineData("Liberation Sans", 10, "single", 18)]   // the invoice's Arial hyperlinks
    [InlineData("Liberation Sans", 12, "single", 21)]   // and the fixture's
    [InlineData("Carlito", 14, "single", 49)]           // 002_Contextures' Calibri 14 `<u/>` cells
    [InlineData("Carlito", 11, "strike", 28)]           // TK-Syllabus' Calibri 11 strikethroughs
    [InlineData("Liberation Sans", 12, "double", 14)]   // the fixture's double underline
    public void TheSevenPairsO64WasOpenedOnAreReproduced(
        string family, int size, string kind, int hundredths)
    {
        OpenTypeFace face = Require(family);
        LineSpacing.RuleWidths widths = LineSpacing.ResolveRuleWidths(
            face, LineSpacing.Resolve(face), Length.FromPoints(size), MetricGrid.TextLine);

        Length got = kind switch
        {
            "single" => widths.Underline,
            "double" => widths.DoubleUnderline,
            _ => widths.Strikeout,
        };

        got.ShouldBe(Length.FromMm100(hundredths));
    }

    /// <summary>
    /// A double underline is thinner than a single one, on both branches.
    /// </summary>
    /// <remarks>
    /// The arm a design-unit model cannot have at all, because it drew both at one thickness:
    /// <c>mnDUnderlineSize</c> is two thirds of the single size on the HarfBuzz branch and
    /// <c>((descent × 16) + 50) / 100</c> against the single rule's 25 on the other.
    /// </remarks>
    [Theory]
    [InlineData("Liberation Sans")]
    [InlineData("Carlito")]
    public void ADoubleUnderlineIsThinnerThanASingleOne(string family)
    {
        OpenTypeFace face = Require(family);
        LineSpacing.RuleWidths widths = LineSpacing.ResolveRuleWidths(
            face, LineSpacing.Resolve(face), Length.FromPoints(12), MetricGrid.TextLine);

        widths.DoubleUnderline.ShouldBeLessThan(widths.Underline);
    }

    /// <summary>
    /// The thickness is a staircase in the size and not a fraction of the em.
    /// </summary>
    /// <remarks>
    /// The property that separates this from every design-unit answer, and the reason O64 could
    /// not be closed by a scale factor: the reference draws 10 pt and 11 pt Liberation Sans at
    /// 18 and 21 hundredths of a millimetre, a 17 % step for a 10 % rise, and 9 pt and 10 pt at
    /// the same 18. A model proportional to the size cannot produce a repeat and a jump.
    /// </remarks>
    [Fact]
    public void TheThicknessStepsRatherThanScaling()
    {
        OpenTypeFace face = Require("Liberation Sans");
        LineMetrics line = LineSpacing.Resolve(face);

        Length At(double points) => LineSpacing
            .ResolveRuleWidths(face, line, Length.FromPoints(points), MetricGrid.TextLine)
            .Underline;

        At(9).ShouldBe(At(10));
        At(11).ShouldBeGreaterThan(At(10));
        At(11).ShouldBe(At(12));
    }

    /// <summary>
    /// A page drawn through a transform quantises its rule BEFORE the multiply, and the witness is
    /// <c>RMP 2011-2014 and Inventory.xls</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// O70. The workbook prints at Calc's own 60% and states a 10 pt Arial, so the drawn em is
    /// <c>353 × 0.6</c> hundredths of a millimetre — 6.004 pt, which is the size 26.2.4.2's PDF
    /// states — the device sets 60 pixels for it, and the descent branch answers <b>3</b> pixels of
    /// thickness at <b>7</b> of depth to the stroke's centre. Those two pixel counts are the same
    /// on both sides of this test; what differs is which side of the transform they are converted
    /// on.
    /// </para>
    /// <para>
    /// Quantising after it gives <c>round(3 × 2540/720) = 11</c> hundredths and
    /// <c>round(7 × 2540/720) = 25</c>, which are 0.3118 pt and 0.7086 — what this tree drew until
    /// this round, and both are asserted below so that the two answers are visibly different
    /// numbers rather than a tolerance apart. Quantising before it gives <c>18 × 0.6 = 10.8</c> and
    /// <c>41 × 0.6 = 24.6</c>, which are 0.30614 pt and 0.69732; <c>appendMappedLength</c> writes
    /// thousandths of a point, so the reference's own PDF says <b>0.306</b> and <b>0.697</b>, and
    /// that is what was read off it.
    /// </para>
    /// <para>
    /// Over the 31 corpus documents holding a scaled rule this moves 1418 paired reference rules
    /// from 544 agreeing to 0.0015 pt to <b>1412</b>. <c>probes/rulescale-r132/</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void AScaledPageQuantisesItsRuleBeforeTheTransformAndNotAfterIt()
    {
        OpenTypeFace face = Require("Liberation Sans");
        LineMetrics line = LineSpacing.Resolve(face);

        // What the Calc chain hands the rule metric for a 10 pt Arial at 60%: a whole hundredth of
        // a millimetre of the unscaled page, multiplied by the print zoom. See
        // `SheetDeviceUnits.SnapFontSize`.
        Length drawn = Length.FromMm100(353) * 0.6;

        LineSpacing.RuleWidths after = LineSpacing.ResolveRuleWidths(
            face, line, drawn, MetricGrid.TextLine.Scaled(0.6));

        after.Underline.ShouldBe(Length.FromMm100(18) * 0.6);
        Centre(after.UnderlineOffset, after.Underline).ShouldBe(Length.FromMm100(41) * 0.6);
        after.Underline.Points.ShouldBe(0.306, 0.0005);
        Centre(after.UnderlineOffset, after.Underline).Points.ShouldBe(0.697, 0.0005);

        // And the same call on an unscaled grid is the number this tree used to draw, so the two
        // are separated by the change and not by a tolerance.
        LineSpacing.RuleWidths before = LineSpacing.ResolveRuleWidths(
            face, line, drawn, MetricGrid.TextLine);

        before.Underline.ShouldBe(Length.FromMm100(11));
        Centre(before.UnderlineOffset, before.Underline).ShouldBe(Length.FromMm100(25));
    }

    /// <summary>
    /// A transform of one, and anything that is not a positive finite number, leaves the grid alone.
    /// </summary>
    /// <remarks>
    /// The guard that keeps the change confined to the pages it is about: an unscaled sheet, a
    /// slide and every word-processing page go through the identical arithmetic they did before,
    /// so the 15 062 of 15 103 corpus rules round 123 scored cannot move. A zero or a NaN reaching
    /// here would otherwise divide the whole page away.
    /// </remarks>
    [Theory]
    [InlineData(1.0)]
    [InlineData(0.0)]
    [InlineData(-0.5)]
    [InlineData(double.NaN)]
    [InlineData(double.PositiveInfinity)]
    public void AGridWithNoUsableTransformIsTheGridItself(double scale)
        => MetricGrid.TextLine.Scaled(scale).ShouldBe(MetricGrid.TextLine);

    /// <summary>
    /// A face LibreOffice will read the metrics of and one it will not answer differently at the
    /// same size, and the discriminator is the name alone.
    /// </summary>
    /// <remarks>
    /// The control on the branch, and it is worth having as a rule rather than as a table row:
    /// Liberation Sans' <c>post</c> would give <c>ceil(150 × 100 / 2048)</c> = 8 pixels at 10 pt,
    /// which is 28 hundredths of a millimetre, and the reference draws 18. Spelling the family
    /// anything else takes the other branch.
    /// </remarks>
    [Fact]
    public void TheBlacklistIsWhatChoosesTheBranchAndItIsCheckedByName()
    {
        OpenTypeFace face = Require("Liberation Sans");
        LineMetrics line = LineSpacing.Resolve(face);
        Length size = Length.FromPoints(10);

        LineSpacing.ResolveRuleWidths(
            "Liberation Sans", face.Post, face.Os2, line, size, MetricGrid.TextLine)
            .Underline.ShouldBe(Length.FromMm100(18));

        LineSpacing.ResolveRuleWidths(
            "Not Liberation Sans", face.Post, face.Os2, line, size, MetricGrid.TextLine)
            .Underline.ShouldBe(Length.FromMm100(28));
    }
}
