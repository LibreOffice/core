using System.Buffers.Binary;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Ww8;

/// <content>
/// A table cell's borders and shading: the <c>BRC</c> and <c>SHD</c> structures, and the two spellings
/// each of them has.
/// </content>
/// <remarks>
/// <para>
/// Both are stated twice over, and a document written by any recent producer states both — an older form
/// naming a colour by its index in Word's seventeen-entry palette, and a newer one carrying full RGB. The
/// newer wins wherever it says anything, which is the rule LibreOffice applies
/// (<c>ww8par2.cxx</c>, <c>SetTabShades</c> and the <c>aTSetBrc90s</c> pass that runs after
/// <c>aTSetBrcs</c>). Reading only the older form gives <c>#C0C0C0</c> where the document says
/// <c>#CCCCCC</c> — close enough to look right and far enough to fail a comparison.
/// </para>
/// <para>
/// Neither is stated per cell in the markup sense. A border is a <c>BRC</c> applied to a <em>range</em> of
/// cells and a set of sides, and shading is one array per row indexed by cell — so both are resolved
/// against a cell's index in the row that <see cref="Ww8TableDefinition"/> defines, and both live on the
/// paragraph whose mark <em>ends</em> that row.
/// </para>
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>The size of a WW8 <c>BRC</c>: a width, a type, a palette index and flags.</summary>
    private const int Brc80Size = 4;

    /// <summary>The size of a WW9 <c>BRC</c>, which is the same plus a four-byte colour in front.</summary>
    private const int Brc90Size = 8;

    /// <summary>How many bytes one cell's entry takes in the newer, RGB shading array.</summary>
    private const int RgbShadingSize = 10;

    /// <summary>
    /// Reads a <c>BRC</c>, in whichever of its two forms.
    /// </summary>
    /// <param name="bytes">The structure's bytes; only the first four or eight are read.</param>
    /// <param name="isVersion9">True for the newer form, whose colour is RGB rather than an index.</param>
    /// <returns>
    /// The border, or null when the structure states none at all — which is not the same as stating that
    /// there is no border. A <c>brcType</c> of zero means "nothing said here", and falls through to
    /// whatever the table's own defaults say; a <em>nil</em> BRC means "no border", and does not.
    /// </returns>
    private static Ww8Border? ReadBorder(ReadOnlySpan<byte> bytes, bool isVersion9)
    {
        if (isVersion9)
        {
            if (bytes.Length < Brc90Size) return null;

            // Nil is all ones across the four bytes that are not the colour, which is why it has to be
            // tested before the type: a nil BRC's type reads as 0xFF and would otherwise be a border of
            // some unknown style rather than the absence of one.
            if (BinaryPrimitives.ReadUInt32LittleEndian(bytes[4..]) == 0xFFFFFFFF)
                return new Ww8Border(Ww8Border.Nil, 0, null);

            return bytes[5] == Ww8Border.Unset
                ? null
                : new Ww8Border(bytes[5], bytes[4], ColourOf(bytes[..4]));
        }

        if (bytes.Length < Brc80Size) return null;
        if (bytes[0] == 0xFF && bytes[1] == 0xFF) return new Ww8Border(Ww8Border.Nil, 0, null);

        return bytes[1] == Ww8Border.Unset
            ? null
            : new Ww8Border(bytes[1], bytes[0], IcoPalette[bytes[2] < IcoPalette.Length ? bytes[2] : 0]);
    }

    /// <summary>
    /// Reads one <c>sprmTSetBrc</c> or <c>sprmTSetBrc80</c>: a range of cells, a set of sides, and a BRC.
    /// </summary>
    /// <remarks>
    /// The one structural difference from every other cell property WW8 has: it names a half-open range of
    /// cells rather than one, so a row whose four cells all gain a left border is a single sprm and a row
    /// whose second cell alone gains one is a sprm covering cells 1 to 2. A reader that applied it to the
    /// first cell only draws a quarter of the borders in the document.
    /// </remarks>
    private static Ww8CellBorderChange? ReadCellBorderChange(
        ReadOnlyMemory<byte> operand, bool isVersion9)
    {
        ReadOnlySpan<byte> bytes = operand.Span;
        if (bytes.Length < 3) return null;

        int first = bytes[0];
        int limit = bytes[1];
        if (first >= limit || limit > MaxTableColumns + 1) return null;

        // A BRC that states nothing is still a change: it clears whatever the cell descriptor said, and
        // the table's defaults then fill the side back in. LibreOffice assigns the parsed BRC to the
        // selected sides unconditionally, zeroes and all.
        return new Ww8CellBorderChange(first, limit, bytes[2], ReadBorder(bytes[3..], isVersion9));
    }

    /// <summary>
    /// Reads a <c>sprmTTableBorders</c>: the six borders a table falls back on where a cell states none.
    /// </summary>
    /// <remarks>
    /// Six rather than four, because the two extra are the ones a cell in the middle of the table uses:
    /// the inside horizontal and the inside vertical. Which of the six a given side falls back to depends
    /// on where the cell sits — <see cref="Ww8TableBorders.For"/> — and that is why Word can write a table
    /// with a thick outline and hairline gridlines as one sprm rather than one per cell.
    /// </remarks>
    private static Ww8TableBorders? ReadTableBorders(ReadOnlyMemory<byte> operand, bool isVersion9)
    {
        int size = isVersion9 ? Brc90Size : Brc80Size;
        if (operand.Length < 6 * size) return null;

        Ww8Border? At(int index) => ReadBorder(operand.Span.Slice(index * size, size), isVersion9);

        return new Ww8TableBorders(At(0), At(1), At(2), At(3), At(4), At(5));
    }

    /// <summary>
    /// Reads a <c>sprmTDefTableShd80</c>: one two-byte <c>SHD</c> per cell.
    /// </summary>
    /// <remarks>
    /// Two five-bit palette indices and a pattern, packed into a word. The pattern is what decides which of
    /// the two colours shows: with the usual "clear" pattern it is the <em>background</em>, and reading the
    /// foreground instead paints an ordinary grey cell black. It is the same trap DOCX's <c>w:color</c> and
    /// RTF's <c>\clcfpat</c> set.
    /// </remarks>
    private static Colour?[]? ReadPaletteShading(ReadOnlyMemory<byte> operand)
    {
        ReadOnlySpan<byte> bytes = operand.Span;
        int cells = Math.Min(bytes.Length / 2, MaxTableColumns);
        if (cells == 0) return null;

        Colour?[] shades = new Colour?[cells];
        for (int i = 0; i < cells; i++)
        {
            ushort packed = BinaryPrimitives.ReadUInt16LittleEndian(bytes[(2 * i)..]);

            // A wholly zero SHD is the automatic colour, which is not a shade at all — and the check has
            // to be on the whole word rather than on the background index, since index zero is itself
            // "automatic" and blending it as white would shade every unshaded cell.
            shades[i] = packed == 0
                ? null
                : Shade(Ico(packed & 0x1F), Ico((packed >> 5) & 0x1F), (packed >> 10) & 0x3F);
        }

        return shades;

        static Colour? Ico(int index) => IcoPalette[index < IcoPalette.Length ? index : 0];
    }

    /// <summary>
    /// Reads one of the three <c>sprmTDefTableShd</c> sprms, merging it into what the others said.
    /// </summary>
    /// <param name="existing">What the sprms read so far said, or null when this is the first.</param>
    /// <param name="operand">Ten bytes per cell: a foreground, a background and a pattern.</param>
    /// <param name="firstCell">
    /// Which cell this sprm starts at. WW8 splits the array across three sprms at cells 0, 22 and 44,
    /// because a sprm's operand is length-prefixed with a single byte and 63 cells of ten bytes do not fit
    /// in one.
    /// </param>
    private static Colour?[]? ReadRgbShading(
        IReadOnlyList<Colour?>? existing, ReadOnlyMemory<byte> operand, int firstCell)
    {
        ReadOnlySpan<byte> bytes = operand.Span;
        int stated = bytes.Length / RgbShadingSize;
        if (stated == 0 || firstCell >= MaxTableColumns) return existing?.ToArray();

        int cells = Math.Max(existing?.Count ?? 0, Math.Min(firstCell + stated, MaxTableColumns));
        Colour?[] shades = new Colour?[cells];
        for (int i = 0; i < (existing?.Count ?? 0); i++) shades[i] = existing![i];

        for (int i = 0; i < stated && firstCell + i < cells; i++)
        {
            ReadOnlySpan<byte> entry = bytes.Slice(i * RgbShadingSize, RgbShadingSize);
            shades[firstCell + i] = Shade(
                ColourOf(entry[..4]),
                ColourOf(entry[4..8]),
                BinaryPrimitives.ReadUInt16LittleEndian(entry[8..]));
        }

        return shades;
    }

    /// <summary>
    /// A colour from a WW8 <c>COLORREF</c>, or null for the automatic colour.
    /// </summary>
    /// <remarks>
    /// Stored blue-first with a fourth byte that is a flag rather than an alpha: 0xFF there is Word's
    /// "automatic", written as <c>0xFF000000</c>. Reading the four bytes as ARGB gives a transparent black
    /// for every automatic colour and swaps red for blue in every other.
    /// </remarks>
    private static Colour? ColourOf(ReadOnlySpan<byte> colorref)
        => colorref.Length < 4 || colorref[3] == 0xFF
            ? null
            : new Colour(colorref[0], colorref[1], colorref[2]);

    /// <summary>
    /// The colour a shaded cell ends up, from a foreground, a background and a fill pattern.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A WW8 shade is a <em>pattern</em>, not a colour: the two colours are the pattern's ink and its
    /// paper, and what shows is the two mixed in the pattern's own proportion. So "clear" — which is what
    /// every ordinary shaded cell uses — shows the background alone, and only a solid pattern shows the
    /// foreground alone. Ported from <c>SwWW8Shade::SetShade</c>.
    /// </para>
    /// <para>
    /// Automatic means different things on the two sides, which is the part worth stating: an automatic
    /// foreground is black and an automatic background is white, because a pattern has to be drawn in
    /// something — but a clear pattern over an automatic background is <em>no shading</em> rather than a
    /// white one, and returning white there would paint over a coloured page.
    /// </para>
    /// </remarks>
    private static Colour? Shade(Colour? foreground, Colour? background, int pattern)
    {
        int mix = pattern >= 0 && pattern < ShadingPatterns.Length ? ShadingPatterns[pattern] : 0;
        if (mix == 0) return background;

        Colour ink = foreground ?? Colour.Black;
        Colour paper = background ?? Colour.White;

        return new Colour(Channel(ink.R, paper.R), Channel(ink.G, paper.G), Channel(ink.B, paper.B));

        byte Channel(byte from, byte to) => (byte)(((from * mix) + (to * (1000 - mix))) / 1000);
    }

    /// <summary>
    /// How much of the foreground each fill pattern shows, in thousandths.
    /// </summary>
    /// <remarks>
    /// Word's <c>ipat</c> values, copied from <c>SwWW8Shade::SetShade</c>'s <c>eMSGrayScale</c>. The first
    /// two are the ones that matter — 0 is clear and 1 is solid — and the rest are percentages, with the
    /// hatch patterns approximated as a third of the foreground because a flat fill is all this can express.
    /// </remarks>
    private static readonly int[] ShadingPatterns =
    [
        0, 1000,
        50, 100, 200, 250, 300, 400, 500, 600, 700, 750, 800, 900,
        333, 333, 333, 333, 333, 333, 333, 333, 333, 333, 333, 333,
        500, 500, 500, 500, 500, 500, 500, 500, 500,
        25, 75, 125, 150, 175, 225, 275, 325, 350, 375, 425, 450, 475,
        525, 550, 575, 625, 650, 675, 725, 775, 825, 850, 875, 925, 950, 975,
    ];
}

