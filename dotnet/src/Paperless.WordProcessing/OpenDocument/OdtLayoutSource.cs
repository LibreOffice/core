using System.Text;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
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
    /// <param name="stylesRoot">
    /// The root of the styles part, for the <c>text:notes-configuration</c> that says how each class of note
    /// is numbered. Null leaves both classes on LibreOffice's defaults.
    /// </param>
    /// <param name="availableWidth">
    /// How wide the text area is, for a table that states no width and so fills it. Zero leaves such a table
    /// with columns at Writer's minimum width rather than at nothing.
    /// </param>
    public OdtLayoutSource(
        OdfStyles styles,
        SystemFontResolver? fonts = null,
        IReadOnlyDictionary<string, int>? masterPages = null,
        XElement? stylesRoot = null,
        Length availableWidth = default)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
        _masterPages = masterPages ?? new Dictionary<string, int>(StringComparer.Ordinal);
        _footnotes = NumberingIn(stylesRoot, "footnote", NoteNumbering.Footnotes);
        _endnotes = NumberingIn(stylesRoot, "endnote", NoteNumbering.Endnotes);
        _availableWidth = availableWidth;
    }

    /// <summary>
    /// How wide the text area is, for a table that states no width of its own.
    /// </summary>
    /// <remarks>
    /// The one piece of page geometry the content walk needs, and it needs it because ODF lets a table say
    /// "as wide as the text" by saying nothing: such a table fills the text area and divides it between its
    /// columns. Everything else here is resolution-independent, which is why this arrives as a constructor
    /// argument rather than the walk being given the section it is in.
    /// </remarks>
    private readonly Length _availableWidth;

    /// <summary>How the document's footnotes are numbered.</summary>
    private readonly NoteNumbering _footnotes;

    /// <summary>How its endnotes are numbered, which is a separate sequence in a separate format.</summary>
    private readonly NoteNumbering _endnotes;

    /// <summary>
    /// The numbering one class of note declares, or the class's default when it declares none.
    /// </summary>
    /// <remarks>
    /// <c>text:notes-configuration</c>, which lives in <c>office:styles</c> rather than in the content and is
    /// written once per class. ODF states the format by <em>example</em> — <c>style:num-format</c> holds the
    /// literal "1", "i" or "A" — which is why parsing it belongs with OOXML's naming of the same set rather
    /// than here. <c>text:start-numbering-at</c> can ask for a per-page or per-chapter restart and is not read:
    /// a restart has to be applied while pages are being filled, not while the document is being read.
    /// </remarks>
    private static NoteNumbering NumberingIn(
        XElement? stylesRoot, string noteClass, NoteNumbering fallback)
    {
        XElement? styles = stylesRoot?.Element(XName.Get("styles", OdfNamespaces.Office));

        XElement? configuration = styles?
            .Elements(XName.Get("notes-configuration", OdfNamespaces.Text))
            .FirstOrDefault(element =>
                element.Attribute(XName.Get("note-class", OdfNamespaces.Text))?.Value == noteClass);

        if (configuration is null) return fallback;

        NoteNumberFormat format =
            NoteNumbering.Parse(
                configuration.Attribute(XName.Get("num-format", OdfNamespaces.Style))?.Value)
            ?? fallback.Format;

        // `text:start-value` is an *offset*, not the first note's number, which is measured rather than read
        // off the specification: a document stating 7 renders VIII and IX. LibreOffice maps the attribute to
        // `SwFootnoteInfo::nFootnoteOffset` and adds one when it numbers, so a reader taking the value as the
        // first number is out by one on every citation in the document.
        int start = int.TryParse(
            configuration.Attribute(XName.Get("start-value", OdfNamespaces.Text))?.Value,
            System.Globalization.NumberStyles.Integer,
            System.Globalization.CultureInfo.InvariantCulture,
            out int offset)
            ? offset + 1
            : fallback.StartAt;

        // Where the class collects. ODF states this only for footnotes, as `text:footnotes-position`, whose
        // "document" is the same thing an endnote does by default; endnotes have no such attribute at all, so
        // theirs is always the class default. Carrying the fallback's placement through is not optional: a
        // document that declares a configuration purely to set a start value would otherwise have its
        // endnotes silently moved to the foot of the page, because a fresh NoteNumbering defaults there.
        NotePlacement placement =
            configuration.Attribute(XName.Get("footnotes-position", OdfNamespaces.Text))?.Value switch
            {
                "document" => NotePlacement.DocumentEnd,
                "page" => NotePlacement.PageBottom,
                _ => fallback.Placement,
            };

        return new NoteNumbering(format, start) { Placement = placement };
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

    /// <summary>
    /// Reads one paragraph, with an optional prefix its own text does not contain.
    /// </summary>
    /// <param name="element">The <c>text:p</c> or <c>text:h</c>.</param>
    /// <param name="prefix">
    /// Text to place before the paragraph's own, or null for none. Used for a footnote's citation at the
    /// start of the note's first paragraph, which LibreOffice draws there and the file does not contain.
    /// </param>
    /// <param name="prefixStyle">The character style the prefix takes.</param>
    private PageParagraph? Paragraph(
        XElement element, string? prefix = null, OdfStyleReference? prefixStyle = null)
    {
        string? styleName = element
            .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

        // A paragraph whose style names a master page moves the document onto that master, and everything
        // after it follows until another paragraph says otherwise.
        if (MasterPageOf(styleName) is { } section) _sectionIndex = section;

        OdfTextStyle text = OdfParagraphFormats.ResolveText(_styles, styleName);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        RunWalker walker = new(styleName, CitationOf, _footnoteNumber, _endnoteNumber);
        walker.Walk(element, prefix, prefixStyle);

        // Notes are numbered across the document, so the counters advance by however many this paragraph
        // cited — and the bodies are read after the walk, since reading one recurses into this method and
        // would otherwise renumber from the middle of the paragraph that cites it.
        _footnoteNumber += walker.FootnotesSeen;
        _endnoteNumber += walker.EndnotesSeen;

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
            Notes = NotesOf(walker.Notes),
            Frames = FramesOf(walker.Frames),
            Source = element,
        };
    }

    /// <summary>
    /// Turns the drawings anchored in a paragraph into frames the layout engine understands.
    /// </summary>
    /// <remarks>
    /// A drawing with no size is dropped rather than placed: ODF lets a shape state its extent by its
    /// geometry instead of by <c>svg:width</c>, and a frame of no width obstructs nothing, so placing it
    /// would only risk a division by its own zero somewhere later.
    /// </remarks>
    private List<PageFrame> FramesOf(List<FrameAnchorPoint> anchored)
    {
        List<PageFrame> frames = [];

        foreach (FrameAnchorPoint point in anchored)
        {
            XElement drawing = point.Element;

            DocSize size = new(
                Measure(drawing, "width") ?? Length.Zero,
                Measure(drawing, "height") ?? Length.Zero);

            if (size.IsEmpty) continue;

            string? styleName = drawing
                .Attribute(XName.Get("style-name", OdfNamespaces.Draw))?.Value;

            frames.Add(new PageFrame
            {
                Offset = new DocPoint(
                    Measure(drawing, "x") ?? Length.Zero,
                    Measure(drawing, "y") ?? Length.Zero),
                Size = size,
                Anchor = AnchorOf(drawing),
                Wrap = WrapOf(styleName),
                Margins = FrameMargins(styleName),
            });
        }

        return frames;

        static Length? Measure(XElement drawing, string name)
            => OdfWriterUnits.ToCore(
                OdfValue.ParseLength(
                    drawing.Attribute(XName.Get(name, OdfNamespaces.SvgCompatible))?.Value));
    }

    /// <summary>What a drawing's <c>text:anchor-type</c> means to layout.</summary>
    /// <remarks>
    /// <c>as-char</c> never reaches here — it is emitted as an anchor character during the walk instead,
    /// because such a frame sits in the line rather than beside it. <c>frame</c>, ODF's fifth value, anchors
    /// to the enclosing frame and is treated as a paragraph anchor: this reader has no frame to nest in.
    /// </remarks>
    private static FrameAnchor AnchorOf(XElement drawing)
        => drawing.Attribute(XName.Get("anchor-type", OdfNamespaces.Text))?.Value switch
        {
            "char" => FrameAnchor.Character,
            "page" => FrameAnchor.Page,
            _ => FrameAnchor.Paragraph,
        };

    /// <summary>
    /// How text treats the frame, from the graphic style's <c>style:wrap</c>.
    /// </summary>
    /// <remarks>
    /// ODF's default is <c>parallel</c> and it is written far more often than it is omitted, but the default
    /// matters: a frame whose style says nothing still pushes text aside, so treating the absence as
    /// <c>run-through</c> would draw the frame over the text it should have moved.
    /// </remarks>
    private TextWrap WrapOf(string? styleName)
        => _styles.ResolveProperty(
            styleName, OdfStyleFamily.Graphic, OdfPropertyKind.Graphic,
            OdfNamespaces.Style, "wrap").Value switch
        {
            "none" => TextWrap.None,
            "left" => TextWrap.Left,
            "right" => TextWrap.Right,
            "dynamic" => TextWrap.Dynamic,
            "run-through" => TextWrap.Through,
            _ => TextWrap.Parallel,
        };

    /// <summary>
    /// The gap the frame keeps between itself and the text, from its graphic style's margins.
    /// </summary>
    /// <remarks>
    /// <c>fo:margin</c> and its four sides, which on a graphic style mean the wrap distance rather than the
    /// space around a paragraph. Measured against LibreOffice's own render: a 5 cm frame at the left margin
    /// with a 0.2 cm right margin pushes text to 204.1 pt, which is 56.7 + 141.73 + 5.67 — so the margin is
    /// part of the region text avoids and not part of the frame.
    /// </remarks>
    private CellPadding FrameMargins(string? styleName)
    {
        Length Side(string name)
            => OdfWriterUnits.ToCore(
                   OdfValue.ParseLength(
                       _styles.ResolveProperty(
                           styleName, OdfStyleFamily.Graphic, OdfPropertyKind.Graphic,
                           OdfNamespaces.FoCompatible, name).Value))
               ?? Length.Zero;

        return new CellPadding(
            Side("margin-left"), Side("margin-right"), Side("margin-top"), Side("margin-bottom"));
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
    /// Reads each anchored note's body.
    /// </summary>
    /// <remarks>
    /// <para>
    /// After the citing paragraph's own walk rather than during it, because reading a note's body recurses
    /// into <see cref="Paragraph"/> — and doing that mid-walk would advance the note counter from inside the
    /// paragraph that cites the note, numbering a note in the body of note one as note two.
    /// </para>
    /// <para>
    /// The citation is placed at the start of the note's first paragraph, which is where LibreOffice draws
    /// it and where the file does not have it. An endnote is read the same way and marked, so that a caller
    /// need not guess later which of its notes collect at the end of the document.
    /// </para>
    /// </remarks>
    private List<PageNote> NotesOf(List<NoteAnchor> anchors)
    {
        if (anchors.Count == 0) return [];

        List<PageNote> notes = new(anchors.Count);

        foreach (NoteAnchor anchor in anchors)
        {
            XElement? body = anchor.Element.Element(XName.Get("note-body", OdfNamespaces.Text));
            if (body is null) continue;

            List<PageBlock> blocks = ReadNoteBody(body, anchor.Citation);
            if (blocks.Count == 0) continue;

            notes.Add(new PageNote
            {
                Blocks = blocks,
                Offset = anchor.Offset,
                IsEndnote = IsEndnote(anchor.Element),
                Placement = (IsEndnote(anchor.Element) ? _endnotes : _footnotes).Placement,
            });
        }

        return notes;
    }

    /// <summary>
    /// Reads a note's body, putting the citation at the start of its first paragraph.
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
            if (child.Name.Namespace == OdfNamespaces.Text
                && child.Name.LocalName is "p" or "h")
            {
                PageParagraph? paragraph = first
                    ? Paragraph(child, citation)
                    : Paragraph(child);

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
            if (range.IsCitation) style = AsCitation(style);

            OpenTypeFace face = Face(style) ?? paragraphFace;

            // The escapement is resolved here rather than where it was read, because its rise is a fraction
            // of the face's height and the face is only known now.
            Core.Units.Length size = style.Escapement.SizeOf(style.Size);
            Core.Units.Length rise = style.Escapement.RiseOf(face, style.Size);

            if (face != paragraphFace
                || size != paragraph.Size
                || style.Colour != paragraph.Colour
                || style.Language != paragraph.Language
                || rise != Core.Units.Length.Zero)
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
    /// An <em>anchor</em> citation's style, defaulted to superscript when the document names none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice draws the number in the sentence in its built-in <c>Footnote Anchor</c> character style,
    /// which carries an automatic superscript and which it does not write into the file — a saved document
    /// names no style on <c>text:note-citation</c> at all, and LibreOffice's own round trip of a document
    /// that <em>did</em> name one drops it. So a reader applying only what the file states draws the citation
    /// full size and on the baseline, where it fuses with the word before it and pushes the rest of the line
    /// along by the difference.
    /// </para>
    /// <para>
    /// The <em>note body's</em> number is deliberately not defaulted the same way, and that asymmetry is
    /// measured rather than assumed: Writer's other built-in style, <c>Footnote Characters</c>, states nothing
    /// at all (<c>DocumentStylePoolManager.cxx</c> falls straight through for it), and LibreOffice duly draws
    /// the number at the head of a note full size on the note's own baseline. Only the anchor is raised.
    /// </para>
    /// </remarks>
    private static OdfTextStyle AsCitation(OdfTextStyle style)
        => style.Escapement.IsNone ? style with { Escapement = Escapement.Superscript } : style;

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
    /// <param name="IsCitation">
    /// True for a note's citation, which is drawn superscript even when the document names no style for it.
    /// </param>
    private readonly record struct StyledRange(
        int Start, int Length, OdfStyleReference[] Cascade, bool IsCitation = false);

    /// <summary>A note found while walking a paragraph, before its body has been read.</summary>
    /// <param name="Offset">Where its citation sits in the paragraph's text.</param>
    /// <param name="Element">The <c>text:note</c> element, whose body is read separately.</param>
    /// <param name="Citation">The number it is cited by, counted across the document and already formatted.</param>
    private readonly record struct NoteAnchor(int Offset, XElement Element, string Citation);

    /// <summary>True when a <c>text:note</c> is an endnote rather than a footnote.</summary>
    /// <remarks>
    /// <c>text:note-class</c>, whose only other value is <c>footnote</c> — and which is worth reading rather
    /// than inferring, because the two are numbered by different sequences in different formats and placed on
    /// different pages.
    /// </remarks>
    private static bool IsEndnote(XElement note)
        => note.Attribute(XName.Get("note-class", OdfNamespaces.Text))?.Value == "endnote";

    /// <summary>
    /// How a note of each class is cited, which is not the same for the two.
    /// </summary>
    /// <remarks>
    /// Two sequences in two formats, from the document's <c>text:notes-configuration</c> where it has one and
    /// from LibreOffice's own defaults where it does not — footnotes 1, 2, 3 and endnotes i, ii, iii, which is
    /// measured rather than assumed.
    /// </remarks>
    /// <param name="isEndnote">True for an endnote.</param>
    /// <param name="index">How many notes of the class came before, counted from zero.</param>
    private string CitationOf(bool isEndnote, int index)
        => (isEndnote ? _endnotes : _footnotes).Citation(index);

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
        private readonly List<NoteAnchor> _notes = [];

        /// <summary>Creates a walker over a paragraph with a given style.</summary>
        /// <param name="paragraphStyleName">The paragraph's own style name, which roots the cascade.</param>
        /// <param name="citation">How a note of a class and an index is cited.</param>
        /// <param name="footnote">
        /// How many footnotes came before this paragraph. Passed in because notes are numbered across the
        /// document rather than within a paragraph, so the counters belong to the source.
        /// </param>
        /// <param name="endnote">How many endnotes came before it, counted separately.</param>
        internal RunWalker(
            string? paragraphStyleName,
            Func<bool, int, string> citation,
            int footnote = 0,
            int endnote = 0)
        {
            _cascade.Add(new OdfStyleReference(paragraphStyleName, OdfStyleFamily.Paragraph));
            _citationOf = citation;
            _footnote = footnote;
            _endnote = endnote;
        }

        /// <summary>How a note of a class and an index is cited, which the source resolves.</summary>
        /// <remarks>
        /// A delegate because the walker is nested but not owned: the numbering comes from the document's
        /// <c>text:notes-configuration</c>, which the source read, and a walker is built per paragraph.
        /// </remarks>
        private readonly Func<bool, int, string> _citationOf;

        private int _footnote;
        private int _endnote;

        /// <summary>How many footnotes this paragraph cited, which is what advances the source's counter.</summary>
        internal int FootnotesSeen { get; private set; }

        /// <summary>How many endnotes it cited.</summary>
        internal int EndnotesSeen { get; private set; }

        /// <summary>The paragraph's text.</summary>
        internal string Text => _builder.ToString();

        /// <summary>The ranges, in order, partitioning the text.</summary>
        internal IReadOnlyList<StyledRange> Ranges => _ranges;

        /// <summary>The notes anchored in the paragraph, with the offsets their citations occupy.</summary>
        internal List<NoteAnchor> Notes => _notes;

        /// <summary>The drawings anchored in this paragraph that float, in document order.</summary>
        internal List<FrameAnchorPoint> Frames { get; } = [];

        /// <summary>A drawing's <c>text:anchor-type</c>, defaulting the way ODF defaults it.</summary>
        /// <remarks>
        /// <c>paragraph</c> when absent, which is ODF's own default and also the commonest value written.
        /// </remarks>
        private static string AnchorTypeOf(XElement drawing)
            => drawing.Attribute(XName.Get("anchor-type", OdfNamespaces.Text))?.Value ?? "paragraph";

        /// <summary>Walks a <c>text:p</c> or <c>text:h</c>.</summary>
        /// <param name="paragraph">The paragraph element.</param>
        /// <param name="prefix">
        /// Text to emit before the paragraph's own, or null for none. What a footnote's citation is at the
        /// start of the note's first paragraph — emitted here rather than spliced in afterwards, because a
        /// prefix added later would shift every run's offset and every note anchor by its length.
        /// </param>
        /// <param name="prefixStyle">The character style the prefix is drawn in, if any.</param>
        internal void Walk(
            XElement paragraph, string? prefix = null, OdfStyleReference? prefixStyle = null)
        {
            if (!string.IsNullOrEmpty(prefix))
            {
                bool pushed = prefixStyle is { } style && !string.IsNullOrEmpty(style.Name);
                if (pushed) _cascade.Add(prefixStyle!.Value);

                Emit(prefix);

                if (pushed) _cascade.RemoveAt(_cascade.Count - 1);
            }

            Append(paragraph, depth: 0);
        }

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

                    // A drawing is not part of the paragraph's text and must not be walked into: a text box
                    // holds paragraphs of its own, and descending would splice its words into the middle of
                    // the sentence that anchors it. What it contributes is a *position* — and only when it
                    // is anchored as a character, which is the one kind that sits in the line.
                    case XElement child when child.Name.NamespaceName == OdfNamespaces.Draw
                                             && child.Name.LocalName
                                                 is "frame" or "custom-shape" or "g"
                                                 or "rect" or "line" or "polygon" or "path":
                        if (AnchorTypeOf(child) == "as-char") Emit(AnchorCharacter.ToString());
                        else Frames.Add(new FrameAnchorPoint(_builder.Length, child));
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

                case "note":
                    // A footnote's citation is drawn *in the sentence*, as a superscript, and again at the
                    // start of the note itself. So the anchor is not a bare placeholder the way a comment's
                    // is: it carries the citation's own text, under the citation's own character style,
                    // which is what makes it small and raised.
                {
                    bool isEndnote = IsEndnote(child);
                    string citation = _citationOf(isEndnote, isEndnote ? _endnote : _footnote);

                    _notes.Add(new NoteAnchor(_builder.Length, child, citation));
                    Citation(child, citation);

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

                case "annotation" or "annotation-end":
                    // A comment's anchor occupies a position and draws nothing; its body is another flow.
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

        /// <summary>
        /// Emits a note's citation at the anchor, under the citation's own character style.
        /// </summary>
        /// <remarks>
        /// The citation's <em>text</em> comes from the caller rather than from the file: LibreOffice ignores
        /// <c>text:note-citation</c>'s content and numbers the notes itself in document order, so a document
        /// stating 2 and 5 renders 1 and 2. The style does come from the file, since that is what makes the
        /// citation superscript — and a document that declares none falls back to an automatic superscript,
        /// which is what LibreOffice's built-in <c>Footnote Anchor</c> style carries.
        /// </remarks>
        /// <param name="note">The <c>text:note</c> element, for its citation's style.</param>
        /// <param name="text">The number the citation stands for, counted rather than read.</param>
        private void Citation(XElement note, string text)
        {
            XElement? citation = note.Element(XName.Get("note-citation", OdfNamespaces.Text));
            if (citation is null) return;

            string? styleName = citation
                .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

            bool pushed = !string.IsNullOrEmpty(styleName);
            if (pushed) _cascade.Add(new OdfStyleReference(styleName, OdfStyleFamily.Text));

            _inCitation = true;
            Emit(text);
            _inCitation = false;

            if (pushed) _cascade.RemoveAt(_cascade.Count - 1);
        }

        /// <summary>True while the citation's own text is being emitted.</summary>
        private bool _inCitation;

        /// <summary>Appends text under the cascade currently in force.</summary>
        private void Emit(string text)
        {
            if (text.Length == 0) return;

            _builder.Append(text);

            if (_ranges.Count > 0
                && _ranges[^1].IsCitation == _inCitation
                && SameCascade(_ranges[^1].Cascade))
            {
                _ranges[^1] = _ranges[^1] with { Length = _ranges[^1].Length + text.Length };
                return;
            }

            _ranges.Add(new StyledRange(
                _builder.Length - text.Length, text.Length, [.. _cascade], _inCitation));
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

/// <summary>One drawing found while walking a paragraph, with the offset it was found at.</summary>
/// <remarks>
/// The offset is kept even though only a character-anchored frame uses it, because that is the one thing
/// the walk knows and the resolution cannot recover: by the time the frames are turned into
/// <see cref="Layout.PageFrame"/>s the paragraph's text is one string and the element's place in it is gone.
/// </remarks>
/// <param name="Offset">Where in the paragraph's text the drawing was declared.</param>
/// <param name="Element">The <c>draw:</c> element itself.</param>
internal readonly record struct FrameAnchorPoint(int Offset, XElement Element);
