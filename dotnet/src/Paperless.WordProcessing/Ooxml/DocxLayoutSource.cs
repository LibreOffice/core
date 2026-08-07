using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
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

    /// <summary>What <c>w:beforeAutospacing</c> and <c>w:afterAutospacing</c> stand for here.</summary>
    private readonly Length _autoSpacing;

    /// <summary>
    /// The device grid every font metric is rounded onto, or null for printer-independent layout.
    /// </summary>
    /// <remarks>
    /// Set by <c>w:usePrinterMetrics</c>, which writerfilter turns into
    /// <c>PrinterIndependentLayout::DISABLED</c> at
    /// <c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:10173</c> — the same state
    /// <c>WW8Dop::fUsePrinterMetrics</c> puts a DOC into, and the same 300 dpi grid
    /// <see cref="Ww8.DocReader"/> already passes.
    /// </remarks>
    private readonly MetricGrid? _metrics;
    private readonly DrawingTheme? _theme;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), OpenTypeFace?> _faces = [];
    private readonly Dictionary<(string? Family, int Weight, bool Italic), FontReference> _references =
        [];

    /// <summary>Creates a source over a document's styles and settings.</summary>
    /// <param name="styles">The document's styles, including its <c>w:docDefaults</c>.</param>
    /// <param name="settings">The document's <c>w:settings</c> root, or null.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    /// <param name="footnotes">The footnote bodies by <c>w:id</c>, or null for a document with none.</param>
    /// <param name="endnotes">The endnote bodies by <c>w:id</c>.</param>
    /// <param name="theme">The document's theme, for themed run colours, or null.</param>
    /// <param name="pictures">
    /// How to reach the bytes an <c>a:blip</c> names, or null to lay the document out with its picture
    /// frames empty — which is what a caller who wants only measurements should pay for.
    /// </param>
    /// <param name="numbering">
    /// The document's <c>numbering.xml</c>, or null for a document with no lists. Its counters are
    /// advanced by this walk, which is why <see cref="Read"/> and <see cref="ReadFlow"/> reset them: a
    /// caller sharing one instance with the extraction pass must not have the two interleave.
    /// </param>
    public DocxLayoutSource(
        WordStyles styles,
        XElement? settings = null,
        SystemFontResolver? fonts = null,
        IReadOnlyDictionary<string, XElement>? footnotes = null,
        IReadOnlyDictionary<string, XElement>? endnotes = null,
        DrawingTheme? theme = null,
        DocxPictures? pictures = null,
        WordNumbering? numbering = null)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _numbering = numbering ?? new WordNumbering();
        Pictures = pictures;
        _theme = theme;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _defaultTabInterval = TabInterval(settings);
        _compatibilityMode = CompatibilityMode(settings);
        WordCompatibility compatibility = WordCompatibility.Read(settings);
        _autoSpacing = compatibility.DoNotUseHtmlParagraphAutoSpacing
            ? WordParagraphFormats.WordAutoSpacing
            : WordParagraphFormats.HtmlAutoSpacing;
        _metrics = compatibility.UsesPrinterMetrics ? MetricGrid.Printer : null;
        _footnotes = footnotes ?? new Dictionary<string, XElement>(StringComparer.Ordinal);
        _endnotes = endnotes ?? new Dictionary<string, XElement>(StringComparer.Ordinal);
        _footnoteNumbering = NumberingIn(settings, "footnotePr", NoteNumbering.Footnotes);
        _endnoteNumbering = NumberingIn(settings, "endnotePr", NoteNumbering.Endnotes);
    }

    /// <summary>
    /// How a picture's bytes are reached, or null when this source was built without a package.
    /// </summary>
    /// <remarks>
    /// Exposed rather than private because its <see cref="DocxPictures.Scope"/> has to follow the walk:
    /// relationship ids are numbered from one in every part, so whoever hands this source a header to
    /// read must say which part it came from first.
    /// </remarks>
    public DocxPictures? Pictures { get; }

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
        _blocksInSection = 0;

        // The body is where the document's lists start counting. Reset rather than assumed clean,
        // because the numbering may be the same instance the extraction pass already walked.
        _numbering.ResetCounters();

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
        SuppressAutoSpacingInCell(blocks);
        return blocks;
    }

    /// <summary>
    /// Drops the HTML auto margin at a cell's top and bottom edges.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>w:beforeAutospacing</c> means fourteen points in a body paragraph and <em>nothing</em> on the
    /// first paragraph of a table cell; <c>w:afterAutospacing</c> likewise on the last. LibreOffice
    /// applies both — the first in <c>DomainMapper_Impl::finishParagraph</c>
    /// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:2458-2470</c>, where
    /// <c>bFirstParagraphInCell</c> at matching table depth forces the margin to zero) and the second in
    /// <c>ClearPreviousParagraph</c> (<c>:5457-5468</c>, called from <c>TableManager::closeCell</c>).
    /// Without it every row of a table whose style carries the flag is fourteen points taller than the
    /// document asks for, which on a form of thirty single-line rows is seven pages of invented height.
    /// </para>
    /// <para>
    /// A <em>stated</em> <c>w:before</c> survives, which is why this asks how the margin was arrived at
    /// rather than merely whether it is fourteen points: the suppression is of the auto rule, not of
    /// paragraph spacing in cells.
    /// </para>
    /// <para>
    /// The bottom rule spares a numbered paragraph, exactly as <c>ClearPreviousParagraph</c> does — it
    /// reads the paragraph's numbering rules and leaves the margin alone when it has any.
    /// </para>
    /// <para>
    /// <b>Not done:</b> the same <c>if</c> in <c>finishParagraph</c> also zeroes the top margin of the
    /// first paragraph of a <em>shape</em> and of the first paragraph of the document's first section.
    /// Both are the same rule and both are unimplemented here, because neither was measured — a cell is
    /// where the corpus showed it, and the other two move the body flow, which is not free to change on
    /// an argument from symmetry alone.
    /// </para>
    /// </remarks>
    private void SuppressAutoSpacingInCell(List<PageBlock> blocks)
    {
        // Only a paragraph at the very edge is affected; a nested table there shields whatever follows,
        // because the rule is about the cell's own first and last paragraph.
        if (blocks.Count > 0 && blocks[0] is PageParagraph first
            && WordParagraphFormats.IsAutoSpaced(
                _styles, Word.Child(first.Source as XElement, "pPr"), _tableStyle, before: true))
        {
            blocks[0] = first with { Format = first.Format with { SpaceBefore = Length.Zero } };
        }

        if (blocks.Count > 0 && blocks[^1] is PageParagraph last
            && last.Label is null
            && WordParagraphFormats.IsAutoSpaced(
                _styles, Word.Child(last.Source as XElement, "pPr"), _tableStyle, before: false))
        {
            blocks[^1] = last with { Format = last.Format with { SpaceAfter = Length.Zero } };
        }
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

        // Each flow numbers its own lists: a numbered paragraph in a footer does not continue the
        // body's count, which is the same rule the extraction reader applies between flows.
        _numbering.ResetCounters();

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
    /// How many blocks the current section has already contributed, reset when a section closes.
    /// </summary>
    /// <remarks>
    /// Only <see cref="IsSectionMarkOnly"/> reads it, and only to answer "is this section mark the whole
    /// section". Counted for every paragraph and table the walk passes rather than for the ones it keeps,
    /// which is what Writer's <c>bIsFirstParaInSection</c> counts — a paragraph the reader could not
    /// resolve a face for still separates the section mark from the start of its section.
    /// </remarks>
    private int _blocksInSection;

    /// <summary>
    /// Whether a paragraph that closes a section is nothing but the section mark, and so is not laid out.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word stores a section break as a paragraph mark carrying <c>w:sectPr</c>, and that mark is not a
    /// paragraph: it takes no line and no spacing. Writer's DOCX importer says so directly — "if the
    /// paragraph contains only the section properties and it has no runs, we should not create a paragraph
    /// for it in Writer, unless that would remove the whole section"
    /// (<c>writerfilter/dmapper/DomainMapper.cxx</c>:4840, the <c>bRemove</c> expression) — and it is the
    /// <c>!bSingleParagraphAfterRedline</c> term there that spells the exception: a mark that is both the
    /// first and the last paragraph of its section is kept, because dropping it would leave the section
    /// with no content to hang a page on.
    /// </para>
    /// <para>
    /// Measured on <c>easa-form-1.docx</c>, whose first section ends with an ordinary empty paragraph and
    /// then a section mark: laying the mark out overflowed the page by one line and produced a sixth page
    /// carrying nothing but the section's footer. LibreOffice's own flat-ODF export of that document holds
    /// one empty paragraph where the DOCX has two, which is this rule visible in its output.
    /// </para>
    /// <para>
    /// "Nothing but the mark" is read from what the paragraph produced rather than from its markup, so a
    /// mark that anchors a frame, cites a note, or defers a page break keeps its paragraph: each of those
    /// is content the page would otherwise lose, and Writer guards them individually
    /// (<c>HasTopAnchoredObjects</c>, <c>IsParaWithInlineObject</c>, the column-break test).
    /// </para>
    /// </remarks>
    private static bool IsSectionMarkOnly(PageParagraph paragraph)
        => paragraph.Text.Length == 0
           && paragraph.Frames.Count == 0
           && paragraph.Notes.Count == 0
           && !paragraph.Format.StartsNewPage;

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
    /// The <c>w:pPr</c> chain of the table style enclosing the paragraph being read, or null in the body.
    /// </summary>
    /// <remarks>
    /// A field rather than a parameter because a cell's content is read by the same recursive walk the
    /// body uses, and threading it through every overload would touch a dozen signatures for one value
    /// that changes only when a table is entered. Saved and restored around each table, so a nested table
    /// takes its own style and the outer one resumes after it.
    /// </remarks>
    private IReadOnlyList<XElement>? _tableStyle;

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
                bool endsSection = Word.Child(Word.Child(child, "pPr"), "sectPr") is not null;

                // Read it either way: even a paragraph that is only a section mark can leave a page break
                // behind for the paragraph after it, and that bookkeeping lives in `Paragraph`.
                if (Paragraph(child) is { } paragraph
                    && !(endsSection && _blocksInSection > 0 && IsSectionMarkOnly(paragraph))
                    && paragraph is T block)
                {
                    into.Add(block);
                }

                _blocksInSection++;

                // A DOCX states a section's properties at its *end*: the w:sectPr inside a paragraph's
                // properties closes the section that paragraph finishes. So the counter advances after the
                // paragraph, which is what puts that paragraph in the section it ends rather than the next.
                if (endsSection)
                {
                    _sectionIndex++;
                    _blocksInSection = 0;
                }

                continue;
            }

            if (Word.Is(child, "tbl"))
            {
                if (Table(child) is { } table && table is T grid) into.Add(grid);
                _blocksInSection++;
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

        WordTextStyle text = WordParagraphFormats.ResolveText(_styles, properties, _theme);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        // Taken before the walk and put back after it, because the walk can set a *new* one. What is
        // read here was left by the paragraph before this one; what the walk leaves belongs to the
        // paragraph after.
        bool breaksPage = _pageBreakPending;
        _pageBreakPending = false;

        RunWalker walker = new(CitationOf, _footnoteNumber, _endnoteNumber);
        walker.Walk(element, citation);

        // Where the note's own number landed, for a renumbering pass that has to find it again. A field
        // rather than an out parameter because this method reads an ordinary paragraph and a note's first
        // paragraph alike, and only the call that supplied a citation can have produced one.
        if (citation is not null) _noteCitationOffset = walker.CitationOffset;

        // Notes are numbered across the document, so the counters advance by however many this paragraph
        // referenced — and the bodies are read after the walk, since reading one recurses into this method
        // and would otherwise renumber from the middle of the paragraph that references it.
        _footnoteNumber += walker.FootnotesSeen;
        _endnoteNumber += walker.EndnotesSeen;

        ParagraphFormat format =
            WordParagraphFormats.Resolve(
                _styles, properties, _defaultTabInterval, _autoSpacing, _tableStyle,
                _compatibilityMode >= 15);

        // After the walk, because reading a note body or a text box re-enters this method and a list
        // counter advanced from inside a nested flow would number the paragraph after it wrongly.
        (PageLabel? label, format) = ListFormatting(properties, format, text, face);

        // The runs first, then the text they map: `Apply` rewrites both together, and the offsets it
        // preserves are the ones the notes and frames below were recorded against.
        List<PageRun> runs = RunsOf(walker.Ranges, properties, text, face);
        string mapped = CaseMapping.Apply(walker.Text, runs);

        PageParagraph read = new()
        {
            SectionIndex = _sectionIndex,
            Text = mapped,
            Face = face,
            Font = _references.GetValueOrDefault(text.FaceKey),
            Colour = text.Colour ?? Colour.Black,
            Shading = ShadeColour(WordParagraphFormats.ShadingOf(_styles, properties)),
            Format = breaksPage || walker.BreaksPageHere
                ? format with { StartsNewPage = true }
                : format,
            Label = label,

            // #i3952#: a tab or a run of spaces does not raise a line's height in a Word document, and a
            // DOCX imports with the setting on. See PageParagraph.BlanksAreTransparentToHeight.
            BlanksAreTransparentToHeight = true,
            Metrics = _metrics,
            EmSize = text.Size,
            Language = text.Language,
            Shaping = new ShapingOptions(
                Language: text.Language, DisableKerning: !text.AutoKerning),
            Tracking = text.Tracking,
            Runs = runs,
            Notes = NotesOf(walker.Notes),
            Frames = FramesOf(walker.Frames),
            Source = element,
        };

        // After the note bodies and the text boxes above, which recurse into this method and share the
        // field. Writer ignores a page break inside either — `DomainMapper.cxx:4376` applies a deferred
        // one only when it is not in a footnote, a shape or a comment — and overwriting here is what
        // makes that true here too: whatever a nested flow left behind is replaced by this paragraph's
        // own answer, so a break inside a caption cannot push the paragraph after the caption's frame.
        _pageBreakPending = walker.BreaksPage;

        return read;
    }

    /// <summary>Whether the paragraph read next begins a page, because the one before ended with a break.</summary>
    private bool _pageBreakPending;

    /// <summary>How many footnotes the walk has passed, counted across the document.</summary>
    private int _footnoteNumber;

    /// <summary>
    /// Where the last note body's own citation was emitted, or −1 when it emitted none.
    /// </summary>
    /// <remarks>
    /// A DOCX marks the place: a <c>w:footnoteRef</c> in the note's first paragraph, which a note beginning
    /// with a tab puts at one rather than at nought. Recorded so that a renumbering pass can rewrite the
    /// number at the head of the note as well as the one in the sentence.
    /// </remarks>
    private int _noteCitationOffset = -1;

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

            List<PageBlock> blocks = ReadNoteBody(body, anchor.Citation, out int bodyOffset);
            if (blocks.Count == 0) continue;

            notes.Add(new PageNote
            {
                Blocks = blocks,
                Offset = anchor.Offset,
                IsEndnote = anchor.IsEndnote,
                Placement =
                    (anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering).Placement,
                Restart = (anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering).Restart,
                Numbering = anchor.IsEndnote ? _endnoteNumbering : _footnoteNumbering,
                Citation = anchor.Citation,
                BodyOffset = bodyOffset,
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
    private List<PageBlock> ReadNoteBody(XElement body, string citation, out int citationOffset)
    {
        List<PageBlock> blocks = [];
        bool first = true;

        _noteCitationOffset = -1;

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

        // Nought when the part marks no place for its number, which is what a note whose first paragraph
        // holds no `w:footnoteRef` is: the citation was never emitted and there is nothing to rewrite.
        citationOffset = Math.Max(0, _noteCitationOffset);
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
                : WordParagraphFormats.ResolveRun(
                    _styles, paragraphProperties, range.RunProperties, _theme);

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
                || rise != Length.Zero
                // A case map has to survive the uniform-paragraph shortcut: it is the one property here
                // that changes the *characters*, so dropping the runs would draw the text as stored.
                || style.CaseMap != PageCaseMap.None
                // So does a highlight: the paragraph carries none of its own, so a paragraph highlighted
                // end to end is uniform by every other test and would lose its band entirely.
                || style.Highlight is not null
                // And so do the two rules, for the same reason: neither changes a width, so a paragraph
                // underlined end to end is uniform by every measurement test and would be drawn plain.
                || style.IsUnderlined
                || style.IsStruckThrough
                // Kerning, unlike the two rules, does change a measurement — so a run that kerns
                // inside a paragraph that does not has to survive the shortcut or its width is the
                // paragraph's answer rather than its own.
                || style.AutoKerning != paragraph.AutoKerning
                // And tracking, for the same reason and more sharply: it is a distance per character,
                // so a run that disagrees with its paragraph mark is wrong by its own length.
                || style.Tracking != paragraph.Tracking)
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
                new ShapingOptions(Language: style.Language, DisableKerning: !style.AutoKerning),
                rise,
                style.CaseMap,
                Highlight: style.Highlight ?? default,
                IsUnderlined: style.IsUnderlined,
                IsStruckThrough: style.IsStruckThrough,
                Tracking: style.Tracking));
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

            frames.AddRange(DocxFrames.ReadAll(anchor.Element, content, anchor.Offset, Pictures));
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

        /// <summary>Where in the text the last <c>w:br w:type="page"</c> fell, or −1 for none.</summary>
        private int _pageBreakAt = -1;

        /// <summary>
        /// True when a <c>w:br w:type="page"</c> ended the paragraph, so the <em>next</em> one starts a page.
        /// </summary>
        /// <remarks>
        /// A page break is written at the point in the text where the page ends, and the layout model says
        /// "this paragraph starts a page" — the same shape Writer's <c>BreakType_PAGE_BEFORE</c> has, and
        /// the same shape the DOC and RTF forms state directly. Which paragraph it lands on is decided by
        /// what follows the break rather than by the paragraph boundary: LibreOffice defers the break and
        /// applies it at the next run of text (<c>DomainMapper::lcl_utext</c>, which calls
        /// <c>deferBreak(PAGE_BREAK)</c> for U+000C and inserts <c>BreakType_PAGE_BEFORE</c> into the
        /// <em>current</em> paragraph context on the next text it sees). So a break with text after it in
        /// the same paragraph breaks before that paragraph, and only one with nothing after it carries over.
        /// </remarks>
        internal bool BreaksPage => _pageBreakAt >= 0 && _pageBreakAt >= _builder.Length;

        /// <summary>True when the break fell before this paragraph's own text, so this one starts a page.</summary>
        internal bool BreaksPageHere => _pageBreakAt >= 0 && _pageBreakAt < _builder.Length;

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

        /// <summary>Where that number was emitted, or −1 when the paragraph marked no place for one.</summary>
        internal int CitationOffset { get; private set; } = -1;

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

                    // A `w:br` is three things wearing one name and only one of them is a line break.
                    // `w:type="page"` moves everything after it to the next page and contributes no
                    // character at all: LibreOffice turns it back into the DOC's own U+000C
                    // (`OOXMLBreakHandler::~OOXMLBreakHandler`, `writerfilter/ooxml/Handler.cxx:246`)
                    // and then *defers* it, applying it to the paragraph that follows as
                    // `BreakType_PAGE_BEFORE` (`dmapper/DomainMapper.cxx:4379`).
                    case "br" when !_inInstruction:
                        if (Word.Attribute(child, "type") == "page") _pageBreakAt = _builder.Length;
                        else Emit(LineSeparator.ToString());
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
                            CitationOffset = _builder.Length;
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