/// <summary>
/// One WW8 border code — a <c>BRC</c> — as the two things a border is: how thick and what colour.
/// </summary>
/// <remarks>
/// The type is kept because it is not only a style: three of Word's types change the width the border
/// actually occupies, so it cannot be discarded before <see cref="Width"/> is worked out.
/// </remarks>
/// <param name="Kind">The <c>brcType</c>: 1 single, 3 double, 10 triple, and two dozen more.</param>
/// <param name="EighthPoints">
/// The <c>dptLineWidth</c>, in <em>eighths of a point</em> — the one WW8 measurement that is neither twips
/// nor half-points, and the same unit OOXML's <c>w:sz</c> uses.
/// </param>
/// <param name="Colour">Its colour, or null for Word's automatic colour.</param>
public readonly record struct Ww8Border(int Kind, int EighthPoints, Colour? Colour)
{
    /// <summary>The <c>brcType</c> meaning the structure states nothing about this side.</summary>
    public const int Unset = 0;

    /// <summary>The <c>brcType</c> a nil BRC reads as: the side is stated, and stated to have no border.</summary>
    public const int Nil = 0xFF;

    /// <summary>
    /// How much space the border takes, which is not simply its stated width.
    /// </summary>
    /// <remarks>
    /// Eighths of a point become twips by multiplying by 20 and dividing by 8, so the commonest border of
    /// all — <c>dptLineWidth</c> 4 — is ten twips, half a point. Three types then override that, because
    /// Word draws more than one line for them and reserves the room: a triple line is five times its
    /// nominal width, except at the two smallest sizes where it is three and four and a half; a wave and a
    /// double wave add a fixed 45 and 90 twips. Ported from <c>WW8_BRCVer9::DetermineBorderProperties</c>.
    /// </remarks>
    public Length Width
    {
        get
        {
            if (Kind is Unset or Nil) return Length.Zero;

            int twips = EighthPoints * 20 / 8;

            return Length.FromTwips(Kind switch
            {
                10 => twips switch { 5 => 15, 10 => 45, _ => twips * 5 },
                20 => twips + 45,
                21 => twips + 90,
                _ => twips,
            });
        }
    }

    /// <summary>
    /// The border as the layout engine wants it.
    /// </summary>
    /// <remarks>
    /// Word's automatic colour becomes black rather than staying automatic, which is what LibreOffice does
    /// for a border and only for a border: <c>GetLineIndex</c>'s "no AUTO for borders as yet, so if AUTO,
    /// use BLACK".
    /// </remarks>
    public Layout.TableBorder Resolved => new(Width, Colour ?? Core.Graphics.Colour.Black);
}

