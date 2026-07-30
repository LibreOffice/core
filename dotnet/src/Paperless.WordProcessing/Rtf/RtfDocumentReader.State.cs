using System.Text;
using Paperless.Core.Extraction;

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

        public GroupState Clone() => new()
        {
            Destination = Destination,
            Bold = Bold,
            Italic = Italic,
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
            // Deliberately not inherited: a nested group collects its own text, and the
            // stylesheet and font-table bookkeeping belongs to the entry that declared it.
        };

        /// <summary>Applies <c>\plain</c>: character formatting back to nothing.</summary>
        public void ResetCharacter()
        {
            Bold = false;
            Italic = false;
            Underline = false;
            Strike = false;
            Hidden = false;
            VerticalPosition = 0;
            CharacterStyleId = 0;
        }

        /// <summary>Applies <c>\pard</c>: paragraph formatting back to nothing.</summary>
        public void ResetParagraph()
        {
            ParagraphStyleId = 0;
            OutlineLevel = null;
            ListId = 0;
            ListLevel = 0;
            InTable = false;
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

        // Table state. A row definition is open between \trowd and \row, and paragraphs finished
        // while it is open belong to a cell rather than to the section.
        public bool InRowDefinition { get; set; }
        public List<CellDefinition> CellDefinitions { get; } = [];
        public List<CellDraft> RowCells { get; } = [];
        public List<ContentNode> CellContent { get; } = [];
        public List<RowDraft> TableRows { get; } = [];
        public bool PendingCellMergesFirst { get; set; }
        public bool PendingCellMerged { get; set; }
        public bool PendingCellVerticalFirst { get; set; }
        public bool PendingCellVerticalMerged { get; set; }
        public int RowLeftEdge { get; set; }
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
    private readonly record struct CellDefinition(
        int RightEdge, bool MergesFirst, bool Merged, bool VerticalFirst, bool VerticalMerged);

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
        public bool IsHorizontallyMerged { get; init; }

        public bool ContinuesMergeAbove { get; init; }
        public List<ContentNode> Content { get; } = [];
    }

    private sealed class RowDraft
    {
        public int Index { get; init; }

        /// <summary>
        /// The row's left edge in twips, from <c>\trleft</c>. The first cell starts here, so a row
        /// indented from the table's left margin still lines up with the column grid.
        /// </summary>
        public int LeftEdge { get; init; }

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
        string? language = state.LanguageId == 0 ? null : LanguageTagOf(state.LanguageId);
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

    /// <summary>Ends the current paragraph at a <c>\par</c>.</summary>
    private void EmitParagraph(GroupState state)
    {
        if (state.Destination is RtfDestination.Skip or RtfDestination.FontTable
            or RtfDestination.StyleSheet or RtfDestination.Picture) return;

        FinishParagraph(CurrentFlow, force: true);
    }

    /// <summary>
    /// Materialises the half-built paragraph, if there is one.
    /// </summary>
    /// <param name="flow">The flow to finish a paragraph in.</param>
    /// <param name="force">
    /// True at an explicit <c>\par</c>, which produces a paragraph even when it is empty — a blank
    /// line is content. False when merely closing a flow, where an empty trailing paragraph is an
    /// artefact of the markup rather than a blank line the document has.
    /// </param>
    private static void FinishParagraph(Flow flow, bool force = false)
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

        if (flow.InRowDefinition)
        {
            // A paragraph inside an open row definition belongs to a cell, not to the section.
            flow.CellContent.Add(paragraph);
        }
        else
        {
            // Consecutive rows form a table only by being adjacent, so the first thing that is
            // not a row is what closes it — and the table has to land before this paragraph.
            FinishTable(flow);
            flow.Target.Children.Add(paragraph);
        }

        ResetParagraphState(flow);
    }

    private static void ResetParagraphState(Flow flow)
    {
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
                _fieldHyperlink = ParseHyperlink(state.Collected.ToString()) ?? _fieldHyperlink;
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
            FinishParagraph(finished);
            FinishTable(finished);
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

    /// <summary>
    /// Extracts the target from a <c>HYPERLINK</c> field instruction.
    /// </summary>
    /// <remarks>
    /// RTF has no hyperlink markup: a link is a field whose instruction reads
    /// <c>HYPERLINK "https://…"</c>, optionally with switches. The quoted argument is the target,
    /// and <c>\\l</c> introduces a bookmark within the document.
    /// </remarks>
    private static string? ParseHyperlink(string instruction)
    {
        string text = instruction.Trim();
        if (!text.StartsWith("HYPERLINK", StringComparison.OrdinalIgnoreCase)) return null;

        int firstQuote = text.IndexOf('"', StringComparison.Ordinal);
        if (firstQuote < 0) return null;
        int secondQuote = text.IndexOf('"', firstQuote + 1);
        if (secondQuote < 0) return null;

        string target = text[(firstQuote + 1)..secondQuote];
        return target.Length == 0 ? null : target;
    }

    /// <summary>
    /// A BCP 47 tag for an RTF language id.
    /// </summary>
    /// <remarks>
    /// RTF records a Windows LCID. The full mapping is a large static table
    /// (<c>research/05-infrastructure.md</c> section F.3) that the legacy binary readers will need
    /// in full; until then this covers the ids that appear in Western documents and returns null
    /// rather than guessing for the rest, since a wrong language tag is worse than none.
    /// </remarks>
    private static string? LanguageTagOf(int languageId) => languageId switch
    {
        1024 or 1023 => null,   // "no language" and "process none"
        1033 => "en-US",
        2057 => "en-GB",
        3081 => "en-AU",
        4105 => "en-CA",
        1031 => "de-DE",
        2055 => "de-CH",
        3079 => "de-AT",
        1036 => "fr-FR",
        2060 => "fr-BE",
        3084 => "fr-CA",
        1034 => "es-ES",
        1040 => "it-IT",
        1043 => "nl-NL",
        1030 => "da-DK",
        1044 => "nb-NO",
        1053 => "sv-SE",
        1035 => "fi-FI",
        1045 => "pl-PL",
        1029 => "cs-CZ",
        1038 => "hu-HU",
        1049 => "ru-RU",
        1032 => "el-GR",
        1055 => "tr-TR",
        1041 => "ja-JP",
        2052 => "zh-CN",
        1028 => "zh-TW",
        1042 => "ko-KR",
        1046 => "pt-BR",
        2070 => "pt-PT",
        _ => null,
    };
}
