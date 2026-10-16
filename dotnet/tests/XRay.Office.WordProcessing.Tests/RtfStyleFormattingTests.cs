using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Where a run's face comes from when the run does not name one: <c>\deff</c>, <c>\plain</c>, and
/// the paragraph style's own character formatting.
/// </summary>
/// <remarks>
/// <para>
/// The reading this replaces was that RTF's stylesheet is decorative — that a writer restates a
/// paragraph's effective formatting inline after the <c>\s</c> that names its style, so a reader
/// has no cascade to resolve. LibreOffice's own RTF export does not: it writes the
/// <em>difference</em> from the style, so a whole table cell can read
/// <c>\pard\plain \s24\li85\lin85\intbl{\cf8\fs20\b TC Holder}</c> and name no font at all. Its
/// importer resolves that cascade at <c>RTFDocumentImpl::getProperties</c>
/// (<c>sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx</c>:534-637), whose comment at :616-618 is
/// <em>"Take paragraph style into account for character properties as well, as paragraph style may
/// contain character properties"</em>; <c>\pard</c> selects style zero when the paragraph names none
/// (<c>rtfdispatchflag.cxx</c>:600-614) and <c>\plain</c> restores the default character state,
/// which carries <c>\deff</c>'s face (<c>:575-583</c>, and <c>rtfdocumentimpl.cxx</c>:2494-2500 for
/// where that face is put there).
/// </para>
/// <para>
/// Reading it the other way put every such run in font zero, which is <c>Times New Roman</c> in
/// every file LibreOffice writes. <strong>215 of the 338 converted corpus <c>.rtf</c> declare a
/// <c>\deff</c> naming a different face from their <c>\f0</c></strong>, and on
/// <c>Annex-10…GCAA.rtf</c> the reference lays 148 pages of table out in Carlito where this tree laid
/// them out in Liberation Serif and paginated to 169. See
/// <c>dotnet/probes/rtf-page-r77/results.md</c>.
/// </para>
/// </remarks>
public sealed class RtfStyleFormattingTests
{
    /// <summary>
    /// A run that names no <c>\f</c> is set in <c>\deff</c>'s face, not in font zero's.
    /// </summary>
    [Fact]
    public void ADeffDecidesTheFaceOfARunThatNamesNoFont()
        => First(@"\pard\plain\fs20 MARKER\par").Family.ShouldBe("Liberation Sans");

    /// <summary>
    /// <c>\plain</c> puts the face back to <c>\deff</c>'s rather than leaving the last run's.
    /// </summary>
    /// <remarks>
    /// The witness is a table row whose empty cells are written <c>\pard\plain …\f0\fs18 \cell</c>:
    /// with nothing to restore the default, that <c>\f0</c> set the face of every paragraph after it.
    /// </remarks>
    [Fact]
    public void PlainPutsTheFaceBackToTheDefault()
        => Read(@"\pard\plain\f0\fs20 FIRST\par \pard\plain\fs20 SECOND\par")
            .Select(paragraph => paragraph.Family)
            .ShouldBe(["Liberation Serif", "Liberation Sans"]);

    /// <summary>And the size with it: RTF's own default is <c>\fs24</c>, which is twelve points.</summary>
    [Fact]
    public void PlainPutsTheSizeBackToTheDefault()
        => Read(@"\pard\plain\fs40 FIRST\par \pard\plain SECOND\par")
            .Select(paragraph => paragraph.Size)
            .ShouldBe([Length.FromPoints(20), Length.FromPoints(12)]);

    /// <summary>
    /// A paragraph style's font <em>face</em> reaches no run at all — <c>\deff</c>'s wins.
    /// </summary>
    /// <remarks>
    /// The reading this replaces was that a style's face reaches a run that names none. It does
    /// not, at any level of the chain: <c>\deff</c>'s name is put into the importer's default
    /// character state (<c>rtfdocumentimpl.cxx</c>:2494-2500), every group state is copied from
    /// that state, and a run therefore carries the face as <em>direct</em> formatting that no style
    /// can beat. Measured on four probes whose <c>\deff</c>, whose style's <c>\f</c> and whose
    /// <c>Times New Roman</c> fallback are three different faces — style's own or inherited, with
    /// <c>\plain</c> and without — 26.2.4.2 answers <c>\deff</c>'s in all four
    /// (<c>probes/rtf-resid-r80</c>, the <c>d-*</c> family).
    /// </remarks>
    [Fact]
    public void AParagraphStylesFaceReachesNoRun()
        => First(
                @"\pard\plain\s2\fs20 MARKER\par",
                styles: @"{\s2\snext2\f2 Boxed;}")
            .Family.ShouldBe("Liberation Sans");

