using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// Which paragraph owns the space proportional line spacing adds at a paragraph boundary.
/// </summary>
/// <remarks>
/// <para>
/// The answer is the <em>previous</em> one, and it is not the obvious answer. Proportional line
/// spacing widens a line by a percentage of its own text height and the extra sits above the text —
/// but Writer applies that only from a paragraph's second line onwards. <c>SwTextFormatter::CalcRealHeight</c>
/// guards the whole inter-line-spacing switch with <c>if( !IsParaLine() )</c> and says why on the line
/// above it: <em>"Note: for the _first_ line the line spacing of the previous paragraph is applied in
/// SwFlowFrame::CalcUpperSpace()"</em> (<c>sw/source/core/text/itrform2.cxx</c>:2424).
/// </para>
/// <para>
/// So the leading above a paragraph's first line is a property of the paragraph <em>above</em> it.
/// <c>SwFlowFrame::CalcUpperSpace</c> adds <c>nPrevLineSpacing</c> to the gap in both of its branches —
/// the one that sums the two paragraph spacings and the one that takes their maximum
/// (<c>sw/source/core/layout/flowfrm.cxx</c>:1656 and :1722) — and that value comes from
/// <c>GetSpacingValuesOfFrame</c> (<c>sw/source/core/layout/frmtool.cxx</c>:4064) and thence from
/// <c>SwTextFrame::GetLineSpace</c> (<c>sw/source/core/text/txtfrm.cxx</c>:3996), which is
/// <c>GetHeightOfLastLine() × prop / 100 − GetHeightOfLastLine()</c> — the percentage taken against the
/// height of the previous paragraph's <em>last</em> line.
/// </para>
/// <para>
/// Between two paragraphs at the same spacing the distinction is invisible, which is why it survived
/// every comparison this project runs: the leading that leaves one paragraph's first line arrives again
/// as the previous paragraph's, and every baseline lands where it did. It shows only where the spacing
/// or the size changes across the boundary — a 16 pt heading between 11 pt 115%-spaced body paragraphs
/// takes 1.95 pt of the body's leading that this engine used to withhold, and hands none of its own to
/// the paragraph after it. The block is the same height either way, which is why no page break moved and
/// no word-box comparison could see it.
/// </para>
/// <para>
/// A frame's first line has no previous paragraph and so no leading at all, which is the same answer the
/// top-of-page rule already gave: <c>CalcUpperSpace</c> reaches the line-spacing term only when
/// <c>GetPrevFrameForUpperSpaceCalc_</c> finds a previous frame, and at the top of a page or a column
/// there is none.
/// </para>
/// <para>
/// <b>What is <em>below</em> the paragraph does not matter, and that half was missing for sixty rounds.</b>
/// <c>CalcUpperSpace</c> adds <c>nPrevLineSpacing</c> to <c>nUpper</c> in all four of its branches before
/// <c>pOwn</c> is looked at, and <c>pOwn-&gt;IsTextFrame()</c> guards only the frame's <em>own</em> leading
/// — so a <c>SwTabFrame</c> takes the paragraph above's leading exactly as a text frame does. This engine
/// handed it only from paragraph to paragraph, so every table under a proportionally-spaced paragraph
/// started a point or so too high. Measured on
/// <c>097_Business_Case_Template_Elegant_Layout_3ba9cbf2.docx</c>: four such boundaries, +0.95, +1.00,
/// +1.05 and +1.00 pt, which is the whole of that document's 3.36 pt deficit and the reason its trailing
/// empty paragraph fitted on page 1 here and takes a second page on the reference. 275 boundaries in 85
/// of the corpus's 271 <c>.docx</c>; 80 renderings changed, one page count, no regression.
/// <c>probes/words-r61/</c> and <see cref="XRay.Office.WordProcessing.Layout.PageTable"/>'s placement in
/// <c>Paginator</c> and <c>FlowLayouter</c>.
/// </para>
/// <para>
/// The converse is not true and is the control: <c>GetSpacingValuesOfFrame</c>
/// (<c>sw/source/core/layout/frmtool.cxx</c>:4060) reads a line spacing only when
/// <c>rFrame.IsTextFrame()</c>, so a <em>table</em> hands nothing down to whatever follows it. Both
/// directions are in <c>table-paragraph-leading.docx</c>.
/// </para>
/// </remarks>
internal static class ParagraphLeading
{
    /// <summary>
    /// The leading a paragraph hands down to whatever follows it, or zero when nothing does.
    /// </summary>
    /// <param name="paragraph">The paragraph above, or null when there is none.</param>
    /// <param name="spacing">Its line spacing; an <c>atLeast</c> paragraph hands down nothing.</param>
    /// <remarks>
    /// Taken from the last line rather than from the format, because that is what Writer measures it
    /// against — a paragraph whose last line is taller than its first, which is any paragraph whose
    /// runs vary in size, hands down more.
    /// </remarks>
    public static Length Below(LaidOutParagraph? paragraph, LineSpacingRule spacing)
        => RaisesEveryLine(spacing) || paragraph is not { Lines.Count: > 0 }
            ? Length.Zero
            : paragraph.Lines[^1].SpaceAbove;

