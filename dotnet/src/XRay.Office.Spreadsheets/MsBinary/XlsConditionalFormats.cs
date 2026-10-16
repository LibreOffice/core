using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using CellIs = XRay.Office.Spreadsheets.Layout.SheetConditions.CellIs;
using Comparison = XRay.Office.Spreadsheets.Layout.SheetConditions.Comparison;
using ICondition = XRay.Office.Spreadsheets.Layout.SheetConditions.ICondition;
using Operand = XRay.Office.Spreadsheets.Layout.SheetConditions.Operand;
using Sheet = XRay.Office.Spreadsheets.Layout.SheetConditions.Sheet;
using Value = XRay.Office.Spreadsheets.Layout.SheetConditions.Value;

namespace XRay.Office.Spreadsheets.MsBinary;

/// <summary>
/// The <c>CONDFMT</c> and <c>CF</c> records of one worksheet: which ranges a conditional format
/// covers, what each of its rules tests, and the fill and font the rule paints where it holds.
/// </summary>
/// <remarks>
/// <para>
/// <c>XclImpCondFormat</c> (<c>sc/source/filter/excel/xicontent.cxx</c>:504-745). The pair is one
/// header record and then one <c>CF</c> per rule: <c>ReadCondfmt</c> (<c>:516-524</c>) takes a
/// <c>ccf</c> count, ignores ten bytes and reads an <c>XclRangeList</c>, and <c>ReadCF</c>
/// (<c>:526-713</c>) takes a type byte, an operator byte, the byte lengths of two RPN formulas, a
/// flag word saying which format blocks follow, and then those blocks in a fixed order before
/// the formulas themselves.
/// </para>
/// <para>
/// <strong>Reading the header alone measures nothing.</strong> A round of this project read
/// <c>CONDFMT</c> — a count and a range list — concluded from 64 byte-identical renderings that
/// BIFF conditional formats have no reach, and had to withdraw it: the rules are in the <c>CF</c>
/// records, so a reader that stops at the header cannot move a pixel whatever the corpus holds.
/// Asked of 26.2.4.2 instead, <c>--convert-to fods</c> resolves <strong>149 conditional formats
/// and 187 conditions across 5 of the corpus's 64 <c>.xls</c></strong>, 175 of them naming a
/// style that carries a fill, a font colour or bold.
/// </para>
/// <para>
/// <strong>The condition itself is the same predicate the OOXML reader already evaluates</strong>
/// — both filters build an <c>ScCondFormatEntry</c> with an <c>ScConditionMode</c> — so only the
/// notation differs, and the evaluation is shared through <see cref="SheetConditions"/>. What is
/// genuinely BIFF's is the operator table (<c>EXC_CF_CMP_*</c>,
/// <c>sc/source/filter/inc/xlcontent.hxx</c>:61-69), the inline format blocks, and the RPN token
/// array a formula is stated as rather than text.
/// </para>
/// <para>
/// <strong>A rule's formula is anchored on the <em>first</em> range of the list, not on its
/// top-left corner.</strong> <c>ReadCF</c> takes <c>maRanges.front().aStart</c> (<c>:657</c>) and
/// hands it to every <c>ScCondFormatEntry</c> as the position the token array was written for.
/// That is a different rule from the OOXML path, where <c>ScRangeList::GetTopLeftCorner</c>
/// picks the smallest range start under <c>ScAddress</c>'s <c>(tab, col, row)</c> ordering, and
/// the corpus discriminates: <c>EHEST-Pre-departure-checklist</c> states blocks whose first range
/// is <c>E46:E57</c> and whose top-left corner is <c>E1</c>. 26.2.4.2's own
/// <c>calcext:base-cell-address</c> for that block is <c>E46</c>.
/// </para>
/// <para>
/// <strong><c>CF12</c> is not read, and neither does the reference read it.</strong> Excel 2007
/// writes the extended rule families — data bars, colour scales, icon sets — into a
/// <c>CF12</c> record (<c>0x087A</c>) beside the <c>CF</c> it also writes for compatibility.
/// <c>ImportExcel8::Read</c> has no case for it, so 26.2.4.2 sees only the <c>CF</c>; the 42
/// <c>CF12</c> records the corpus states are matched one for one by a <c>CF</c> that is read.
/// </para>
/// </remarks>
internal sealed class XlsConditionalFormats
{
    /// <summary>
    /// How many cell positions all of a sheet's rules together may be evaluated over.
    /// </summary>
    /// <remarks>
    /// The same budget the OOXML reader carries, and for the same reason: a BIFF range list can
    /// name a whole column, and the reference's own duplicate-condition cache clamps its walk to
    /// the used area for exactly this cost (<c>conditio.cxx</c>:820-828).
    /// </remarks>
    private const long PositionBudget = 8_000_000;

