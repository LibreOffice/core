using System.Text;
using Paperless.Core.Extraction;
using Paperless.Core.Globalization;
using Paperless.Core.Numbering;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Ww8;

/// <content>The character walk that turns positions into paragraphs, runs, cells and fields.</content>
public sealed partial class Ww8DocumentReader
{
    /// <summary>The special characters WW8 uses instead of markup.</summary>
    private static class Special
    {
        /// <summary>A picture or embedded object placeholder.</summary>
        public const char Picture = '\u0001';

        /// <summary>An auto-numbered footnote, endnote or comment reference mark.</summary>
        public const char AutoNumberedReference = '\u0002';

        /// <summary>A comment reference.</summary>
        public const char AnnotationReference = '\u0005';

        /// <summary>A drawn object.</summary>
        public const char DrawnObject = '\u0008';

        /// <summary>A tab.</summary>
        public const char Tab = '\t';

        /// <summary>A line break within a paragraph.</summary>
        public const char LineBreak = '\u000B';

        /// <summary>A section break, which also ends the paragraph.</summary>
        public const char SectionMark = '\u000C';

        /// <summary>A field's beginning: what follows is its instruction.</summary>
        public const char FieldBegin = '\u0013';

        /// <summary>A field's separator: what follows is its cached result.</summary>
        public const char FieldSeparator = '\u0014';

        /// <summary>A field's end.</summary>
        public const char FieldEnd = '\u0015';

        /// <summary>A non-breaking hyphen.</summary>
        public const char NonBreakingHyphen = '\u001E';

        /// <summary>An optional hyphen, drawn only where a line breaks.</summary>
        public const char OptionalHyphen = '\u001F';
    }

    /// <summary>
    /// Reads a range of character positions into a section.
    /// </summary>
    /// <remarks>
    /// One pass produces text, runs, paragraphs and table structure together, because in WW8 they
    /// are all decided by the same characters. Splitting it into separate passes would mean
    /// resolving each position's formatting more than once.
    /// </remarks>
    private void ReadRange(Ww8Range range, ContentNode target)
    {
        if (range.Length <= 0) return;

        string text = _pieces.ReadText(range.Start, range.End, _diagnostics);
        if (text.Length == 0) return;

        // Each flow numbers its own lists: a list in a footnote does not continue the body's count.
        _numbering.ResetCounters();

        WalkState state = new(target);

        for (int index = 0; index < text.Length; index++)
        {
            int position = range.Start + index;
            char character = text[index];

            switch (character)
            {
                case ParagraphMark:
                    EndParagraph(state, position);
                    continue;

                case CellMark:
                    EndCellOrRow(state, position);
                    continue;

                case Special.SectionMark:
                    EndParagraph(state, position);
                    continue;

                case Special.LineBreak:
                    Append(state, position, "\n");
                    continue;

                case Special.Tab:
                    Append(state, position, "\t");
                    continue;

                case Special.NonBreakingHyphen:
                    Append(state, position, "\u2011");
                    continue;

                case Special.OptionalHyphen:
                    continue;

                case Special.FieldBegin:
                    state.FieldDepth++;
                    state.InFieldInstruction = true;
                    state.Instruction.Clear();
                    // A hyperlink around a nested field keeps its target for the inner field's
                    // result too, so the outer target is stacked rather than overwritten.
                    state.Hyperlinks.Push(state.CurrentHyperlink);
                    continue;

                case Special.FieldSeparator:
                    state.InFieldInstruction = false;
                    // The instruction is the only place a hyperlink's target appears: the cached
                    // result is the text a reader saw, and says nothing about where it points.
                    if (FieldInstructions.HyperlinkTarget(state.Instruction.ToString()) is { } link)
                    {
                        FlushRun(state);
                        state.CurrentHyperlink = link;
                    }
                    continue;

                case Special.FieldEnd:
                    if (state.FieldDepth > 0) state.FieldDepth--;
                    state.InFieldInstruction = false;
                    FlushRun(state);
                    state.CurrentHyperlink = state.Hyperlinks.Count > 0 ? state.Hyperlinks.Pop() : null;
                    continue;

                case Special.AutoNumberedReference:
                    // The footnote's number, which the file does not store: Word computes it, and so
                    // does this. The note's own text is a separate subdocument.
                    if (target is ContentSection { Kind: SectionKind.Note })
                    {
                        // Inside a note this is the note's own mark, and the section already carries
                        // the number.
                        continue;
                    }
                    Append(state, position, OutlineNumbers.Digits(_footnoteNumber + 1));
                    _footnoteNumber++;
                    continue;

                case Special.AnnotationReference:
                    // The comment itself is read from its own subdocument.
                    continue;

                case Special.Picture:
                    // Only outside a field. A SHAPE or INCLUDEPICTURE field's cached result uses this
                    // same character for its anchor, so counting every one of them reports a picture
                    // for every shape in the document — and the shape's own text has already arrived
                    // as a frame section.
                    if (state.InFieldInstruction || state.FieldDepth > 0) continue;
                    FlushRun(state);
                    state.PendingImages.Add(new ContentImage());
                    continue;

                case Special.DrawnObject:
                    // A drawing anchor, not a picture. Telling an embedded image from a shape needs
                    // the Escher record stream, which extraction does not read.
                    continue;

                default:
                    // Everything below space that is not handled above is a control character WW8
                    // uses for bookkeeping, not text.
                    if (character < ' ' && character != '\n') continue;
                    Append(state, position, character.ToString());
                    continue;
            }
        }

        // A range need not end with a paragraph mark, or with the tables in it closed.
        FinishParagraph(state, force: false, state.ParagraphFormat.Level);
        CloseTablesDeeperThan(state, 0);
    }

