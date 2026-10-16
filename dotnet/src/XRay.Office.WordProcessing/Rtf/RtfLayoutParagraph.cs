using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Fonts;

namespace XRay.Office.WordProcessing.Rtf;

/// <summary>
/// One stretch of an RTF paragraph's text and the character formatting in force over it.
/// </summary>
/// <remarks>
/// Only what a page needs: what decides a glyph's width, and the colour it is drawn in. RTF states these
/// as loose control words rather than as a property set, so the state in force is snapshotted as the text
/// is appended — there is no group to point back at afterwards.
/// </remarks>
/// <param name="Start">Its first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="FamilyName">The family the font table names, or null when it names none.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
/// <param name="Colour">The colour the text is drawn in, or null for the automatic colour.</param>
/// <param name="Escapement">
/// The superscript or subscript <c>\super</c> and <c>\sub</c> ask for, unresolved. Unresolved because both
/// halves of it — the rise and the smaller size — are fractions of the <em>face's</em> height and size, and
/// this reader has no faces: it records what the token stream said and the layout reader loads the fonts.
/// </param>
/// <param name="CaseMap">The case <c>\caps</c> or <c>\scaps</c> draws the run in.</param>
/// <param name="Highlight">The band <c>\highlight</c> draws behind the run, or null when it has none.</param>
/// <param name="Underline">How the <c>\ul…</c> words in force rule the run — <c>\uldb</c> and
/// <c>\ululdbwave</c> with two lines and the other fifteen with one.</param>
/// <param name="IsStruckThrough">True when <c>\strike</c> or <c>\striked</c> draws one through it.</param>
/// <param name="AutoKerning">
/// True when a nonzero <c>\kerning</c> asks for the run's pairs to be kerned. Off unless it does.
/// </param>
/// <param name="WidthPerCent">
/// The character width <c>\charscalex</c> states, as a percentage.
/// <para>
/// It multiplies every advance in the run rather than adding to it, so it has to survive
/// <see cref="RtfLayoutRun.MatchesFormatting"/> as well as the uniform-paragraph shortcut: a producer
/// restates <c>\f0\fs22</c> before every run whether or not anything changed, and the merge is what
/// keeps those restatements from breaking the shaping context — but a property missing from the
/// comparison lets a scaled stretch be absorbed into the unscaled run beside it and take its width.
/// </para>
/// </param>
/// <param name="Link">
/// Where the run leads when it is clicked, or null when it is not a link: an address, or <c>#name</c> for a
/// bookmark in the same document. Part of <see cref="MatchesFormatting"/>, so two runs that differ only in
/// where they lead are not merged.
/// </param>
public readonly record struct RtfLayoutRun(
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
    TextUnderline Underline = TextUnderline.None,
    bool IsStruckThrough = false,
    bool AutoKerning = false,
    int WidthPerCent = 100,
    string? Link = null)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>
    /// True when another run's formatting is identical, whatever its range.
    /// </summary>
    /// <remarks>
    /// What decides whether two adjacent stretches merge. RTF restates properties freely — a producer
    /// writes <c>\f0\fs22</c> before every run whether or not anything changed — and an unmerged
    /// restatement would break the shaping context for no reason, losing a kern pair mid-word.
    /// </remarks>
    public bool MatchesFormatting(RtfLayoutRun other)
        => string.Equals(FamilyName, other.FamilyName, StringComparison.Ordinal)
           && Size == other.Size
           && Weight == other.Weight
           && IsItalic == other.IsItalic
           && string.Equals(Language, other.Language, StringComparison.Ordinal)
           && Colour == other.Colour
           && Escapement == other.Escapement
           && CaseMap == other.CaseMap
           && Highlight == other.Highlight
           && Underline == other.Underline
           && IsStruckThrough == other.IsStruckThrough
           && AutoKerning == other.AutoKerning
           && WidthPerCent == other.WidthPerCent
           && string.Equals(Link, other.Link, StringComparison.Ordinal);
}