    private readonly List<Block> _blocks = [];

    /// <summary>True when the sheet stated no conditional format at all.</summary>
    public bool IsEmpty => _blocks.Count == 0;

    /// <summary>Reads a <c>CONDFMT</c> record, which opens a block of rules.</summary>
    /// <remarks>
    /// <c>XclImpCondFormat::ReadCondfmt</c>. The ten ignored bytes are the "needs recalc" flag
    /// and the bounding box of the range list, both of which the list itself restates.
    /// </remarks>
    /// <param name="stream">The record stream, positioned after the record header.</param>
    public void ReadCondfmt(BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        if (stream.RecordLeft < 12) return;

        int count = stream.ReadUInt16();
        stream.Skip(10);

        int ranges = stream.RecordLeft >= 2 ? stream.ReadUInt16() : 0;
        Block block = new(count);

        for (int at = 0; at < ranges && stream.RecordLeft >= 8; at++)
        {
            int firstRow = stream.ReadUInt16();
            int lastRow = stream.ReadUInt16();
            int firstColumn = stream.ReadUInt16();
            int lastColumn = stream.ReadUInt16();

            if (lastRow < firstRow || lastColumn < firstColumn) continue;
            block.Ranges.Add(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
        }

        _blocks.Add(block);
    }

    /// <summary>Reads one <c>CF</c> record into the block the last <c>CONDFMT</c> opened.</summary>
    /// <param name="stream">The record stream, positioned after the record header.</param>
    /// <param name="palette">The workbook's colours, for the font and pattern blocks.</param>
    public void ReadCf(BiffRecordReader stream, XlsCellFormats palette)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(palette);

        // `XclImpCondFormatManager::ReadCF` hands the record to the *last* format read, and
        // `XclImpCondFormat::ReadCF` refuses it once that format's own `ccf` is exhausted or its
        // range list came back empty.
        if (_blocks.Count == 0) return;

        Block block = _blocks[^1];
        if (block.Rules.Count >= block.Count || block.Ranges.Count == 0) return;
        if (stream.RecordLeft < 12) return;

        int type = stream.ReadByte();
        int comparison = stream.ReadByte();
        int firstFormulaSize = stream.ReadUInt16();
        int secondFormulaSize = stream.ReadUInt16();
        uint flags = stream.ReadUInt32();
        stream.Skip(2);

        string? mode = type switch
        {
            CfTypeCell => ComparisonOf(comparison),
            CfTypeFormula => DirectMode,
            _ => null,
        };

        // The reference treats the two refusals differently and so does this. An unknown *type*
        // is a bare `return` before `++mnCondIndex` (`xicontent.cxx`:573-575), so the block's
        // condition index does not advance and a following `CF` takes this one's place. An
        // unknown *comparison* is not a refusal at all: `ReadCF` logs it, leaves `eMode` at
        // `ScConditionMode::NONE` and builds an entry that `IsValid`'s `default:` arm can never
        // satisfy — one rule's worth of the block consumed and nothing painted. No corpus record
        // reaches either arm: every `CF` states type 1 or 2 and comparison 1, 3, 5 or 6.
        if (type is not (CfTypeCell or CfTypeFormula)) return;

        if (mode is null)
        {
            block.Rules.Add(null);
            return;
        }

        SheetConditionalText text = default;
        Colour? background = null;

        if ((flags & BlockNumberFormat) != 0) SkipNumberFormat(stream, (flags & NumberFormatIsUser) != 0);
        if ((flags & BlockFont) != 0) text = ReadFontBlock(stream, palette);
        if ((flags & BlockAlignment) != 0) stream.Skip(8);
        if ((flags & BlockBorder) != 0) stream.Skip(8);
        if ((flags & BlockArea) != 0) background = ReadAreaBlock(stream, palette, flags);
        if ((flags & BlockProtection) != 0) stream.Skip(2);

        byte[] first = stream.ReadBytes(Math.Min(firstFormulaSize, Math.Max(0, stream.RecordLeft)));
        byte[] second = stream.ReadBytes(Math.Min(secondFormulaSize, Math.Max(0, stream.RecordLeft)));

        // Counted before the condition is built, because the reference counts it there too: a
        // formula it cannot convert still consumes the block's `ccf`.
        block.Rules.Add(null);

        if (text.IsNone && background is null) return;

        // The anchor is the start of the *first* range, which is what `ReadCF` hands every entry
        // as `rPos` (`maRanges.front().aStart`, `xicontent.cxx`:657).
        SheetRange anchor = block.Ranges[0];
        if (ConditionOf(mode, first, second, anchor.FirstRow, anchor.FirstColumn) is not { } test)
            return;

        block.Rules[^1] = new Rule(test, text, background);
    }