    /// <summary>Nor does an inherited one, which is the same rule one link up.</summary>
    [Fact]
    public void AnInheritedFaceReachesNoRunEither()
        => First(
                @"\pard\plain\s3\fs20 MARKER\par",
                styles: @"{\s2\snext2\f2 Middle;}{\s3\sbasedon2\snext3\li85 Boxed;}")
            .Family.ShouldBe("Liberation Sans");

    /// <summary>
    /// A style's own <c>\fs</c> does not reach a paragraph that names that style, and an
    /// ancestor's does.
    /// </summary>
    /// <remarks>
    /// <c>getDefaultSPRM</c> answers <c>24</c> — twelve points — for a size
    /// (<c>sw/source/writerfilter/rtftok/rtfsprm.cxx</c>:158-161), and
    /// <c>cloneAndDeduplicateSprm</c>'s <em>"not found - try to override style with default"</em>
    /// branch (:319-327) writes it over the paragraph for every property the <em>named</em> style
    /// states. Only the ancestors' statements escape it.
    /// </remarks>
    [Fact]
    public void AStylesOwnSizeIsReplacedByTheDefaultAndAnInheritedOneIsNot()
    {
        First(@"\pard\plain\s2 MARKER\par", styles: @"{\s2\snext2\fs36 Big;}")
            .Size.ShouldBe(Length.FromPoints(12));

        First(@"\pard\plain\s3 MARKER\par",
                styles: @"{\s2\snext2\fs36 Big;}{\s3\sbasedon2\snext3\li85 Child;}")
            .Size.ShouldBe(Length.FromPoints(18));
    }

    /// <summary>
    /// <c>\pard</c> selects style zero, and style zero's own statements are therefore the named
    /// style's — so a bare paragraph takes none of them.
    /// </summary>
    [Fact]
    public void PardSelectsStyleZeroAndTakesNoneOfItsOwnStatements()
        => First(@"\pard\plain MARKER\par", styles: @"{\s0\snext0\fs36 Normal;}")
            .Size.ShouldBe(Length.FromPoints(12));

    /// <summary>
    /// A style replaces the one before it rather than piling onto it.
    /// </summary>
    /// <remarks>
    /// <c>\pard</c> selects style zero and the <c>\sN</c> after it selects another, so accumulating
    /// the two would leave style zero's bold on a paragraph in a style that inherits nothing from it.
    /// The bold is stated one link up in both, because a style's own statement never reaches the
    /// paragraph that names it.
    /// </remarks>
    [Fact]
    public void ASelectedStyleReplacesTheOneBefore()
    {
        Formatting formatting = First(
            @"\pard\plain\s3\fs20 MARKER\par",
            styles: @"{\s1\snext1\b Bold;}{\s0\sbasedon1\snext0 Normal;}{\s3\snext3 Boxed;}");

        formatting.Weight.ShouldBe(400);
    }

    /// <summary>
    /// A style that turns a toggle off beats a parent that turns it on, and one that says nothing
    /// inherits it.
    /// </summary>
    /// <remarks>
    /// This is what the stated-word set is for. <c>\b0</c> and an absent <c>\b</c> leave the same
    /// group state, so recording the state alone would make every style say "not bold" and no
    /// <c>\sbasedon</c> parent could ever pass bold down.
    /// </remarks>
    [Fact]
    public void AStyleStatingBoldOffBeatsABoldParentAndSilenceDoesNot()
    {
        First(
            @"\pard\plain\s3\fs20 MARKER\par",
            styles: @"{\s2\snext2\b\f2 Bold;}{\s3\sbasedon2\snext3\b0 Quiet;}")
            .Weight.ShouldBe(400);

        First(
            @"\pard\plain\s3\fs20 MARKER\par",
            styles: @"{\s2\snext2\b\f2 Bold;}{\s3\sbasedon2\snext3\li85 Quiet;}")
            .Weight.ShouldBe(700);
    }

