namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// The <c>PPT_PST_*</c> record types of the legacy binary PowerPoint format.
/// </summary>
/// <remarks>
/// Numbering follows <c>include/filter/msfilter/svdfppt.hxx:1297</c>. These share the
/// eight-byte header and the same stream as the Escher drawing records, so the two
/// vocabularies interleave: a slide container holds PowerPoint records, one of which
/// (<c>PPDrawing</c>) holds an Escher drawing, whose shapes hold PowerPoint text records
/// again.
/// </remarks>
public static class PptRecordTypes
{
    /// <summary>The document container; the root of everything the persist directory reaches.</summary>
    public const ushort Document = 1000;

    /// <summary>Slide and notes page sizes, and the handout and notes master persist ids.</summary>
    public const ushort DocumentAtom = 1001;

    /// <summary>One slide.</summary>
    public const ushort Slide = 1006;

    /// <summary>A slide's layout, master id, notes id and flags.</summary>
    public const ushort SlideAtom = 1007;

    /// <summary>One notes page.</summary>
    public const ushort Notes = 1008;

    /// <summary>A notes page's slide id and flags.</summary>
    public const ushort NotesAtom = 1009;

    /// <summary>Document-wide text defaults: the font collection and the default styles.</summary>
    public const ushort Environment = 1010;

    /// <summary>An entry in a slide list: a persist id, a slide id and a text count.</summary>
    public const ushort SlidePersistAtom = 1011;

    /// <summary>A main master slide.</summary>
    public const ushort MainMaster = 1016;

    /// <summary>The drawing group: the document-wide Escher state.</summary>
    public const ushort DrawingGroup = 1035;

    /// <summary>One page's Escher drawing.</summary>
    public const ushort Drawing = 1036;

    /// <summary>Slide-show information, including whether the slide is skipped.</summary>
    public const ushort SlideShowSlideInfoAtom = 1017;

    /// <summary>Announces that the file contains VBA.</summary>
    public const ushort VbaInfo = 1023;

    /// <summary>The list container the document's non-slide sub-containers live in.</summary>
    public const ushort List = 2000;

    /// <summary>The document's font collection.</summary>
    public const ushort FontCollection = 2005;

    /// <summary>A slide's colour scheme.</summary>
    public const ushort ColorSchemeAtom = 2032;

    /// <summary>Marks a shape as a layout placeholder and says which one.</summary>
    public const ushort PlaceholderAtom = 3011;

    /// <summary>
    /// A reference to text held in the document's slide list rather than in the shape.
    /// </summary>
    public const ushort OutlineTextRefAtom = 3998;

    /// <summary>Announces a text run and which master style it takes its defaults from.</summary>
    public const ushort TextHeaderAtom = 3999;

    /// <summary>A text run's characters, as UTF-16.</summary>
    public const ushort TextCharsAtom = 4000;

    /// <summary>A text run's paragraph and character property runs.</summary>
    public const ushort StyleTextPropAtom = 4001;

    /// <summary>A master's per-outline-level text styles.</summary>
    public const ushort TxMasterStyleAtom = 4003;

    /// <summary>A text run's ruler: tab stops and per-level indents.</summary>
    public const ushort TextRulerAtom = 4006;

    /// <summary>
    /// A shape's PowerPoint 97+ paragraph extensions: picture bullets and automatic numbering.
    /// </summary>
    /// <remarks>
    /// <c>PPT_PST_ExtendedParagraphAtom</c>, <c>include/filter/msfilter/svdfppt.hxx:1339</c>. It
    /// never sits beside the text records: it is buried in the shape's <c>ClientData</c> under
    /// <see cref="ProgTags"/> → <see cref="ProgBinaryTag"/> named <c>___PPT9</c> →
    /// <see cref="BinaryTagData"/>, which is why a reader that walks the client text box alone
    /// cannot see it. <c>SdrPowerPointImport::SeekToContentOfProgTag(9, …)</c>,
    /// <c>svdfppt.cxx:6547-6551</c>.
    /// </remarks>
    public const ushort ExtendedParagraphAtom = 4012;

    /// <summary>
    /// A main master's PowerPoint 97+ paragraph extensions, one atom per text kind.
    /// </summary>
    /// <remarks>
    /// <c>PPT_PST_ExtendedParagraphMasterAtom</c>, <c>svdfppt.hxx:1340</c>. It sits in the
    /// <see cref="MainMaster"/> container's own <c>___PPT9</c> tag rather than in any shape's, and
    /// its record <em>instance</em> is the <see cref="PptTextKind"/> whose levels it states —
    /// <c>PPTExtParaProv</c>'s constructor, <c>svdfppt.cxx:3316-3349</c>. A picture bullet a deck
    /// applies to a whole outline level lives here and nowhere else, so a reader that looks only
    /// at a shape's <see cref="ExtendedParagraphAtom"/> finds nothing on such a deck.
    /// </remarks>
    public const ushort ExtendedParagraphMasterAtom = 4013;

    /// <summary>The document's store of picture-bullet graphics.</summary>
    /// <remarks>
    /// <c>PPT_PST_ExtendedBuGraContainer</c>, <c>svdfppt.hxx:1322</c>. One level under the
    /// document's <see cref="List"/> record's <c>___PPT9</c> tag, and shared by every master and
    /// every shape: a <c>buBlip</c> anywhere in the file is an index into this container.
    /// </remarks>
    public const ushort ExtendedBuGraContainer = 2040;

