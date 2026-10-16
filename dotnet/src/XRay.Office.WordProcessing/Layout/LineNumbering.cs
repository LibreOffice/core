using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Shaping;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// A document's margin line numbers: the counting rule and the face they are drawn in.
/// </summary>
/// <remarks>
/// <para>
/// Word states this per section, in <c>w:sectPr/w:lnNumType</c>, and Writer holds it per <em>document</em>
/// — <c>SwDoc::GetLineNumberInfo</c> returns one <c>SwLineNumberInfo</c> for the whole file. The DOCX
/// importer says so in as many words: <em>"line numbering in Writer is a global document setting, in Word
/// is a section setting; if line numbering is switched on anywhere in the document it's set at the global
/// settings"</em> (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:1213). So this sits on
/// <see cref="PaginationOptions"/> beside the other document-wide choices rather than on a section, which
/// is not a simplification but the behaviour being reproduced.
/// </para>
/// <para>
/// <b>What is numbered.</b> Body text lines, and only those: not a header, not a footer, not a footnote,
/// not a text frame (<c>m_bCountInFlys</c> is false by default and the DOCX importer sets
/// <c>CountLinesInFrames</c> false explicitly), and — measured on the reference rather than read — not a
/// table. On page 11 of <c>xx_SETIS_PWS_template_10.19.22.docx</c> the numbers run 364…384 down to the
/// line above the table and resume at 385 on the first body line below it, so the table's own lines
/// neither carry a number nor advance the count. An empty paragraph <em>is</em> numbered
/// (<c>m_bCountBlankLines</c> defaults true, and 366, 368 and 371 on that page are blank).
/// </para>
/// <para>
/// <b>Where.</b> Right-aligned, its right edge <see cref="Distance"/> in from the text's left edge, on the
/// line's own baseline. Right-aligned is measured, not assumed: on that document the one-digit numbers sit
/// at x=52.95, the two-digit ones at 47.95 and the three-digit ones at 42.95, which is one 5 pt digit
/// advance apart each time and a right edge fixed at 57.95 pt against a 72.1 pt text edge — 0.5 cm, which
/// is <c>SwLineNumberInfo</c>'s own default (<c>o3tl::toTwips(5, mm)</c>, <c>lineinfo.cxx</c>:55) and the
/// value the importer substitutes when <c>w:distance</c> is absent (<c>DomainMapper.cxx</c>:2818).
/// </para>
/// <para>
/// <b>The size is the document's default character size, not the paragraph's.</b> Writer's
/// <c>Line Numbering</c> character style is created with no attributes at all
/// (<c>DocumentStylePoolManager.cxx</c>:1593 falls through the switch), so it inherits the document's
/// default. Established on four authored probes rendered through 26.2.4.2: with no <c>w:sz</c> in
/// <c>docDefaults</c> the numbers come out 10 pt whether <c>Normal</c> says 12 pt or 20 pt, and with
/// <c>docDefaults</c> at 10 pt they come out 10 pt. One thing those probes show that is <em>not</em>
/// modelled here: a <c>docDefaults</c> of 20, 24 or 32 pt all draw at 11.70 pt, so LibreOffice caps the
/// size somewhere. No corpus document states a default size, so the cap is recorded and left alone rather
/// than guessed at.
/// </para>
/// </remarks>
public sealed record LineNumbering
{
    /// <summary>Writer's default distance from the text edge: 0.5 cm.</summary>
    /// <remarks>
    /// <c>SwLineNumberInfo::SwLineNumberInfo</c> initialises <c>m_nPosFromLeft</c> to 5 mm in twips, and
    /// the DOCX importer puts the same value in when <c>w:distance</c> is absent.
    /// </remarks>
    public static Length DefaultDistance { get; } = Length.FromTwips(283);

    /// <summary>Writer's default size for the numbers, being its default character size.</summary>
    public static Length DefaultEmSize { get; } = Length.FromPoints(10);

    /// <summary>The face the numbers are drawn in.</summary>
    public required OpenTypeFace Face { get; init; }

    /// <summary>The face's reference, for a backend that has to name or embed it.</summary>
    public FontReference? Font { get; init; }

    /// <summary>How big the numbers are drawn.</summary>
    public Length EmSize { get; init; } = DefaultEmSize;

    /// <summary>How they are shaped.</summary>
    public ShapingOptions Shaping { get; init; }

