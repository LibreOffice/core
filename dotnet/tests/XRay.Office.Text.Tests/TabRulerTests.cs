using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Checks where a tab advances to, and where the text after it sits.
/// </summary>
/// <remarks>
/// Against a measurement of one point per character rather than against a font, because what is under test
/// is the arithmetic of the stops and not the shaping. That makes every expected number legible: a stretch
/// of five characters is five points wide, so a right stop at 100 pt puts it at 95.
/// </remarks>
public sealed class TabRulerTests
{
    /// <summary>A character is a point wide, so a stretch's width is its length.</summary>
    private static Length Measure(int from, int to) => Length.FromPoints(Math.Max(to - from, 0));

    private static ParagraphFormat With(params TabStop[] stops) => new()
    {
        TabStops = stops,
        DefaultTabInterval = Length.FromPoints(10),
    };

    [Fact]
    public void ATabWithoutStopsLandsOnTheNextMultipleOfTheInterval()
    {
        ParagraphFormat format = With();

        // "abc" is 3 pt wide, so the tab at 3 pt goes to 10; "de" then ends at 12, and the next tab to 20.
        List<TabbedSegment> segments = TabRuler.Segments("abc\tde\tf", 0, 8, format, Measure);

        segments.Count.ShouldBe(3);
        segments[0].Left.ShouldBe(Length.Zero);
        segments[1].Left.ShouldBe(Length.FromPoints(10));
        segments[2].Left.ShouldBe(Length.FromPoints(20));
    }

    [Fact]
    public void ATabLandingExactlyOnAStopAdvancesToTheNextOne()
    {
        // A tab always moves. "abcdefghij" is exactly ten points, so the tab after it sits on the first
        // default stop — and must go to the second, or a tab would take no room and a table would collapse.
        List<TabbedSegment> segments =
            TabRuler.Segments("abcdefghij\tx", 0, 12, With(), Measure);

        segments[1].Left.ShouldBe(Length.FromPoints(20));
    }

    [Fact]
    public void AnExplicitStopBeatsTheInterval()
    {
        List<TabbedSegment> segments =
            TabRuler.Segments("ab\tcd", 0, 5, With(new TabStop(Length.FromPoints(7))), Measure);

        segments[1].Left.ShouldBe(Length.FromPoints(7));
    }

    [Fact]
    public void ARightStopPutsTheStretchsEndOnIt()
    {
        List<TabbedSegment> segments = TabRuler.Segments(
            "ab\tcdef",
            0,
            7,
            With(new TabStop(Length.FromPoints(30), TabAlignment.Right)),
            Measure);

        // Four characters, so four points: the stretch starts at 26 and ends on the stop.
        segments[1].Left.ShouldBe(Length.FromPoints(26));
        segments[1].Right.ShouldBe(Length.FromPoints(30));
    }

    [Fact]
    public void ACentreStopPutsTheStretchsMiddleOnIt()
    {
        List<TabbedSegment> segments = TabRuler.Segments(
            "ab\tcdef",
            0,
            7,
            With(new TabStop(Length.FromPoints(30), TabAlignment.Centre)),
            Measure);

        segments[1].Left.ShouldBe(Length.FromPoints(28));
        segments[1].Right.ShouldBe(Length.FromPoints(32));
    }

    [Fact]
    public void ADecimalStopPutsTheSeparatorOnIt()
    {
        List<TabbedSegment> segments = TabRuler.Segments(
            "ab\t12.75",
            0,
            8,
            With(new TabStop(Length.FromPoints(30), TabAlignment.DecimalSeparator)),
            Measure);

        // Two digits before the point, so the stretch starts two points before the stop — and the digits
        // after it hang past, which is the whole point of the alignment.
        segments[1].Left.ShouldBe(Length.FromPoints(28));
        segments[1].Right.ShouldBe(Length.FromPoints(33));
    }

    [Fact]
    public void ADecimalStopWithNoSeparatorAlignsOnTheEnd()
    {
        // Which is what lines a column of whole numbers up with a column of fractional ones.
        List<TabbedSegment> segments = TabRuler.Segments(
            "ab\t125",
            0,
            6,
            With(new TabStop(Length.FromPoints(30), TabAlignment.DecimalSeparator)),
            Measure);

        segments[1].Right.ShouldBe(Length.FromPoints(30));
    }

