using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

/// <summary>
/// Turns a DOCX body into the paragraphs the paginator takes.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over <c>document.xml</c>, for the same reason ODF has one: extraction discards the font
/// sizes, indents and spacing layout needs, and making it carry them would charge every caller for a
/// feature most never use.
/// </para>
/// <para>
/// The same gap as the ODF source: per-run font sizes are not honoured, so a paragraph is measured
/// wholly in the font its paragraph mark carries. The tallest run on a line sets that line's height, so
/// a paragraph mixing sizes lays out slightly short until the runs are walked.
/// </para>
/// </remarks>
public sealed partial class DocxLayoutSource
{
    /// <summary>How many paragraphs are read before the rest are ignored.</summary>
    public const int MaxParagraphs = 200000;

    /// <summary>
    /// The character an anchor occupies: a field result, a note reference, an inline drawing.
    /// </summary>
    /// <remarks>
    /// The same one the document model and the ODF source use, so an offset means the same thing
    /// wherever it was counted.
    /// </remarks>
    private const char AnchorCharacter = '\u0001';

    /// <summary>
    /// The character a <c>w:br</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, whose UAX #14 class is a mandatory break, so the break iterator honours it without layout
    /// special-casing anything. A newline would break the same way but would read as the end of a
    /// paragraph to anything that later scans the text, which a break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    private readonly WordStyles _styles;
    private readonly SystemFontResolver _fonts;
    private readonly Length _defaultTabInterval;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), OpenTypeFace?> _faces = [];
    private readonly Dictionary<(string? Family, int Weight, bool Italic), FontReference> _references =
        [];

    /// <summary>Creates a source over a document's styles and settings.</summary>
    /// <param name="styles">The document's styles, including its <c>w:docDefaults</c>.</param>
    /// <param name="settings">The document's <c>w:settings</c> root, or null.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    public DocxLayoutSource(
        WordStyles styles, XElement? settings = null, SystemFontResolver? fonts = null)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _defaultTabInterval = TabInterval(settings);
    }

    /// <summary>The substitutions made while resolving the document's fonts.</summary>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>Reads the body's blocks — its paragraphs and its tables — in document order.</summary>
    /// <param name="body">The <c>w:body</c> element.</param>
    public List<PageBlock> Read(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        _sectionIndex = 0;
        List<PageBlock> blocks = [];
        Walk(body, blocks, depth: 0);
        return blocks;
    }

    /// <summary>
    /// Reads a table cell's blocks, tables included.
    /// </summary>
    /// <remarks>
    /// Unlike <see cref="ReadFlow"/>, which is for a header or a footer, a cell keeps its tables: a table
    /// inside a cell is how a nested table is written, and <see cref="FlowLayouter"/> lays one out. The two
    /// differ only in the list they fill, which is what the generic walk is for.
    /// </remarks>
    /// <param name="element">The cell element.</param>
    public List<PageBlock> ReadCell(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);

        List<PageBlock> blocks = [];
        Walk(element, blocks, depth: 0);
        return blocks;
    }

    /// <summary>
    /// Reads a flow's paragraphs only: a header or a footer.
    /// </summary>
    /// <remarks>
    /// Paragraphs only, unlike <see cref="ReadCell"/>. A header <em>could</em> hold a table — the layouter
    /// places one either way — but no reader has been shown to need it, and dropping one here is a smaller
    /// wrong answer than the alternative was: a table stacked into a header as loose paragraphs would give
    /// the header a height no table has and push the body text down by it.
    /// </remarks>
    /// <param name="element">The element whose block-level children to read.</param>
    public List<PageParagraph> ReadFlow(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);

        List<PageParagraph> paragraphs = [];
        Walk(element, paragraphs, depth: 0);
        return paragraphs;
    }

    /// <summary>
    /// Which section the walk is in, advanced by each paragraph that closes one.
    /// </summary>
    /// <remarks>
    /// A field rather than a walk parameter because the walk recurses through content controls and tracked
    /// insertions, and a section can end inside one — so the count has to survive returning from a nested
    /// call rather than being restored with it.
    /// </remarks>
    private int _sectionIndex;

    /// <summary>
    /// Walks the body's block-level children.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:sdt</c> — a structured-document tag, which is what a content control is — wraps ordinary
    /// content inside a <c>w:sdtContent</c>, so a walk that stopped at it would lose every paragraph in
    /// a form.
    /// </para>
    /// <para>
    /// Generic in what it fills, which is how one walk serves both the body and a flow. A body takes
    /// <see cref="PageBlock"/> and so keeps the tables; a header, a footer or a cell takes
    /// <see cref="PageParagraph"/>, and a table simply does not fit in the list — so it is dropped by the
    /// type rather than by a flag that could be passed the wrong way round.
    /// </para>
    /// </remarks>
    private void Walk<T>(XElement element, List<T> into, int depth)
        where T : PageBlock
    {
        if (depth > 64 || into.Count >= MaxParagraphs) return;

        foreach (XElement child in element.Elements())
        {
            if (into.Count >= MaxParagraphs) return;

            if (Word.Is(child, "p"))
            {
                if (Paragraph(child) is { } paragraph && paragraph is T block) into.Add(block);

                // A DOCX states a section's properties at its *end*: the w:sectPr inside a paragraph's
                // properties closes the section that paragraph finishes. So the counter advances after the
                // paragraph, which is what puts that paragraph in the section it ends rather than the next.
                if (Word.Child(Word.Child(child, "pPr"), "sectPr") is not null) _sectionIndex++;
                continue;
            }

            if (Word.Is(child, "tbl"))
            {
                if (Table(child) is { } table && table is T grid) into.Add(grid);
                continue;
            }

            if (Word.Is(child, "sdt") || Word.Is(child, "sdtContent"))
            {
                Walk(child, into, depth + 1);
            }
        }
    }

    private PageParagraph? Paragraph(XElement element)
    {
        XElement? properties = Word.Child(element, "pPr");

        WordTextStyle text = WordParagraphFormats.ResolveText(_styles, properties);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        RunWalker walker = new();
        walker.Walk(element);

        return new PageParagraph
        {
            SectionIndex = _sectionIndex,
            Text = walker.Text,
            Face = face,
            Font = _references.GetValueOrDefault(text.FaceKey),
            Colour = text.Colour ?? Colour.Black,
            Format = WordParagraphFormats.Resolve(_styles, properties, _defaultTabInterval),
            EmSize = text.Size,
            Language = text.Language,
            Shaping = new ShapingOptions(Language: text.Language),
            Runs = RunsOf(walker.Ranges, properties, text, face),
            Source = element,
        };
    }

    /// <summary>
    /// The paragraph's runs, or nothing when every one of them is the paragraph's own formatting.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Returning an empty list for a uniform paragraph is not only an optimisation: it puts plain prose
    /// back on the single-face path, which shapes the whole paragraph in one call. A run boundary also
    /// breaks shaping context, so a paragraph split into runs it does not need loses a kern pair at each
    /// boundary and measures very slightly wide — and a DOCX splits runs for reasons that have nothing to
    /// do with formatting, a spell-check marker or a revision id being enough.
    /// </para>
    /// <para>
    /// A range whose font cannot be loaded falls back to the paragraph's face rather than being dropped:
    /// its text is still part of the paragraph, and losing it would silently shorten the document.
    /// </para>
    /// </remarks>
    private List<PageRun> RunsOf(
        IReadOnlyList<StyledRange> ranges,
        XElement? paragraphProperties,
        WordTextStyle paragraph,
        OpenTypeFace paragraphFace)
    {
        List<PageRun> runs = new(ranges.Count);
        bool varies = false;

        foreach (StyledRange range in ranges)
        {
            WordTextStyle style = range.RunProperties is null
                ? paragraph
                : WordParagraphFormats.ResolveRun(_styles, paragraphProperties, range.RunProperties);

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

    /// <summary>
    /// A stretch of a paragraph's text and the run properties in force over it.
    /// </summary>
    /// <param name="Start">Its first character, as an index into the paragraph's text.</param>
    /// <param name="Length">How many characters it covers.</param>
    /// <param name="RunProperties">
    /// The enclosing <c>w:r</c>'s <c>w:rPr</c>, or null when the run states none — in which case the
    /// paragraph mark's own formatting applies.
    /// </param>
    private readonly record struct StyledRange(int Start, int Length, XElement? RunProperties);

    /// <summary>
    /// Walks a paragraph, building the text as laid out and the ranges its runs divide it into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two things have to be got right or the measurement is of the wrong string. A <c>w:del</c> holds
    /// text a tracked change removed, and it is still in the file — measuring it lays out words the
    /// document does not show. And a field's instruction lives in the same run sequence as its result,
    /// bracketed by <c>w:fldChar</c> markers, so a reader that takes every <c>w:t</c> lays out
    /// <c>PAGE \* Arabic</c> in the middle of a sentence.
    /// </para>
    /// <para>
    /// <c>w:tab</c> and <c>w:br</c> are elements rather than characters, as in ODF, and dropping them
    /// silently closes up the space they occupy.
    /// </para>
    /// <para>
    /// The ranges come from the same walk rather than from a second pass, because they are offsets into
    /// that text and the text is not a concatenation of the paragraph's <c>w:t</c> values — every tab,
    /// break and anchor shifts everything after it, and every skipped deletion shifts it back.
    /// </para>
    /// </remarks>
    private sealed class RunWalker
    {
        /// <summary>How deep a paragraph's element nesting is followed.</summary>
        /// <remarks>
        /// Hyperlinks, content controls, smart tags and change regions all wrap runs and do nest, but a
        /// generated file can nest indefinitely and this recurses on untrusted input.
        /// </remarks>
        private const int MaxDepth = 64;

        private readonly StringBuilder _builder = new();
        private readonly List<StyledRange> _ranges = [];
        private XElement? _runProperties;
        private bool _inInstruction;

        /// <summary>The paragraph's text, as laid out.</summary>
        internal string Text => _builder.ToString();

        /// <summary>The ranges, in order, partitioning the text.</summary>
        internal IReadOnlyList<StyledRange> Ranges => _ranges;

        /// <summary>Walks a <c>w:p</c>.</summary>
        internal void Walk(XElement paragraph) => Append(paragraph, depth: 0);

        private void Append(XElement element, int depth)
        {
            if (depth > MaxDepth) return;

            foreach (XElement child in element.Elements())
            {
                switch (child.Name.LocalName)
                {
                    case "del" or "delText" or "instrText":
                        // Deleted text and field instructions are in the file and not on the page.
                        break;

                    case "fldChar":
                        // "separate" ends the instruction and starts the result; "end" closes the field.
                        string? type = Word.Attribute(child, "fldCharType");
                        if (type == "begin") _inInstruction = true;
                        else if (type is "separate" or "end") _inInstruction = false;
                        break;

                    case "t" when !_inInstruction:
                        Emit(child.Value);
                        break;

                    case "tab" when !_inInstruction:
                        Emit("\t");
                        break;

                    case "br" when !_inInstruction:
                        Emit(LineSeparator.ToString());
                        break;

                    case "footnoteReference" or "endnoteReference" or "commentReference"
                        or "drawing" or "pict" or "object":
                        Emit(AnchorCharacter.ToString());
                        break;

                    case "pPr" or "bookmarkStart" or "bookmarkEnd" or "proofErr" or "rPr":
                        break;

                    case "r":
                        // The one element that carries character formatting. Runs do not nest, but this
                        // saves and restores anyway so that a malformed file cannot lose the outer state.
                        XElement? outer = _runProperties;
                        _runProperties = Word.Child(child, "rPr");
                        Append(child, depth + 1);
                        _runProperties = outer;
                        break;

                    default:
                        Append(child, depth + 1);
                        break;
                }
            }
        }

        /// <summary>Appends text under the run properties currently in force.</summary>
        private void Emit(string text)
        {
            if (text.Length == 0) return;

            _builder.Append(text);

            // Adjacent runs with the same properties merge, which matters because a DOCX splits runs for
            // reasons that are not formatting: a proofing error, a revision id, a bookmark boundary.
            if (_ranges.Count > 0 && _ranges[^1].RunProperties == _runProperties)
            {
                _ranges[^1] = _ranges[^1] with { Length = _ranges[^1].Length + text.Length };
                return;
            }

            _ranges.Add(new StyledRange(
                _builder.Length - text.Length, text.Length, _runProperties));
        }
    }

    /// <summary>
    /// The document's default tab interval.
    /// </summary>
    /// <remarks>
    /// Half an inch when the document does not say, which is what Word uses. A zero would make a tab
    /// advance nowhere, so it is treated as absent rather than honoured.
    /// </remarks>
    private static Length TabInterval(XElement? settings)
        => Word.Attribute(Word.Child(settings, "defaultTabStop"), "val") is { } text
           && long.TryParse(text, CultureInfo.InvariantCulture, out long twips)
           && twips > 0
            ? Length.FromTwips(twips)
            : Length.FromTwips(720);

    private OpenTypeFace? Face(WordTextStyle text)
    {
        (string? Family, int Weight, bool Italic) key = text.FaceKey;
        if (_faces.TryGetValue(key, out OpenTypeFace? cached)) return cached;

        OpenTypeFace? face = null;
        try
        {
            FontReference reference = _fonts.Resolve(
                new FontRequest(text.FamilyName ?? string.Empty, text.Weight, text.IsItalic));

            face = _fonts.LoadOpenType(reference);
            _references[key] = reference;
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // Nothing to measure the paragraph with. Dropping it gives a shorter document rather than
            // an exception out of the middle of a layout.
        }

        _faces[key] = face;
        return face;
    }
}
