using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.Core.Numbering;
using Paperless.Ooxml;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Walks WordprocessingML content into the format-independent content tree.
/// </summary>
/// <remarks>
/// <para>
/// Three things about WordprocessingML make a naive walk produce wrong text, and each is
/// handled explicitly below rather than by recursing hopefully:
/// </para>
/// <para>
/// <strong>A field's instruction is not its result.</strong> A field is a flat run of
/// <c>w:fldChar</c> markers with the code in <c>w:instrText</c> and the cached result in
/// ordinary runs after the <c>separate</c> marker. Walking everything emits the code —
/// <c>PAGE \* ARABIC</c> — into the text.
/// </para>
/// <para>
/// <strong>Deleted text is still in the file.</strong> <c>w:del</c> wraps runs whose text
/// <c>w:delText</c> was removed by a tracked change. Emitting it puts text no reader shows into
/// the output, and it reads as duplicated content where a word was replaced.
/// </para>
/// <para>
/// <strong>Alternate content is written twice.</strong> A text box appears once as a DrawingML
/// shape and again as a VML fallback. Both are resolved away at load, so exactly one survives —
/// see <see cref="OoxmlXml.Normalise"/>.
/// </para>
/// <para>
/// Notes, comments and text-box flows are hoisted into their own sections rather than spliced
/// into the paragraph that anchors them, for the same reason as in the ODF reader: they are
/// separate flows, and splicing them corrupts the anchoring paragraph's text.
/// </para>
/// </remarks>
public sealed partial class DocxContentReader
{
    /// <summary>How deeply nesting is followed before the reader gives up on a branch.</summary>
    /// <remarks>
    /// Untrusted input parsed with recursion, so the depth is bounded rather than trusted. Real
    /// documents nest a few dozen levels at most — a table in a text box in a cell.
    /// </remarks>
    public const int MaxNestingDepth = 96;

    private readonly DocxFile _file;
    private readonly WordStyles _styles;
    private readonly WordNumbering _numbering;
    private readonly List<Diagnostic> _diagnostics;
    private readonly List<ContentNode> _hoisted = [];

    // Pending-run state, so that adjacent text sharing formatting becomes one ContentRun rather
    // than one per w:t. A DOCX splits runs at every property change and at every proofing mark,
    // so without this a sentence can arrive as a dozen fragments.
    private readonly StringBuilder _pendingText = new();
    private WordCharacterFormat? _pendingFormat;
    private string? _pendingHyperlink;
    private string? _pendingStyleName;

    // Field nesting: true while inside the instruction half of a field, where text is the code
    // rather than the result. A stack because fields nest.
    private readonly List<bool> _fieldInstruction = [];

    // Field instructions, one buffer per open field, so that a nested field's code does not land in
    // its parent's. Parallel to _fieldInstruction, which says which half of each field the walk is in.
    private readonly List<FieldFrame> _fieldFrames = [];

    private readonly Model.WritingMarkBuilder _marks = new();

    private int _footnoteNumber;
    private int _endnoteNumber;
    private int _depth;
    private bool _reportedDepthLimit;

    /// <summary>One open field: the instruction being collected and where its result began.</summary>
    private sealed class FieldFrame
    {
        public StringBuilder Instruction { get; } = new();

        /// <summary>Where the cached result starts, set at the <c>separate</c> marker.</summary>
        public Model.WritingPosition? ResultStart { get; set; }

        /// <summary>The offset the result starts at, for slicing it back out of the paragraph.</summary>
        public int ResultOffset { get; set; }
    }

    /// <summary>The marks the walk recorded: tracked changes, bookmarks and fields.</summary>
    public Model.WritingMarks Marks => _marks.Build();

    /// <summary>Creates a reader over one open document.</summary>
    public DocxContentReader(DocxFile file, List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(diagnostics);
        _file = file;
        _styles = file.Styles;
        _numbering = file.Numbering;
        _diagnostics = diagnostics;
    }

    /// <summary>
    /// Reads the block-level content of a container: a body, a table cell, a text box, a header,
    /// a note or a comment.
    /// </summary>
    public void ReadBlocks(XElement container, ContentNode target)
    {
        ArgumentNullException.ThrowIfNull(container);
        ArgumentNullException.ThrowIfNull(target);

        if (!EnterDepth()) return;
        foreach (XElement child in container.Elements()) ReadBlock(child, target);
        _depth--;
    }