    /// <summary>
    /// Applies every rule to the cells the sheet holds.
    /// </summary>
    /// <param name="formatting">The sheet's decoration, which takes the conditional fills.</param>
    /// <param name="cells">The sheet's values, for the rules to be asked against.</param>
    /// <returns>What each matching cell's winning rule changes about its text.</returns>
    public Dictionary<(int Row, int Column), SheetConditionalText> Apply(
        SheetFormatting formatting, Sheet cells)
    {
        ArgumentNullException.ThrowIfNull(formatting);
        ArgumentNullException.ThrowIfNull(cells);

        Dictionary<(int Row, int Column), SheetConditionalText> text = [];
        if (cells.LastRow < 0) return text;

        HashSet<(int Row, int Column)> decided = [];
        long budget = PositionBudget;

        // Blocks in the order the sheet states them and rules in the order the block states them,
        // which is the reference's: `AddCondFormat` hands out an index per format as it is
        // applied and `ScDocument::GetCondResult` walks a cell's indices in that order
        // (`documen4.cxx`:1119-1143), while a BIFF block's entries go into one
        // `ScConditionalFormat` in `CF` order with no priority attribute to reorder them.
        foreach (Block block in _blocks)
        {
            // A `CONDFMT` whose range list came back empty keeps its place in the list — its
            // `CF` records were refused rather than reassigned, exactly as `ReadCF` refuses them
            // — and has no anchor to read.
            if (block.Ranges.Count == 0) continue;

            (int anchorRow, int anchorColumn) = (block.Ranges[0].FirstRow, block.Ranges[0].FirstColumn);

            foreach (Rule? rule in block.Rules)
            {
                if (rule is null) continue;

                foreach (SheetRange range in block.Ranges)
                {
                    int firstRow = Math.Max(0, range.FirstRow);
                    int lastRow = Math.Min(cells.LastRow, range.LastRow);
                    int firstColumn = Math.Max(0, range.FirstColumn);
                    int lastColumn = Math.Min(cells.LastColumn, range.LastColumn);
                    if (lastRow < firstRow || lastColumn < firstColumn) continue;

                    long span = (long)(lastRow - firstRow + 1) * (lastColumn - firstColumn + 1);
                    if (span > budget) return text;
                    budget -= span;

                    for (int row = firstRow; row <= lastRow; row++)
                    {
                        for (int column = firstColumn; column <= lastColumn; column++)
                        {
                            if (!decided.Add((row, column))) continue;
                            if (!rule.Test.Holds(cells, row, column, anchorRow, anchorColumn))
                            {
                                decided.Remove((row, column));
                                continue;
                            }

                            if (rule.Background is { } fill)
                                formatting.SetConditionalBackground(row, column, fill);
                            if (!rule.Text.IsNone) text[(row, column)] = rule.Text;
                        }
                    }
                }
            }
        }

        return text;
    }

    /// <summary>One <c>CONDFMT</c> and the <c>CF</c> records that follow it.</summary>
    /// <remarks>
    /// A rule the reader could not turn into a condition is kept as a null entry rather than
    /// dropped, because the block's <c>ccf</c> counts it and a later <c>CF</c> must not take its
    /// place.
    /// </remarks>
    private sealed class Block(int count)
    {
        public int Count { get; } = count;

        public List<SheetRange> Ranges { get; } = [];

        public List<Rule?> Rules { get; } = [];
    }

    /// <summary>One <c>CF</c>: what it tests and what it paints where the test holds.</summary>
    private sealed record Rule(ICondition Test, SheetConditionalText Text, Colour? Background);

