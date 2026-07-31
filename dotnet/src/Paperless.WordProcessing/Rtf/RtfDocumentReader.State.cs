using System.Text;
using Paperless.Core.Extraction;
using Paperless.Core.Globalization;
using Paperless.Core.Graphics;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Rtf;

/// <content>
/// The state machine's data: what a group means, what a flow accumulates, and how text becomes
/// paragraphs, list labels, table cells and metadata.
/// </content>
public sealed partial class RtfDocumentReader
{
    /// <summary>What the current group's text means.</summary>
    private enum RtfDestination
    {
        /// <summary>Document content.</summary>
        Body = 0,

        /// <summary>Content Paperless does not read: a picture's bytes, a private extension.</summary>
        Skip,

        /// <summary>The font table, read for its character sets.</summary>
        FontTable,

        /// <summary>The colour table, read so that a run can be drawn in its own colour.</summary>
        ColourTable,

        /// <summary>The stylesheet, read for style names and outline levels.</summary>
        StyleSheet,

        /// <summary>The <c>{\info}</c> group itself.</summary>
        Info,

        /// <summary>One text-valued metadata field.</summary>
        InfoField,

        /// <summary>One date-valued metadata field, whose parts are control words.</summary>
        InfoTimestamp,

        /// <summary>A rendered list label, which RTF writes out rather than leaving to be generated.</summary>
        ListText,

        /// <summary>A field's code, which is not its result.</summary>
        FieldInstruction,

        /// <summary>A comment's author name.</summary>
        AnnotationAuthor,

        /// <summary>A picture, recorded as a graphic without decoding its bytes.</summary>
        Picture,
    }

    /// <summary>
    /// The formatting and destination state a group saves and restores.
    /// </summary>
    /// <remarks>
    /// RTF's braces are the only scoping it has: an opening brace saves everything and a closing
    /// brace restores it. So this is copied on every <c>{</c> — which is why it is a small class
    /// of value-typed fields rather than something with references to share.
    /// </remarks>
    private sealed class GroupState
    {
        public RtfDestination Destination { get; set; }
        public bool Bold { get; set; }
        public bool Italic { get; set; }

        /// <summary>
        /// The paragraph's tab stops so far, in the order the control words gave them.
        /// </summary>
        /// <remarks>
        /// A list on the group state because RTF states a paragraph's stops the same way it states its
        /// indents — loose control words before the <c>\par</c> — and <c>\pard</c> clears them. It is
        /// copied when a group nests, so a stop set inside a group does not leak out of it.
        /// </remarks>
        public List<Text.Layout.TabStop> TabStops { get; set; } = [];

        /// <summary>
        /// The alignment the next <c>\tx</c> will carry, which precedes it.
        /// </summary>
        /// <remarks>
        /// RTF writes the kind before the position — <c>\tqr\tx5000</c> — and the kind applies to that one
        /// stop only. So it is held here and consumed by the position that follows.
        /// </remarks>
        public TabAlignment PendingTabAlignment { get; set; }

        /// <summary>The leader the next <c>\tx</c> will carry, or <c>'\0'</c> for none.</summary>
        public char PendingTabLeader { get; set; }

        /// <summary>
        /// The <c>\cf</c> index into the colour table, or null for the automatic colour.
        /// </summary>
        /// <remarks>
        /// An index rather than a colour, because <c>\cf</c> can precede the <c>\colortbl</c> in a
        /// malformed file and because the table is a document-level thing while this is group state.
        /// </remarks>
        public int? ForegroundColourIndex { get; set; }
        public bool Underline { get; set; }
        public bool Strike { get; set; }
        public bool Hidden { get; set; }
        public int VerticalPosition { get; set; }
        public int FontIndex { get; set; }
        public int LanguageId { get; set; }
        public int CharacterStyleId { get; set; }
        public int ParagraphStyleId { get; set; }
        public int? OutlineLevel { get; set; }
        public int ListId { get; set; }
        public int ListLevel { get; set; }
        public bool InTable { get; set; }
        public int UnicodeSkip { get; set; } = 1;
        public string? HyperlinkTarget { get; set; }
        public string? InfoField { get; set; }
        public int? FontTableIndex { get; set; }
        public int? StyleSheetId { get; set; }
        public bool StyleSheetIsCharacter { get; set; }
        public int? StyleSheetBasedOn { get; set; }

        /// <summary>Text collected by a group whose destination is not document content.</summary>
        public StringBuilder Collected { get; } = new();

        // ---- layout formatting
        //
        // Nullable where "the document did not say" has to stay distinguishable from "it said zero":
        // a zero indent is meaningful and a missing one falls back to the style's. The alignment and
        // the flags are not nullable because RTF states them as toggles that \pard clears.

        /// <summary>The font size in half-points, from <c>\fs</c>.</summary>
        public int? FontSizeHalfPoints { get; set; }

        /// <summary>The left indent in twips, from <c>\li</c>.</summary>
        public int? LeftIndent { get; set; }

        /// <summary>The right indent in twips, from <c>\ri</c>.</summary>
        public int? RightIndent { get; set; }

        /// <summary>The first line's extra indent in twips, from <c>\fi</c>; may be negative.</summary>
        public int? FirstLineIndent { get; set; }

