using System.Globalization;
using System.Xml.Linq;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// Which of a table style's conditional layers a table has switched on — <c>w:tblLook</c>.
/// </summary>
/// <remarks>
/// <para>
/// A table style states its formatting in parts: an unconditional one and up to twelve
/// <c>w:tblStylePr</c> layers, each naming a region of the table it applies to. The layers are inert
/// until the table asks for them, and <c>w:tblLook</c> is that request. So the same style makes one
/// table's first row bold and leaves another's plain, and reading the style without reading the look
/// is how a reader ends up applying either all of it or none of it.
/// </para>
/// <para>
/// Both spellings are read because both are in the corpus. Word 2007 wrote a hexadecimal bitmask in
/// <c>w:val</c>; Word 2010 and later write the same bits as named attributes and usually keep the
/// mask beside them. Where a file states the attributes they win, since they are the newer and
/// unambiguous form — which is the precedence <c>DomainMapperTableManager::sprm</c> applies.
/// </para>
/// </remarks>
/// <param name="FirstRow">The heading row takes <c>firstRow</c>.</param>
/// <param name="LastRow">The final row takes <c>lastRow</c>.</param>
/// <param name="FirstColumn">The leading column takes <c>firstCol</c>.</param>
/// <param name="LastColumn">The trailing column takes <c>lastCol</c>.</param>
/// <param name="HorizontalBanding">Rows take <c>band1Horz</c>/<c>band2Horz</c> in turn.</param>
/// <param name="VerticalBanding">Columns take <c>band1Vert</c>/<c>band2Vert</c> in turn.</param>
public readonly record struct WordTableLook(
    bool FirstRow,
    bool LastRow,
    bool FirstColumn,
    bool LastColumn,
    bool HorizontalBanding = false,
    bool VerticalBanding = false)
{
    /// <summary>What a table stating no <c>w:tblLook</c> at all asks for: nothing.</summary>
    public static WordTableLook None => default;

    /// <summary>Reads the look off a table's <c>w:tblPr</c>.</summary>
    /// <remarks>
    /// <para>
    /// The bits are §17.4.56's: <c>0x0020</c> first row, <c>0x0040</c> last row, <c>0x0080</c> first
    /// column, <c>0x0100</c> last column, <c>0x0200</c> <em>no</em> horizontal banding and
    /// <c>0x0400</c> <em>no</em> vertical banding.
    /// </para>
    /// <para>
    /// <strong>The two band bits are stated the other way up</strong> — the attribute is
    /// <c>noHBand</c>, not <c>hBand</c> — so a table that says nothing about banding is banded, and
    /// reading them like the other four turns banding on exactly where the file turns it off. That is
    /// the one asymmetry in this element and it is worth the extra reader.
    /// </para>
    /// <para>
    /// A table stating no <c>w:tblLook</c> at all still gets <see cref="None"/>, banding included: an
    /// absent look asks for no conditional formatting rather than for the default one.
    /// </para>
    /// </remarks>
    /// <param name="tableProperties">The table's <c>w:tblPr</c>, or null.</param>
    public static WordTableLook Read(XElement? tableProperties)
    {
        XElement? look = Word.Child(tableProperties, "tblLook");
        if (look is null) return None;

        int mask = 0;
        string? value = Word.Attribute(look, "val");
        if (value is { Length: > 0 })
        {
            _ = int.TryParse(value, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out mask);
        }

        return new WordTableLook(
            Flag(look, "firstRow", mask, 0x0020),
            Flag(look, "lastRow", mask, 0x0040),
            Flag(look, "firstColumn", mask, 0x0080),
            Flag(look, "lastColumn", mask, 0x0100),
            !Flag(look, "noHBand", mask, 0x0200),
            !Flag(look, "noVBand", mask, 0x0400));

        static bool Flag(XElement look, string name, int mask, int bit)
            => Word.Attribute(look, name) switch
            {
                "1" or "true" or "on" => true,
                "0" or "false" or "off" => false,
                _ => (mask & bit) != 0,
            };
    }
}