    [Fact]
    public void AStopThatCannotHoldItsTextDoesNotDrawBackwards()
    {
        // A right stop at 12 pt with five points of text would start at 7 — behind the ten points already
        // set. The text continues from the pen instead, because the alternative is drawing over the column
        // before it. A stop behind the pen never even arises: the lookup only returns stops beyond it.
        List<TabbedSegment> segments = TabRuler.Segments(
            "abcdefghij\tabcde",
            0,
            16,
            With(new TabStop(Length.FromPoints(12), TabAlignment.Right)),
            Measure);

        segments[1].Left.ShouldBe(Length.FromPoints(10));
    }

    [Fact]
    public void TheWidthOfALineIsWhereItsLastStretchEnds()
    {
        TabRuler.WidthOf("ab\tcd", 0, 5, With(), Measure).ShouldBe(Length.FromPoints(12));
    }

    [Fact]
    public void ALineWithoutTabsNeedsNoneOfIt()
    {
        TabRuler.HasTab("plain text", 0, 10).ShouldBeFalse();
        TabRuler.HasTab("plain\ttext", 0, 10).ShouldBeTrue();

        // The range matters, not the string: a tab past the line's end is the next line's problem.
        TabRuler.HasTab("plain\ttext", 0, 5).ShouldBeFalse();
    }

    [Fact]
    public void AStretchCarriesTheBlankItsTabAdvancedAcross()
    {
        List<TabbedSegment> segments = TabRuler.Segments(
            "ab\tcd", 0, 5, With(new TabStop(Length.FromPoints(30), Leader: '.')), Measure);

        // The first stretch was placed by no tab at all, so it has no blank before it and no leader.
        segments[0].GapLeft.ShouldBe(segments[0].Left);
        segments[0].HasLeader.ShouldBeFalse();

        // The second was: the tab began where "ab" ended and carried the pen to the stop, so the blank
        // is [2, 30) — which is what a dot leader has to fill.
        segments[1].GapLeft.ShouldBe(Length.FromPoints(2));
        segments[1].Left.ShouldBe(Length.FromPoints(30));
        segments[1].GapWidth.ShouldBe(Length.FromPoints(28));
        segments[1].Leader.ShouldBe('.');
        segments[1].HasLeader.ShouldBeTrue();
    }

    [Fact]
    public void ARightStopPutsTheBlankBeforeTheTextRatherThanBeforeTheStop()
    {
        // The leader of a contents line runs up to where the page number starts, not up to the stop the
        // number's *end* sits on — so the blank has to be measured against the placed text.
        List<TabbedSegment> segments = TabRuler.Segments(
            "ab\tcd",
            0,
            5,
            With(new TabStop(Length.FromPoints(30), TabAlignment.Right, '.')),
            Measure);

        segments[1].Left.ShouldBe(Length.FromPoints(28));
        segments[1].GapWidth.ShouldBe(Length.FromPoints(26));
    }

    [Fact]
    public void ADefaultStopHasNoLeaderAndASpaceFillIsNoneAtAll()
    {
        // Only an explicit stop can carry a fill: Writer sets cFill to 0 for the stops it synthesises on
        // the default grid (sw/source/core/text/txttab.cxx:218).
        TabRuler.Segments("ab\tcd", 0, 5, With(), Measure)[1].HasLeader.ShouldBeFalse();

        // And both formats spell "no leader" as a space on a stop that has the attribute at all.
        TabRuler.Segments(
                "ab\tcd", 0, 5, With(new TabStop(Length.FromPoints(30), Leader: ' ')), Measure)[1]
            .HasLeader.ShouldBeFalse();
    }