        /// <summary>The space above the paragraph in twips, from <c>\sb</c>.</summary>
        public int? SpaceBefore { get; set; }

        /// <summary>The space below it in twips, from <c>\sa</c>.</summary>
        public int? SpaceAfter { get; set; }

        /// <summary>
        /// <c>\sl</c>'s value, whose meaning depends on <see cref="IsMultipleLineSpacing"/> and its sign.
        /// </summary>
        public int? LineSpacing { get; set; }

        /// <summary>True when <c>\slmult1</c> said the spacing is a multiple rather than twips.</summary>
        public bool IsMultipleLineSpacing { get; set; }

        /// <summary>The alignment, as one of RTF's four <c>\q</c> words.</summary>
        public TextAlignment Alignment { get; set; }

        /// <summary>True when the paragraph must stay with the next.</summary>
        public bool KeepWithNext { get; set; }

        /// <summary>True when it must not split across pages.</summary>
        public bool KeepTogether { get; set; }

        /// <summary>True when widow and orphan control applies.</summary>
        public bool HasWidowControl { get; set; }

        /// <summary>True when the paragraph starts a page.</summary>
        public bool StartsNewPage { get; set; }

        /// <summary>True when spacing between paragraphs of one style is suppressed.</summary>
        public bool HasContextualSpacing { get; set; }

        public GroupState Clone() => new()
        {
            Destination = Destination,
            Bold = Bold,
            Italic = Italic,
            ForegroundColourIndex = ForegroundColourIndex,

            // A copy, not the same list: a stop set inside a group belongs to that group.
            TabStops = [.. TabStops],
            PendingTabAlignment = PendingTabAlignment,
            PendingTabLeader = PendingTabLeader,
            Underline = Underline,
            Strike = Strike,
            Hidden = Hidden,
            VerticalPosition = VerticalPosition,
            FontIndex = FontIndex,
            LanguageId = LanguageId,
            CharacterStyleId = CharacterStyleId,
            ParagraphStyleId = ParagraphStyleId,
            OutlineLevel = OutlineLevel,
            ListId = ListId,
            ListLevel = ListLevel,
            InTable = InTable,
            UnicodeSkip = UnicodeSkip,
            HyperlinkTarget = HyperlinkTarget,
            FontSizeHalfPoints = FontSizeHalfPoints,
            LeftIndent = LeftIndent,
            RightIndent = RightIndent,
            FirstLineIndent = FirstLineIndent,
            SpaceBefore = SpaceBefore,
            SpaceAfter = SpaceAfter,
            LineSpacing = LineSpacing,
            IsMultipleLineSpacing = IsMultipleLineSpacing,
            Alignment = Alignment,
            KeepWithNext = KeepWithNext,
            KeepTogether = KeepTogether,
            HasWidowControl = HasWidowControl,
            StartsNewPage = StartsNewPage,
            HasContextualSpacing = HasContextualSpacing,
            // Deliberately not inherited: a nested group collects its own text, and the
            // stylesheet and font-table bookkeeping belongs to the entry that declared it.
        };

        /// <summary>Applies <c>\plain</c>: character formatting back to nothing.</summary>
        public void ResetCharacter()
        {
            Bold = false;
            Italic = false;
            ForegroundColourIndex = null;
            TabStops = [];
            PendingTabAlignment = TabAlignment.Left;
            PendingTabLeader = '\0';
            Underline = false;
            Strike = false;
            Hidden = false;
            VerticalPosition = 0;
            CharacterStyleId = 0;
        }

        /// <summary>Applies <c>\pard</c>: paragraph formatting back to nothing.</summary>
        /// <remarks>
        /// The layout properties are cleared too, which is what makes <c>\pard</c> usable at all: RTF
        /// writes it before every paragraph and then restates only what differs, so anything left behind
        /// leaks into the next paragraph. The font size is <em>not</em> cleared, because <c>\fs</c> is
        /// character formatting and <c>\plain</c> is what resets that.
        /// </remarks>
        public void ResetParagraph()
        {
            ParagraphStyleId = 0;
            OutlineLevel = null;
            ListId = 0;
            ListLevel = 0;
            InTable = false;

            LeftIndent = null;
            RightIndent = null;
            FirstLineIndent = null;
            SpaceBefore = null;
            SpaceAfter = null;
            LineSpacing = null;
            IsMultipleLineSpacing = false;
            Alignment = TextAlignment.Start;
            KeepWithNext = false;
            KeepTogether = false;
            HasWidowControl = false;
            StartsNewPage = false;
            HasContextualSpacing = false;
        }
    }

    /// <summary>
    /// One flow of content: the body, a header, a footnote, a comment or a shape's text.
    /// </summary>
    /// <remarks>
    /// A flow exists because these are not part of each other. A footnote's paragraphs belong to
    /// the footnote, not to the paragraph that references it, so each gets its own section and its
    /// own half-built paragraph — and RTF nests them by group, so they nest here too.
    /// </remarks>
    private sealed class Flow(ContentSection target)
    {
        public ContentSection Target { get; } = target;

        /// <summary>The group depth this flow was opened at, so the matching brace closes it.</summary>
        public int Depth { get; init; }

