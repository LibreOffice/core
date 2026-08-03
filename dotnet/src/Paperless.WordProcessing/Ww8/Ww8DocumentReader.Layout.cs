using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Globalization;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Ww8;

/// <content>
/// The layout pass: the body's paragraphs with the formatting that decides where their text goes.
/// </content>
/// <remarks>
/// A second walk over the same character range the content pass reads, for the same reason the XML
/// formats have one — the content tree discards font sizes, indents and spacing because extraction must
/// not pay for them. Here it is a second walk rather than a second parse: the piece table, the
/// formatting tables and the stylesheet are all already built, so this costs a scan and a sprm decode
/// per paragraph.
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>How many paragraphs are read before the rest are ignored.</summary>
    public const int MaxLayoutParagraphs = 200000;

    /// <summary>
    /// One paragraph, with its text and the formatting layout needs.
    /// </summary>
    /// <param name="Text">The paragraph's text, without its terminating mark.</param>
    /// <param name="Format">Its resolved layout properties.</param>
    /// <param name="FamilyName">The family the document names, or null when it names none.</param>
    /// <param name="Size">The em size.</param>
    /// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
    /// <param name="IsItalic">True when the text is italic.</param>
    /// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
    /// <param name="IsInTable">
    /// True when the paragraph is inside a table. Tables are laid out as grids rather than as a run of
    /// paragraphs, so a caller filling pages skips these rather than stacking them.
    /// </param>
    /// <param name="Colour">The colour the paragraph's mark carries, or null for the automatic colour.</param>
    /// <param name="Runs">
    /// The stretches its character formatting divides it into, in order. Always populated, even where the
    /// whole paragraph is uniform — the layout source decides whether they are worth carrying, since it is
    /// the only party that can compare two <em>resolved</em> faces rather than two requested families.
    /// </param>
    /// <param name="SectionIndex">Which of the document's sections the paragraph sits in.</param>
    /// <param name="Notes">The notes anchored in the paragraph's text, or null when it cites none.</param>
    /// <param name="Frames">
    /// The floating shapes anchored in the paragraph's text, or null when none is. On the paragraph
    /// rather than on the document because that is where WW8 puts the anchor — an <c>FSPA</c> names a
    /// character position and nothing else — and because which page a shape lands on is decided by
    /// where its anchor paragraph lands, which is a pagination result rather than a property.
    /// </param>
    /// <param name="ListMarker">
    /// The label this item draws, or null when it draws none.
    /// <para>
    /// Computed rather than read: WW8 stores the template and the counters and never the rendered label,
    /// which is why it has to be produced by a walk in document order and why the same paragraph must not
    /// be described twice.
    /// </para>
    /// </param>
    /// <param name="ListFollow">
    /// The level's <c>ixchFollow</c>: what closes the gap between the label and the item's first word —
    /// 0 a tab to <paramref name="ListTabStop"/>, 1 a space, 2 nothing.
    /// </param>
    /// <param name="ListTabStop">Where that tab lands, in twips from the text area's start edge.</param>
    /// <param name="ListLabelSize">
    /// The size the level sets its label at, from <c>sprmCHps</c> in the level's <c>grpprlChpx</c>, or
    /// zero when the level states none and the label takes the item's own size. Regularly different
    /// from <paramref name="Size"/>, and then it makes the item's first line taller — see
    /// <see cref="Layout.PageParagraph.LabelRaisesFirstLine"/>.
    /// </param>
    public readonly record struct Ww8LayoutParagraph(
        int SectionIndex,
        string Text,
        Text.Layout.ParagraphFormat Format,
        string? FamilyName,
        Length Size,
        int Weight,
        bool IsItalic,
        string? Language,
        bool IsInTable,
        Colour? Colour = null,
        IReadOnlyList<Ww8LayoutRun>? Runs = null,
        IReadOnlyList<Ww8LayoutNote>? Notes = null,
        IReadOnlyList<Ww8LayoutFrame>? Frames = null,
        string? ListMarker = null,
        byte ListFollow = 2,
        int ListTabStop = 0,
        Length ListLabelSize = default)
    {
        /// <summary>
        /// True when <see cref="Text.Layout.ParagraphFormat.SpaceBefore"/> came from
        /// <c>sprmPFDyaBeforeAuto</c> rather than from a stated <c>dyaBefore</c>.
        /// </summary>
        /// <remarks>
        /// Carried because the suppression rules ask how the margin was arrived at rather than what it
        /// is: an auto margin is dropped at a cell's top edge and on a flow's first paragraph, and a
        /// stated margin of the same size is not.
        /// </remarks>
        public bool HasAutoSpaceBefore { get; init; }

        /// <inheritdoc cref="HasAutoSpaceBefore"/>
        public bool HasAutoSpaceAfter { get; init; }

        /// <summary>The list this paragraph belongs to, or zero when it belongs to none.</summary>
        /// <remarks>
        /// The <c>ilfo</c>, which is what LibreOffice compares between neighbours to decide whether an
        /// auto margin falls between two items of one list — where Word draws none — or between two
        /// lists, where it draws one.
        /// </remarks>
        public int ListRule { get; init; }

        /// <summary>
        /// True when the paragraph mark's own formatting asks for pair kerning.
        /// </summary>
        /// <remarks>
        /// The mark's, not the paragraph's — the same character format that supplies
        /// <see cref="Size"/> and <see cref="Language"/> — because it is what a paragraph with no runs
        /// of its own is set in, and what its label is drawn in.
        /// </remarks>
        public bool AutoKerning { get; init; }

        /// <summary>
        /// The text frame this paragraph asks to be part of, empty when it asks for none.
        /// </summary>
        /// <remarks>
        /// Kept on every paragraph rather than only on the ones that have one, because it is the
        /// <em>comparison</em> between neighbours that delimits a frame: a run of paragraphs stating the
        /// same non-empty position is one frame, and the first that differs starts another or ends it.
        /// </remarks>
        public Ww8TextFramePosition TextFrame { get; init; }

        /// <summary>
        /// The text frames this paragraph anchors, or null when it anchors none.
        /// </summary>
        /// <remarks>
        /// A frame's own paragraphs are taken out of the flow and its anchor becomes the first paragraph
        /// left after them — which is where Writer's insertion point ends up once <c>StopApo</c> has
        /// moved back out of the fly (<c>sw/source/filter/ww8/ww8par6.cxx:2674</c>).
        /// </remarks>
        public IReadOnlyList<Ww8LayoutTextFrame>? TextFrames { get; init; }
    }

    /// <summary>
    /// One stretch of a paragraph's text and the character formatting in force over it.
    /// </summary>
    /// <remarks>
    /// A CHPX covers a range of the file rather than of the paragraph, and one paragraph can span several
    /// — that is how WW8 stores a bold word. The ranges here are into the paragraph's <em>text</em>, which
    /// is not the same thing: an optional hyphen and a field marker occupy a character position in the file
    /// and none on the page, so each shifts everything after it.
    /// </remarks>
    /// <param name="Start">Its first character, as an index into the paragraph's text.</param>
    /// <param name="Length">How many characters it covers.</param>
    /// <param name="FamilyName">The family the document names, or null when it names none.</param>
    /// <param name="Size">The em size.</param>
    /// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
    /// <param name="IsItalic">True when the text is italic.</param>
    /// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
    /// <param name="Colour">The colour the text is drawn in, or null for the automatic colour.</param>
    /// <param name="Escapement">
    /// The superscript or subscript <c>sprmCIss</c> asks for, unresolved — its rise is a fraction of the
    /// face's height and this reader has no faces.
    /// </param>
    /// <param name="CaseMap">The case <c>sprmCFCaps</c> or <c>sprmCFSmallCaps</c> draws the run in.</param>
    /// <param name="Highlight">The band drawn behind the run, or null when it has none.</param>
    /// <param name="IsUnderlined">True when <c>sprmCKul</c> asks for a rule under the run.</param>
    /// <param name="IsStruckThrough">
    /// True when <c>sprmCFStrike</c> or <c>sprmCFDStrike</c> asks for one through it.
    /// </param>
    /// <param name="AutoKerning">
    /// True when <c>sprmCHpsKern</c> asks for the run's pairs to be kerned. Off unless it does.
    /// </param>
    public readonly record struct Ww8LayoutRun(
        int Start,
        int Length,
        string? FamilyName,
        Length Size,
        int Weight,
        bool IsItalic,
        string? Language,
        Colour? Colour,
        Layout.Escapement Escapement = default,
        Layout.PageCaseMap CaseMap = Layout.PageCaseMap.None,
        Colour? Highlight = null,
        bool IsUnderlined = false,
        bool IsStruckThrough = false,
        bool AutoKerning = false)
    {
        /// <summary>One past the run's last character.</summary>
        public int End => Start + Length;
    }

    /// <summary>The document's font table, read on demand.</summary>
    private Ww8FontTable? _fonts;

    /// <summary>The document's <c>Dop</c>, read on demand.</summary>
    private Ww8DocumentProperties? _properties;

    /// <summary>
    /// The document-wide layout decisions: the default tab interval and how spacings combine.
    /// </summary>
    /// <remarks>
    /// Read on demand and cached, because extraction never asks — and a document whose <c>Dop</c> is
    /// missing gets the defaults rather than an exception, which is the same leniency every other table
    /// gets here.
    /// </remarks>
    public Ww8DocumentProperties DocumentProperties =>
        _properties ??= _fib.Has(Ww8FibTable.DocumentProperties)
            ? Ww8DocumentProperties.Parse(Slice(Ww8FibTable.DocumentProperties))
            : Ww8DocumentProperties.Default;

    /// <summary>The families the document's <c>sprmCRgFtc0</c> indexes name.</summary>
    public Ww8FontTable Fonts =>
        _fonts ??= _fib.Has(Ww8FibTable.FontNames)
            ? Ww8FontTable.Parse(Slice(Ww8FibTable.FontNames))
            : Ww8FontTable.Empty;

    /// <summary>
    /// Reads the body's paragraphs with the formatting layout needs.
    /// </summary>
    /// <remarks>
    /// The body range only. The headers and footers are read by <see cref="ReadLayoutFurniture"/>; a note
    /// arrives hanging off the paragraph that cites it, since that is what decides which page carries it.
    /// </remarks>
    public List<Ww8LayoutBlock> ReadLayoutBlocks()
    {
        _layoutNoteNumber = 0;
        _layoutEndnoteNumber = 0;

        // The body is where the document's lists start counting, and the numbering is the same instance
        // the extraction pass advances — so it is reset rather than assumed clean.
        _numbering.ResetCounters();

        return ReadLayoutBlocks(Ranges.Body, keepTrailingEmpty: true);
    }

    /// <summary>
    /// The next number of a note's class, formatted the way LibreOffice formats that class.
    /// </summary>
    /// <remarks>
    /// Two sequences in two formats, from the DOP where the document states them and from LibreOffice's own
    /// defaults where it does not — footnotes 1, 2, 3 and endnotes i, ii, iii. The DOP's
    /// <c>rncFootnote</c>/<c>rncEdn</c> restart rules are still unread, because a restart has to be applied
    /// while pages are being filled rather than while the document is read.
    /// </remarks>
    /// <param name="isEndnote">True for an endnote.</param>
    private string CitationOf(bool isEndnote)
        => isEndnote
            ? DocumentProperties.EndnoteNumbering.Citation(_layoutEndnoteNumber++)
            : DocumentProperties.FootnoteNumbering.Citation(_layoutNoteNumber++);

    /// <summary>How many endnotes the layout walk has passed.</summary>
    private int _layoutEndnoteNumber;

    /// <summary>How many notes the layout walk has passed, which is what numbers the next one.</summary>
    /// <remarks>
    /// Its own counter rather than the content pass's, because the two walk the same text independently and
    /// sharing one would make each run's numbering depend on whether the other had run first.
    /// </remarks>
    private int _layoutNoteNumber;

    /// <summary>
    /// The notes anchored in the paragraph being read, waiting for it to close.
    /// </summary>
    /// <remarks>
    /// A field rather than a local because the reference and the paragraph's end are found by the same
    /// single pass over the text, several characters apart.
    /// </remarks>
    private readonly List<Ww8LayoutNote> _pendingNotes = [];

    /// <summary>
    /// Where each note's text is, by the body position of the reference that cites it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two PLCFs make this, and both are needed: <c>PlcffndRef</c> gives the body positions of the footnote
    /// references and <c>PlcffndTxt</c> the extents of their texts, with the <em>n</em>th reference owning
    /// the <em>n</em>th text. The endnote pair is the same arrangement over a different subdocument, which is
    /// what tells the two kinds of note apart — the reference character in the body is the same U+0002 for
    /// both, so nothing in the text says which it is.
    /// </para>
    /// <para>
    /// Built once and cached, because it is walked per reference and a document can have hundreds.
    /// </para>
    /// </remarks>
    private Dictionary<int, (Ww8Range Text, bool IsEndnote)>? _noteTexts;

    /// <inheritdoc cref="_noteTexts"/>
    private Dictionary<int, (Ww8Range Text, bool IsEndnote)> NoteTexts
    {
        get
        {
            if (_noteTexts is not null) return _noteTexts;

            _noteTexts = [];
            Ww8Ranges ranges = Ranges;

            Collect(ranges.Footnotes, Ww8FibTable.FootnoteReferences, Ww8FibTable.FootnoteTexts, false);
            Collect(ranges.Endnotes, Ww8FibTable.EndnoteReferences, Ww8FibTable.EndnoteTexts, true);

            return _noteTexts;

            void Collect(
                Ww8Range subdocument, Ww8FibTable references, Ww8FibTable boundaries, bool isEndnote)
            {
                // A reference record is a two-byte FRD, and the positions are what matter.
                Ww8Plcf reference = PlcfOf(references, recordSize: 2);
                List<Ww8Range> texts = [.. SplitSubdocument(subdocument, boundaries)];

                for (int i = 0; i < reference.Count && i < texts.Count; i++)
                {
                    _noteTexts![reference.Positions[i]] = (texts[i], isEndnote);
                }
            }
        }
    }

    /// <summary>
    /// One range's paragraphs only, for a flow that has no room for a table.
    /// </summary>
    /// <remarks>
    /// A header, a footer or a cell is laid out by <c>FlowLayouter</c>, which stacks paragraphs and knows
    /// nothing of grids — so a table inside one is dropped. Filtered from the block walk rather than read by
    /// a walk of its own, because a second walk would be a second place for the run and tab handling to be
    /// got right.
    /// </remarks>
    private List<Ww8LayoutParagraph> ReadLayoutParagraphs(Ww8Range range, bool keepTrailingEmpty)
        => [.. ReadLayoutBlocks(range, keepTrailingEmpty)
            .Where(block => block.Paragraph is not null)
            .Select(block => block.Paragraph!.Value)];

    /// <summary>
    /// The first section's headers and footers, with the formatting layout needs.
    /// </summary>
    /// <remarks>
    /// <para>
    /// DOC keeps its furniture in the header subdocument as a flat run of stories: six that are not
    /// furniture at all — the footnote and endnote separators, continuation separators and continuation
    /// notices — and then six per section, in a fixed order. The order is the whole mapping, and it does
    /// not match the order the other formats use, so it is spelled out in <see cref="FurnitureSlots"/>.
    /// </para>
    /// <para>
    /// Word writes all six whether the section uses them or not, so most hold nothing but a paragraph
    /// mark; an empty story therefore means "this section has no such header" rather than "it has an empty
    /// one", and filling the slot with the empty paragraph would draw a blank line on every page and push
    /// nothing anywhere. Emptiness is the only thing distinguishing the two.
    /// </para>
    /// <para>
    /// Six stories per section, so the section's own six start six further along for each section before
    /// it — which is what makes a document with a landscape appendix able to give that appendix its own
    /// running head.
    /// </para>
    /// </remarks>
    /// <param name="section">Which section's furniture to read.</param>
    public Ww8LayoutFurniture ReadLayoutFurniture(int section = 0)
    {
        List<Ww8Range> stories = [.. SplitSubdocument(Ranges.Headers, Ww8FibTable.HeaderTexts)];

        Dictionary<Model.PageFurnitureSlot, List<Ww8LayoutBlock>> headers = [];
        Dictionary<Model.PageFurnitureSlot, List<Ww8LayoutBlock>> footers = [];

        for (int slot = 0; slot < FurnitureSlots.Length; slot++)
        {
            int story = SeparatorStories + (Math.Max(0, section) * FurnitureSlots.Length) + slot;
            if (story >= stories.Count) break;
            if (stories[story].Length <= 0) continue;

            // Each flow numbers its own lists: a numbered paragraph in a running head does not continue
            // the body's count.
            _numbering.ResetCounters();

            // Blocks rather than paragraphs, so a table in a running head survives: a two-part running head
            // is a two-cell table, and stacking its cells as loose paragraphs would give the header a height
            // no table has and push the body text down by the difference on every page.
            List<Ww8LayoutBlock> blocks = ReadLayoutBlocks(stories[story], keepTrailingEmpty: false);

            // Word writes all six stories whether the section uses them or not, so an empty paragraph is a
            // placeholder rather than a blank line — but only when there is nothing else in the story.
            //
            // "Nothing else" has to include the shapes anchored in it. A running head that is one logo and
            // no words is an ordinary thing, and its paragraph reads back with no text at all: the U+0001
            // that stood for the picture is consumed by the frame it made (see CollectFrame), so testing
            // the text alone throws the whole header away and leaves the body starting at the top margin.
            blocks.RemoveAll(
                block => block.Paragraph is { Text.Length: 0, Frames: null or { Count: 0 } }
                    && block.Table is null);

            if (blocks.Count == 0) continue;

            (bool isHeader, Model.PageFurnitureSlot which) = FurnitureSlots[slot];
            (isHeader ? headers : footers)[which] = blocks;
        }

        return new Ww8LayoutFurniture(headers, footers);
    }

    /// <summary>How many stories precede the first section's furniture in the header subdocument.</summary>
    private const int SeparatorStories = 6;

    /// <summary>
    /// What each of a section's six header stories is, in the order DOC writes them.
    /// </summary>
    /// <remarks>
    /// Even before odd, headers before footers, and the first page's pair last — so the odd header is the
    /// <em>default</em> slot, since a document's first page is a right-hand page and the odd stories are
    /// what every page takes that no other story claims.
    /// </remarks>
    private static readonly (bool IsHeader, Model.PageFurnitureSlot Slot)[] FurnitureSlots =
    [
        (true, Model.PageFurnitureSlot.Even),
        (true, Model.PageFurnitureSlot.Default),
        (false, Model.PageFurnitureSlot.Even),
        (false, Model.PageFurnitureSlot.Default),
        (true, Model.PageFurnitureSlot.First),
        (false, Model.PageFurnitureSlot.First),
    ];

    /// <summary>
    /// Reads one range's paragraphs with the formatting layout needs.
    /// </summary>
    /// <param name="body">The range to read, which is the body's for the body pass.</param>
    /// <param name="keepTrailingEmpty">
    /// True to close a final paragraph that no paragraph mark closed, and to yield one empty paragraph for
    /// a range that produced none at all. Right for the body — a document has at least one paragraph
    /// whatever its text says — and wrong for furniture, where nothing to lay out has to stay
    /// distinguishable from one blank line to lay out.
    /// </param>
    /// <param name="noteCitation">
    /// The number a note's own mark stands for, when the range being read <em>is</em> a note's — the mark at
    /// the head of a note repeats the number of the reference that cites it rather than taking a fresh one.
    /// Null for the body, where each reference advances the counter.
    /// </param>
    /// <param name="allowTextFrames">
    /// False inside a text box's own story, where Word ignores a frame the paragraph properties ask for
    /// — <c>SwWW8ImplReader::TestApo</c> declines outright when <c>m_bTxbxFlySection</c>
    /// (<c>sw/source/filter/ww8/ww8par2.cxx:404</c>), and its comment says why: "word appears to ignore
    /// them if inside a text autoshape".
    /// </param>
    private List<Ww8LayoutBlock> ReadLayoutBlocks(
        Ww8Range body,
        bool keepTrailingEmpty,
        string? noteCitation = null,
        bool allowTextFrames = true)
    {
        LayoutTableAssembler assembler = new();
        if (body.Length <= 0) return assembler.Finished();

        string text = _pieces.ReadText(body.Start, body.End, _diagnostics);
        if (text.Length == 0) return assembler.Finished();

        int emitted = 0;
        StringBuilder current = new();

        // The source position of each character in `current`. A paragraph's text is not a slice of the
        // file's — optional hyphens and field markers are dropped — so a run's range cannot be recovered
        // from the offsets afterwards and has to be carried alongside.
        List<int> positions = [];
        int start = 0;

        // How many fields deep the walk is inside an *instruction*. A field's text holds both halves —
        // the instruction and the cached result — separated by a U+0014, and only the second is shown.
        int instruction = 0;

        // The fields the walk is inside, innermost last, by type. Separate from `instruction`, which
        // counts only the hidden half: a shape sits in the *result*, where `instruction` is already
        // back to nought and the field is still open. Nested fields are why it is a stack and not a
        // single value — `IsInlineEscherHack` asks about the innermost one alone.
        (Ww8FieldTypes fieldTypes, int fieldBase) = FieldTypesOf(body);
        Stack<int> openFields = new();

        // How many enclosing fields had their result replaced by a computed one, and which they were.
        // A count for the same reason `instruction` is one — fields nest, and the result of an outer
        // field can contain a whole inner field whose characters are equally not to be drawn — with the
        // stack beside it so that the right field's end is the one that stops the suppression.
        int computed = 0;
        Stack<bool> replacedFields = new();

        // Where the last U+000C fell, until the paragraph after it has been read and can be asked whether
        // it starts a new section. Null everywhere else, which is every paragraph of a document that has
        // neither a section break nor a hard page break in it.
        int? pageBreakAt = null;

        // Whether the character just read closed a paragraph — LibreOffice's <c>m_bWasParaEnd</c>. False
        // at the start, which is what makes a range beginning with a U+000C still yield its first
        // paragraph.
        bool wasParagraphMark = false;

        for (int index = 0; index < text.Length && emitted < MaxLayoutParagraphs; index++)
        {
            char character = text[index];
            int position = body.Start + index;

            // LibreOffice's `m_bWasParaEnd`, which it recomputes on every character read
            // (`ww8par.cxx`:3714) and which decides whether a U+000C ends a paragraph of its own.
            bool afterParagraphMark = wasParagraphMark;
            wasParagraphMark = character is ParagraphMark or CellMark;

            // Everything between a field's start and its separator is its instruction and is not drawn.
            // Not a refinement: LibreOffice's own DOC export writes a picture as a SHAPE field, so
            // keeping the instruction puts the literal word "SHAPE" into the sentence — measured on
            // `picture-flow.doc`, where it made our word count 191 against the reference's 190. The
            // markers themselves are handled below and carry no width either way.
            if (instruction > 0 && character is not (Special.FieldBegin or Special.FieldSeparator
                or Special.FieldEnd or ParagraphMark or Special.SectionMark or CellMark))
            {
                continue;
            }

            // The cached result of a field this reader computes itself is suppressed the same way, and
            // for a sharper reason: the cache is what the field said when the document was last saved,
            // and a FILENAME's is stale the moment the file is renamed. Both corpus documents carrying
            // one are wrong today — `DEP2008-1900.doc`'s reads "EMS-P16 Travel Procedure (3)EMS-P16
            // Travel Procedure (2)", a doubled string from some earlier save, where LibreOffice draws
            // `DEP2008-1900.doc`. The replacement was written at the separator, below.
            if (computed > 0 && character is not (Special.FieldBegin or Special.FieldSeparator
                or Special.FieldEnd or ParagraphMark or Special.SectionMark or CellMark))
            {
                continue;
            }

            switch (character)
            {
                case ParagraphMark or Special.SectionMark:
                    // A U+000C ends a paragraph only when one is under way. `HandlePageBreakChar`
                    // (`ww8par.cxx`:3438) adds a paragraph end exactly when the character before it was
                    // not one — `if (!m_bWasParaEnd && IsTemp)` — and otherwise lets the break settle on
                    // the empty paragraph already open. Closing one unconditionally put a blank line
                    // above every hard page break and every section break in the document, which is a
                    // line's worth of height per break and eventually a page.
                    if (character == ParagraphMark || !afterParagraphMark)
                    {
                        Close(position, endsCell: false);
                    }

                    // A U+000C is *either* a section break or a hard page break, and WW8 says which only
                    // by whether a section ends at this position. LibreOffice looks the position up in the
                    // section PLCF and, finding no section boundary, inserts an ordinary
                    // `SvxBreak::PageBefore` on the paragraph that follows
                    // (`SwWW8ImplReader::ReadText`, `ww8par.cxx`:4097, after `HandlePageBreakChar` set
                    // `m_bPgSecBreak`). Without this, every Ctrl+Enter in a DOC is silently dropped —
                    // the paragraph after it simply carries on down the same page.
                    // The lookup is done as "does the next paragraph belong to another section", which is
                    // the same question without the character-position arithmetic: a section-terminating
                    // U+000C is the last character of its section, so the mark after it is in the next one.
                    pageBreakAt = character == Special.SectionMark ? position : null;
                    start = index + 1;
                    continue;

                case CellMark:
                    // A cell boundary — or a row's, which only the paragraph's own sprmPFTtp
                    // distinguishes. The assembler decides; the character alone cannot.
                    Close(position, endsCell: true);
                    start = index + 1;
                    continue;

                case Special.LineBreak:
                    Emit(current, positions, LineSeparator, position);
                    continue;

                case Special.Tab:
                    Emit(current, positions, '\t', position);
                    continue;

                case Special.NonBreakingHyphen:
                    Emit(current, positions, NonBreakingHyphen, position);
                    continue;

                case Special.OptionalHyphen:
                    continue;

                case Special.FieldBegin:
                    // Nested fields are legal and Word writes them — a hyperlink around a cross
                    // reference is two — so this counts rather than toggling.
                    instruction++;
                    openFields.Push(fieldTypes.At(position - fieldBase) ?? 0);
                    replacedFields.Push(false);
                    continue;

                case Special.FieldSeparator:
                {
                    // The instruction ends and the cached result begins. A field with no separator has
                    // no result, and its instruction stays hidden until its end.
                    if (instruction > 0) instruction--;

                    // The one point at which a computed field can be written: the instruction has been
                    // read, so the field's type is known, and the result it is replacing starts here.
                    if (instruction == 0
                        && computed == 0
                        && openFields.Count > 0
                        && openFields.Peek() == Ww8FieldTypes.FileName
                        && FileName is { Length: > 0 } name)
                    {
                        foreach (char letter in name) Emit(current, positions, letter, position);

                        replacedFields.Pop();
                        replacedFields.Push(true);
                        computed++;
                    }

                    continue;
                }

                case Special.FieldEnd:
                    if (instruction > 0) instruction--;
                    if (openFields.Count > 0) openFields.Pop();
                    if (replacedFields.Count > 0 && replacedFields.Pop() && computed > 0) computed--;
                    continue;

                case Special.AutoNumberedReference:
                {
                    // A note's mark, in the body or inside the note itself. WW8 does not store the number:
                    // Word computes it and so does this, which is also why the same character serves both
                    // places — the mark at the head of a note repeats the citing number rather than taking a
                    // fresh one, so only a reference in the body advances the counter.
                    // Which kind of note this is comes from the reference tables rather than from the text:
                    // the character is the same U+0002 for both, and the two kinds are numbered by separate
                    // sequences in different formats. So the lookup happens *before* the number is formatted.
                    (Ww8Range Text, bool IsEndnote) note = default;
                    bool found = noteCitation is null
                        && NoteTexts.TryGetValue(position, out note);

                    string citation = noteCitation is { } repeated
                        ? repeated
                        : CitationOf(found && note.IsEndnote);

                    // Emitted at the reference's own position, so that the CHPX covering it governs the run.
                    // Word writes the mark with a character style carrying sprmCIss, which is what makes it
                    // superscript — the same arrangement the other three formats have, reached differently.
                    foreach (char digit in citation) Emit(current, positions, digit, position);

                    if (found)
                    {
                        List<Ww8LayoutBlock> read = ReadNoteBody(note.Text, citation);
                        if (read.Count > 0)
                        {
                            _pendingNotes.Add(
                                new Ww8LayoutNote(
                                    current.Length - citation.Length,
                                    note.IsEndnote,
                                    read,
                                    note.IsEndnote
                                        ? DocumentProperties.EndnoteNumbering.Placement
                                        : DocumentProperties.FootnoteNumbering.Placement,
                                    note.IsEndnote
                                        ? DocumentProperties.EndnoteNumbering.Restart
                                        : DocumentProperties.FootnoteNumbering.Restart)
                                {
                                    Numbering = note.IsEndnote
                                        ? DocumentProperties.EndnoteNumbering
                                        : DocumentProperties.FootnoteNumbering,
                                    Citation = citation,
                                });
                        }
                    }

                    continue;
                }

                case Special.Picture or Special.DrawnObject or Special.AnnotationReference:
                    // Collected before the character is considered, so that the frame's offset is where
                    // the anchor sits rather than one past it — which is what an as-character frame
                    // needs and what a character origin measures from.
                    //
                    // A character stands in the text only when nothing was made of it. LibreOffice does
                    // the same and states why: `if (!pResult) cInsert = ' '` (ww8par.cxx:3637) — a
                    // graphic that arrives replaces the U+0001 entirely, and only one that fails leaves
                    // something behind, so that a document with a missing picture still has a word gap
                    // where the picture was. A comment's U+0005 makes no frame and so keeps its
                    // placeholder, which is what the mark tables index it by.
                    //
                    // Keeping the character for a frame that *did* arrive is not free, and this is what
                    // it cost: `word-features.doc` writes its text box as the pair U+0008 U+0001, and
                    // the two shaped to 18.67 pt of .notdef between "Before the box." and "After the
                    // box." — invisible while the box was misplaced far to the left, and exactly the
                    // width by which the sentence overshot once the box was put in the right place.
                    if (CollectFrame(position, current.Length)) continue;

                    // The other half of the same rule, and it needs one character of lookahead.
                    // `ww8par.cxx:3602` reads a U+0001 inside a SHAPE field as the shape's own
                    // placeholder — the shape itself was written at the U+0008 just before it — and
                    // imports nothing for it, leaving `cInsert` at nought so no character is inserted
                    // either. The lookahead is what distinguishes it from the case the comment beside
                    // it names: "in a special case, the code is 0x1 0x1, which yields a simple
                    // picture", where the pair really is a picture and the first of them stands for it.
                    if (character == Special.Picture
                        && openFields.Count > 0 && openFields.Peek() == Ww8FieldTypes.Shape
                        && (index + 1 >= text.Length || text[index + 1] != Special.Picture))
                    {
                        continue;
                    }

                    Emit(current, positions, AnchorCharacter, position);
                    continue;

                default:
                    if (character >= ' ' || character == '')
                    {
                        Emit(current, positions, character, position);
                    }
                    continue;
            }
        }

        if (current.Length > 0 || (keepTrailingEmpty && emitted == 0))
        {
            Close(body.End - 1, endsCell: false);
        }

        List<Ww8LayoutBlock> finished = allowTextFrames
            ? LiftTextFrames(assembler.Finished())
            : assembler.Finished();
        SuppressAutoSpacing(finished);
        return finished;

        // One paragraph, handed to the assembler with the properties of the mark that ended it — which is
        // what says whether it was in a table, whether the mark closed a row, and what that row's columns
        // are. Resolved once here rather than twice, since Describe needs the same lookup.
        void Close(int markPosition, bool endsCell)
        {
            Ww8ParagraphFormat format = ResolveParagraphFormat(markPosition);

            Ww8LayoutParagraph paragraph =
                Describe(current.ToString(), positions, body.Start + start, markPosition) with
                {
                    Notes = _pendingNotes.Count == 0 ? null : [.. _pendingNotes],
                    Frames = _pendingFrames.Count == 0 ? null : [.. _pendingFrames],
                };

            // The U+000C above this paragraph was a hard page break rather than a section boundary, so
            // this paragraph starts a page. Not inside a table: "#i1909# section/page breaks should not
            // occur in tables, word itself ignores them in this case" — `HandlePageBreakChar` declines
            // outright when `m_nInTable`.
            if (pageBreakAt is { } breakAt
                && !paragraph.IsInTable
                && SectionAt(breakAt) == paragraph.SectionIndex)
            {
                paragraph = paragraph with
                {
                    Format = paragraph.Format with { StartsNewPage = true },
                };
            }

            pageBreakAt = null;

            assembler.Add(paragraph, format, endsCell);

            current.Clear();
            positions.Clear();
            _pendingNotes.Clear();
            _pendingFrames.Clear();
            emitted++;
        }

        // The shape anchored at a character position, if one is. Reading its own text here rather than
        // when the frame is built keeps the recursion inside the walk that already handles it: a text
        // box's story goes through ReadLayoutBlocks exactly as a note's body does, which is what makes a
        // table inside a text box work without a second path. Returns whether a frame was made, which is
        // what decides whether the anchor character stays in the text.
        bool CollectFrame(int position, int offset)
        {
            if (Drawings.AnchorAt(position) is not { } anchor)
            {
                // No FSPA, which for a U+0001 means an inline picture: its run states a
                // sprmCPicLocation instead, and nothing in the anchor table mentions it at all.
                if (InlinePicture(position, offset) is not { } inline) return false;

                _pendingFrames.Add(inline);
                return true;
            }

            MsBinary.Escher.EscherShape? shape = Drawings.Shape(anchor.ShapeId);
            _pendingFrames.Add(
                new Ww8LayoutFrame(anchor, shape, offset, ReadShapeText(shape, anchor.IsHeaderAnchor))
                {
                    Picture = PictureOf(shape),

                    // SwWW8ImplReader::IsInlineEscherHack, ww8par.hxx:1737 — the innermost open field
                    // being a SHAPE is the whole of the test, and ww8graf.cxx:2355 then anchors the
                    // shape FLY_AS_CHAR instead of FLY_AT_CHAR. This is how Word writes a picture that
                    // sits in the run of text: it still gets an FSPA, and the field around it is the
                    // only thing that says the FSPA's position is not to be believed.
                    IsSetInLine = openFields.Count > 0 && openFields.Peek() == Ww8FieldTypes.Shape,
                });

            return true;
        }
    }

    /// <summary>
    /// The floating shapes anchored in the paragraph being read, waiting for it to close.
    /// </summary>
    /// <inheritdoc cref="_pendingNotes"/>
    private readonly List<Ww8LayoutFrame> _pendingFrames = [];

    /// <summary>
    /// A shape's own text, from the text-box subdocument its <c>lTxid</c> indexes.
    /// </summary>
    /// <remarks>
    /// Two subdocuments, chosen by which <c>PlcSpa</c> the anchor came from: a text box in a running
    /// head has its text in <c>PlcfHdrtxbxTxt</c> and not in the body's table, and a document with a
    /// text box in its header therefore has an <em>empty</em> body text-box subdocument. Reading the
    /// wrong one gives a header's box no text and, in a document with both, gives it another box's.
    /// </remarks>
    private List<Ww8LayoutBlock> ReadShapeText(MsBinary.Escher.EscherShape? shape, bool isHeader)
    {
        int story = Ww8Frames.TextStoryIndex(shape);
        if (story < 0) return [];

        // A text box whose own story anchors a shape naming that story again would recur without
        // bound. Nothing legitimate does it, but the index comes from the file rather than from the
        // structure, so a document can say it — and the guard costs one set membership per shape.
        if (!_openStories.Add((isHeader, story))) return [];

        try
        {
            return ReadShapeTextCore(story, isHeader);
        }
        finally
        {
            _openStories.Remove((isHeader, story));
        }
    }

    /// <summary>Which text-box stories the walk is already inside, guarding a self-referential index.</summary>
    private readonly HashSet<(bool IsHeader, int Story)> _openStories = [];

    /// <inheritdoc cref="ReadShapeText"/>
    private List<Ww8LayoutBlock> ReadShapeTextCore(int story, bool isHeader)
    {
        (Ww8Range subdocument, Ww8FibTable boundaries) = isHeader
            ? (Ranges.HeaderTextBoxes, Ww8FibTable.HeaderTextBoxTexts)
            : (Ranges.TextBoxes, Ww8FibTable.TextBoxTexts);

        List<Ww8Range> stories =
            [.. SplitSubdocument(subdocument, boundaries, recordSize: TextBoxRecordSize)];
        if (story >= stories.Count) return [];

        // Saved and restored for the same reason a note's body is: the paragraph that anchors this
        // shape is still open, and the shape's own paragraphs must not walk off with the frames and
        // notes it has collected so far.
        List<Ww8LayoutNote> outerNotes = [.. _pendingNotes];
        List<Ww8LayoutFrame> outerFrames = [.. _pendingFrames];
        _pendingNotes.Clear();
        _pendingFrames.Clear();

        List<Ww8LayoutBlock> blocks =
            ReadLayoutBlocks(stories[story], keepTrailingEmpty: false, allowTextFrames: false);

        _pendingNotes.Clear();
        _pendingNotes.AddRange(outerNotes);
        _pendingFrames.Clear();
        _pendingFrames.AddRange(outerFrames);

        return blocks;
    }

    /// <summary>
    /// Reads a note's body from the note subdocument.
    /// </summary>
    /// <remarks>
    /// The same walk the body takes, which is what makes a table inside a note work — and the citation is
    /// handed in rather than counted, because the mark at the head of a note repeats the number of the
    /// reference that cites it. Recursion is bounded by the ranges themselves: a note's text is in a
    /// different subdocument than the body, so a note cannot contain its own reference.
    /// </remarks>
    /// <param name="range">The note's extent in the note subdocument.</param>
    /// <param name="citation">The number the note is cited by.</param>
    private List<Ww8LayoutBlock> ReadNoteBody(Ww8Range range, string citation)
    {
        // Saved and restored: the body's own half-built paragraph is still open, and a note read from inside
        // it must not walk off with the notes that paragraph has collected so far.
        List<Ww8LayoutNote> outer = [.. _pendingNotes];
        _pendingNotes.Clear();

        List<Ww8LayoutBlock> blocks =
            ReadLayoutBlocks(range, keepTrailingEmpty: false, noteCitation: citation);

        _pendingNotes.Clear();
        _pendingNotes.AddRange(outer);

        return blocks;
    }

    /// <summary>
    /// Appends one character, recording where in the file it came from.
    /// </summary>
    /// <remarks>
    /// The pairing is what lets a CHPX's file range become a range of the paragraph's text. Appending to
    /// the builder directly anywhere would silently desynchronise the two and misattribute a run's
    /// formatting to the text beside it.
    /// </remarks>
    private static void Emit(
        StringBuilder text, List<int> positions, char character, int position)
    {
        text.Append(character);
        positions.Add(position);
    }

    /// <summary>
    /// The character a line break becomes: U+2028, whose UAX #14 class is a mandatory break.
    /// </summary>
    private const char LineSeparator = '\u2028';

    /// <summary>The non-breaking hyphen a WW8 U+001E becomes.</summary>
    private const char NonBreakingHyphen = '\u2011';

    /// <summary>The character an anchor occupies, matching the other formats' readers.</summary>
    private const char AnchorCharacter = '\u0001';

    /// <summary>
    /// Resolves one paragraph's formatting from its mark's position.
    /// </summary>
    /// <remarks>
    /// The mark's position, not the paragraph's start, because that is where WW8 stores a paragraph's
    /// properties — the PAPX describing a paragraph is found by looking up the character position of the
    /// mark that ends it. Looking up the first character instead finds the <em>previous</em> paragraph's
    /// properties, which is a mistake that produces a document formatted one paragraph out of step.
    /// </remarks>
    private Ww8LayoutParagraph Describe(
        string text, List<int> positions, int start, int markPosition)
    {
        Ww8LayoutFormat layout = ResolveLayoutFormat(markPosition);
        Ww8ParagraphFormat paragraph = ResolveParagraphFormat(markPosition);

        // The run properties at the paragraph's mark, which is what its mark carries and what an empty
        // paragraph is as tall as. The text's own formatting comes from the runs below.
        Ww8LayoutFormat character = ResolveCharacterLayout(
            Math.Min(Math.Max(start, 0), Math.Max(markPosition, 0)));

        Length size = SizeOf(character);

        Text.Layout.ParagraphFormat format = layout.ToParagraphFormat(size) with
        {
            DefaultTabInterval = DocumentProperties.DefaultTabInterval,

            // Word measures its tab stops from the text area rather than from the paragraph's
            // indent, which is what `ww8par.cxx` records by clearing TABS_RELATIVE_TO_INDENT.
            TabsRelativeToIndent = false,
        };

        // The level is looked up whether or not it draws a label, because a continuation paragraph of a
        // list item is written with the same indents and no marker.
        Ww8ListLevel? level = paragraph.ListNumber > 0
            ? _numbering.FindLevel(paragraph.ListNumber, paragraph.ListLevel ?? 0)
            : null;

        if (level is { } stated) format = WithListIndents(format, stated, layout, markPosition);

        return new Ww8LayoutParagraph(
            SectionAt(markPosition),
            text,
            format,
            character.FontIndex is { } index ? Fonts.Name(index) : null,
            size,
            character.IsBold == true ? 700 : 400,
            character.IsItalic == true,
            LanguageOf(character),
            paragraph.IsInTable,
            character.Colour,
            ReadRuns(text, positions, markPosition),
            ListMarker: LabelAt(paragraph),
            ListFollow: level?.Follow ?? LabelFollowsWithNothing,
            ListTabStop: level?.TabPosition ?? 0,
            ListLabelSize: level is { HalfPointSize: > 0 } sized
                ? Length.FromPoints(sized.HalfPointSize / 2.0)
                : default)
        {
            HasAutoSpaceBefore = layout.HasAutoSpaceBefore ?? false,
            HasAutoSpaceAfter = layout.HasAutoSpaceAfter ?? false,
            ListRule = paragraph.ListNumber,
            AutoKerning = character.AutoKerning ?? false,

            // Not for a paragraph in a table: Word applies a frame to a whole row or to nothing, and
            // LibreOffice declines the test outright unless the paragraph is the first in the first cell
            // (`SwWW8ImplReader::TestApo`, ww8par2.cxx:440). Declining for every cell paragraph is the
            // conservative half of that and leaves a table where the document put it.
            TextFrame = paragraph.IsInTable
                ? Ww8TextFramePosition.None
                : ResolveTextFrame(markPosition),
        };
    }

    /// <summary>
    /// The <c>ixchFollow</c> a paragraph with no level of its own is given: nothing at all.
    /// </summary>
    /// <remarks>
    /// Two rather than nought, because nought means a tab and a paragraph whose level could not be found
    /// has no stop to aim at — so the label would be pushed to the paragraph's own indent for reasons the
    /// document never stated.
    /// </remarks>
    private const byte LabelFollowsWithNothing = 2;

    /// <summary>
    /// The label a paragraph's list level draws, advancing that level's counter.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A side effect, so it belongs where a paragraph is described exactly once — which is why it is here
    /// rather than beside <see cref="ResolveParagraphFormat"/>, whose result is asked for freely.
    /// </para>
    /// <para>
    /// An <c>ilfo</c> of zero means the paragraph is not in a list at all, which is how a continuation
    /// paragraph inside an item is written. Its indents still come from its own <c>sprmPDxaLeft</c> and
    /// <c>sprmPDxaLeft1</c>, which Word writes onto every list paragraph, so nothing further is needed to
    /// line it up under the item above it.
    /// </para>
    /// </remarks>
    private string? LabelAt(Ww8ParagraphFormat paragraph)
        => paragraph.ListNumber > 0
            ? _numbering.Advance(paragraph.ListNumber, paragraph.ListLevel ?? 0)
            : null;

    /// <summary>
    /// The stretches a paragraph's character formatting divides its text into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Walked by position rather than by reading the CHPX table's own boundaries, because those are file
    /// offsets and a paragraph's text is not a slice of the file: the piece table can move between
    /// eight-bit and sixteen-bit pieces mid-paragraph, and this pass drops characters. So the cost is one
    /// piece lookup per character — but only one CHPX decode per actual run, because the table reports the
    /// byte range each entry covers and a position still inside it reuses the last answer.
    /// </para>
    /// <para>
    /// The paragraph style's character half is resolved once, outside the walk: it is constant within a
    /// paragraph, and it is the half that makes a heading's runs large and bold.
    /// </para>
    /// </remarks>
    private List<Ww8LayoutRun> ReadRuns(string text, List<int> positions, int markPosition)
    {
        List<Ww8LayoutRun> runs = [];
        if (text.Length == 0 || positions.Count == 0) return runs;

        Ww8LayoutFormat inherited = CharacterStyleFormat(markPosition);
        int count = Math.Min(text.Length, positions.Count);

        ReadOnlyMemory<byte> properties = default;
        int cachedFrom = 0;
        int cachedTo = 0;
        bool cached = false;

        for (int index = 0; index < count; index++)
        {
            int byteOffset = _pieces.FileOffsetOf(positions[index]);

            if (!cached || byteOffset < cachedFrom || byteOffset >= cachedTo)
            {
                (properties, cachedFrom, cachedTo) =
                    _characterProperties.FindWithRange(byteOffset);
                cached = true;

                // A table with no entry for this offset reports an empty range, which would make every
                // character a fresh lookup. Treating the one character as the range stops that.
                if (cachedTo <= cachedFrom) cachedTo = cachedFrom + 1;
            }

            Ww8LayoutFormat format = ApplyCharacterException(inherited, properties);
            Ww8LayoutRun run = new(
                index,
                1,
                format.FontIndex is { } font ? Fonts.Name(font) : null,
                SizeOf(format),
                format.IsBold == true ? 700 : 400,
                format.IsItalic == true,
                LanguageOf(format),
                format.Colour,
                format.Escapement ?? Layout.Escapement.None,
                format.CaseMap,
                format.Highlight,
                format.IsUnderlined ?? false,
                format.IsStruckThrough ?? false,
                format.AutoKerning ?? false);

            if (runs.Count > 0 && MatchesFormatting(runs[^1], run))
            {
                runs[^1] = runs[^1] with { Length = runs[^1].Length + 1 };
                continue;
            }

            runs.Add(run);
        }

        return runs;
    }

    /// <summary>True when two runs' formatting is identical, whatever their ranges.</summary>
    private static bool MatchesFormatting(Ww8LayoutRun a, Ww8LayoutRun b)
        => string.Equals(a.FamilyName, b.FamilyName, StringComparison.Ordinal)
           && a.Size == b.Size
           && a.Weight == b.Weight
           && a.IsItalic == b.IsItalic
           && string.Equals(a.Language, b.Language, StringComparison.Ordinal)
           && a.Colour == b.Colour
           && a.Escapement == b.Escapement
           && a.CaseMap == b.CaseMap
           && a.Highlight == b.Highlight
           && a.IsUnderlined == b.IsUnderlined
           && a.IsStruckThrough == b.IsStruckThrough
           && a.AutoKerning == b.AutoKerning;

    /// <summary>
    /// The em size a character format states, defaulting to ten points.
    /// </summary>
    /// <remarks>
    /// Ten rather than twelve, because that is what Word's own default is for a document whose stylesheet
    /// states none. The bound rejects the absurd rather than the merely large: 4000 half-points is 2000 pt.
    /// </remarks>
    private static Length SizeOf(Ww8LayoutFormat format)
        => format.FontSizeHalfPoints is { } halves and > 0 and <= 4000
            ? Length.FromPoints(halves / 2.0)
            : Length.FromPoints(10);

    private static string? LanguageOf(Ww8LayoutFormat format)
        => format.LanguageId is { } id and > 0 and <= ushort.MaxValue
            ? WindowsLanguages.TagOf((ushort)id)
            : null;

    /// <summary>
    /// The layout sprms in force on a paragraph, style chain first and its own last.
    /// </summary>
    private Ww8LayoutFormat ResolveLayoutFormat(int position)
    {
        int byteOffset = _pieces.FileOffsetOf(position);
        (ushort styleIndex, ReadOnlyMemory<byte> direct) =
            Ww8FormattingTable.SplitParagraphProperties(_paragraphProperties.Find(byteOffset));

        Ww8LayoutFormat format = default;
        foreach (ReadOnlyMemory<byte> inherited in _styles.ResolveChain(styleIndex))
        {
            format = ApplyLayoutSprms(format, inherited);
        }

        return ApplyLayoutSprms(format, direct) with { StyleIndex = styleIndex };
    }

    /// <summary>
    /// Takes each run of framed paragraphs out of the flow and hangs it on the paragraph that follows.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is <c>StartApo</c>/<c>StopApo</c> without the insertion point: a run of paragraphs stating
    /// the same non-empty position is one frame, and Writer's own loop finds its end the same way —
    /// <c>TestSameApo</c> compares the new paragraph's <c>WW8FlyPara</c> against the open one's and
    /// closes the frame when they differ (<c>sw/source/filter/ww8/ww8par2.cxx:483</c>).
    /// </para>
    /// <para>
    /// The frame hangs on the <em>following</em> paragraph because that is the node Writer's insertion
    /// point is at once the fly has been filled and left. Where a frame ends the flow there is no
    /// following paragraph, and it hangs on the preceding one instead rather than being dropped: a
    /// document whose last thing is a framed block still draws it.
    /// </para>
    /// <para>
    /// A frame's own paragraphs keep whatever they stated, including their frame position — nothing
    /// below reads it, and clearing it would lose the record of why they are where they are.
    /// </para>
    /// </remarks>
    /// <param name="blocks">The flow's blocks, in order.</param>
    private static List<Ww8LayoutBlock> LiftTextFrames(List<Ww8LayoutBlock> blocks)
    {
        bool any = false;
        foreach (Ww8LayoutBlock block in blocks)
        {
            if (block.Paragraph is { } paragraph && !paragraph.TextFrame.IsEmpty) { any = true; break; }
        }

        if (!any) return blocks;

        List<Ww8LayoutBlock> kept = new(blocks.Count);
        List<Ww8LayoutTextFrame> pending = [];

        for (int index = 0; index < blocks.Count; index++)
        {
            if (blocks[index].Paragraph is not { } paragraph || paragraph.TextFrame.IsEmpty)
            {
                kept.Add(Anchoring(blocks[index], pending));
                continue;
            }

            Ww8TextFramePosition position = paragraph.TextFrame;
            List<Ww8LayoutBlock> inside = [];

            while (index < blocks.Count
                && blocks[index].Paragraph is { } member
                && member.TextFrame == position)
            {
                inside.Add(blocks[index]);
                index++;
            }

            index--;
            pending.Add(new Ww8LayoutTextFrame(position, inside));
        }

        // A frame that nothing follows: hang it on the last block that stayed in the flow. Only a
        // paragraph can carry one, so a flow ending in a table has nowhere to put it and drops it —
        // which is still better than laying its text out in the middle of the table.
        for (int index = kept.Count - 1; index >= 0 && pending.Count > 0; index--)
        {
            if (kept[index].Paragraph is null) continue;
            kept[index] = Anchoring(kept[index], pending);
            break;
        }

        return kept;

        static Ww8LayoutBlock Anchoring(Ww8LayoutBlock block, List<Ww8LayoutTextFrame> pending)
        {
            if (pending.Count == 0 || block.Paragraph is not { } paragraph) return block;

            Ww8LayoutBlock anchored = new(paragraph with { TextFrames = [.. pending] });
            pending.Clear();
            return anchored;
        }
    }

    /// <summary>
    /// Where a paragraph's properties say it belongs, if they say it belongs in a text frame.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The style chain first and the paragraph's own PAPX last, which is <c>WW8FlyPara</c>'s own order:
    /// a style can declare a frame — <c>m_vColl[nStyle].m_xWWFly</c> — and the paragraph then restates
    /// or overrides parts of it. The result is compared field for field against the neighbouring
    /// paragraphs' to find where the frame begins and ends, so every field has to be resolved the same
    /// way for every paragraph or two paragraphs of one frame will look like two frames.
    /// </para>
    /// <para>
    /// #i8798#, which <c>WW8FlyPara::Read</c> applies at the end of both of its overloads: a frame whose
    /// <c>dyaAbs</c> nobody stated ignores whatever vertical origin the binding names and stays relative
    /// to the text, so the binding is rewritten to say so rather than left to mislead the placement.
    /// </para>
    /// </remarks>
    /// <param name="position">The paragraph mark's position.</param>
    private Ww8TextFramePosition ResolveTextFrame(int position)
    {
        int byteOffset = _pieces.FileOffsetOf(position);
        (ushort styleIndex, ReadOnlyMemory<byte> direct) =
            Ww8FormattingTable.SplitParagraphProperties(_paragraphProperties.Find(byteOffset));

        Ww8TextFramePosition frame = Ww8TextFramePosition.None;
        bool fromStyle = false;

        foreach (ReadOnlyMemory<byte> inherited in _styles.ResolveChain(styleIndex))
        {
            (Ww8TextFramePosition applied, bool statesBinding, _) =
                ApplyTextFrameSprms(frame, inherited);

            // A style contributes a frame only through `sprmPPc`, because that sprm's handler is the
            // only thing that ever builds one — `Read_ApoPPC` makes the style's `WW8FlyPara`, fills it
            // from the whole style, and throws it away again when it comes out empty
            // (<c>sw/source/filter/ww8/ww8par6.cxx:5492</c>).
            if (!statesBinding) continue;

            frame = applied;
            fromStyle = !applied.IsEmpty;
        }

        (frame, bool statesPPc, bool statesWrap) = ApplyTextFrameSprms(frame, direct);

        // `ApoTestResults::HasFrame`, which is the whole gate: the paragraph's *own* properties state
        // one of the two sprms, or its style declared a frame. Nothing else counts, and the rewrite
        // below must not be reached without it — an unframed paragraph would come out with a binding of
        // 0x20 and so stop looking empty, which is every paragraph in the document turned into a frame.
        if (!statesPPc && !statesWrap && !fromStyle) return Ww8TextFramePosition.None;
        if (frame.IsEmpty) return Ww8TextFramePosition.None;

        return frame.StatesVerticalPosition
            ? frame
            : frame with { Binding = (byte)((frame.Binding & 0xCF) | 0x20) };
    }

    /// <summary>Applies one grpprl's text-frame sprms.</summary>
    /// <remarks>
    /// The ids are <c>sprmids.hxx</c>'s (lines 401–417). <c>sprmPDxaFromText</c> and
    /// <c>sprmPDyaFromText</c> each set two of <c>WW8FlyPara</c>'s four margins, which is why one sprm
    /// lands in one field here rather than in two.
    /// </remarks>
    /// <returns>
    /// The frame, and whether this grpprl stated <c>sprmPPc</c> and <c>sprmPWr</c> — which is what
    /// decides whether there is a frame at all, separately from what the frame then says.
    /// </returns>
    private static (Ww8TextFramePosition Frame, bool StatesBinding, bool StatesWrap)
        ApplyTextFrameSprms(Ww8TextFramePosition frame, ReadOnlyMemory<byte> grpprl)
    {
        bool binding = false;
        bool wrap = false;

        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            switch (sprm.Identifier)
            {
                case TextFrameSprms.Binding:
                    frame = frame with { Binding = sprm.Byte };
                    binding = true;
                    break;
                case TextFrameSprms.Wrap:
                    frame = frame with { Wrap = sprm.Byte };
                    wrap = true;
                    break;
                case TextFrameSprms.XOffset:
                    frame = frame with { XOffset = sprm.SignedWord };
                    break;
                case TextFrameSprms.YOffset:
                    frame = frame with { YOffset = sprm.SignedWord, StatesVerticalPosition = true };
                    break;
                case TextFrameSprms.Width:
                    frame = frame with { Width = sprm.SignedWord };
                    break;
                case TextFrameSprms.Height:
                    frame = frame with { Height = sprm.SignedWord };
                    break;
                case TextFrameSprms.FromTextX:
                    frame = frame with { FromTextX = sprm.SignedWord };
                    break;
                case TextFrameSprms.FromTextY:
                    frame = frame with { FromTextY = sprm.SignedWord };
                    break;
                default:
                    continue;
            }
        }

        return (frame, binding, wrap);
    }

    /// <summary>The paragraph sprms that describe a text frame, from <c>sprmids.hxx</c>.</summary>
    private static class TextFrameSprms
    {
        /// <summary><c>sprmPPc</c>, the pair of two-bit origins.</summary>
        internal const ushort Binding = 0x261B;

        /// <summary><c>sprmPDxaAbs</c>.</summary>
        internal const ushort XOffset = 0x8418;

        /// <summary><c>sprmPDyaAbs</c>.</summary>
        internal const ushort YOffset = 0x8419;

        /// <summary><c>sprmPDxaWidth</c>.</summary>
        internal const ushort Width = 0x841A;

        /// <summary><c>sprmPWHeightAbs</c>.</summary>
        internal const ushort Height = 0x442B;

        /// <summary><c>sprmPDyaFromText</c>.</summary>
        internal const ushort FromTextY = 0x842E;

        /// <summary><c>sprmPDxaFromText</c>.</summary>
        internal const ushort FromTextX = 0x842F;

        /// <summary><c>sprmPWr</c>.</summary>
        internal const ushort Wrap = 0x2423;
    }

    /// <summary>
    /// What a paragraph's <em>own</em> PAPX states about its indents and its list, as opposed to what it
    /// inherits.
    /// </summary>
    /// <remarks>
    /// The distinction Writer's <c>SwTextNode::AreListLevelIndentsApplicableImpl</c> turns on: a
    /// hard-set indent beats the list level's, and a list named directly on the paragraph beats the
    /// indents its <em>style</em> sets. A WW8 import puts exactly the paragraph's own sprms on the node
    /// and its style's on the format, so "hard-set" is the direct grpprl and nothing else.
    /// </remarks>
    private (bool SetsLeftIndent, bool SetsFirstLineIndent, bool NamesList) DirectParagraphSprms(
        int position)
    {
        (_, ReadOnlyMemory<byte> direct) =
            Ww8FormattingTable.SplitParagraphProperties(
                _paragraphProperties.Find(_pieces.FileOffsetOf(position)));

        bool left = false;
        bool firstLine = false;
        bool list = false;

        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(direct))
        {
            switch (sprm.Identifier)
            {
                case LayoutSprms.LeftIndent or LayoutSprms.LeftIndent80:
                    left = true;
                    break;
                case LayoutSprms.FirstLineIndent or LayoutSprms.FirstLineIndent80:
                    firstLine = true;
                    break;
                case Ww8SprmReader.Ids.ListFormatOverride:
                    list = true;
                    break;
                default:
                    continue;
            }
        }

        return (left, firstLine, list);
    }

    /// <summary>
    /// A list paragraph's format with whichever of its level's indents it is entitled to take.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Word writes a level's geometry into the <c>LVL</c>'s own <c>grpprlPapx</c> and usually — but not
    /// always — repeats it on every paragraph in the list. On the documents that do not repeat it, a
    /// reader taking only the paragraph's own sprms gives every item a nought indent and no hanging one,
    /// so the label is drawn exactly where the item's first word starts and the two fuse.
    /// </para>
    /// <para>
    /// The rule is Writer's, per item: a hard-set indent on the paragraph wins; failing that a list named
    /// directly on the paragraph wins over the style chain; failing that the style chain wins. The last
    /// arm is the conservative reading — Writer would still let the level through when the style carrying
    /// the numbering is met before any indent, and telling those apart needs the chain walked in order,
    /// which is not worth it for a case no corpus document exercises.
    /// </para>
    /// </remarks>
    private Text.Layout.ParagraphFormat WithListIndents(
        Text.Layout.ParagraphFormat format,
        Ww8ListLevel level,
        Ww8LayoutFormat resolved,
        int markPosition)
    {
        (bool setsLeft, bool setsFirstLine, bool namesList) = DirectParagraphSprms(markPosition);

        bool leftApplies = !setsLeft && (namesList || resolved.LeftIndent is null);
        bool firstLineApplies = !setsFirstLine && (namesList || resolved.FirstLineIndent is null);

        if (leftApplies) format = format with { StartIndent = Length.FromTwips(level.IndentAt) };

        if (firstLineApplies)
        {
            format = format with { FirstLineIndent = Length.FromTwips(level.FirstLineIndent) };
        }

        return format;
    }

    /// <summary>
    /// The character sprms in force at a position, resolved through the paragraph style's own half.
    /// </summary>
    /// <remarks>
    /// A paragraph style carries a CHPX as well as a PAPX, and that half is how a heading style makes its
    /// runs large and bold — so it has to be applied before the direct formatting, exactly as the content
    /// pass does for emphasis.
    /// </remarks>
    private Ww8LayoutFormat ResolveCharacterLayout(int position)
        => ApplyCharacterException(
            CharacterStyleFormat(position),
            _characterProperties.Find(_pieces.FileOffsetOf(position)));

    /// <summary>
    /// Applies one CHPX over an inherited format: its character style first, then its own sprms.
    /// </summary>
    /// <remarks>
    /// The two halves cannot be applied in one pass, because the sprm naming the character style sits
    /// inside the same grpprl as the direct formatting — so a single pass would lay the style's properties
    /// over the direct ones that were meant to override them.
    /// <para>
    /// Skipping the style half entirely is worse than a subtle ordering bug, and it is the mistake that is
    /// easy to make: LibreOffice's own DOC export writes emphasis as a character style rather than as
    /// direct sprms, so a reader that only decodes the exception finds a document with no bold in it at
    /// all.
    /// </para>
    /// </remarks>
    private Ww8LayoutFormat ApplyCharacterException(
        Ww8LayoutFormat inherited, ReadOnlyMemory<byte> exception)
    {
        Ww8LayoutFormat format = inherited;

        // Index zero is not "no character style" — in WW8 the stylesheet is one table and istd 0 is
        // *Normal*, a paragraph style. Resolving its chain here would lay the document's default font size
        // over the paragraph style's own, so every run of an 11 pt paragraph would come out at 12.
        if (CharacterStyleIndexIn(exception) is var styleIndex and not 0)
        {
            Colour? outer = format.Highlight;

            foreach (ReadOnlyMemory<byte> fromStyle in _styles.ResolveCharacterChain(styleIndex))
            {
                format = ApplyLayoutSprms(format, fromStyle);
            }

            // Word ignores character highlighting in a *character* style, and only there — a paragraph
            // style's CHPX carries it as it carries everything else. `SwWW8ImplReader::Read_CharHighlight`
            // says so in its first two lines (`ww8par6.cxx`:4237): it returns without reading the operand
            // when the style being built is a RES_CHRFMT.
            format = format with { Highlight = outer };
        }

        return ApplyLayoutSprms(format, exception);
    }

    /// <summary>
    /// The character formatting a position's paragraph style contributes, without its direct formatting.
    /// </summary>
    /// <remarks>
    /// Separated out because it is constant within a paragraph while the direct formatting is not, so a
    /// run walk resolves this once and layers each CHPX over it — rather than re-walking the style chain
    /// for every run.
    /// </remarks>
    private Ww8LayoutFormat CharacterStyleFormat(int position)
    {
        Ww8LayoutFormat format = default;
        foreach (ReadOnlyMemory<byte> inherited in
                 _styles.ResolveCharacterChain(ParagraphStyleIndexAt(position)))
        {
            format = ApplyLayoutSprms(format, inherited);
        }

        return format;
    }

    /// <summary>
    /// Applies one grpprl's layout sprms.
    /// </summary>
    /// <remarks>
    /// The ids come from LibreOffice's <c>sprmids.hxx</c>, which states each operand's size on the same
    /// line — the half that decides whether the <em>following</em> sprm decodes at all. Both the modern
    /// and the Word 97 forms of the indents and the alignment are handled, because a document saved by
    /// any version of Word may carry either and they are different numbers.
    /// </remarks>
    private Ww8LayoutFormat ApplyLayoutSprms(
        Ww8LayoutFormat format, ReadOnlyMemory<byte> grpprl)
    {
        // What `sprmPFDyaBeforeAuto` and `sprmPFDyaAfterAuto` stand for in this document. Fourteen
        // points ordinarily and five when the document switched HTML auto-spacing off, which is the
        // whole of `SwWW8ImplReader::GetParagraphAutoSpace` (`ww8par6.cxx:4609`).
        int autoSpacing = DocumentProperties.CollapsesSpacing
            ? Ww8LayoutFormat.HtmlAutoSpacingTwips
            : Ww8LayoutFormat.WordAutoSpacingTwips;

        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(grpprl))
        {
            switch (sprm.Identifier)
            {
                // Which of the two sprms stated it travels with the value, because the two disagree
                // about what nought and two mean in a right-to-left paragraph — see
                // Ww8LayoutFormat.IsJustificationAbsolute.
                case LayoutSprms.Justification:
                    format = format with
                    {
                        Justification = sprm.Byte,
                        IsJustificationAbsolute = false,
                    };
                    break;
                case LayoutSprms.Justification80:
                    format = format with
                    {
                        Justification = sprm.Byte,
                        IsJustificationAbsolute = true,
                    };
                    break;
                case LayoutSprms.RightToLeft:
                    format = format with { IsRightToLeft = sprm.Byte != 0 };
                    break;

                case LayoutSprms.LeftIndent or LayoutSprms.LeftIndent80:
                    format = format with { LeftIndent = sprm.SignedWord };
                    break;
                case LayoutSprms.RightIndent or LayoutSprms.RightIndent80:
                    format = format with { RightIndent = sprm.SignedWord };
                    break;
                case LayoutSprms.FirstLineIndent or LayoutSprms.FirstLineIndent80:
                    format = format with { FirstLineIndent = sprm.SignedWord };
                    break;

                case LayoutSprms.SpaceBefore:
                    format = format with { SpaceBefore = sprm.Word };
                    break;
                case LayoutSprms.SpaceAfter:
                    format = format with { SpaceAfter = sprm.Word };
                    break;

                // The auto-spacing pair sets the margin outright rather than flagging it, and is applied
                // in file order beside `sprmPDyaBefore` so that whichever the document states last wins —
                // which is exactly how `Read_ParaAutoBefore` and `Read_UL` compose on one `SvxULSpaceItem`.
                // Switched *off* the sprm states nothing about the margin at all, only about the flag,
                // which is why the else branch leaves the spacing alone.
                case LayoutSprms.SpaceBeforeAuto:
                    format = sprm.Byte != 0
                        ? format with { HasAutoSpaceBefore = true, SpaceBefore = autoSpacing }
                        : format with { HasAutoSpaceBefore = false };
                    break;
                case LayoutSprms.SpaceAfterAuto:
                    format = sprm.Byte != 0
                        ? format with { HasAutoSpaceAfter = true, SpaceAfter = autoSpacing }
                        : format with { HasAutoSpaceAfter = false };
                    break;

                case LayoutSprms.LineSpacing:
                {
                    // An LSPD: a signed spacing then a flag, and the flag changes the first field's
                    // unit as well as its meaning.
                    ReadOnlySpan<byte> operand = sprm.Operand.Span;
                    if (operand.Length >= 4)
                    {
                        format = format with
                        {
                            LineSpacing = System.Buffers.Binary.BinaryPrimitives
                                .ReadInt16LittleEndian(operand),
                            IsMultipleLineSpacing = System.Buffers.Binary.BinaryPrimitives
                                .ReadUInt16LittleEndian(operand[2..]) != 0,
                        };
                    }
                    break;
                }

                case LayoutSprms.ColourIndex:
                    format = format with
                    {
                        Colour = sprm.Byte < IcoPalette.Length ? IcoPalette[sprm.Byte] : null,
                    };
                    break;

                case LayoutSprms.Highlight:
                    // Index nought is the palette's automatic entry and reads back as null, which is what
                    // "no highlighter" already means here — so out-of-range values fall to the same place
                    // rather than to black, as Read_CharHighlight's `if (b > 16) b = 0` does.
                    format = format with
                    {
                        Highlight = sprm.Byte < IcoPalette.Length ? IcoPalette[sprm.Byte] : null,
                    };
                    break;

                case LayoutSprms.VerticalPosition:
                    format = format with
                    {
                        Escapement = sprm.Byte switch
                        {
                            1 => Layout.Escapement.Superscript,
                            2 => Layout.Escapement.Subscript,
                            _ => Layout.Escapement.None,
                        },
                    };
                    break;

                case LayoutSprms.Colour:
                {
                    // A COLORREF, which is 0x00bbggrr as a little-endian DWORD — so the bytes arrive as
                    // red, green, blue, flag, and taking them in that order is the same swap
                    // LibreOffice's own BGRToRGB performs. All four set is COL_AUTO, the automatic
                    // colour, which is not the same thing as opaque white.
                    ReadOnlySpan<byte> operand = sprm.Operand.Span;
                    bool automatic = operand.Length >= 4 && operand[..4] is [0xFF, 0xFF, 0xFF, 0xFF];

                    format = format with
                    {
                        Colour = operand.Length >= 3 && !automatic
                            ? Colour.FromRgb((uint)((operand[0] << 16) | (operand[1] << 8) | operand[2]))
                            : null,
                    };
                    break;
                }

                case LayoutSprms.TabStops:
                    format = format with
                    {
                        TabStops = ApplyTabChange(format.TabStops, sprm.Operand.Span),
                    };
                    break;

                case LayoutSprms.KeepTogether:
                    format = format with { KeepTogether = sprm.Byte != 0 };
                    break;
                case LayoutSprms.KeepWithNext:
                    format = format with { KeepWithNext = sprm.Byte != 0 };
                    break;
                case LayoutSprms.PageBreakBefore:
                    format = format with { StartsNewPage = sprm.Byte != 0 };
                    break;
                case LayoutSprms.WidowControl:
                    format = format with { HasWidowControl = sprm.Byte != 0 };
                    break;
                case LayoutSprms.ContextualSpacing:
                    format = format with { HasContextualSpacing = sprm.Byte != 0 };
                    break;

                case LayoutSprms.FontSize:
                    format = format with { FontSizeHalfPoints = sprm.Word };
                    break;
                case LayoutSprms.FontIndex:
                    format = format with { FontIndex = sprm.Word };
                    break;
                case LayoutSprms.Bold:
                    format = format with
                    {
                        IsBold = sprm.ResolveToggle(format.IsBold ?? false),
                    };
                    break;
                case LayoutSprms.Italic:
                    format = format with
                    {
                        IsItalic = sprm.ResolveToggle(format.IsItalic ?? false),
                    };
                    break;
                case LayoutSprms.SmallCaps:
                    format = format with
                    {
                        IsSmallCapitalised = sprm.ResolveToggle(format.IsSmallCapitalised ?? false),
                    };
                    break;
                case LayoutSprms.Caps:
                    format = format with
                    {
                        IsCapitalised = sprm.ResolveToggle(format.IsCapitalised ?? false),
                    };
                    break;

                // Both strike sprms are dispatched to `Read_BoldUsw` — the toggle handler — so both
                // carry WW8's four-state operand rather than a boolean, and the doubled one folds onto
                // the same flag because the page model draws one rule.
                case LayoutSprms.Strike:
                    format = format with
                    {
                        IsStruckThrough = sprm.ResolveToggle(format.IsStruckThrough ?? false),
                    };
                    break;
                case LayoutSprms.DoubleStrike:
                    format = format with
                    {
                        IsStruckThrough = sprm.ResolveToggle(format.IsStruckThrough ?? false),
                    };
                    break;

                case LayoutSprms.Underline:
                    format = format with { IsUnderlined = IsUnderlineStyle(sprm.Byte) };
                    break;

                // Not a toggle: the operand is the threshold size, and only its being nonzero
                // survives into Writer's boolean item.
                case LayoutSprms.FontKern:
                    format = format with { AutoKerning = sprm.Word != 0 };
                    break;
                case LayoutSprms.Language or LayoutSprms.Language80:
                    format = format with { LanguageId = sprm.Word };
                    break;

                default:
                    break;
            }
        }

        return format;
    }

    /// <summary>
    /// Whether a <c>kul</c> names a line that is actually drawn.
    /// </summary>
    /// <remarks>
    /// The set is <c>SwWW8ImplReader::Read_Underline</c>'s switch
    /// (<c>sw/source/filter/ww8/ww8par6.cxx</c>:3600), and taking it from there rather than testing the
    /// byte for non-zero matters in both directions: 5 is "hidden" and 8 is a dot style Word never
    /// writes, and neither has a case in that switch, so both fall to <c>LINESTYLE_NONE</c> and draw
    /// nothing. 255 is the cancelling value and is likewise absent. Every value that <em>is</em> listed
    /// is drawn as one plain rule, because the page model carries no line style.
    /// </remarks>
    internal static bool IsUnderlineStyle(int kul) => kul
        is 1 or 2 or 3 or 4 or 6 or 7 or 9 or 10 or 11
        or 20 or 23 or 25 or 26 or 27 or 39 or 43 or 55;

    /// <summary>The layout sprms, from LibreOffice's <c>sprmids.hxx</c>.</summary>
    private static class LayoutSprms
    {
        internal const ushort Justification80 = 0x2403;
        internal const ushort KeepTogether = 0x2405;
        internal const ushort KeepWithNext = 0x2406;
        internal const ushort PageBreakBefore = 0x2407;
        internal const ushort RightIndent80 = 0x840E;
        internal const ushort LeftIndent80 = 0x840F;
        internal const ushort FirstLineIndent80 = 0x8411;
        internal const ushort LineSpacing = 0x6412;
        internal const ushort SpaceBefore = 0xA413;
        internal const ushort SpaceAfter = 0xA414;
        internal const ushort SpaceBeforeAuto = 0x245B;
        internal const ushort SpaceAfterAuto = 0x245C;
        internal const ushort WidowControl = 0x2431;
        internal const ushort RightIndent = 0x845D;
        internal const ushort LeftIndent = 0x845E;
        internal const ushort FirstLineIndent = 0x8460;
        internal const ushort Justification = 0x2461;
        internal const ushort ContextualSpacing = 0x246D;
        internal const ushort RightToLeft = 0x2441;

        internal const ushort Bold = 0x0835;
        internal const ushort Italic = 0x0836;
        internal const ushort Strike = 0x0837;
        internal const ushort SmallCaps = 0x083A;
        internal const ushort Caps = 0x083B;

        /// <summary>
        /// <c>sprmCFDStrike</c>: the second line of a double strike-through.
        /// </summary>
        /// <remarks>
        /// Out of sequence with the other character toggles, which is why LibreOffice's
        /// <c>Read_BoldUsw</c> singles it out before computing its bit — but a toggle all the same, and
        /// dispatched to the same handler as <see cref="Strike"/>.
        /// </remarks>
        internal const ushort DoubleStrike = 0x2A53;

        /// <summary>
        /// <c>sprmCKul</c>: the <em>style</em> of the rule drawn under the run, not a switch.
        /// </summary>
        /// <remarks>
        /// See <see cref="IsUnderlineStyle"/> — nought, 255 and two values in between all mean no line,
        /// so reading this byte as a boolean underlines text Word leaves plain.
        /// </remarks>
        internal const ushort Underline = 0x2A3E;

        /// <summary>
        /// <c>sprmCHpsKern</c>: the size at or above which the run is pair-kerned.
        /// </summary>
        /// <remarks>
        /// Two bytes of half-points (<c>sw/source/filter/ww8/sprmids.hxx:330</c>), read as a boolean
        /// because that is all Writer can hold — see <see cref="Ww8LayoutFormat.AutoKerning"/>.
        /// </remarks>
        internal const ushort FontKern = 0x484B;

        internal const ushort FontSize = 0x4A43;
        internal const ushort FontIndex = 0x4A4F;
        internal const ushort Language80 = 0x486D;
        internal const ushort Language = 0x4873;
        internal const ushort ColourIndex = 0x2A42;

        /// <summary>
        /// <c>sprmCHighlight</c>: an <c>ico</c> index naming the band drawn behind the text.
        /// </summary>
        /// <remarks>
        /// The same seventeen-entry palette <see cref="ColourIndex"/> indexes, and index nought means
        /// <em>no</em> highlight rather than an automatic colour — Word's highlighter has an explicit
        /// "none", and <c>SwWW8ImplReader::Read_CharHighlight</c>
        /// (<c>sw/source/filter/ww8/ww8par6.cxx</c>) turns it into <c>COL_TRANSPARENT</c>.
        /// </remarks>
        internal const ushort Highlight = 0x2A0C;

        /// <summary>
        /// <c>sprmCIss</c>: 1 for superscript, 2 for subscript, 0 for neither.
        /// </summary>
        /// <remarks>
        /// The one WW8 spells as a *kind* rather than a distance, which is why it can be read as an
        /// automatic escapement: the shift and the smaller size both come with it. Its companion
        /// <c>sprmCHpsPos</c> (0x4845) states a half-point offset outright and is not read yet.
        /// </remarks>
        internal const ushort VerticalPosition = 0x2A48;
        internal const ushort Colour = 0x6870;
        internal const ushort TabStops = 0xC60D;
    }

    /// <summary>
    /// Applies one <c>sprmPChgTabsPapx</c> to the stops accumulated so far.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The sprm is a <em>change</em>, not a list, and its operand packs three arrays whose lengths depend on
    /// each other: a count of deletions, that many two-byte positions, a count of insertions, that many
    /// two-byte positions, and finally that many one-byte descriptors. Reading the descriptors from the
    /// wrong offset gives every stop a plausible-looking wrong alignment, so the bounds are checked the way
    /// <c>SwWW8ImplReader::Read_Tab</c> checks them — a record claiming more than it carries is discarded
    /// whole rather than half-read.
    /// </para>
    /// <para>
    /// A descriptor's low three bits are the alignment and the next three the leader. Alignment 4 is a bar
    /// tab — a vertical rule rather than an advance — which LibreOffice ignores here, and so does this: a
    /// bar recorded as a left stop would put a column boundary where the document asked for a line.
    /// </para>
    /// </remarks>
    private static List<TabStop> ApplyTabChange(
        IReadOnlyList<TabStop>? inherited, ReadOnlySpan<byte> operand)
    {
        List<TabStop> stops = inherited is null ? [] : [.. inherited];
        if (operand.Length < 1) return stops;

        int deletions = operand[0];
        if (operand.Length < (2 * deletions) + 2) return stops;

        int insertions = operand[(2 * deletions) + 1];

        // 2 + 2*del + 2*ins + 1*ins, which is the length the record needs to describe what it claims.
        if (2 + (2 * deletions) + (3 * insertions) > operand.Length) return stops;

        for (int i = 0; i < deletions; i++)
        {
            long position = Length
                .FromTwips(BinaryPrimitives.ReadUInt16LittleEndian(operand[(1 + (2 * i))..])).Emu;

            stops.RemoveAll(stop => stop.Position.Emu == position);
        }

        int positions = (2 * deletions) + 2;
        int descriptors = positions + (2 * insertions);

        for (int i = 0; i < insertions && stops.Count < MaxTabStops; i++)
        {
            byte descriptor = operand[descriptors + i];
            int alignment = descriptor & 0x7;
            if (alignment == 4) continue;

            stops.Add(new TabStop(
                Length.FromTwips(
                    BinaryPrimitives.ReadUInt16LittleEndian(operand[(positions + (2 * i))..])),
                alignment switch
                {
                    1 => TabAlignment.Centre,
                    2 => TabAlignment.Right,
                    3 => TabAlignment.DecimalSeparator,
                    _ => TabAlignment.Left,
                },
                ((descriptor >> 3) & 0x7) switch
                {
                    1 => '.',
                    2 => '-',
                    3 or 4 => '_',
                    _ => '\0',
                }));
        }

        stops.Sort((left, right) => left.Position.Emu.CompareTo(right.Position.Emu));
        return stops;
    }

    /// <summary>How many tab stops a paragraph may declare before the rest are ignored.</summary>
    /// <remarks>
    /// A guard on untrusted input. Word's own limit is 64 and a real paragraph uses a handful; each stop
    /// costs a lookup on every tab in the paragraph.
    /// </remarks>
    public const int MaxTabStops = 256;

    /// <summary>
    /// The seventeen colours a <c>sprmCIco</c> index names.
    /// </summary>
    /// <remarks>
    /// Copied from <c>SwWW8ImplReader::GetCol</c> (<c>sw/source/filter/ww8/ww8par6.cxx</c>), whose order is
    /// not the obvious one: index 2 is <em>light</em> blue and index 9 is blue, so the palette runs bright
    /// colours first and dark ones second. Index 0 is the automatic colour and is null rather than black,
    /// so the document's own default applies.
    /// </remarks>
    private static readonly Colour?[] IcoPalette =
    [
        null,
        Colour.FromRgb(0x000000),
        Colour.FromRgb(0x0000FF),
        Colour.FromRgb(0x00FFFF),
        Colour.FromRgb(0x00FF00),
        Colour.FromRgb(0xFF00FF),
        Colour.FromRgb(0xFF0000),
        Colour.FromRgb(0xFFFF00),
        Colour.FromRgb(0xFFFFFF),
        Colour.FromRgb(0x000080),
        Colour.FromRgb(0x008080),
        Colour.FromRgb(0x008000),
        Colour.FromRgb(0x800080),
        Colour.FromRgb(0x800000),
        Colour.FromRgb(0x808000),
        Colour.FromRgb(0x808080),
        Colour.FromRgb(0xC0C0C0),
    ];
}

/// <summary>
/// A DOC section's headers and footers, as the layout pass reads them.
/// </summary>
/// <remarks>
/// A pair rather than one dictionary keyed by both, because every consumer wants one or the other: a
/// header goes in the header area and a footer in the footer area, and nothing iterates the twelve
/// together.
/// </remarks>
/// <param name="Headers">The headers, by slot; a slot with no entry has no header.</param>
/// <param name="Footers">The footers, by slot.</param>
public sealed record Ww8LayoutFurniture(
    IReadOnlyDictionary<Model.PageFurnitureSlot, List<Ww8LayoutBlock>> Headers,
    IReadOnlyDictionary<Model.PageFurnitureSlot, List<Ww8LayoutBlock>> Footers);
