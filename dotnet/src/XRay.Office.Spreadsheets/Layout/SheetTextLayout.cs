using System.Collections.Concurrent;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Itemisation;
using XRay.Office.Text.Layout;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>What one cell's text needs to know about the sheet around it.</summary>
/// <param name="Scale">The print zoom as a factor; everything drawn is multiplied by it.</param>
/// <param name="IsAvailable">
/// Whether the cell at a row and column is free for a neighbour's text to spill into — empty, and
/// neither merged nor overlapped. <c>ScOutputData::IsAvailable</c>,
/// <c>sc/source/ui/view/output2.cxx:1178</c>.
/// </param>
/// <param name="ColumnWidth">The printed width of a column, already scaled.</param>
/// <param name="BlockLeft">
/// The left edge of the block of columns being printed, scaled — Calc's <c>mnScrX</c>.
/// </param>
/// <param name="BlockRight">Its right edge, Calc's <c>mnScrX + mnScrW</c>.</param>
/// <param name="BreaksStartLines">
/// Whether a hard break starts a line in a cell that does not wrap. An importer's decision rather
/// than a cell's; see <see cref="SheetLayout.CellBreaksStartLines"/>.
/// </param>
internal readonly record struct SheetTextContext(
    double Scale,
    Func<int, int, bool> IsAvailable,
    Func<int, Length> ColumnWidth,
    Length BlockLeft = default,
    Length BlockRight = default,
    bool BreaksStartLines = false);

/// <summary>One cell as it is about to be drawn.</summary>
/// <param name="Text">The text the number format produced.</param>
/// <param name="Value">Its typed value; null for a blank cell and a string for a text one.</param>
/// <param name="Format">Its resolved text format.</param>
/// <param name="Row">The zero-based row.</param>
/// <param name="Column">The zero-based column.</param>
/// <param name="Box">Where the cell sits on the page, scaled.</param>
/// <param name="Portions">
/// The stretches its text is split into when they are not all in the cell's own format, or null
/// when they are. See <see cref="SheetRichText"/>.
/// </param>
/// <param name="IsField">
/// Whether the cell's whole content is one EditEngine field — a hyperlink. A field is drawn as one
/// indivisible portion, so it neither breaks across lines nor loses its tail to a narrow column.
/// See <see cref="SheetLayout.HoldsField"/>.
/// </param>
internal readonly record struct SheetCellText(
    string Text,
    object? Value,
    SheetCellFormat Format,
    int Row,
    int Column,
    DocRect Box,
    IReadOnlyList<SheetTextPortion>? Portions = null,
    bool IsField = false);

/// <summary>
/// Places and draws one cell's text.
/// </summary>
/// <remarks>
/// <para>
/// A port of Calc's own text output, <c>ScOutputData::LayoutStringsImpl</c>
/// (<c>sc/source/ui/view/output2.cxx:1595-2290</c>), which states the alignment, overflow,
/// clipping and <c>###</c> rules directly. The order it does things in is load-bearing and is
/// kept: resolve the alignment from the cell's <em>type</em>, work out how much room the text
/// needs, widen that room into empty neighbours, then shrink, wrap or hash whatever still does
/// not fit — each step reading the clip flags the step before it set.
/// </para>
/// <para>
/// <strong>Four margins of twenty twips each, and they are not decoration.</strong>
/// <c>ATTR_MARGIN</c>'s default is <c>SvxMarginItem(20, 20, 20, 20)</c>
/// (<c>svx/source/items/algitem.cxx:123</c>), and all four are measurable in a reference
/// rendering: a sheet with a two-centimetre page margin starts its first column's text at
/// 57.7 pt rather than 56.7, and bottom-aligns its baseline one point above the row's bottom
/// rather than on it.
/// </para>
/// <para>
/// <strong>A cell's line height is not the word processor's.</strong> Calc builds it from the
/// font metric alone — ascent plus descent, no external leading (<c>output2.cxx:734</c>) — where
/// Writer adds the line gap. Ten-point Liberation Sans wraps at a pitch of 11.17 pt here and
/// 11.50 pt there, so a three-line cell drawn with Writer's pitch has its last line a point low.
/// </para>
/// </remarks>
internal static class SheetTextLayout
{
    /// <summary>
    /// The margin between a cell's edge and its text when the cell states none.
    /// </summary>
    /// <remarks>
    /// The item pool's default. What a given cell actually uses is
    /// <see cref="SheetCellFormat.Margin"/>, because the BIFF filter overrides it — see that
    /// property. This is kept as the default the format itself carries and as the value the
    /// callers that have no cell in hand fall back to.
    /// </remarks>
    public static readonly Length CellMargin = Length.FromTwips(20);

    /// <summary>How many times the shrink loop is allowed to try again.</summary>
    /// <remarks><c>SC_SHRINKAGAIN_MAX</c>; each attempt takes a further tenth off the scale.</remarks>
    private const int ShrinkAttempts = 7;

    /// <summary>What a numeric cell that will not fit draws instead of its number.</summary>
    private const string HashText = "###";

    /// <summary>
    /// The measure a cell that does not wrap is broken against: wide enough that only a hard
    /// break can end a line.
    /// </summary>
    /// <remarks>
    /// A metre and a half, which no cell's text approaches and which leaves the length arithmetic
    /// far from overflow. Calc reaches the same place by giving such a cell a paper it can never
    /// fill rather than by turning the breaker off.
    /// </remarks>
    private static readonly Length UnboundedWidth = Length.FromPoints(4096);