    /// <summary>Direct formatting written after the <c>\s</c> still wins, which is the usual case.</summary>
    /// <remarks>
    /// A guard rather than a new behaviour — the direct <c>\f</c> was always honoured — and it is
    /// here because the style is now applied where the <c>\s</c> appears, which is a place a later
    /// change could easily move past the run's own words.
    /// </remarks>
    [Fact]
    public void DirectFormattingAfterTheStyleWins()
        => First(
                @"\pard\plain\s2\f0\fs20 MARKER\par",
                styles: @"{\s2\snext2\f2 Boxed;}")
            .Family.ShouldBe("Liberation Serif");

    /// <summary>
    /// A style named after one of Word's headings whose <c>\sbasedon</c> the sheet cannot resolve
    /// keeps Writer's own <em>Heading</em> pool parent, which is fourteen points.
    /// </summary>
    /// <remarks>
    /// Both spellings of "cannot resolve" are here, because the RTF files that show it use the
    /// second: <c>\sbasedon</c> absent, and <c>\sbasedon</c> naming a style declared <em>later</em>
    /// in the same sheet — which <c>getStyleName</c> (<c>rtfdocumentimpl.cxx</c>:873-885) answers
    /// the empty string for, so <c>lcl_findParentStyle</c> (<c>:275-298</c>) finds nothing.
    /// <c>StyleSheetTable::ApplyStyleSheets</c> then reuses Writer's existing <c>Heading 4</c> and
    /// leaves its parent alone (<c>StyleSheetTable.cxx</c>:1099-1121), and that parent is
    /// <c>COLL_HEADLINE_BASE</c> at <c>PT_14</c>
    /// (<c>DocumentStylePoolManager.cxx</c>:768-819, <c>:809</c>).
    /// </remarks>
    [Fact]
    public void AHeadingWhoseParentDoesNotResolveTakesWritersPoolHeadingSize()
    {
        First(@"\pard\plain\s4 MARKER\par", styles: @"{\s0\snext0\f0\fs20 Normal;}{\s4\snext0 heading 4;}")
            .Size.ShouldBe(Length.FromPoints(14));

        First(
            @"\pard\plain\s4 MARKER\par",
            styles: @"{\s0\snext0\f0\fs20 Normal;}{\s4\sbasedon9\snext0 heading 4;}"
                    + @"{\s9\sbasedon0\snext9\fs18\b Notes;}")
            .Size.ShouldBe(Length.FromPoints(14));
    }

    /// <summary>A <c>\sbasedon</c> the sheet <em>can</em> resolve replaces that pool parent.</summary>
    /// <remarks>
    /// The same two entries in the other order: <c>setParentStyle</c>
    /// (<c>StyleSheetTable.cxx</c>:1156-1169) puts the named style in the pool parent's place, so
    /// nine points reach the paragraph and fourteen does not. This is the control that keeps the
    /// rule a fallback rather than an override.
    /// </remarks>
    [Fact]
    public void AResolvableParentReplacesThePoolHeading()
        => First(
                @"\pard\plain\s4 MARKER\par",
                styles: @"{\s0\snext0\f0\fs20 Normal;}{\s9\sbasedon0\snext9\fs18\b Notes;}"
                        + @"{\s4\sbasedon9\snext0 heading 4;}")
            .Size.ShouldBe(Length.FromPoints(9));

    /// <summary>
    /// Only a name Writer answers takes it, and only when the style states no size of its own.
    /// </summary>
    /// <remarks>
    /// Two controls in one, and each was measured against 26.2.4.2 rather than reasoned about. A
    /// style called <c>Widget Heading</c> matches nothing in <c>ConvertStyleName</c>'s map
    /// (<c>StyleSheetTable.cxx</c>:1620-1660), so it is inserted as a new style and its paragraph
    /// keeps what <c>\pard\plain</c> left, which is RTF's own <c>\fs24</c>. And a heading stating
    /// its <em>own</em> <c>\fs20</c> reaches neither ten points nor fourteen but the same twelve,
    /// because a named style's own character size never reaches its paragraphs at all — the rule
    /// this file already carries.
    /// </remarks>
    [Fact]
    public void APoolHeadingNeitherRenamedNorRestatedIsTheOnlyOneThatTakesIt()
    {
        First(@"\pard\plain\s7 MARKER\par", styles: @"{\s0\snext0\f0\fs20 Normal;}{\s7\snext0 Widget Heading;}")
            .Size.ShouldBe(Length.FromPoints(12));

        First(@"\pard\plain\s4 MARKER\par", styles: @"{\s0\snext0\f0\fs20 Normal;}{\s4\snext0\fs20 heading 4;}")
            .Size.ShouldBe(Length.FromPoints(12));
    }

