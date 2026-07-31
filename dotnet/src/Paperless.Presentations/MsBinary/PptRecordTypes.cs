namespace Paperless.Presentations.MsBinary;

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

    /// <summary>A text run's characters, one byte each.</summary>
    public const ushort TextBytesAtom = 4008;

    /// <summary>Per-run language and spelling information.</summary>
    public const ushort TextSpecInfoAtom = 4010;

    /// <summary>One font in the font collection.</summary>
    public const ushort FontEntityAtom = 4023;

    /// <summary>A length-prefixed UTF-16 string, used for names throughout.</summary>
    public const ushort CString = 4026;

    /// <summary>The drawing group: the document-wide Escher state.</summary>
    public const ushort DrawingGroup = 1035;

    /// <summary>One page's Escher drawing.</summary>
    public const ushort Drawing = 1036;

    /// <summary>The headers and footers of a master.</summary>
    public const ushort HeadersFooters = 4057;

    /// <summary>One of the document's three slide lists, distinguished by the header instance.</summary>
    public const ushort SlideListWithText = 4080;

    /// <summary>One edit session; the head of the chain that finds the current persist directory.</summary>
    public const ushort UserEditAtom = 4085;

    /// <summary>The atom in the <c>Current User</c> stream naming the newest edit.</summary>
    public const ushort CurrentUserAtom = 4086;

    /// <summary>Application-private tagged data hanging off a container.</summary>
    public const ushort ProgTags = 5000;

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
