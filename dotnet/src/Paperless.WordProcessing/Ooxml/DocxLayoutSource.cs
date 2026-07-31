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
    private readonly int _compatibilityMode;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), OpenTypeFace?> _faces = [];
    private readonly Dictionary<(string? Family, int Weight, bool Italic), FontReference> _references =
        [];

    /// <summary>Creates a source over a document's styles and settings.</summary>
    /// <param name="styles">The document's styles, including its <c>w:docDefaults</c>.</param>
    /// <param name="settings">The document's <c>w:settings</c> root, or null.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    /// <param name="footnotes">The footnote bodies by <c>w:id</c>, or null for a document with none.</param>
    /// <param name="endnotes">The endnote bodies by <c>w:id</c>.</param>
    public DocxLayoutSource(
        WordStyles styles,
        XElement? settings = null,
        SystemFontResolver? fonts = null,
        IReadOnlyDictionary<string, XElement>? footnotes = null,
        IReadOnlyDictionary<string, XElement>? endnotes = null)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _defaultTabInterval = TabInterval(settings);
        _compatibilityMode = CompatibilityMode(settings);
        _footnotes = footnotes ?? new Dictionary<string, XElement>(StringComparer.Ordinal);
        _endnotes = endnotes ?? new Dictionary<string, XElement>(StringComparer.Ordinal);
        _footnoteNumbering = NumberingIn(settings, "footnotePr", NoteNumbering.Footnotes);
        _endnoteNumbering = NumberingIn(settings, "endnotePr", NoteNumbering.Endnotes);
    }

    /// <summary>The footnote bodies by <c>w:id</c>, from <c>footnotes.xml</c>.</summary>
    /// <remarks>
    /// The parts rather than the whole package, because that is all layout needs of it — and passing the
    /// package would let this reach for things the extraction pass owns.
    /// </remarks>
    private readonly IReadOnlyDictionary<string, XElement> _footnotes;

    /// <summary>The endnote bodies by <c>w:id</c>.</summary>
    private readonly IReadOnlyDictionary<string, XElement> _endnotes;

    /// <summary>How the document's footnotes are numbered.</summary>
    private readonly NoteNumbering _footnoteNumbering;

    /// <summary>How its endnotes are numbered, which is a separate sequence in a separate format.</summary>
    private readonly NoteNumbering _endnoteNumbering;

    /// <summary>
    /// The numbering one class of note declares in the document's settings, or the class's default.
    /// </summary>
    /// <remarks>
    /// <c>w:footnotePr</c> and <c>w:endnotePr</c> in <c>w:settings</c>, whose <c>w:numStart</c> is the first
    /// note's number outright — one-based, unlike ODF's <c>text:start-value</c>, which is an offset. A
    /// <em>section</em> can carry the same two elements and override the document's; that is not read, and a
    /// document doing it is numbered by the document-wide values instead.
    /// </remarks>
    /// <param name="settings">The <c>w:settings</c> root, or null.</param>
    /// <param name="element">Which of the two elements to read.</param>
    /// <param name="fallback">The class's default, for whatever the file leaves unsaid.</param>
    private static NoteNumbering NumberingIn(
        XElement? settings, string element, NoteNumbering fallback)
    {
        XElement? properties = Word.Child(settings, element);
        if (properties is null) return fallback;

        NoteNumberFormat format =
            NoteNumbering.Parse(Word.Attribute(Word.Child(properties, "numFmt"), "val"))
            ?? fallback.Format;

        int start = int.TryParse(
            Word.Attribute(Word.Child(properties, "numStart"), "val"),
            NumberStyles.Integer,
            CultureInfo.InvariantCulture,
            out int stated)
            ? stated
            : fallback.StartAt;

        // Where the class collects. `w:pos` means different things for the two elements and only the endnote
        // one matters here: a footnote's `beneathText` is still the foot of its page, while an endnote's
        // `sectEnd` moves it into the note area of the section's last page.
        NotePlacement placement = Word.Attribute(Word.Child(properties, "pos"), "val") switch
        {
            "sectEnd" => NotePlacement.SectionEnd,
            "docEnd" => NotePlacement.DocumentEnd,
            _ => fallback.Placement,
        };

        // Where the count begins again. `eachSect` is not a third kind of restart: Writer has no per-section
        // one, so its chapter restart is what OOXML's `eachSect` both exports from and reads back as.
        NoteRestart restart =
            NoteNumbering.ParseRestart(Word.Attribute(Word.Child(properties, "numRestart"), "val"))
            ?? fallback.Restart;

        return new NoteNumbering(format, start) { Placement = placement, Restart = restart };
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
    /// Reads a flow's blocks: a header's or a footer's.
    /// </summary>
    /// <remarks>
    /// The same walk a cell takes, tables included, because a table is how a two-part running head is usually
    /// laid out — one cell hard left, another hard right — and <see cref="FlowLayouter"/> places one either
    /// way. Dropping the table instead is not the harmless simplification it looks like: its paragraphs would
    /// stack as loose lines, giving the header a height no table has and pushing the body text down by the
    /// difference on every page.
    /// </remarks>
    /// <param name="element">The element whose block-level children to read.</param>
    public List<PageBlock> ReadFlow(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);

        List<PageBlock> blocks = [];
        Walk(element, blocks, depth: 0);
        return blocks;
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
    /// How many tables enclose the one being read, counted while its rows are walked.
    /// </summary>
    /// <remarks>
    /// A field for the same reason <see cref="_sectionIndex"/> is one: a cell's blocks are read by the walk
    /// that reads a paragraph's, so a nested table is found several calls deep rather than through a
    /// parameter somebody could pass wrongly. Only the table's own left edge depends on it.
    /// </remarks>
    private int _tableDepth;

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

    /// <summary>
    /// Reads one paragraph, with an optional prefix its own text does not contain.
    /// </summary>
    /// <param name="element">The <c>w:p</c>.</param>
    /// <param name="citation">
    /// The number a <c>w:footnoteRef</c> in this paragraph stands for, or null when it is not a note's. The
    /// number is not in the file: Word marks the place and counts the notes itself.
    /// </param>
    private PageParagraph? Paragraph(XElement element, string? citation = null)
    {
        XElement? properties = Word.Child(element, "pPr");

        WordTextStyle text = WordParagraphFormats.ResolveText(_styles, properties);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        RunWalker walker = new(CitationOf, _footnoteNumber, _endnoteNumber);
        walker.Walk(element, citation);

        // Notes are numbered across the document, so the counters advance by however many this paragraph
        // referenced — and the bodies are read after the walk, since reading one recurses into this method
        // and would otherwise renumber from the middle of the paragraph that references it.
        _footnoteNumber += walker.FootnotesSeen;
        _endnoteNumber += walker.EndnotesSeen;

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
            Notes = NotesOf(walker.Notes),
            Frames = FramesOf(walker.Frames),
            Source = element,
        };
    }

    /// <summary>How many footnotes the walk has passed, counted across the document.</summary>
    private int _footnoteNumber;

    /// <summary>
    /// The number the next endnote is cited by, counted separately from the footnotes.
    /// </summary>
    /// <remarks>
    /// Its own counter because the two sequences are independent — a document with two footnotes and two
    /// endnotes cites 1, 2, i and ii, not 1, 2, iii and iv — and because they are formatted differently.
    /// </remarks>
    private int _endnoteNumber;

    /// <summary>
    /// Reads each referenced note's body from the document's notes part.
    /// </summary>
    /// <remarks>
    /// By <c>w:id</c>, which is what a DOCX gives instead of putting the body at the reference: the note
    /// lives in <c>footnotes.xml</c> and the sentence holds only its number. The citation is placed at the
    /// head of the note's first paragraph, which is where Word draws it and where the part does not have it.
    /// </remarks>
    private List<PageNote> NotesOf(List<NoteAnchor> anchors)
    {
        if (anchors.Count == 0) return [];

        List<PageNote> notes = new(anchors.Count);

        foreach (NoteAnchor anchor in anchors)
        {
            if (anchor.Id is null) continue;

            IReadOnlyDictionary<string, XElement> part =
                anchor.IsEndnote ? _endnotes : _footnotes;

            if (!part.TryGetValue(anchor.Id, out XElement? body)) continue;

            List<PageBlock> blocks = ReadNoteBody(body, anchor.Citation);
            if (blocks.Count == 0) continue;

            notes.Add(new PageNote
            {
                Blocks = blocks,
                Offset = anchor.Offset,
                IsEndnote = anchor.IsEndnote,
                Placement =
                    (anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering).Placement,
                Restart = (anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering).Restart,
            });
        }

        return notes;
    }

    /// <summary>
    /// Reads a note's body, putting the citation at the head of its first paragraph.
    /// </summary>
    /// <remarks>
    /// Its own walk rather than <see cref="ReadCell"/>'s, only because the first paragraph takes the
    /// citation and the rest do not — everything else about it is the same, tables included.
    /// </remarks>
    private List<PageBlock> ReadNoteBody(XElement body, string citation)
    {
        List<PageBlock> blocks = [];
        bool first = true;

        foreach (XElement child in body.Elements())
        {
            if (Word.Is(child, "p"))
            {
                PageParagraph? paragraph = first ? Paragraph(child, citation) : Paragraph(child);

                if (paragraph is not null)
                {
                    blocks.Add(paragraph);
                    first = false;
                }

                continue;
            }

            Walk(child, blocks, depth: 0);
        }

        return blocks;
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

            if (range.IsCitation) style = AsCitation(style);

            OpenTypeFace face = Face(style) ?? paragraphFace;

            // The escapement is resolved here rather than where it was read, because its rise is a fraction
            // of the face's height and the face is only known now.
            Length size = style.Escapement.SizeOf(style.Size);
            Length rise = style.Escapement.RiseOf(face, style.Size);

            if (face != paragraphFace
                || size != paragraph.Size
                || style.Colour != paragraph.Colour
                || style.Language != paragraph.Language
                || rise != Length.Zero)
            {
                varies = true;
            }

            runs.Add(new PageRun(
                range.Start,
                range.Length,
                face,
                size,
                _references.GetValueOrDefault(style.FaceKey),
                style.Colour ?? paragraph.Colour ?? Colour.Black,
                new ShapingOptions(Language: style.Language),
                rise));
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
    /// <param name="IsCitation">
    /// True for a note's citation, which Word draws superscript whether the run says so or not.
    /// </param>
    private readonly record struct StyledRange(
        int Start, int Length, XElement? RunProperties, bool IsCitation = false);

    /// <summary>A note found while walking a paragraph, before its body has been read.</summary>
    /// <param name="Offset">Where its citation sits in the paragraph's text.</param>
    /// <param name="Id">The <c>w:id</c> naming its body in the notes part.</param>
    /// <param name="IsEndnote">True for an endnote, whose body lives in a different part.</param>
    /// <param name="Citation">The number it is cited by, counted rather than read, and already formatted.</param>
    private readonly record struct NoteAnchor(int Offset, string? Id, bool IsEndnote, string Citation);

    /// <summary>One floating frame in a paragraph, with the character offset it is anchored at.</summary>
    private readonly record struct FrameAnchor(int Offset, XElement Element);

    /// <summary>
    /// How deeply a frame's own text may hold further frames before the innermost is dropped.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input: a text frame holds paragraphs, a paragraph holds drawings, and a file
    /// claiming a hundred levels would read the same walk a hundred deep. Real documents nest one.
    /// </remarks>
    private const int MaxFrameNesting = 8;

    /// <summary>How many frames enclose the paragraph currently being read.</summary>
    private int _frameDepth;

    /// <summary>
    /// Reads the frames a paragraph anchors, with their own text laid out inside them.
    /// </summary>
    /// <remarks>
    /// A frame's content goes through <see cref="ReadFlow"/> — the same walk a header takes — so a frame
    /// containing a table or a list needs nothing of its own. The reader therefore re-enters itself, which
    /// is why the depth is counted.
    /// </remarks>
    private List<PageFrame> FramesOf(List<FrameAnchor> anchors)
    {
        if (anchors.Count == 0) return [];

        List<PageFrame> frames = [];

        foreach (FrameAnchor anchor in anchors)
        {
            Func<XElement, IReadOnlyList<PageBlock>>? content =
                _frameDepth < MaxFrameNesting ? Content : null;

            if (DocxFrames.Read(anchor.Element, content, anchor.Offset) is { } frame)
            {
                frames.Add(frame);
            }
        }

        return frames;

        IReadOnlyList<PageBlock> Content(XElement box)
        {
            _frameDepth++;
            try
            {
                return ReadFlow(box);
            }
            finally
            {
                _frameDepth--;
            }
        }
    }

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
        /// <summary>Creates a walker.</summary>
        /// <param name="citation">How a note of a class and an index is cited.</param>
        /// <param name="footnote">
        /// How many footnotes came before this paragraph. Passed in because notes are numbered across the
        /// document rather than within a paragraph, so the counters belong to the source.
        /// </param>
        /// <param name="endnote">How many endnotes came before it, counted separately.</param>
        internal RunWalker(Func<bool, int, string> citation, int footnote = 0, int endnote = 0)
        {
            _citationOf = citation;
            _footnote = footnote;
            _endnote = endnote;
        }

        /// <summary>How a note of a class and an index is cited, which the source resolves.</summary>
        /// <remarks>
        /// A delegate because the walker is nested but not owned: the numbering comes from the document's
        /// settings, which the source read, and a walker is built per paragraph.
        /// </remarks>
        private readonly Func<bool, int, string> _citationOf;

        /// <summary>How deep a paragraph's element nesting is followed.</summary>
        /// <remarks>
        /// Hyperlinks, content controls, smart tags and change regions all wrap runs and do nest, but a
        /// generated file can nest indefinitely and this recurses on untrusted input.
        /// </remarks>
        private const int MaxDepth = 64;

        private readonly StringBuilder _builder = new();
        private readonly List<StyledRange> _ranges = [];
        private readonly List<NoteAnchor> _notes = [];
        private readonly List<FrameAnchor> _frames = [];
        private int _footnote;
        private int _endnote;
        private XElement? _runProperties;

        /// <summary>How many footnotes this paragraph cited, which advances the source's counter.</summary>
        internal int FootnotesSeen { get; private set; }

        /// <summary>How many endnotes it cited.</summary>
        internal int EndnotesSeen { get; private set; }
        private bool _inInstruction;

        /// <summary>The paragraph's text, as laid out.</summary>
        internal string Text => _builder.ToString();

        /// <summary>The ranges, in order, partitioning the text.</summary>
        internal IReadOnlyList<StyledRange> Ranges => _ranges;

        /// <summary>The notes referenced in the paragraph, with the offsets their citations occupy.</summary>
        internal List<NoteAnchor> Notes => _notes;

        /// <summary>The floating frames anchored in the paragraph, with the offsets they sit at.</summary>
        internal List<FrameAnchor> Frames => _frames;

        /// <summary>Walks a <c>w:p</c>.</summary>
        /// <param name="paragraph">The paragraph element.</param>
        /// <param name="citation">
        /// The number a <c>w:footnoteRef</c> in this paragraph stands for, or null when the paragraph is not
        /// a note's. Unlike ODF, a DOCX marks the place its citation goes: the note's own first paragraph
        /// contains a <c>w:footnoteRef</c>, inside a run whose character style is what makes the number
        /// superscript. So the citation is emitted where the file says rather than prepended.
        /// </param>
        internal void Walk(XElement paragraph, string? citation = null)
        {
            _citation = citation;
            Append(paragraph, depth: 0);
        }

        /// <summary>The number a <c>w:footnoteRef</c> stands for, when this paragraph is a note's.</summary>
        private string? _citation;

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

                    case "footnoteReference" or "endnoteReference":
                    {
                        // A note reference carries its citation, which Word draws in the sentence as a
                        // superscript and again at the head of the note. The style comes from the run this
                        // reference sits in, which is what carries w:vertAlign="superscript".
                        bool isEndnote = Word.Is(child, "endnoteReference");
                        string number = _citationOf(isEndnote, isEndnote ? _endnote : _footnote);

                        _notes.Add(new NoteAnchor(
                            _builder.Length, Word.Attribute(child, "id"), isEndnote, number));

                        _inCitation = true;
                        Emit(number);
                        _inCitation = false;

                        if (isEndnote)
                        {
                            _endnote++;
                            EndnotesSeen++;
                        }
                        else
                        {
                            _footnote++;
                            FootnotesSeen++;
                        }

                        break;
                    }

                    case "footnoteRef" or "endnoteRef":
                        // The note's own citation, at the place the file marks for it. Marked as a citation
                        // so that it falls back to superscript, because the style that should supply it
                        // usually does not: LibreOffice exports its built-in `Footnote Characters` as an
                        // *empty* w:rPr and relies on the importer knowing what that style is. A reader
                        // taking the file at its word draws the number full size on the baseline, where it
                        // fuses with the note's first word.
                        if (_citation is not null)
                        {
                            _inCitation = true;
                            Emit(_citation);
                            _inCitation = false;
                        }

                        break;

                    // A floating frame occupies a position in the paragraph and is not part of it: its
                    // own text belongs to a rectangle of its own. Recorded with the offset it sits at,
                    // which is what an anchor is measured in; the anchor character stands for it, as it
                    // does for every other thing that takes a position and is not text.
                    case "drawing":
                        _frames.Add(new FrameAnchor(_builder.Length, child));
                        Emit(AnchorCharacter.ToString());
                        break;

                    case "commentReference" or "pict" or "object":
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
            if (_ranges.Count > 0
                && _ranges[^1].IsCitation == _inCitation
                && _ranges[^1].RunProperties == _runProperties)
            {
                _ranges[^1] = _ranges[^1] with { Length = _ranges[^1].Length + text.Length };
                return;
            }

            _ranges.Add(new StyledRange(
                _builder.Length - text.Length, text.Length, _runProperties, _inCitation));
        }

        /// <summary>True while a note's citation is being emitted.</summary>
        private bool _inCitation;
    }

    /// <summary>
    /// A citation's style, defaulted to superscript when the run does not say so.
    /// </summary>
    /// <remarks>
    /// Word's own <c>FootnoteReference</c> character style sets <c>w:vertAlign="superscript"</c>, and a
    /// document that has it is read correctly without this. LibreOffice's DOCX export does not always write
    /// it, and a document whose notes were added by something else may not either — so the default matches
    /// what Word draws rather than what the file happens to state. Applied only when nothing has been said,
    /// so a run that does state a shift keeps it.
    /// </remarks>
    private static WordTextStyle AsCitation(WordTextStyle style)
        => style.Escapement.IsNone
            ? style with { Escapement = Layout.Escapement.Superscript }
            : style;

    /// <summary>
    /// How a note of each class is cited, which is not the same for the two.
    /// </summary>
    /// <remarks>
    /// Two sequences in two formats, from the document's <c>w:footnotePr</c> and <c>w:endnotePr</c> where it
    /// has them and from LibreOffice's own defaults where it does not — footnotes 1, 2, 3 and endnotes
    /// i, ii, iii, which is measured rather than assumed.
    /// </remarks>
    /// <param name="isEndnote">True for an endnote.</param>
    /// <param name="index">How many notes of the class came before, counted from zero.</param>
    private string CitationOf(bool isEndnote, int index)
        => (isEndnote ? _endnoteNumbering : _footnoteNumbering).Citation(index);

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

    /// <summary>
    /// Which version of Word wrote the file, from <c>w:compat</c>, or <c>-1</c> when it does not say.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A single number covering a decade of behaviour changes: 12 is Word 2007, 14 is 2010 and 15 is 2013
    /// and after. Word writes it as a <c>w:compatSetting</c> named <c>compatibilityMode</c> in the
    /// Microsoft namespace, which is a URI rather than the <c>w:</c> one — so the name and the URI both
    /// have to match, and a setting from another vendor's namespace is not this one.
    /// </para>
    /// <para>
    /// Absent stays <c>-1</c> rather than defaulting to 12, following
    /// <c>SettingsTable::GetWordCompatibilityMode</c>: everything that consults it asks whether the mode is
    /// <em>below</em> 15, and −1 is, so a file that says nothing gets the older behaviour without the
    /// reader having to invent a version for it.
    /// </para>
    /// </remarks>
    private static int CompatibilityMode(XElement? settings)
    {
        const string wordUri = "http://schemas.microsoft.com/office/word";

        foreach (XElement setting in Word.Children(Word.Child(settings, "compat"), "compatSetting"))
        {
            if (Word.Attribute(setting, "name") != "compatibilityMode") continue;
            if (Word.Attribute(setting, "uri") != wordUri) continue;

            if (int.TryParse(
                    Word.Attribute(setting, "val"), NumberStyles.Integer, CultureInfo.InvariantCulture,
                    out int mode))
            {
                return mode;
            }
        }

        return -1;
    }

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
