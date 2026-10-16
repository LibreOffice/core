using XRay.Office.Core.Units;

namespace XRay.Office.Text.Layout;

/// <summary>One stretch of a line between tabs, and where it sits.</summary>
/// <param name="Start">Its first character, as an index into the paragraph's text.</param>
/// <param name="End">One past its last character.</param>
/// <param name="Left">Where it starts, measured from the line's own left edge.</param>
/// <param name="Width">How wide its text is.</param>
/// <param name="GapLeft">
/// Where the tab that introduced the segment began, in the same coordinates as <paramref name="Left"/> —
/// so <c>[GapLeft, Left)</c> is the blank the tab advanced across. Equal to <paramref name="Left"/> for
/// the first stretch of a line, which no tab introduced.
/// </param>
/// <param name="Leader">
/// The character filling that blank, or <c>'\0'</c> for none. It belongs to the stop rather than to the
/// text, which is why it is carried on the stretch the stop placed instead of on the paragraph.
/// </param>
/// <param name="Deferred">
/// True when the stop that placed the stretch is one whose blank could only be measured <em>after</em>
/// the stretch was — a right, centred or decimal stop, all of which need the width of the text that
/// follows them before they know how far to advance. Writer settles exactly those in
/// <c>SwTabPortion::PostFormat</c> rather than in <c>PreFormat</c>, so the text after such a tab is
/// fitted to the line while the tab is still one twip wide. See <see cref="TabRuler.WidthOf"/>.
/// </param>
/// <param name="Stop">
/// Where the stop that placed the stretch was declared, before any clamping — which is not
/// recoverable from <paramref name="Left"/>, since an aligned stop's text is placed relative to it
/// and a stop past the frame's edge is honoured at the edge instead. Null for the first stretch of a
/// line, which no stop placed. <see cref="TabRuler.WidthOf"/> needs it to tell a stop inside the
/// line from one out in the paragraph's end indent, which Writer fits differently.
/// </param>
public readonly record struct TabbedSegment(
    int Start, int End, Length Left, Length Width, Length GapLeft = default, char Leader = '\0',
    bool Deferred = false, Length? Stop = null)
{
    /// <summary>Where the segment ends.</summary>
    public Length Right => Left + Width;

    /// <summary>True when the segment holds no characters, which an adjacent pair of tabs produces.</summary>
    public bool IsEmpty => End <= Start;

    /// <summary>How wide the blank before the segment is, which is what a leader fills.</summary>
    public Length GapWidth => Left - GapLeft;

    /// <summary>True when a leader was asked for and there is room to draw any of it.</summary>
    public bool HasLeader => Leader != '\0' && GapWidth > Length.Zero;
}

/// <summary>
/// Resolves the tabs in a line: how far each one advances, and where the text after it sits.
/// </summary>
/// <remarks>
/// <para>
/// A tab is the one character whose width is not a property of the font. It advances to the next stop, so
/// its width depends on where in the line it happens to fall — which means a line holding one cannot be
/// measured by adding up glyph advances, and the same walk has to serve both the measuring and the
/// drawing. Two walks would eventually disagree, and the symptom would be text drawn a little to one side
/// of the column it was measured into.
/// </para>
/// <para>
/// The stops themselves come from the paragraph and are measured from where its text starts, not from the
/// page — so an indented paragraph's stops move with it, which is what makes a tabbed list line up under
/// an indent.
/// </para>
/// </remarks>
public static class TabRuler
{
    /// <summary>
    /// How many segments a line is split into before the rest is measured without tabs.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input. A real line holds a handful of tabs; a generated file can hold
    /// thousands, and each one costs a stop lookup and a measurement.
    /// </remarks>
    public const int MaxSegments = 4096;

    /// <summary>The character a tab is.</summary>
    private const char Tab = '\t';