/// <summary>
/// One of an RTF body's paragraphs, with the formatting layout needs.
/// </summary>
/// <remarks>
/// <para>
/// Collected during the content walk rather than by a second pass, which is the one structural
/// difference from the XML formats' layout sources. RTF is a token stream with nothing to revisit —
/// re-reading it would mean running the whole state machine again, including its encoding and
/// destination handling, and two runs could then disagree about what the document says. So the state in
/// force is recorded as each paragraph closes.
/// </para>
/// <para>
/// The properties themselves are translated through <see cref="Ww8.Ww8LayoutFormat"/>, because RTF and
/// the binary format state them the same way: twips throughout, a font size in half-points, and a line
/// spacing whose sign and companion flag together choose between a multiple, a minimum and a fixed
/// height. Writing that arithmetic twice would be two chances to get the sign wrong.
/// </para>
/// </remarks>
/// <param name="Text">The paragraph's text.</param>
/// <param name="Format">Its resolved layout properties.</param>
/// <param name="FamilyName">The family the font table names, or null when it names none.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1-1000 scale.</param>
/// <param name="IsItalic">True when the text is italic.</param>
/// <param name="Language">A BCP 47 tag, or null when the document states none.</param>
/// <param name="Colour">The colour the paragraph's mark carries, or null for the automatic colour.</param>
/// <param name="Runs">
/// The stretches its character formatting divides it into, in order. Always populated, even where the
/// whole paragraph is uniform — the layout source decides whether they are worth carrying, since it is
/// the only party that can compare two <em>resolved</em> faces rather than two requested families.
/// </param>
/// <param name="SectionIndex">Which of the document's sections the paragraph sits in.</param>
/// <param name="Notes">The notes anchored in the paragraph's text, or null when it cites none.</param>
/// <param name="Frames">The floating frames anchored in it, or null when it anchors none.</param>
/// <param name="PageFields">
/// The <c>PAGE</c> and <c>NUMPAGES</c> results in <paramref name="Text"/>, or null when it holds none.
/// <para>
/// Recorded rather than left at the producer's cached value for the reason
/// <see cref="Layout.PageFields"/> gives at length: the cached result is right on one page of the
/// document and wrong on every other, so a footer reading <c>Page 9</c> is drawn on all nine pages. All
/// three other word-processing readers have recorded them since round 45 and this one did not, which is
/// why an RTF running head printed one number throughout.
/// </para>
/// </param>
/// <param name="ListMarker">
/// The label this item draws, or null when it draws none.
/// <para>
/// RTF is the one format of the four that writes the label out rather than the counters:
/// <c>{\listtext …}</c> holds the rendered <c>1.</c> or <c>•</c> together with the tab that follows it,
/// so nothing has to be counted and the marker is by definition the one the writer displayed. The
/// trailing tab is trimmed off, because the distance it stands for is the hanging indent the same
/// paragraph states in <c>\fi</c>.
/// </para>
/// </param>
/// <param name="AutoKerning">
/// True when a nonzero <c>\kerning</c> is in force at the paragraph's mark, which is what a paragraph
/// with no runs of its own is set in and what its label is drawn in.
/// </param>
/// <param name="WidthPerCent">
/// The character width <c>\charscalex</c> states at the paragraph's mark, as a percentage. The mark's,
/// for the same reason as <paramref name="AutoKerning"/>: a paragraph set end to end in one scaled
/// style carries no runs at all and is uniform by every test the layout reader makes.
/// </param>
/// <param name="Anchors">
/// The bookmarks the paragraph holds, with the offsets of its text they sit at, or null for none.
/// </param>
/// <param name="OutlineLevel">The paragraph's outline level, 1 and up, or 0 for body text.</param>
public readonly record struct RtfLayoutParagraph(
    string Text,
    ParagraphFormat Format,
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language,
    Colour? Colour = null,
    IReadOnlyList<RtfLayoutRun>? Runs = null,
    int SectionIndex = 0,
    IReadOnlyList<RtfLayoutNote>? Notes = null,
    IReadOnlyList<RtfLayoutFrame>? Frames = null,
    IReadOnlyList<Layout.PageFieldSpan>? PageFields = null,
    string? ListMarker = null,
    bool AutoKerning = false,
    int WidthPerCent = 100,
    IReadOnlyList<RtfLayoutAnchor>? Anchors = null,
    int OutlineLevel = 0);

