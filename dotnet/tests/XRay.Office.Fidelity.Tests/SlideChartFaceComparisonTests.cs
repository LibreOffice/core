using System.Text.RegularExpressions;
using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

/// <summary>
/// A chart's text is set in the theme's minor Latin face, with a stated <c>a:latin</c>
/// overriding it — not in a fixed default.
/// </summary>
/// <remarks>
/// <para>
/// <c>SlideChart</c> set every chart label in Liberation Sans for four rounds, on the evidence
/// that <c>pdffonts</c> reported Liberation Sans in LibreOffice's own PDF of
/// <c>chart-bar-deck.pptx</c>. That deck's chart states <c>&lt;a:latin typeface="Arial"/&gt;</c>
/// eleven times, and fontconfig substitutes Arial with Liberation Sans — so the measurement is
/// equally consistent with a fixed face and with the stated one, and the corpus cannot separate
/// them without also changing the theme, the sizes and the data at once.
/// </para>
/// <para>
/// So the two decks here are that deck with <em>one</em> thing moved. Both set the theme's minor
/// Latin face to <strong>Liberation Mono</strong>, because a monospace is the widest available
/// separation from either proportional candidate and it moves the plot area's left edge as well
/// as the embedded font list. <c>chart-face-theme-minor.pptx</c> then states no <c>a:latin</c> at
/// all and <c>chart-face-stated.pptx</c> states a literal <c>Liberation Serif</c>. LibreOffice
/// 24.2.7.2 embeds LiberationMono in the first and LiberationSerif in the second, which is the
/// pattern only "the theme's minor face, overridden by a stated one" predicts: a fixed face gives
/// two identical answers, and reading only a stated face leaves the first on a fallback.
/// </para>
/// <para>
/// <strong>The face assertion alone would be a metadata test</strong>, and a face that is merely
/// named right moves no ink — the round-nineteen <c>/BaseFont</c> finding is exactly that shape.
/// So the third case measures <em>one digit's advance</em>, taken as the gap between the pen of
/// the value axis' three-digit labels and the pen of its two-digit ones. They are right-aligned
/// on the same edge, so that gap is one digit and nothing else: 6.01 pt in ten-point Liberation
/// Mono against 5.55 in Liberation Sans.
/// </para>
/// <para>
/// <strong>Two more obvious quantities were measured first and both are the wrong thing to
/// assert.</strong> An absolute pen position carries the composition as well as the face: the
/// value axis' labels land 0.36 pt from the reference with this change and 0.96 pt without —
/// better — while the legend lands 2.49 pt out with it and 1.39 pt out without, because the
/// composition has a legend-reservation error of its own that the wrong face was partly
/// cancelling. And a word's ink width carries the two writers' show splitting: the reference
/// positions each digit of "100" separately, so poppler ends its box at the last glyph's ink and
/// reports 17.25 where our single show reports the full 18.03 advance. Neither difference is
/// about the face. The gap between two of the axis' own labels is.
/// </para>
/// </remarks>
// [reference moved 24.2.7.2 -> 26.2.4.2, corrected] `TheThemesFaceDecidesTheValueLabelsAdvances`
// asserted that the *reference's* digit advance is the design metric, and under 26.2.4.2 it is not: the
// face is still Liberation Mono — `pdffonts` reports one font, `BAAAAA+LiberationMono`, and
// `AChartUnstatedTakesTheThemesMinorFace` still passes — but the reference draws its digits 5.839 pt
// apart against 24.2.7.2's 6.010 and the face's own 6.004. It is not about the face.
//
// [2026-09-06] **Found, and it was ours.** A chart's text is not laid out by Impress: `chart2`'s view
// builds it as plain text shapes on the `VirtualDevice` that `DrawModelWrapper` creates from
// `Application::GetDefaultDevice()` with `MapUnit::Map100thMM`
// (`chart2/source/view/main/DrawModelWrapper.cxx`:88-99), and that device is **96 dpi**
// (`SvpSalGraphics::GetResolution`, `vcl/headless/svpgdi.cxx`:44). An `OutputDevice` instantiates a
// font at a whole number of device pixels, so a 10 pt label is laid out at **13** pixels rather than
// 13.333 and every advance in it is 2.5% narrow — 5.85 pt against the face's 6.004. `SlideChart` drew
// the design metric; it now goes through `MetricGrid.Chart`, which is the rule `SheetBandText.ChartShape`
// has applied to a workbook's charts since round 62 and which the slides and words tracks were left out
// of. `probes/chart-text-metafile/` establishes it: the drawn advance follows `round(px96)/px96` over
// twelve sizes in **both** reference binaries, residual at most 0.003, while the same string in an
// ordinary slide text box on the same slide of the same deck stays within 0.7% of the design metric at
// every one of them.
//
// So the earlier reading of this file — that 24.2.7.2 "sat on the design metric" and only 26.2.4.2
// departed — was wrong, and the way it was wrong is worth keeping. 24.2.7.2 quantises the advance the
// same way; what it *also* does is snap each glyph position to a whole 96 dpi pixel, so its gaps are 7
// or 8 pixels where 26.2.4.2's are a flat 7.79, and it right-aligns these labels on their *design*
// widths while drawing them from the device's narrower array. That inconsistency is what made
// `pen("80") - pen("100")` read 6.010 there. 26.2.4.2 uses one width for both, which is why the same
// difference reads 5.839 — and why ours, which also uses one width for both, now reads 5.839 too.
//
// The `TJ` adjustment of 16 at every inter-glyph position is that scale expressed in thousandths of an
// em: 600 x (1 - 13/13.342) = 15.4. It is not the text-advance divergence `CLAUDE.md`'s rule 3 used to
// record, and that rule stays withdrawn.