    /// <summary>The <c>CF</c> type byte for a rule that compares the cell being formatted.</summary>
    /// <remarks><c>EXC_CF_TYPE_CELL</c>, <c>sc/source/filter/inc/xlcontent.hxx</c>:58.</remarks>
    private const int CfTypeCell = 0x01;

    /// <summary>The type byte for a rule whose formula is itself the condition.</summary>
    /// <remarks><c>EXC_CF_TYPE_FMLA</c>, the same header, <c>:59</c>.</remarks>
    private const int CfTypeFormula = 0x02;

    /// <summary>The marker this reader uses for <c>ScConditionMode::Direct</c>.</summary>
    private const string DirectMode = "direct";

    private const uint BlockNumberFormat = 0x02000000;
    private const uint BlockFont = 0x04000000;
    private const uint BlockAlignment = 0x08000000;
    private const uint BlockBorder = 0x10000000;
    private const uint BlockArea = 0x20000000;
    private const uint BlockProtection = 0x40000000;
    private const uint NumberFormatIsUser = 0x1;

    /// <summary>Whether a pattern, a foreground or a background colour is <em>not</em> stated.</summary>
    /// <remarks>
    /// The sense is inverted in the file and <c>XclImpCellArea::FillFromCF8</c>
    /// (<c>xistyle.cxx</c>:1073-1091) reads it that way — <c>mbForeUsed = !get_flag(nFlags,
    /// EXC_CF_AREA_FGCOLOR)</c>. A set bit means the rule leaves that part of the cell alone.
    /// </remarks>
    private const uint AreaPatternUnused = 0x00010000;

    /// <inheritdoc cref="AreaPatternUnused"/>
    private const uint AreaForegroundUnused = 0x00020000;

    /// <inheritdoc cref="AreaPatternUnused"/>
    private const uint AreaBackgroundUnused = 0x00040000;

    /// <summary>System window text, <c>EXC_COLOR_WINDOWTEXT</c> — an unstated foreground.</summary>
    private const int WindowTextColour = 64;

    /// <summary>System window background, <c>EXC_COLOR_WINDOWBACK</c> — an unstated background.</summary>
    private const int WindowBackColour = 65;

    /// <summary>
    /// The <c>ScConditionMode</c> a <c>CF</c> comparison byte names, in this reader's spelling.
    /// </summary>
    /// <remarks>
    /// <c>ReadCF</c>'s <c>EXC_CF_TYPE_CELL</c> arm (<c>xicontent.cxx</c>:552-575). The names are
    /// the OOXML <c>operator</c> attribute's, which is what <see cref="CellIs"/> parses, and the
    /// two tables meet at the same <c>ScConditionMode</c> value: <c>EXC_CF_CMP_GREATER_EQUAL</c>
    /// and <c>cfRule operator="greaterThanOrEqual"</c> are both <c>EqGreater</c>.
    /// </remarks>
    private static string? ComparisonOf(int stated) => stated switch
    {
        0x01 => "between",
        0x02 => "notBetween",
        0x03 => "equal",
        0x04 => "notEqual",
        0x05 => "greaterThan",
        0x06 => "lessThan",
        0x07 => "greaterThanOrEqual",
        0x08 => "lessThanOrEqual",
        _ => null,
    };

    /// <summary>The condition one <c>CF</c> states, or null when its formulas cannot be read.</summary>
    private static ICondition? ConditionOf(
        string mode, byte[] first, byte[] second, int anchorRow, int anchorColumn)
    {
        if (mode == DirectMode) return XlsConditionFormula.Parse(first, anchorRow, anchorColumn);

        bool two = mode is "between" or "notBetween";
        if (XlsConditionFormula.Operand(first, anchorRow, anchorColumn) is not { } left) return null;
        if (!two) return new CellIs(mode, left, null);

        return XlsConditionFormula.Operand(second, anchorRow, anchorColumn) is { } right
            ? new CellIs(mode, left, right)
            : null;
    }

    /// <summary>
    /// Steps over the <c>CF</c> number-format block, which states no ink.
    /// </summary>
    /// <remarks>
    /// <c>XclImpNumFmtBuffer::ReadCFFormat</c> (<c>xistyle.cxx</c>): a user-defined format is a
    /// <c>ShortXLUnicodeString</c> followed by one reserved byte, and a built-in one is a
    /// two-byte index. Skipped rather than read because a conditional number format changes what
    /// a cell says and this reader is the fill-and-font half; no corpus <c>CF</c> states one.
    /// </remarks>
    private static void SkipNumberFormat(BiffRecordReader stream, bool userDefined)
    {
        if (!userDefined)
        {
            stream.Skip(Math.Min(2, stream.RecordLeft));
            return;
        }

        if (stream.RecordLeft < 2) return;

        int characters = stream.ReadByte();
        byte flags = stream.ReadByte();
        stream.ReadRawUnicodeString(characters, (flags & 0x01) != 0);
        stream.Skip(Math.Min(1, stream.RecordLeft));
    }