        public List<ContentRun> PendingRuns { get; } = [];
        public StringBuilder PendingText { get; } = new();

        /// <summary>
        /// The paragraph's runs as <em>layout</em> divides them, which is not how content does.
        /// </summary>
        /// <remarks>
        /// A separate list because the two split on different things. A content run splits on the coarse
        /// emphasis flags, a character style and a hyperlink; a layout run splits on anything that moves a
        /// glyph or changes its colour — the font index, the size, the weight, the slant, the language.
        /// A size change alone splits one and not the other.
        /// </remarks>
        public List<RtfLayoutRun> LayoutRuns { get; } = [];

        /// <summary>How many characters of the paragraph have been appended so far.</summary>
        /// <remarks>
        /// Tracked rather than derived, because a layout run's offsets are into the paragraph's whole text
        /// and that text is only concatenated once the paragraph closes.
        /// </remarks>
        public int LayoutLength { get; set; }
        public RunEmphasis PendingEmphasis { get; set; }
        public string? PendingStyleName { get; set; }
        public string? PendingLanguage { get; set; }
        public string? PendingHyperlink { get; set; }
        public bool HasPendingFormat { get; set; }

        public StringBuilder ListMarker { get; } = new();
        public int? ListLevel { get; set; }
        public int? HeadingLevel { get; set; }
        public string? StyleName { get; set; }
        public List<ContentImage> PendingImages { get; } = [];

        /// <summary>
        /// One table under construction per nesting level, outermost first.
        /// </summary>
        /// <remarks>
        /// A list rather than one set of drafts because a nested table is being built while the table
        /// whose cell contains it is still open, and the inner one finishes first.
        /// </remarks>
        public List<TableLevel> Levels { get; } = [];

        /// <summary>
        /// The nesting level the paragraphs being read belong to, from <c>\itap</c>.
        /// </summary>
        /// <remarks>
        /// Zero outside a table. <c>\intbl</c> without <c>\itap</c> means the top level, which is how
        /// a producer that predates nesting writes a table — so the two have to be combined rather
        /// than either being trusted alone.
        /// </remarks>
        public int TableLevelIndex { get; set; }

        /// <summary>True when <c>\intbl</c> has been seen for the paragraph being read.</summary>
        public bool InTable { get; set; }

        /// <summary>The level that <c>\trowd</c> and the cell declarations after it apply to.</summary>
        /// <remarks>
        /// A nested row's definition arrives <em>after</em> its cells, inside
        /// <c>{\*\nesttableprops}</c>, and names no level of its own — so it belongs to whichever
        /// level the cells before it were at.
        /// </remarks>
        public int DefinitionLevel { get; set; }
    }

    /// <summary>One table under construction, at one nesting level.</summary>
    private sealed class TableLevel
    {
        /// <summary>The cell declarations from the row definition, in the order they arrived.</summary>
        /// <remarks>
        /// Applied to the cells when the row closes rather than as each cell ends, because a nested
        /// row's definition comes after its cells — so at the moment a nested cell ends, nothing is
        /// yet known about its geometry.
        /// </remarks>
        public List<CellDefinition> CellDefinitions { get; } = [];

        public List<CellDraft> RowCells { get; } = [];
        public List<ContentNode> CellContent { get; } = [];

        /// <summary>The current cell's layout blocks, until the cell mark collects them.</summary>
        public List<RtfLayoutBlock> CellLayout { get; } = [];

        public List<RowDraft> TableRows { get; } = [];

        /// <summary>The half-gap between cells from <c>\trgaph</c>, in twips, or null for none.</summary>
        /// <remarks>
        /// RTF's oldest way of stating cell padding, and the one LibreOffice writes: a single value that
        /// applies to the left and right of every cell in the row. The per-cell <c>\clpad*</c> words
        /// override it where present.
        /// </remarks>
        public int? RowHalfGap { get; set; }

        /// <summary>The row's declared height from <c>\trrh</c>, in twips.</summary>
        public int RowHeight { get; set; }

        /// <summary>
        /// The padding <c>\clpad*</c> stated for the cell being declared, in left, right, top, bottom
        /// order — which is not the order the control words name.
        /// </summary>
        public int?[] PendingCellPadding { get; } = new int?[4];

        /// <summary>The row-wide default cell padding from <c>\trpadd*</c>, same order.</summary>
        /// <remarks>
        /// A default for every cell of the row rather than a property of the row itself, which is how
        /// LibreOffice's importer treats it — it writes the value to the table's cell margins and to the
        /// current cell at once (<c>rtfdispatchvalue.cxx</c>, <c>RTFKeyword::TRPADDL</c>).
        /// </remarks>
        public int?[] RowPadding { get; } = new int?[4];

        /// <summary>The vertical alignment <c>\clvertal*</c> stated for the cell being declared.</summary>
        public Layout.CellVerticalAlignment PendingCellAlignment { get; set; }

        public bool PendingCellMergesFirst { get; set; }
        public bool PendingCellMerged { get; set; }
        public bool PendingCellVerticalFirst { get; set; }
        public bool PendingCellVerticalMerged { get; set; }

        public int RowLeftEdge { get; set; }