public sealed partial class SlideChartFaceComparisonTests : IDisposable
{
    /// <summary>
    /// One digit's advance in ten-point Liberation Mono, as <c>chart2</c>'s device measures it.
    /// </summary>
    /// <remarks>
    /// The face's own digit is 0.6009 em, or 6.004 pt at the 10.005 pt these labels are drawn at.
    /// A chart's 10 pt em is 13 whole pixels of a 96 dpi device rather than 13.342, so the advance
    /// it measures is <c>6.004 x 13 / 13.342</c> = <b>5.85</b> pt. Both reference binaries and this
    /// tree now draw it there; see the note above the class.
    /// </remarks>
    private const double MonospacedDigitAdvance = 5.85;

    /// <summary>The same digit's advance in ten-point Liberation Sans, on the same device.</summary>
    /// <remarks>
    /// The face this test exists to rule out — the fixed one <c>SlideChart</c> used for four rounds.
    /// 0.5560 em, 5.561 pt by the design metric and <b>5.42</b> through the same 13/13.342. The two
    /// candidates are still 0.43 pt apart, four times the tolerance below, because the device's
    /// scale is a property of the size and applies to either face equally: quantising the em cannot
    /// turn one face into the other.
    /// </remarks>
    private const double ProportionalDigitAdvance = 5.42;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("xray-office-chart-face").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    [Fact]
    public void AChartUnstatedTakesTheThemesMinorFace()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

        const string deck = "chart-face-theme-minor.pptx";

