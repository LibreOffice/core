using System.Text;
using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.OpenDocument.Styles;

namespace Paperless.OpenDocument;

/// <summary>
/// Walks ODF text content into the format-independent content tree.
/// </summary>
/// <remarks>
/// <para>
/// ODF's text content model — paragraphs, spans, lists, tables, frames — is shared by all
/// three applications, which is why this lives in the shared library rather than in the
/// word-processing one. LibreOffice arranges it the same way: one
/// <c>XMLTextImportHelper</c> serves Writer, Calc and Impress
/// (<c>dotnet/research/02-writer.md</c> section D), because a paragraph inside a
/// spreadsheet cell and a paragraph inside a slide's text box are the same construct.
/// </para>
/// <para>
/// Two decisions worth knowing about before reading the code:
/// </para>
/// <para>
/// <strong>Unknown inline elements are recursed into, not skipped.</strong> Nearly every
/// ODF field — page number, date, cross-reference, variable — stores its last computed
/// result as its element content, so recursing gives the cached result for free and matches
/// what a reference renderer displays. The handful of elements where that would produce
/// text no reader ever sees (deleted text inside <c>text:tracked-changes</c>, index
/// templates, declaration blocks) are skipped explicitly.
/// </para>
/// <para>
/// <strong>Notes and comments are hoisted, not inlined.</strong> A footnote's body is not
/// part of the paragraph that references it, and splicing it in mid-sentence would corrupt
/// the paragraph's text. Each becomes its own <see cref="ContentSection"/> appended after
/// the content that referenced it.
/// </para>
/// </remarks>
public sealed partial class OdfContentReader
{
    /// <summary>
    /// How deeply nesting is followed before the reader gives up on a branch.
    /// </summary>
    /// <remarks>
    /// Real documents nest a few dozen levels at most — a table in a frame in a cell in a
    /// section. A hostile or corrupt file can nest arbitrarily, and this is untrusted input
    /// parsed with recursion, so the depth is bounded rather than trusted.
    /// </remarks>
    public const int MaxNestingDepth = 96;

    private readonly OdfFile _file;
    private readonly OdfStyles _styles;
    private readonly List<Diagnostic> _diagnostics;
    private readonly List<ContentNode> _hoisted = [];
    private readonly List<OdfStyleReference> _cascade = [];
    private readonly Dictionary<string, OdfTextFormat> _formatCache = new(StringComparer.Ordinal);

    // Pending-run state. Text is buffered so that adjacent characters sharing formatting
    // become one ContentRun rather than one per XML text node, which a document with heavy
    // field or bookmark use would otherwise fragment badly.
    private readonly StringBuilder _pendingText = new();
    private OdfTextFormat? _pendingFormat;
    private string? _pendingHyperlink;
    private string? _pendingStyleName;

    // White-space collapsing state, per paragraph.
    private bool _atBlockStart = true;
    private bool _lastWasSpace;

    private readonly List<int> _listCounters = [];
    private OdfListStyle? _currentListStyle;
    private int _listLevel;
    private int _depth;
    private bool _reportedDepthLimit;

    /// <summary>Creates a reader over one open document.</summary>
    /// <param name="file">The document being read; used for styles and for image parts.</param>
    /// <param name="diagnostics">Receives problems found while reading.</param>
    public OdfContentReader(OdfFile file, List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(diagnostics);
        _file = file;
        _styles = file.Styles;
        _diagnostics = diagnostics;
    }

    /// <summary>
    /// Reads the block-level content of a container — a body, a section, a table cell, a
    /// slide — appending it to <paramref name="target"/>.
    /// </summary>
    public void ReadBlocks(XElement container, ContentNode target)
    {
        ArgumentNullException.ThrowIfNull(container);
        ArgumentNullException.ThrowIfNull(target);

        if (!EnterDepth()) return;
        foreach (XElement child in container.Elements()) ReadBlock(child, target);
        _depth--;
    }

    /// <summary>
    /// Removes and returns the note and comment sections collected so far.
    /// </summary>
    /// <remarks>
    /// Called by the format readers once a top-level section's blocks are in place, so the
    /// notes belonging to it land immediately after it rather than at the end of the
    /// document.
    /// </remarks>
    public List<ContentNode> TakeHoisted()
    {
        List<ContentNode> hoisted = [.. _hoisted];
        _hoisted.Clear();
        return hoisted;
    }