        /// <summary><c>\trhdr</c>: the row repeats as a header at the top of every page.</summary>
        public bool RowIsHeader { get; set; }
    }

    /// <summary>A cell's declaration from <c>\cellx</c> and the merge flags before it.</summary>
    /// <param name="RightEdge">
    /// The cell's right edge in twips, which is what <c>\cellx</c> actually states — and, with the
    /// rest of the table's edges, what reveals a merge that carries no flag.
    /// </param>
    /// <param name="MergesFirst"><c>\clmgf</c>: this cell starts a horizontal merge.</param>
    /// <param name="Merged"><c>\clmrg</c>: this cell is merged into the one before it.</param>
    /// <param name="VerticalFirst"><c>\clvmgf</c>: this cell starts a vertical merge.</param>
    /// <param name="VerticalMerged"><c>\clvmrg</c>: this cell continues a vertical merge.</param>
    /// <param name="Padding">
    /// The four <c>\clpad*</c> values in left, right, top, bottom order, each null when the cell states
    /// none and so falls back to the row's half-gap.
    /// </param>
    /// <param name="VerticalAlignment">Where the cell's text sits inside its row.</param>
    private readonly record struct CellDefinition(
        int RightEdge,
        bool MergesFirst,
        bool Merged,
        bool VerticalFirst,
        bool VerticalMerged,
        int?[]? Padding = null,
        Layout.CellVerticalAlignment VerticalAlignment = Layout.CellVerticalAlignment.Top);

    private sealed class CellDraft
    {
        /// <summary>
        /// The column the cell starts at. Settable because a horizontal merge earlier in the row
        /// shifts every cell after it, and that is only known once the whole row is read.
        /// </summary>
        public int ColumnStart { get; set; }

        public int ColumnSpan { get; set; } = 1;
        public int RowSpan { get; set; } = 1;

        /// <summary>The cell's right edge in twips, from <c>\cellx</c>.</summary>
        public int RightEdge { get; set; }

        /// <summary>True when <c>\clmrg</c> merged this cell into the one before it.</summary>
        public bool IsHorizontallyMerged { get; set; }

        public bool ContinuesMergeAbove { get; set; }

        /// <summary>The gap between the cell's edges and its text, resolved when the row closes.</summary>
        public Layout.CellPadding Padding { get; set; }

        /// <summary>Where its text sits when the row is taller than its content.</summary>
        public Layout.CellVerticalAlignment VerticalAlignment { get; set; }

        public List<ContentNode> Content { get; } = [];

        /// <summary>
        /// The cell's blocks with the formatting layout needs, beside the content nodes.
        /// </summary>
        /// <remarks>
        /// Two lists rather than one, for the same reason the body has two: extraction discards the sizes
        /// and indents that decide where a line breaks, and a cell's text has to break at the cell's width.
        /// Blocks rather than paragraphs because a cell can hold a table — RTF nests by <c>\itap</c> depth,
        /// and a finished inner table is appended to whichever cell of the enclosing level was open.
        /// </remarks>
        public List<RtfLayoutBlock> LayoutBlocks { get; } = [];
    }

    private sealed class RowDraft
    {
        public int Index { get; init; }

        /// <summary>True when the row repeats as a header on every page the table spans.</summary>
        public bool IsHeader { get; init; }

        /// <summary>
        /// The row's left edge in twips, from <c>\trleft</c>. The first cell starts here, so a row
        /// indented from the table's left margin still lines up with the column grid.
        /// </summary>
        public int LeftEdge { get; init; }

        /// <summary>The row's declared height in twips, from <c>\trrh</c>; zero for none.</summary>
        public int Height { get; init; }

        public List<CellDraft> Cells { get; } = [];
    }

    private Flow CurrentFlow => _flows[^1];

    private int _groupDepth;
    private string? _fieldHyperlink;
    private int _fieldDepth = -1;

    // ------------------------------------------------------------------------ text

    /// <summary>
    /// Routes text to whatever the current group is collecting.
    /// </summary>
    private void AppendText(GroupState state, string text)
    {
        if (text.Length == 0) return;

        switch (state.Destination)
        {
            case RtfDestination.Body:
                // Hidden text is not displayed by any reader, so extracting it would inject text
                // the document does not show.
                if (state.Hidden) return;
                AppendToParagraph(state, text);
                return;

            case RtfDestination.ListText:
                CurrentFlow.ListMarker.Append(text);
                return;

            case RtfDestination.ColourTable:
                // A colour table's entries are separated by semicolons in its text; the components
                // themselves are control words, so the text carries only the delimiters.
                foreach (char character in text)
                {
                    if (character == ';') EndColourTableEntry();
                }
                return;

            case RtfDestination.FontTable:
                // The family name, which the reader previously discarded because extraction never needs
                // it — a run's font does not change its text. Layout does need it, and this is the only
                // place RTF states it.
                if (state.FontTableIndex is { } fontIndex) RecordFontFamily(fontIndex, text);
                return;

            case RtfDestination.StyleSheet:
            case RtfDestination.InfoField:
            case RtfDestination.AnnotationAuthor:
            case RtfDestination.FieldInstruction:
                state.Collected.Append(text);
                return;

            default:
                return;
        }
    }