    /// <summary>Removes and returns the note, comment and frame sections collected so far.</summary>
    public List<ContentNode> TakeHoisted()
    {
        List<ContentNode> hoisted = [.. _hoisted];
        _hoisted.Clear();
        return hoisted;
    }

    /// <summary>
    /// The section properties in force at the end of a body, which name its header and footer
    /// parts.
    /// </summary>
    /// <remarks>
    /// A document's final <c>w:sectPr</c> is a child of <c>w:body</c>; earlier sections keep
    /// theirs inside the last paragraph of the section. Both are collected, because a header
    /// belongs to whichever section names it.
    /// </remarks>
    public static IEnumerable<XElement> SectionProperties(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        foreach (XElement paragraph in Word.Children(body, "p"))
        {
            if (Word.Child(Word.Child(paragraph, "pPr"), "sectPr") is { } inParagraph)
                yield return inParagraph;
        }
        if (Word.Child(body, "sectPr") is { } final) yield return final;
    }

    // ----------------------------------------------------------------------- block level

    private void ReadBlock(XElement element, ContentNode target)
    {
        if (element.Name.NamespaceName != OoxmlNamespaces.WordprocessingML)
        {
            // Anything outside the WordprocessingML namespace at block level is markup a
            // producer added — Office's own w14/w15 extensions, or a custom vocabulary. None of
            // it carries document text that is not also present in the standard elements.
            return;
        }

        switch (element.Name.LocalName)
        {
            case "p":
                target.Children.Add(ReadParagraph(element));
                return;

            case "tbl":
                ReadTable(element, target);
                return;

            // A structured document tag — a content control — wraps ordinary content.
            case "sdt":
                if (Word.Child(element, "sdtContent") is { } sdtContent) ReadBlocks(sdtContent, target);
                return;

            case "customXml":
                ReadBlocks(element, target);
                return;

            case "ins" or "moveTo":
                // Inserted and moved-to content is present in the document; the wrapper is
                // tracked-change bookkeeping — which is recorded rather than merely stepped over.
                RecordInsertion(element, paragraph: null, () => ReadBlocks(element, target));
                return;

            case "del" or "moveFrom":
                // Deleted and moved-from content is *not* in the document any more. Reading it
                // would inject text no reader shows; recording it keeps the account of who removed
                // what without putting the words back.
                RecordDeletion(element, paragraph: null);
                return;

            case "bookmarkStart":
                RecordBookmarkStart(element, paragraph: null);
                return;

            case "bookmarkEnd":
                RecordBookmarkEnd(element, paragraph: null);
                return;

            case "sectPr" or "proofErr"
                 or "permStart" or "permEnd" or "commentRangeStart" or "commentRangeEnd":
                return;

            case "altChunk":
                // An embedded foreign document — HTML, RTF, another DOCX — imported by
                // reference. Reading it means running another reader over another part, which
                // is worth doing but is not this walk's job.
                _diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2120",
                    "The document embeds an external chunk (w:altChunk), whose content is not "
                    + "extracted yet."));
                return;

