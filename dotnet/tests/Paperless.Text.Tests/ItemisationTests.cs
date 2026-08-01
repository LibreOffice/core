using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Cutting a paragraph into the sub-runs a shaper is handed: one direction, one script, one face.
/// </summary>
/// <remarks>
/// The most important test here is the one that asserts nothing happens. A paragraph of Latin prose
/// must come out as exactly one sub-run and shape in exactly the call it shaped in before any of this
/// existed — a paragraph split into runs it does not need loses the shaping context at each boundary,
/// measures very slightly wide, and breaks its lines somewhere else.
/// </remarks>
public class ItemisationTests
{
    private static OpenTypeFace Carlito()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
    }

    private static OpenTypeFace? DejaVuSans()
    {
        string? path = FindFont("DejaVuSans.ttf");
        return path is null ? null : OpenTypeFace.ReadFile(path);
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/dejavu",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }
        return null;
    }

    // ------------------------------------------------------------------------------ script runs

    [Fact]
    public void LatinProseIsOneScriptRun()
    {
        List<ScriptRun> runs = ScriptItemiser.Itemise("The quick brown fox, 123 of them!");

        runs.Count.ShouldBe(1);
        runs[0].Script.ShouldBe("Latn");
        runs[0].Length.ShouldBe(33);
    }

    [Fact]
    public void SharedPunctuationTakesTheScriptAroundIt()
    {
        // UAX #24's resolution rule, and the reason it matters here: a boundary at every full stop
        // would cost the shaper its context at each one, and the measured width with it.
        List<ScriptRun> runs = ScriptItemiser.Itemise("Ελληνικά, και άλλα.");

        runs.Count.ShouldBe(1);
        runs[0].Script.ShouldBe("Grek");
    }

    [Fact]
    public void AScriptChangeStartsANewRunAndWeakCharactersJoinWhatPrecedesThem()
    {
        List<ScriptRun> runs = ScriptItemiser.Itemise("abc שלום xyz");

        // "abc " is Latin — the space after it is weak and joins it — then the Hebrew takes over,
        // keeps the space after itself, and the Latin resumes at "x".
        runs.Select(run => (run.Start, run.Length, run.Script)).ShouldBe(
        [
            (0, 4, "Latn"),
            (4, 5, "Hebr"),
            (9, 3, "Latn"),
        ]);
    }

    [Fact]
    public void ACombiningMarkTakesTheScriptOfWhatItIsAttachedTo()
    {
        // tdf#154549: a non-spacing mark reports Inherited whatever its own script says, so it never
        // opens a run of its own — a mark shaped apart from its base is a mark on nothing.
        List<ScriptRun> runs = ScriptItemiser.Itemise("aו́b");

        runs.Count.ShouldBe(3);
        runs[1].Script.ShouldBe("Hebr");
        runs[1].Length.ShouldBe(2);
    }

    [Fact]
    public void AClosingBracketTakesTheScriptOfItsOpener()
    {
        // The bracket stack, which is why ICU's algorithm has one: the closing parenthesis has to
        // match the opening one rather than whatever precedes it, or the two halves of a pair land
        // in different runs and are shaped by different faces.
        List<ScriptRun> runs = ScriptItemiser.Itemise("(Ελληνικά) abc");

        runs[0].Script.ShouldBe("Grek");
        runs[0].Start.ShouldBe(0);
        runs[0].Length.ShouldBe(11);
    }

    [Fact]
    public void KatakanaAndHiraganaShareARun()
    {
        // Three Unicode script codes, one OpenType script tag, so splitting them would cost a
        // shaping boundary for nothing (getScript in vcl/source/gdi/scrptrun.cxx).
        List<ScriptRun> runs = ScriptItemiser.Itemise("かなカナ");

        runs.Count.ShouldBe(1);
        runs[0].Script.ShouldBe("Hira");
    }

    // -------------------------------------------------------------------------------- text items

    [Fact]
    public void APlainParagraphIsExactlyOneItem()
    {
        List<TextItem> items = TextItemiser.Itemise("The quick brown fox jumps over 13 lazy dogs.");

        items.Count.ShouldBe(1);
        items[0].Start.ShouldBe(0);
        items[0].Length.ShouldBe(44);
        items[0].Level.ShouldBe((byte)0);
        items[0].Script.ShouldBe("Latn");
    }

    [Fact]
    public void DirectionAndScriptBoundariesAreBothHonoured()
    {
        // The paragraph LibreOffice's own portions were measured against. The digits sit at level 2
        // inside a level-1 Hebrew stretch, and "end." starts a new script run one character after
        // the direction returns to the paragraph's.
        List<TextItem> items = TextItemiser.Itemise("Start שלום 123 עולם end.");

        items.Select(item => (item.Start, item.Length, item.Level, item.Script)).ShouldBe(
        [
            (0, 6, (byte)0, "Latn"),
            (6, 5, (byte)1, "Hebr"),
            (11, 3, (byte)2, "Hebr"),
            (14, 5, (byte)1, "Hebr"),
            (19, 1, (byte)0, "Hebr"),
            (20, 4, (byte)0, "Latn"),
        ]);
    }

    [Fact]
    public void FormatControlCharactersAreCutOutRatherThanShaped()
    {
        // ImplLayoutArgs::AddRun splits a run at every control character so none reaches the shaper.
        // Handed one, HarfBuzz returns a missing-glyph box with a real advance — visible, and wide.
        List<TextItem> items = TextItemiser.Itemise("ab‎cd");

        items.Select(item => (item.Start, item.Length)).ShouldBe([(0, 2), (3, 2)]);
    }

    [Fact]
    public void TheVisualOrderIsTheOrderTheItemsAreDrawnIn()
    {
        List<TextItem> items = TextItemiser.Itemise("Start שלום 123 עולם end.");
        List<TextItem> visual = TextItemiser.InVisualOrder(items);

        visual.Select(item => item.Start).ShouldBe([0, 14, 11, 6, 19, 20]);
    }

    // --------------------------------------------------------------- the no-op case, measured

    [Fact]
    public void ALatinParagraphShapesExactlyAsItDidBeforeSubRunsExisted()
    {
        OpenTypeFace face = Carlito();
        const string Text = "The quick brown fox jumps over the lazy dog, 13 times, in office.";

        // What the shaper produces when handed the whole run with no itemisation at all.
        ShapedText whole = TextShaper.Default.Shape(face, Text);

        MeasuredParagraph measured = MeasuredParagraph.Measure(
            Text, [new FormattedRun(0, Text.Length, face, Length.FromPoints(11))]);

        // One sub-run, not several: the itemiser must not have found anything to split at.
        measured.Runs.Count.ShouldBe(1);
        measured.Items.Count.ShouldBe(1);

        // And the glyphs are the same glyphs, in the same order, with the same advances and offsets
        // — not merely a total width that happens to agree.
        ShapedText part = measured.Runs[0].Shaped;
        part.Glyphs.Count.ShouldBe(whole.Glyphs.Count);
        part.Glyphs.ShouldBe(whole.Glyphs);
        part.AdvanceInDesignUnits.ShouldBe(whole.AdvanceInDesignUnits);
    }

    [Fact]
    public void AMixedParagraphIsSplitAndStillMeasuresMonotonically()
    {
        OpenTypeFace? face = DejaVuSans();
        Assert.SkipWhen(face is null, "DejaVu Sans is not installed");

        const string Text = "Start שלום 123 עולם end.";
        MeasuredParagraph measured = MeasuredParagraph.Measure(
            Text, [new FormattedRun(0, Text.Length, face!, Length.FromPoints(12))]);

        measured.Runs.Count.ShouldBe(6);
        measured.Runs.Sum(run => run.Run.Length).ShouldBe(Text.Length);

        // The prefix table has to stay monotonic across sub-runs, because the line filler reads
        // widths out of it as differences and a dip would measure a range as negative.
        for (int i = 1; i <= Text.Length; i++)
        {
            measured.WidthBetween(0, i).ShouldBeGreaterThanOrEqualTo(measured.WidthBetween(0, i - 1));
        }

        measured.WidthBetween(0, Text.Length).ShouldBe(measured.Width);
    }

    /// <summary>
    /// An inline object widens every prefix past its boundary and none at or before it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The whole of the width rule, and the two consequences worth pinning are its edges. A range ending
    /// at the boundary does not pay for the object, so a picture too wide for the room left on a line moves
    /// to the next line whole rather than hanging off the margin; a range starting there does pay, so it
    /// arrives on that next line with its width intact.
    /// </para>
    /// <para>
    /// A boundary rather than a character because three of the four word-processing formats put no
    /// character in the text for an inline picture — see <c>InlineObject</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnInlineObjectWidensThePrefixesPastItsBoundary()
    {
        OpenTypeFace face = Carlito();
        const string Text = "before after";
        const int Boundary = 7;

        List<FormattedRun> runs = [new FormattedRun(0, Text.Length, face, Length.FromPoints(11))];
        Length picture = Length.FromMillimetres(10);

        MeasuredParagraph plain = MeasuredParagraph.Measure(Text, runs);
        MeasuredParagraph withPicture = MeasuredParagraph.Measure(
            Text, runs, objects: [new InlineObject(Boundary, picture, picture)]);

        withPicture.Width.ShouldBe(plain.Width + picture);

        // At and before the boundary, nothing has changed.
        withPicture.WidthBetween(0, Boundary).ShouldBe(plain.WidthBetween(0, Boundary));

        // Past it, everything has moved along by exactly the object.
        withPicture.WidthBetween(0, Boundary + 1)
            .ShouldBe(plain.WidthBetween(0, Boundary + 1) + picture);

        // A range starting at the boundary carries the object, which is what puts a picture that moved to
        // the next line at the head of that line rather than nowhere.
        withPicture.WidthBetween(Boundary, Text.Length)
            .ShouldBe(plain.WidthBetween(Boundary, Text.Length) + picture);
    }

    /// <summary>
    /// An inline object raises the line's ascent to its own height and nothing else.
    /// </summary>
    /// <remarks>
    /// It rests its bottom on the baseline, so only the ascent grows; the height then follows from
    /// <c>Max(height, ascent + descent)</c>, which is <c>SwLineLayout::CalcLine</c>'s own rule and was
    /// already there for a run taller than its line. The text's descent survives, which is the part a
    /// reader who simply set the height would lose.
    /// </remarks>
    [Fact]
    public void AnInlineObjectRaisesTheLinesAscentAndKeepsItsDescent()
    {
        OpenTypeFace face = Carlito();
        const string Text = "before after";

        List<FormattedRun> runs = [new FormattedRun(0, Text.Length, face, Length.FromPoints(11))];
        Length tall = Length.FromMillimetres(10);

        (Length plainHeight, Length plainAscent) =
            MeasuredParagraph.Measure(Text, runs).HeightOf(0, Text.Length);

        (Length height, Length ascent) = MeasuredParagraph.Measure(
                Text, runs, objects: [new InlineObject(7, tall, tall)])
            .HeightOf(0, Text.Length);

        plainAscent.ShouldBeLessThan(tall);
        ascent.ShouldBe(tall);
        height.ShouldBe(tall + (plainHeight - plainAscent));
    }

    [Fact]
    public void AControlCharacterTakesNoWidth()
    {
        OpenTypeFace face = Carlito();
        const string Plain = "ab";
        const string WithMark = "a‎b";

        Length plain = MeasuredParagraph.Measure(
            Plain, [new FormattedRun(0, Plain.Length, face, Length.FromPoints(11))]).Width;
        Length marked = MeasuredParagraph.Measure(
            WithMark, [new FormattedRun(0, WithMark.Length, face, Length.FromPoints(11))]).Width;

        marked.ShouldBe(plain);
    }

    // ------------------------------------------------------------------------- glyph fallback

    [Fact]
    public void ARunIsSplitWhereItsFaceHasNoGlyphAndTheSubstitutionIsReported()
    {
        OpenTypeFace face = Carlito();
        OpenTypeFace? fallback = DejaVuSans();
        Assert.SkipWhen(fallback is null, "DejaVu Sans is not installed");

        // Carlito has no Hebrew. Without fallback the whole run draws missing-glyph boxes at
        // Carlito's .notdef width, which is both visible and the wrong width.
        face.HasGlyphFor('ש').ShouldBeFalse("Carlito was expected to have no Hebrew");

        const string Text = "abשלg";
        List<GlyphFallback> reported = [];

        List<FaceRun> runs = FontItemiser.Split(
            Text, 0, Text.Length, face, new FixedFallback(fallback!), reported.Add);

        runs.Select(run => (run.Start, run.Length, run.IsFallback)).ShouldBe(
        [
            (0, 2, false),
            (2, 2, true),
            (4, 1, false),
        ]);

        // Reported, not silent: a fallback face is chosen for its coverage rather than its metrics,
        // so the run it lands in measures differently and every line after it can break elsewhere.
        // Once per stretch rather than once per character, so a paragraph in a script the face does
        // not cover leaves one entry and not a thousand.
        reported.Count.ShouldBe(1);
        reported[0].CodePoint.ShouldBe('ש');
        reported[0].ToFamily.ShouldBe(fallback!.FamilyName);
        reported[0].IsResolved.ShouldBeTrue();
    }

    [Fact]
    public void ACharacterNothingCanDrawIsStillReported()
    {
        OpenTypeFace face = Carlito();
        List<GlyphFallback> reported = [];

        // U+E000 is in the private use area, which no ordinary face claims.
        const string Text = "ab";
        List<FaceRun> runs = FontItemiser.Split(
            Text, 0, Text.Length, face, new NoFallback(), reported.Add);

        runs.Count.ShouldBe(1);
        runs[0].IsFallback.ShouldBeFalse();

        reported.Count.ShouldBe(1);
        reported[0].IsResolved.ShouldBeFalse();
        reported[0].ToFamily.ShouldBeNull();
    }

    [Fact]
    public void NoFallbackResolverMeansNoCoverageChecksAtAll()
    {
        OpenTypeFace face = Carlito();
        const string Text = "abשלg";

        List<FaceRun> runs = FontItemiser.Split(Text, 0, Text.Length, face, fallback: null);

        runs.Count.ShouldBe(1);
        runs[0].Length.ShouldBe(Text.Length);
    }

    [Fact]
    public void TheSystemResolverFindsAFaceForACharacterItsOwnLacks()
    {
        SystemFontResolver resolver = SystemFontResolver.Build();
        Assert.SkipWhen(resolver.Index.FamilyCount == 0, "no fonts are installed");

        OpenTypeFace? found = resolver.FallbackFor('ש');
        Assert.SkipWhen(found is null, "nothing installed has Hebrew");

        found!.HasGlyphFor('ש').ShouldBeTrue();

        // Cached, because a run of unsupported text asks the same question for every character and
        // answering it means opening font files until one covers it.
        ReferenceEquals(resolver.FallbackFor('ש'), found).ShouldBeTrue();
    }

    private sealed class FixedFallback(OpenTypeFace face) : IGlyphFallbackResolver
    {
        public OpenTypeFace? FallbackFor(int codePoint, int weight = 400, bool isItalic = false)
            => face.HasGlyphFor(codePoint) ? face : null;
    }

    private sealed class NoFallback : IGlyphFallbackResolver
    {
        public OpenTypeFace? FallbackFor(int codePoint, int weight = 400, bool isItalic = false)
            => null;
    }
}