    /// <summary>
    /// Which lines carry a printed number; every line is still counted.
    /// </summary>
    /// <remarks>
    /// <c>w:countBy</c>, Writer's <c>m_nCountBy</c>. A count of five prints on the fifth, tenth and
    /// fifteenth line and on no others — the numbers between exist and are simply not drawn, which is why
    /// this filters the drawing rather than the counting.
    /// </remarks>
    public int CountBy { get; init; } = 1;

    /// <summary>The number the first counted line takes.</summary>
    /// <remarks><c>w:start</c>, which Word writes as the first number rather than as an offset.</remarks>
    public int Start { get; init; } = 1;

    /// <summary>How far the numbers' right edge sits in from the text's left edge.</summary>
    public Length Distance { get; init; } = DefaultDistance;

    /// <summary>True when the count begins again on every page.</summary>
    /// <remarks>
    /// <c>w:restart="newPage"</c> and nothing else: the importer maps <c>continuous</c> and
    /// <c>newSection</c> alike to false (<c>DomainMapper.cxx</c>:1237), because Writer has no
    /// per-section restart to map <c>newSection</c> onto.
    /// </remarks>
    public bool RestartsEachPage { get; init; }

    /// <summary>
    /// The pages with their margin numbers filled in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A pass over finished pages rather than something pagination does as it fills them, and that is
    /// exact rather than convenient: a margin number is drawn <em>outside</em> the text area, so it takes
    /// no room from the body and cannot move a line. Nothing about where the text went depends on it, so
    /// there is no feedback loop to damp and no reason to pay for it during the fill.
    /// </para>
    /// <para>
    /// The count runs over the pages in order, so a document that does not restart carries it from page to
    /// page. A page's own lines are taken in placement order, which for a multi-column page is column by
    /// column — the order Writer numbers them in, since it walks the page's text frames.
    /// </para>
    /// </remarks>
    /// <param name="pages">The finished pages.</param>
    /// <param name="blocks">The blocks the body lines index into.</param>
    public IReadOnlyList<LaidOutPage> Applied(
        IReadOnlyList<LaidOutPage> pages, IReadOnlyList<PageBlock> blocks)
    {
        ArgumentNullException.ThrowIfNull(pages);
        ArgumentNullException.ThrowIfNull(blocks);

        int countBy = Math.Max(1, CountBy);
        int counted = 0;

        List<LaidOutPage> numbered = new(pages.Count);

        foreach (LaidOutPage page in pages)
        {
            if (RestartsEachPage) counted = 0;

            IReadOnlyList<PageBlock> own = page.Blocks ?? blocks;
            List<PageLineNumber> marks = [];

            // A line drawn as several boxes — one paragraph flowing round a frame — is one line and takes
            // one number, on the box that starts it. The boxes of one line are chained by
            // `SharesLineWithNext`, so a box whose predecessor said it shared is a continuation.
            bool continuation = false;

            foreach (PlacedLine line in page.Lines)
            {
                bool shares = line.Box.SharesLineWithNext;
                bool isContinuation = continuation;
                continuation = shares;

                if (isContinuation) continue;
                if (line.ParagraphIndex < 0 || line.ParagraphIndex >= own.Count) continue;
                if (own[line.ParagraphIndex] is not PageParagraph paragraph) continue;
                if (paragraph.SuppressesLineNumbers) continue;

                counted++;

                int number = Start + counted - 1;
                if (number % countBy != 0 && countBy > 1) continue;

                DocRect area = page.ColumnArea(line);

                marks.Add(new PageLineNumber(
                    number.ToString(System.Globalization.CultureInfo.InvariantCulture),
                    new DocPoint(area.X - Distance, area.Y + line.Baseline)));
            }

            numbered.Add(
                marks.Count == 0 ? page : page with { LineNumbers = marks, Numbering = this });
        }

        return numbered;
    }
}

/// <summary>One margin line number: what it says and where its right edge sits.</summary>
/// <remarks>
/// The <em>right</em> edge, because the numbers are right-aligned against the text edge and their widths
/// differ — a page carrying both 9 and 10 draws them at two different left edges and one right one. The
/// width is the drawing backend's to measure, since measuring it here would mean shaping twice.
/// </remarks>
/// <param name="Text">The number as it is drawn.</param>
/// <param name="RightBaseline">Its right edge, on the line's own baseline, in page coordinates.</param>
public readonly record struct PageLineNumber(string Text, DocPoint RightBaseline);
