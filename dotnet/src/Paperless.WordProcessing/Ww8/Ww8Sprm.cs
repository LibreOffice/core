using System.Buffers.Binary;

namespace Paperless.WordProcessing.Ww8;

/// <summary>One sprm: a tagged, variably sized formatting instruction.</summary>
/// <param name="Identifier">
/// The sprm's 16-bit id, whose own bits encode which property it sets and how big its operand is.
/// </param>
/// <param name="Operand">The operand bytes, excluding any length prefix.</param>
public readonly record struct Ww8Sprm(ushort Identifier, ReadOnlyMemory<byte> Operand)
{
    /// <summary>The operand read as one byte, or zero when it is empty.</summary>
    public byte Byte => Operand.Length > 0 ? Operand.Span[0] : (byte)0;

    /// <summary>The operand read as a signed 16-bit value.</summary>
    public short SignedWord => Operand.Length >= 2
        ? BinaryPrimitives.ReadInt16LittleEndian(Operand.Span)
        : Byte;

    /// <summary>The operand read as an unsigned 16-bit value.</summary>
    public ushort Word => Operand.Length >= 2
        ? BinaryPrimitives.ReadUInt16LittleEndian(Operand.Span)
        : Byte;

    /// <summary>The operand read as a signed 32-bit value.</summary>
    public int DoubleWord => Operand.Length >= 4
        ? BinaryPrimitives.ReadInt32LittleEndian(Operand.Span)
        : SignedWord;

    /// <summary>
    /// A toggle operand resolved against what the style already says.
    /// </summary>
    /// <remarks>
    /// WW8 spells a toggle in four states, not two: 0 off, 1 on, 128 "leave as the style has it",
    /// 129 "invert what the style has". The last two are how a document says "bold here toggles
    /// whatever the paragraph style did", which is the same idea as OOXML's toggle properties and
    /// wrong in the same way if read as a boolean.
    /// </remarks>
    public bool ResolveToggle(bool fromStyle) => Byte switch
    {
        0 => false,
        1 => true,
        128 => fromStyle,
        129 => !fromStyle,
        _ => Byte != 0,
    };
}

/// <summary>
/// Walks a grpprl: a run of sprms packed one after another with no count or terminator.
/// </summary>
/// <remarks>
/// <para>
/// A grpprl can only be read forwards, because a sprm's length comes from its id and the next one
/// begins where the last ended. The id's high bits give the operand size for every case but one:
/// size code 6 means the operand carries its own length byte — and <c>sprmTDefTable</c> is the
/// exception to the exception, using two bytes. Getting any of this wrong desynchronises the walk
/// and produces a stream of nonsense sprms rather than an error.
/// </para>
/// <para>
/// The same walker serves direct formatting, style definitions and list levels, because WW8 stores
/// all three as grpprls (<c>research/02-writer.md</c> section C.2, points 4 and 5).
/// </para>
/// </remarks>
public static class Ww8SprmReader
{
    /// <summary>The sprm ids Paperless acts on.</summary>
    /// <remarks>
    /// Named rather than written as numbers at the call sites: the ids are dense and adjacent, so a
    /// mistyped digit silently reads a different property — <c>0x0836</c> is italic and
    /// <c>0x0837</c> is strike-through.
    /// </remarks>
    public static class Ids
    {
        /// <summary>The paragraph's style index.</summary>
        public const ushort ParagraphStyle = 0x4600;

        /// <summary>The paragraph's list level.</summary>
        public const ushort ListLevel = 0x260A;

        /// <summary>The paragraph's list format override, which names its list.</summary>
        public const ushort ListFormatOverride = 0x460B;

        /// <summary>The paragraph is inside a table.</summary>
        public const ushort InTable = 0x2416;

        /// <summary>The paragraph mark ends a table row.</summary>
        public const ushort IsTableRowEnd = 0x2417;

        /// <summary>
        /// The paragraph's nesting depth in tables: 1 for a top-level table, 2 for one inside a cell.
        /// </summary>
        public const ushort TableDepth = 0x6649;

        /// <summary>The paragraph's mark ends a cell of the <em>inner</em> table.</summary>
        /// <remarks>
        /// A nested table's cell marks are indistinguishable from the enclosing table's without this:
        /// both are U+0007 inside the same run of text, so a reader that ignores it turns one nested
        /// table into a handful of extra cells in the row that contains it.
        /// </remarks>
        public const ushort IsInnerTableCell = 0x244B;

        /// <summary>The paragraph's mark ends a row of the inner table.</summary>
        public const ushort IsInnerTableRowEnd = 0x244C;

        /// <summary>The row this paragraph ends repeats as a header on every page.</summary>
        public const ushort IsTableHeaderRow = 0x3404;