    private void Append(WalkState state, int position, string text)
    {
        // A field's instruction is its code, not its result: emitting it puts PAGE and HYPERLINK
        // into the document's text. It is still collected, because a hyperlink's target is in it.
        if (state.InFieldInstruction)
        {
            state.Instruction.Append(text);
            return;
        }

        Ww8CharacterFormat format = ResolveCharacterFormat(position, state);
        if (format.IsHidden || format.IsDeleted) return;

        // The link comes from the field the walk is inside rather than from any sprm, so it is
        // applied after the exception is resolved — and included in the format, so a run ends where
        // the link does.
        format = format with { HyperlinkTarget = state.CurrentHyperlink };

        if (state.HasFormat && state.Format != format) FlushRun(state);

        state.HasFormat = true;
        state.Format = format;
        state.Text.Append(text);
    }

    private static void FlushRun(WalkState state)
    {
        if (state.Text.Length > 0)
        {
            state.Runs.Add(new ContentRun
            {
                Text = state.Text.ToString(),
                StyleName = state.Format.CharacterStyleName,
                Language = state.Format.Language,
                Emphasis = state.Format.Emphasis,
                HyperlinkTarget = state.Format.HyperlinkTarget,
            });
        }
        state.Text.Clear();
        state.HasFormat = false;
    }

    /// <summary>
    /// Ends a paragraph at its mark, whose position is where its properties live.
    /// </summary>
    /// <remarks>
    /// A paragraph mark inside a nested table is also that table's cell or row end. Only the
    /// outermost table uses U+0007 for its cells; a nested one reuses the paragraph mark and says
    /// what it means with <c>sprmPFInnerTableCell</c> and <c>sprmPFInnerTtp</c> — so a reader that
    /// treats every carriage return as merely a paragraph puts a nested table's whole contents into
    /// one cell.
    /// </remarks>
    private void EndParagraph(WalkState state, int markPosition)
    {
        Ww8ParagraphFormat format = ResolveParagraphFormat(markPosition);
        state.ParagraphFormat = format;

        if (!format.IsInnerTableCell)
        {
            FinishParagraph(state, force: true, format.Level);
            return;
        }

        // At least two: the flag means "a table nested inside a cell", so it cannot be the
        // outermost one however the depth sprm reads.
        int level = Math.Max(2, format.Level);

        if (format.IsInnerTableRowEnd)
        {
            FinishParagraph(state, force: false, level);
            FinishRow(state, format, level);
            return;
        }

        FinishParagraph(state, force: true, level);
        FinishCell(state, level);
    }

