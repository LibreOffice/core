using System.Text;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// Turns an ODF text document's body into the paragraphs the paginator takes.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over <c>content.xml</c>, separate from the one that builds the extraction tree, and
/// deliberately so: extraction discards font sizes, indents and spacing because it must not pay for
/// them, and layout needs nothing else. Sharing one walk would mean either extraction carrying
/// formatting it never reads or layout re-deriving it from a tree that no longer has it.
/// </para>
/// <para>
/// The spans are walked as well as the paragraph, because ODF has no inline formatting: one bold word is
/// an automatic style and a <c>text:span</c> pointing at it, so a reader that resolved only the paragraph
/// style would measure a mixed paragraph wholly in its body font — short lines wherever the emphasis is
/// larger, and the wrong face wherever it is bolder. A paragraph whose spans resolve to the paragraph's
/// own formatting carries no runs at all, which keeps plain prose on the cheap single-face path.
/// </para>
/// </remarks>
public sealed partial class OdtLayoutSource
{
    /// <summary>
    /// How many paragraphs are read before the rest are ignored.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input rather than a real limit. A book is tens of thousands of paragraphs;
    /// a generated file can claim millions, and each one costs a shaping pass.
    /// </remarks>
    public const int MaxParagraphs = 200000;

    /// <summary>
    /// The character a <c>text:line-break</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, the Unicode line separator, because UAX #14 gives it a mandatory break and the break
    /// iterator therefore honours it without layout having to special-case anything. A newline would do
    /// the same but would also read as the end of a paragraph to anything that later scans the text,
    /// which a line break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    /// <summary>
    /// The character an anchor occupies.
    /// </summary>
    /// <remarks>
    /// The same one the document model uses, so an offset counted here means the same thing there. It
    /// has a width but no text: a footnote's citation occupies a position in the sentence that cites it
    /// while its body lives in another flow.
    /// </remarks>
    private const char AnchorCharacter = '\u0001';