/// <summary>The four <c>BRC</c>s a cell states, each null where it states nothing.</summary>
/// <remarks>
/// Nullable per side rather than a border of zero width, because the two differ in what happens next: a
/// side stated as nothing keeps it, while a side left unstated falls through to the table's own defaults.
/// The bits are WW8's own order — top, left, bottom, right — the same order the padding sprms use, and
/// neither the order the sides are usually written in nor OOXML's.
/// </remarks>
/// <param name="Top">Its top edge.</param>
/// <param name="Left">Its left edge.</param>
/// <param name="Bottom">Its bottom edge.</param>
/// <param name="Right">Its right edge.</param>
public readonly record struct Ww8CellBorders(
    Ww8Border? Top, Ww8Border? Left, Ww8Border? Bottom, Ww8Border? Right)
{
    /// <summary>The bit that selects the top.</summary>
    public const int TopSide = 1 << 0;

    /// <summary>The bit that selects the left.</summary>
    public const int LeftSide = 1 << 1;

    /// <summary>The bit that selects the bottom.</summary>
    public const int BottomSide = 1 << 2;

    /// <summary>The bit that selects the right.</summary>
    public const int RightSide = 1 << 3;

    /// <summary>This cell's borders with a BRC applied to whichever sides a mask selects.</summary>
    public Ww8CellBorders With(int sides, Ww8Border? border) => new(
        (sides & TopSide) != 0 ? border : Top,
        (sides & LeftSide) != 0 ? border : Left,
        (sides & BottomSide) != 0 ? border : Bottom,
        (sides & RightSide) != 0 ? border : Right);
}