    /// <summary>
    /// Ends a table cell, and the row too when the paragraph's properties say so.
    /// </summary>
    /// <remarks>
    /// U+0007 means both things. Only <c>sprmPFTtp</c> on the paragraph that contains it
    /// distinguishes the mark that ends a row from the one that ends a cell — so a reader that
    /// treats every U+0007 the same either produces one row per cell or one cell per row. This mark
    /// always belongs to the outermost table; a nested table's cells end at paragraph marks instead.
    /// </remarks>
    private void EndCellOrRow(WalkState state, int markPosition)
    {
        Ww8ParagraphFormat format = ResolveParagraphFormat(markPosition);
        state.ParagraphFormat = format;

        const int OutermostTable = 1;

        if (format.IsTableRowEnd)
        {
            // The row-end mark also closes whatever cell was open. The format is passed on rather
            // than read back from the state, because finishing the paragraph clears it — and the
            // row's geometry lives on this paragraph, so losing it loses every column span.
            FinishParagraph(state, force: false, OutermostTable);
            FinishRow(state, format, OutermostTable);
            return;
        }

        FinishParagraph(state, force: true, OutermostTable);
        FinishCell(state, OutermostTable);
    }

    private void FinishParagraph(WalkState state, bool force, int level)
    {
        FlushRun(state);

        bool hasContent = state.Runs.Count > 0 || state.PendingImages.Count > 0;
        if (!hasContent && !force)
        {
            ResetParagraph(state);
            return;
        }

        Ww8ParagraphFormat format = state.ParagraphFormat;

        // The label has to be produced here rather than when the format was resolved, because
        // advancing a counter is a side effect: a paragraph whose properties are read twice must
        // still count once.
        bool numbered = format.ListNumber > 0;
        int listLevel = format.ListLevel ?? 0;

        ContentParagraph paragraph = new()
        {
            StyleName = format.StyleName,
            HeadingLevel = format.HeadingLevel,
            ListLevel = numbered ? listLevel : null,
            ListMarker = numbered ? _numbering.Advance(format.ListNumber, listLevel) : null,
        };
        foreach (ContentRun run in state.Runs) paragraph.Children.Add(run);
        foreach (ContentImage image in state.PendingImages) paragraph.Children.Add(image);

        // Anything shallower than the open tables closes them, since WW8 marks no table end — a
        // paragraph back at the enclosing level is what says the nested table finished.
        CloseTablesDeeperThan(state, level);
        Destination(state, level).Add(paragraph);

        ResetParagraph(state);
    }

    private static void ResetParagraph(WalkState state)
    {
        state.Runs.Clear();
        state.PendingImages.Clear();
        state.ParagraphFormat = default;
    }

    // ------------------------------------------------------------------- formatting

    /// <summary>
    /// The paragraph formatting at a paragraph mark.
    /// </summary>
    /// <remarks>
    /// Resolved by applying the style chain's sprms and then the paragraph's own, so the nearest
    /// wins — the same shape as the other formats' resolvers, over a completely different encoding.
    /// </remarks>
    private Ww8ParagraphFormat ResolveParagraphFormat(int position)
    {
        int byteOffset = _pieces.FileOffsetOf(position);
        (ushort styleIndex, ReadOnlyMemory<byte> direct) =
            Ww8FormattingTable.SplitParagraphProperties(_paragraphProperties.Find(byteOffset));

        Ww8ParagraphFormat format = new()
        {
            StyleName = _styles.NameOf(styleIndex),
        };

        foreach (ReadOnlyMemory<byte> inherited in _styles.ResolveChain(styleIndex))
            format = ApplyParagraphSprms(format, inherited);

        return ApplyParagraphSprms(format, direct);
    }