    /// <summary>
    /// The width a wrapping cell is broken against when its column leaves it none.
    /// </summary>
    /// <remarks>
    /// One twip, standing for the one unit of the engine's own map mode that
    /// <c>ImpEditEngine::calculateMaxLineWidth</c> clamps to
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:542-543). No glyph fits it, so the fill loop's
    /// "a line takes at least one character" rule puts every character on a line of its own.
    /// </remarks>
    private static readonly Length NarrowestWrapWidth = Length.FromTwips(1);

    /// <summary>
    /// The colour a hyperlink cell's text is painted in, whatever the file says it is.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>#000080</c>: the application's configured <c>LINKS</c> colour, which is
    /// <c>COL_BLUE</c> — <c>svtools/source/config/colorcfg.cxx:534</c> lists
    /// <c>{ COL_BLUE, Color(0x1D99F3) }</c> for <c>LINKS</c>, light theme first, and
    /// <c>include/tools/color.hxx:443</c> defines <c>COL_BLUE</c> as
    /// <c>Color(0x00, 0x00, 0x80)</c>. Navy, not the pure blue the name suggests, and not the
    /// <c>#0000FF</c> every workbook's own hyperlink style states.
    /// </para>
    /// <para>
    /// <strong>The substitution is unconditional, and that was established rather than assumed.</strong>
    /// A hyperlink cell is an <c>SvxURLField</c> inside an <c>EditTextObject</c>
    /// (<c>WorksheetGlobals::insertHyperlink</c>,
    /// <c>sc/source/filter/oox/worksheethelper.cxx:1062-1080</c>) and the EditEngine paints a URL
    /// field in the configured link colour rather than in the character colour, so the character
    /// colour never reaches the page. Measured with an authored probe holding a hyperlink cell
    /// stated <c>#FF0000</c> and a second stated <c>#00B050</c>, each beside an unlinked control
    /// in the same colour: the reference painted both hyperlink cells <c>#000080</c> and left both
    /// controls alone. So this beats a stated colour rather than filling in for an absent one.
    /// </para>
    /// <para>
    /// It applies to whatever <see cref="SheetLayout.HoldsField"/> is true of and to nothing else —
    /// the same predicate that already decides that such a cell neither wraps nor shortens, so the
    /// three consequences of being a field are stated once between them.
    /// </para>
    /// </remarks>
    private static readonly Colour LinkColour = Colour.FromRgb(0x000080);

    /// <summary>The ink one run is painted with: its own colour, the cell's, or the field's.</summary>
    /// <param name="portion">The rich portion's own colour, or null for the cell's.</param>
    /// <param name="fallback">The cell's colour.</param>
    /// <param name="field">
    /// What a hyperlink field paints the whole cell in, from <see cref="FieldInk"/>, or null when
    /// the cell holds no field.
    /// </param>
    private static Colour Ink(Colour? portion, Colour fallback, Colour? field)
        => field ?? portion ?? fallback;

    /// <summary>
    /// What a hyperlink cell's text is painted in — one colour for the whole cell — or null when
    /// it holds no field.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>One colour for the whole cell, because the reference collapses the cell to one
    /// character.</strong> Both importers replace the cell's text with a single
    /// <c>EE_FEATURE_FIELD</c> — <c>lclInsertUrl</c> with
    /// <c>QuickInsertField(…, ESelection::All())</c> and <c>insertHyperlink</c> after
    /// <c>rEE.Clear()</c> — so there are no per-run colours left to paint with, and the attribute
    /// that decides the colour is whichever <c>EE_CHAR_COLOR</c> survives at position zero. That
    /// is the <em>first</em> portion's where the cell is rich and the cell's own where it is not.
    /// </para>
    /// <para>
    /// <see cref="SheetCellFormat.ColourIsHard"/> is the test, and it is what makes the answer
    /// differ by format on identical content: measured on 26.2.4.2, an <c>.xlsx</c> hyperlink cell
    /// stated <c>#FF0000</c> is drawn <c>#000080</c> and the <c>.xls</c> the reference itself
    /// converts that file to draws the same cell <c>#FF0000</c>. <c>probes/quantise-r120/</c> §5.
    /// </para>
    /// </remarks>
    /// <summary>
    /// The colour the cell's number format states for the subformat its value selects, or null.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Only a <em>numeric</em> cell asks. A subformat's colour applies to the value it formats,
    /// and the text subformat is the fourth one; a cell holding a string takes its own colour,
    /// which is what <see cref="NumberFormatCode.SelectForText"/> would have to be consulted for
    /// and which no corpus document states a colour on.
    /// </para>
    /// <para>
    /// Null is the answer for every format that names no colour, which is nearly all of them —
    /// 14 of 240 corpus workbooks state one at all, and 8 hold a value that selects a coloured
    /// subformat. See <c>probes/numfmtcolour-r139/results.md</c>.
    /// </para>
    /// </remarks>
    private static Colour? NumberFormatColour(in SheetCellText cell)
        => cell.Value is double value && cell.Format.NumberFormat is { } code
            ? code.SelectFor(value).Colour
            : null;

    private static Colour? FieldInk(in SheetCellText cell)
    {
        if (!cell.IsField) return null;

        SheetCellFormat at = cell.Portions is { Count: > 0 } ? cell.Portions[0].Format : cell.Format;

        return at.ColourIsHard ? at.Colour : LinkColour;
    }

    /// <summary>The rule under one run: the one it states, or the one a field always takes.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The link's underline and the link's colour come out of one function call</strong>,
    /// and this tree modelled only the colour half. <c>ScEditUtil::GetCellFieldValue</c>
    /// (<c>sc/source/core/tool/editutil.cxx:209-244</c>) handles a
    /// <c>text::textfield::Type::URL</c> by writing <em>both</em>
    /// <c>*ppTextColor = …GetColorValue(LINKS)</c> and
    /// <c>*ppFldLineStyle = FontLineStyle::LINESTYLE_SINGLE</c>; <c>ScFieldEditEngine</c> passes
    /// both out through <c>CalcFieldValue</c> (<c>:895-906</c>), <c>ImpEditEngine::UpdateFields</c>
    /// stores them on the field's own character attribute
    /// (<c>editeng/source/editeng/impedit2.cxx:3229-3231</c>), and
    /// <c>EditCharAttribField::SetFont</c> (<c>editeng/source/editeng/editattr.cxx:349-350</c>)
    /// applies them to the font the run is drawn with. So the two travel together and neither is
    /// conditional on what the file says — see <see cref="LinkColour"/> for the colour half.
    /// </para>
    /// <para>
    /// <c>SetUnderline</c> <em>replaces</em> whatever the run's own font asked for, so a field
    /// takes exactly one line even where the cell states two. That is why this returns the style
    /// rather than the maximum of the two.
    /// </para>
    /// <para>
    /// <strong>The reference's own flat export cannot see this and says the opposite.</strong>
    /// <c>ScXMLExport</c> resolves a field with <c>GetCellFieldValue(*pField, &amp;rDoc, nullptr,
    /// nullptr)</c> (<c>sc/source/filter/xml/xmlexprt.cxx:3062</c>) — both out-parameters null —
    /// so a <c>--convert-to fods</c> of a workbook with a hyperlink cell prints
    /// <c>style:text-underline-style="none"</c> and the cell's stated colour on the very cells
    /// 26.2.4.2's own PDF draws navy and underlined. Measured on
    /// <c>084_Service_invoice_Use_this_template</c>: the flat export gives cell B13
    /// <c>underline=none, colour=#000000</c>, the PDF a <c>#000080</c> stroke 0.51 pt thick from
    /// x 83.8 to 193.3. The resolved view is the wrong instrument for this one attribute.
    /// </para>
    /// </remarks>
    private static SheetUnderline Line(SheetUnderline stated, bool field)
        => field ? SheetUnderline.SingleLine : stated;

    private static readonly ConcurrentDictionary<string, ParagraphLayouter> Layouters =
        new(StringComparer.Ordinal);

    /// <summary>
    /// Whether a cell is free for a neighbour's text to run through.
    /// </summary>
    /// <remarks>
    /// A cell object is not the same thing as a cell with something in it. Both readers
    /// materialise the gaps inside a row so that extracted text keeps its columns, so a row
    /// holding A1 and D1 carries four cells of which two are blank — and a test of "is there a
    /// cell here" stops a long string at B1 where Calc runs it through to D1
    /// (<c>ScOutputData::IsEmptyCellText</c>, <c>output2.cxx:1178</c>).
    /// </remarks>
    /// <param name="cell">The neighbouring cell, or null when the sheet has nothing there.</param>
    public static bool IsAvailable(ContentTableCell? cell)
        => cell is null || (cell.Value is null && cell.GetOwnText().Length == 0);

    /// <summary>Draws one cell's text, or nothing when there is none to draw.</summary>
    /// <param name="sink">Where to draw.</param>
    /// <param name="context">The sheet around the cell.</param>
    /// <param name="cell">The cell.</param>
    public static void Draw(IDrawingSink sink, in SheetTextContext context, in SheetCellText cell)
    {
        ArgumentNullException.ThrowIfNull(sink);

        if (cell.Text.Length == 0) return;
        if (SheetFonts.For(cell.Format) is not { } face) return;

        Placement placement = Place(context, cell, face);
        if (placement.Lines.Count == 0) return;
        if (IsOutside(context, placement)) return;

        // The cell's own colour is the fallback rather than the answer: a rich cell's portions
        // carry theirs, and a plain one's segment carries none so that the two paths emit the
        // same paint for the same cell.
        //
        // A number format's subformat may state one of its own — `#,##0 ;[Red](#,##0)` draws a
        // negative red — and it beats the cell's, because Calc paints the colour the *selected
        // subformat* named: `ScOutputData` takes it from the `ScPatternAttr`'s format through
        // `SvNumberFormatter::GetOutputString`, which returns the subformat's colour beside the
        // text. Which subformat that is depends on the cell's value, so this is resolved per
        // cell here rather than per style on `SheetCellFormat`. See
        // `NumberFormatSection.Colour`, including why `[COLOR n]` is left unresolved.
        Colour fallback = NumberFormatColour(cell) ?? cell.Format.Colour;

        if (cell.Format.IsRotated)
        {
            DrawRotated(sink, context, cell, placement, fallback);
            return;
        }

        // Whatever a shortened string still hangs over the edge, and every wrapped line taller
        // than its row, is cut off rather than drawn across the neighbour. Calc sets the clip
        // region to the same rectangle it aligned in (output2.cxx:2126) and only when it is
        // needed, which is worth keeping: a clip per cell would put two operators around every
        // run in the file.
        //
        // `ClipPathKeepingText`, because a cell's words are the document's own: LibreOffice cuts
        // the ink at the same edge and leaves every glyph in its PDF's text layer, and dropping
        // them instead lost 124 words on one workbook alone. See
        // <see cref="IDrawingSink.ClipPathKeepingText"/>.
        (bool clipped, Length clipLeft, Length clipRight) = ClipTo(context, placement);
        if (clipped)
        {
            sink.Save();
            sink.ClipPathKeepingText(Rectangle(new DocRect(
                clipLeft,
                placement.Top,
                clipRight - clipLeft,
                placement.Bottom - placement.Top)));
        }

        try
        {
            foreach (PlacedLine line in placement.Lines)
            {
                foreach ((GlyphRun run, Colour? colour) in
                         line.Run.At(new DocPoint(line.X, line.Baseline)))
                {
                    // An empty paragraph's line carries a segment for its metrics and no glyphs;
                    // it has taken its height already and there is nothing to draw or underline.
                    if (run.Glyphs.Count == 0) continue;

                    sink.DrawGlyphRun(run, Paint.Solid(Ink(colour, fallback, FieldInk(cell))));
                }

                // A rich cell answers per segment, because a run may underline part of a line and
                // the segment already knows where it starts and how wide it is. A plain cell keeps
                // the whole-line rule: it has one format, and one rule across the line is what
                // Calc draws.
                if (cell.Portions is { Count: > 0 })
                {
                    foreach (SheetTextSegment segment in line.Run.Segments)
                    {
                        DecorateSegment(
                            sink, context.Scale, segment, line,
                            Ink(segment.Colour, fallback, FieldInk(cell)), cell.IsField);
                    }
                }
                else
                {
                    Decorate(
                        sink, context.Scale, cell.Format, face, line,
                        Ink(null, fallback, FieldInk(cell)), cell.IsField);
                }
            }
        }
        finally
        {
            if (clipped) sink.Restore();
        }
    }

    /// <summary>
    /// Whether the room the cell was given falls entirely outside the block being printed.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>bOutside</c> (<c>output2.cxx:2037</c>), and it is the whole reason a page does not
    /// carry every neighbour of its own last column. Calc's string loop looks one cell
    /// <em>past</em> the block so that a long string reaching in from the right is drawn — but
    /// it then asks of every cell whether what it occupies overlaps the block at all, and draws
    /// nothing when it does not. A short string in that column is therefore skipped and a long
    /// one is not, because only the long one's output area, widened through its empty
    /// neighbours, reaches the paper.
    /// </para>
    /// <para>
    /// <strong>This is a test of the room the text was given, not of where its anchor is, and it
    /// is not what keeps a rightward spill off the following page.</strong> That is decided
    /// before a cell reaches here, by which cells the page offers at all — see
    /// <see cref="SheetPageDrawing"/>, whose remarks carry the measurement. Reading this test as
    /// the seat of the "painted on every page it crosses" defect is the natural mistake and the
    /// wrong one: a run anchored several columns to the left genuinely <em>does</em> overlap the
    /// next block, so <c>bOutside</c> answers "inside" for it in Calc too, and Calc still does
    /// not draw it — because in a tagged PDF the loop never visits its anchor column.
    /// </para>
    /// <para>
    /// Measured: <c>ExampleWhiteListData.xlsx</c> drew twenty part numbers off the left edge of
    /// its last two pages — <strong>838 words against the reference's 821</strong> — because
    /// every one of them was the nearest cell left of a band and none of them spilled into it.
    /// That case is now caught twice over, since no cell left of a band is offered to this test
    /// at all; the test still earns its place on the cell found <em>right</em> of one.
    /// </para>
    /// <para>
    /// Calc's rectangle is inclusive at the right, so a cell ending exactly where the block
    /// begins is outside it; hence the <c>&lt;=</c>.
    /// </para>
    /// </remarks>
    private static bool IsOutside(in SheetTextContext context, in Placement placement)
        => context.BlockRight > context.BlockLeft
           && (placement.Right <= context.BlockLeft || placement.Left >= context.BlockRight);

    /// <summary>
    /// The rectangle the ink is cut to, and whether it has to be cut at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScOutputData::AdjustAreaParamClipRect</c> (<c>output2.cxx:2928-2954</c>), and it is not
    /// the clamp its name suggests. It trims the output area to the printed block —
    /// <c>[mnScrX, mnScrX + mnScrW]</c> — and where it has to trim it <strong>sets
    /// <c>mbLeftClip</c> or <c>mbRightClip</c></strong>. <c>LayoutStrings</c> computes
    /// <c>bHClip</c> from those two flags <em>after</em> calling it (<c>:2038-2039</c>), and
    /// <c>DrawEditStandard</c> ors them into <c>bClip</c> the same way (<c>:3239</c>), so the trim
    /// does not merely narrow a clip that was going to be set anyway: <strong>it turns one on for
    /// a cell whose text fitted the room it was given perfectly well.</strong> A merge wider than
    /// the columns the page prints is the commonest way in; a long string that overflows into free
    /// neighbours past the block's last column is the other.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 with <c>sheet-clip-block.fods</c> — five 3 cm columns to a page, so the
    /// block runs 56.693–481.890 pt — by reading the reference's own content stream:
    /// </para>
    /// <list type="table">
    ///   <item><term>a long string in A, free neighbours</term>
    ///     <description><c>56.693..481.890</c>: the area was widened past the block and trimmed
    ///     back</description></item>
    ///   <item><term>a long string in E, the page's last column</term>
    ///     <description><c>396.850..481.889</c>: trimmed at the block, not at column F</description></item>
    ///   <item><term>a merge C:H centred, straddling the break</term>
    ///     <description><c>226.772..481.890</c> on page 1 and <c>56.693..311.754</c> on page 2:
    ///     trimmed at the near edge of the block each time</description></item>
    ///   <item><term>a long string in C blocked by D</term>
    ///     <description><c>226.772..311.755</c>: the ordinary case, the cell's own edge</description></item>
    ///   <item><term>a string that fits</term><description>no clip at all</description></item>
    /// </list>
    /// <para>
    /// <strong>This is what "it is not a clipping rule" missed.</strong> That reading measured a
    /// rightward overflow reaching 617.63 pt on a 612 pt page and concluded the run was not
    /// clipped — but it measured the <em>text layer</em>, which a clip region never touches. The
    /// glyphs stay in the PDF's text and only the ink is cut, which is exactly why this defect
    /// survived a word-count gate untouched on both documents that showed it.
    /// </para>
    /// <para>
    /// Only the horizontal half is reproduced. Calc trims the vertical to
    /// <c>[mnScrY, mnScrY + mnScrH]</c> by the same code and widens the unclipped axis to the whole
    /// block (<c>:2114-2123</c>); nothing measured in the corpus turns on it, our vertical extent
    /// is deliberately the union of the cell and its text rather than a cut (see
    /// <see cref="Place"/>), and <see cref="RowBand"/> carries no bottom edge to trim against.
    /// </para>
    /// </remarks>
    private static (bool Clipped, Length Left, Length Right) ClipTo(
        in SheetTextContext context, in Placement placement)
    {
        Length left = placement.Left;
        Length right = placement.Right;

        // Asked after IsOutside and never before it, which is Calc's order: bOutside is decided
        // against the untrimmed area (output2.cxx:2036) and would answer "inside" for every cell
        // once the area had been folded into the block.
        if (context.BlockRight <= context.BlockLeft)
            return (placement.Clipped, left, right);

        bool trimmed = false;
        if (left < context.BlockLeft)
        {
            left = context.BlockLeft;
            trimmed = true;
        }

        if (right > context.BlockRight)
        {
            right = context.BlockRight;
            trimmed = true;
        }

        return (placement.Clipped || trimmed, left, right);
    }

    /// <summary>
    /// Draws the rules a font asks for under and through one line of a cell.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A cell's underline is a font property in all three formats and is drawn by the output
    /// device rather than shaped, so it is a filled rectangle under the run and not a glyph. The
    /// offset and the thickness come from the face's own <c>post</c> and <c>OS/2</c> tables
    /// through <see cref="LineSpacing.ResolveDecorations(OpenTypeFace, LineMetrics)"/>, which is the same resolution and the
    /// same fallbacks the rest of the project uses — a font that declares neither would otherwise
    /// draw a zero-thickness line, which is to say none.
    /// </para>
    /// <para>
    /// Excel's two accounting underline styles run the full width of the <em>cell</em> rather
    /// than of the text; both are folded onto their plain counterparts here, so an accounting
    /// underline is as wide as its number. See <see cref="SheetUnderline"/>.
    /// </para>
    /// <para>
    /// Per line for a plain cell, which has one format, and <strong>per segment for a rich
    /// one</strong> — see <see cref="DecorateSegment"/>. This used to be per line in both cases,
    /// on the grounds that "the run geometry to place a partial rule with does not exist yet";
    /// it did, in <see cref="SheetTextSegment"/>'s <c>Offset</c> and <c>Width</c>, and the
    /// consequence was that a cell underlining only its first run drew no underline at all
    /// whenever the cell's own font was not underlined — which is how a German price table came
    /// to lose the rule under <c>Innereuropäische Flüge</c>.
    /// </para>
    /// </remarks>
    private static void Decorate(
        IDrawingSink sink, double scale, SheetCellFormat format, SheetFace face, PlacedLine line,
        Colour colour, bool field)
        => Rules(sink, scale, Line(format.Underline, field), format.IsStruckThrough, face,
                 line.Run.Size, line.X, line.Run.Width, line.Baseline, colour);

    /// <summary>
    /// The rules one segment of a rich cell asks for, under that segment alone.
    /// </summary>
    /// <remarks>
    /// The segment's own face and size are used rather than the line's, because a rich cell can
    /// change both at a portion boundary and an underline's thickness and offset are read from the
    /// face it sits under. Its <c>Offset</c> and <c>Width</c> are the geometry whose absence used
    /// to be the reason this was done per line.
    /// </remarks>
    private static void DecorateSegment(
        IDrawingSink sink, double scale, SheetTextSegment segment, PlacedLine line, Colour colour,
        bool field)
        => Rules(sink, scale, Line(segment.Underline, field), segment.StruckThrough, segment.Face,
                 segment.Size, line.X + segment.Offset, segment.Width, line.Baseline, colour);

    private static void Rules(
        IDrawingSink sink,
        double scale,
        SheetUnderline underline,
        bool struckThrough,
        SheetFace face,
        Length size,
        Length x,
        Length width,
        Length baseline,
        Colour colour)
    {
        if (underline == SheetUnderline.None && !struckThrough) return;
        if (size <= Length.Zero || width <= Length.Zero) return;

        // Thickness AND offset, both from the device -- see `LineSpacing.ResolveRuleWidths`, which
        // answers the whole hundredth of a millimetre the reference's own 720 dpi device quantises
        // each of them to (O64 for the thickness, round 123 for the offset).
        //
        // AND the print zoom goes into the grid rather than being left to multiply the answer, O70:
        // Calc paints through a map mode carrying the zoom, so the whole hundredth of a millimetre
        // is a hundredth of the UNSCALED page. `size` is already the drawn size and stays that way
        // -- the device pixel count is taken from it and the scale is already inside it. See
        // `MetricGrid.PageScale`.
        LineSpacing.RuleWidths widths = LineSpacing.ResolveRuleWidths(
            face.Face, face.Metrics, size, MetricGrid.TextLine.Scaled(scale));

        if (underline == SheetUnderline.DoubleLine)
        {
            // Both lines AND both offsets come off the device. Round 120 gave this the right
            // thickness and kept the single underline's offset for the first line and twice the
            // thickness below it for the second, which is wrong in both places: the pair straddles
            // where a single rule would sit, its separation is floored on the device rather than
            // scaled with the em, and the writer adds a further whole thickness to the second.
            // See `LineSpacing.RuleWidths.DoubleUnderlineSecond`.
            Rule(sink, x, baseline + widths.DoubleUnderlineFirst, width, widths.DoubleUnderline,
                 colour);
            Rule(sink, x, baseline + widths.DoubleUnderlineSecond, width, widths.DoubleUnderline,
                 colour);
        }
        else if (underline != SheetUnderline.None)
        {
            Rule(sink, x, baseline + widths.UnderlineOffset, width, widths.Underline, colour);
        }

        if (struckThrough)
        {
            Rule(sink, x, baseline + widths.StrikeoutOffset, width, widths.Strikeout, colour);
        }
    }

    /// <summary>One horizontal rule, filled rather than stroked so its thickness is exact.</summary>
    private static void Rule(
        IDrawingSink sink, Length x, Length top, Length width, Length thickness, Colour colour)
    {
        if (thickness <= Length.Zero) return;

        sink.FillPath(Rectangle(new DocRect(x, top, width, thickness)), Paint.Solid(colour));
    }

    private static GraphicsPath Rectangle(DocRect rect)
        => new GraphicsPath()
           .MoveTo(new DocPoint(rect.X, rect.Y))
           .LineTo(new DocPoint(rect.X + rect.Width, rect.Y))
           .LineTo(new DocPoint(rect.X + rect.Width, rect.Y + rect.Height))
           .LineTo(new DocPoint(rect.X, rect.Y + rect.Height))
           .Close();

    // ------------------------------------------------------------------------------ placement

    private readonly record struct PlacedLine(SheetTextRun Run, Length X, Length Baseline);

    /// <summary>Where a cell's lines ended up, and what has to be cut off around them.</summary>
    /// <param name="Lines">The placed lines.</param>
    /// <param name="Clipped">True when the text still runs past what it was given.</param>
    /// <param name="Left">The left edge of the room it was given, neighbours included.</param>
    /// <param name="Right">The right edge of the same.</param>
    /// <param name="Top">The top of the clip, which is the cell's or the text's, whichever is higher.</param>
    /// <param name="Bottom">Its bottom, likewise.</param>
    private readonly record struct Placement(
        List<PlacedLine> Lines, bool Clipped = false,
        Length Left = default, Length Right = default,
        Length Top = default, Length Bottom = default);

    private static Placement Place(in SheetTextContext context, in SheetCellText cell, SheetFace face)
    {
        SheetCellFormat format = cell.Format;
        double scale = context.Scale;

        // Both snapped to the drawing device's hundredth of a millimetre, and the two round
        // differently. See SheetDeviceUnits: a font height rounds and a margin truncates, which is
        // what puts a ten-point cell's text at 10.0063 pt and its left edge 0.9921 pt inside the
        // column rather than a whole point.
        Length size = SheetDeviceUnits.SnapFontSize(format.FontSize, scale);
        Length margin = SheetDeviceUnits.Snap(format.Margin) * scale;

        // The indent counts only when the cell states left or right alignment outright. Calc reads
        // ATTR_INDENT solely in that case (output2.cxx:445), so a General-aligned cell carrying an
        // indent is drawn without one — which looks like a bug in the port until the reference
        // renderer is measured and agrees.
        Length indent = format.Horizontal is SheetHorizontalAlignment.Left
                            or SheetHorizontalAlignment.Right
            ? SheetDeviceUnits.Snap(format.Indent) * scale
            : Length.Zero;

        bool isValue = cell.Value is not null and not string;
        SheetHorizontalAlignment horizontal = Resolve(format.Horizontal, isValue);

        // A field cell wraps like any other; what it does *not* have is anywhere to wrap. The
        // comment beside `rWrapFields` — "Fields aren't wrapped, so clipping is enabled to prevent
        // a field from being drawn beyond the cell size" (output2.cxx:2560-2567) — describes the
        // clip it switches on at :3239, not a suppression of breaking: `mbBreak` is untouched, so
        // the EditEngine paper stays the column's width and the text still has to fit it. Reading
        // it as "does not break" cost 22 rows of `Published_Issuances_2024.xlsx` their second line.
        //
        // The clip is why `breaks` has to be true here rather than only inside `Wrap`: it is what
        // passes `blocked` to `OutputArea` and stops a field borrowing an empty neighbour.
        //
        // Where a field differs is that it is atomic to the *breaker* — see `SheetFieldBreaker`,
        // which `Wrap` uses instead, and which turns every one of its lines into a chop.
        bool breaks = Breaks(format, isValue);
        bool fills = format.Horizontal == SheetHorizontalAlignment.Fill && !breaks;
        bool shrinks = format.ShrinksToFit && !breaks && !fills;

        Length leftTotal = margin + indent;
        Length totalMargin = leftTotal + margin;

        (string text, int fillAt, char fillChar) = Fill(cell);

        // A value is never rich: SpreadsheetML's formatting runs and ODF's spans belong to a
        // string, and a number that showed several fonts would have nowhere to put them once it
        // was re-rendered as ### or in scientific notation.
        IReadOnlyList<SheetTextPortion>? portions =
            !isValue && cell.Portions is { Count: > 0 } stated ? stated : null;

        // Every re-shape below is a range of the cell's own text at a percentage of its size, so
        // that a rich cell keeps its portions lined up with its characters through shortening and
        // wrapping. A plain cell takes the same route with one segment and one face.
        SheetTextRun? ShapeRange(int start, int end, long percent)
        {
            if (portions is not null)
                return SheetText.ShapeRich(text, portions, scale, start, end, percent);

            Length em = percent == 100
                ? size
                : SheetDeviceUnits.SnapFontSize(Length.FromTwips(size.Twips * percent / 100));

            return em > Length.Zero
                ? SheetText.Shape(text[Math.Max(start, 0)..Math.Min(end, text.Length)], face, em)
                : null;
        }

        // How much of its stated size the cell is being drawn at. Only shrink-to-fit moves it, and
        // everything after that has to re-shape at the same percentage or a shortened cell comes
        // back at full size — which is the sort of change that shows as one character more or fewer
        // and nowhere else.
        long percent = 100;

        SheetTextRun? run = ShapeRange(0, text.Length, percent);
        if (run is null) return new Placement([]);

        Area area = OutputArea(
            context, cell, horizontal, run.Width + totalMargin, isValue || fills || shrinks || breaks);

        // A turned or stacked cell never reaches this path in Calc at all. `aVars.IsRotated()` or
        // a stacked orientation sets bUseEditEngine before GetOutputArea is even called
        // (output2.cxx:1800-1803), so DrawStrings skips the cell and `DrawEdit`/`DrawRotated` draw
        // it — and none of what follows is theirs: the EditEngine path neither shrinks a string to
        // fit, nor hashes a number, nor drops the characters it cannot show. It turns the text
        // about the cell's bottom-left corner and lets it run out of the cell, which is the whole
        // point of a 45-degree column heading. Measured on `sheet-rich-text.xlsx`: the reference
        // draws all fifteen characters of "Slanted heading" and X-Ray.Office drew eleven, and on the
        // .xls — whose columns LibreOffice's BIFF import makes a shade narrower — nine.
        if (format.IsRotated) area = area.Unclipped();

        // A quarter-turned cell that wraps breaks against the room its lines have *after* the
        // turn, which is measured down the cell rather than across it: `calcPaperSize` gives a
        // vertically oriented cell a paper whose width is the align rectangle's *height*
        // (`output2.cxx:2691`). Breaking at the column width instead put several lines on a
        // heading LibreOffice draws as one — measured on
        // `Keywords_Mapping_Graphs_and_Charts.xlsx` page 43, where the reference writes its 28
        // glyphs at one x and we wrote six lines of them.
        //
        // An *obliquely* turned cell has its own paper — `nOutHeight/|sin|`, then narrowed over
        // five steps until the turned block fits the cell (`:4977`, `:5033-5062`) — and is left
        // alone here. Nothing in the corpus turns a cell by anything but a quarter, so there is
        // nothing to measure the second formula against.
        Length available = IsQuarterTurned(format) && breaks
            ? cell.Box.Height - (2 * margin)
            : cell.Box.Width - totalMargin;

        // A wrapping cell whose text begins outside its own column draws nothing at all: see
        // `StartsOutsideItsCell`. A turned cell is left alone — its paper is the cell's height and
        // nothing was measured there.
        if (breaks && !IsQuarterTurned(format)
            && StartsOutsideItsCell(cell.Box.Width, leftTotal))
            return new Placement([]);

        // Between the output area and the shrink, which is where Calc does it
        // (output2.cxx:1853): the fill is measured against the cell's own column and not
        // against the room a neighbour lent, so it must not see the widened area — and
        // everything after it re-measures the text it produced.
        if (fillAt >= 0 && portions is null
            && RepeatToFill(text, fillAt, fillChar, face, size, available, run.Width) is { } filled)
        {
            text = filled;
            run = ShapeRange(0, text.Length, percent) ?? run;
        }

        if (shrinks && area.IsClipped && available > Length.Zero && run.Width > Length.Zero)
        {
            (run, percent) = Shrink(ShapeRange, text.Length, run, available);
            if (run.Width <= available) area = area.Unclipped();
        }

        bool hashed = false;
        if (isValue && area.IsClipped)
        {
            (run, text, hashed) = Hash(cell, face, run.Size, available, run);
            if (run.Width + totalMargin <= area.Width) area = area.Unclipped();
        }

        // A cell holding a no-break space or one of the six other characters of
        // HasEditCharacters is drawn by DrawEditStandard rather than by DrawStrings, and that
        // path clips the string to the cell without dropping a character from it. Everything
        // else about the two agrees — the same GetOutputArea with the same lending from empty
        // neighbours, the same ### for a value that will not fit — so the only thing to skip is
        // the shortening. A cell whose whole content is a hyperlink field is on the same path,
        // for the same reason: it is an EditTextObject rather than a string.
        //
        // **And so is every cell the importers stored as one**, which is the larger set and the
        // one the character list above kept hiding. `LayoutStrings` reaches for the EditEngine
        // before it looks at a single character: `else if (aCell.getType() == CELLTYPE_EDIT)
        // bUseEditEngine = true` (`sc/source/ui/view/output2.cxx:1710-1712`). Both importers
        // build such a cell from a string that carries formatting runs or a hard break —
        // `putRichString` on the OOXML side (`sheetdatabuffer.cxx:125-133`) and
        // `XclImpString::SetToDocument` on the BIFF side (`xihelper.cxx:246-256`).
        // See <see cref="IsEditCell"/>.
        if (!isValue && !breaks && area.IsClipped
            && !cell.IsField && !HasEditCharacters(text, fillAt)
            && !IsEditCell(text, portions))
        {
            run = Shorten(run, text, ShapeRange, percent, horizontal, area);
        }

        // `###` is never wrapped, even in a wrapping cell. Measured on `sheet-hash.fods`'s
        // `wrapdate` row under 26.2.4.2: a wrapping date cell too narrow for its date draws
        // `###` on **one** line at the same baseline as the unwrapped row beside it, and the row
        // keeps its single-line height. We drew three lines of one `#` each and made the row
        // three times as tall, which moves every row under it. The seat is that Calc replaces the
        // *engine text* with the hash string after the paper has been decided
        // (`output2.cxx:3605`, `:3849`, `:4070`), so there is nothing left to break.
        // A cell that does not wrap still breaks at its own paragraphs when the importer made
        // several of them, and then it breaks at those alone: the paper it is formatted against is
        // unbounded across, so nothing is ever broken for width. See
        // <see cref="SheetLayout.CellBreaksStartLines"/> for which importer does that and why.
        bool splitsAtBreaks = !breaks && !hashed && context.BreaksStartLines
                              && !cell.IsField && HoldsHardBreak(text);

        List<int> paragraphStarts = [0];
        List<SheetTextRun> lines = (breaks && !hashed) || splitsAtBreaks
            ? Wrap(
                text, portions, face, size, scale,
                splitsAtBreaks ? UnboundedWidth : available, ShapeRange, percent,
                out paragraphStarts, cell.IsField)
            : [run];
        if (lines.Count == 0) return new Placement([]);

        // How far down the next line starts. A field's lines are set closer together than any
        // other cell's, and the gap is not small: LibreOffice advances by the face's **ascent
        // alone**, where every other cell advances by ascent plus descent.
        //
        // Measured, because no reading of the source predicts it. `dotnet/probes/sheets-wrap-01`
        // holds sixteen single-cell workbooks — Calibri, Arial, DejaVu Sans and Times New Roman at
        // 8, 10, 14 and 20 pt — each one hyperlinked, wrap-enabled, and holding a run of `X` so
        // that every line chops at the same glyph and the gap between two lines' bounding boxes is
        // the pitch exactly. All sixteen line *counts* already agreed; all sixteen pitches came
        // back at the face's `hhea` ascent, to the tenth of a point the reference's device
        // quantises to. Carlito reads 9.50 pt at 10 pt against an ascent of 1950/2048 em = 9.52,
        // where `LineHeightAt` is 12.21; Liberation Sans 9.10 against 1854/2048 em = 9.05;
        // DejaVu Sans 9.30 against 1901/2048 em = 9.28; Liberation Serif 8.90 against
        // 1824/2048 em = 8.91.
        //
        // **What was measured is fields, and only fields.** Whether the same holds for the other
        // cells Calc sends to an EditEngine — a rich cell, one holding a hard break — is untested
        // here and deliberately not assumed: those keep `LineHeight`, which is what the corpus
        // was fitted against. See `results.md`.
        bool isField = cell.IsField;   // `cell` is an `in` parameter and cannot be captured.
        Length Pitch(SheetTextRun line) => isField ? line.Ascent : line.LineHeight;

        // The lines a wrapping cell has no room for are never formatted, so they are never drawn
        // and are not in the reference's PDF text layer either. See `SkipOutsideFormat`; this is
        // the one rule on this path that moves a word count rather than ink.
        if (breaks && !format.IsRotated
            && format.Vertical is SheetVerticalAlignment.Top or SheetVerticalAlignment.Standard)
        {
            SkipOutsideFormat(lines, paragraphStarts, cell.Box.Height - (2 * margin), Pitch);
        }

        // A justified cell's lines are stretched to the paper they were broken against, which
        // happens after the lines are settled and before any of them is placed: Calc maps both
        // `justify` and `distributed` to `SvxAdjust::Block` and the engine shares the room left
        // over among each line's blanks (`ImpEditEngine::ImpAdjustBlocks`,
        // `editeng/source/editeng/impedit3.cxx:1694-1701`). Nothing above moves — the break
        // positions, the line count and every row height are decided before this and are not
        // touched by it, which is why a workbook with no justified cell cannot be affected.
        //
        // A paragraph's **last** line is not stretched (`!bEOC`, `:1699`), and `distributed` is
        // exactly the setting that lifts that exemption — `bDistLastLine` is
        // `GetJustifyMethod(nPara) == SvxCellJustifyMethod::Distribute` (`:1696`), which Calc sets
        // from `ATTR_HOR_JUSTIFY_METHOD`. A cell that does not wrap has one line, and that line is
        // its paragraph's last, so only `distributed` reaches this at all without a wrap.
        //
        // `format.Horizontal` and not the resolved `horizontal`: `Resolve` has already turned both
        // of these into `Left`, which is where a justified line *starts*, and the stretch is the
        // half of the same setting it could not carry.
        if (format.Horizontal is SheetHorizontalAlignment.Justify
                or SheetHorizontalAlignment.Distributed
            && available > Length.Zero)
        {
            bool distributesLastLine =
                format.Horizontal == SheetHorizontalAlignment.Distributed;

            for (int at = 0; at < lines.Count; at++)
            {
                bool endsParagraph = at == lines.Count - 1 || paragraphStarts.Contains(at + 1);
                if (endsParagraph && !distributesLastLine) continue;

                lines[at] = SheetText.Justified(lines[at], available);
            }
        }

        // The block's height is the sum of its lines rather than a pitch times a count, because a
        // rich cell's lines are not all the same height: EditEngine makes a line as tall as the
        // tallest portion on it. For a cell in one face the two are the same number. The last line
        // contributes its whole height whatever the pitch is — there is nothing below it to close
        // up against.
        Length textHeight = lines[^1].LineHeight;
        for (int at = 0; at < lines.Count - 1; at++) textHeight += Pitch(lines[at]);

        Length top = VerticalOffset(format.Vertical, cell.Box.Height, textHeight, margin);
        Length y = cell.Box.Y + top;

        List<PlacedLine> placed = new(lines.Count);
        foreach (SheetTextRun line in lines)
        {
            placed.Add(new PlacedLine(
                line,
                Horizontal(
                    horizontal, cell.Box, AlignedWidth(horizontal, line, breaks ? available : Length.Zero),
                    leftTotal, margin + indent, margin),
                y + line.Ascent));
            y += Pitch(line);
        }

        // A wrapping field is clipped to its cell, vertically as well as horizontally, and always
        // — `bWrapFields` is OR'd straight into `bClip` before anything is measured
        // (`ScOutputData::Clip`, output2.cxx:3442-3445), so the "don't clip for text height when
        // printing rows with optimal height" branch below it never gets to say otherwise. The clip
        // rectangle is `aAreaParam.maClipRect`, which the text never grew.
        //
        // Read out of the reference's own content stream on `Published_Issuances_2024.xlsx`: 22
        // clip rectangles, one per link cell, each `402.096..534.824` wide and each exactly as tall
        // as its row — 19.006, 12.939, 6.872, 28.689 — including the tall rows where nothing
        // overflows. Ours had grown four of them to 12.671 and 14.087 to fit the text, and two
        // blind reviewers reading the rendered pair independently reported the consequence: our
        // second line painted over the row beneath it where the reference's is cut off.
        //
        // The `re W* n` these appear as is why an earlier grep found none: LibreOffice writes the
        // even-odd form, and a pattern written for `W n` reports a reference that never clips.
        bool fieldClip = breaks && cell.IsField;

        // Everywhere else the clip never cuts the text vertically. Calc does not clip a printed
        // cell's height unless the row's height was set by hand ("no vertical clipping when
        // printing cells with optimal height", output2.cxx:2093).
        //
        // That comment used to continue "and a wrapped cell taller than its row is exactly the
        // case that would lose a line to it", as though the clip were the only way such a cell
        // could lose one. It is not, and the other way is upstream of every clip: the lines past
        // the room are never *formatted*. See `SkipOutsideFormat`, which is where a wrapped cell
        // taller than its row loses its tail — in the text layer as well as in the ink.
        Length textTop = fieldClip
            ? cell.Box.Y
            : Length.Min(cell.Box.Y, placed[0].Baseline - lines[0].Ascent);
        Length textBottom = fieldClip
            ? cell.Box.Y + cell.Box.Height
            : Length.Max(cell.Box.Y + cell.Box.Height, placed[^1].Baseline + lines[^1].Descent);

        return new Placement(
            placed, area.IsClipped || fieldClip, area.Left, area.Right, textTop, textBottom);
    }

    /// <summary>
    /// Calc's default alignment, which is the cell's <em>type</em> rather than a constant.
    /// </summary>
    /// <remarks>
    /// <c>getAlignmentFromContext</c> (<c>output2.cxx:1443</c>): a value goes right and everything
    /// else left. The right-to-left branch, which turns both round when the text begins with a
    /// right-to-left character, is not reproduced — nothing in the corpus reaches it and it needs
    /// the cell's writing direction, which no reader carries yet.
    /// </remarks>
    private static SheetHorizontalAlignment Resolve(SheetHorizontalAlignment stated, bool isValue)
        => stated switch
        {
            SheetHorizontalAlignment.General => isValue
                ? SheetHorizontalAlignment.Right
                : SheetHorizontalAlignment.Left,

            // Justified and distributed text is placed from the left and stretched. Placing is
            // all this decides — the stretch is `SheetText.Justified`, applied to the lines a
            // moment after they are broken. Fill repeats from the left as well.
            SheetHorizontalAlignment.Justify or SheetHorizontalAlignment.Distributed
                or SheetHorizontalAlignment.Fill => SheetHorizontalAlignment.Left,

            _ => stated,
        };

    /// <summary>
    /// Whether the cell wraps.
    /// </summary>
    /// <remarks>
    /// Justified alignment forces it, in either direction, and a plain number never takes it:
    /// "disable automatic line breaks for all number formats" (<c>output2.cxx:1834</c>, i#111387),
    /// which is why a wide number in a wrapping column shows <c>###</c> rather than folding onto a
    /// second line. A date or a time is not a plain number format and does wrap.
    /// </remarks>
    internal static bool Breaks(SheetCellFormat format, bool isValue)
    {
        bool breaks = format.Wraps
                      || format.Horizontal is SheetHorizontalAlignment.Justify
                          or SheetHorizontalAlignment.Distributed
                      || format.Vertical is SheetVerticalAlignment.Justify
                          or SheetVerticalAlignment.Distributed;

        return breaks && isValue ? !format.HasPlainNumberFormat : breaks;
    }

    /// <summary>
    /// Whether the cell's text holds a character that sends Calc to the EditEngine.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScDrawStringsVars::HasEditCharacters</c> (<c>output2.cxx:823-847</c>), consulted at
    /// <c>output2.cxx:1812</c> before anything about the output area has been decided. Seven code
    /// points force it — a no-break space, a soft hyphen, a zero-width space, the two bidi marks,
    /// a non-breaking hyphen and a word joiner — and the consequence is not cosmetic:
    /// <c>DrawStrings</c> skips the cell entirely and <c>DrawEditStandard</c> draws it, which
    /// clips the string to the cell and never shortens it. The plain path drops the characters it
    /// cannot show; the EditEngine path leaves them in the text layer behind a clip.
    /// </para>
    /// <para>
    /// The no-break space is excluded when the cell has a repeat directive, which is tdf#122676:
    /// "Ignore CHAR_NBSP (this is thousand separator in any number) if repeat character is set".
    /// The string tested is the cell's <em>display</em> text, so a number whose format groups with
    /// a no-break space reaches this the same way a piece of typed text does.
    /// </para>
    /// </remarks>
    /// <param name="text">The cell's display text.</param>
    /// <param name="fillAt">Where the repeat directive expands, or −1 when there is none.</param>
    internal static bool HasEditCharacters(string text, int fillAt = -1)
    {
        foreach (char c in text)
        {
            switch (c)
            {
                case '\u00A0' when fillAt < 0:  // CHAR_NBSP
                case '\u00AD':                  // CHAR_SHY
                case '\u200B':                  // CHAR_ZWSP
                case '\u200E':                  // CHAR_LRM
                case '\u200F':                  // CHAR_RLM
                case '\u2011':                  // CHAR_NBHY
                case '\u2060':                  // CHAR_WJ
                    return true;
            }
        }

        return false;
    }

    /// <summary>
    /// How many lines a cell's text breaks into at a width.
    /// </summary>
    /// <remarks>
    /// For <see cref="SheetOptimalRowHeights"/>, which needs the count and none of the rest of the
    /// placement — the height it is deriving is what decides where the lines go, so it cannot ask
    /// for them. A hard break starts a line of its own whatever the width is, which is why the
    /// text is split before it is wrapped rather than handed to the layouter whole.
    /// </remarks>
    /// <param name="text">The cell's text.</param>
    /// <param name="face">The face it is set in.</param>
    /// <param name="size">The em size.</param>
    /// <param name="available">The room its lines have, margins already taken off.</param>
    internal static int LineCount(string text, SheetFace face, Length size, Length available)
    {
        if (text.Length == 0) return 0;

        ParagraphLayouter? layouter = null;
        int lines = 0;

        foreach (string paragraph in
                 text.Replace("\r\n", "\n", StringComparison.Ordinal).Split(['\n', '\r']))
        {
            if (paragraph.Length == 0 || available <= Length.Zero)
            {
                lines++;
                continue;
            }

            layouter ??= Layouters.GetOrAdd(
                face.Reference.FaceKey,
                _ => new ParagraphLayouter(
                    face.Face, shaper: SheetFonts.Shaper, breaksOverflowingBlanks: true));

            LaidOutParagraph laid = layouter.Layout(
                paragraph, emSize: size, textAreaWidth: available, options: SheetText.NoKerning);

            lines += Math.Max(1, laid.Lines.Count);
        }

        return lines;
    }

    /// <summary>
    /// Whether a wrapping cell's text begins at or past its own column's right edge, in which
    /// case none of it is drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Only a wrapping cell is clipped to its column at all</strong>, and only a wrapping
    /// cell is given a paper: <c>DrawEditParam::calcPaperSize</c>
    /// (<c>sc/source/ui/view/output2.cxx</c>:2684-2700) sets the EditEngine's paper to
    /// <c>rAlignRect.GetWidth() − nLeftM − nRightM</c> and is called only under
    /// <c>if (rParam.mbBreak)</c>, while a cell that does not wrap keeps the initial
    /// <c>Size(1000000, 1000000)</c> and writes across whatever is beside it. The left margin
    /// carries the indent — <c>calcMargins</c> (<c>:2665-2682</c>) adds <c>ATTR_INDENT</c> to
    /// whichever side the cell is aligned to — so an indented cell in a column narrower than that
    /// indent has its whole block laid out beyond the column's right edge, and
    /// <c>ImpEditEngine::DrawText_ToRectangle</c> then strips its portions against the cell's
    /// rectangle and returns without emitting one, <c>!aContentRange.overlaps(aClipRange)</c>
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:3408-3440).
    /// </para>
    /// <para>
    /// <strong>The predicate is the block's start against the column's width, and it is not the
    /// paper's own sign.</strong> A negative paper is not enough: with the indent removed,
    /// 26.2.4.2 draws every character of the same cell in the same column, one per line, because
    /// <c>calculateMaxLineWidth</c> clamps the line width to one unit
    /// (<c>impedit3.cxx</c>:530-545) and the block still begins inside the cell. Measured over
    /// twelve renderings of <c>075_Idea_planner_tasks</c> — eight column widths at one indent
    /// level and four one-attribute variants of the indent — the step falls between column widths
    /// of <strong>5.25 pt and 6.11 pt</strong> against a margin and indent summing to
    /// <strong>5.70 pt</strong>, and the same file with <c>indent="1"</c> removed is drawn in full
    /// at every width. <c>features/sheet-narrow-wrap.fods</c> is the other side of it: a 1.42 pt
    /// column with no indent, where 26.2.4.2 draws <c>N A R R O W</c> on six lines and a rule
    /// keyed on the paper's sign would draw none of them.
    /// </para>
    /// <para>
    /// Three further one-attribute variants say what does <em>not</em> decide it: removing
    /// <c>wrapText</c> makes the reference draw the whole string across its neighbours exactly
    /// where this tree drew it, emptying the occupied neighbour changes nothing on either side,
    /// and a row twenty times as tall changes nothing either — so it is neither the spill rule nor
    /// <see cref="SkipOutsideFormat"/>. <c>probes/xlsx-chart-r84/idea-variants.py</c>.
    /// </para>
    /// <para>
    /// A right-aligned cell carries its indent on the other side, and this uses the same magnitude
    /// for it because <c>calcMargins</c> does; no document in the corpus has one, so that half
    /// follows from the C++ rather than from a measurement.
    /// </para>
    /// </remarks>
    /// <param name="width">The cell's own column width.</param>
    /// <param name="leftTotal">Its left margin plus its indent — where the text begins.</param>
    internal static bool StartsOutsideItsCell(Length width, Length leftTotal)
        => leftTotal >= width;

    /// <summary>
    /// The character ranges a cell in several formats breaks into at a width.
    /// </summary>
    /// <remarks>
    /// The ranges rather than the count, because a rich cell's lines are not all the same height:
    /// EditEngine makes a line as tall as the tallest portion on it
    /// (<c>ImpEditEngine::CreateLines</c>, <c>editeng/source/editeng/impedit3.cxx:1516-1519</c>,
    /// over the per-portion maxima <c>RecalcFormatterFontMetrics</c> accumulates at <c>:3160</c>),
    /// so <see cref="SheetOptimalRowHeights"/> has to know which portions sit on which line. The
    /// breaking itself is the same run-aware path <see cref="Wrap"/> takes, so a row is measured
    /// against exactly the lines the cell will be drawn with.
    /// <para>
    /// <strong>With one addition the layouter cannot make: a trailing hard break leaves an empty
    /// paragraph, and it is a line of the row's height.</strong> The layouter ends a paragraph on
    /// its break and never opens the empty one after it — <c>"a\n"</c> lays out as one line where
    /// <see cref="LineCount"/>, which splits on the break before it wraps anything, answers two.
    /// The two have to give the same number, because a plain cell and a rich one holding the same
    /// characters are the same row in Calc: an <c>EditTextObject</c> holds one paragraph per
    /// <c>text:p</c>, the trailing empty one included, and <c>ScColumn::GetNeededSize</c>'s height
    /// branch is <c>pEngine-&gt;GetTextHeight()</c> over all of them (<c>column2.cxx</c>:571-577).
    /// It costs no glyph — an empty paragraph draws nothing — which is why the count shows up in
    /// the height alone and <see cref="Wrap"/> is left as it is.
    /// </para>
    /// <para>
    /// Measured at 26.2.4.2 rather than argued. Row 5 of <c>Reader Instructions</c> in
    /// <c>TK-Syllabus-Comparison-Document-v2.ods</c> holds eighteen paragraphs of which the
    /// eighteenth is empty, and the reference's own <c>--convert-to fods</c> gives that row
    /// <strong>4864.752 twips</strong>; deleting that one paragraph and converting back gives
    /// <strong>4597.2</strong>, one line less, which is what this tree computed without the rule.
    /// See <c>dotnet/probes/sheet-wrap-r99</c>. Its reach is measured rather than censused:
    /// 1261 cells in 63 of the 550 <c>.ods</c> and <c>.xlsx</c> sheets documents state a trailing
    /// empty paragraph and <strong>10 of the 63 change a rendering</strong> — the other 53
    /// documents hold it only in cells that are in one format, where the rule was already right,
    /// or that are not the tallest in their row.
    /// Sixty-one non-candidate renderings are byte-identical either way, and no gate column moves
    /// on any of the ten.
    /// </para>
    /// </remarks>
    /// <param name="text">The cell's text.</param>
    /// <param name="portions">The stretches it is split into.</param>
    /// <param name="face">The cell's own face, which names the layouter to break with.</param>
    /// <param name="available">The room its lines have, margins already taken off.</param>
    /// <param name="device">
    /// The grid every portion's em size is rounded onto before it is measured, or null to measure
    /// at the size the file states. Non-null only when a row is being measured rather than drawn:
    /// Calc decides a row's height against a 96 dpi virtual device, which can only set a font at a
    /// whole number of pixels. See <see cref="MetricGrid.ToEmSize"/>.
    /// </param>
    internal static IReadOnlyList<(int Start, int End)> RichLineRanges(
        string text,
        IReadOnlyList<SheetTextPortion> portions,
        SheetFace face,
        Length available,
        MetricGrid? device = null)
    {
        if (text.Length == 0) return [];

        ParagraphLayouter layouter = Layouters.GetOrAdd(
            face.Reference.FaceKey,
            _ => new ParagraphLayouter(
                face.Face, shaper: SheetFonts.Shaper, breaksOverflowingBlanks: true));

        LaidOutParagraph laid = layouter.Layout(
            Measured(text, portions, scale: 1.0, device), textAreaWidth: available);

        List<(int Start, int End)> ranges = new(laid.Lines.Count + 1);
        foreach (LineBox box in laid.Lines)
            ranges.Add((box.Line.Start, Math.Min(box.Line.End, text.Length)));

        // The empty paragraph a trailing break leaves. Empty, so no portion covers it and
        // `RichPixels` measures it in the cell's own face — which is what EditEngine gives a
        // paragraph holding no portion of its own.
        if (IsHardBreak(text[^1])) ranges.Add((text.Length, text.Length));

        return ranges;
    }

    // --------------------------------------------------------------------------------- fill

    /// <summary>
    /// Where a <c>*c</c> fill directive expands in this cell's text, and with which character.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reader already produced the cell's text with the directive dropped, because
    /// extraction has no column to fill. Finding the position again means putting the value
    /// through the code a second time with <c>NumberFormatter.FillMarker</c> left in — which is
    /// only done for the formats that carry one, and those are the accounting formats.
    /// </para>
    /// <para>
    /// The re-render is trusted only when it reproduces the text the reader produced. The two
    /// calls resolve the workbook's epoch separately and layout does not carry it, so a date
    /// format with a fill would come back different — and a disagreement must change nothing
    /// rather than replace a correct string with a plausible one.
    /// </para>
    /// </remarks>
    private static (string Text, int At, char Fill) Fill(in SheetCellText cell)
    {
        if (cell.Format.NumberFormat is not { HasFillDirective: true } code) return (cell.Text, -1, '\0');
        if (cell.Value is not double value) return (cell.Text, -1, '\0');

        string marked = NumberFormatter.Format(code, value, keepFillMarkers: true);
        int at = marked.IndexOf(NumberFormatter.FillMarker, StringComparison.Ordinal);
        if (at < 0 || at + 1 >= marked.Length) return (cell.Text, -1, '\0');

        char fill = marked[at + 1];
        string plain = marked.Remove(at, 2);
        return plain == cell.Text ? (plain, at, fill) : (cell.Text, -1, '\0');
    }

    /// <summary>
    /// Pads the fill point with as many copies of the fill character as the column has room for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScDrawStringsVars::RepeatToFill</c> (<c>output2.cxx:572</c>), including the two
    /// deliberate truncations it marks in its own comments. The character's width is taken from
    /// a twenty-character sample rather than from one copy — "measuring a string containing a
    /// single copy of the repeat char is inaccurate" — and both the width and the count are
    /// truncated towards zero, so the fill can never overrun the column by a rounding.
    /// </para>
    /// <para>
    /// Nothing is added when the space left is no wider than one character: an accounting cell
    /// in a column that only just fits its number shows its symbol against its digits, which is
    /// what Calc draws.
    /// </para>
    /// </remarks>
    /// <returns>The padded text, or null when nothing fits.</returns>
    private static string? RepeatToFill(
        string text, int at, char fill, SheetFace face, Length size, Length available, Length width)
    {
        const int SampleSize = 20;

        if (at > text.Length || available <= Length.Zero) return null;
        if (SheetText.Shape(new string(fill, SampleSize), face, size) is not { } sample) return null;

        double averageWidth = (double)sample.Width.Emu / SampleSize;
        long characterWidth = (long)averageWidth;
        if (characterWidth < 1) return null;

        long spaceToFill = (available - width).Emu;
        if (spaceToFill <= characterWidth) return null;

        int count = (int)(spaceToFill / averageWidth);
        return count <= 0 ? null : text.Insert(at, new string(fill, count));
    }

    // -------------------------------------------------------------------------- output area

    /// <summary>How far the text may run, and how much of it is cut off at either end.</summary>
    private readonly record struct Area(Length Left, Length Right, Length LeftMissing, Length RightMissing)
    {
        public bool LeftClip => LeftMissing > Length.Zero;

        public bool RightClip => RightMissing > Length.Zero;

        public bool IsClipped => LeftClip || RightClip;

        public Length Width => Right - Left;

        public Area Unclipped() => this with { LeftMissing = Length.Zero, RightMissing = Length.Zero };
    }

    /// <summary>
    /// The rectangle the text is allowed to occupy: the cell, widened into empty neighbours.
    /// </summary>
    /// <remarks>
    /// <c>ScOutputData::GetOutputArea</c> (<c>output2.cxx:1204</c>). Three of its conditions
    /// decide the visible behaviour. Only what is missing is walked for, so a left-aligned string
    /// spills to the right and a right-aligned one to the left; the walk stops at the first
    /// neighbour that is not available, which is what clips a long string beside an occupied cell
    /// rather than writing over it; and a value never spills at all — a number too wide shows
    /// <c>###</c> instead, which is the asymmetry that makes a spreadsheet's overflow rule
    /// surprising.
    /// </remarks>
    private static Area OutputArea(
        in SheetTextContext context,
        in SheetCellText cell,
        SheetHorizontalAlignment horizontal,
        Length needed,
        bool blocked)
    {
        Length left = cell.Box.X;
        Length right = cell.Box.X + cell.Box.Width;
        if (needed <= cell.Box.Width) return new Area(left, right, Length.Zero, Length.Zero);

        Length missing = needed - cell.Box.Width;
        Length leftMissing = Length.Zero;
        Length rightMissing = Length.Zero;

        switch (horizontal)
        {
            case SheetHorizontalAlignment.Left:
                rightMissing = missing;
                break;
            case SheetHorizontalAlignment.Right:
                leftMissing = missing;
                break;
            case SheetHorizontalAlignment.Centre:
                leftMissing = missing / 2;
                rightMissing = missing - leftMissing;
                break;
            default:
                break;
        }

        if (!blocked)
        {
            int at = cell.Column;
            while (rightMissing > Length.Zero
                   && at < SheetAddress.MaxColumn
                   && context.IsAvailable(cell.Row, at + 1))
            {
                at++;
                Length add = context.ColumnWidth(at);
                rightMissing -= add;
                right += add;
            }

            at = cell.Column;
            while (leftMissing > Length.Zero && at > 0 && context.IsAvailable(cell.Row, at - 1))
            {
                at--;
                Length add = context.ColumnWidth(at);
                leftMissing -= add;
                left -= add;
            }
        }

        return new Area(left, right, leftMissing, rightMissing);
    }

    // ------------------------------------------------------------------------------- shrink

    /// <summary>
    /// Scales the font down until the text fits, the way Calc's <c>ShrinkEditEngine</c> does.
    /// </summary>
    /// <remarks>
    /// A measure-and-retry rather than a division, and the first guess is a division: the scale is
    /// the integer percentage <c>available × 100 / textWidth</c>, and if that still does not fit
    /// it is cut by a tenth up to seven times (<c>output2.cxx:1864-1885</c>). The integer truncation
    /// is what makes the answer reproducible — <c>sheet-cell-text</c>'s shrunk cell comes out at
    /// 87% of ten point in both renderers, which is 8.70 pt rather than the 8.74 an exact
    /// proportion would give.
    /// </remarks>
    private static (SheetTextRun Run, long Percent) Shrink(
        Func<int, int, long, SheetTextRun?> shape, int length, SheetTextRun run, Length available)
    {
        long percent = available.Emu * 100 / run.Width.Emu;
        if (percent <= 0) return (run, 100);

        SheetTextRun scaled = run;
        long reached = 100;

        for (int attempt = 0; attempt <= ShrinkAttempts; attempt++)
        {
            if (shape(0, length, percent) is not { } shaped) break;

            scaled = shaped;
            reached = percent;
            if (shaped.Width <= available) break;

            percent = percent * 9 / 10;
            if (percent <= 0) break;
        }

        return (scaled, reached);
    }

    // --------------------------------------------------------------------------------- hash

    /// <summary>
    /// What a numeric cell too narrow for its text draws instead.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScDrawStringsVars::SetTextToWidthOrHash</c> (<c>output2.cxx:610</c>), and the rule is
    /// narrower than "a number that does not fit shows hashes". Only a <em>non-</em><c>General</c>
    /// format hashes outright; a <c>General</c> one is re-rendered with as many characters as the
    /// column has digit widths, and only falls back to scientific notation from there. That is why
    /// 123 456 789 012 in a 43 pt column draws as <c>1.2E+11</c> in Calc and not as <c>###</c> —
    /// the trap that cost the most time here, because a port that hashes every value that does not
    /// fit produces plausible output and disagrees with the reference on every wide number.
    /// </para>
    /// <para>
    /// Rendering only. <c>xray-office extract</c> keeps reporting the full text, which is a recorded
    /// decision: hashes are a function of a column width that extracted text does not have.
    /// </para>
    /// </remarks>
    private static (SheetTextRun Run, string Text, bool Hashed) Hash(
        in SheetCellText cell, SheetFace face, Length size, Length available, SheetTextRun run)
    {
        if (cell.Value is double value && cell.Format.HasGeneralFormat)
        {
            Length digit = face.MaxDigitWidthAt(size);
            int characters = digit > Length.Zero ? (int)(available.Emu / digit.Emu) : 0;
            string shortened = SheetGeneralWidth.Render(value, characters);

            // **The shortening can fail, and then the cell hashes like any other.** This is the
            // last three lines of `SetTextToWidthOrHash` — "Even after the decimal adjustment the
            // text doesn't fit. Give up." (`output2.cxx:704-710`) — and leaving it out is what
            // made a column 0.43 characters wide draw `1E+00` where Calc draws `###`, 1099 times
            // in one workbook. Re-shape and re-measure rather than counting characters: the
            // budget is digit widths and the answer is a shaped run, so only the run can decide.
            if (SheetText.Shape(shortened, face, size) is { } fitted && fitted.Width <= available)
            {
                return (fitted, shortened, false);
            }
        }

        return (SheetText.Shape(HashText, face, size) ?? run, HashText, true);
    }

    // ------------------------------------------------------------------------------ shorten

    /// <summary>
    /// Drops the characters a clipped string cannot show.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice does this for speed — "if the string is clipped, make it shorter for better
    /// performance since drawing by HarfBuzz is quite expensive" (<c>output2.cxx:2202</c>) — and
    /// it is reproduced because it is <em>visible</em> in the output rather than only faster: the
    /// PDF holds the shortened text, so a cell blocked by its neighbour extracts as the 23
    /// characters that fit rather than the 31 it holds. Reproducing it is what makes a
    /// run-for-run comparison of glyph counts mean anything.
    /// </para>
    /// <para>
    /// The estimate is deliberately crude on both sides — the ratio of visible width to total
    /// width, times the character count, plus one — so it over-keeps rather than under-keeps and
    /// the clip does the rest. Right-aligned text keeps its <em>end</em>, and keeping it needs no
    /// compensating shift: dropping the head of a string leaves every remaining glyph where it
    /// already was, and <see cref="Horizontal"/> is handed the shortened run's own width, so
    /// <c>right − margin − shortened</c> is exactly where the tail was standing. Shifting it right
    /// by the width dropped carried the whole run over the cell's right edge by that much.
    /// </para>
    /// </remarks>
    private static SheetTextRun Shorten(
        SheetTextRun run,
        string text,
        Func<int, int, long, SheetTextRun?> shape,
        long percent,
        SheetHorizontalAlignment horizontal,
        Area area)
    {
        if (run.Width <= Length.Zero || text.Length == 0) return run;

        if (horizontal == SheetHorizontalAlignment.Left && area.RightClip)
        {
            double ratio = (double)(run.Width - area.RightMissing).Emu / run.Width.Emu;
            if (ratio is <= 0.0 or >= 1.0) return run;

            int keep = Math.Clamp((int)(ratio * text.Length) + 1, 1, text.Length);
            return shape(0, keep, percent) ?? run;
        }

        if (horizontal == SheetHorizontalAlignment.Right && area.LeftClip)
        {
            double ratio = (double)(run.Width - area.LeftMissing).Emu / run.Width.Emu;
            if (ratio is <= 0.0 or >= 1.0) return run;

            int keep = Math.Clamp((int)(ratio * text.Length) + 1, 1, text.Length);
            return shape(text.Length - keep, text.Length, percent) ?? run;
        }

        return run;
    }

    // --------------------------------------------------------------------------------- wrap

    /// <summary>
    /// Breaks a wrapping cell into lines.
    /// </summary>
    /// <remarks>
    /// Through the shared <see cref="ParagraphLayouter"/> rather than a second line breaker: the
    /// greedy fill, the trailing-space rule and the "a word too long takes the line alone" rule
    /// are the same in a cell as in a paragraph, and having two implementations of them would mean
    /// two sets of break positions to keep in step. Only the vertical geometry is Calc's own, so
    /// only the line <em>ranges</em> are taken from the result and the pitch is applied here.
    /// <para>
    /// <paramref name="atomic"/> is true for a cell that is one EditEngine field. The layouter is
    /// then given <see cref="SheetFieldBreaker"/> instead of the Unicode one, so the text offers no
    /// break opportunity and every line it produces is the fill loop's character-level chop — which
    /// is what LibreOffice does, and why a hyperlinked URL breaks mid-token where the same string
    /// unlinked breaks after a solidus.
    /// </para>
    /// <para>
    /// <c>paragraphStarts</c> receives the index, into the returned list, of each line that begins
    /// a paragraph — always starting with 0. Only <see cref="SkipOutsideFormat"/> reads it, and it
    /// needs it because Calc's "format at least a few lines" allowance is counted per paragraph
    /// rather than per cell: <c>nLine</c> in <c>ImpEditEngine::CreateLines</c> is the index within
    /// one, and the paragraph after a full cell is dropped whole.
    /// </para>
    /// </remarks>
    private static List<SheetTextRun> Wrap(
        string text,
        IReadOnlyList<SheetTextPortion>? portions,
        SheetFace face,
        Length size,
        double scale,
        Length available,
        Func<int, int, long, SheetTextRun?> shape,
        long percent,
        out List<int> paragraphStarts,
        bool atomic = false)
    {
        paragraphStarts = [0];

        SheetTextRun? whole = shape(0, text.Length, percent);
        if (whole is null) return [];

        // A hard break is not a suggestion, so the "it all fits" shortcut cannot take it: the
        // text has to reach the layouter, which breaks at one whatever the width says. Only the
        // shortcut is conditional — a cell with no break in it measures and draws exactly as it
        // did. `LineCount` beside this has always split on the break first, so before this the
        // reserved row height and the drawn lines were computed by two rules that disagreed.
        //
        // A field takes the shortcut on width alone: its representation is not in the content
        // node, so a break character inside one is a character like any other and starts nothing.
        // No room is not a reason to stop breaking. `ImpEditEngine::calculateMaxLineWidth`
        // (`editeng/source/editeng/impedit3.cxx`:530-545) ends with
        // `if (nMaxLineWidth <= 0) nMaxLineWidth = 1;`, so a cell narrower than its own margins is
        // broken against one unit and takes a character per line rather than being written across
        // its neighbours in one. Measured on `features/sheet-narrow-wrap.fods`, whose 1.42 pt
        // column 26.2.4.2 draws as `N A R R O W` on six lines.
        if (available <= Length.Zero) available = NarrowestWrapWidth;

        if (whole.Width <= available && (atomic || !HoldsHardBreak(text)))
            return [whole];

        // Two layouters per face, because the breaker is fixed at construction and the cache is
        // keyed by string. The prefix cannot collide with a face key, which is what `Ordinal`
        // comparison on the key makes safe.
        ParagraphLayouter layouter = atomic
            ? Layouters.GetOrAdd(
                "\0field\0" + face.Reference.FaceKey,
                _ => new ParagraphLayouter(
                    face.Face, breaker: SheetFieldBreaker.Instance, shaper: SheetFonts.Shaper,
                    breaksOverflowingBlanks: true))
            : Layouters.GetOrAdd(
                face.Reference.FaceKey,
                _ => new ParagraphLayouter(
                    face.Face, shaper: SheetFonts.Shaper, breaksOverflowingBlanks: true));

        // A rich cell breaks against its own runs rather than against one face, through the
        // layouter's run-aware overload: a bold word is wider than the same characters set
        // regular, so measuring the line in the cell's face alone puts the break in the wrong
        // place. The single-face path is left exactly as it was.
        LaidOutParagraph laid = portions is null
            ? layouter.Layout(
                text, emSize: size, textAreaWidth: available, options: SheetText.NoKerning)
            : layouter.Layout(
                Measured(text, portions, scale), textAreaWidth: available);

        List<SheetTextRun> lines = [];
        foreach (LineBox box in laid.Lines)
        {
            // To End rather than to VisibleEnd: Calc's own output shows a line's trailing spaces,
            // so a reference PDF's first wrapped line of "Wrapped text that needs …" holds
            // eighteen glyphs, not the seventeen the visible text has. The break character
            // itself is the one thing dropped — it is Writer's break portion, "zero width, and
            // no glyph", and a cell whose lines carry it would both measure the character's
            // advance into a centred line's width and put it in the PDF's text layer.
            int start = box.Line.Start;
            int full = Math.Min(box.Line.End, text.Length);
            int end = full;
            while (end > start && IsHardBreak(text[end - 1])) end--;

            // A field's text is one indivisible paragraph however many break characters its
            // representation happens to hold, so only the real breaker's lines start one.
            if (!atomic && start > 0 && start <= text.Length && IsHardBreak(text[start - 1]))
                paragraphStarts.Add(lines.Count);

            // A break on its own is an empty paragraph, and an empty paragraph is still a line
            // with a height. It is shaped from the break — a run's ascent and descent come from
            // its face and size rather than from its glyphs — and then emptied, so that the line
            // occupies its pitch without putting a .notdef box on the page or a U+000A in the
            // text layer.
            if (end == start)
            {
                if (shape(start, full, percent) is { } blank) lines.Add(Blank(blank));
                continue;
            }

            if (shape(start, end, percent) is { } shaped) lines.Add(shaped);
        }

        return lines.Count == 0 ? [whole] : lines;
    }

    /// <summary>
    /// How many lines of a paragraph are formatted before the room runs out is allowed to stop it.
    /// </summary>
    /// <remarks>
    /// <c>nLine &gt; 2</c> in the guard quoted on <see cref="SkipOutsideFormat"/>, whose own comment
    /// says why: "Format at least two lines though, in case something detects whether the text has
    /// been wrapped or something similar." Counted from the outside — the number of lines that
    /// survive however short the row is — it is <strong>four</strong>, measured rather than read off
    /// the increment: a 0.2 cm row and a 1.6 cm row both draw four lines of the same cell through
    /// 26.2.4.2, and so does a 1 cm row at every vertical alignment that truncates at all.
    /// </remarks>
    private const int MinimumFormattedLines = 4;

    /// <summary>
    /// Drops the lines a wrapping cell has no room to format.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This is not a clip, and reading it as one is what hid it.</strong> A clip cuts ink
    /// and leaves every glyph in the PDF's text layer — which is why the horizontal rule
    /// <see cref="ClipTo"/> reproduces moves no word count in either direction. This rule is
    /// upstream of drawing: the EditEngine is told not to <em>format</em> the lines past the room
    /// it was given, so they are never laid out, never drawn, and never reach the text layer.
    /// It is the only thing on this path that moves a word count.
    /// </para>
    /// <para>
    /// Calc switches it on for every cell it sends to <c>DrawEditStandard</c>:
    /// </para>
    /// <code>
    /// rParam.mpEngine->EnableSkipOutsideFormat(rParam.meVerJust==SvxCellVerJustify::Top
    ///     || rParam.meVerJust==SvxCellVerJustify::Standard);   // output2.cxx:3115
    /// </code>
    /// <para>
    /// and the engine acts on it while it is building the lines
    /// (<c>ImpEditEngine::CreateLines</c>, <c>impedit3.cxx:1801-1806</c>):
    /// </para>
    /// <code>
    /// if( mbSkipOutsideFormat &amp;&amp; nLine > 2
    ///     &amp;&amp; !maStatus.AutoPageHeight() &amp;&amp; maPaperSize.Height() &lt; nCurrentPosY )
    ///     break;
    /// </code>
    /// <para>
    /// with a second, coarser guard one level up that drops a whole paragraph whose first line
    /// would start past the room (<c>impedit3.cxx:676-680</c>, <c>nPara != 0</c>).
    /// </para>
    /// <para>
    /// <strong>The room is the cell's, and only a wrapping cell has any.</strong>
    /// <c>calcPaperSize</c> (<c>output2.cxx:2684-2700</c>) sets the engine's paper to
    /// <c>rAlignRect.GetHeight() - nTopM - nBottomM</c>, and it is called only under
    /// <c>if (rParam.mbBreak)</c> — a cell that does not wrap keeps the initial
    /// <c>Size(1000000, 1000000)</c> and is never truncated however many hard breaks it holds.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 with an authored twelve-row sweep — Liberation Sans 10 pt in a 4 cm
    /// column, row heights 0.4 cm to 3.2 cm, pitch 11.20 pt — against
    /// <c>max(4, floor(paperHeight / pitch) + 1)</c>: <strong>twelve of twelve exact</strong>. Four
    /// further cases pin the guard rather than the arithmetic, all read out of the reference's own
    /// output:
    /// </para>
    /// <list type="table">
    ///   <item><term>vertical <c>bottom</c>, row far too short</term>
    ///     <description>all sixty words drawn — the guard excludes it</description></item>
    ///   <item><term>vertical <c>middle</c></term><description>likewise, all sixty</description></item>
    ///   <item><term>vertical unstated (<c>Standard</c>)</term>
    ///     <description>truncated to four lines, and still placed from the bottom</description></item>
    ///   <item><term>no wrap, twenty hard-break paragraphs in a 1 cm row</term>
    ///     <description>all twenty drawn</description></item>
    /// </list>
    /// <para>
    /// The comparison is strict, so a cell whose room is an exact multiple of its pitch gets one
    /// line more than the multiple: a 58 pt row at 11.20 pt draws <strong>six</strong>. That is
    /// why this walks and compares rather than dividing — <c>ceil</c> would answer five.
    /// </para>
    /// <para>
    /// <strong>It is not the optimal-height branch.</strong> The <c>CRFlags::ManualSize</c> test at
    /// <c>output2.cxx:3255-3261</c> decides only whether a hard clip rectangle is emitted around
    /// the ink; both sides of it truncate. Measured both ways: an authored manual-height row is
    /// truncated <em>and</em> carries a clip rectangle, and
    /// <c>sheets/batch-011/xls/T0A0D0000090006XLSE.xls</c>'s optimal-height rows are truncated with
    /// <strong>no clip operator on the page at all</strong>.
    /// </para>
    /// </remarks>
    /// <param name="lines">The wrapped lines, truncated in place.</param>
    /// <param name="paragraphStarts">Which of them begin a paragraph; see <see cref="Wrap"/>.</param>
    /// <param name="paperHeight">The cell's height less its top and bottom margins.</param>
    /// <param name="pitch">How far each line advances the next one.</param>
    private static void SkipOutsideFormat(
        List<SheetTextRun> lines,
        List<int> paragraphStarts,
        Length paperHeight,
        Func<SheetTextRun, Length> pitch)
    {
        Length y = Length.Zero;
        int nextParagraph = 0;
        int inParagraph = 0;

        for (int at = 0; at < lines.Count; at++)
        {
            if (nextParagraph < paragraphStarts.Count && paragraphStarts[nextParagraph] == at)
            {
                // A paragraph after the first is not formatted at all when the ones before it
                // have already used the room up — no line of it survives, not even the
                // allowance below.
                if (nextParagraph > 0 && y > paperHeight)
                {
                    lines.RemoveRange(at, lines.Count - at);
                    return;
                }

                nextParagraph++;
                inParagraph = 0;
            }

            y += pitch(lines[at]);

            if (inParagraph >= MinimumFormattedLines - 1 && y > paperHeight)
            {
                lines.RemoveRange(at + 1, lines.Count - at - 1);
                return;
            }

            inParagraph++;
        }
    }

    /// <summary>
    /// The same line with nothing on it: one segment, kept for its metrics, holding no glyphs.
    /// </summary>
    /// <remarks>
    /// The first segment only. A line's height is the tallest thing on it and there is nothing on
    /// this one, so the face and size the paragraph would have been set in is the whole of what
    /// an empty paragraph contributes.
    /// </remarks>
    private static SheetTextRun Blank(SheetTextRun run)
    {
        SheetTextSegment first = run.Segments[0];

        return new SheetTextRun(
            [first with
            {
                Glyphs = [],
                Clusters = [],
                Text = string.Empty,
                Offset = Length.Zero,
                Width = Length.Zero,
            }],
            Length.Zero);
    }

    /// <summary>Whether a cell's text holds a break that starts a line whatever the width is.</summary>
    /// <remarks>
    /// The same two characters <see cref="LineCount"/> splits on, and deliberately no more: the
    /// row height it derives and the lines <see cref="Wrap"/> draws have to be computed from one
    /// rule. Every reader that can put a break inside a cell produces one of these — BIFF's own
    /// U+000A survives <c>ReadRawUnicodeString</c> unchanged, SpreadsheetML writes
    /// <c>&amp;#10;</c>, and ODF's <c>text:line-break</c> is read as <c>'\n'</c>.
    /// </remarks>
    private static bool IsHardBreak(char character) => character is '\n' or '\r';

    /// <inheritdoc cref="IsHardBreak"/>
    private static bool HoldsHardBreak(string text)
    {
        foreach (char c in text)
        {
            if (IsHardBreak(c)) return true;
        }

        return false;
    }

    /// <summary>
    /// Whether the importers would have stored this cell as an <c>EditTextObject</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The distinction decides one thing here and it is not a small one: a cell Calc drew through
    /// <c>DrawStrings</c> loses the characters that will not fit — <c>ScDrawStringsVars</c> shortens
    /// the string before it is shown — and a cell it drew through <c>DrawEdit</c> keeps every one of
    /// them behind a clip. Only the second leaves the hidden tail in the PDF's text layer, which is
    /// the half a word count scores.
    /// </para>
    /// <para>
    /// Two things make such a cell, and both importers agree on them. A shared string carrying
    /// formatting runs becomes rich text (<c>putRichString</c>,
    /// <c>sc/source/filter/oox/sheetdatabuffer.cxx:125-133</c>;
    /// <c>XclImpString::SetToDocument</c>, <c>sc/source/filter/excel/xihelper.cxx:246-256</c>), and
    /// so does one holding a hard break, whether or not the cell wraps — the break makes it an edit
    /// cell even where <c>SetSingleLine</c> stops it from starting a line.
    /// </para>
    /// <para>
    /// Measured on <c>dotnet/probes/sheets-rest-01/mkclipprobe.py</c> under the installed 26.2.4.2,
    /// five rows differing in one property each, all in a column too narrow for their text with the
    /// neighbour occupied so that nothing may spill: the plain row's text layer holds
    /// <strong>22</strong> of its 130 characters, and the rich, hard-break and rich-plus-break rows
    /// hold all <strong>130</strong>. It is the whole of
    /// <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.xls</c>'s 1440-word deficit: its remediation and audit
    /// columns hold exactly this shape, a paragraph with blank lines between its parts.
    /// </para>
    /// </remarks>
    /// <param name="text">The cell's display text.</param>
    /// <param name="portions">Its formatting runs, or null when it has none.</param>
    private static bool IsEditCell(string text, IReadOnlyList<SheetTextPortion>? portions)
        => portions is { Count: > 0 } || HoldsHardBreak(text);

    /// <summary>A rich cell's text, shaped run by run so that it can be broken into lines.</summary>
    private static MeasuredParagraph Measured(
        string text,
        IReadOnlyList<SheetTextPortion> portions,
        double scale,
        MetricGrid? device = null)
    {
        List<FormattedRun> runs = [];
        foreach (SheetTextPortion portion in portions)
        {
            // The same two fallbacks SheetText.ShapeRich takes, and for the same reason: a run
            // left out here is measured in whatever run precedes it (MeasuredParagraph.Normalise
            // fills a gap from the run before), so the line breaks in the wrong place and the
            // drawn segments no longer match the measured ones.
            SheetFace? face = SheetFonts.For(portion.Format) ?? SheetText.DefaultFace;
            if (face is null) continue;

            Length size = SheetText.SizeOf(SheetText.SizeStatedBy(portion.Format), scale, 100);
            if (device is { } grid) size = grid.ToEmSize(size);

            runs.Add(new FormattedRun(
                portion.Start, portion.Length, face.Value.Face, size, SheetText.NoKerning));
        }

        // With the same glyph fallback the single-face path measures through: a rich cell whose
        // portions name a Latin face and whose text is not Latin otherwise measures its ideographs
        // at that face's `.notdef` advance, and breaks its lines accordingly.
        return MeasuredParagraph.Measure(
            text,
            runs,
            itemisation: new ItemisationOptions { GlyphFallback = SheetFonts.Fallback });
    }

    // ---------------------------------------------------------------------------- placement

    /// <summary>
    /// The width a centred line is placed by, which is not always the width it draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A centred line whose trailing blanks overflow the room it was broken against
    /// starts left of the cell, and the cell's clip then takes the word that begins it.</strong>
    /// Measured on <c>Infotabelle_WLAN im Flugzeug.xlsx</c> page 2, a centred wrapping cell whose
    /// runs of spaces stand in for tab stops: line two carried 46 trailing spaces worth 151 pt of
    /// a 436 pt line against 283 pt of room, so it was placed at x = −25.2 pt in a cell clipped
    /// from 50.4 pt, and the bold word <c>kostenlos</c> was drawn entirely outside it. The word is
    /// in the file, in <c>xray-office extract</c>, in the wrapped line's range and in the shaped
    /// run; the placement alone lost it, and the PDF held <c>kostenlos</c> five times against the
    /// reference's six.
    /// </para>
    /// <para>
    /// EditEngine never reaches a negative offset here, and the reason is structural rather than a
    /// clamp: it keeps only as many trailing blanks as <em>fit</em>, so a wrapped line's width is
    /// at most the width it was broken against and <c>(nMaxLineWidth - nCenterWidth) / 2</c>
    /// cannot go below nought (<c>ImpEditEngine::CreateLines</c>,
    /// <c>editeng/source/editeng/impedit3.cxx:1643-1683</c>). We keep every blank up to the next
    /// word instead — see <see cref="Wrap"/>, which takes a line to its <c>End</c> because Calc
    /// draws a line's trailing spaces — so the invariant has to be restored here.
    /// </para>
    /// <para>
    /// Hence the two bounds, and neither alone is right:
    /// </para>
    /// <list type="bullet">
    /// <item><description>
    /// A line that fits keeps its full width, blanks included, because the reference keeps the
    /// blanks that fit. Nothing about a cell that was already right moves.
    /// </description></item>
    /// <item><description>
    /// A line that overflows is placed by at most the room it had, which is the reference's
    /// filled line: it starts at the left margin. That is where the reference puts both of the
    /// two lines measured — 52.044 pt and 52.611 pt — and where this now puts them, at 51.392 pt.
    /// The remaining 0.7 pt is the cell margin, and it is the same on every cell in the file.
    /// </description></item>
    /// <item><description>
    /// But never by less than its own visible width, or a centred word longer than its column
    /// would be pushed flush left instead of overflowing evenly. The reference does let that one
    /// hang out both sides, since there is no blank in it for the first rule to have caught.
    /// </description></item>
    /// </list>
    /// <para>
    /// <strong>What this does not fix</strong> is where the overflowing blanks end up. The
    /// reference carries them onto the <em>next</em> line as leading blanks; we drop them at the
    /// break. So the third line of that cell is still centred on its visible text alone, at
    /// 129.9 pt against the reference's 205.9 pt. That is a line-breaking difference in
    /// <see cref="Wrap"/> and is left alone here.
    /// </para>
    /// </remarks>
    /// <param name="horizontal">The resolved alignment; only centring is affected.</param>
    /// <param name="line">The line being placed.</param>
    /// <param name="available">The width the line was broken against, or nought when it was not.</param>
    private static Length AlignedWidth(
        SheetHorizontalAlignment horizontal, SheetTextRun line, Length available)
    {
        if (horizontal != SheetHorizontalAlignment.Centre
            || available <= Length.Zero
            || line.Width <= available)
        {
            return line.Width;
        }

        return Length.Max(line.WithoutTrailingBlanks, available);
    }

    /// <summary>Where a line starts, given its width and the cell's.</summary>
    /// <remarks>
    /// The centre case is not <c>(width - text) / 2</c>: Calc folds the two margins in
    /// asymmetrically — <c>(availWidth - textWidth + leftTotal - rightMargin) / 2</c>
    /// (<c>output2.cxx:2054</c>) — so an indented centred cell drifts right by half its indent.
    /// </remarks>
    private static Length Horizontal(
        SheetHorizontalAlignment horizontal,
        DocRect box,
        Length width,
        Length leftTotal,
        Length rightTotal,
        Length rightMargin)
        => horizontal switch
        {
            SheetHorizontalAlignment.Right => box.X + box.Width - width - rightTotal,
            SheetHorizontalAlignment.Centre => box.X + ((box.Width - width + leftTotal - rightMargin) / 2),
            _ => box.X + leftTotal,
        };

    /// <summary>How far below the cell's top the text block starts.</summary>
    /// <remarks>
    /// <c>Standard</c> is bottom, which Calc settles in one line before any drawing happens
    /// (<c>output2.cxx:348</c>). The centre case again folds the margins in asymmetrically.
    /// </remarks>
    private static Length VerticalOffset(
        SheetVerticalAlignment vertical, Length height, Length textHeight, Length margin)
        => vertical switch
        {
            SheetVerticalAlignment.Top or SheetVerticalAlignment.Justify
                or SheetVerticalAlignment.Distributed => margin,

            SheetVerticalAlignment.Centre => (height + margin - textHeight - margin) / 2,

            _ => height - textHeight - margin,
        };

    // ------------------------------------------------------------------------------ rotation

    /// <summary>
    /// Draws turned or stacked text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Calc turns the text about the cell's bottom-left corner and lets it run out of the cell,
    /// which is what makes a row of 45-degree column headings legible
    /// (<c>ScOutputData::DrawRotated</c>, <c>output2.cxx:4710</c>). Stacked text — Excel's
    /// rotation 255 and ODF's <c>style:direction="ttb"</c> — is a different shape again: one
    /// character per line, centred.
    /// </para>
    /// <para>
    /// A quarter turn is not an angle at all as far as Calc is concerned: it draws those through
    /// <c>DrawEditBottomTop</c> and <c>DrawEditTopBottom</c>, whose anchor, whose paper and whose
    /// alignment are all worked out differently. See <see cref="IsQuarterTurned"/>, which is what
    /// splits the two paths below.
    /// </para>
    /// <para>
    /// What is <em>not</em> reproduced is the clipping of rotated text against its neighbours,
    /// which needs the rotated bounding box fed back into the row. The rotated cell's effect on its
    /// row's <em>height</em> is now reproduced — see <see cref="SheetOptimalRowHeights"/>.
    /// </para>
    /// </remarks>
    private static void DrawRotated(
        IDrawingSink sink,
        in SheetTextContext context,
        in SheetCellText cell,
        Placement placement,
        Colour fallback)
    {
        if (cell.Format.IsStacked)
        {
            DrawStacked(sink, context, cell, placement, fallback);
            return;
        }

        Length margin = cell.Format.Margin * context.Scale;
        bool quarter = IsQuarterTurned(cell.Format);

        // Anticlockwise the block runs up and to the right of the cell's bottom-left corner;
        // clockwise it runs down and to the left, so it hangs from a point its own cross-extent to
        // the right of the top-left. Calc says exactly that as `aLogicStart.AdjustY(aPSize.Width())`
        // in `DrawEditBottomTop` (`output2.cxx:3654`) against `aLogicStart.AdjustX(nEngineWidth)` in
        // `DrawEditTopBottom` (`:3902`).
        DocPoint anchor = quarter && cell.Format.RotationDegrees < 0
            ? new DocPoint(cell.Box.X + margin + Stack(placement), cell.Box.Y + margin)
            : new DocPoint(cell.Box.X + margin, cell.Box.Y + cell.Box.Height - margin);

        Length inner = cell.Box.Height - (2 * margin);

        sink.Save();
        try
        {
            sink.Transform(About(anchor, -cell.Format.RotationDegrees * Math.PI / 180.0));

            // The block is laid out unturned with its top-left corner on the anchor and the whole
            // of it is then turned about that corner — which is what `DrawText_ToPosition` does
            // with an EditEngine and an orientation. So a line's own place inside the block is an
            // offset from the anchor taken *before* the turn: an ascent down for the first line, a
            // line height further for each after it, and along the line whatever the cell's
            // vertical justification asked for. Drawing every line on the anchor instead put six
            // lines of a wrapped heading on one origin and every single-line turned cell one
            // ascent — 10.48 pt at eleven point — from where the reference puts it.
            //
            // An obliquely turned cell keeps the corner it had and takes no offset, because
            // neither is `DrawEditBottomTop`'s: `DrawRotated` centres its block across the column
            // and lifts the anchor by the block's own height times the cosine (`:5290-5330`), and
            // nothing in the corpus is turned by anything but a quarter to measure that against.
            Length down = Length.Zero;
            foreach (PlacedLine line in placement.Lines)
            {
                DocPoint origin = quarter
                    ? new DocPoint(
                        anchor.X + AlongOffset(cell.Format, inner, line.Run.Width),
                        anchor.Y + down + line.Run.Ascent)
                    : anchor;

                foreach ((GlyphRun run, Colour? colour) in line.Run.At(origin))
                {
                    if (run.Glyphs.Count == 0) continue;

                    sink.DrawGlyphRun(run, Paint.Solid(Ink(colour, fallback, FieldInk(cell))));
                }

                down += line.Run.LineHeight;
            }
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>How tall the block of lines is across the direction they read in.</summary>
    private static Length Stack(Placement placement)
    {
        Length stack = Length.Zero;
        foreach (PlacedLine line in placement.Lines) stack += line.Run.LineHeight;
        return stack;
    }

    /// <summary>
    /// Where one line sits along its own direction, which after a quarter turn is up or down the
    /// page.
    /// </summary>
    /// <remarks>
    /// A quarter-turned cell's <em>vertical</em> justification becomes the EditEngine's paragraph
    /// adjust — <c>setAlignmentToEngine</c> (<c>output2.cxx:2777-2800</c>) — and the two
    /// orientations map it opposite ways, because their lines read in opposite directions. Bottom
    /// is the default and is the whole of the corpus: it leaves an anticlockwise cell's text on the
    /// anchor and pushes a clockwise cell's to the far end of its line, which is what makes both of
    /// them finish at the cell's bottom. Measured on the probe: a 53-glyph clockwise line the
    /// reference ends 5 pt above the cell's bottom edge, and starting it at the anchor instead put
    /// it 87 pt low.
    /// </remarks>
    private static Length AlongOffset(SheetCellFormat format, Length inner, Length width)
    {
        Length gap = inner - width;
        bool clockwise = format.RotationDegrees < 0;

        return format.Vertical switch
        {
            SheetVerticalAlignment.Centre => gap / 2,
            SheetVerticalAlignment.Top => clockwise ? Length.Zero : gap,
            SheetVerticalAlignment.Bottom or SheetVerticalAlignment.Standard =>
                clockwise ? gap : Length.Zero,
            _ => Length.Zero,
        };
    }

    /// <summary>
    /// Whether the cell's <em>orientation</em> — rather than its angle — is not Calc's standard.
    /// </summary>
    /// <remarks>
    /// <c>ScPatternAttr::GetCellOrientation</c> (<c>patattr.cxx:529-547</c>) reads exactly 9000 and
    /// exactly 27000 as <c>BottomUp</c> and <c>TopBottom</c>, which Calc draws through
    /// <c>DrawEditBottomTop</c> and <c>DrawEditTopBottom</c>; every other angle stays
    /// <c>Standard</c> and reaches <c>DrawRotated</c> instead, whose paper and whose anchor are
    /// worked out differently.
    /// </remarks>
    private static bool IsQuarterTurned(SheetCellFormat format)
        => !format.IsStacked && Math.Abs(format.RotationDegrees) == 90;

    /// <summary>A rotation about a point rather than about the page's origin.</summary>
    private static AffineTransform About(DocPoint pivot, double radians)
        => AffineTransform.Concat(
            AffineTransform.Concat(
                AffineTransform.Translation(-pivot.X.Emu, -pivot.Y.Emu),
                AffineTransform.Rotation(radians)),
            AffineTransform.Translation(pivot.X.Emu, pivot.Y.Emu));

    /// <summary>Draws one character under the next, which needs no transform at all.</summary>
    private static void DrawStacked(
        IDrawingSink sink,
        in SheetTextContext context,
        in SheetCellText cell,
        Placement placement,
        Colour fallback)
    {
        if (SheetFonts.For(cell.Format) is not { } face) return;

        Length size = placement.Lines[0].Run.Size;
        Length pitch = face.LineHeightAt(size);
        Length y = cell.Box.Y + (cell.Format.Margin * context.Scale) + face.AscentAt(size);

        foreach (char character in cell.Text)
        {
            if (SheetText.Shape(character.ToString(), face, size) is not { } glyph) continue;

            Length x = cell.Box.X + ((cell.Box.Width - glyph.Width) / 2);
            foreach ((GlyphRun run, Colour? colour) in glyph.At(new DocPoint(x, y)))
                sink.DrawGlyphRun(run, Paint.Solid(Ink(colour, fallback, FieldInk(cell))));
            y += pitch;
        }
    }
}