    /// <summary>
    /// The pool heading brings its spacing with it: twelve points above and six below.
    /// </summary>
    /// <remarks>
    /// <c>SvxULSpaceItem aUL(PT_12, PT_6, RES_UL_SPACE)</c>
    /// (<c>DocumentStylePoolManager.cxx</c>:810). The space <em>below</em> is the one this cannot
    /// carry through <c>\sa</c>: an RTF style's own space after is written back as zero by
    /// <c>getDefaultSPRM</c>, and the pool's is not an RTF sprm at all. Measured as the distance
    /// between the paragraph after the heading and the heading itself, against the same document
    /// with the style renamed.
    /// </remarks>
    [Fact]
    public void ThePoolHeadingBringsTwelvePointsAboveSixBelowAndKeepWithNext()
    {
        ParagraphFormat heading = Formats(
            @"\pard\plain\s4 HEAD\par",
            @"{\s0\snext0\f0\fs20 Normal;}{\s4\snext0 heading 4;}")[0];

        heading.SpaceBefore.ShouldBe(Length.FromPoints(12));
        heading.SpaceAfter.ShouldBe(Length.FromPoints(6));
        heading.KeepWithNext.ShouldBeTrue();

        // The same paragraph under a name Writer answers nothing for: it takes none of the three,
        // and a `\sa` written after the `\s` still beats the pool's.
        ParagraphFormat plain = Formats(
            @"\pard\plain\s7 HEAD\par",
            @"{\s0\snext0\f0\fs20 Normal;}{\s7\snext0 Widget Heading;}")[0];

        plain.SpaceBefore.ShouldBe(Length.Zero);
        plain.SpaceAfter.ShouldBe(Length.Zero);
        plain.KeepWithNext.ShouldBeFalse();

        Formats(
                @"\pard\plain\s4\sb0\sa0 HEAD\par",
                @"{\s0\snext0\f0\fs20 Normal;}{\s4\snext0 heading 4;}")[0]
            .SpaceAfter.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// <c>Title</c> and <c>Subtitle</c> take the same pool <em>Heading</em>, and neither takes what
    /// its own pool style states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both map to a pool style under <c>COLL_DOC_BITS</c>, whose parent is
    /// <c>COLL_HEADLINE_BASE</c> (<c>GetPoolParent</c>, <c>sw/source/core/doc/poolfmt.cxx</c>:279-289)
    /// — so a name that looks nothing like a heading takes a heading's fourteen points, twelve above
    /// and six below. The 28 pt bold centring <c>COLL_DOC_TITLE</c> states
    /// (<c>DocumentStylePoolManager.cxx</c>:1365-1374) and the 18 pt of <c>COLL_DOC_SUBTITLE</c>
    /// (<c>:1376-1387</c>) are the entry's own properties and the import resets those, which is the
    /// same half of the rule that costs the nine headings their <c>aHeadlineSizes</c> percentages.
    /// </para>
    /// <para>
    /// Measured before it was written: 26.2.4.2 draws all three of <c>heading 4</c>, <c>Title</c>
    /// and <c>Subtitle</c> at 14 pt with 28.16 pt above and 17.49 below, unturned and unbolded,
    /// whatever the document's <c>Normal</c> entry states. <c>probes/rtf-bookmark-r88/genpool.py</c>.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("Title")]
    [InlineData("Subtitle")]
    public void TheDocumentTitleStylesTakeTheSamePoolHeading(string name)
    {
        ParagraphFormat format = Formats(
            @"\pard\plain\s7 HEAD\par",
            @"{\s0\snext0\f0\fs20 Normal;}{\s7\sbasedon9\snext0 " + name + ";}"
                + @"{\s9\sbasedon0\snext9\fs18\b Notes;}")[0];

        format.SpaceBefore.ShouldBe(Length.FromPoints(12));
        format.SpaceAfter.ShouldBe(Length.FromPoints(6));
        format.KeepWithNext.ShouldBeTrue();

        First(@"\pard\plain\s7 MARKER\par",
                @"{\s0\snext0\f0\fs20 Normal;}{\s7\sbasedon9\snext0 " + name + ";}"
                    + @"{\s9\sbasedon0\snext9\fs18\b Notes;}")
            .Size.ShouldBe(Length.FromPoints(14));
    }

    /// <summary>
    /// A heading-parented style takes the document's own <c>Normal</c> in every property
    /// <em>Heading</em> does not state — bold, italic and alignment among them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>COLL_HEADLINE_BASE</c>'s own pool parent is <c>COLL_STANDARD</c>
    /// (<c>sw/source/core/doc/poolfmt.cxx</c>:279-289), so <em>Heading</em> is an intermediate and
    /// not the end of the walk. Rounds 87 and 95 both folded it in as a constant and neither
    /// measured it, which is why the structural citation is not what this test rests on: the
    /// intermediate states four things and each of them shadows <c>Normal</c>, so what survives had
    /// to be measured property by property.
    /// </para>
    /// <para>
    /// Measured at 26.2.4.2 on 75 probes — twelve heading-parented names × five arms, plus three
    /// controls (<c>probes/rtf-heading-r98/genheading.py</c>, read out of
    /// <c>--convert-to fodt</c>). All twelve resolve as <c>Heading_20_N&gt;Heading&gt;Standard</c>
    /// and take <c>bold@Standard</c>, <c>italic@Standard</c>, <c>#ff0000@Standard</c>,
    /// <c>solid@Standard</c> underline and strike, <c>uppercase@Standard</c> and
    /// <c>center@Standard</c> from the document's own <c>Normal</c>.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("heading 1")]
    [InlineData("heading 2")]
    [InlineData("heading 5")]
    [InlineData("heading 9")]
    [InlineData("Heading 1")]
    [InlineData("Title")]
    [InlineData("Subtitle")]
    public void AHeadingParentedStyleTakesTheDocumentsOwnNormal(string name)
    {
        string styles = @"{\s0\snext0\f0\fs20\b\i\qc Normal;}"
            + @"{\s7\sbasedon9\snext0 " + name + ";}"
            + @"{\s9\sbasedon0\snext9\fs18 Notes;}";

        First(@"\pard\plain\s7 MARKER\par", styles).Weight.ShouldBe(700);
        Italic(@"\pard\plain\s7 HEAD\par", styles).ShouldBeTrue();
        Formats(@"\pard\plain\s7 HEAD\par", styles)[0].Alignment.ShouldBe(TextAlignment.Centre);
    }

    /// <summary>
    /// And it takes none of the four <em>Heading</em> does state, however loudly <c>Normal</c>
    /// states them.
    /// </summary>
    /// <remarks>
    /// The other half of the same rule, and the half that keeps the walk from being a regression:
    /// the reference's resolved <em>Heading</em> is <c>fo:font-size="14pt"</c>,
    /// <c>fo:margin-top="0.1665in"</c>, <c>fo:margin-bottom="0.0835in"</c> and
    /// <c>fo:keep-with-next="always"</c> over <c>style:parent-style-name="Standard"</c>, so a
    /// <c>Normal</c> of <c>\fs36\sb400</c> changes none of them. <c>\fs36</c> rather than
    /// <c>\fs28</c> because <c>PT_14</c> <em>is</em> <c>\fs28</c> and a probe that cannot tell the
    /// two apart measures nothing. <c>probes/rtf-heading-r98/</c>, the <c>size</c> and <c>paras</c>
    /// arms.
    /// </remarks>
    [Theory]
    [InlineData("heading 1")]
    [InlineData("heading 4")]
    [InlineData("Title")]
    [InlineData("Subtitle")]
    public void TheHeadingPoolStillShadowsSizeSpacingAndKeep(string name)
    {
        string styles = @"{\s0\snext0\f0\fs36\sb400 Normal;}"
            + @"{\s7\sbasedon9\snext0 " + name + ";}"
            + @"{\s9\sbasedon0\snext9\fs18\b Notes;}";

        First(@"\pard\plain\s7 MARKER\par", styles).Size.ShouldBe(Length.FromPoints(14));

        ParagraphFormat format = Formats(@"\pard\plain\s7 HEAD\par", styles)[0];
        format.SpaceBefore.ShouldBe(Length.FromPoints(12));
        format.SpaceAfter.ShouldBe(Length.FromPoints(6));
        format.KeepWithNext.ShouldBeTrue();
    }

    /// <summary>
    /// <c>Body Text</c> and <c>caption</c> take the document's own <c>Normal</c>, not a constant.
    /// </summary>
    /// <remarks>
    /// Both names' pool styles have <c>COLL_STANDARD</c> for a parent (<c>poolfmt.cxx</c>:201-204,
    /// :229-235), so what 26.2.4.2 gives them is the document's own <c>Normal</c> entry — measured
    /// on <c>probes/rtf-bookmark-r88/genpool.py</c>, <b>10 pt under <c>\fs20</c> and 14 pt under
    /// <c>\fs28</c></b>, while <c>heading 4</c>, <c>Title</c> and <c>Subtitle</c> answer 14 to
    /// both. That pair of sizes is the whole assertion: one <c>Normal</c> cannot tell a pool
    /// constant from inheritance, which is what round 87 read the other way.
    /// </remarks>
    [Theory]
    [InlineData("Body Text", 20, 10)]
    [InlineData("Body Text", 28, 14)]
    [InlineData("caption", 20, 10)]
    [InlineData("caption", 28, 14)]
    [InlineData("Caption", 20, 10)]
    [InlineData("header", 20, 10)]
    [InlineData("header", 28, 14)]
    [InlineData("Header", 28, 14)]
    [InlineData("footer", 20, 10)]
    [InlineData("footer", 28, 14)]
    [InlineData("Footer", 28, 14)]
    [InlineData("toc 1", 20, 10)]
    [InlineData("toc 1", 28, 14)]
    [InlineData("toc 3", 28, 14)]
    [InlineData("TOC 1", 28, 14)]
    [InlineData("Index 1", 28, 14)]
    [InlineData("Heading", 20, 10)]
    [InlineData("Heading", 28, 14)]
    [InlineData("Comment", 28, 14)]
    [InlineData("Signature", 28, 14)]
    public void APoolStyleUnderStandardTakesTheDocumentsOwnNormal(string name, int normal, int size)
        => First(@"\pard\plain\s7 MARKER\par",
                @"{\s0\snext0\f0\fs" + normal + " Normal;}"
                    + @"{\s7\sbasedon9\snext0 " + name + ";}"
                    + @"{\s9\sbasedon0\snext9\fs18\b Notes;}")
            .Size.ShouldBe(Length.FromPoints(size));

    /// <summary>
    /// A bare <c>Heading</c> answers <em>Standard</em> and not the heading constants.
    /// </summary>
    /// <remarks>
    /// The trap the name sets: <c>Heading</c> <em>is</em> <c>COLL_HEADLINE_BASE</c>, so the very
    /// style that gives <c>heading 1</c>…<c>heading 9</c> their 14 pt, 12/6 and keep-with-next is
    /// the one <c>SetPropertiesToDefault</c> resets when a document declares a style of that name
    /// (<c>StyleSheetTable.cxx</c>:1111). It is also not in <c>ConvertStyleName</c>'s map at all —
    /// what reaches Writer's style is <c>hasByName</c> on the name as written. Measured at
    /// 26.2.4.2: no space above, no space below, and the document's own <c>Normal</c> size —
    /// <c>probes/rtf-style-r97/genpool2.py</c>, six probes.
    /// </remarks>
    [Fact]
    public void ABareHeadingIsStandardRatherThanTheHeadingPool()
    {
        ParagraphFormat format = Formats(
            @"\pard\plain\s7 HEAD\par",
            @"{\s0\snext0\f0\fs20 Normal;}{\s7\sbasedon9\snext0 Heading;}"
                + @"{\s9\sbasedon0\snext9\fs18\b Notes;}")[0];

        format.SpaceBefore.ShouldBe(Length.Zero);
        format.SpaceAfter.ShouldBe(Length.Zero);
        format.KeepWithNext.ShouldBeFalse();
    }

    /// <summary>
    /// The five <c>COLL_LABEL_*</c> names take Writer's <em>Caption</em>: italic, 12 pt, 6/6.
    /// </summary>
    /// <remarks>
    /// <c>Figure</c>, <c>Illustration</c>, <c>Table</c>, <c>Drawing</c> and <c>Text</c> all have
    /// <c>COLL_LABEL</c> for a pool parent (<c>poolfmt.cxx</c>:248-252), and unlike the style the
    /// entry matched, <em>that</em> style is not reset — which is what makes this a different rule
    /// from <see cref="APoolStyleUnderStandardTakesTheDocumentsOwnNormal"/> rather than the same
    /// one. The size is the 240 twips <c>SwDocShell::InitNew</c> writes over the block's own
    /// <c>PT_10</c> (<c>docshini.cxx</c>:224-289), which is why it does not answer 10.
    /// <b>The pair of <c>Normal</c> sizes is the assertion</b>: 12 pt under both is what says the
    /// intermediate shadows the reference underneath it. <c>probes/rtf-style-r97/genpool3.py</c>.
    /// </remarks>
    [Theory]
    [InlineData("Figure", 20)]
    [InlineData("Figure", 28)]
    [InlineData("Illustration", 20)]
    [InlineData("Table", 28)]
    [InlineData("Drawing", 28)]
    [InlineData("Text", 20)]
    [InlineData("Text", 28)]
    public void TheLabelFamilyTakesWritersCaption(string name, int normal)
    {
        string styles = @"{\s0\snext0\f0\fs" + normal + " Normal;}"
            + @"{\s7\sbasedon9\snext0 " + name + ";}"
            + @"{\s9\sbasedon0\snext9\fs18\b Notes;}";

        First(@"\pard\plain\s7 MARKER\par", styles).Size.ShouldBe(Length.FromPoints(12));

        ParagraphFormat format = Formats(@"\pard\plain\s7 HEAD\par", styles)[0];
        format.SpaceBefore.ShouldBe(Length.FromPoints(6));
        format.SpaceAfter.ShouldBe(Length.FromPoints(6));
        Italic(@"\pard\plain\s7 HEAD\par", styles).ShouldBeTrue();
    }

    /// <summary>
    /// A document that declares a <c>caption</c> of its own replaces the whole of that pool style.
    /// </summary>
    /// <remarks>
    /// <c>ApplyStyleSheets</c> resets the matched style and writes the entry's own properties back
    /// over it (<c>StyleSheetTable.cxx</c>:1101-1121), so <em>Caption</em> stops being a constant
    /// and becomes a second reference — and the italic, the 12 pt and the 6/6 all go. It is not a
    /// corner: <b>both</b> corpus documents that apply a <c>COLL_LABEL_*</c> name declare a
    /// <c>caption</c> entry, so this arm is the one the corpus actually takes. Measured at
    /// 26.2.4.2 on all five names × two <c>Normal</c> sizes,
    /// <c>probes/rtf-style-r97/genpool3.py</c>: 10 pt, bold, upright, no added spacing.
    /// </remarks>
    [Theory]
    [InlineData("Figure")]
    [InlineData("Text")]
    public void ADeclaredCaptionReplacesTheCaptionPool(string name)
    {
        string styles = @"{\s0\snext0\f0\fs28 Normal;}"
            + @"{\s7\sbasedon9\snext0 " + name + ";}"
            + @"{\s9\sbasedon0\snext9\fs18\b Notes;}"
            + @"{\s8\sbasedon0\snext0\f0\fs20\b caption;}";

        Formatting formatting = First(@"\pard\plain\s7 MARKER\par", styles);
        formatting.Size.ShouldBe(Length.FromPoints(10));
        formatting.Weight.ShouldBe(700);
        Italic(@"\pard\plain\s7 HEAD\par", styles).ShouldBeFalse();

        ParagraphFormat format = Formats(@"\pard\plain\s7 HEAD\par", styles)[0];
        format.SpaceBefore.ShouldBe(Length.Zero);
        format.SpaceAfter.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// An intermediate that is <em>not</em> <em>Caption</em> or <em>Heading</em> is not modelled.
    /// </summary>
    /// <remarks>
    /// <c>List Indent</c> is <c>COLL_CONFRONTATION</c>, whose pool parent is <c>COLL_TEXT</c> —
    /// <em>Text body</em>, which states 7 pt below and 115 % line spacing
    /// (<c>DocumentStylePoolManager.cxx</c>:694-700) and which the import does not reset either.
    /// 26.2.4.2 answers the document's own <c>Normal</c> size with about 8.7 pt more below;
    /// this tree answers <c>\pard\plain</c>'s twelve points. It is left open because the type
    /// here carries no proportional line spacing and because the corpus reach is nil: <b>0 of the
    /// 338 converted <c>.rtf</c></b> apply any <c>COLL_TEXT</c>-parented name
    /// (<c>probes/rtf-style-r97/hasbyname-census.py</c>). Four of the 116 probes in that round
    /// are these, and they are reported as differing rather than as agreement.
    /// </remarks>
    [Fact]
    public void AnIntermediateBelowTextBodyIsNotModelledYet()
        => First(@"\pard\plain\s7 MARKER\par",
                @"{\s0\snext0\f0\fs20 Normal;}{\s7\sbasedon9\snext0 List Indent;}"
                    + @"{\s9\sbasedon0\snext9\fs18\b Notes;}")
            .Size.ShouldBe(Length.FromPoints(12));

    /// <summary>
    /// A name the map answers nothing for keeps <c>\pard\plain</c>'s twelve points.
    /// </summary>
    /// <remarks>
    /// The control for both pool rules, and it is not hypothetical: <c>Quote</c>,
    /// <c>List Paragraph</c> and <c>Normal (Web)</c> are in <c>ConvertStyleName</c>'s map with an
    /// <em>empty</em> Writer name (<c>StyleSheetTable.cxx</c>:1794, :1883-1884), so no existing
    /// style is reused and the entry gets no pool parent at all.
    /// </remarks>
    [Theory]
    [InlineData("Quote")]
    [InlineData("List Paragraph")]
    [InlineData("Normal (Web)")]
    [InlineData("Marginalia")]
    [InlineData("Text body indent")]
    public void ANameWriterHasNoStyleForKeepsTheResetSize(string name)
        => First(@"\pard\plain\s7 MARKER\par",
                @"{\s0\snext0\f0\fs20 Normal;}{\s7\sbasedon9\snext0 " + name + ";}"
                    + @"{\s9\sbasedon0\snext9\fs18\b Notes;}")
            .Size.ShouldBe(Length.FromPoints(12));

    /// <summary>The resolved layout format of each body paragraph that carries text.</summary>
    private static IReadOnlyList<ParagraphFormat> Formats(string body, string styles)
    {
        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(Wrap(body, styles))), "styles.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return [.. pages.Paragraphs.Where(p => p.Text.Length > 0).Select(p => p.Format)];
    }

    /// <summary>The face, size and weight one paragraph's text is set in.</summary>
    /// <param name="Family">The family the document asked for, before substitution.</param>
    /// <param name="Size">The em size.</param>
    /// <param name="Weight">400 or 700.</param>
    private readonly record struct Formatting(string? Family, Length Size, int Weight);

    private static Formatting First(string body, string styles = "")
        => Read(body, styles)[0];

    /// <summary>
    /// The formatting of each body paragraph, taking a uniform paragraph's own rather than its
    /// runs — <see cref="PageParagraph.Runs"/> is empty exactly when nothing varies across the text.
    /// </summary>
    private static IReadOnlyList<Formatting> Read(string body, string styles = "")
    {
        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(Wrap(body, styles))), "styles.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return
        [
            .. pages.Paragraphs
                .Where(paragraph => paragraph.Text.Length > 0)
                .Select(paragraph => paragraph.HasRuns
                    ? new Formatting(
                        paragraph.Runs[0].Font?.RequestedFamily,
                        paragraph.Runs[0].EmSize,
                        paragraph.Runs[0].Font?.Weight ?? 400)
                    : new Formatting(
                        paragraph.Font?.RequestedFamily, paragraph.EmSize, paragraph.Font?.Weight ?? 400)),
        ];
    }