    private static Ww8ParagraphFormat ApplyParagraphSprms(
        Ww8ParagraphFormat format, ReadOnlyMemory<byte> grpprl)
    {
        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            switch (sprm.Identifier)
            {
                case Ww8SprmReader.Ids.OutlineLevel:
                    // Zero-based, and 9 is WW8's "body text" rather than a tenth heading level.
                    format = format with
                    {
                        HeadingLevel = sprm.Byte <= 8 ? sprm.Byte + 1 : null,
                    };
                    break;

                case Ww8SprmReader.Ids.ListLevel:
                    format = format with { ListLevel = sprm.Byte };
                    break;

                case Ww8SprmReader.Ids.ListFormatOverride:
                    format = format with { ListNumber = sprm.Word };
                    break;

                case Ww8SprmReader.Ids.InTable:
                    format = format with { IsInTable = sprm.Byte != 0 };
                    break;

                case Ww8SprmReader.Ids.IsTableRowEnd:
                    format = format with { IsTableRowEnd = sprm.Byte != 0 };
                    break;

                case Ww8SprmReader.Ids.TableDepth:
                    format = format with { TableDepth = sprm.DoubleWord };
                    break;

                case Ww8SprmReader.Ids.CellPaddingDefault or Ww8SprmReader.Ids.CellPadding:
                    if (ReadCellPadding(
                            sprm.Operand,
                            isDefault: sprm.Identifier == Ww8SprmReader.Ids.CellPaddingDefault)
                        is { } padding)
                    {
                        format = format with
                        {
                            CellPaddings = [.. format.CellPaddings ?? [], padding],
                        };
                    }

                    break;

                case Ww8SprmReader.Ids.IsInnerTableCell:
                    format = format with { IsInnerTableCell = sprm.Byte != 0 };
                    break;

                case Ww8SprmReader.Ids.IsInnerTableRowEnd:
                    format = format with { IsInnerTableRowEnd = sprm.Byte != 0 };
                    break;

                case Ww8SprmReader.Ids.IsTableHeaderRow:
                    format = format with { IsTableHeaderRow = sprm.Byte != 0 };
                    break;
                case Ww8SprmReader.Ids.RowHeight:
                    // Signed as the sprm gave it: positive is a floor and negative an exact height that
                    // clips, and the sign is the only thing that says which.
                    format = format with { RowHeightTwips = sprm.SignedWord };
                    break;

                case Ww8SprmReader.Ids.TableDefinition:
                    format = format with { TableDefinition = ReadTableDefinition(sprm.Operand) };
                    break;
            }
        }
        return format;
    }

    /// <summary>
    /// The character formatting at a position.
    /// </summary>
    /// <remarks>
    /// Memoised on the exception's byte range rather than resolved per character: the walk asks for
    /// every position in the document, and a formatting run usually covers hundreds of them.
    /// </remarks>
    private Ww8CharacterFormat ResolveCharacterFormat(int position, WalkState state)
    {
        int byteOffset = _pieces.FileOffsetOf(position);
        if (state.FormatCacheValid && byteOffset >= state.FormatCacheStart && byteOffset < state.FormatCacheEnd)
            return state.CachedFormat;

        (ReadOnlyMemory<byte> direct, int start, int end) = _characterProperties.FindWithRange(byteOffset);

        Ww8CharacterFormat format = new();

        // The paragraph style contributes character formatting too — that is how a heading style
        // makes its text bold without every run repeating it.
        foreach (ReadOnlyMemory<byte> inherited in
                 _styles.ResolveCharacterChain(ParagraphStyleIndexAt(position)))
            format = ApplyCharacterSprms(format, inherited);

        // Then the run's own character style, which has to be found before the exception is applied
        // rather than while applying it: the sprm naming the style sits inside the same grpprl as the
        // direct formatting, so a single pass would apply the style's properties over the direct ones
        // that were meant to override them.
        //
        // Index zero is skipped rather than resolved. WW8 keeps paragraph and character styles in one
        // table and istd 0 is *Normal*, so resolving it here would lay a paragraph style over the run —
        // which for emphasis is harmless and for anything with a value is not.
        if (CharacterStyleIndexIn(direct) is var characterStyle and not 0)
        {
            foreach (ReadOnlyMemory<byte> inherited in
                     _styles.ResolveCharacterChain(characterStyle))
                format = ApplyCharacterSprms(format, inherited);
        }

        format = ApplyCharacterSprms(format, direct);

        state.FormatCacheValid = end > start;
        state.FormatCacheStart = start;
        state.FormatCacheEnd = end;
        state.CachedFormat = format;
        return format;
    }

    /// <summary>
    /// The character style a grpprl names, or zero when it names none.
    /// </summary>
    /// <remarks>
    /// Zero means none. It is not <c>Default Paragraph Font</c>, which is what the name suggests: WW8
    /// keeps paragraph and character styles in one table and istd 0 is <em>Normal</em>, so a caller must
    /// treat zero as "the run names no character style" rather than resolving it.
    /// </remarks>
    private static ushort CharacterStyleIndexIn(ReadOnlyMemory<byte> grpprl)
    {
        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            if (sprm.Identifier == Ww8SprmReader.Ids.CharacterStyle) return sprm.Word;
        }
        return 0;
    }

    private Ww8CharacterFormat ApplyCharacterSprms(
        Ww8CharacterFormat format, ReadOnlyMemory<byte> grpprl)
    {
        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            switch (sprm.Identifier)
            {
                case Ww8SprmReader.Ids.Bold:
                    format = format with { IsBold = sprm.ResolveToggle(format.IsBold) };
                    break;
                case Ww8SprmReader.Ids.Italic:
                    format = format with { IsItalic = sprm.ResolveToggle(format.IsItalic) };
                    break;
                case Ww8SprmReader.Ids.Strike:
                    format = format with { IsStruckThrough = sprm.ResolveToggle(format.IsStruckThrough) };
                    break;
                case Ww8SprmReader.Ids.DoubleStrike:
                    format = format with { IsStruckThrough = sprm.ResolveToggle(format.IsStruckThrough) };
                    break;
                case Ww8SprmReader.Ids.Vanish:
                    format = format with { IsHidden = sprm.ResolveToggle(format.IsHidden) };
                    break;
                case Ww8SprmReader.Ids.IsDeleted:
                    format = format with { IsDeleted = sprm.ResolveToggle(format.IsDeleted) };
                    break;
                case Ww8SprmReader.Ids.Underline:
                    // The operand is the line style; zero is none.
                    format = format with { IsUnderlined = sprm.Byte != 0 };
                    break;
                case Ww8SprmReader.Ids.VerticalPosition:
                    format = format with
                    {
                        IsSuperscript = sprm.Byte == 1,
                        IsSubscript = sprm.Byte == 2,
                    };
                    break;
                case Ww8SprmReader.Ids.FontSize:
                    format = format with { FontSize = Length.FromPoints(sprm.Word / 2.0) };
                    break;
                case Ww8SprmReader.Ids.CharacterStyle:
                    format = format with { CharacterStyleName = _styles.NameOf(sprm.Word) };
                    break;
                case Ww8SprmReader.Ids.Language:
                    format = format with { Language = WindowsLanguages.TagOf(sprm.Word) };
                    break;
            }
        }
        return format;
    }

    /// <summary>The mutable state of one range's walk.</summary>
    private sealed class WalkState(ContentNode target)
    {
        public ContentNode Target { get; } = target;

        public StringBuilder Text { get; } = new();
        public bool HasFormat { get; set; }
        public Ww8CharacterFormat Format { get; set; }
        public List<ContentRun> Runs { get; } = [];
        public List<ContentImage> PendingImages { get; } = [];
        public Ww8ParagraphFormat ParagraphFormat { get; set; }

        public int FieldDepth { get; set; }
        public bool InFieldInstruction { get; set; }
        public StringBuilder Instruction { get; } = new();
        public string? CurrentHyperlink { get; set; }
        public Stack<string?> Hyperlinks { get; } = new();

        /// <summary>
        /// One table under construction per nesting level, outermost first.
        /// </summary>
        /// <remarks>
        /// A list rather than a single set of drafts because a nested table is being built at the same
        /// time as the table whose cell contains it, and the inner one finishes first.
        /// </remarks>
        public List<Ww8TableLevel> Levels { get; } = [];

        public bool FormatCacheValid { get; set; }
        public int FormatCacheStart { get; set; }
        public int FormatCacheEnd { get; set; }
        public Ww8CharacterFormat CachedFormat { get; set; }
    }

}