            default:
                return;
        }
    }

    private ContentParagraph ReadParagraph(XElement paragraph)
    {
        XElement? properties = Word.Child(paragraph, "pPr");
        string? styleId = Word.Value(properties, "pStyle");

        int? headingLevel = HeadingLevelOf(properties, styleId);
        (int? listLevel, string? marker) = ResolveNumbering(properties, styleId, headingLevel);

        ContentParagraph result = new()
        {
            StyleName = _styles.DisplayName(styleId, WordStyleType.Paragraph),
            HeadingLevel = headingLevel,
            ListLevel = listLevel,
            ListMarker = marker,
        };

        _marks.OpenParagraph();
        ReadRuns(paragraph, result, styleId, hyperlink: null);
        FlushPendingRun(result);
        _marks.CloseParagraph(CurrentText(result));
        return result;
    }

    /// <summary>
    /// The outline level of a paragraph, or null when it is body text.
    /// </summary>
    /// <remarks>
    /// <c>w:outlineLvl</c> is zero-based and lives in the paragraph properties or, far more
    /// often, in the heading style — so it has to be resolved through the style chain. Level 9
    /// is WordprocessingML's "body text" value and is not a heading.
    /// </remarks>
    private int? HeadingLevelOf(XElement? properties, string? styleId)
    {
        WordProperty outline = _styles.ResolveParagraphProperty("outlineLvl", properties, styleId);
        int? level = outline.IntegerValue;
        return level is >= 0 and <= 8 ? level + 1 : null;
    }

    /// <summary>
    /// The list level and rendered marker for a paragraph, advancing the list counters.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The numbering reference may be direct on the paragraph or inherited from its style, and
    /// both matter: a list paragraph names its list directly, while a numbered heading gets it
    /// from the heading style. A <c>w:numId</c> of zero means "not numbered" and is how Word
    /// writes a continuation paragraph inside a list item.
    /// </para>
    /// <para>
    /// A heading gets its marker — the chapter number is visible text — but no list level, since
    /// its depth is already reported as a heading level and reporting it twice would make a
    /// heading look like a list item.
    /// </para>
    /// </remarks>
    private (int? Level, string? Marker) ResolveNumbering(
        XElement? properties, string? styleId, int? headingLevel)
    {
        XElement? numberingProperties = Word.Child(properties, "numPr");
        string? numId = Word.Value(numberingProperties, "numId");
        int level = int.TryParse(Word.Value(numberingProperties, "ilvl"), out int parsed) ? parsed : 0;

        if (numId is null && styleId is not null)
        {
            WordProperty fromStyle = _styles.ResolveInStyleChain(
                styleId, WordStyleType.Paragraph, runProperty: false, "numPr");
            if (fromStyle.HasValue)
            {
                numId = Word.Value(fromStyle.Element, "numId");
                level = int.TryParse(Word.Value(fromStyle.Element, "ilvl"), out int fromStyleLevel)
                    ? fromStyleLevel
                    : 0;
            }
            else if (_numbering.FindInstanceForStyle(styleId) is { } styleLinked)
            {
                // A style-linked list names the style rather than the other way round, which is
                // how heading numbering is usually written.
                numId = styleLinked;
                level = headingLevel is { } heading ? heading - 1 : 0;
            }
        }

        if (numId is null or "0") return (null, null);

        string? marker = _numbering.Advance(numId, Math.Clamp(level, 0, WordNumbering.LevelCount - 1));
        return (headingLevel is null ? level : null, marker);
    }

    // ------------------------------------------------------------------------ inline level

    private void ReadRuns(
        XElement container, ContentParagraph paragraph, string? paragraphStyleId, string? hyperlink)
    {
        if (!EnterDepth()) return;

        foreach (XElement child in container.Elements())
        {
            if (child.Name.NamespaceName != OoxmlNamespaces.WordprocessingML) continue;

            switch (child.Name.LocalName)
            {
                case "r":
                    ReadRun(child, paragraph, paragraphStyleId, hyperlink);
                    break;

                case "hyperlink":
                    ReadRuns(child, paragraph, paragraphStyleId, HyperlinkTarget(child) ?? hyperlink);
                    break;

                // Wrappers around runs: content controls, smart tags, inserted and moved-to
                // tracked changes, bidirectional overrides.
                case "sdt":
                    if (Word.Child(child, "sdtContent") is { } sdtContent)
                        ReadRuns(sdtContent, paragraph, paragraphStyleId, hyperlink);
                    break;

                case "smartTag" or "bdo" or "dir":
                    ReadRuns(child, paragraph, paragraphStyleId, hyperlink);
                    break;

                case "ins" or "moveTo":
                {
                    XElement wrapper = child;
                    RecordInsertion(
                        wrapper,
                        paragraph,
                        () => ReadRuns(wrapper, paragraph, paragraphStyleId, hyperlink));
                    break;
                }

                // A field whose result is stored inline rather than between fldChar markers.
                case "fldSimple":
                    ReadSimpleField(child, paragraph, paragraphStyleId, hyperlink);
                    break;

                case "del" or "moveFrom":
                    // Deleted text. Present in the file, absent from the document — and recorded,
                    // because the record is the only place the words survive.
                    RecordDeletion(child, paragraph);
                    break;

                case "bookmarkStart":
                    RecordBookmarkStart(child, paragraph);
                    break;

                case "bookmarkEnd":
                    RecordBookmarkEnd(child, paragraph);
                    break;

                case "subDoc":
                    // A master document's reference to a subdocument: another file entirely.
                    break;

                default:
                    // pPr, bookmarks, proofing marks, comment range markers, permissions.
                    break;
            }
        }

        _depth--;
    }

    private void ReadRun(
        XElement run, ContentParagraph paragraph, string? paragraphStyleId, string? hyperlink)
    {
        XElement? runProperties = Word.Child(run, "rPr");
        WordCharacterFormat format = WordCharacterFormat.Resolve(_styles, runProperties, paragraphStyleId);

        // Hidden text is not displayed by any reader, so extracting it would inject text the
        // document does not show. Unlike a header or a footnote — which a reader does see and
        // the reference filter merely drops — this is genuinely invisible.
        if (format.IsHidden) return;

        string? characterStyleName = _styles.DisplayName(
            Word.Value(runProperties, "rStyle"), WordStyleType.Character);

        foreach (XElement child in run.Elements())
        {
            if (child.Name.NamespaceName != OoxmlNamespaces.WordprocessingML)
            {
                // The DrawingML a w:drawing wraps lives in another namespace.
                continue;
            }

            switch (child.Name.LocalName)
            {
                case "t":
                    // xml:space="preserve" is honoured by keeping the value verbatim: unlike
                    // ODF, WordprocessingML does not collapse white space, and w:t is exact.
                    Emit(paragraph, child.Value, format, characterStyleName, hyperlink);
                    break;

                case "tab":
                    Emit(paragraph, "\t", format, characterStyleName, hyperlink);
                    break;

                case "cr":
                    Emit(paragraph, "\n", format, characterStyleName, hyperlink);
                    break;

                case "br":
                    // Only a text-wrapping break — the default — is a line break in the text. A
                    // page or column break moves the following content elsewhere without
                    // breaking the line, so emitting a newline for it would insert one the
                    // document does not have.
                    if (Word.Attribute(child, "type") is null or "textWrapping")
                        Emit(paragraph, "\n", format, characterStyleName, hyperlink);
                    break;

                case "noBreakHyphen":
                    Emit(paragraph, "‑", format, characterStyleName, hyperlink);
                    break;

                case "softHyphen":
                    // A discretionary hyphen is only drawn when the line breaks there, so it
                    // contributes nothing to the text.
                    break;

                case "sym":
                    EmitSymbol(child, paragraph, format, characterStyleName, hyperlink);
                    break;

                case "instrText" or "delInstrText":
                    // The field's code, never its result — but collected, because the code is half
                    // of what a field is and a hyperlink's target is in nothing else.
                    if (_fieldFrames.Count > 0) _fieldFrames[^1].Instruction.Append(child.Value);
                    break;

                case "delText":
                    // Text a tracked change removed. Its own w:del wrapper records it.
                    break;

                case "fldChar":
                    HandleFieldCharacter(child, paragraph);
                    break;

                case "footnoteReference":
                    HoistNote(child, paragraph, format, characterStyleName, hyperlink, endnote: false);
                    break;

                case "endnoteReference":
                    HoistNote(child, paragraph, format, characterStyleName, hyperlink, endnote: true);
                    break;

                case "commentReference":
                    HoistComment(Word.Attribute(child, "id"));
                    break;

                case "footnoteRef" or "endnoteRef":
                    // The auto-number placeholder inside a note's own body. The number is
                    // supplied when the note is hoisted, so emitting it here would double it.
                    break;

                case "drawing" or "pict" or "object":
                    ReadAnchoredContent(child, paragraph);
                    break;

                case "separator" or "continuationSeparator" or "lastRenderedPageBreak" or "ptab":
                    break;

                default:
                    break;
            }
        }
    }

    /// <summary>
    /// Emits a <c>w:sym</c>: a character named by code point in a symbol font.
    /// </summary>
    /// <remarks>
    /// The code point is usually in a Private Use Area, because that is where Symbol and
    /// Wingdings put their glyphs. Such a code point means nothing outside that font, so a
    /// bullet-shaped one is normalised to a real bullet rather than passed through as
    /// uninterpretable text.
    /// </remarks>
    private void EmitSymbol(
        XElement symbol,
        ContentParagraph paragraph,
        WordCharacterFormat format,
        string? characterStyleName,
        string? hyperlink)
    {
        string? code = Word.Attribute(symbol, "char");
        if (code is null) return;
        if (!int.TryParse(code, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out int value))
            return;
        if (value is <= 0 or > 0x10FFFF || value is >= 0xD800 and <= 0xDFFF) return;

        string text = OutlineNumbers.NormaliseBullet(char.ConvertFromUtf32(value));
        Emit(paragraph, text, format, characterStyleName, hyperlink);
    }

    /// <summary>
    /// Tracks the field state machine so that a field's code is skipped and its result kept.
    /// </summary>
    /// <remarks>
    /// A field is not nested markup: it is a flat sequence of <c>begin</c>, the instruction,
    /// <c>separate</c>, the cached result, and <c>end</c>. Fields do nest, so the state is a
    /// stack, and text is emitted only when no enclosing field is still in its instruction half.
    /// </remarks>
    private void HandleFieldCharacter(XElement fieldCharacter, ContentParagraph paragraph)
    {
        switch (Word.Attribute(fieldCharacter, "fldCharType"))
        {
            case "begin":
                _fieldInstruction.Add(true);
                _fieldFrames.Add(new FieldFrame());
                break;

            case "separate":
                if (_fieldInstruction.Count > 0) _fieldInstruction[^1] = false;
                if (_fieldFrames.Count > 0)
                {
                    FieldFrame frame = _fieldFrames[^1];
                    frame.ResultOffset = CurrentOffset(paragraph);
                    frame.ResultStart = _marks.At(frame.ResultOffset);
                }
                break;

            case "end":
                if (_fieldInstruction.Count > 0) _fieldInstruction.RemoveAt(_fieldInstruction.Count - 1);
                if (_fieldFrames.Count > 0)
                {
                    FieldFrame frame = _fieldFrames[^1];
                    _fieldFrames.RemoveAt(_fieldFrames.Count - 1);

                    // A field with no separator has no cached result at all — it is all
                    // instruction — so its range collapses onto its end rather than covering the
                    // paragraph from offset zero, which is what an unset result offset would mean.
                    int end = CurrentOffset(paragraph);
                    _marks.AddField(
                        frame.Instruction.ToString(),
                        frame.ResultStart is null ? null : Slice(paragraph, frame.ResultOffset, end),
                        frame.ResultStart ?? _marks.At(end),
                        _marks.At(end));
                }
                break;
        }
    }

    private bool InFieldInstruction()
    {
        foreach (bool inInstruction in _fieldInstruction)
        {
            if (inInstruction) return true;
        }
        return false;
    }

    private string? HyperlinkTarget(XElement hyperlink)
    {
        // An external link is a relationship; an internal one names a bookmark.
        if (_file.Relationship(Word.RelationshipId(hyperlink)) is { } relationship)
        {
            string target = relationship.Target;
            if (Word.Attribute(hyperlink, "anchor") is { Length: > 0 } fragment)
                target += "#" + fragment;
            return target;
        }

        return Word.Attribute(hyperlink, "anchor") is { Length: > 0 } anchor ? "#" + anchor : null;
    }

    // ------------------------------------------------------------------------------- runs

    private void Emit(
        ContentParagraph paragraph,
        string text,
        WordCharacterFormat format,
        string? styleName,
        string? hyperlink)
    {
        if (text.Length == 0 || InFieldInstruction()) return;

        if (_pendingFormat is not null
            && (_pendingFormat != format
                || !string.Equals(_pendingHyperlink, hyperlink, StringComparison.Ordinal)
                || !string.Equals(_pendingStyleName, styleName, StringComparison.Ordinal)))
        {
            FlushPendingRun(paragraph);
        }

        _pendingFormat = format;
        _pendingHyperlink = hyperlink;
        _pendingStyleName = styleName;
        _pendingText.Append(text);
    }

    private void FlushPendingRun(ContentParagraph paragraph)
    {
        if (_pendingText.Length > 0)
        {
            WordCharacterFormat format = _pendingFormat ?? WordCharacterFormat.None;
            paragraph.Children.Add(new ContentRun
            {
                Text = _pendingText.ToString(),
                StyleName = _pendingStyleName,
                Language = format.Language,
                Emphasis = format.Emphasis,
                HyperlinkTarget = _pendingHyperlink,
            });
        }
        _pendingText.Clear();
        _pendingFormat = null;
        _pendingHyperlink = null;
        _pendingStyleName = null;
    }

    // ------------------------------------------------------------- notes and annotations

    /// <summary>
    /// Emits a note's number inline and hoists its body into its own section.
    /// </summary>
    /// <remarks>
    /// Unlike ODF, a DOCX caches no note number: the file records only the reference and the
    /// body, and Word computes the number when it lays the document out. So it is counted here,
    /// in reference order — which is what a reader sees, and what makes the extracted text match
    /// LibreOffice's.
    /// </remarks>
    private void HoistNote(
        XElement reference,
        ContentParagraph paragraph,
        WordCharacterFormat format,
        string? characterStyleName,
        string? hyperlink,
        bool endnote)
    {
        string? id = Word.Attribute(reference, "id");
        IReadOnlyDictionary<string, XElement> notes = endnote ? _file.Endnotes : _file.Footnotes;
        if (id is null || !notes.TryGetValue(id, out XElement? body)) return;

        WordNoteNumbering numbering = endnote ? _file.EndnoteNumbering : _file.FootnoteNumbering;
        int ordinal = endnote ? _endnoteNumber++ : _footnoteNumber++;
        string citation = WordNumbering.FormatNumber(numbering.Start + ordinal, numbering.Format);
        Emit(paragraph, citation, format, characterStyleName, hyperlink);

        ContentSection section = new()
        {
            Kind = SectionKind.Note,
            Index = _hoisted.Count,
            Name = citation,
        };

        ReadingState state = SuspendReading();
        ReadBlocks(body, section);
        ResumeReading(state);

        _hoisted.Add(section);
    }

    private void HoistComment(string? id)
    {
        if (id is null || !_file.Comments.TryGetValue(id, out XElement? comment)) return;

        ContentSection section = new()
        {
            Kind = SectionKind.Comment,
            Index = _hoisted.Count,
            Name = Word.Attribute(comment, "author"),
        };

        ReadingState state = SuspendReading();
        ReadBlocks(comment, section);
        ResumeReading(state);

        _hoisted.Add(section);
    }

    /// <summary>
    /// The reading state that belongs to one block context: the half-built run and the field
    /// stack.
    /// </summary>
    private readonly record struct ReadingState(
        string PendingText,
        WordCharacterFormat? PendingFormat,
        string? PendingHyperlink,
        string? PendingStyleName,
        bool[] FieldInstruction,
        FieldFrame[] FieldFrames);

    /// <summary>
    /// Puts the current block context aside so a note, comment or text box can be read as an
    /// independent one, and the interrupted paragraph can carry on afterwards.
    /// </summary>
    private ReadingState SuspendReading()
    {
        ReadingState state = new(
            _pendingText.ToString(), _pendingFormat, _pendingHyperlink, _pendingStyleName,
            [.. _fieldInstruction], [.. _fieldFrames]);

        _pendingText.Clear();
        _pendingFormat = null;
        _pendingHyperlink = null;
        _pendingStyleName = null;
        // A field cannot span flows, so the nested flow starts outside any field.
        _fieldInstruction.Clear();
        _fieldFrames.Clear();
        return state;
    }

    private void ResumeReading(ReadingState state)
    {
        _pendingText.Clear();
        _pendingText.Append(state.PendingText);
        _pendingFormat = state.PendingFormat;
        _pendingHyperlink = state.PendingHyperlink;
        _pendingStyleName = state.PendingStyleName;
        _fieldInstruction.Clear();
        _fieldInstruction.AddRange(state.FieldInstruction);
        _fieldFrames.Clear();
        _fieldFrames.AddRange(state.FieldFrames);
    }

    // ------------------------------------------------------------------ drawings and shapes

    /// <summary>
    /// Reads a <c>w:drawing</c>, <c>w:pict</c> or <c>w:object</c> anchored in a run.
    /// </summary>
    /// <remarks>
    /// An image belongs in the paragraph it sits in. A text box does not: it holds its own
    /// paragraphs, and splicing them into the anchoring paragraph would join two unrelated
    /// sentences and split that paragraph in two at the anchor point. So anything with text
    /// becomes a <see cref="SectionKind.Frame"/> section instead.
    /// </remarks>
    private void ReadAnchoredContent(XElement drawing, ContentParagraph paragraph)
    {
        if (!EnterDepth()) return;

        ContentSection frame = new()
        {
            Kind = SectionKind.Frame,
            Index = _hoisted.Count,
            Name = DrawingName(drawing),
        };

        ReadingState state = SuspendReading();

        // Text bodies first: both DrawingML shapes and VML shapes wrap them in w:txbxContent, so
        // one search finds either.
        foreach (XElement textBody in drawing.Descendants(Word.Name("txbxContent")))
            ReadBlocks(textBody, frame);

        List<ContentImage> images = [.. ReadImages(drawing)];
        ResumeReading(state);

        if (frame.Children.Count > 0) _hoisted.Add(frame);

        if (images.Count > 0)
        {
            // Flushed first, or the image would be placed before the text that precedes it.
            FlushPendingRun(paragraph);
            foreach (ContentImage image in images) paragraph.Children.Add(image);
        }
    }

    /// <summary>
    /// The images a drawing contains: DrawingML <c>a:blip</c> references and VML
    /// <c>v:imagedata</c> ones.
    /// </summary>
    private IEnumerable<ContentImage> ReadImages(XElement drawing)
    {
        string? description = DrawingDescription(drawing);
        string? name = DrawingName(drawing);

        foreach (XElement blip in drawing.Descendants(XName.Get("blip", OoxmlNamespaces.DrawingML)))
        {
            // r:embed names a part in the package; r:link points outside it, and Paperless never
            // follows an external reference.
            string? embedId = blip.Attribute(XName.Get("embed", OoxmlNamespaces.Relationships))?.Value;
            yield return new ContentImage
            {
                AlternativeText = description ?? name,
                MediaType = PartMediaType(embedId),
                PartName = PartName(embedId),
            };
        }

        foreach (XElement imageData in drawing.Descendants(XName.Get("imagedata", OoxmlNamespaces.Vml)))
        {
            string? id = Word.RelationshipId(imageData);
            yield return new ContentImage
            {
                AlternativeText = imageData.Attribute("title")?.Value ?? description ?? name,
                MediaType = PartMediaType(id),
                PartName = PartName(id),
            };
        }
    }

    private static string? DrawingName(XElement drawing)
    {
        // wp:docPr carries the name and description a user typed; VML shapes use attributes.
        XElement? properties = drawing
            .Descendants(XName.Get("docPr", OoxmlNamespaces.DrawingMLWordprocessing))
            .FirstOrDefault();
        return properties?.Attribute("name")?.Value
               ?? drawing.Descendants(XName.Get("shape", OoxmlNamespaces.Vml))
                         .FirstOrDefault()?.Attribute("ID")?.Value;
    }

    private static string? DrawingDescription(XElement drawing)
        => drawing.Descendants(XName.Get("docPr", OoxmlNamespaces.DrawingMLWordprocessing))
                  .FirstOrDefault()?.Attribute("descr")?.Value;

    private string? PartName(string? relationshipId)
        => _file.Relationship(relationshipId) is { IsExternal: false } relationship
            ? relationship.Target
            : null;

    private string? PartMediaType(string? relationshipId)
        => PartName(relationshipId) is { } part ? _file.Package.GetPart(part)?.MediaType : null;

    // ------------------------------------------------------------------------------ helpers

    private bool EnterDepth()
    {
        if (_depth < MaxNestingDepth)
        {
            _depth++;
            return true;
        }

        if (!_reportedDepthLimit)
        {
            _reportedDepthLimit = true;
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2121",
                $"Content is nested more than {MaxNestingDepth} levels deep; the deeper content "
                + "has been skipped."));
        }
        return false;
    }
}