    /// <summary>
    /// True when a slide is marked as skipped during a presentation.
    /// </summary>
    /// <remarks>
    /// The flag lives on the slide's drawing-page style rather than on the slide, so it has to
    /// be resolved through the style chain — a slide whose style inherits the hidden flag from
    /// its parent is hidden too.
    /// </remarks>
    public bool IsDrawingPageHidden(XElement page)
    {
        ArgumentNullException.ThrowIfNull(page);
        return _styles.ResolveProperty(
            Attribute(page, OdfNamespaces.Draw, "style-name"),
            OdfStyleFamily.DrawingPage,
            OdfPropertyKind.DrawingPage,
            OdfNamespaces.Presentation,
            "visibility").Is("hidden");
    }

    // ----------------------------------------------------------------------- block level

    private void ReadBlock(XElement element, ContentNode target)
    {
        string ns = element.Name.NamespaceName;
        string name = element.Name.LocalName;

        if (ns == OdfNamespaces.Text)
        {
            switch (name)
            {
                case "p" or "h":
                    target.Children.Add(ReadParagraph(element, listLevel: null, listMarker: null));
                    return;

                case "list":
                    ReadList(element, target);
                    return;

                case "numbered-paragraph":
                    ReadNumberedParagraph(element, target);
                    return;

                case "section":
                    ReadBlocks(element, target);
                    return;

                // Indexes and tables of contents keep their generated text in an index body.
                // That text is what a reader sees, so it is content; the *source* and
                // *template* children describe how to regenerate it and are not.
                case "table-of-content" or "illustration-index" or "table-index"
                     or "object-index" or "user-index" or "alphabetical-index"
                     or "bibliography":
                    XElement? body = element.Element(XName.Get("index-body", OdfNamespaces.Text));
                    if (body is not null) ReadBlocks(body, target);
                    return;

                // Deliberately skipped: declarations carry no visible text, and
                // text:tracked-changes holds *deleted* text that no reader displays.
                case "tracked-changes" or "sequence-decls" or "variable-decls"
                     or "user-field-decls" or "dde-connection-decls" or "soft-page-break"
                     or "alphabetical-index-auto-mark-file" or "notes-configuration"
                     or "linenumbering-configuration" or "bibliography-configuration"
                     or "change" or "change-start" or "change-end":
                    return;
            }
        }
        else if (ns == OdfNamespaces.Table)
        {
            if (name == "table") { ReadTable(element, target); return; }

            // Everything else at table level in a spreadsheet body — named expressions,
            // database ranges, data pilot tables, calculation settings — is not content.
            return;
        }
        else if (ns is OdfNamespaces.Draw or OdfNamespaces.Dr3d)
        {
            ReadShape(element, target);
            return;
        }
        else if (ns == OdfNamespaces.Office)
        {
            switch (name)
            {
                case "annotation":
                    HoistAnnotation(element);
                    return;
                case "forms":
                    // Control definitions. Their captions are rendered by the control, not
                    // as document text, and their values duplicate cell content.
                    return;
            }
        }
        else if (ns == OdfNamespaces.Presentation)
        {
            // presentation:notes and presentation:settings are handled by the presentation
            // reader, which decides where notes belong in the section order.
            return;
        }

        // Anything unrecognised may still be a wrapper around real content — LibreOffice's
        // loext: extensions are commonly exactly that — so recurse rather than drop it.
        if (element.HasElements) ReadBlocks(element, target);
    }

    private ContentParagraph ReadParagraph(XElement element, int? listLevel, string? listMarker)
    {
        string? styleName = Attribute(element, OdfNamespaces.Text, "style-name");

        ContentParagraph paragraph = new()
        {
            StyleName = DisplayStyleName(styleName, OdfStyleFamily.Paragraph),
            HeadingLevel = HeadingLevelOf(element, styleName),
            ListLevel = listLevel,
            ListMarker = listMarker,
        };

        int cascadeDepth = _cascade.Count;
        _cascade.Add(new OdfStyleReference(styleName, OdfStyleFamily.Paragraph));

        _atBlockStart = true;
        _lastWasSpace = false;
        ReadInline(element, paragraph, hyperlink: null);
        FlushPendingRun(paragraph);

        _cascade.RemoveRange(cascadeDepth, _cascade.Count - cascadeDepth);
        return paragraph;
    }