/// <summary>Paragraph formatting resolved from sprms.</summary>
public readonly record struct Ww8ParagraphFormat
{
    /// <summary>The paragraph style's name, when the document names it.</summary>
    public string? StyleName { get; init; }

    /// <summary>The heading level, or null for body text.</summary>
    public int? HeadingLevel { get; init; }

    /// <summary>The list nesting level, when the paragraph is in a list.</summary>
    public int? ListLevel { get; init; }

    /// <summary>The list this paragraph belongs to, as a list-format-override index.</summary>
    public int ListNumber { get; init; }

    /// <summary>True when the paragraph is inside a table.</summary>
    public bool IsInTable { get; init; }

    /// <summary>
    /// How deeply the paragraph is nested in tables: 1 for a top-level table, 2 for one inside a cell.
    /// </summary>
    public int TableDepth { get; init; }

    /// <summary>True when the paragraph's mark ends a cell of the inner table.</summary>
    public bool IsInnerTableCell { get; init; }

    /// <summary>True when the paragraph's mark ends a row of the inner table.</summary>
    public bool IsInnerTableRowEnd { get; init; }

    /// <summary>
    /// The table nesting level the paragraph belongs to, or zero when it is not in a table.
    /// </summary>
    /// <remarks>
    /// The depth sprm is what a nested table is expressed with, but an older producer may set only
    /// <see cref="IsInTable"/> — so "in a table with no stated depth" means the top level rather than
    /// no level.
    /// </remarks>
    public int Level => IsInTable || TableDepth > 0 ? Math.Max(1, TableDepth) : 0;

    /// <summary>True when the paragraph's mark ends a table row rather than a cell.</summary>
    public bool IsTableRowEnd { get; init; }

    /// <summary>True when the row this paragraph ends repeats as a header on every page.</summary>
    public bool IsTableHeaderRow { get; init; }

    /// <summary>
    /// The row's declared height in twips from <c>sprmTDyaRowHeight</c>, signed; zero for none.
    /// </summary>
    /// <remarks>
    /// Signed on purpose — positive is a floor, negative is an exact height that clips — and carried on the
    /// paragraph format because WW8 states a row's properties on the paragraph mark that <em>ends</em> the row
    /// rather than anywhere a row begins.
    /// </remarks>
    public int RowHeightTwips { get; init; }

    /// <summary>
    /// The cell padding the row declares, as the entries the document stated.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A list rather than four values, because each entry names <em>one value and the sides it applies
    /// to</em> — so a row whose four sides differ carries four entries, and one whose second cell differs
    /// carries a fifth. LibreOffice's own export writes exactly four for a uniform table, one per side,
    /// which is what makes keeping only the last of them a bug that leaves three sides at Word's default.
    /// </para>
    /// <para>
    /// Both spellings land here: <c>sprmTCellPaddingDefault</c> as an entry covering every cell, and
    /// <c>sprmTCellPadding</c> as one covering a range. The range form wins where both apply, which is what
    /// <see cref="Ww8CellPadding.CellLimit"/> distinguishes them by.
    /// </para>
    /// </remarks>
    public IReadOnlyList<Ww8CellPadding>? CellPaddings { get; init; }

    /// <summary>
    /// The row's geometry, when its row-end paragraph declares one.
    /// </summary>
    /// <remarks>
    /// Carried on the paragraph because that is where WW8 puts it: a row's column edges and merge
    /// flags live in the properties of the paragraph whose mark ends the row, not with its cells.
    /// </remarks>
    public Ww8TableDefinition? TableDefinition { get; init; }

}

