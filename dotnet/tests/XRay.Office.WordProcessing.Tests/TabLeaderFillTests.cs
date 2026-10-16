using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// How many fill characters a dot leader holds, and how they are pitched across the blank.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwTabPortion::Paint</c> (<c>sw/source/core/text/txttab.cxx</c>:645-659) counts the fill characters
/// as <c>Width() / nCharWidth</c>, and both of those are <c>SwTwips</c> — whole twips. Dividing by the
/// font's exact advance instead loses a character or two on every contents line: Carlito's full stop at
/// 12 pt is 60.586 twips, so an 8051-twip blank takes 134 dots at Writer's 60 and 132 at the exact
/// width. The fixture is the same arithmetic at 9 pt — 45.44 twips against Writer's 45 — and it turns
/// 103 dots into 104.
/// </para>
/// <para>
/// The short leader is visible as a hole in front of the page number, and it is also worth a spurious
/// extracted <em>word</em>: poppler starts a new token at a gap of about a tenth of the em, so a
/// contents line comes out as <c>Revision History………</c> plus <c>4</c> where the reference's comes out
/// as one token. Across <c>words/done-*</c> that was 422 extra extracted words on 23 documents, and it
/// was invisible to the gate because the band is 2% of a long document.
/// </para>
/// </remarks>
public sealed class TabLeaderFillTests
{
    /// <summary>
    /// The leader holds every fill character the blank has room for at Writer's measurement of one.
    /// </summary>
    /// <remarks>
    /// Stated as a property of the drawn run rather than as the division that produced it: one more
    /// character would not fit, and one fewer would leave room for it. The whole twip is named because
    /// it is the thing under test — the same assertion against the font's exact advance is satisfied by
    /// a leader one character short, which is how this went unnoticed.
    /// </remarks>
    [Fact]
    public void ADotLeaderHoldsEveryFillCharacterTheBlankHasRoomFor()
    {
        foreach ((DrawnGlyphRun leader, DrawnGlyphRun next) in LeadersWithWhatFollows())
        {
            int count = leader.Run.Glyphs.Count;
            double blank = next.Origin.X.Points - leader.Origin.X.Points;
            double fill = Length.FromTwips(Natural(leader).Emu / Length.EmuPerTwip).Points;

            (count * fill).ShouldBeLessThanOrEqualTo(
                blank + 0.001,
                $"{count} fill characters of {fill:F3} pt have to fit the {blank:F2} pt blank");
            ((count + 1) * fill).ShouldBeGreaterThan(
                blank,
                $"a {count + 1}th fill character of {fill:F3} pt does not fit the {blank:F2} pt blank, "
                    + "so the leader is a character short of the stop it points at");
        }
    }

    /// <summary>
    /// A fill too long for its blank is squeezed onto it; one too short is left where it fell.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Counting at the whole twip and setting at the font's own width would run the fill past the stop,
    /// so <c>SwTabPortion::Paint</c> passes <c>bKern</c> to <c>SwTextPaintInfo::DrawText</c> and reaches
    /// <c>SwFont::DrawStretchText_</c>, which lays the run out against the portion's width.
    /// </para>
    /// <para>
    /// It compresses and it does not expand, because VCL's <c>GenericSalLayout::Justify</c> spreads a
    /// widening across the blanks of the string it is given and a run of dots has none. Both halves are
    /// readable in the reference's own PDFs: <c>system_design__technical_architecture_template.docx</c>,
    /// where Carlito's 60.586 twips truncates, carries a <c>-1</c> or <c>-2</c> adjustment after every
    /// dot and ends flush against the page number; <c>Agile_Arc_SysDes.docx</c>, where Liberation
    /// Serif's full stop is 55 twips exactly and nothing truncates, writes its dots as one unadjusted
    /// show and stops 2.05 to 2.35 pt short.
    /// </para>
    /// </remarks>
    [Fact]
    public void AFillTooLongForItsBlankIsCompressedOntoItAndOneTooShortIsNot()
    {
        foreach ((DrawnGlyphRun leader, DrawnGlyphRun next) in LeadersWithWhatFollows())
        {
            int count = leader.Run.Glyphs.Count;
            double blank = next.Origin.X.Points - leader.Origin.X.Points;
            double natural = Natural(leader).Points;
            double pitch = leader.Run.Glyphs[0].Advance.Points;

            if (count * natural > blank)
            {
                (count * pitch).ShouldBe(
                    blank, 0.01, "an overlong fill is compressed onto exactly the blank it has");
            }
            else
            {
                pitch.ShouldBe(natural, 0.001, "a fill with room to spare keeps the width the face gives it");
            }
        }
    }

    /// <summary>
    /// One fill character at the width the face gives it, taken from the run's own last glyph.
    /// </summary>
    /// <remarks>
    /// The last glyph rather than the first, because the compression is carried as tracking — the gap
    /// <em>between</em> characters — so the trailing one keeps its natural advance and is the width to
    /// measure the others against without reshaping anything.
    /// </remarks>
    private static Length Natural(DrawnGlyphRun leader) => leader.Run.Glyphs[^1].Advance;

    /// <summary>Each drawn leader on the fixture, with the run drawn after it on the same line.</summary>
    private static List<(DrawnGlyphRun Leader, DrawnGlyphRun Next)> LeadersWithWhatFollows()
    {
        List<(DrawnGlyphRun, DrawnGlyphRun)> found = [];

        foreach (DrawnPage page in Drawn())
        {
            foreach (DrawnGlyphRun run in page.Runs)
            {
                if (run.Text.Length < 4 || run.Text.Any(c => c != '.')) continue;

                DrawnGlyphRun? next = page.Runs
                    .Where(other => other.Origin.X.Emu > run.Origin.X.Emu
                        && Math.Abs(other.Origin.Y.Emu - run.Origin.Y.Emu) <= Length.FromPoints(0.5).Emu)
                    .MinBy(other => other.Origin.X.Emu);

                if (next is not null) found.Add((run, next));
            }
        }

        found.ShouldNotBeEmpty("the fixture's contents entries are drawn with dot leaders");
        return found;
    }

    private static IReadOnlyList<DrawnPage> Drawn()
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source =
               DocumentSource.FromFile(Corpus.Require("style-tab-stops.docx")))
        {
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink.Pages;
    }
}