/// <summary>
/// A floating frame as RTF states it: a shape's rectangle, its wrap, and the text inside it.
/// </summary>
/// <remarks>
/// <para>
/// RTF says almost nothing about a shape in RTF's own syntax. <c>\shpleft</c> and its three companions
/// give the rectangle in twips and <c>\shpwr</c> the wrap; everything else — the fill, the line, the
/// distances from text, and what the coordinates are relative to when <c>\shpbxignore</c> is set — lives
/// in <c>{\sp{\sn name}{\sv value}}</c> pairs, which are Escher's property table written out as text.
/// Those pairs sit inside <c>{\*\shpinst}</c>, an ignorable destination this reader skips whole, so what
/// is read here is the geometry and the wrap and nothing else.
/// </para>
/// <para>
/// Kept as the file's own numbers rather than resolved, for the same reason the notes are: the layout
/// reader is the half that has the fonts and the page geometry, and translating twice would be two
/// chances to disagree.
/// </para>
/// </remarks>
/// <param name="Offset">Where the shape sits in the anchoring paragraph's text.</param>
/// <param name="Left">The rectangle's left edge, in twips, relative to whatever the origin is.</param>
/// <param name="Top">Its top edge.</param>
/// <param name="Right">Its right edge.</param>
/// <param name="Bottom">Its bottom edge.</param>
/// <param name="Wrap">
/// <c>\shpwr</c>, kept as the file's own number because the reading of it belongs with the layout.
/// LibreOffice's dispatch (<c>rtfdispatchvalue.cxx</c>:1222-1247) is what this project is measured
/// against and it is not the specification's list: <b>1 puts no text beside the shape at all</b>, 2 and
/// 4 put text on both sides, and <b>3 and 5 are both <em>through</em></b>. See
/// <c>RtfReader.WrapOf</c>, which carries the probe.
/// </param>
/// <param name="WrapSide"><c>\shpwrk</c>: 0 both sides, 1 left, 2 right, 3 the larger side.</param>
/// <param name="HorizontalOrigin">Which <c>\shpbx*</c> word was seen, or null for none.</param>
/// <param name="VerticalOrigin">Which <c>\shpby*</c> word was seen, or null for none.</param>
/// <param name="Blocks">The shape's own text, from <c>{\shptxt}</c>.</param>
/// <param name="WrapDistance">
/// How far text must stay clear, or null when the shape said nothing about it. Null and zero are
/// different answers: LibreOffice's RTF import supplies 0.2 cm on every side when the shape states none,
/// which is measured rather than assumed — the corpus document's wrapped lines start 114 twips past the
/// shape's right edge with the property absent and 1 twip past it with the property present and zero.
/// </param>
public sealed record RtfLayoutFrame(
    int Offset,
    int Left,
    int Top,
    int Right,
    int Bottom,
    int Wrap,
    int WrapSide,
    string? HorizontalOrigin,
    string? VerticalOrigin,
    IReadOnlyList<RtfLayoutBlock> Blocks,
    Core.Geometry.Margins? WrapDistance = null)
{
    /// <summary>
    /// True when this is a <c>{\pict}</c> in the run of text rather than a <c>{\shp}</c> beside it.
    /// </summary>
    /// <remarks>
    /// One record for both because the two really are one thing to layout — a rectangle anchored in a
    /// paragraph — and they differ only in what the rectangle is measured from. A shape states a
    /// position; an inline picture has none, and hangs where its offset in the text puts it.
    /// </remarks>
    public bool IsInline { get; init; }

    /// <summary>The picture the frame holds, or nothing when it holds none.</summary>
    public FramePicture Picture { get; init; }

    /// <summary>
    /// The inset between the shape's edge and its own text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Escher's <c>dxTextLeft</c>, <c>dyTextTop</c>, <c>dxTextRight</c> and <c>dyTextBottom</c>, which
    /// <c>RTFSdrImport::resolve</c> divides by 360 to get 1/100 mm and hands to the frame's four
    /// <c>*BorderDistance</c> properties (<c>rtfsdrimport.cxx</c>:600-625).
    /// </para>
    /// <para>
    /// <b>The default is not zero.</b> A shape that states none of the four keeps
    /// <c>getTextFrameDefaults</c>'s (<c>rtfsdrimport.cxx</c>:111-124) <c>91440 / 360</c> = 254 across
    /// and <c>45720 / 360</c> = 127 down — 0.1 inch and 0.05 inch — so a reader that supplies nothing
    /// puts a shape's first line 7.2 pt to the left of where the reference draws it and gives every
    /// line 14.4 pt more room than the reference has. Measured on a wrapping probe against 26.2.4.2.
    /// </para>
    /// </remarks>
    public Core.Geometry.Margins TextInset { get; init; } = DefaultTextInset;

    /// <summary>
    /// <c>getTextFrameDefaults</c>' four border distances, which are what a shape stating none gets.
    /// </summary>
    public static Core.Geometry.Margins DefaultTextInset { get; } = new(
        Core.Units.Length.FromMm100(254),
        Core.Units.Length.FromMm100(127),
        Core.Units.Length.FromMm100(254),
        Core.Units.Length.FromMm100(127));
}