    /// <summary>
    /// The outline level of a paragraph, or null when it is body text.
    /// </summary>
    /// <remarks>
    /// A heading is normally <c>text:h</c> with an explicit <c>text:outline-level</c>. But a
    /// paragraph style may itself declare <c>style:default-outline-level</c>, which makes
    /// <c>text:p</c> paragraphs in that style headings too — so the style chain is consulted
    /// as well, or such headings would be reported as ordinary text.
    /// </remarks>
    private int? HeadingLevelOf(XElement element, string? styleName)
    {
        if (element.Name.LocalName == "h" && element.Name.NamespaceName == OdfNamespaces.Text)
            return OdfValue.ParseInt(Attribute(element, OdfNamespaces.Text, "outline-level")) ?? 1;

        OdfStyle? style = _styles.Find(styleName, OdfStyleFamily.Paragraph);
        for (int depth = 0; style is not null && depth < OdfStyles.MaxParentChainDepth; depth++)
        {
            if (style.DefaultOutlineLevel is { } level and > 0) return level;
            style = _styles.Find(style.ParentStyleName, OdfStyleFamily.Paragraph);
        }
        return null;
    }

    // ------------------------------------------------------------------------ inline level

    private void ReadInline(XElement element, ContentParagraph paragraph, string? hyperlink)
    {
        if (!EnterDepth()) return;

        foreach (XNode node in element.Nodes())
        {
            if (node is XText literal) { AppendCollapsed(paragraph, literal.Value, hyperlink); continue; }
            if (node is not XElement child) continue;

            string ns = child.Name.NamespaceName;
            string name = child.Name.LocalName;

            if (ns == OdfNamespaces.Text)
            {
                switch (name)
                {
                    case "span":
                        ReadStyledInline(child, paragraph, hyperlink, OdfStyleFamily.Text);
                        continue;

                    case "a":
                        // xlink:href on the anchor wins over any enclosing one; a nested
                        // anchor without an href keeps the outer target.
                        ReadStyledInline(child, paragraph,
                                         Attribute(child, OdfNamespaces.XLink, "href") ?? hyperlink,
                                         OdfStyleFamily.Text);
                        continue;

                    case "s":
                        // text:c is the count; a missing count means exactly one space.
                        int spaces = Math.Clamp(
                            OdfValue.ParseInt(Attribute(child, OdfNamespaces.Text, "c")) ?? 1, 0, 4096);
                        AppendLiteral(paragraph, new string(' ', spaces), hyperlink,
                                      producesSpace: true);
                        continue;

                    case "tab":
                        AppendLiteral(paragraph, "\t", hyperlink, producesSpace: true);
                        continue;

                    case "line-break":
                        AppendLiteral(paragraph, "\n", hyperlink, producesSpace: true);
                        continue;

                    case "note":
                        HoistNote(child, paragraph, hyperlink);
                        continue;

                    // The generated list label, cached by the writer. ListMarker already
                    // carries it, so emitting it here would duplicate every number.
                    case "number":
                        continue;

                    case "ruby":
                        // The base text is the content; the ruby annotation above it is a
                        // gloss, and inlining it would interleave two readings.
                        XElement? rubyBase = child.Element(XName.Get("ruby-base", OdfNamespaces.Text));
                        if (rubyBase is not null) ReadInline(rubyBase, paragraph, hyperlink);
                        continue;

                    case "bookmark" or "bookmark-start" or "bookmark-end"
                         or "reference-mark" or "reference-mark-start" or "reference-mark-end"
                         or "soft-page-break" or "change" or "change-start" or "change-end"
                         or "alphabetical-index-mark" or "alphabetical-index-mark-start"
                         or "alphabetical-index-mark-end" or "toc-mark" or "toc-mark-start"
                         or "toc-mark-end" or "user-index-mark" or "user-index-mark-start"
                         or "user-index-mark-end":
                        continue;
                }
            }
            else if (ns == OdfNamespaces.Office)
            {
                switch (name)
                {
                    case "annotation":
                        HoistAnnotation(child);
                        continue;
                    case "annotation-end":
                        continue;
                }
            }
            else if (ns is OdfNamespaces.Draw or OdfNamespaces.Dr3d)
            {
                ReadAnchoredShape(child, paragraph);
                continue;
            }

            // Fields, text:meta, loext wrappers: the element content is the cached result.
            ReadInline(child, paragraph, hyperlink);
        }

        _depth--;
    }

    private void ReadStyledInline(
        XElement element, ContentParagraph paragraph, string? hyperlink, OdfStyleFamily family)
    {
        string? styleName = Attribute(element, OdfNamespaces.Text, "style-name");
        if (styleName is null)
        {
            ReadInline(element, paragraph, hyperlink);
            return;
        }

        int cascadeDepth = _cascade.Count;
        _cascade.Add(new OdfStyleReference(styleName, family));
        ReadInline(element, paragraph, hyperlink);
        _cascade.RemoveRange(cascadeDepth, _cascade.Count - cascadeDepth);
    }

    // ------------------------------------------------------------------------------- runs

