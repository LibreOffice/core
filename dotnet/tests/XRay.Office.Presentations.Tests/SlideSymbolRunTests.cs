using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.Ooxml;
using XRay.Office.Text.Fonts;
using Shouldly;
using System.Xml.Linq;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What an ordinary run carrying an <c>a:rPr/a:sym</c> is drawn in.
/// </summary>
/// <remarks>
/// <para>
/// The companion of <see cref="SlideSymbolBulletGlyphTests"/>, which covers the same recode
/// reached from a <c>a:buFont</c>/<c>a:buChar</c> bullet. The recode itself was present and
/// correct and was wired for bullets only: <c>a:sym</c> was read nowhere in
/// <c>XRay.Office.Presentations</c>, so a symbol character in the middle of a sentence drew from
/// whatever face the paragraph happened to be in. Diagnosed in
/// <c>dotnet/probes/slides-solog-01/results.md</c> §6.2 and fixed in
/// <c>dotnet/probes/slides-sym-01/results.md</c>.
/// </para>
/// <para>
/// The rule is <c>oox/source/drawingml/textrun.cxx:96-135</c>, and its shape is the thing to get
/// right: the face switch covers each maximal stretch of characters satisfying
/// <c>(ch &amp; 0xff00) == 0xf000</c> and is <em>reset after every one</em>, so a run is split
/// rather than reassigned. Measured over the slides track, 45 of the affected <c>a:t</c> values
/// hold both kinds of character, so the split is the common case.
/// </para>
/// <para>
/// Measured against the banked 26.2.4.2 reference on
/// <c>slides/batch-004/pptx/solog_orientation_august_2019.pptx</c> page 9, whose one such run is
/// a Wingdings <c>U+F0E0</c> arrow mid-sentence. The reference draws one 28.01 pt OpenSymbol
/// glyph at x = 412.33 and resumes its text at 440.28; before this rule we drew a DejaVu Serif
/// glyph 16.81 pt wide at the same pen, and afterwards one OpenSymbol glyph at 412.88 resuming
/// at 440.89 — an advance of 28.01 against the reference's 27.95.
/// </para>
/// </remarks>
public class SlideSymbolRunTests
{
    private static readonly XNamespace A =
        "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static DocRect Area =>
        new(Length.Zero, Length.Zero, Length.FromPoints(600), Length.FromPoints(200));

    /// <summary>
    /// A one-paragraph body holding <paramref name="text"/> in Carlito, with
    /// <paramref name="sym"/> as the run's own <c>a:rPr</c> children and
    /// <paramref name="levelDefault"/> as the body's level-one default.
    /// </summary>
    private static SlideTextBody Body(
        string text, string sym = "", string levelDefault = "", string attributes = "")
        => PptxTextBody.Read(XElement.Parse(
            $"""
             <a:txBody xmlns:a="{A}">
               <a:bodyPr/>
               <a:lstStyle>{levelDefault}</a:lstStyle>
               <a:p><a:r>
                 <a:rPr lang="en-US" sz="2000" {attributes}>
                   <a:latin typeface="Carlito"/>{sym}
                 </a:rPr>
                 <a:t>{text}</a:t>
               </a:r></a:p>
             </a:txBody>
             """));

    private static SlideTextRun Read(string text, string sym = "", string levelDefault = "")
        => Body(text, sym, levelDefault).Paragraphs[0].Runs[0];

    /// <summary>The glyph runs a body draws, left to right.</summary>
    private static List<GlyphRun> Drawn(SlideTextBody body)
        => [.. SlideTextLayout.Place(body, Area, new SlideFonts())
            .Select(placed => placed.Run)
            .OrderBy(run => run.Origin.X.Emu)];

    private const string Wingdings = "<a:sym typeface=\"Wingdings\" pitchFamily=\"2\" charset=\"2\"/>";

    // ---- the reader ----

    [Fact]
    public void ARunStatingNoSymbolFaceCarriesNone()
        => Read("plain").SymbolFont.ShouldBeNull();

    [Fact]
    public void ARunsOwnSymbolFaceIsRead()
        => Read("\uF0E0", Wingdings).SymbolFont!.Value.Typeface.ShouldBe("Wingdings");