/// <summary>
/// A footnote or endnote as layout sees it: where it is cited, and the blocks of its body.
/// </summary>
/// <remarks>
/// Collected while the citing paragraph is still open, because that is how RTF writes it — the
/// <c>{\*\footnote …}</c> group sits immediately after the <c>\chftn</c> that references it, part way through
/// the sentence rather than in a store of its own. So the note is finished before the paragraph that carries
/// it is, and waits on the flow until the paragraph closes.
/// </remarks>
/// <param name="Offset">Where the citation sits in the citing paragraph's text.</param>
/// <param name="IsEndnote">True for an endnote, which collects at the end of the document.</param>
/// <param name="Blocks">The note's body.</param>
/// <param name="Placement">Where it collects, which for an endnote is not always the end of the document.</param>
/// <param name="Restart">Where the count begins again, from the <c>\ftnrst*</c> family.</param>
public sealed record RtfLayoutNote(
    int Offset,
    bool IsEndnote,
    IReadOnlyList<RtfLayoutBlock> Blocks,
    Layout.NotePlacement Placement = Layout.NotePlacement.PageBottom,
    Layout.NoteRestart Restart = Layout.NoteRestart.Never)
{
    /// <summary>How the note's class is numbered, for a pagination pass that numbers it again.</summary>
    public Layout.NoteNumbering Numbering { get; init; } = Layout.NoteNumbering.Footnotes;

    /// <summary>The citation as it was read, which sits at <see cref="Offset"/> and at the note's head.</summary>
    public string Citation { get; init; } = "";
}

/// <summary>
/// A bookmark's place in a paragraph, whose name may not be known yet.
/// </summary>
/// <remarks>
/// A class, not a record, because RTF settles a bookmark's name only when its closing half is read — often in
/// a later paragraph — and the paragraph that holds the opening half has long since been recorded by then.
/// Layout reads <see cref="Name"/> after the whole document has been parsed.
/// </remarks>
public sealed class RtfLayoutAnchor
{
    /// <summary>The offset of the paragraph's text the bookmark opens at.</summary>
    public int Offset { get; set; }

    /// <summary>The bookmark's name, empty until its closing half has been read.</summary>
    public string Name { get; set; } = string.Empty;
}

/// <summary>
/// One block of an RTF flow as layout sees it: a paragraph or a table, never both.
/// </summary>
/// <remarks>
/// A wrapper struct rather than a class hierarchy because <see cref="RtfLayoutParagraph"/> is a struct, and
/// making it a class to gain a base type would allocate one per paragraph of every document read — for a
/// distinction only the body's block list needs. Exactly one of the two is non-null; the constructors are
/// the only way to build one, so that stays true.
/// </remarks>
public readonly record struct RtfLayoutBlock
{
    /// <summary>Wraps a paragraph.</summary>
    public RtfLayoutBlock(RtfLayoutParagraph paragraph) => Paragraph = paragraph;

    /// <summary>Wraps a table.</summary>
    public RtfLayoutBlock(RtfLayoutTable table) => Table = table;

    /// <summary>The paragraph, when this block is one.</summary>
    public RtfLayoutParagraph? Paragraph { get; }

    /// <summary>The table, when this block is one.</summary>
    public RtfLayoutTable? Table { get; }
}