    /// <summary>
    /// Splits a range of a line at its tabs and places each stretch.
    /// </summary>
    /// <remarks>
    /// The pen never goes backwards: a stop behind the text already set is skipped, which is what Writer
    /// does when a centred or right stop cannot fit what it was given. Without that a long entry in a
    /// tabbed column would draw over the one before it.
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where the line's visible text ends.</param>
    /// <param name="format">The paragraph's stops and default interval.</param>
    /// <param name="widthBetween">Measures a range of the text, tabs excluded.</param>
    /// <param name="isFirstLine">
    /// True for the paragraph's first line. It decides where the line's left edge sits relative to the
    /// stops, which differs by the first-line indent — and a hanging one puts the edge <em>before</em> the
    /// stops' own origin, which is the case <see cref="ParagraphFormat.NextTabStop(Length)"/> treats specially.
    /// </param>
    /// <param name="rightEdge">
    /// Where the text frame's right boundary is, in the same coordinates as the stops, or null to let a
    /// stop stand wherever it was declared. The <em>frame's</em> edge rather than the line's, so a
    /// paragraph's right indent does not pull it in. See <see cref="Place"/>: a right, centred or decimal
    /// stop past this is honoured <em>at</em> it instead.
    /// </param>
    public static List<TabbedSegment> Segments(
        string text,
        int start,
        int end,
        ParagraphFormat format,
        Func<int, int, Length> widthBetween,
        bool isFirstLine = true,
        Length? rightEdge = null)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(format);
        ArgumentNullException.ThrowIfNull(widthBetween);

        int last = Math.Min(end, text.Length);
        List<TabbedSegment> segments = [];

        // Where this line's left edge is in the coordinates the stops are stated in. The pen is kept in
        // those coordinates throughout and converted back only when a segment is recorded, since a stop's
        // position means nothing until the two agree on where zero is.
        Length origin = format.TabLineOffset(isFirstLine);
        Length pen = origin;
        int at = start;

        // The first stretch begins at the line's left edge; every later one begins at a stop.
        TabStop? pending = null;

        while (at <= last && segments.Count < MaxSegments)
        {
            int tab = text.IndexOf(Tab, at);
            int stretchEnd = tab >= 0 && tab < last ? tab : last;

            Length width = widthBetween(at, stretchEnd);
            Length left = pending is { } stop
                ? Place(stop, pen, width, text, at, stretchEnd, widthBetween, rightEdge)
                : pen;

            // `pen` is still where the tab began, so the blank it advanced across is [pen, left). A
            // leader fills that blank; a space fill means none at all, which is how both formats spell
            // "no leader" on a stop that has one syntactically.
            char leader = pending is { Leader: var fill and not ' ' } ? fill : '\0';

            segments.Add(new TabbedSegment(
                at, stretchEnd, left - origin, width, pen - origin, leader,
                Deferred: pending is { Alignment: not TabAlignment.Left },
                Stop: pending?.Position));
            pen = left + width;

            if (stretchEnd >= last) break;

            // The tab itself: it takes no width of its own, it moves the pen to the next stop.
            pending = format.NextTabStop(pen);
            at = stretchEnd + 1;
        }