    [Fact]
    public void ARightStopPastTheLineEdgeIsHonouredAtTheEdge()
    {
        // `nRight = std::min(GetTabPos(), rInf.Width())` — SwTabPortion::PostFormat,
        // sw/source/core/text/txttab.cxx:503. Without it "cd" ends at 30 pt on a line 20 pt wide, the
        // line does not fit, and a contents entry breaks into one line per stretch.
        ParagraphFormat format = With(new TabStop(Length.FromPoints(30), TabAlignment.Right, '.'));

        TabRuler.Segments("ab\tcd", 0, 5, format, Measure)[1]
            .Left.ShouldBe(Length.FromPoints(28));

        TabRuler.Segments("ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(20))[1]
            .Left.ShouldBe(Length.FromPoints(18));

        TabRuler.WidthOf("ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(20))
            .ShouldBe(Length.FromPoints(20));
    }

    [Fact]
    public void ACentredStopPastTheLineEdgeCentresOnTheEdge()
    {
        ParagraphFormat format = With(new TabStop(Length.FromPoints(40), TabAlignment.Centre));

        // "cd" is 2 pt, so centring it on 20 pt puts its left edge at 19.
        TabRuler.Segments("ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(20))[1]
            .Left.ShouldBe(Length.FromPoints(19));
    }

    [Fact]
    public void ALeftStopPastTheLineEdgeIsNotClamped()
    {
        // Writer breaks the line at such a tab rather than pulling it back — PreFormat's `bFull`, same
        // file — so the ruler must leave it where it was declared and let the filler decide.
        ParagraphFormat format = With(new TabStop(Length.FromPoints(30)));

        TabRuler.Segments("ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(20))[1]
            .Left.ShouldBe(Length.FromPoints(30));
    }

    [Fact]
    public void ATabEndingTheParagraphPastTheEdgeDoesNotWidenTheLineItIsFittedBy()
    {
        // `bFull = false` where `bTabCompat && bAtParaEnd && GetTabPos() >= nTextFrameWidth` —
        // SwTabPortion::PreFormat, sw/source/core/text/txttab.cxx:448-458. The tab is the paragraph's
        // last character, so nothing follows it that a second line could hold, and Writer keeps the line.
        // Measured on 150_5300_13_chg10.doc, whose footer "Chap 4\t\t<page>\t" broke after its second tab
        // and put the page number on a line of its own at the left margin.
        ParagraphFormat format = With(new TabStop(Length.FromPoints(20), TabAlignment.Right));

        // "ab" ends at 2, the tab takes the right stop at the edge and "cd" ends on it at 20. The trailing
        // tab has no stop left, takes the next default interval at 30, and overruns.
        TabRuler.WidthOf(
                "ab\tcd\t", 0, 6, format, Measure, rightEdge: Length.FromPoints(20),
                countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(20));

        // The drawn width is Writer's own: PreFormat sets the portion's width before it takes the break
        // back and does not take that back with it.
        TabRuler.WidthOf("ab\tcd\t", 0, 6, format, Measure, rightEdge: Length.FromPoints(20))
            .ShouldBe(Length.FromPoints(30));
    }

    [Fact]
    public void ATabInsideTheParagraphPastTheEdgeStillBreaksTheLine()
    {
        // The other half of `bAtParaEnd`: a tab with text after it has somewhere to break to, so the
        // forgiveness must not reach it. "ef" follows the trailing tab here and the line is full.
        ParagraphFormat format = With(new TabStop(Length.FromPoints(20), TabAlignment.Right));

        TabRuler.WidthOf(
                "ab\tcd\tef", 0, 8, format, Measure, rightEdge: Length.FromPoints(20),
                countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(32));
    }

    [Fact]
    public void TabOverSpacingBreaksTheLineAtThatTabInstead()
    {
        // The branch above the rescue — txttab.cxx:429-440 — returns `bFull = true` for a left stop at
        // or past the frame and never falls through, so a file writerfilter read never reaches it. The
        // same footer therefore keeps its page number on one line in a .doc and not in a .docx.
        ParagraphFormat format = With(new TabStop(Length.FromPoints(20), TabAlignment.Right)) with
        {
            TabsOverSpacing = true,
        };

        TabRuler.WidthOf(
                "ab\tcd\t", 0, 6, format, Measure, rightEdge: Length.FromPoints(20),
                countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(30));
    }

    /// <summary>
    /// A trailing right stop past the line's end lets its text use the paragraph's end indent.
    /// </summary>
    /// <remarks>
    /// <c>SwTextFormatInfo::GetLineWidth</c> (<c>sw/source/core/text/inftxt.cxx</c>:2132-2182) answers
    /// <c>Width() - X()</c> until there is a pending tab whose stop is past <c>Width()</c>, and then
    /// answers the frame's width less the <em>left</em> margin alone — <em>"text is allowed to use the
    /// full text frame area to the right (RR above, but not LL)"</em>. Here the line ends at 20 pt, the
    /// paragraph's end indent is 4 pt so the frame's edge is 24, and the stop at 24 is past the line —
    /// so <c>cd</c> is fitted against 24 and the width comes back 4 pt short of its unstretched end.
    /// </remarks>
    [Fact]
    public void AStopPastTheLineEndGivesTheEndIndentBack()
    {
        ParagraphFormat format = With(new TabStop(Length.FromPoints(24), TabAlignment.Right)) with
        {
            TabsOverSpacing = true,
            EndIndent = Length.FromPoints(4),
        };

        // "ab" ends at 2 and "cd" is 2 wide, so the unstretched end is 4; less the 4 pt given back,
        // floored at where the tab began, because the give-back is the trailing stretch's alone.
        TabRuler.WidthOf(
                "ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(24),
                countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(2));
    }

    /// <summary>
    /// The text in front of the tab cannot borrow the indent to stay on the line.
    /// </summary>
    /// <remarks>
    /// Writer reaches the wider limit only once the right tab is the pending one
    /// (<c>GetLastTab()</c>, <c>sw/source/core/text/inftxt.cxx</c>:2142-2144), so everything before
    /// that tab was already fitted against <c>rInf.Width()</c>. Subtracting the indent from the
    /// whole line instead lets a long title stay on a line the reference wraps — measured on
    /// <c>SPA-02_mcar_part-2_and_IS_v2.9</c>, whose table of contents this tree fitted one word too
    /// far and then pushed the bare page number onto a line of its own.
    /// </remarks>
    [Fact]
    public void TheTextBeforeTheTabDoesNotGetTheIndent()
    {
        ParagraphFormat format = With(new TabStop(Length.FromPoints(24), TabAlignment.Right)) with
        {
            TabsOverSpacing = true,
            EndIndent = Length.FromPoints(4),
        };

        // A 22 pt title runs past a 20 pt line on its own; the trailing "cd" may reach into the
        // indent, but the answer must still report the title's own overrun rather than 22 + 2 - 4.
        TabRuler.WidthOf(
                new string('a', 22) + "\tcd", 0, 25, format, Measure,
                rightEdge: Length.FromPoints(24), countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(22));
    }

    /// <summary>The control: a stop inside the line gives nothing back.</summary>
    /// <remarks>
    /// This is the direction the corpus settles as well — moving <c>02_mcar</c>'s TOC stops back to
    /// their own line end makes <em>the reference</em> wrap exactly as this tree did before the rule.
    /// </remarks>
    [Fact]
    public void AStopInsideTheLineGivesNothingBack()
    {
        ParagraphFormat format = With(new TabStop(Length.FromPoints(20), TabAlignment.Right)) with
        {
            TabsOverSpacing = true,
            EndIndent = Length.FromPoints(4),
        };

        TabRuler.WidthOf(
                "ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(24),
                countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(4));
    }

    /// <summary>
    /// And a document without <c>TabOverSpacing</c> gives nothing back either.
    /// </summary>
    /// <remarks>
    /// <c>GetLineWidth</c> returns before the pending tab is even looked at unless the document carries
    /// <c>TAB_OVER_MARGIN</c> or <c>TAB_OVER_SPACING</c>. A binary <c>.doc</c> carries the first, whose
    /// arm is a flat 558 mm rather than the frame's edge and is deliberately not modelled, so a
    /// paragraph that states neither keeps the line's own boundary.
    /// </remarks>
    [Fact]
    public void WithoutTabOverSpacingNothingIsGivenBack()
    {
        ParagraphFormat format = With(new TabStop(Length.FromPoints(24), TabAlignment.Right)) with
        {
            EndIndent = Length.FromPoints(4),
        };

        TabRuler.WidthOf(
                "ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(24),
                countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(4));
    }

    /// <summary>The drawn width is untouched by any of it.</summary>
    /// <remarks>
    /// The give-back is a fitting rule: Writer settles the tab's real width in
    /// <c>SwTabPortion::PostFormat</c> afterwards, so the line is still <em>drawn</em> with its text
    /// ending on the stop. Subtracting the indent from the placed width would draw every such entry
    /// 4 pt short of where it was measured to.
    /// </remarks>
    [Fact]
    public void ThePlacedWidthIsUntouched()
    {
        ParagraphFormat format = With(new TabStop(Length.FromPoints(24), TabAlignment.Right)) with
        {
            TabsOverSpacing = true,
            EndIndent = Length.FromPoints(4),
        };

        TabRuler.WidthOf("ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(24))
            .ShouldBe(Length.FromPoints(24));
    }

    /// <summary>A stretch a <em>left</em> stop placed is never given the indent.</summary>
    /// <remarks>
    /// <c>GetLineWidth</c>'s <c>TabOverSpacing</c> arm narrows the answer again for a pending
    /// <c>TabLeft</c>, and a left stop is never deferred in the first place — only a right, centred or
    /// decimal one is settled after its text is fitted.
    /// </remarks>
    [Fact]
    public void ALeftStopIsNotGivenTheIndent()
    {
        ParagraphFormat format = With(new TabStop(Length.FromPoints(24), TabAlignment.Left)) with
        {
            TabsOverSpacing = true,
            EndIndent = Length.FromPoints(4),
        };

        TabRuler.WidthOf(
                "ab\tcd", 0, 5, format, Measure, rightEdge: Length.FromPoints(24),
                countsDeferredStretch: false)
            .ShouldBe(Length.FromPoints(26));
    }

    [Fact]
    public void ATabReachingTheLineEdgeEndsTheLineAtItself()
    {
        // `SwTabPortion::PreFormat` runs once per tab portion, and a tab that finds itself at or past
        // the line's boundary sets bFull, zeroes itself and drops the rest of the chain
        // (txttab.cxx:462-476) — so the line ends in front of the tab and not at the last break
        // opportunity behind it. "ab" ends at 2, the tabs take 10, 20 and 30; the one landing on 20
        // reaches the edge and is not the paragraph's last character.
        TabRuler.BreakAt(
                "ab\t\t\t", 0, With(), Measure, isFirstLine: true,
                lineEdge: Length.FromPoints(20), rightEdge: Length.FromPoints(20))
            .ShouldBe(3);
    }

    [Fact]
    public void TheLastTabOfTheParagraphStillEndsNoLine()
    {
        // The same three tabs against a wider line: only the last of them reaches the edge, and
        // `bAtParaEnd` forgives exactly that one.
        TabRuler.BreakAt(
                "ab\t\t\t", 0, With(), Measure, isFirstLine: true,
                lineEdge: Length.FromPoints(30), rightEdge: Length.FromPoints(30))
            .ShouldBeNull();
    }

    [Fact]
    public void AnAlignedStopNeverEndsTheLine()
    {
        // A right, centred or decimal stop is settled in PostFormat with the text after it already
        // fitted, and never sets bFull.
        TabRuler.BreakAt(
                "ab\tcd", 0, With(new TabStop(Length.FromPoints(20), TabAlignment.Right)), Measure,
                isFirstLine: true, lineEdge: Length.FromPoints(20), rightEdge: Length.FromPoints(20))
            .ShouldBeNull();
    }

    [Fact]
    public void ATabAtTheLineStartIsFilledRatherThanBrokenFor()
    {
        // `if (rInf.GetIdx() == rInf.GetLineStart())` — PreFormat fills the line with the tab instead
        // of opening an empty one, and a rule that broke there would not terminate.
        TabRuler.BreakAt(
                "ab\t\t\t", 3, With(), Measure, isFirstLine: false,
                lineEdge: Length.FromPoints(10), rightEdge: Length.FromPoints(10))
            .ShouldBeNull();
    }
}