    private void AppendToParagraph(GroupState state, string text)
    {
        Flow flow = CurrentFlow;

        RunEmphasis emphasis = EmphasisOf(state);
        string? styleName = state.CharacterStyleId == 0
            ? null
            : _styles.CharacterStyle(state.CharacterStyleId)?.Name;
        string? language = state.LanguageId is < 0 or > ushort.MaxValue
            ? null
            : WindowsLanguages.TagOf((ushort)state.LanguageId);
        string? hyperlink = _fieldHyperlink ?? state.HyperlinkTarget;

        if (flow.HasPendingFormat
            && (flow.PendingEmphasis != emphasis
                || !string.Equals(flow.PendingStyleName, styleName, StringComparison.Ordinal)
                || !string.Equals(flow.PendingLanguage, language, StringComparison.Ordinal)
                || !string.Equals(flow.PendingHyperlink, hyperlink, StringComparison.Ordinal)))
        {
            FlushRun(flow);
        }

        flow.HasPendingFormat = true;
        flow.PendingEmphasis = emphasis;
        flow.PendingStyleName = styleName;
        flow.PendingLanguage = language;
        flow.PendingHyperlink = hyperlink;
        flow.PendingText.Append(text);
        RecordLayoutRun(flow, state, text.Length);

        // The paragraph's own properties are whatever was in force when its text was written.
        flow.StyleName ??= state.ParagraphStyleId == 0
            ? null
            : _styles.ParagraphStyle(state.ParagraphStyleId)?.Name;
        flow.HeadingLevel ??= HeadingLevelOf(state);
        if (flow.ListLevel is null && state.ListId != 0) flow.ListLevel = state.ListLevel;
    }

    private static RunEmphasis EmphasisOf(GroupState state)
    {
        RunEmphasis emphasis = RunEmphasis.None;
        if (state.Bold) emphasis |= RunEmphasis.Bold;
        if (state.Italic) emphasis |= RunEmphasis.Italic;
        if (state.Underline) emphasis |= RunEmphasis.Underline;
        if (state.Strike) emphasis |= RunEmphasis.Strikethrough;
        if (state.VerticalPosition > 0) emphasis |= RunEmphasis.Superscript;
        if (state.VerticalPosition < 0) emphasis |= RunEmphasis.Subscript;
        return emphasis;
    }

    /// <summary>
    /// The outline level in force, from the paragraph directly or from its style.
    /// </summary>
    /// <remarks>
    /// RTF's <c>\outlinelevel</c> is zero-based, and 9 is its "body text" value rather than a
    /// tenth heading level.
    /// </remarks>
    private int? HeadingLevelOf(GroupState state)
    {
        int? level = state.OutlineLevel
                     ?? (state.ParagraphStyleId == 0 ? null : _styles.OutlineLevelOf(state.ParagraphStyleId));
        return level is >= 0 and <= 8 ? level + 1 : null;
    }

    private static void FlushRun(Flow flow)
    {
        if (flow.PendingText.Length > 0)
        {
            flow.PendingRuns.Add(new ContentRun
            {
                Text = flow.PendingText.ToString(),
                StyleName = flow.PendingStyleName,
                Language = flow.PendingLanguage,
                Emphasis = flow.PendingEmphasis,
                HyperlinkTarget = flow.PendingHyperlink,
            });
        }
        flow.PendingText.Clear();
        flow.HasPendingFormat = false;
    }

    // ------------------------------------------------------------------- paragraphs

    /// <summary>
    /// Records a font table entry's family name.
    /// </summary>
    /// <remarks>
    /// Appended rather than assigned, because a name can arrive in several text chunks — an escape or a
    /// Unicode character in the middle of it splits the run. The entry ends at a semicolon, so anything
    /// after one belongs to the table's syntax rather than to the name, and a trailing space is the
    /// delimiter of the control word that preceded it.
    /// </remarks>
    private void RecordFontFamily(int index, string text)
    {
        string name = _fontFamilies.GetValueOrDefault(index, string.Empty) + text;

        int terminator = name.IndexOf(';', StringComparison.Ordinal);
        if (terminator >= 0) name = name[..terminator];

        name = name.Trim();
        if (name.Length > 0) _fontFamilies[index] = name;
    }

    /// <summary>Ends the current paragraph at a <c>\par</c>.</summary>
    private void EmitParagraph(GroupState state)
    {
        if (state.Destination is RtfDestination.Skip or RtfDestination.FontTable
            or RtfDestination.StyleSheet or RtfDestination.Picture) return;

        FinishParagraph(CurrentFlow, state, force: true);
    }