    /// <summary>
    /// Appends literal text with ODF's white-space collapsing applied.
    /// </summary>
    /// <remarks>
    /// ODF collapses runs of white space to a single space and drops it at the start of a
    /// paragraph, exactly as HTML does — which is why the format has <c>text:s</c> at all.
    /// Skipping the collapse would add spurious spaces to any document that was
    /// pretty-printed, and pretty-printed ODF is common because the format is XML.
    /// </remarks>
    private void AppendCollapsed(ContentParagraph paragraph, string text, string? hyperlink)
    {
        if (text.Length == 0) return;

        StringBuilder collapsed = new(text.Length);
        foreach (char character in text)
        {
            if (character is ' ' or '\t' or '\r' or '\n')
            {
                if (_atBlockStart || _lastWasSpace) continue;
                collapsed.Append(' ');
                _lastWasSpace = true;
            }
            else
            {
                collapsed.Append(character);
                _lastWasSpace = false;
                _atBlockStart = false;
            }
        }

        if (collapsed.Length > 0) Emit(paragraph, collapsed.ToString(), hyperlink);
    }

    /// <summary>
    /// Appends text that is already exactly what the document means, bypassing collapsing.
    /// </summary>
    /// <param name="paragraph">The paragraph being built.</param>
    /// <param name="text">The exact text to append.</param>
    /// <param name="hyperlink">The hyperlink target in force, if any.</param>
    /// <param name="producesSpace">
    /// True when the text ends in white space the document stated explicitly — the spaces of
    /// <c>text:s</c>, a tab, a line break. Whitespace immediately after those is still
    /// collapsed away, so the caller has to say which case it is: marking a footnote
    /// citation as space-producing would swallow the space that follows it.
    /// </param>
    private void AppendLiteral(
        ContentParagraph paragraph, string text, string? hyperlink, bool producesSpace)
    {
        if (text.Length == 0) return;
        Emit(paragraph, text, hyperlink);
        _atBlockStart = false;
        _lastWasSpace = producesSpace;
    }