    /// <summary>
    /// An <c>a:sym</c> on a level default reaches the run.
    /// </summary>
    /// <remarks>
    /// <c>TextCharacterProperties::assignUsed</c> takes <c>maSymbolFont</c> from its source
    /// whenever the source states one (<c>textcharacterproperties.cxx:55</c>), so the symbol face
    /// inherits down the same chain the size and the weight do. Reading only the run's own
    /// <c>a:rPr</c> would be right on every corpus deck and wrong on the format.
    /// </remarks>
    [Fact]
    public void ASymbolFaceOnALevelDefaultReachesTheRun()
        => Read(
                "\uF0E0",
                levelDefault: $"<a:lvl1pPr><a:defRPr>{Wingdings}</a:defRPr></a:lvl1pPr>")
            .SymbolFont!.Value.Typeface.ShouldBe("Wingdings");

    /// <summary>
    /// The run's own <c>a:sym</c> beats an inherited one, which is what <c>First</c> is for.
    /// </summary>
    [Fact]
    public void ARunsOwnSymbolFaceBeatsAnInheritedOne()
        => Read(
                "\uF0E0",
                sym: "<a:sym typeface=\"Symbol\"/>",
                levelDefault: $"<a:lvl1pPr><a:defRPr>{Wingdings}</a:defRPr></a:lvl1pPr>")
            .SymbolFont!.Value.Typeface.ShouldBe("Symbol");

    // ---- the layout ----

    /// <summary>
    /// The whole point: the slot is drawn as OpenSymbol's glyph for the same picture.
    /// </summary>
    /// <remarks>
    /// <c>U+F0E0</c> is Wingdings slot 0xE0, a heavy rightwards arrow, and the same picture is
    /// <c>U+E4A6</c> in OpenSymbol. Asking OpenSymbol for <c>U+F0E0</c> directly is
    /// <c>.notdef</c> — its whole F000–F0FF coverage is the ten digits — so the table is the only
    /// route between the two.
    /// </remarks>
    [Fact]
    public void APrivateUseSlotIsDrawnAsItsOpenSymbolGlyph()
    {
        List<GlyphRun> drawn = Drawn(Body("\uF0E0", Wingdings));

        drawn.Count.ShouldBe(1);
        drawn[0].Text.ShouldBe("\uE4A6");
        drawn[0].Font.FamilyName.ShouldBe("OpenSymbol");
    }

    /// <summary>
    /// Adobe Symbol recodes too, and it is the corpus's commonest symbol face by glyph count.
    /// </summary>
    /// <remarks>
    /// Worth a case of its own because <c>fc-match Symbol</c> answers <em>OpenSymbol</em> here
    /// rather than falling through to a text face — so this run reaches the recode by the
    /// <c>IsSubstituteFamily</c> arm of the guard where Wingdings reaches it by
    /// <c>IsSubstituted</c>. Both arms have to work: 51 of the track's 92 recodeable glyphs are
    /// this face.
    /// </remarks>
    [Fact]
    public void AnAdobeSymbolSlotIsRecodedToo()
        => Drawn(Body("\uF0AE", "<a:sym typeface=\"Symbol\" charset=\"2\"/>"))[0]
            .Text.ShouldBe("\uE124");

    /// <summary>
    /// The charset is read, and it is the value 2 alone that makes the request symbol-encoded.
    /// </summary>
    /// <remarks>
    /// <c>mnCharset = rAttribs.getInteger(XML_charset, WINDOWS_CHARSET_DEFAULT)</c> and
    /// <c>*pbSymbol = mnCharset == WINDOWS_CHARSET_SYMBOL</c>
    /// (<c>oox/source/drawingml/textfont.cxx:57-62,87-94</c>). The default is 1, so an absent
    /// charset is <em>not</em> symbol-encoded — which is the case the corpus actually contains.
    /// </remarks>
    [Theory]
    [InlineData("", false)]
    [InlineData(" charset=\"0\"", false)]
    [InlineData(" charset=\"1\"", false)]
    [InlineData(" charset=\"2\"", true)]
    public void TheSymbolCharsetIsReadAndOnlyTwoIsSymbolEncoded(string charset, bool encoded)
        => Read("\uF0E0", $"<a:sym typeface=\"Wingdings\"{charset}/>")
            .SymbolFont!.Value.IsMicrosoftEncoded.ShouldBe(encoded);

    /// <summary>
    /// An <c>a:sym</c> with no typeface is not an <c>a:sym</c>.
    /// </summary>
    /// <remarks>
    /// <c>implGetFontData</c> returns <c>!rFontName.isEmpty()</c>, and <c>textrun.cxx:113</c>
    /// guards the switch on that return value, so an empty name changes nothing at all.
    /// </remarks>
    [Fact]
    public void ANamelessSymbolFaceIsIgnored()
        => Read("\uF0E0", "<a:sym typeface=\"\"/>").SymbolFont.ShouldBeNull();