/// <summary>
/// One <c>sprmTCellPadding</c> or <c>sprmTCellPaddingDefault</c>: a value, the sides it applies to, and
/// the cells it applies to.
/// </summary>
/// <remarks>
/// One structure for both because the two sprms have the same six-byte operand and differ only in how the
/// first two bytes read — the default form names cell zero and means all of them, and the specific form
/// names a half-open range. Both state a single value for however many sides the bits select, which is why
/// a row with four different paddings carries four of these.
/// </remarks>
/// <param name="FirstCell">The first cell it applies to.</param>
/// <param name="CellLimit">One past the last, or zero when it applies to every cell.</param>
/// <param name="Sides">
/// Which sides it sets, as WW8's own bits: 1 top, 2 left, 4 bottom, 8 right. Note the order — top before
/// left — which is neither the order the sides are usually written in nor the order OOXML uses.
/// </param>
/// <param name="Twips">The value.</param>
public readonly record struct Ww8CellPadding(int FirstCell, int CellLimit, int Sides, int Twips)
{
    /// <summary>The bit that selects the top.</summary>
    public const int Top = 1 << 0;

    /// <summary>The bit that selects the left.</summary>
    public const int Left = 1 << 1;

    /// <summary>The bit that selects the bottom.</summary>
    public const int Bottom = 1 << 2;

    /// <summary>The bit that selects the right.</summary>
    public const int Right = 1 << 3;

    /// <summary>True when this entry applies to the cell at an index.</summary>
    public bool Covers(int cell) => CellLimit <= 0 || (cell >= FirstCell && cell < CellLimit);

    /// <summary>The value for a side, or null when this entry does not set that side.</summary>
    public int? For(int side) => (Sides & side) != 0 ? Twips : null;
}