    /// <summary>Whether the first body paragraph's text is set in an italic face.</summary>
    private static bool Italic(string body, string styles)
    {
        using DocumentSource source = DocumentSource.FromStream(
            new MemoryStream(Encoding.ASCII.GetBytes(Wrap(body, styles))), "styles.rtf");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        PageParagraph paragraph = pages.Paragraphs.First(p => p.Text.Length > 0);
        return (paragraph.HasRuns ? paragraph.Runs[0].Font : paragraph.Font)?.IsItalic ?? false;
    }

    /// <summary>One body in a document with three faces and an optional stylesheet.</summary>
    /// <remarks>
    /// <c>\deff1</c> rather than <c>\deff0</c>, so "the default font" and "font zero" are two
    /// different answers and a test cannot pass by taking the wrong one.
    /// </remarks>
    private static string Wrap(string body, string styles)
        => @"{\rtf1\ansi\deff1"
           + @"{\fonttbl{\f0\froman Liberation Serif;}{\f1\fswiss Liberation Sans;}"
           + @"{\f2\fmodern Liberation Mono;}}"
           + (styles.Length == 0 ? string.Empty : @"{\stylesheet" + styles + "}")
           + @"\paperw11906\paperh16838\margl1440\margr1440\margt1440\margb1440\sectd"
           + body
           + "}";
}