/// <summary>
/// A table as layout sees it: the column grid in twips, and cells holding paragraphs.
/// </summary>
/// <remarks>
/// The grid comes from the <c>\cellx</c> edges the extraction pass already resolved, which is where RTF
/// keeps it: an edge is a cell's <em>right</em> boundary in twips from the row's left, so the widths are
/// the differences between consecutive edges and the merges fall out of the same arithmetic. Recording the
/// widths rather than the edges is what the layout engine wants, and converting once here keeps the two
/// conventions from meeting anywhere else.
/// </remarks>
/// <param name="ColumnWidths">The grid's column widths, left to right.</param>
/// <param name="Rows">The rows, top to bottom.</param>
/// <param name="HeaderRowCount">How many rows at the top repeat across a page break.</param>
/// <param name="LeftIndent">How far the table's left edge sits from the body area's.</param>
/// <param name="SectionIndex">Which of the document's sections the table sits in.</param>
/// <param name="ColumnFit">
/// How to size the columns when the <c>\cellx</c> edges stated none, and null when they stated them all.
/// </param>
/// <param name="IsPositioned">
/// True when the row definitions made this a <em>positioned</em> table — RTF's Positioned Wrapped
/// Tables, which LibreOffice's importer turns into <c>w:tblpPr</c> and Writer into a fly holding a
/// table. See <see cref="Layout.PageTable.IsPositioned"/>, which is where it ends up.
/// </param>
/// <param name="VerticalOrigin">What <paramref name="VerticalOffset"/> is measured from.</param>
/// <param name="VerticalOffset"><c>\tposy</c>: how far below that origin the table's top goes.</param>
/// <param name="HorizontalPosition">
/// <c>\tposxc</c> and friends: the edge the table aligns against instead of taking
/// <paramref name="LeftIndent"/>, and null when it stated none or anchored to the page.
/// </param>
/// <param name="LowerSpacing"><c>\tdfrmtxtBottom</c>: the gap the flow keeps below the fly.</param>
public sealed record RtfLayoutTable(
    IReadOnlyList<Core.Units.Length> ColumnWidths,
    IReadOnlyList<RtfLayoutRow> Rows,
    int HeaderRowCount,
    Core.Units.Length LeftIndent,
    int SectionIndex = 0,
    Layout.TableColumnFit? ColumnFit = null,
    bool IsPositioned = false,
    Layout.FrameVerticalOrigin VerticalOrigin = Layout.FrameVerticalOrigin.Paragraph,
    Core.Units.Length VerticalOffset = default,
    Layout.FrameHorizontalAlignment? HorizontalPosition = null,
    Core.Units.Length LowerSpacing = default);

/// <summary>One row of an RTF table.</summary>
/// <param name="Cells">Its cells, left to right; a cell covered by a merge above is absent.</param>
/// <param name="MinHeight">Its declared height, as a magnitude.</param>
/// <param name="IsHeader">True when <c>\trhdr</c> marked it a heading row.</param>
/// <param name="HasExactHeight">
/// True when <c>\trrh</c>'s parameter was negative, which is how RTF says the height is exact rather than a
/// floor — the row is that tall and content past it is clipped.
/// </param>
/// <param name="CanSplit">
/// False when <c>\trkeep</c> forbade breaking the row across a page — see
/// <see cref="Layout.PageTableRow.CanSplit"/>.
/// </param>
/// <param name="CoveredTopRule">
/// The widest top rule stated by the cells this row drops because they continue a vertical merge —
/// charged to the row's height and drawn nowhere. See <see cref="Layout.PageTableRow.CoveredTopRule"/>.
/// </param>
/// <param name="CoveredBottomRule">
/// Their widest bottom rule, which is charged to the band below the row and <em>is</em> drawn, because it
/// is the merge's own outer edge. See <see cref="Layout.PageTableRow.CoveredBottomRule"/>.
/// </param>
public sealed record RtfLayoutRow(
    IReadOnlyList<RtfLayoutCell> Cells,
    Core.Units.Length MinHeight,
    bool IsHeader,
    bool HasExactHeight = false,
    bool CanSplit = true,
    Core.Units.Length CoveredTopRule = default,
    Core.Units.Length CoveredBottomRule = default);

/// <summary>One cell of an RTF table.</summary>
/// <param name="Column">The grid column it starts at.</param>
/// <param name="ColumnSpan">How many grid columns it covers.</param>
/// <param name="RowSpan">How many rows it covers downwards.</param>
/// <param name="Padding">The gap between its edges and its text.</param>
/// <param name="VerticalAlignment">Where its text sits when the row is taller than its content.</param>
/// <param name="Blocks">The blocks inside it, in order — paragraphs, and any table nested in it.</param>
/// <param name="Shading">The colour behind its text, or null when it is not shaded.</param>
/// <param name="Borders">Its four borders.</param>
/// <param name="TextDirection">
/// Which way its text runs — RTF's <c>\cltxbtlr</c> and its siblings, which LibreOffice's tokeniser
/// turns into <c>w:textDirection</c> (<c>rtfdispatchflag.cxx</c>:483-508).
/// </param>
public sealed record RtfLayoutCell(
    int Column,
    int ColumnSpan,
    int RowSpan,
    Layout.CellPadding Padding,
    Layout.VerticalTextAlignment VerticalAlignment,
    IReadOnlyList<RtfLayoutBlock> Blocks,
    Colour? Shading = null,
    Layout.CellBorders Borders = default,
    Layout.CellTextDirection TextDirection = Layout.CellTextDirection.LeftToRight);