        Faces(Ours(deck)).ShouldContain("LiberationMono");
        Faces(_libreOffice.ConvertToPdf(Corpus.Require(deck), _workDirectory))
            .ShouldContain("LiberationMono");
    }

    [Fact]
    public void AChartStatingAFaceTakesTheStatedOneInstead()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

        const string deck = "chart-face-stated.pptx";

        List<string> ours = Faces(Ours(deck));
        ours.ShouldContain("LiberationSerif");
        ours.ShouldNotContain("LiberationMono");

        List<string> theirs = Faces(_libreOffice.ConvertToPdf(Corpus.Require(deck), _workDirectory));
        theirs.ShouldContain("LiberationSerif");
        theirs.ShouldNotContain("LiberationMono");
    }

    /// <summary>
    /// The face is what the labels are <em>measured</em> in, not only what they are named in.
    /// </summary>
    [Fact]
    public void TheThemesFaceDecidesTheValueLabelsAdvances()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);
        Assert.SkipUnless(PdfWords.IsAvailable, "pdftotext is not installed");

        const string deck = "chart-face-theme-minor.pptx";

        double ours = DigitAdvance(PdfWords.Read(Ours(deck)));
        double theirs = DigitAdvance(
            PdfWords.Read(_libreOffice.ConvertToPdf(Corpus.Require(deck), _workDirectory)));

        // Ours against the literal first, so this tests X-Ray.Office rather than an agreement.
        ours.ShouldBe(MonospacedDigitAdvance, 0.1, "our digit advance");

        // And the reference's against the two candidates rather than against a literal, because the
        // two reference binaries do not agree with each other here and neither is wrong about the
        // face. 26.2.4.2 draws 5.839 — the device's advance, which is what we now draw. 24.2.7.2
        // draws 6.010, because it right-aligns these labels on their design widths while drawing
        // them from the device's narrower array; the face is the same in both, `pdffonts` reporting
        // the single font `BAAAAA+LiberationMono` for each, and the deck's theme is what puts it
        // there.
        //
        // Written as "nearer Mono than Sans" the assertion survives that disagreement and still
        // does the job this test exists for: 5.839 is 0.011 from Mono and 0.419 from Sans, and
        // 6.010 is 0.160 from Mono and 0.590 from Sans. Both readings are nearer by more than a
        // factor of three, and no face substitution can produce that.
        Math.Abs(theirs - MonospacedDigitAdvance).ShouldBeLessThan(
            Math.Abs(theirs - ProportionalDigitAdvance),
            $"the reference's digit advance of {theirs:F3} pt is nearer Liberation Mono's "
            + $"{MonospacedDigitAdvance} than Liberation Sans' {ProportionalDigitAdvance}");
    }

    /// <summary>
    /// One digit's advance: the gap between the pens of the value axis' <c>100</c> and <c>80</c>
    /// labels, which are right-aligned on the same edge.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both are found by their text, because a glyph count cannot be compared across the two
    /// writers: <c>PdfTextRuns</c> counts hexadecimal show strings and LibreOffice writes literal
    /// ones, so every reference run reports zero glyphs. A word's own left edge is poppler's
    /// reading of the pen at the start of that word, which is the same quantity on both sides.
    /// </para>
    /// <para>
    /// Measured: 6.009 for ours and 6.010 for the reference against 5.556 for the fixed face this
    /// replaces, so the tolerance below separates them by four times over.
    /// </para>
    /// </remarks>
    private static double DigitAdvance(List<PdfWord> words)
        => Pen(words, "80") - Pen(words, "100");

    private static double Pen(List<PdfWord> words, string label)
    {
        List<PdfWord> found = [.. words.Where(w => w.PageIndex == 0 && w.Text == label)];
        found.Count.ShouldBe(1, $"one label reading {label} on the first slide");
        return found[0].Left;
    }

    /// <summary>Every <c>/BaseFont</c> in the file, subset prefix stripped.</summary>
    private static List<string> Faces(string pdfPath)
    {
        string text = System.Text.Encoding.Latin1.GetString(File.ReadAllBytes(pdfPath));
        return [.. BaseFont().Matches(text).Select(m => m.Groups[1].Value).Distinct()];
    }

    private string Ours(string deck)
    {
        string source = Corpus.Require(deck);
        string destination = Path.Combine(
            _workDirectory, Path.GetFileNameWithoutExtension(deck) + "-xray-office.pdf");

        using IDocument document = OfficeDocument.Open(source);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }

    [GeneratedRegex(@"/BaseFont\s*/(?:[A-Z]{6}\+)?([A-Za-z0-9-]+)")]
    private static partial Regex BaseFont();
}