        return segments;
    }

    /// <summary>The width of a range of a line, with its tabs resolved.</summary>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="start">Where the line starts.</param>
    /// <param name="end">Where the measured text ends.</param>
    /// <param name="format">The paragraph's stops and default interval.</param>
    /// <param name="widthBetween">Measures a range of the text, tabs excluded.</param>
    /// <param name="isFirstLine">As <see cref="Segments"/>'s.</param>
    /// <param name="rightEdge">As <see cref="Segments"/>'s.</param>
    /// <param name="countsDeferredStretch">
    /// True to count the blank a trailing right, centred or decimal stop advanced across, which is the
    /// width the line will actually be drawn at. False to leave it out, which is the width the line is
    /// <em>fitted</em> by: Writer decides whether the text after such a stop fits while the tab is still
    /// one twip wide and only settles its width afterwards, in <c>SwTabPortion::PostFormat</c>
    /// (<c>sw/source/core/text/txttab.cxx</c>). The distinction is the difference between a table of
    /// contents entry whose page number sits at a stop past the paragraph's own right indent — which is
    /// where Writer puts it — and the same entry broken across four lines because the stretch was
    /// counted against a limit it was never tested against. Only the last stretch can be affected: an
    /// earlier stop is settled by the tab that follows it, before that tab's own text is fitted.
    /// </param>
    public static Length WidthOf(
        string text,
        int start,
        int end,
        ParagraphFormat format,
        Func<int, int, Length> widthBetween,
        bool isFirstLine = true,
        Length? rightEdge = null,
        bool countsDeferredStretch = true)
    {
        List<TabbedSegment> segments =
            Segments(text, start, end, format, widthBetween, isFirstLine, rightEdge);

        if (segments.Count == 0) return Length.Zero;

        TabbedSegment last = segments[^1];

        if (!countsDeferredStretch
            && ForgivesTrailingTab(segments, text, format, isFirstLine, rightEdge))
        {
            return last.GapLeft;
        }

        if (countsDeferredStretch || !last.Deferred) return last.Right;

        // Never below `GapLeft`: the give-back is the trailing stretch's alone. Writer reaches the
        // wider limit only once the right tab is the pending one, so everything in front of that tab
        // was already fitted against the line's own width and a long title cannot borrow the indent
        // to stay on the line. Reported as a width rather than as a limit, so the floor is where the
        // tab began.
        return Length.Max(
            last.GapLeft, last.GapLeft + last.Width - GivenBack(last, format, rightEdge));
    }

    /// <summary>
    /// How much of the paragraph's end indent the text after a trailing aligned stop may use.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A right, centred or decimal stop declared past the line's own end gives the end indent
    /// back to the text that follows it.</strong> <c>SwTextFormatInfo::GetLineWidth</c>
    /// (<c>sw/source/core/text/inftxt.cxx</c>:2132-2182) answers <c>Width() - X()</c> until there is
    /// a pending tab whose stop is past <c>Width()</c>, and then answers
    /// <c>nTextFrameWidth - X()</c> instead — where <c>nTextFrameWidth</c> is the frame's width less
    /// the paragraph's <em>left</em> margin alone, under the comment <em>"text is allowed to use the
    /// full text frame area to the right (RR above, but not LL)"</em>. The two differ by exactly the
    /// end indent. The <c>TabLeft</c> special case below it does not apply here, because that arm is
    /// guarded on the pending tab being a left one and a left stop is never deferred.
    /// </para>
    /// <para>
    /// <strong>Measured, on the document it costs a page.</strong> <c>02_mcar_part-2_and_IS_v2.10</c>
    /// declares its <c>TOC1</c>–<c>TOC9</c> with a dot-leadered right stop at 9360 twips — the frame's
    /// right edge — over <c>w:ind w:right="720"</c>, so the stop is 720 twips past the line. A dozen
    /// of its entries' page numbers wrapped onto a second line here and stay on the line in 26.2.4.2,
    /// which shrinks the leader to nothing to keep them there. Deleting the right indent from those
    /// styles and nothing else takes this tree from 313 pages to the reference's 312; moving the stop
    /// back to the line's end instead takes <em>the reference</em> to 313, which is this tree's
    /// answer. One attribute, both directions. <c>probes/tocwrap-r175</c>.
    /// </para>
    /// <para>
    /// <strong>Only the <c>TabOverSpacing</c> arm is modelled.</strong> <c>GetLineWidth</c> needs
    /// <c>TAB_OVER_MARGIN</c> or <c>TAB_OVER_SPACING</c>, and a binary <c>.doc</c> carries the first
    /// rather than the second — under which the limit is not the frame's edge at all but a flat
    /// 558 mm (<c>tdf#158658</c>, <em>"Put content after tab into margin like Word"</em>). That is a
    /// far wider give-back and it is deliberately left: no corpus <c>.doc</c> is known to need it, and
    /// reproducing it would move every <c>.doc</c> with a trailing right tab at once.
    /// </para>
    /// </remarks>
    private static Length GivenBack(TabbedSegment last, ParagraphFormat format, Length? rightEdge)
    {
        if (!format.TabsOverSpacing || format.EndIndent <= Length.Zero) return Length.Zero;
        if (rightEdge is not { } edge || last.Stop is not { } stop) return Length.Zero;

        // `rightEdge` is the frame's, which is the line's plus the end indent, so this is
        // `GetTabPos() > Width()` in the coordinates the stops are stated in.
        return stop > edge - format.EndIndent ? format.EndIndent : Length.Zero;
    }

    /// <summary>
    /// True when the line ends on a tab the paragraph's last character, landing past the frame's edge —
    /// the one tab Writer refuses to break a line for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A left or default stop past the frame <em>does</em> break the line, and
    /// <see cref="Place"/> deliberately leaves it unclamped for that reason. <c>SwTabPortion::PreFormat</c>
    /// then takes the break back for exactly one case: <c>bFull = false</c> where
    /// <c>bTabCompat &amp;&amp; bAtParaEnd &amp;&amp; GetTabPos() &gt;= nTextFrameWidth</c>
    /// (<c>sw/source/core/text/txttab.cxx</c>:448-458). <c>bAtParaEnd</c> is
    /// <c>rInf.GetIdx() + GetLen() == rInf.GetText().getLength()</c>: the tab is the paragraph's final
    /// character, so nothing follows it that a second line could hold. Every Word document has
    /// <c>TAB_COMPAT</c> — the WW8 filter sets it at <c>sw/source/filter/ww8/ww8par.cxx</c>:1949 and
    /// writerfilter with it — so for word processing the compatibility half is always satisfied.
    /// </para>
    /// <para>
    /// <strong>The rescue is unreachable for a file writerfilter read</strong>, and that is
    /// <see cref="ParagraphFormat.TabsOverSpacing"/>: the branch above it returns <c>bFull = true</c> for
    /// a left stop at or past the frame and never falls through (<c>txttab.cxx</c>:429-440). So this is a
    /// <c>.doc</c> and <c>.odt</c> rule, not a <c>.docx</c> one — measured, and the measurement is what
    /// found it. Applying it to every word-processing document instead took
    /// <c>19-06 Assistive Technology TAB Final - 508.docx</c> (words/done-012) from 8 pages to 7, by
    /// keeping a row of form-field checkboxes over stops at 1980 to 9936 twips on one line.
    /// </para>
    /// <para>
    /// Measured on <c>150_5300_13_chg10.doc</c> (words/table-001), whose figure-page footer reads
    /// <c>Chap 4</c>, tab, tab, the page-number field, tab, over stops at 3 in centred and 7 in right in a
    /// text area exactly 7 in wide. The second tab lands the number on the right stop at the frame's edge;
    /// the third has no stop left to find, takes the next default interval at 7.5 in, and overran the line.
    /// We broke there and put the page number on a second line at the left margin — 4 pages of 78 —
    /// where LibreOffice keeps the footer one line high. A two-line footer is a shorter body, and on
    /// page 36 that was enough to push a figure's caption onto a page of its own.
    /// </para>
    /// <para>
    /// <strong>The rescue is that tab's alone.</strong> <c>PreFormat</c> runs once per tab portion and
    /// <c>bAtParaEnd</c> is a statement about <em>that</em> tab, so a line ending in six tabs is six
    /// separate decisions and only the sixth can be forgiven. An earlier one that reached the line's
    /// boundary has already ended the line — see <see cref="BreakAt"/>, which is the other half of this
    /// rule and the half that decides where.
    /// </para>
    /// <para>
    /// Only the <em>fitting</em> width, never the placed one. Writer sets the tab portion's width to
    /// <c>GetTabPos() - rInf.X()</c> before it decides <c>bFull</c> and does not take it back with the
    /// break, so the line really is that wide once drawn — it simply is not a reason to break. The
    /// stretch the tab opened holds no characters, so nothing is drawn out there either way.
    /// </para>
    /// </remarks>
    private static bool ForgivesTrailingTab(
        List<TabbedSegment> segments,
        string text,
        ParagraphFormat format,
        bool isFirstLine,
        Length? rightEdge)
    {
        // Two at least: the first stretch is the line's own start, which no tab introduced.
        if (segments.Count < 2 || rightEdge is not { } edge) return false;

        // A document Word 2013 or later wrote never reaches the rescue: PreFormat's TabOverSpacing
        // branch has already set bFull and broken out.
        if (format.TabsOverSpacing) return false;

        TabbedSegment last = segments[^1];

        // A right, centred or decimal stop is settled in PostFormat and is already forgiven above; this
        // is the left and default case, which PreFormat breaks for.
        if (last.Deferred || !last.IsEmpty || last.Start < text.Length) return false;

        // The stops are stated from the paragraph's tab origin and the stretch from the line's own start.
        return last.Left + format.TabLineOffset(isFirstLine) >= edge;
    }

    /// <summary>
    /// The index of the tab a line starting at a position has to end before, or null when no tab ends it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A tab ends a line at itself, not at the nearest break opportunity behind it.</strong>
    /// Writer builds a line as a chain of portions in document order, and a tab portion that finds
    /// itself at or past the line's boundary sets <c>bFull</c>, zeroes its own width and length and
    /// drops the rest of the chain (<c>SwTabPortion::PreFormat</c>,
    /// <c>sw/source/core/text/txttab.cxx</c>:462-476) — so the line ends immediately in front of the
    /// tab and the tab itself opens the next one. A width-driven filler cannot express that: it asks
    /// the break iterator where a line may end, and between <c>Chap 2</c> and six tabs the only
    /// opportunity is the space, so the whole overrun is charged to the one word in front of it.
    /// </para>
    /// <para>
    /// Measured on <c>150_5300_13_chg12.doc</c> (words/pagination-002), whose <c>Chap 2</c> footer is
    /// exactly that line: stops at 3 in centred and 6 in right, a 0.25 in default interval, a 7 in text
    /// area with 0.25 in of end indent, so the six tabs land at 216, 432, 450, 468, 486 and 504 pt
    /// against a line boundary of 486. The fifth reaches it — <c>bFull</c> is
    /// <c>rInf.Width() &lt;= rInf.X() + PrtWidth()</c>, inclusive — and is not the paragraph's last
    /// character, so Writer ends the line there and the foot is two lines high. Breaking at the space
    /// instead gave three lines and <c>Chap</c> alone on the first.
    /// </para>
    /// <para>
    /// The boundary is the <em>line's</em>, <c>rInf.Width()</c>, and not the frame's — the two differ
    /// by the paragraph's end indent, and the frame's is what <paramref name="rightEdge"/> carries for
    /// <see cref="Place"/>'s benefit.
    /// </para>
    /// <para>
    /// A tab at the line's own start never breaks it: <c>PreFormat</c>'s <c>bFull</c> arm fills the
    /// line with it rather than opening an empty one (<c>txttab.cxx</c>:464-471), and a rule that
    /// broke there would not terminate. Past that tab <c>rInf.X()</c> is the line's whole width, which
    /// is the case <c>txttab.cxx</c>:428 hands to <c>PostFormat</c> — so nothing after it breaks the
    /// line either, and this returns null for such a line.
    /// </para>
    /// </remarks>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="start">Where the line starts.</param>
    /// <param name="format">The paragraph's stops and default interval.</param>
    /// <param name="widthBetween">Measures a range of the text, tabs excluded.</param>
    /// <param name="isFirstLine">As <see cref="Segments"/>'s.</param>
    /// <param name="lineEdge">
    /// The line's own right boundary, in the coordinates the stops are stated in.
    /// </param>
    /// <param name="rightEdge">As <see cref="Segments"/>'s: the frame's edge, for the aligned stops.</param>
    public static int? BreakAt(
        string text,
        int start,
        ParagraphFormat format,
        Func<int, int, Length> widthBetween,
        bool isFirstLine,
        Length lineEdge,
        Length? rightEdge)
    {
        ArgumentNullException.ThrowIfNull(text);
        ArgumentNullException.ThrowIfNull(format);

        List<TabbedSegment> segments =
            Segments(text, start, text.Length, format, widthBetween, isFirstLine, rightEdge);
        Length origin = format.TabLineOffset(isFirstLine);

        for (int index = 1; index < segments.Count; index++)
        {
            TabbedSegment segment = segments[index];

            // A right, centred or decimal stop is settled in PostFormat, with the text after it already
            // fitted to the line, and never sets bFull.
            if (segment.Deferred || segment.Left + origin < lineEdge) continue;

            // The tab that opened the stretch. One at the line's own start is forced onto it.
            int tab = segment.Start - 1;
            if (tab <= start) return null;

            // The paragraph's final tab, which TAB_COMPAT forgives for a document writerfilter did
            // not read.
            if (index == segments.Count - 1
                && ForgivesTrailingTab(segments, text, format, isFirstLine, rightEdge))
            {
                return null;
            }

            return tab;
        }

        return null;
    }

    /// <summary>True when a range holds a tab, and so needs any of this.</summary>
    /// <remarks>
    /// Asked before the machinery is used, because a paragraph without tabs — nearly all of them — should
    /// measure exactly as it did before this existed: one shaped prefix table and a subtraction.
    /// </remarks>
    public static bool HasTab(string text, int start, int end)
    {
        ArgumentNullException.ThrowIfNull(text);

        int at = text.IndexOf(Tab, Math.Max(start, 0));
        return at >= 0 && at < Math.Min(end, text.Length);
    }

    /// <summary>
    /// Where a stretch of text sits relative to the stop that introduced it.
    /// </summary>
    /// <remarks>
    /// A left stop puts the text's start on it, a right stop its end, a centre stop its middle, and a
    /// decimal stop its separator — which is why this needs the stretch's width and, for the decimal case,
    /// its text. A stop already behind the pen cannot be honoured at all, so the text simply continues.
    /// </remarks>
    /// <remarks>
    /// A right, centred or decimal stop declared past the boundary is honoured <em>at</em> the boundary
    /// instead. Writer says so in one line and says why in the comment above it — <em>"If the tab
    /// position is larger than the right margin, it gets scaled down by default"</em>,
    /// <c>SwTabPortion::PostFormat</c>, <c>sw/source/core/text/txttab.cxx</c>:503, where
    /// <c>nRight = std::min(GetTabPos(), rInf.GetTextFrame()-&gt;getFrameArea().Right())</c> for the
    /// <c>TabOverSpacing</c> setting every writerfilter document carries, and
    /// <c>std::min(GetTabPos(), rInf.Width())</c> only for a document carrying neither compatibility
    /// flag. So the boundary is the <em>frame's</em> right edge and not the line's: the paragraph's own
    /// right indent does not move it, and a stop past that indent is honoured out in the indent. Callers
    /// pass that edge; see <see cref="ParagraphFormat.ClampsTabsAtLineEdge"/> for what it costs to
    /// confuse the two. A <em>left</em> stop past the boundary is deliberately not clamped: Writer
    /// breaks the line there instead (<c>PreFormat</c>, same file, sets <c>bFull</c>), which is what
    /// happens here already.
    /// </remarks>
    private static Length Place(
        TabStop stop,
        Length pen,
        Length width,
        string text,
        int start,
        int end,
        Func<int, int, Length> widthBetween,
        Length? rightEdge = null)
    {
        Length position = rightEdge is { } edge && stop.Position > edge && stop.Alignment != TabAlignment.Left
            ? edge
            : stop.Position;

        Length left = stop.Alignment switch
        {
            TabAlignment.Right => position - width,
            TabAlignment.Centre => position - (width / 2.0),
            TabAlignment.DecimalSeparator =>
                position - widthBetween(start, SeparatorIn(text, start, end)),
            _ => position,
        };

        return left < pen ? pen : left;
    }

    /// <summary>
    /// Where the decimal separator in a stretch is, or its end when it has none.
    /// </summary>
    /// <remarks>
    /// Both spellings, because the separator is a comma across most of Europe and a document does not say
    /// which it meant. Taking the <em>last</em> one: <c>1.234,56</c> uses both, and it is the comma that
    /// separates the fraction. A stretch with no separator aligns on its end, which is what makes a column
    /// of whole numbers line up with a column of fractional ones.
    /// </remarks>
    private static int SeparatorIn(string text, int start, int end)
    {
        int last = Math.Min(end, text.Length);
        int found = -1;

        for (int at = start; at < last; at++)
        {
            if (text[at] is '.' or ',') found = at;
        }

        return found >= 0 ? found : last;
    }
}