    private void Emit(ContentParagraph paragraph, string text, string? hyperlink)
    {
        OdfTextFormat format = ResolveFormat();
        string? styleName = InnermostCharacterStyleName();

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
            OdfTextFormat format = _pendingFormat ?? OdfTextFormat.None;
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

    /// <summary>
    /// Resolves the current cascade's character formatting, memoised.
    /// </summary>
    /// <remarks>
    /// A document reuses the same handful of (paragraph style, span style) pairs for
    /// thousands of runs, and each resolution is a dozen dictionary walks up a parent chain.
    /// Caching on the cascade's identity turns that into one lookup.
    /// </remarks>
    private OdfTextFormat ResolveFormat()
    {
        if (_cascade.Count == 0) return OdfTextFormat.None;

        StringBuilder key = new();
        foreach (OdfStyleReference reference in _cascade)
        {
            key.Append((char)('0' + (int)reference.Family)).Append(reference.Name).Append('\u0001');
        }

        string cacheKey = key.ToString();
        if (_formatCache.TryGetValue(cacheKey, out OdfTextFormat? cached)) return cached;

        OdfTextFormat format = OdfTextFormat.Resolve(_styles, _cascade);
        _formatCache[cacheKey] = format;
        return format;
    }

    private string? InnermostCharacterStyleName()
    {
        for (int i = _cascade.Count - 1; i >= 0; i--)
        {
            if (_cascade[i].Family == OdfStyleFamily.Text)
                return DisplayStyleName(_cascade[i].Name, OdfStyleFamily.Text);
        }
        return null;
    }

    // ------------------------------------------------------------------------------ lists

    /// <summary>
    /// Reads a <c>text:list</c> or <c>text:numbered-paragraph</c>, tracking the counters
    /// that produce each item's rendered marker.
    /// </summary>
    /// <remarks>
    /// ODF expresses list nesting in the XML tree rather than as a per-paragraph level
    /// attribute, so the nesting depth is simply how deep this recursion has gone. The
    /// marker text, though, has to be generated: the file records the counters' effect only
    /// as an optional cached <c>text:number</c>, and computing it is what turns a bare
    /// paragraph into "3." or "•".
    /// </remarks>
    private void ReadList(XElement list, ContentNode target)
    {
        if (!EnterDepth()) return;

        int level = _listLevel + 1;
        OdfListStyle? style = ResolveListStyle(list) ?? _currentListStyle;

        // A list restarts numbering unless it explicitly continues an earlier one.
        bool continues = OdfValue.ParseBoolean(Attribute(list, OdfNamespaces.Text, "continue-numbering")) == true
                         || Attribute(list, OdfNamespaces.Text, "continue-list") is not null;
        if (!continues) ResetCounter(level, style);

        OdfListStyle? outerStyle = _currentListStyle;
        int outerLevel = _listLevel;
        _currentListStyle = style ?? outerStyle;
        _listLevel = level;

        foreach (XElement child in list.Elements())
        {
            bool isItem = child.Name.NamespaceName == OdfNamespaces.Text
                          && child.Name.LocalName is "list-item" or "numbered-paragraph";
            bool isHeader = child.Name.NamespaceName == OdfNamespaces.Text
                            && child.Name.LocalName == "list-header";

            if (isItem)
            {
                if (OdfValue.ParseInt(Attribute(child, OdfNamespaces.Text, "start-value")) is { } start)
                    SetCounter(level, start - 1);
                IncrementCounter(level);
                ReadListItem(child, target, level, numbered: true);
            }
            else if (isHeader)
            {
                // A list header is unnumbered introductory text at this level.
                ReadListItem(child, target, level, numbered: false);
            }
            else
            {
                ReadBlock(child, target);
            }
        }

        _listLevel = outerLevel;
        _currentListStyle = outerStyle;
        _depth--;
    }

    private void ReadListItem(XElement item, ContentNode target, int level, bool numbered)
    {
        // Only the first paragraph of an item carries the marker; the rest are continuation
        // paragraphs at the same level, which is how ODF represents a multi-paragraph item.
        bool markerPending = numbered;

        foreach (XElement child in item.Elements())
        {
            bool isParagraph = child.Name.NamespaceName == OdfNamespaces.Text
                               && child.Name.LocalName is "p" or "h";
            if (isParagraph)
            {
                string? marker = markerPending
                    ? _currentListStyle?.FormatLabel(level, _listCounters)
                    : null;
                markerPending = false;

                // ListLevel is zero-based in the content tree; ODF levels start at one.
                target.Children.Add(ReadParagraph(child, level - 1, marker));
            }
            else
            {
                ReadBlock(child, target);
            }
        }
    }

    /// <summary>
    /// Reads a <c>text:numbered-paragraph</c>: a single numbered paragraph that is its own
    /// list, used for chapter-style numbering outside a <c>text:list</c>.
    /// </summary>
    /// <remarks>
    /// It carries its level as an attribute rather than by nesting, so unlike
    /// <see cref="ReadList"/> the depth comes from the file. Numbering continues from any
    /// earlier paragraph at the same level unless the element restarts it, which is what
    /// makes a run of these behave as one list.
    /// </remarks>
    private void ReadNumberedParagraph(XElement element, ContentNode target)
    {
        if (!EnterDepth()) return;

        int level = Math.Clamp(OdfValue.ParseInt(Attribute(element, OdfNamespaces.Text, "level")) ?? 1, 1, 10);
        OdfListStyle? style = _styles.FindListStyle(Attribute(element, OdfNamespaces.Text, "style-name"))
                              ?? _currentListStyle;

        OdfListStyle? outerStyle = _currentListStyle;
        int outerLevel = _listLevel;
        _currentListStyle = style ?? outerStyle;
        _listLevel = level;

        if (OdfValue.ParseInt(Attribute(element, OdfNamespaces.Text, "start-value")) is { } start)
            SetCounter(level, start - 1);
        IncrementCounter(level);
        ReadListItem(element, target, level, numbered: true);

        _listLevel = outerLevel;
        _currentListStyle = outerStyle;
        _depth--;
    }

    /// <summary>
    /// Finds the list style governing a <c>text:list</c>.
    /// </summary>
    /// <remarks>
    /// The list may name one directly. When it does not — which is the common case for
    /// nested lists — the style comes from the paragraph style applied to the items, via
    /// <c>style:list-style-name</c> somewhere up its parent chain. That indirection is how
    /// LibreOffice's "List Bullet" paragraph style carries its bullets.
    /// </remarks>
    private OdfListStyle? ResolveListStyle(XElement list)
    {
        if (_styles.FindListStyle(Attribute(list, OdfNamespaces.Text, "style-name")) is { } direct)
            return direct;

        foreach (XElement item in list.Elements())
        {
            foreach (XElement paragraph in item.Elements())
            {
                if (paragraph.Name.NamespaceName != OdfNamespaces.Text) continue;
                if (paragraph.Name.LocalName is not ("p" or "h")) continue;

                OdfStyle? style = _styles.Find(
                    Attribute(paragraph, OdfNamespaces.Text, "style-name"), OdfStyleFamily.Paragraph);
                for (int depth = 0; style is not null && depth < OdfStyles.MaxParentChainDepth; depth++)
                {
                    if (_styles.FindListStyle(style.ListStyleName) is { } viaStyle) return viaStyle;
                    style = _styles.Find(style.ParentStyleName, OdfStyleFamily.Paragraph);
                }
            }
            break;
        }
        return null;
    }

    private void ResetCounter(int level, OdfListStyle? style)
    {
        EnsureCounters(level);
        _listCounters[level - 1] = (style?.GetLevel(level)?.StartValue ?? 1) - 1;
    }

    private void SetCounter(int level, int value)
    {
        EnsureCounters(level);
        _listCounters[level - 1] = value;
    }

    private void IncrementCounter(int level)
    {
        EnsureCounters(level);
        _listCounters[level - 1]++;
        // Entering a new item resets everything below it, so a second top-level item's
        // sub-list starts from one again.
        for (int deeper = level; deeper < _listCounters.Count; deeper++)
            _listCounters[deeper] = (_currentListStyle?.GetLevel(deeper + 1)?.StartValue ?? 1) - 1;
    }

    private void EnsureCounters(int level)
    {
        while (_listCounters.Count < level) _listCounters.Add(0);
    }

    // ------------------------------------------------------------- notes and annotations

    /// <summary>
    /// Emits a note's citation inline and hoists its body into its own section.
    /// </summary>
    /// <remarks>
    /// The citation — the little superscript number — is genuinely part of the referencing
    /// paragraph's text and is what LibreOffice's own text export emits, so it stays inline.
    /// The cached value in <c>text:note-citation</c> is used rather than a recomputed one: the
    /// numbering depends on the notes configuration, restart points and note class, and the
    /// value the writer cached is by definition the one it displayed.
    /// </remarks>
    private void HoistNote(XElement note, ContentParagraph paragraph, string? hyperlink)
    {
        string? citation = note.Element(XName.Get("note-citation", OdfNamespaces.Text))?.Value;
        XElement? body = note.Element(XName.Get("note-body", OdfNamespaces.Text));

        if (!string.IsNullOrEmpty(citation))
            AppendLiteral(paragraph, citation, hyperlink, producesSpace: false);

        ContentSection section = new()
        {
            Kind = SectionKind.Note,
            Index = _hoisted.Count,
            Name = string.IsNullOrWhiteSpace(citation) ? null : citation.Trim(),
        };

        // Saved and restored because a note's body is a fresh block context: its paragraphs
        // must not inherit the referencing paragraph's white-space or list state.
        ReadingState state = SuspendReading();
        if (body is not null) ReadBlocks(body, section);
        ResumeReading(state);

        _hoisted.Add(section);
    }

    private void HoistAnnotation(XElement annotation)
    {
        string? author = annotation.Element(XName.Get("creator", OdfNamespaces.DublinCore))?.Value;

        ContentSection section = new()
        {
            Kind = SectionKind.Comment,
            Index = _hoisted.Count,
            Name = string.IsNullOrWhiteSpace(author) ? null : author.Trim(),
        };

        ReadingState state = SuspendReading();
        foreach (XElement child in annotation.Elements())
        {
            // dc:creator and dc:date describe the comment rather than being its text.
            if (child.Name.NamespaceName == OdfNamespaces.DublinCore) continue;
            ReadBlock(child, section);
        }
        ResumeReading(state);

        _hoisted.Add(section);
    }

    /// <summary>
    /// The reading state that belongs to one block context: the half-built run, the
    /// white-space collapsing position, and the list nesting.
    /// </summary>
    private readonly record struct ReadingState(
        bool AtBlockStart,
        bool LastWasSpace,
        int ListLevel,
        OdfListStyle? ListStyle,
        string PendingText,
        OdfTextFormat? PendingFormat,
        string? PendingHyperlink,
        string? PendingStyleName);

    /// <summary>
    /// Puts the current block context aside so a hoisted note or comment can be read as an
    /// independent one, and the interrupted paragraph can carry on afterwards.
    /// </summary>
    private ReadingState SuspendReading()
    {
        ReadingState state = new(
            _atBlockStart, _lastWasSpace, _listLevel, _currentListStyle,
            _pendingText.ToString(), _pendingFormat, _pendingHyperlink, _pendingStyleName);

        _pendingText.Clear();
        _pendingFormat = null;
        _pendingHyperlink = null;
        _pendingStyleName = null;
        _listLevel = 0;
        _currentListStyle = null;
        return state;
    }

    private void ResumeReading(ReadingState state)
    {
        _atBlockStart = state.AtBlockStart;
        _lastWasSpace = state.LastWasSpace;
        _listLevel = state.ListLevel;
        _currentListStyle = state.ListStyle;
        _pendingText.Clear();
        _pendingText.Append(state.PendingText);
        _pendingFormat = state.PendingFormat;
        _pendingHyperlink = state.PendingHyperlink;
        _pendingStyleName = state.PendingStyleName;
    }

    // ----------------------------------------------------------------------------- shapes

    /// <summary>
    /// Reads a shape anchored inside a paragraph, deciding whether its content belongs in the
    /// paragraph or in a section of its own.
    /// </summary>
    /// <remarks>
    /// An inline image is part of the sentence it sits in and stays there. A text box is not:
    /// it holds its own paragraphs, and splicing them into the anchoring paragraph would join
    /// two unrelated sentences and break the paragraph in two at the anchor point. So a shape
    /// that turns out to hold text becomes a <see cref="SectionKind.Frame"/> section instead,
    /// hoisted to just after the content that anchors it.
    /// </remarks>
    private void ReadAnchoredShape(XElement shape, ContentParagraph paragraph)
    {
        // Read into a section first: what the shape contains is only knowable afterwards.
        ContentSection frame = new()
        {
            Kind = SectionKind.Frame,
            Index = _hoisted.Count,
            Name = Attribute(shape, OdfNamespaces.Draw, "name"),
        };

        ReadingState state = SuspendReading();
        ReadShape(shape, frame);
        ResumeReading(state);

        if (frame.Children.Count == 0) return;

        if (frame.Children.All(child => child is ContentImage))
        {
            // The pending text has to be flushed first, or the image would be placed before
            // the text that precedes it in the paragraph.
            FlushPendingRun(paragraph);
            foreach (ContentNode child in frame.Children) paragraph.Children.Add(child);
            return;
        }

        _hoisted.Add(frame);
    }

    /// <summary>
    /// Reads a drawing object: an image, a text box, a grouped shape or a shape with text.
    /// </summary>
    /// <remarks>
    /// Extraction records that an image exists and what it is called, and reads the text of
    /// anything that holds text. It does not decode pixels — that is rendering's job — and
    /// it does not re-execute embedded OLE objects.
    /// </remarks>
    private void ReadShape(XElement shape, ContentNode target)
    {
        if (!EnterDepth()) return;

        string name = shape.Name.LocalName;

        // A shape's own style carries the character formatting its text inherits, which for
        // a slide is where nearly all of the formatting lives.
        int cascadeDepth = _cascade.Count;
        if (Attribute(shape, OdfNamespaces.Presentation, "style-name") is { } presentationStyle)
            _cascade.Add(new OdfStyleReference(presentationStyle, OdfStyleFamily.Presentation));
        else if (Attribute(shape, OdfNamespaces.Draw, "style-name") is { } graphicStyle)
            _cascade.Add(new OdfStyleReference(graphicStyle, OdfStyleFamily.Graphic));
        if (Attribute(shape, OdfNamespaces.Draw, "text-style-name") is { } textStyle)
            _cascade.Add(new OdfStyleReference(textStyle, OdfStyleFamily.Paragraph));

        switch (name)
        {
            case "frame":
                ReadFrame(shape, target);
                break;

            case "image":
                target.Children.Add(ReadImage(shape, shape.Parent));
                break;

            case "g" or "a":
                // A group, or a hyperlinked shape: neither draws anything itself.
                foreach (XElement child in shape.Elements()) ReadShape(child, target);
                break;

            case "page-thumbnail" or "object" or "object-ole" or "applet" or "plugin" or "floating-frame":
                // Placeholders for content Paperless does not read during extraction: a
                // slide preview, or an embedded object whose own document would have to be
                // opened. Recorded as a graphic so the caller knows something is there.
                target.Children.Add(new ContentImage
                {
                    AlternativeText = DescriptionOf(shape) ?? Attribute(shape, OdfNamespaces.Draw, "name"),
                    MediaType = Attribute(shape, OdfNamespaces.Draw, "mime-type"),
                    PartName = NormaliseHref(Attribute(shape, OdfNamespaces.XLink, "href")),
                });
                break;

            default:
                // Every other shape — rectangles, custom shapes, connectors, captions — may
                // carry text directly as paragraph children.
                foreach (XElement child in shape.Elements())
                {
                    if (child.Name.NamespaceName is OdfNamespaces.Draw or OdfNamespaces.Dr3d)
                        ReadShape(child, target);
                    else
                        ReadBlock(child, target);
                }
                break;
        }

        _cascade.RemoveRange(cascadeDepth, _cascade.Count - cascadeDepth);
        _depth--;
    }

    private void ReadFrame(XElement frame, ContentNode target)
    {
        foreach (XElement child in frame.Elements())
        {
            if (child.Name.NamespaceName != OdfNamespaces.Draw)
            {
                // svg:title and svg:desc describe the frame; office:event-listeners and
                // office:binary-data are not text.
                continue;
            }

            switch (child.Name.LocalName)
            {
                case "image":
                    target.Children.Add(ReadImage(child, frame));
                    break;

                case "text-box":
                    ReadBlocks(child, target);
                    break;

                default:
                    ReadShape(child, target);
                    break;
            }
        }
    }

    private ContentImage ReadImage(XElement image, XElement? frame)
    {
        string? href = Attribute(image, OdfNamespaces.XLink, "href");
        string? partName = NormaliseHref(href);

        return new ContentImage
        {
            AlternativeText = (frame is null ? null : DescriptionOf(frame)) ?? DescriptionOf(image),
            MediaType = Attribute(image, OdfNamespaces.Draw, "mime-type")
                        ?? (partName is null ? null : MediaTypeOfPart(partName)),
            PartName = partName,
        };
    }

    /// <summary>
    /// The alternative text for a graphic: <c>svg:desc</c> if present, else
    /// <c>svg:title</c>.
    /// </summary>
    /// <remarks>
    /// ODF keeps both, and they are not interchangeable — the title is a short label and the
    /// description the accessible text — so the description is preferred and the title used
    /// only as a fallback.
    /// </remarks>
    private static string? DescriptionOf(XElement element)
    {
        string? description = element.Element(XName.Get("desc", OdfNamespaces.SvgCompatible))?.Value;
        if (!string.IsNullOrWhiteSpace(description)) return description.Trim();

        string? title = element.Element(XName.Get("title", OdfNamespaces.SvgCompatible))?.Value;
        return string.IsNullOrWhiteSpace(title) ? null : title.Trim();
    }

    /// <summary>
    /// Turns an <c>xlink:href</c> into a package part name, or null for an external
    /// reference.
    /// </summary>
    /// <remarks>
    /// An internal reference is a package-relative path, usually written <c>./Pictures/…</c>
    /// or <c>Pictures/…</c>. Anything with a scheme points outside the package: Paperless
    /// never fetches those, both because extraction must not make network requests and
    /// because doing so on untrusted input is an SSRF.
    /// </remarks>
    private static string? NormaliseHref(string? href)
    {
        if (string.IsNullOrWhiteSpace(href)) return null;
        string trimmed = href.Trim();
        if (trimmed.Contains("://", StringComparison.Ordinal)
            || trimmed.StartsWith("data:", StringComparison.OrdinalIgnoreCase))
            return null;
        if (trimmed.StartsWith("./", StringComparison.Ordinal)) trimmed = trimmed[2..];
        return trimmed.Length == 0 ? null : trimmed;
    }

    private string? MediaTypeOfPart(string partName)
        => _file.Package?.GetPart(partName)?.MediaType;

    // ------------------------------------------------------------------------------ helpers

    /// <summary>
    /// One namespaced attribute's value, or null when the element does not carry it.
    /// </summary>
    /// <remarks>
    /// Public because the family readers need the same lookup for the handful of attributes
    /// they interpret themselves — a slide's name, a sheet's name — and because matching on
    /// namespace rather than prefix is the part that must not be got wrong: a document may use
    /// any prefix it likes for a given namespace.
    /// </remarks>
    public static string? Attribute(XElement element, string ns, string localName)
    {
        ArgumentNullException.ThrowIfNull(element);
        return element.Attribute(XName.Get(localName, ns))?.Value;
    }

    /// <summary>
    /// The style name to report to callers: the user-visible one where the file records it,
    /// otherwise the reference name with ODF's name escaping undone.
    /// </summary>
    private string? DisplayStyleName(string? styleName, OdfStyleFamily family)
    {
        if (styleName is null) return null;
        OdfStyle? style = _styles.Find(styleName, family);
        if (style?.DisplayName is { Length: > 0 } display) return display;

        // An automatic style has no display name of its own; the name a caller cares about
        // is its parent's, since that is the style the user actually applied.
        if (style is { Source: OdfStyleSource.Automatic, ParentStyleName: { Length: > 0 } parentName })
        {
            OdfStyle? parent = _styles.FindNamed(parentName, family);
            return parent?.DisplayName is { Length: > 0 } parentDisplay
                ? parentDisplay
                : OdfNames.Decode(parentName);
        }

        return OdfNames.Decode(styleName);
    }

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
                DiagnosticSeverity.Error, "PL2020",
                $"Content is nested more than {MaxNestingDepth} levels deep; the deeper "
                + "content has been skipped."));
        }
        return false;
    }
}