    /// <summary>
    /// Materialises the half-built paragraph, if there is one.
    /// </summary>
    /// <param name="flow">The flow to finish a paragraph in.</param>
    /// <param name="state">
    /// The formatting in force, when the caller has it. RTF states a paragraph's properties
    /// <em>before</em> its <c>\par</c>, so this is the paragraph's own formatting and not the next
    /// one's — which is why it is taken at the point the paragraph closes rather than where it began.
    /// Null where a flow is being closed and no group state applies, in which case the paragraph still
    /// reaches the content tree and simply is not recorded for layout.
    /// </param>
    /// <param name="force">
    /// True at an explicit <c>\par</c>, which produces a paragraph even when it is empty — a blank
    /// line is content. False when merely closing a flow, where an empty trailing paragraph is an
    /// artefact of the markup rather than a blank line the document has.
    /// </param>
    private void FinishParagraph(Flow flow, GroupState? state = null, bool force = false)
    {
        FlushRun(flow);

        bool hasContent = flow.PendingRuns.Count > 0 || flow.PendingImages.Count > 0;
        if (!hasContent && !force)
        {
            ResetParagraphState(flow);
            return;
        }

        string marker = flow.ListMarker.ToString().Trim();
        ContentParagraph paragraph = new()
        {
            StyleName = flow.StyleName,
            HeadingLevel = flow.HeadingLevel,
            ListLevel = flow.HeadingLevel is null ? flow.ListLevel : null,
            ListMarker = marker.Length == 0 ? null : marker,
        };
        foreach (ContentRun run in flow.PendingRuns) paragraph.Children.Add(run);
        foreach (ContentImage image in flow.PendingImages) paragraph.Children.Add(image);

        // Consecutive rows form a table only by being adjacent, so the first paragraph that is not
        // in one is what closes it — and the table has to land before that paragraph. Resolved before the
        // layout record for exactly that reason: closing the table is what appends it to the block list,
        // so recording this paragraph first would put the paragraph after the table it follows.
        IList<ContentNode> destination = Destination(flow, LevelOf(flow));

        if (state is not null)
        {
            // The layout text differs from the extracted text in one character: a manual line break, which
            // this reader appends as a newline because that is what an extracted paragraph should hold,
            // becomes U+2028 for layout — the separator the other three readers use, and the one UAX #14
            // gives a mandatory break without also reading as the end of a paragraph.
            RecordLayoutParagraph(
                flow,
                state,
                string.Concat(flow.PendingRuns.Select(run => run.Text))
                    .Replace('\n', LayoutLineSeparator));
        }

        destination.Add(paragraph);

        ResetParagraphState(flow);
    }

    /// <summary>U+2028, the line separator a manual line break becomes for layout.</summary>
    private const char LayoutLineSeparator = '\u2028';

    /// <summary>Starts a fresh colour table, discarding anything a previous one declared.</summary>
    /// <remarks>
    /// A document has one, but a malformed one can open a second, and the entries are positional — so
    /// appending to the old table would shift every <c>\cf</c> after it.
    /// </remarks>
    private void BeginColourTable()
    {
        _colours.Clear();
        ResetColourComponents();
    }

    /// <summary>Records one component of the colour table entry being built.</summary>
    private void SetColourComponent(string name, int value)
    {
        int component = Math.Clamp(value, 0, 255);

        switch (name)
        {
            case "red": _colourRed = component; break;
            case "green": _colourGreen = component; break;
            case "blue": _colourBlue = component; break;
            default: return;
        }

        _colourStated = true;
    }

    /// <summary>
    /// Closes a colour table entry at its semicolon.
    /// </summary>
    /// <remarks>
    /// The semicolon is the delimiter, not the components — an entry with no <c>\red</c>/<c>\green</c>/
    /// <c>\blue</c> at all is the "automatic" colour, and it still occupies an index. Dropping it would
    /// shift every colour in the document by one.
    /// </remarks>
    private void EndColourTableEntry()
    {
        if (_colours.Count < MaxColours)
        {
            _colours.Add(_colourStated
                ? Colour.FromRgb((uint)((_colourRed << 16) | (_colourGreen << 8) | _colourBlue))
                : null);
        }

        ResetColourComponents();
    }

    private void ResetColourComponents()
    {
        _colourRed = 0;
        _colourGreen = 0;
        _colourBlue = 0;
        _colourStated = false;
    }

    /// <summary>
    /// The colour a <c>\cf</c> index names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// Zero-based, as the specification numbers the table. An index past the end is a producer error and
    /// resolves to nothing rather than to black, so the document's own default applies — as does an index
    /// naming an entry that stated no components, which is what <c>\cf0</c> usually is.
    /// </remarks>
    private Colour? ColourAt(int? index)
        => index is { } at && at >= 0 && at < _colours.Count ? _colours[at] : null;

    /// <summary>
    /// Records the character formatting in force over a stretch of the paragraph's text.
    /// </summary>
    /// <remarks>
    /// Appended as the text is, because RTF has nothing to revisit: the state machine's current group is
    /// the only place the formatting exists, and by the time the paragraph closes it holds whatever the
    /// last run set. Consecutive stretches with identical formatting merge, which matters because RTF
    /// restates properties freely — a producer writes <c>\f0\fs22</c> before every run whether or not
    /// anything changed, and each restatement would otherwise break the shaping context.
    /// </remarks>
    private void RecordLayoutRun(Flow flow, GroupState state, int length)
    {
        if (length <= 0) return;

        RtfLayoutRun run = new(
            flow.LayoutLength,
            length,
            _fontFamilies.GetValueOrDefault(state.FontIndex),
            SizeOf(state),
            state.Bold ? 700 : 400,
            state.Italic,
            state.LanguageId is > 0 and <= ushort.MaxValue
                ? WindowsLanguages.TagOf((ushort)state.LanguageId)
                : null,
            ColourAt(state.ForegroundColourIndex));

        flow.LayoutLength += length;

        if (flow.LayoutRuns.Count > 0 && flow.LayoutRuns[^1].MatchesFormatting(run))
        {
            flow.LayoutRuns[^1] = flow.LayoutRuns[^1] with
            {
                Length = flow.LayoutRuns[^1].Length + length,
            };
            return;
        }

        flow.LayoutRuns.Add(run);
    }