    /// <summary>
    /// A Wingdings slot that is not symbol-encoded is <em>not</em> recoded.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The correction this round's first implementation needed, and the only part of it a
    /// citation would not have given: a non-symbol-encoded request is answered by fontconfig,
    /// which has never heard of Wingdings and returns DejaVu Sans, so the slot is drawn from
    /// DejaVu Sans as it stands. Recoding it anyway drew an OpenSymbol arrow where the reference
    /// draws a DejaVu glyph, on <c>16 - UTM - (NASA).pptx</c> page 11 and
    /// <c>Stakeholders-v08052017 - v5.pptx</c> page 8 — the latter at (175.9, 94.3) and
    /// (189.2, 29.1), where the banked reference draws <c>DejaVuSans</c>.
    /// </para>
    /// <para>
    /// Asserted as "not OpenSymbol" rather than as a named face because which face fontconfig
    /// picks is a property of the machine; what is being pinned is that the recode does not fire.
    /// </para>
    /// </remarks>
    [Fact]
    public void AWingdingsSlotThatIsNotSymbolEncodedIsNotRecoded()
    {
        List<GlyphRun> drawn = Drawn(Body("\uF0E0", "<a:sym typeface=\"Wingdings\"/>"));

        drawn.Count.ShouldBe(1);
        drawn[0].Text.ShouldBe("\uF0E0");
        drawn[0].Font.FamilyName.ShouldNotBe("OpenSymbol");
    }

    /// <summary>
    /// An Adobe Symbol slot <em>is</em> recoded even when it is not symbol-encoded.
    /// </summary>
    /// <remarks>
    /// The other half of the same rule, and the reason it cannot be written as "the charset
    /// decides whether to recode". Fontconfig resolves the family <c>Symbol</c> to OpenSymbol on
    /// its own — <c>fc-match Symbol</c> answers <c>opens___.ttf</c> — so the substitution lands on
    /// OpenSymbol whichever path it took, and <c>GetRecodeData</c> then supplies the table.
    /// Measured on <c>Structural Testing.pptx</c>, which states <c>charset="0"</c> on all five of
    /// its <c>Symbol</c> runs and whose reference draws all five from OpenSymbol, within 0.3 pt of
    /// where we now draw them on pages 4, 5, 6 and 26.
    /// </remarks>
    [Fact]
    public void AnAdobeSymbolSlotIsRecodedEvenWhenNotSymbolEncoded()
        => Drawn(Body("\uF0AE", "<a:sym typeface=\"Symbol\" charset=\"0\"/>"))[0]
            .Text.ShouldBe("\uE124");

    /// <summary>
    /// The face switch covers the private-use characters and stops there.
    /// </summary>
    /// <remarks>
    /// This is the assertion that fails if the run is reassigned wholesale instead of split.
    /// <c>textrun.cxx</c> resets <c>CharFontName</c> after every symbol stretch, so the words
    /// around the arrow stay in the paragraph's own face — and drawing them from OpenSymbol
    /// instead would turn a sentence into dingbats.
    /// </remarks>
    [Fact]
    public void OnlyThePrivateUseCharactersTakeTheSymbolFace()
    {
        List<GlyphRun> drawn = Drawn(Body("see \uF0E0 here", Wingdings));

        // Not a run count: the layouter segments ordinary text for its own reasons — this line
        // comes out as four glyph runs because " here" is broken at the space — and asserting on
        // that would be testing the line breaker rather than the face switch.
        string.Concat(drawn.Select(run => run.Text)).ShouldBe("see \uE4A6 here");

        drawn.Where(run => run.Font.FamilyName == "OpenSymbol")
            .Select(run => run.Text)
            .ShouldBe(["\uE4A6"]);

        drawn.Where(run => run.Font.FamilyName != "OpenSymbol")
            .ShouldAllBe(run => run.Font.FamilyName == "Carlito");
    }

    /// <summary>
    /// Two symbols separated by ordinary text are two switches, not one long one.
    /// </summary>
    [Fact]
    public void EachPrivateUseStretchSwitchesSeparately()
    {
        List<GlyphRun> drawn = Drawn(Body("\uF0E0a\uF0D8", Wingdings));

        drawn.Select(run => run.Text).ShouldBe(["\uE4A6", "a", "\uE49E"]);
        drawn.Select(run => run.Font.FamilyName)
            .ShouldBe(["OpenSymbol", "Carlito", "OpenSymbol"]);
    }

    /// <summary>
    /// Adjacent private-use characters are one stretch and one glyph run.
    /// </summary>
    [Fact]
    public void AdjacentPrivateUseCharactersAreOneStretch()
    {
        List<GlyphRun> drawn = Drawn(Body("\uF0E0\uF0D8", Wingdings));

        drawn.Count.ShouldBe(1);
        drawn[0].Text.ShouldBe("\uE4A6\uE49E");
    }

