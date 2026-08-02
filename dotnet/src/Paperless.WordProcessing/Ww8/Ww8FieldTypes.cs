using Paperless.Core.Diagnostics;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// The type each field in a story is, indexed by the character position of its <c>U+0013</c>.
/// </summary>
/// <remarks>
/// <para>
/// WW8 puts a field's markers in the text — <c>U+0013</c> begin, <c>U+0014</c> separator,
/// <c>U+0015</c> end — and its <em>type</em> nowhere near them: the type is a byte in the
/// <c>PlcFld</c> beside the marker's position. So a walk over the text alone can see that it is
/// inside a field and never see which field, which is the whole reason this table is read.
/// </para>
/// <para>
/// <strong>Why a walk needs the type at all.</strong> One field changes how a shape is anchored.
/// LibreOffice's <c>SwWW8ImplReader::IsInlineEscherHack</c> (<c>ww8par.hxx:1737</c>) is exactly
/// <c>the innermost open field is a SHAPE</c>, and <c>ProcessEscherAlign</c> then anchors the shape
/// <c>FLY_AS_CHAR</c> rather than <c>FLY_AT_CHAR</c> (<c>ww8graf.cxx:2355</c>). That is how Word
/// writes a picture that sits in the run of text: an <c>FSPA</c> like any floating shape, wrapped in
/// a <c>SHAPE</c> field that means "and it is not floating after all". Without the type a reader has
/// an <c>FSPA</c> and no reason to distrust it, so every such picture comes back floating.
/// </para>
/// <para>
/// A plain <see cref="Ww8Plcf"/> of two-byte records: <c>FLD.ch</c> in the low five bits of the first
/// byte says which of the three markers this is, and for a begin the second byte is the type. The
/// separator and the end restate a byte that is not a type — <c>0xFF</c> and a flags byte — so only a
/// begin is recorded, and the two others are recognised by their <c>ch</c> alone.
/// </para>
/// </remarks>
public sealed class Ww8FieldTypes
{
    /// <summary>The <c>SHAPE</c> field, whose shapes are set in the line rather than floating.</summary>
    /// <remarks>
    /// 95, the number <c>IsInlineEscherHack</c> compares against literally. Named here because the
    /// comparison reads as a magic number wherever it is made, and because LibreOffice's own field
    /// table (<c>ww8par5.cxx</c>, <c>aWW8FieldTab</c>) has no entry for it at all — it is handled by
    /// this side effect and not as a field.
    /// </remarks>
    public const int Shape = 95;

    /// <summary>The <c>FILENAME</c> field, which names the file the document was read from.</summary>
    /// <remarks>
    /// 29, <c>Read_F_FileName</c>'s slot in <c>aWW8FieldTab</c> (<c>ww8par5.cxx</c>:874). One of the
    /// fields whose cached result must not be believed: it is whatever the document was called when it
    /// was last saved, so renaming the file makes every copy of it wrong at once, and LibreOffice
    /// re-evaluates it on load rather than drawing the cache.
    /// </remarks>
    public const int FileName = 29;

    /// <summary>A field's beginning, as <c>FLD.ch</c> states it.</summary>
    private const byte BeginMarker = 19;

    /// <summary>A field's end, as <c>FLD.ch</c> states it.</summary>
    private const byte EndMarker = 21;

    private readonly Dictionary<int, int> _beginnings;

    private Ww8FieldTypes(Dictionary<int, int> beginnings) => _beginnings = beginnings;

    /// <summary>A story that declares no fields.</summary>
    public static Ww8FieldTypes Empty { get; } = new([]);

    /// <summary>How many field beginnings the table describes.</summary>
    public int Count => _beginnings.Count;

    /// <summary>
    /// Reads a <c>PlcFld</c>.
    /// </summary>
    /// <param name="bytes">The table's bytes, or empty for a story with no fields.</param>
    /// <param name="diagnostics">Where a malformed table is reported; nothing is thrown.</param>
    public static Ww8FieldTypes Parse(ReadOnlySpan<byte> bytes, IList<Diagnostic>? diagnostics = null)
    {
        if (bytes.Length < 4 + 4 + 2) return Empty;

        Ww8Plcf plcf = Ww8Plcf.Parse(bytes, recordSize: 2);
        if (plcf.Count == 0) return Empty;

        Dictionary<int, int> beginnings = new(plcf.Count);
        int unbalanced = 0;
        int depth = 0;

        for (int index = 0; index < plcf.Count; index++)
        {
            ReadOnlySpan<byte> record = plcf.Record(index);
            if (record.Length < 2) continue;

            byte marker = (byte)(record[0] & 0x1F);
            if (marker == BeginMarker)
            {
                beginnings[plcf.Positions[index]] = record[1];
                depth++;
            }
            else if (marker == EndMarker)
            {
                if (depth == 0) unbalanced++;
                else depth--;
            }
        }

        if (unbalanced > 0 || depth > 0)
        {
            // Reported and not repaired. A walk keeps its own stack and pops only what it pushed, so
            // an unbalanced table costs a shape its anchor and never an exception — and a document
            // whose field markers and field table disagree is one Word itself renders.
            diagnostics?.Add(new Diagnostic(
                DiagnosticSeverity.Information, "PL2378",
                $"A field table has {unbalanced} unmatched end marker(s) and {depth} unclosed "
                + "field(s), so a shape inside one may be anchored as though it were outside."));
        }

        return new Ww8FieldTypes(beginnings);
    }

    /// <summary>
    /// The type of the field that begins at a character position, or null when none begins there.
    /// </summary>
    /// <remarks>
    /// Keyed by the beginning's own position rather than by a range, because the walk that asks is
    /// already at the <c>U+0013</c> and keeps the stack itself. A range would have to be built by the
    /// same pairing the walk does, and would then disagree with it on a document whose markers and
    /// table are not balanced the same way.
    /// </remarks>
    public int? At(int position)
        => _beginnings.TryGetValue(position, out int type) ? type : null;
}