/// <summary>Character formatting resolved from sprms.</summary>
public readonly record struct Ww8CharacterFormat
{
    /// <summary>True when the run is bold.</summary>
    public bool IsBold { get; init; }

    /// <summary>True when the run is italic.</summary>
    public bool IsItalic { get; init; }

    /// <summary>True when the run is underlined.</summary>
    public bool IsUnderlined { get; init; }

    /// <summary>True when the run is struck through.</summary>
    public bool IsStruckThrough { get; init; }

    /// <summary>True when the run is raised.</summary>
    public bool IsSuperscript { get; init; }

    /// <summary>True when the run is lowered.</summary>
    public bool IsSubscript { get; init; }

    /// <summary>True when the run is hidden, so no reader displays it.</summary>
    public bool IsHidden { get; init; }

    /// <summary>True when a tracked change marks the run deleted.</summary>
    public bool IsDeleted { get; init; }

    /// <summary>
    /// Where the run links to, when it sits inside a <c>HYPERLINK</c> field.
    /// </summary>
    /// <remarks>
    /// Part of the format rather than beside it so that a run ends where the link does: two adjacent
    /// pieces of text with identical formatting but different targets are two runs.
    /// </remarks>
    public string? HyperlinkTarget { get; init; }

    /// <summary>The font size, when a sprm sets one.</summary>
    public Length? FontSize { get; init; }

    /// <summary>The character style's name, when the run names one.</summary>
    public string? CharacterStyleName { get; init; }

    /// <summary>The run's language as a BCP 47 tag.</summary>
    public string? Language { get; init; }

    /// <summary>The coarse emphasis flags the content tree records.</summary>
    public RunEmphasis Emphasis
    {
        get
        {
            RunEmphasis emphasis = RunEmphasis.None;
            if (IsBold) emphasis |= RunEmphasis.Bold;
            if (IsItalic) emphasis |= RunEmphasis.Italic;
            if (IsUnderlined) emphasis |= RunEmphasis.Underline;
            if (IsStruckThrough) emphasis |= RunEmphasis.Strikethrough;
            if (IsSuperscript) emphasis |= RunEmphasis.Superscript;
            if (IsSubscript) emphasis |= RunEmphasis.Subscript;
            return emphasis;
        }
    }
}