        /// <summary>
        /// The row's geometry: its column edges and its cells' merge flags.
        /// </summary>
        /// <remarks>
        /// The one sprm whose operand length is two bytes rather than one, because a table definition
        /// can exceed 255 bytes — see <see cref="Read"/>.
        /// </remarks>
        public const ushort TableDefinition = 0xD608;

        /// <summary>The paragraph's outline level, which is what makes it a heading.</summary>
        public const ushort OutlineLevel = 0x2640;

        /// <summary>The run's character style index.</summary>
        public const ushort CharacterStyle = 0x4A30;

        /// <summary>Bold.</summary>
        public const ushort Bold = 0x0835;

        /// <summary>Italic.</summary>
        public const ushort Italic = 0x0836;

        /// <summary>Single strike-through.</summary>
        public const ushort Strike = 0x0837;

        /// <summary>Small capitals.</summary>
        public const ushort SmallCaps = 0x083A;

        /// <summary>All capitals.</summary>
        public const ushort Caps = 0x083B;

        /// <summary>Hidden text.</summary>
        public const ushort Vanish = 0x083C;

        /// <summary>Double strike-through.</summary>
        public const ushort DoubleStrike = 0x2A53;

        /// <summary>The underline style, where zero is none.</summary>
        public const ushort Underline = 0x2A3E;

        /// <summary>Superscript or subscript.</summary>
        public const ushort VerticalPosition = 0x2A48;

        /// <summary>Font size in half-points.</summary>
        public const ushort FontSize = 0x4A43;

        /// <summary>The run's language, for Latin script.</summary>
        public const ushort Language = 0x486D;

        /// <summary>The run is a footnote or endnote reference mark.</summary>
        public const ushort IsSpecial = 0x0855;

        /// <summary>
        /// The run is marked deleted by a tracked change.
        /// </summary>
        /// <remarks>
        /// Deleted text is still in the file — that is what makes the change reversible — so emitting
        /// it invents content the document does not say. The insertion flag needs no handling for the
        /// same reason: inserted text <em>is</em> content.
        /// </remarks>
        public const ushort IsDeleted = 0x0800;
    }

    /// <summary>
    /// Reads the sprms in a grpprl, in order.
    /// </summary>
    /// <remarks>
    /// Stops rather than resynchronising when a sprm's length runs past the end of the buffer: once
    /// the walk has desynchronised, everything after it is noise, and emitting it would be worse
    /// than stopping short.
    /// </remarks>
    public static IEnumerable<Ww8Sprm> Read(ReadOnlyMemory<byte> grpprl)
    {
        int position = 0;
        while (position + 2 <= grpprl.Length)
        {
            ushort id = BinaryPrimitives.ReadUInt16LittleEndian(grpprl.Span[position..]);
            position += 2;

            int operandLength = OperandLength(id, grpprl, position, out int prefixLength);
            position += prefixLength;

            if (operandLength < 0 || position + operandLength > grpprl.Length) break;

            yield return new Ww8Sprm(id, grpprl.Slice(position, operandLength));
            position += operandLength;
        }
    }

    /// <summary>
    /// How many operand bytes a sprm has, from the size code in its id.
    /// </summary>
    /// <param name="id">The sprm's identifier, whose high bits carry the size code.</param>
    /// <param name="grpprl">The buffer being walked, needed to read a self-describing length.</param>
    /// <param name="position">Where the operand — or its length prefix — begins.</param>
    /// <param name="prefixLength">
    /// How many bytes of length prefix precede the operand, for the variable-length codes.
    /// </param>
    private static int OperandLength(
        ushort id, ReadOnlyMemory<byte> grpprl, int position, out int prefixLength)
    {
        prefixLength = 0;

        // Bits 13-15 of the id are the operand's size code.
        int sizeCode = (id >> 13) & 0x07;
        switch (sizeCode)
        {
            case 0 or 1:
                return 1;
            case 2 or 4 or 5:
                return 2;
            case 3:
                return 4;
            case 7:
                return 3;
            default:
                // Code 6: the operand states its own length. sprmTDefTable is the one sprm whose
                // length is two bytes rather than one, because a table definition can exceed 255
                // bytes — and a reader that assumes one byte desynchronises on every table.
                //
                // The two-byte field is the operand's length *plus one*, not plus two: it counts
                // itself as one byte rather than as the two it occupies. Subtracting two loses the
                // last byte of the operand and leaves the walk one byte out of step, so every sprm
                // after a table definition decodes as something else — which for a table means the
                // repeat-header flag and the shading that follow it. LibreOffice notes the same
                // quirk at <c>ww8scan.cxx</c>'s <c>L_VAR2</c>.
                if (id == Ids.TableDefinition)
                {
                    if (position + 2 > grpprl.Length) return -1;
                    prefixLength = 2;
                    return BinaryPrimitives.ReadUInt16LittleEndian(grpprl.Span[position..]) - 1;
                }

                if (position + 1 > grpprl.Length) return -1;
                prefixLength = 1;
                return grpprl.Span[position];
        }
    }
}