    /// <summary>
    /// The <c>CF</c> font block: 118 bytes of which six fields are differential.
    /// </summary>
    /// <remarks>
    /// <c>XclImpFont::ReadCFFontBlock</c> (<c>xistyle.cxx</c>). Each property is "used" only when
    /// its own sentinel says so, and the sentinels are not one scheme: the height and the colour
    /// are unused when they exceed <c>0x7FFF</c>, the weight and the italic flag share
    /// <c>EXC_CF_FONT_STYLE</c> in the first flag dword, the strikeout has its own bit in the
    /// same dword, and the underline is governed by a bit in the <em>third</em> dword.
    /// </remarks>
    private static SheetConditionalText ReadFontBlock(BiffRecordReader stream, XlsCellFormats palette)
    {
        const int BlockSize = 118;
        if (stream.RecordLeft < BlockSize)
        {
            stream.Skip(stream.RecordLeft);
            return default;
        }

        stream.Skip(64);
        uint height = stream.ReadUInt32();
        uint style = stream.ReadUInt32();
        int weight = stream.ReadUInt16();
        stream.Skip(2);
        int underline = stream.ReadByte();
        stream.Skip(3);
        uint colour = stream.ReadUInt32();
        stream.Skip(4);
        uint fontFlags1 = stream.ReadUInt32();
        stream.Skip(4);
        uint fontFlags3 = stream.ReadUInt32();
        stream.Skip(18);

        bool styleUsed = (fontFlags1 & FontStyleUnused) == 0;

        return new SheetConditionalText
        {
            FontSize = height <= 0x7FFF ? Length.FromTwips((int)height) : null,
            FontWeight = styleUsed && weight < 0x7FFF ? weight : null,
            IsItalic = styleUsed ? (style & FontStyleUnused) != 0 : null,
            IsStruckThrough = (fontFlags1 & FontStrikeoutUnused) == 0
                ? (style & FontStrikeoutUnused) != 0
                : null,
            Underline = (fontFlags3 & FontUnderlineUnused) == 0 && underline <= 0x7F
                ? UnderlineOf(underline)
                : null,
            Colour = colour <= 0x7FFF ? palette.SchemeColour((int)colour) : null,
        };
    }

    /// <summary>The bit that turns the weight and the posture off, and marks italic when set.</summary>
    /// <remarks>
    /// <c>EXC_CF_FONT_STYLE</c> (<c>xlcontent.hxx</c>:89) is read from two different words and
    /// means opposite things in them: in <c>nFontFlags1</c> it says the rule does not change the
    /// weight or the posture, and in <c>nStyle</c> it is the italic flag itself.
    /// </remarks>
    private const uint FontStyleUnused = 0x00000002;

    /// <inheritdoc cref="FontStyleUnused"/>
    private const uint FontStrikeoutUnused = 0x00000080;

    /// <summary>The bit in the third flag word that turns the underline off.</summary>
    /// <remarks><c>EXC_CF_FONT_UNDERL</c>, <c>xlcontent.hxx</c>:93.</remarks>
    private const uint FontUnderlineUnused = 0x00000001;

    /// <summary>BIFF's underline codes, folded onto the two lines this project draws.</summary>
    private static SheetUnderline UnderlineOf(int stated) => stated switch
    {
        0x01 or 0x21 => SheetUnderline.SingleLine,
        0x02 or 0x22 => SheetUnderline.DoubleLine,
        _ => SheetUnderline.None,
    };

