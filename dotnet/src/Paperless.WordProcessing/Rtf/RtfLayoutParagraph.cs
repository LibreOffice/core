using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Rtf;

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
public readonly record struct RtfLayoutRun(
    int Start,
    int Length,
    string? FamilyName,
    Length Size,
    int Weight,
    bool IsItalic,
    string? Language,
    Colour? Colour,
    Layout.Escapement Escapement = default)
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
           && Escapement == other.Escapement;
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
    IReadOnlyList<RtfLayoutFrame>? Frames = null);

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
/// <c>\shpwr</c>: 1 around, 2 tight, 3 through, 4 top and bottom, 5 none. The numbering is not the order
/// the concepts are usually listed in, and 3 and 5 are the pair that invite a swap — 3 leaves a
/// rectangular hole the text flows through the middle of, and 5 is the one that ignores the shape.
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
    Core.Geometry.Margins? WrapDistance = null);

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
    Layout.NoteRestart Restart = Layout.NoteRestart.Never);

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
public sealed record RtfLayoutTable(
    IReadOnlyList<Core.Units.Length> ColumnWidths,
    IReadOnlyList<RtfLayoutRow> Rows,
    int HeaderRowCount,
    Core.Units.Length LeftIndent,
    int SectionIndex = 0);

/// <summary>One row of an RTF table.</summary>
/// <param name="Cells">Its cells, left to right; a cell covered by a merge above is absent.</param>
/// <param name="MinHeight">Its declared height, as a magnitude.</param>
/// <param name="IsHeader">True when <c>\trhdr</c> marked it a heading row.</param>
/// <param name="HasExactHeight">
/// True when <c>\trrh</c>'s parameter was negative, which is how RTF says the height is exact rather than a
/// floor — the row is that tall and content past it is clipped.
/// </param>
public sealed record RtfLayoutRow(
    IReadOnlyList<RtfLayoutCell> Cells,
    Core.Units.Length MinHeight,
    bool IsHeader,
    bool HasExactHeight = false);

/// <summary>One cell of an RTF table.</summary>
/// <param name="Column">The grid column it starts at.</param>
/// <param name="ColumnSpan">How many grid columns it covers.</param>
/// <param name="RowSpan">How many rows it covers downwards.</param>
/// <param name="Padding">The gap between its edges and its text.</param>
/// <param name="VerticalAlignment">Where its text sits when the row is taller than its content.</param>
/// <param name="Blocks">The blocks inside it, in order — paragraphs, and any table nested in it.</param>
/// <param name="Shading">The colour behind its text, or null when it is not shaded.</param>
/// <param name="Borders">Its four borders.</param>
public sealed record RtfLayoutCell(
    int Column,
    int ColumnSpan,
    int RowSpan,
    Layout.CellPadding Padding,
    Layout.CellVerticalAlignment VerticalAlignment,
    IReadOnlyList<RtfLayoutBlock> Blocks,
    Colour? Shading = null,
    Layout.CellBorders Borders = default);