    /// <summary>
    /// Records a paragraph's layout formatting as it closes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The body's paragraphs and the furniture's, kept in separate lists because they are laid out into
    /// separate frames: the body's index has to mean the same thing to the paginator as it does here, so a
    /// header's paragraph cannot share the numbering. A footnote's or a shape's does not reach layout at
    /// all yet, and neither does anything inside a table — a table is laid out as a grid rather than as a
    /// run of paragraphs.
    /// </para>
    /// <para>
    /// Recorded as each paragraph closes rather than by a second pass, because RTF is a token stream with
    /// no structure to revisit; re-reading it would mean running the whole state machine again.
    /// </para>
    /// </remarks>
    private void RecordLayoutParagraph(Flow flow, GroupState state, string text)
    {
        List<RtfLayoutParagraph>? into;

        List<RtfLayoutBlock>? cell = null;

        if (flow.InTable)
        {
            // A cell's paragraphs, staged on its own table level until the cell mark collects them. Any
            // level, not just the outermost: an inner table's cells are a flow, and a flow holds blocks.
            int level = LevelOf(flow);
            cell = ReferenceEquals(flow, _flows[0]) ? LevelAt(flow, level).CellLayout : null;
            into = null;
        }
        else
        {
            into = ReferenceEquals(flow, _flows[0]) ? _layoutBlocks.Paragraphs : FurnitureList(flow);
        }

        if (cell is null && into is null) return;
        if ((cell?.Count ?? into!.Count) >= MaxLayoutParagraphs) return;

        RtfLayoutParagraph recorded = new(
            text,
            new Ww8.Ww8LayoutFormat
            {
                Justification = null,
                LeftIndent = state.LeftIndent,
                RightIndent = state.RightIndent,
                FirstLineIndent = state.FirstLineIndent,
                SpaceBefore = state.SpaceBefore,
                SpaceAfter = state.SpaceAfter,
                LineSpacing = state.LineSpacing,
                IsMultipleLineSpacing = state.IsMultipleLineSpacing,
                KeepTogether = state.KeepTogether,
                KeepWithNext = state.KeepWithNext,
                StartsNewPage = state.StartsNewPage,
                HasWidowControl = state.HasWidowControl,
                HasContextualSpacing = state.HasContextualSpacing,
            }.ToParagraphFormat(SizeOf(state)) with
            {
                Alignment = state.Alignment,
                TabStops = [.. state.TabStops.OrderBy(stop => stop.Position.Emu)],
                DefaultTabInterval = _defaultTabInterval,
            },
            _fontFamilies.GetValueOrDefault(state.FontIndex),
            SizeOf(state),
            state.Bold ? 700 : 400,
            state.Italic,
            state.LanguageId > 0 ? WindowsLanguages.TagOf((ushort)state.LanguageId) : null,
            ColourAt(state.ForegroundColourIndex),
            [.. flow.LayoutRuns],
            _sectionIndex);

        if (cell is not null) cell.Add(new RtfLayoutBlock(recorded));
        else into!.Add(recorded);
    }

    /// <summary>
    /// The body's blocks in document order, with the staging list the paragraph recorder appends to.
    /// </summary>
    /// <remarks>
    /// A paragraph is appended to <see cref="Staged.Paragraphs"/> as it closes, and the wrapper flushes
    /// that run of paragraphs into the block list before a table is added — so the blocks end up in
    /// document order without the recorder having to know that tables exist.
    /// </remarks>
    private sealed class Staged
    {
        public List<RtfLayoutParagraph> Paragraphs { get; } = [];

        public List<RtfLayoutBlock> Blocks { get; } = [];

        /// <summary>Moves the paragraphs collected so far into the block list.</summary>
        public void Flush()
        {
            foreach (RtfLayoutParagraph paragraph in Paragraphs) Blocks.Add(new RtfLayoutBlock(paragraph));
            Paragraphs.Clear();
        }

        /// <summary>Adds a table after whatever paragraphs preceded it.</summary>
        public void Add(RtfLayoutTable table)
        {
            Flush();
            Blocks.Add(new RtfLayoutBlock(table));
        }

        /// <summary>Everything, with any trailing paragraphs included.</summary>
        public List<RtfLayoutBlock> Finished()
        {
            Flush();
            return Blocks;
        }
    }