    /// <summary>
    /// The <c>CF</c> pattern block: what the rule fills the cell with, or null for nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>XclImpCellArea::FillFromCF8</c> (<c>xistyle.cxx</c>:1073-1091). The two colour indices
    /// are seven bits each inside one word and the pattern is six bits at the top of the other,
    /// which is a different packing from the <c>XF</c> record's.
    /// </para>
    /// <para>
    /// <strong>A stated background with no pattern becomes a solid fill of that colour.</strong>
    /// That is the first of <c>FillFromCF8</c>'s two corrections and it is what the corpus's
    /// rules rely on: 26.2.4.2 writes them out as <c>fo:background-color</c>. The second, which
    /// clears a solid pattern whose background is unstated, leaves the foreground painting —
    /// the same answer this returns.
    /// </para>
    /// </remarks>
    private static Colour? ReadAreaBlock(BiffRecordReader stream, XlsCellFormats palette, uint flags)
    {
        if (stream.RecordLeft < 4)
        {
            stream.Skip(stream.RecordLeft);
            return null;
        }

        int packedPattern = stream.ReadUInt16();
        int packedColour = stream.ReadUInt16();

        int foreground = packedColour & 0x7F;
        int background = (packedColour >> 7) & 0x7F;
        int pattern = (packedPattern >> 10) & 0x3F;

        bool foregroundUsed = (flags & AreaForegroundUnused) == 0;
        bool backgroundUsed = (flags & AreaBackgroundUnused) == 0;
        bool patternUsed = (flags & AreaPatternUnused) == 0;

        if (backgroundUsed && (!patternUsed || pattern == XlsPatternFill.Solid))
            return palette.SchemeColour(background);

        if (!patternUsed || pattern == XlsPatternFill.None) return null;

        // A hatch is the two colours mixed at the weight the pattern number carries, not either
        // of them whole — see `XlsPatternFill`, and note that a `CF` record is where the two
        // colours really can be stated separately, so the window-colour substitutions below are
        // the reference's own (`XclImpCellArea::FillToItemSet`, `xistyle.cxx`:1105-1111) rather
        // than a guard against a bad index.
        return XlsPatternFill.Resolve(
            pattern,
            palette.SchemeColour(foregroundUsed ? foreground : WindowTextColour) ?? Colour.Black,
            palette.SchemeColour(backgroundUsed ? background : WindowBackColour) ?? Colour.White);
    }
}

/// <summary>
/// The RPN token array a <c>CF</c> record states its formulas as.
/// </summary>
/// <remarks>
/// <para>
/// Only what a conditional format can hold, which is a much smaller language than a cell
/// formula's: a literal, a reference, one binary comparison between two of those, and
/// <c>NOT</c>. Anything else answers null and the rule is dropped, which is the same outcome as
/// the reference's own refusal to convert an external formula (<c>xicontent.cxx</c>:667-671) —
/// no format is applied. Guessing at a formula would attach a plausible wrong fill to real
/// cells, which is worse than leaving them alone.
/// </para>
/// <para>
/// Censused over the corpus's five <c>.xls</c> that state a <c>CF</c>: <strong>187 formulas, of
/// which 187 are inside this grammar</strong> — 138 a bare <c>tInt</c> or <c>tStr</c> operand,
/// 20 a reference against a reference, 13 a <c>NOT</c> of a reference, and the rest bare
/// references and one-sided string comparisons.
/// </para>
/// <para>
/// <strong>A reference is stored two ways and only one of them is relative.</strong> <c>tRef</c>
/// (<c>0x24</c>) holds an absolute row and column with the two relative bits in the top of the
/// column word; <c>tRefN</c> (<c>0x2C</c>) holds <em>offsets</em> from the cell the formula was
/// written for, the row as a signed 16-bit and the column as a signed 8-bit. Both appear in the
/// corpus in the same document. Reading a <c>tRefN</c> as a <c>tRef</c> turns
/// <c>NOT([.J100])</c> on <c>J10:J59</c> into <c>NOT([.J91])</c>, which is nine rows of the wrong
/// answer and no error anywhere.
/// </para>
/// </remarks>
internal static class XlsConditionFormula
{
    /// <summary>
    /// The whole formula as a condition, for a rule whose formula <em>is</em> the test.
    /// </summary>
    /// <param name="tokens">The record's RPN bytes.</param>
    /// <param name="anchorRow">The row the formula was written for.</param>
    /// <param name="anchorColumn">The column it was written for.</param>
    public static ICondition? Parse(byte[] tokens, int anchorRow, int anchorColumn)
    {
        List<Node> stack = [];
        if (!Walk(tokens, stack, anchorRow, anchorColumn) || stack.Count != 1) return null;

        return stack[0] switch
        {
            Node.Compare compare => new Comparison(compare.Left, compare.Operator, compare.Right),

            // A formula that is a bare value is `ScConditionMode::Direct` on that value, which
            // holds when it is neither zero nor blank. `A1` on its own and `NOT(A1)` are both
            // ordinary in the corpus, and the reference tests them by evaluating the token array
            // and asking `!= 0`.
            Node.Term term => new Truth(term.Operand, term.Negated),
            _ => null,
        };
    }