    /// <summary>
    /// A run carrying an <c>a:sym</c> but no private-use character is left entirely alone.
    /// </summary>
    /// <remarks>
    /// The control, and not a hypothetical one: <c>a:sym</c> is commonly inherited from a level
    /// default onto every run under it, so most runs that carry one draw ordinary text. Switching
    /// on the <em>declaration</em> rather than on the character would put whole paragraphs into
    /// OpenSymbol.
    /// </remarks>
    [Fact]
    public void ARunWithNoPrivateUseCharacterIsUntouched()
    {
        List<GlyphRun> drawn = Drawn(Body("ordinary", Wingdings));

        drawn.Count.ShouldBe(1);
        drawn[0].Text.ShouldBe("ordinary");
        drawn[0].Font.FamilyName.ShouldBe("Carlito");
    }

    /// <summary>
    /// A symbol face with no recode table keeps its code point and the paragraph's own face.
    /// </summary>
    /// <remarks>
    /// <c>FontAwesome</c> is not a legacy symbol encoding and LibreOffice holds no table for it;
    /// it is also 24 of the 116 private-use glyphs the slides track declares, all on
    /// <c>_1___Opatrny_Ales_United_Kingdom_business_opportunities_final.pptx</c>. Recoding it
    /// through some other face's table would draw a different picture, which is worse than
    /// drawing none — so this is the assertion that keeps the reach honest.
    /// </remarks>
    [Fact]
    public void AFaceWithNoTableIsNotRecoded()
    {
        List<GlyphRun> drawn = Drawn(Body("\uF0E0", "<a:sym typeface=\"FontAwesome\"/>"));

        drawn.Count.ShouldBe(1);
        drawn[0].Text.ShouldBe("\uF0E0");

        // Not "Carlito": the slot is not in Carlito either, so glyph fallback picks the face —
        // which is exactly the state this deck was in before the rule existed. What matters is
        // that the code point is untouched and OpenSymbol was not reached for it.
        drawn[0].Font.FamilyName.ShouldNotBe("OpenSymbol");
    }

    /// <summary>
    /// A private-use character with no <c>a:sym</c> behind it is drawn where the file put it.
    /// </summary>
    /// <remarks>
    /// <c>textrun.cxx:113</c> guards the switch with <c>bSymbol &amp;&amp; getFontData(...)</c>,
    /// and <c>getFontData</c> returns false for an empty typeface — so a slot with no symbol face
    /// named for it gets no substitution at all.
    /// </remarks>
    [Fact]
    public void APrivateUseCharacterWithNoSymbolFaceIsNotRecoded()
        => Drawn(Body("\uF0E0"))[0].Text.ShouldBe("\uF0E0");

    /// <summary>
    /// Splitting a run keeps every character offset, so the run's own decoration still lands.
    /// </summary>
    /// <remarks>
    /// The recode is one code point for one code point, so the paragraph's text keeps its length
    /// and the pieces of a split run keep their positions in it. Asserting it through the
    /// underline rather than through the offsets directly is deliberate: the underline is looked
    /// up by character position in <c>Block.DecorationAt</c>, so a rewrite that moved an index
    /// would lose it on one of the three pieces.
    /// </remarks>
    [Fact]
    public void ASplitRunKeepsItsDecorationOnEveryPiece()
    {
        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            Body("see \uF0E0 here", Wingdings, attributes: "u=\"sng\""),
            Area,
            new SlideFonts());

        placed.Count.ShouldBeGreaterThan(1);
        placed.ShouldAllBe(run => run.Rules != null && run.Rules.Count == 1);
    }

    /// <summary>
    /// The pieces are laid end to end, with the symbol's own advance between them.
    /// </summary>
    /// <remarks>
    /// The recoded glyph is materially wider than what we drew before it — 28.01 pt against
    /// 16.81 on the subject document — which is why the substitution has to happen before line
    /// breaking rather than in the painter.
    /// </remarks>
    [Fact]
    public void ThePiecesAreLaidEndToEnd()
    {
        List<GlyphRun> drawn = Drawn(Body("see \uF0E0 here", Wingdings));

        for (int index = 1; index < drawn.Count; index++)
        {
            Length previous = drawn[index - 1].Origin.X;
            foreach (PositionedGlyph glyph in drawn[index - 1].Glyphs) previous += glyph.Advance;

            Math.Abs((drawn[index].Origin.X - previous).Emu).ShouldBeLessThanOrEqualTo(1);
        }
    }
}