    private readonly OdfStyles _styles;
    private readonly SystemFontResolver _fonts;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), OpenTypeFace> _faces = [];
    private readonly Dictionary<(string? Family, int Weight, bool Italic), FontReference> _references =
        [];

    /// <summary>
    /// Text styles already resolved, keyed by the cascade that produced them.
    /// </summary>
    /// <remarks>
    /// A document has a handful of span styles and thousands of spans referencing them, and resolving one
    /// walks a parent chain per property. The key is the cascade's names in order, because two spans with
    /// the same style inside different paragraph styles resolve differently.
    /// </remarks>
    private readonly Dictionary<string, OdfTextStyle> _resolved = new(StringComparer.Ordinal);

    /// <summary>Creates a source over a document's styles.</summary>
    /// <param name="styles">The document's resolved styles.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    /// <param name="masterPages">
    /// Which section each master page is, by name. Empty for a document laid out on one section, which
    /// leaves every block in section zero.
    /// </param>
    public OdtLayoutSource(
        OdfStyles styles,
        SystemFontResolver? fonts = null,
        IReadOnlyDictionary<string, int>? masterPages = null)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _masterPages = masterPages ?? new Dictionary<string, int>(StringComparer.Ordinal);
    }

    /// <summary>Which section each master page is, by name.</summary>
    private readonly IReadOnlyDictionary<string, int> _masterPages;

    /// <summary>
    /// The section the walk is in, which a paragraph naming a master page changes.
    /// </summary>
    /// <remarks>
    /// ODF has no section delimiter at all: a paragraph reaches its page description through its paragraph
    /// style's <c>style:master-page-name</c>, and a paragraph naming one <em>starts a page</em> on that
    /// master. Everything after it stays there until another paragraph names a different one — which is why
    /// this is state carried along the walk rather than a property of the paragraph.
    /// </remarks>
    private int _sectionIndex;

    /// <summary>The substitutions made while resolving the document's fonts.</summary>
    /// <remarks>
    /// Worth surfacing rather than swallowing: a substitution that is not metric-compatible changes
    /// every line break after the first paragraph that uses it, so it is the first thing to check when a
    /// comparison against a reference renderer disagrees.
    /// </remarks>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>
    /// Reads the body's blocks — its paragraphs and its tables — in document order.
    /// </summary>
    /// <param name="body">The <c>office:text</c> element.</param>
    public List<PageBlock> Read(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        _sectionIndex = 0;
        List<PageBlock> blocks = [];
        Walk(body, blocks, depth: 0);
        return blocks;
    }

    /// <summary>
    /// Reads a flow's paragraphs only: a header, a footer, or the inside of a cell.
    /// </summary>
    /// <param name="element">The element whose block-level children to read.</param>
    /// <remarks>
    /// Paragraphs only because a flow is laid out into a fixed rectangle by
    /// <see cref="FlowLayouter"/>, which stacks paragraphs and knows nothing of grids. A table inside a
    /// header or inside another table is therefore skipped rather than drawn — recorded as a gap in this
    /// library's TODO, since both are legal and neither is common.
    /// </remarks>
    public List<PageParagraph> ReadFlow(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);

        List<PageParagraph> paragraphs = [];
        Walk(element, paragraphs, depth: 0);
        return paragraphs;
    }

    /// <summary>
    /// Walks the body's block-level children.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Sections, lists and the change-tracking wrappers are transparent: their children are body-level
    /// content and a walk that stopped at them would lose whole chapters of a real document.
    /// </para>
    /// <para>
    /// Generic in what it fills, which is how one walk serves both the body and a flow. A body takes
    /// <see cref="PageBlock"/> and so keeps the tables; a header, a footer or a cell takes
    /// <see cref="PageParagraph"/>, and a table simply does not fit in the list — so it is dropped by the
    /// type rather than by a flag that could be passed the wrong way round.
    /// </para>
    /// <para>
    /// A <c>text:h</c> is a paragraph as far as layout is concerned. Its outline level changes which
    /// style it resolves through, which the style chain already handles.
    /// </para>
    /// </remarks>
    private void Walk<T>(XElement element, List<T> into, int depth)
        where T : PageBlock
    {
        // Deep nesting is legal — a list inside a section inside a change region — but a file can nest
        // indefinitely, and this recurses on untrusted input.
        if (depth > 64 || into.Count >= MaxParagraphs) return;

        foreach (XElement child in element.Elements())
        {
            if (into.Count >= MaxParagraphs) return;

            string name = child.Name.LocalName;
            string ns = child.Name.NamespaceName;

            if (ns == OdfNamespaces.Text && name is "p" or "h")
            {
                if (Paragraph(child) is { } paragraph && paragraph is T block) into.Add(block);
                continue;
            }

            if (ns == OdfNamespaces.Text
                && name is "list" or "list-item" or "list-header" or "section" or "index-body"
                    or "table-of-content" or "alphabetical-index" or "illustration-index"
                    or "table-index" or "object-index" or "user-index" or "bibliography"
                    or "tracked-changes" or "deletion" or "insertion")
            {
                // A deletion's content is inside a change region and is not part of the text a reader
                // sees, so it is skipped rather than walked — the same decision every one of Paperless's
                // extraction readers makes.
                if (name is not ("tracked-changes" or "deletion")) Walk(child, into, depth + 1);
                continue;
            }

            if (ns == OdfNamespaces.Table && name == "table")
            {
                // A table goes in whole, as a grid. It is dropped when the caller is filling a flow's
                // paragraph list, which cannot hold one — a table inside a header or inside another table
                // is legal and not laid out yet.
                if (Table(child) is { } table && table is T grid) into.Add(grid);
                continue;
            }
        }
    }

    /// <summary>
    /// The section a paragraph style's master page names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// Followed up the parent chain, because a document's own styles are usually derived from
    /// <c>Standard</c> and it is the derived style that names the master. An empty name is not the same as
    /// an absent one — ODF writes <c>style:master-page-name=""</c> to mean "explicitly no master page, so
    /// no page break here", which is what an automatic style derived from one that names a master uses to
    /// cancel it. Cycle-guarded, since a style pool read from a file can point at itself.
    /// </remarks>
    private int? MasterPageOf(string? styleName)
    {
        if (_masterPages.Count == 0) return null;

        HashSet<string> seen = new(StringComparer.Ordinal);
        string? name = styleName;

        for (int depth = 0; depth < 64 && name is not null && seen.Add(name); depth++)
        {
            OdfStyle? style = _styles.Find(name, OdfStyleFamily.Paragraph);
            if (style is null) return null;

            if (style.MasterPageName is { } master)
            {
                return master.Length == 0 ? null : _masterPages.GetValueOrDefault(master, 0);
            }

            name = style.ParentStyleName;
        }

        return null;
    }

    private PageParagraph? Paragraph(XElement element)
    {
        string? styleName = element
            .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

        // A paragraph whose style names a master page moves the document onto that master, and everything
        // after it follows until another paragraph says otherwise.
        if (MasterPageOf(styleName) is { } section) _sectionIndex = section;

        OdfTextStyle text = OdfParagraphFormats.ResolveText(_styles, styleName);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        RunWalker walker = new(styleName);
        walker.Walk(element);

        return new PageParagraph
        {
            SectionIndex = _sectionIndex,
            Text = walker.Text,
            Face = face,
            Font = _references.GetValueOrDefault(text.FaceKey),
            Colour = text.Colour ?? Colour.Black,
            Format = OdfParagraphFormats.Resolve(_styles, styleName),
            EmSize = text.Size,
            Language = text.Language,
            Shaping = new ShapingOptions(Language: text.Language),
            Runs = RunsOf(walker.Ranges, text, face),
            Source = element,
        };
    }

    /// <summary>
    /// The paragraph's runs, or nothing when every one of them is the paragraph's own formatting.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Returning an empty list for a uniform paragraph is not just an optimisation: it puts plain prose
    /// back on the single-face measuring path, which shapes the whole paragraph in one call rather than
    /// once per run. A run's boundaries also break shaping context, so a paragraph split into runs it does
    /// not need would lose a kern pair at each boundary and measure very slightly wide.
    /// </para>
    /// <para>
    /// A range whose font cannot be loaded falls back to the paragraph's face rather than being dropped:
    /// its text is still part of the paragraph, and losing it would silently shorten the document.
    /// </para>
    /// </remarks>
    private List<PageRun> RunsOf(
        IReadOnlyList<StyledRange> ranges, OdfTextStyle paragraph, OpenTypeFace paragraphFace)
    {
        List<PageRun> runs = new(ranges.Count);
        bool varies = false;

        foreach (StyledRange range in ranges)
        {
            OdfTextStyle style = range.Cascade.Length <= 1 ? paragraph : Resolve(range.Cascade);
            OpenTypeFace face = Face(style) ?? paragraphFace;

            if (face != paragraphFace
                || style.Size != paragraph.Size
                || style.Colour != paragraph.Colour
                || style.Language != paragraph.Language)
            {
                varies = true;
            }

            runs.Add(new PageRun(
                range.Start,
                range.Length,
                face,
                style.Size,
                _references.GetValueOrDefault(style.FaceKey),
                style.Colour ?? paragraph.Colour ?? Colour.Black,
                new ShapingOptions(Language: style.Language)));
        }

        return varies ? runs : [];
    }

    /// <summary>The text style a cascade resolves to, cached because spans repeat their styles.</summary>
    private OdfTextStyle Resolve(OdfStyleReference[] cascade)
    {
        string key = string.Join('\n', cascade.Select(reference => reference.Name));
        if (_resolved.TryGetValue(key, out OdfTextStyle cached)) return cached;

        OdfTextStyle resolved = OdfParagraphFormats.ResolveText(_styles, cascade);
        _resolved[key] = resolved;
        return resolved;
    }

    /// <summary>
    /// A stretch of a paragraph's text and the style cascade in force over it.
    /// </summary>
    /// <param name="Start">Its first character, as an index into the paragraph's text.</param>
    /// <param name="Length">How many characters it covers.</param>
    /// <param name="Cascade">
    /// The styles in force, outermost first: the paragraph style, then each enclosing span's.
    /// </param>
    private readonly record struct StyledRange(
        int Start, int Length, OdfStyleReference[] Cascade);

    /// <summary>
    /// Walks a paragraph, building its text and the ranges its spans divide it into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One walk for both, because the ranges are offsets into the text and the text is not a
    /// concatenation of the element's text nodes: ODF writes runs of spaces, tabs and line breaks as
    /// elements, so every one of them shifts the offsets of everything after it. Building the text first
    /// and the ranges from the tree afterwards would mean re-deriving those shifts.
    /// </para>
    /// <para>
    /// Adjacent stretches with the same cascade merge, which is what makes a paragraph split by a
    /// bookmark or a soft page break still measure as one run.
    /// </para>
    /// </remarks>
    private sealed class RunWalker
    {
        /// <summary>How deep a paragraph's inline nesting is followed.</summary>
        /// <remarks>
        /// Spans nest legitimately — a bold word inside an italic phrase inside a hyperlink — but a
        /// generated file can nest indefinitely, and this recurses on untrusted input.
        /// </remarks>
        private const int MaxDepth = 64;

        private readonly StringBuilder _builder = new();
        private readonly List<OdfStyleReference> _cascade = [];
        private readonly List<StyledRange> _ranges = [];

        /// <summary>Creates a walker over a paragraph with a given style.</summary>
        /// <param name="paragraphStyleName">The paragraph's own style name, which roots the cascade.</param>
        internal RunWalker(string? paragraphStyleName)
            => _cascade.Add(new OdfStyleReference(paragraphStyleName, OdfStyleFamily.Paragraph));

        /// <summary>The paragraph's text.</summary>
        internal string Text => _builder.ToString();

        /// <summary>The ranges, in order, partitioning the text.</summary>
        internal IReadOnlyList<StyledRange> Ranges => _ranges;

        /// <summary>Walks a <c>text:p</c> or <c>text:h</c>.</summary>
        internal void Walk(XElement paragraph) => Append(paragraph, depth: 0);

        private void Append(XElement element, int depth)
        {
            if (depth > MaxDepth) return;

            foreach (XNode node in element.Nodes())
            {
                switch (node)
                {
                    case XText textNode:
                        Emit(textNode.Value);
                        break;

                    case XElement child when child.Name.NamespaceName == OdfNamespaces.Text:
                        AppendTextElement(child, depth);
                        break;

                    case XElement child:
                        Append(child, depth + 1);
                        break;

                    default:
                        break;
                }
            }
        }

        /// <summary>
        /// Appends one <c>text:</c> element, which is where ODF hides most of a paragraph's characters.
        /// </summary>
        /// <remarks>
        /// <para>
        /// A reader that took the descendant text nodes alone would lose every run of spaces, every tab
        /// and every line break — and <c>text:s</c> in particular, which is how any run of two or more
        /// spaces is written.
        /// </para>
        /// <para>
        /// A note's or comment's body is inside the paragraph in the file but is not part of its text, so
        /// it contributes only the anchor character it occupies. Walking into it would put a footnote's
        /// whole text into the middle of the sentence that cites it.
        /// </para>
        /// </remarks>
        private void AppendTextElement(XElement child, int depth)
        {
            switch (child.Name.LocalName)
            {
                case "s":
                    // text:c is the count, and its absence means one space rather than none.
                    int count = 1;
                    if (child.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value is { } value
                        && int.TryParse(value, out int declared)
                        && declared is > 0 and <= 4096)
                    {
                        count = declared;
                    }
                    Emit(new string(' ', count));
                    break;

                case "tab":
                    Emit("\t");
                    break;

                case "line-break":
                    // A line break within a paragraph, which layout has to honour as a forced break —
                    // recorded as the character so the break iterator sees it.
                    Emit(LineSeparator.ToString());
                    break;

                case "note" or "annotation" or "annotation-end":
                    // The anchor occupies a position; its body is a separate flow.
                    Emit(AnchorCharacter.ToString());
                    break;

                case "soft-page-break" or "bookmark" or "bookmark-start" or "bookmark-end"
                    or "reference-mark" or "reference-mark-start" or "reference-mark-end"
                    or "change" or "change-start" or "change-end":
                    break;

                // The three elements that carry character formatting of their own. A hyperlink's is a
                // character style like any other, which is why a link is usually blue and underlined
                // without anything inside it saying so.
                case "span" or "a" or "ruby-base":
                    string? styleName = child
                        .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

                    bool pushed = !string.IsNullOrEmpty(styleName);
                    if (pushed) _cascade.Add(new OdfStyleReference(styleName, OdfStyleFamily.Text));

                    Append(child, depth + 1);

                    if (pushed) _cascade.RemoveAt(_cascade.Count - 1);
                    break;

                default:
                    Append(child, depth + 1);
                    break;
            }
        }

        /// <summary>Appends text under the cascade currently in force.</summary>
        private void Emit(string text)
        {
            if (text.Length == 0) return;

            _builder.Append(text);

            if (_ranges.Count > 0 && SameCascade(_ranges[^1].Cascade))
            {
                _ranges[^1] = _ranges[^1] with { Length = _ranges[^1].Length + text.Length };
                return;
            }

            _ranges.Add(new StyledRange(
                _builder.Length - text.Length, text.Length, [.. _cascade]));
        }

        private bool SameCascade(OdfStyleReference[] other)
        {
            if (other.Length != _cascade.Count) return false;

            for (int i = 0; i < other.Length; i++)
            {
                if (other[i] != _cascade[i]) return false;
            }

            return true;
        }
    }

    /// <summary>
    /// The face a text style resolves to, cached per distinct request.
    /// </summary>
    /// <remarks>
    /// Cached because a document has a handful of fonts and thousands of paragraphs, and resolving one
    /// means walking the substitution chain and reading a font file. Null only when nothing at all could
    /// be loaded, which means the machine has no usable fonts rather than that the document is bad.
    /// </remarks>
    private OpenTypeFace? Face(OdfTextStyle text)
    {
        (string? Family, int Weight, bool Italic) key = (text.FamilyName, text.Weight, text.IsItalic);
        if (_faces.TryGetValue(key, out OpenTypeFace? cached)) return cached;

        try
        {
            FontReference reference = _fonts.Resolve(new FontRequest(
                text.FamilyName ?? string.Empty, text.Weight, text.IsItalic));

            OpenTypeFace face = _fonts.LoadOpenType(reference);
            _faces[key] = face;
            _references[key] = reference;
            return face;
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A font that cannot be read is not a reason to lose the paragraph, but there is nothing to
            // measure it with either — so it is dropped and the caller sees a shorter document rather
            // than an exception from the middle of a layout.
            _faces[key] = null!;
            return null;
        }
    }
}