    /// <summary>
    /// Whether the space above this paragraph's text belongs to the line rather than to the boundary.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SwTextFormatter::CalcRealHeight</c> runs two switches, and only the second is guarded by
    /// <c>if( !IsParaLine() )</c>. The first — <c>SvxLineSpaceRule</c>, which is OOXML's
    /// <c>atLeast</c> and <c>exact</c> — applies to <em>every</em> line, a paragraph's first included
    /// (<c>sw/source/core/text/itrform2.cxx</c>:2397 against :2425). The second —
    /// <c>SvxInterLineSpaceRule</c>, which is proportional spacing and leading — is the one the
    /// paragraph above owns.
    /// </para>
    /// <para>
    /// So an <c>atLeast</c> line keeps its raise wherever it sits and hands none of it on:
    /// <c>SwTextFrame::GetLineSpace</c> answers only for <c>Prop</c> and <c>Fix</c>
    /// (<c>txtfrm.cxx</c>:3996), and <c>Min</c> is neither. Measured on 26.2.4.2 with
    /// <c>w:line="400" w:lineRule="atLeast"</c> over 11 pt Cambria: the reference's first baseline is
    /// 17.25 pt below the body's top edge — a 20.00 pt line with the 7.35 pt raise above the text —
    /// where a stripped raise puts it at 9.90. <c>exact 400</c> is the control and agrees on both
    /// sides at 16.00.
    /// </para>
    /// <para>
    /// <see cref="LineSpacingMode.Exact"/> is not named here because it never produces a raise in the
    /// first place — <c>ParagraphLayouter.BaselineFrom</c> gives an exact line the four-fifths ascent
    /// and no space above at all, so both answers are the same and stating the narrower one keeps the
    /// claim to what was measured.
    /// </para>
    /// </remarks>
    private static bool RaisesEveryLine(LineSpacingRule spacing)
        => spacing.Mode == LineSpacingMode.AtLeast;

    /// <summary>
    /// A paragraph's line as it is drawn, with the leading removed from its first line.
    /// </summary>
    /// <param name="box">The line box, as the layouter produced it.</param>
    /// <param name="isFirstOfParagraph">Whether it is the paragraph's own first line.</param>
    /// <param name="isFirstInFrame">Whether it is the first content in the page, column or frame.</param>
    /// <param name="spacing">
    /// The paragraph's line spacing, because only the inter-line kinds are strippable. See
    /// <see cref="RaisesEveryLine"/>.
    /// </param>
    /// <remarks>
    /// Two rules with one consequence. A paragraph's first line never carries the leading, because it is
    /// the previous paragraph's to give; and the first line in a frame does not carry it either, because
    /// Writer drops the whole upper space at the top of a text frame — which catches the case the first
    /// rule does not, a paragraph carried over from the previous page whose continuation line would
    /// otherwise start a page a line's leading below the margin.
    /// </remarks>
    public static LineBox AsDrawn(
        LineBox box, bool isFirstOfParagraph, bool isFirstInFrame, LineSpacingRule spacing)
        => !RaisesEveryLine(spacing) && (isFirstOfParagraph || isFirstInFrame)
            ? box.WithoutSpaceAbove()
            : box;
}