    /// <summary>
    /// The list a header's or footer's paragraphs go in, or null when the flow is not furniture.
    /// </summary>
    /// <remarks>
    /// <para>
    /// RTF names its four header destinations by suffix, and the mapping onto the three slots every format
    /// shares is not one-to-one. <c>\headerf</c> is the first page's and <c>\headerl</c> the left-hand —
    /// which is the even one, since a document's first page is a right-hand page. <c>\headerr</c> is the
    /// right-hand, so it is the <em>default</em>: the slot every page takes that no other slot claims,
    /// which for a facing-pages document is exactly the odd ones. A document writing both <c>\header</c>
    /// and <c>\headerr</c> means the latter, and it comes second in the file, so the later write winning is
    /// the right answer rather than a coincidence.
    /// </para>
    /// <para>
    /// Only the outermost furniture flow qualifies: a footnote opened inside a header is a
    /// <see cref="SectionKind.Note"/> flow and falls through to null, which keeps its paragraphs out of the
    /// header rather than appending them to it.
    /// </para>
    /// </remarks>
    private List<RtfLayoutParagraph>? FurnitureList(Flow flow)
    {
        bool isHeader = flow.Target.Kind == SectionKind.Header;
        if (!isHeader && flow.Target.Kind != SectionKind.Footer) return null;

        Model.PageFurnitureSlot? slot = flow.Target.Name switch
        {
            "default" or "r" => Model.PageFurnitureSlot.Default,
            "l" => Model.PageFurnitureSlot.Even,
            "f" => Model.PageFurnitureSlot.First,
            _ => null,
        };

        if (slot is null) return null;

        Dictionary<(int, Model.PageFurnitureSlot), List<RtfLayoutParagraph>> slots =
            isHeader ? _headerLayout : _footerLayout;

        // Keyed by the section the flow was opened in: RTF writes a header in the preamble of the section
        // it belongs to, so a document with two running heads has written two headers.
        (int, Model.PageFurnitureSlot) key = (_sectionIndex, slot.Value);

        if (!slots.TryGetValue(key, out List<RtfLayoutParagraph>? paragraphs))
        {
            paragraphs = [];
            slots[key] = paragraphs;
        }

        return paragraphs;
    }

    /// <summary>
    /// The em size in force, defaulting to twelve points.
    /// </summary>
    /// <remarks>
    /// Twelve rather than ten, because RTF's own default is <c>\fs24</c> — a document that states no
    /// size at all is twelve-point text, where an OOXML one relying on its defaults is ten.
    /// </remarks>
    private static Core.Units.Length SizeOf(GroupState state)
        => state.FontSizeHalfPoints is { } halves and > 0 and <= 4000
            ? Core.Units.Length.FromPoints(halves / 2.0)
            : Core.Units.Length.FromPoints(12);

    private static void ResetParagraphState(Flow flow)
    {
        flow.LayoutRuns.Clear();
        flow.LayoutLength = 0;
        flow.PendingRuns.Clear();
        flow.PendingImages.Clear();
        flow.ListMarker.Clear();
        flow.ListLevel = null;
        flow.HeadingLevel = null;
        flow.StyleName = null;
    }

    // ------------------------------------------------------------------------- flows

    /// <summary>
    /// Starts a nested flow — a note, comment, header, footer or shape text — in the current
    /// group.
    /// </summary>
    private void BeginFlow(GroupState state, SectionKind kind, string? name)
    {
        state.Destination = RtfDestination.Body;
        _flows.Add(new Flow(new ContentSection
        {
            Kind = kind,
            Index = _hoisted.Count,
            Name = name,
        })
        {
            Depth = _groupDepth,
        });
    }

    /// <summary>Handles a closing brace: finishes whatever the group was collecting.</summary>
    private void CloseGroup(GroupState state)
    {
        switch (state.Destination)
        {
            case RtfDestination.StyleSheet when state.StyleSheetId is { } id:
                RecordStyle(state, id);
                break;

            case RtfDestination.InfoField when state.InfoField is { } field:
                _info[field] = state.Collected.ToString().TrimEnd(';').Trim();
                break;

            case RtfDestination.AnnotationAuthor:
                _pendingAnnotationAuthor = state.Collected.ToString().Trim();
                break;

            case RtfDestination.FieldInstruction:
                _fieldHyperlink = FieldInstructions.HyperlinkTarget(state.Collected.ToString()) ?? _fieldHyperlink;
                break;

            case RtfDestination.Picture:
                // Recorded so a caller knows a graphic is there. The bytes are not decoded during
                // extraction, and nothing in RTF gives a picture a name or alternative text.
                CurrentFlow.PendingImages.Add(new ContentImage());
                break;
        }

        // A field's hyperlink applies only within that field.
        if (_fieldDepth >= 0 && _groupDepth <= _fieldDepth)
        {
            _fieldHyperlink = null;
            _fieldDepth = -1;
        }

        // A nested flow ends with its group.
        if (_flows.Count > 1 && CurrentFlow.Depth >= _groupDepth)
        {
            Flow finished = CurrentFlow;
            FinishParagraph(finished, state);
            CloseTablesDeeperThan(finished, 0);
            _flows.RemoveAt(_flows.Count - 1);

            if (finished.Target.Children.Count > 0) _hoisted.Add(finished.Target);
        }
    }

    private void RecordStyle(GroupState state, int id)
    {
        string name = state.Collected.ToString().TrimEnd(';').Trim();
        if (name.Length == 0) return;

        _styles.Add(id, new RtfStyle(
            name, state.StyleSheetBasedOn, state.OutlineLevel, state.StyleSheetIsCharacter));
    }

}