    /// <summary>One picture-bullet graphic, indexed by its record instance.</summary>
    /// <remarks>
    /// <c>PPT_PST_ExtendedBuGraAtom</c>, <c>svdfppt.hxx:1323</c> — "the instance of this atom
    /// indices the current graphic". Its content is a two-byte type word and then a blip record
    /// read directly, exactly as <c>SvxMSDffManager::GetBLIPDirect</c> reads one
    /// (<c>svdfppt.cxx:3255-3262</c>); it is <em>not</em> a blip store entry and has no
    /// <c>foDelay</c>.
    /// </remarks>
    public const ushort ExtendedBuGraAtom = 2041;

    /// <summary>A text run's characters, one byte each.</summary>
    public const ushort TextBytesAtom = 4008;

    /// <summary>Per-run language and spelling information.</summary>
    public const ushort TextSpecInfoAtom = 4010;

    /// <summary>One font in the font collection.</summary>
    public const ushort FontEntityAtom = 4023;

    /// <summary>A length-prefixed UTF-16 string, used for names throughout.</summary>
    public const ushort CString = 4026;

    /// <summary>
    /// Marks the position of the slide number inside a run's text.
    /// </summary>
    /// <remarks>
    /// The field's own character in the text is an asterisk, and the atom says which one
    /// (<c>svdfppt.cxx:6984</c> tests for <c>0x2a</c>). A shape whose whole text is that one
    /// asterisk is what a master's slide-number placeholder looks like.
    /// </remarks>
    public const ushort SlideNumberMCAtom = 4056;

    /// <summary>The headers and footers of a page or of a master.</summary>
    public const ushort HeadersFooters = 4057;

    /// <summary>Which of the four running placeholders a page shows, and how it formats a date.</summary>
    public const ushort HeadersFootersAtom = 4058;

    /// <summary>Marks the position of the page's date inside a run's text.</summary>
    public const ushort GenericDateMCAtom = 4088;

    /// <summary>Marks the position of the page's header inside a run's text.</summary>
    public const ushort HeaderMCAtom = 4089;

    /// <summary>Marks the position of the page's footer inside a run's text.</summary>
    public const ushort FooterMCAtom = 4090;

    /// <summary>The document's list of external objects, hyperlinks among them.</summary>
    public const ushort ExObjList = 1033;

    /// <summary>One external hyperlink of the <see cref="ExObjList"/>.</summary>
    public const ushort ExHyperlink = 4055;

    /// <summary>
    /// An <see cref="ExHyperlink"/>'s own identifier, which an
    /// <see cref="InteractiveInfoAtom"/> names.
    /// </summary>
    public const ushort ExHyperlinkAtom = 4051;

    /// <summary>An interactive action attached to a shape or to a range of its text.</summary>
    public const ushort InteractiveInfo = 4082;

    /// <summary>Which <see cref="ExHyperlink"/> an <see cref="InteractiveInfo"/> names.</summary>
    public const ushort InteractiveInfoAtom = 4083;

    /// <summary>
    /// The character range an <see cref="InteractiveInfo"/> covers, when it covers text.
    /// </summary>
    /// <remarks>
    /// It is a <em>sibling</em> of the <see cref="InteractiveInfo"/> rather than a child, and
    /// must be the very next record for the range to be read at all
    /// (<c>filter/source/msfilter/svdfppt.cxx:6911-6921</c>).
    /// </remarks>
    public const ushort TxInteractiveInfoAtom = 4063;

    /// <summary>One of the document's three slide lists, distinguished by the header instance.</summary>
    public const ushort SlideListWithText = 4080;

    /// <summary>One edit session; the head of the chain that finds the current persist directory.</summary>
    public const ushort UserEditAtom = 4085;

    /// <summary>The atom in the <c>Current User</c> stream naming the newest edit.</summary>
    public const ushort CurrentUserAtom = 4086;

    /// <summary>Application-private tagged data hanging off a container.</summary>
    public const ushort ProgTags = 5000;

    /// <summary>One tagged block, named by the <see cref="CString"/> that opens it.</summary>
    public const ushort ProgBinaryTag = 5002;

    /// <summary>
    /// A tagged block's payload, which holds further records despite being an atom.
    /// </summary>
    /// <remarks>
    /// Its header's version is not <c>0xF</c>, so a walker that recurses only into containers
    /// steps straight over the records inside it.
    /// </remarks>
    public const ushort BinaryTagData = 5003;

    /// <summary>A block of the persist directory: persist ids to stream offsets.</summary>
    public const ushort PersistPtrIncrementalBlock = 6002;

    /// <summary>Slide-level flags added in PowerPoint 2002, including the hidden bit.</summary>
    public const ushort SlideFlags10Atom = 12010;
}

/// <summary>
/// Which master style a text run takes its unstated properties from, as the
/// <c>TextHeaderAtom</c> states.
/// </summary>
/// <remarks>
/// Mirrors <c>DFF_TextHeader</c> in <c>include/svx/msdffdef.hxx:79</c>. It doubles as the only
/// reliable statement of what a piece of text <em>is</em>: PowerPoint's own placeholder atom
/// is absent from ordinary text boxes, so a title is recognised by its text header rather than
/// by its shape.
/// </remarks>
public enum PptTextKind
{
    /// <summary>A slide title.</summary>
    Title = 0,

    /// <summary>Outline body text.</summary>
    Body = 1,

    /// <summary>Speaker notes.</summary>
    Notes = 2,

    /// <summary>Not used by any writer.</summary>
    Unused = 3,

    /// <summary>Text in an ordinary shape — the common case outside placeholders.</summary>
    Other = 4,

    /// <summary>The subtitle of a title slide.</summary>
    CentreBody = 5,

    /// <summary>The title of a title slide.</summary>
    CentreTitle = 6,

    /// <summary>The body of a two-column slide.</summary>
    HalfBody = 7,

    /// <summary>The body of a four-body slide.</summary>
    QuarterBody = 8,
}