/// <summary>
/// One <c>sprmTSetBrc</c>: a border, the sides it sets, and the cells it sets them on.
/// </summary>
/// <param name="FirstCell">The first cell it applies to.</param>
/// <param name="CellLimit">One past the last.</param>
/// <param name="Sides">Which sides it sets, as <see cref="Ww8CellBorders"/>'s bits.</param>
/// <param name="Border">The border, or null to clear those sides back to unstated.</param>
public readonly record struct Ww8CellBorderChange(
    int FirstCell, int CellLimit, int Sides, Ww8Border? Border)
{
    /// <summary>True when this change applies to the cell at an index.</summary>
    public bool Covers(int cell) => cell >= FirstCell && cell < CellLimit;
}

/// <summary>
/// A table's six default borders, from <c>sprmTTableBorders</c>.
/// </summary>
/// <remarks>
/// The outline's four sides plus the two insides, which is how Word states a whole table's grid in one
/// place. Only a side no cell states falls back here, and which of the six it falls back to depends on
/// where the cell sits in the table.
/// </remarks>
/// <param name="Top">The table's top edge.</param>
/// <param name="Left">Its left edge.</param>
/// <param name="Bottom">Its bottom edge.</param>
/// <param name="Right">Its right edge.</param>
/// <param name="InsideHorizontal">Every horizontal line between two rows.</param>
/// <param name="InsideVertical">Every vertical line between two cells.</param>
public sealed record Ww8TableBorders(
    Ww8Border? Top,
    Ww8Border? Left,
    Ww8Border? Bottom,
    Ww8Border? Right,
    Ww8Border? InsideHorizontal,
    Ww8Border? InsideVertical)
{
    /// <summary>
    /// The default for one side of a cell at a given place in the table.
    /// </summary>
    /// <remarks>
    /// A cell's top is the table's top only in the first row and an inside line everywhere else, and the
    /// same for the other three — which is the whole point of there being six. Ported from the third pass
    /// of <c>WW8TabDesc::CalcDefaults</c>.
    /// </remarks>
    /// <param name="side">One of <see cref="Ww8CellBorders"/>'s side bits.</param>
    /// <param name="isFirstRow">True when the cell is in the table's first row.</param>
    /// <param name="isLastRow">True when it is in the last.</param>
    /// <param name="isFirstCell">True when it is the first cell of its row.</param>
    /// <param name="isLastCell">True when it is the last.</param>
    public Ww8Border? For(
        int side, bool isFirstRow, bool isLastRow, bool isFirstCell, bool isLastCell) => side switch
        {
            Ww8CellBorders.TopSide => isFirstRow ? Top : InsideHorizontal,
            Ww8CellBorders.LeftSide => isFirstCell ? Left : InsideVertical,
            Ww8CellBorders.BottomSide => isLastRow ? Bottom : InsideHorizontal,
            Ww8CellBorders.RightSide => isLastCell ? Right : InsideVertical,
            _ => null,
        };
}