    /// <summary>
    /// The formula as one operand, for a <c>cellIs</c> rule's comparison value.
    /// </summary>
    /// <param name="tokens">The record's RPN bytes.</param>
    /// <param name="anchorRow">The row the formula was written for.</param>
    /// <param name="anchorColumn">The column it was written for.</param>
    public static Operand? Operand(byte[] tokens, int anchorRow, int anchorColumn)
    {
        List<Node> stack = [];
        if (!Walk(tokens, stack, anchorRow, anchorColumn) || stack.Count != 1) return null;
        return stack[0] is Node.Term { Negated: false } term ? term.Operand : null;
    }

    /// <summary>One entry of the evaluation stack: a value, or a comparison of two.</summary>
    private abstract record Node
    {
        public sealed record Term(Operand Operand, bool Negated) : Node;

        public sealed record Compare(Operand Left, string Operator, Operand Right) : Node;
    }

    /// <summary>
    /// A <c>Direct</c> rule, which holds where its formula is neither zero nor blank.
    /// </summary>
    /// <remarks>
    /// <c>ScConditionEntry::IsValid</c>'s <c>Direct</c> arm answers <c>nVal != 0</c> for a
    /// number and false for an empty cell, and a string operand never satisfies it — which is why
    /// <c>Background_Declaration_Template</c>'s two <c>formula-is("")</c> and
    /// <c>formula-is("&gt;0")</c> rules, whose whole formula is a string literal, paint nothing
    /// at 26.2.4.2 however their cells are filled.
    /// </remarks>
    private sealed record Truth(Operand Operand, bool Negated) : ICondition
    {
        /// <inheritdoc/>
        public bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn)
        {
            Value value = Operand.Resolve(sheet, row - anchorRow, column - anchorColumn);
            bool holds = value.Number is { } number && number != 0;
            return Negated ? !holds : holds;
        }
    }

    /// <summary>Walks the token array, or answers false at the first token outside the grammar.</summary>
    private static bool Walk(byte[] tokens, List<Node> stack, int anchorRow, int anchorColumn)
    {
        int at = 0;

        while (at < tokens.Length)
        {
            byte opcode = tokens[at++];

            // The reference, value and array classes of one operand token differ only in bits 5
            // and 6 of the opcode and carry the same payload, so they are folded before the
            // layout is chosen. The base identifier is the low five bits with bit 5 put back —
            // `opcode & 0x3F` is *not* the fold, because it turns the value class of `tRef`
            // (0x44) into 0x04 rather than 0x24 and every value-class reference then falls
            // through to the refusal below.
            byte kind = (byte)(opcode >= 0x20 ? (opcode & 0x1F) | 0x20 : opcode);

            switch (kind)
            {
                case 0x1E: // tInt
                    if (!Take(tokens, ref at, 2)) return false;
                    stack.Add(Literal(Value.OfNumber(Word(tokens, at - 2))));
                    break;

                case 0x1F: // tNum
                    if (!Take(tokens, ref at, 8)) return false;
                    stack.Add(Literal(Value.OfNumber(
                        BitConverter.ToDouble(tokens, at - 8))));
                    break;

                case 0x1D: // tBool
                    if (!Take(tokens, ref at, 1)) return false;
                    stack.Add(Literal(Value.OfNumber(tokens[at - 1] == 0 ? 0 : 1)));
                    break;

                case 0x17: // tStr, a ShortXLUnicodeString in BIFF8
                {
                    if (!Take(tokens, ref at, 2)) return false;
                    int characters = tokens[at - 2];
                    bool wide = (tokens[at - 1] & 0x01) != 0;
                    int bytes = wide ? characters * 2 : characters;
                    if (!Take(tokens, ref at, bytes)) return false;
                    stack.Add(Literal(Value.OfText(Text(tokens, at - bytes, characters, wide))));
                    break;
                }

                case 0x24: // tRef: the address itself, with two relative bits
                {
                    if (!Take(tokens, ref at, 4)) return false;
                    int row = Word(tokens, at - 4);
                    int packed = Word(tokens, at - 2);
                    stack.Add(new Node.Term(
                        new Operand(true, row, packed & 0x3FFF,
                                    (packed & RelativeRow) == 0, (packed & RelativeColumn) == 0,
                                    Value.Blank),
                        false));
                    break;
                }

                case 0x2C: // tRefN: offsets from the cell the formula was written for
                {
                    if (!Take(tokens, ref at, 4)) return false;
                    int row = (short)Word(tokens, at - 4);
                    int packed = Word(tokens, at - 2);

                    // The column offset is a *signed byte*, not the fourteen bits the address
                    // form uses. `0xFE` here is −2 and not 254: `([.M10]<[.K10])` on
                    // `Background_Declaration_Template`'s `IPR` sheet is stored as two `tRefN`
                    // with column offsets 0 and 0xFE, and reading the wide field would send the
                    // second reference 254 columns to the right of the first.
                    int column = (sbyte)(byte)(packed & 0xFF);

                    bool relativeRow = (packed & RelativeRow) != 0;
                    bool relativeColumn = (packed & RelativeColumn) != 0;

                    // A relative half is kept as the address it has at the anchor, because that
                    // is the shape `SheetConditions.Operand` resolves: it shifts by the distance
                    // from the anchor to the cell being tested, so anchor + offset + shift is the
                    // cell + offset the reference means. An absolute half is already an address.
                    stack.Add(new Node.Term(
                        new Operand(
                            true,
                            relativeRow ? anchorRow + row : row,
                            relativeColumn ? anchorColumn + column : column,
                            !relativeRow,
                            !relativeColumn,
                            Value.Blank),
                        false));
                    break;
                }

                case 0x09 or 0x0A or 0x0B or 0x0C or 0x0D or 0x0E: // tLT tLE tEQ tGE tGT tNE
                {
                    if (stack.Count < 2) return false;
                    if (stack[^2] is not Node.Term left || stack[^1] is not Node.Term right) return false;
                    if (left.Negated || right.Negated) return false;

                    stack.RemoveRange(stack.Count - 2, 2);
                    stack.Add(new Node.Compare(left.Operand, OperatorOf(kind), right.Operand));
                    break;
                }

                case 0x21: // tFunc, which is only ever NOT here
                {
                    if (!Take(tokens, ref at, 2)) return false;
                    if (Word(tokens, at - 2) != NotFunction) return false;
                    if (stack.Count < 1 || stack[^1] is not Node.Term term || term.Negated) return false;

                    stack[^1] = term with { Negated = true };
                    break;
                }

                case 0x15: // tParen, which changes nothing about the value
                    break;

                case 0x19: // tAttr, a hint whose payload is three bytes
                    if (!Take(tokens, ref at, 3)) return false;
                    break;

                default:
                    return false;
            }
        }

        return true;
    }

    /// <summary>The BIFF function index of <c>NOT</c>.</summary>
    /// <remarks>
    /// <c>{ ocNot, 38, 1, 1, V, { VR }, 0, nullptr }</c> in the function table
    /// (<c>sc/source/filter/excel/xlformula.cxx</c>). Thirteen corpus rules are a <c>NOT</c> of
    /// one reference and no other function appears in a corpus <c>CF</c> formula at all.
    /// </remarks>
    private const int NotFunction = 38;

    /// <summary>The bit in a reference token's column word that makes the row relative.</summary>
    private const int RelativeRow = 0x8000;

    /// <inheritdoc cref="RelativeRow"/>
    private const int RelativeColumn = 0x4000;

    private static Node.Term Literal(Value value)
        => new Node.Term(new Operand(false, 0, 0, false, false, value), false);

    private static string OperatorOf(int kind) => kind switch
    {
        0x09 => "<",
        0x0A => "<=",
        0x0B => "=",
        0x0C => ">=",
        0x0D => ">",
        _ => "<>",
    };

    private static int Word(byte[] tokens, int at) => tokens[at] | (tokens[at + 1] << 8);

    private static string Text(byte[] tokens, int at, int characters, bool wide)
    {
        char[] text = new char[characters];
        for (int i = 0; i < characters; i++)
        {
            text[i] = wide
                ? (char)(tokens[at + (i * 2)] | (tokens[at + (i * 2) + 1] << 8))
                : (char)tokens[at + i];
        }

        return new string(text);
    }

    private static bool Take(byte[] tokens, ref int at, int count)
    {
        if (count < 0 || at + count > tokens.Length) return false;
        at += count;
        return true;
    }
}