/// <summary>
/// The <c>w:tblStylePr</c> layers that apply to one cell, in the order they are consulted.
/// </summary>
/// <remarks>
/// <para>
/// §17.7.6 fixes both which layers a cell is in and which of them wins where two disagree. The
/// specification lists them in the order they are <em>applied</em> — whole table, then the bands,
/// then the first and last column, then the first and last row, then the four corner cells — so the
/// last applied is the most specific. <see cref="Names"/> hands them back the other way up, most
/// specific first, because a resolver stops at the first layer that states the property it wants.
/// </para>
/// <para>
/// <b>The band layers are here, and the document that measures them is
/// <c>012_Project_Timeline_Template_Black_and_Brown_Theme</c>.</b> The remark this replaces said the
/// bands were left "for a round that has a document to measure them on"; that document draws
/// <b>48 <c>#F2F2F2</c> fills on its table rows 2, 4, 6 and 8</b> against none of ours, from
/// <c>band1Horz</c> at <c>w:tblStyleRowBandSize="1"</c>, and its <c>firstCol</c> layer draws eight
/// more. The band a cell falls in is counted with the <c>firstRow</c> and <c>lastRow</c> regions
/// excluded — <c>012</c> fixes that, since counting the heading row would put its bands on rows
/// 3, 5, 7 and 9 instead — and band 1 is the <em>first</em> band, so a zero-based index that is even
/// takes <c>band1Horz</c>.
/// </para>
/// <para>
/// Adding them here also hands them to <see cref="WordStyles.TableStyleRunProperties"/>, which is the
/// half that could move a line break. Measured before the change rather than reasoned about: two
/// corpus documents declare a band layer carrying a <c>w:rPr</c> and in both the styles are latent —
/// <b>no table in the corpus names a style whose <c>w:basedOn</c> chain reaches one, 0 of 271</b>
/// (<c>probes/words-r63/tblstylepr-census.py</c>).
/// </para>
/// <para>
/// <b>The <c>w:pPr</c> and <c>w:tblPr</c> halves of a layer are still not applied</b>, and that is
/// scope rather than principle. The <c>w:tcPr</c> half is, for shading only: 749 cells in 42
/// documents (<c>probes/words-r63/tblstyle-reach.py</c>). A layer's <c>w:tcBorders</c> is read by
/// nothing, which is one of the eight strokes <c>012</c> is still missing.
/// </para>
/// </remarks>
/// <param name="Look">Which layers the table switched on.</param>
/// <param name="IsFirstRow">Whether the cell is in the table's first row.</param>
/// <param name="IsLastRow">Whether the cell is in the table's last row.</param>
/// <param name="IsFirstColumn">Whether the cell is in the leading grid column.</param>
/// <param name="IsLastColumn">Whether the cell is in the trailing grid column.</param>
/// <param name="RowBand">
/// Which horizontal band the cell's row falls in, zero-based, counting only rows outside the
/// <c>firstRow</c> and <c>lastRow</c> regions and already divided by
/// <c>w:tblStyleRowBandSize</c>; null for a row in one of those regions, or when the caller has no
/// band size to count with.
/// </param>
/// <param name="ColumnBand">The same for vertical bands and <c>w:tblStyleColBandSize</c>.</param>
public readonly record struct WordTableStyleConditions(
    WordTableLook Look,
    bool IsFirstRow,
    bool IsLastRow,
    bool IsFirstColumn,
    bool IsLastColumn,
    int? RowBand = null,
    int? ColumnBand = null)
{
    /// <summary>A cell in no conditional region at all.</summary>
    public static WordTableStyleConditions None => default;

    /// <summary>
    /// The layer names that apply, most specific first.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A corner cell's layer is offered only when <em>both</em> of the edges meeting there are switched
    /// on, which is what makes <c>nwCell</c> a refinement of <c>firstRow</c> and <c>firstCol</c> rather
    /// than a third independent thing. <c>wholeTable</c> is last and needs no bit: a style's
    /// unconditional formatting applies to every cell of every table that names it.
    /// </para>
    /// <para>
    /// <strong>A cell in a <c>firstRow</c> or <c>lastRow</c> region is in no horizontal band at all</strong>
    /// — not "in band 1" — and the same for the column edges and the vertical bands. That is the same rule
    /// as the one that excludes those rows from the count, seen from the other end, and getting it wrong
    /// puts a band fill on the heading row where the file states one.
    /// </para>
    /// </remarks>
    public IReadOnlyList<string> Names
    {
        get
        {
            bool first = Look.FirstRow && IsFirstRow;
            bool last = Look.LastRow && IsLastRow;
            bool leading = Look.FirstColumn && IsFirstColumn;
            bool trailing = Look.LastColumn && IsLastColumn;

            List<string> names = [];

            if (first && leading) names.Add("nwCell");
            if (first && trailing) names.Add("neCell");
            if (last && leading) names.Add("swCell");
            if (last && trailing) names.Add("seCell");

            if (first) names.Add("firstRow");
            if (last) names.Add("lastRow");
            if (leading) names.Add("firstCol");
            if (trailing) names.Add("lastCol");

            // §17.7.6 applies the bands before the edges and after the whole table, so they are less
            // specific than either edge and more specific than the unconditional half. A cell is in at
            // most one of each pair, so the two orders within a pair cannot both fire.
            if (Look.HorizontalBanding && !first && !last && RowBand is { } row and >= 0)
                names.Add(row % 2 == 0 ? "band1Horz" : "band2Horz");

            if (Look.VerticalBanding && !leading && !trailing && ColumnBand is { } column and >= 0)
                names.Add(column % 2 == 0 ? "band1Vert" : "band2Vert");

            names.Add("wholeTable");
            return names;
        }
    }
}
