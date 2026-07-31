using System.Buffers.Binary;
using Paperless.Core.Diagnostics;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// What a shape's position is measured from, as an <c>FSPA</c> and Word's own
/// <c>posrelh</c>/<c>posrelv</c> shape properties both number it.
/// </summary>
/// <remarks>
/// One enumeration for two fields because the two really do share a numbering — which is the only
/// reason <c>ProcessEscherAlign</c> can fall back from the property to the <c>FSPA</c> without
/// translating (<c>sw/source/filter/ww8/ww8graf.cxx:2316</c>). The horizontal and vertical
/// meanings of 2 and 3 differ, which is why the names are the neutral ones.
/// </remarks>
public enum Ww8ShapeOrigin
{
    /// <summary>The page's printable area — inside its margins.</summary>
    PageMargin = 0,

    /// <summary>The page itself, margins included.</summary>
    Page = 1,

    /// <summary>The text: the column horizontally, the anchor paragraph vertically.</summary>
    Text = 2,

    /// <summary>The anchoring character horizontally, its line vertically.</summary>
    Character = 3,
}

/// <summary>
/// One <c>FSPA</c>: the record that ties a character position in the text to a shape in the
/// drawing, and states everything about the shape that the text cares about.
/// </summary>
/// <remarks>
/// <para>
/// Twenty-six bytes — five signed 32-bit fields, a bitfield word, and a count
/// (<c>WW8_FSPA_SHADOW</c>, <c>sw/source/filter/ww8/ww8struc.hxx:953</c>, with a
/// <c>static_assert</c> on the size beside it because the record's length is what a
/// <c>PlcSpa</c>'s entry count is derived from). The rectangle is in twips relative to whatever
/// <see cref="HorizontalOrigin"/> and <see cref="VerticalOrigin"/> name.
/// </para>
/// <para>
/// The bitfield's shifts are worth taking from <c>WW8FSPAShadowToReal</c>
/// (<c>ww8graf2.cxx:760</c>) rather than counting them out: <c>nwr</c> is four bits at 5 and
/// <c>nwrk</c> four at 9, so the two wrap fields are adjacent but neither is byte-aligned.
/// </para>
/// </remarks>
/// <param name="Position">The character position the shape is anchored at.</param>
/// <param name="ShapeId">The shape's identifier, matching an <c>msofbtSp</c> in the drawing.</param>
/// <param name="Left">The rectangle's left edge in twips, relative to the horizontal origin.</param>
/// <param name="Top">Its top edge in twips, relative to the vertical origin.</param>
/// <param name="Right">Its right edge in twips.</param>
/// <param name="Bottom">Its bottom edge in twips.</param>
/// <param name="IsHeaderAnchor">True when the anchor is in the header subdocument.</param>
/// <param name="HorizontalOrigin">What <paramref name="Left"/> is measured from.</param>
/// <param name="VerticalOrigin">What <paramref name="Top"/> is measured from.</param>
/// <param name="Wrap">The wrapping mode, <c>nwr</c>; see <see cref="Ww8Drawings"/> for the values.</param>
/// <param name="WrapSide">Which side of the shape the text may use, <c>nwrk</c>.</param>
/// <param name="IsPageRelative">
/// <c>bRcaSimple</c>: the rectangle is page-relative whatever the two origins say.
/// </param>
/// <param name="IsBelowText">True when the shape is painted behind the text rather than over it.</param>
public readonly record struct Ww8ShapeAnchor(
    int Position,
    int ShapeId,
    int Left,
    int Top,
    int Right,
    int Bottom,
    bool IsHeaderAnchor,
    Ww8ShapeOrigin HorizontalOrigin,
    Ww8ShapeOrigin VerticalOrigin,
    int Wrap,
    int WrapSide,
    bool IsPageRelative,
    bool IsBelowText)
{
    /// <summary>How wide the shape is, in twips.</summary>
    public int Width => Right - Left;

    /// <summary>How tall the shape is, in twips.</summary>
    public int Height => Bottom - Top;
}

/// <summary>
/// A DOC's drawings: the <c>FSPA</c> table that maps character positions to shapes, and the
/// Escher drawing the shapes themselves live in.
/// </summary>
/// <remarks>
/// <para>
/// Two structures in two places, and neither is usable without the other. The
/// <c>PlcSpaMom</c>/<c>PlcSpaHdr</c> tables say <em>where in the text</em> each shape is anchored,
/// how big it is and how text behaves round it, but nothing about what the shape is. The Escher
/// blob at <c>fcDggInfo</c> says what each shape is — a text box, a picture, a group — but nothing
/// about where in the text it belongs. The shape identifier is the join.
/// </para>
/// <para>
/// The Escher blob's layout is the one thing here that cannot be walked as plain sibling records.
/// It is an <c>OfficeArtDggContainer</c> followed by one <em>Word drawing</em> per subdocument, and
/// a Word drawing is a <strong>single label byte</strong> — 0 for the body, 1 for the headers —
/// and then an <c>OfficeArtDgContainer</c>. That odd byte is why LibreOffice's own reader advances
/// one byte and re-reads the header when the first attempt is not a <c>DgContainer</c>
/// (<c>filter/source/msfilter/msdffimp.cxx:5997</c>, whose comment calls it "trying to get a
/// one-hit wonder"). Walking the blob as records without allowing for it reads the label plus the
/// first three bytes of the container's header as a record type and loses every shape in the
/// document.
/// </para>
/// <para>
/// Shapes are indexed by identifier across <em>all</em> the drawings rather than per drawing,
/// because that is how the <c>FSPA</c> reaches them: the record carries a shape id and no drawing
/// number, and the ids are unique document-wide by construction — the <c>msofbtDgg</c> record's
/// job is to hand out non-overlapping id clusters.
/// </para>
/// </remarks>
public sealed class Ww8Drawings
{
    /// <summary>The size of one <c>FSPA</c> record, which is what a <c>PlcSpa</c>'s count divides by.</summary>
    public const int AnchorRecordSize = 26;

    /// <summary>The label byte that precedes each drawing in the Word Art blob.</summary>
    /// <remarks>
    /// Not read for its value: the anchor's own <c>bHdr</c> bit already says which subdocument a
    /// shape belongs to, and the shape ids are document-wide, so the label is only ever skipped.
    /// </remarks>
    private const int DrawingLabelSize = 1;

    private readonly Dictionary<int, Ww8ShapeAnchor> _anchors;
    private readonly Dictionary<uint, EscherShape> _shapes;

    private Ww8Drawings(Dictionary<int, Ww8ShapeAnchor> anchors, Dictionary<uint, EscherShape> shapes)
    {
        _anchors = anchors;
        _shapes = shapes;
    }

    /// <summary>A document with no drawings at all, which is most of them.</summary>
    public static Ww8Drawings Empty { get; } = new([], []);

    /// <summary>True when the document declares no shapes.</summary>
    public bool IsEmpty => _anchors.Count == 0 && _shapes.Count == 0;

    /// <summary>How many anchors the document holds, across both subdocuments.</summary>
    public int AnchorCount => _anchors.Count;

    /// <summary>How many shapes the drawing holds, groups and their members counted separately.</summary>
    public int ShapeCount => _shapes.Count;

    /// <summary>
    /// Reads a document's drawings.
    /// </summary>
    /// <param name="bodyAnchors">The <c>PlcSpaMom</c> table's bytes.</param>
    /// <param name="headerAnchors">The <c>PlcSpaHdr</c> table's bytes.</param>
    /// <param name="officeArt">The bytes at <c>fcDggInfo</c>, holding the whole drawing.</param>
    /// <param name="headerRangeStart">
    /// Where the header subdocument begins in the document's position space. A header anchor's
    /// <c>FSPA</c> position is relative to that subdocument rather than to the document, so without
    /// rebasing every header shape would appear to be anchored inside the body's first few pages.
    /// </param>
    /// <param name="diagnostics">Where to record what could not be read.</param>
    public static Ww8Drawings Read(
        ReadOnlySpan<byte> bodyAnchors,
        ReadOnlySpan<byte> headerAnchors,
        byte[] officeArt,
        int headerRangeStart,
        List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(officeArt);
        ArgumentNullException.ThrowIfNull(diagnostics);

        Dictionary<int, Ww8ShapeAnchor> anchors = [];
        ReadAnchors(bodyAnchors, offset: 0, isHeader: false, anchors);
        ReadAnchors(headerAnchors, headerRangeStart, isHeader: true, anchors);

        Dictionary<uint, EscherShape> shapes = ReadShapes(officeArt, diagnostics);

        return anchors.Count == 0 && shapes.Count == 0
            ? Empty
            : new Ww8Drawings(anchors, shapes);
    }

    /// <summary>The anchor at a character position, or null when nothing is anchored there.</summary>
    public Ww8ShapeAnchor? AnchorAt(int position)
        => _anchors.TryGetValue(position, out Ww8ShapeAnchor anchor) ? anchor : null;

    /// <summary>The shape an anchor names, or null when the drawing does not hold it.</summary>
    /// <remarks>
    /// A missing shape is not a malformed document: a shape can be deleted from the drawing while
    /// its anchor record is left behind, and Word itself writes files in that state. The caller
    /// reports the anchor as a shape it knows nothing about rather than as a picture.
    /// </remarks>
    public EscherShape? Shape(int shapeId)
        => _shapes.TryGetValue(unchecked((uint)shapeId), out EscherShape? shape) ? shape : null;

    /// <summary>The shape anchored at a character position, or null.</summary>
    public EscherShape? ShapeAt(int position)
        => AnchorAt(position) is { } anchor ? Shape(anchor.ShapeId) : null;

    /// <summary>
    /// Reads one <c>PlcSpa</c> into the anchor index.
    /// </summary>
    /// <remarks>
    /// The PLCF's own machinery is deliberately not used for the positions: an <c>FSPA</c> already
    /// carries the position it applies at as part of the partition, and what a reader wants is a
    /// lookup by position rather than by index. The record's own <c>nSpId</c> being zero is Word's
    /// way of writing an anchor whose shape is gone; LibreOffice bails on the same test
    /// (<c>ww8graf.cxx:2648</c>).
    /// </remarks>
    private static void ReadAnchors(
        ReadOnlySpan<byte> table, int offset, bool isHeader, Dictionary<int, Ww8ShapeAnchor> into)
    {
        if (table.Length < 4 + AnchorRecordSize) return;

        int count = (table.Length - 4) / (4 + AnchorRecordSize);
        int dataStart = 4 * (count + 1);

        for (int i = 0; i < count; i++)
        {
            int at = dataStart + (i * AnchorRecordSize);
            if (at + AnchorRecordSize > table.Length) break;

            ReadOnlySpan<byte> record = table[at..];
            int shapeId = BinaryPrimitives.ReadInt32LittleEndian(record);
            if (shapeId == 0) continue;

            ushort bits = BinaryPrimitives.ReadUInt16LittleEndian(record[20..]);
            int position = offset + BinaryPrimitives.ReadInt32LittleEndian(table[(4 * i)..]);

            into[position] = new Ww8ShapeAnchor(
                position,
                shapeId,
                BinaryPrimitives.ReadInt32LittleEndian(record[4..]),
                BinaryPrimitives.ReadInt32LittleEndian(record[8..]),
                BinaryPrimitives.ReadInt32LittleEndian(record[12..]),
                BinaryPrimitives.ReadInt32LittleEndian(record[16..]),
                isHeader,
                (Ww8ShapeOrigin)((bits >> 1) & 0x3),
                (Ww8ShapeOrigin)((bits >> 3) & 0x3),
                (bits >> 5) & 0xF,
                (bits >> 9) & 0xF,
                (bits & 0x2000) != 0,
                (bits & 0x4000) != 0);
        }
    }

    /// <summary>
    /// Reads every shape in the Office Art blob, indexed by shape identifier.
    /// </summary>
    /// <remarks>
    /// Groups are flattened, because an <c>FSPA</c> can name a shape inside a group as readily as a
    /// top-level one — a grouped picture with its own caption is one anchor per member — and the
    /// index is what the anchor is resolved through.
    /// </remarks>
    private static Dictionary<uint, EscherShape> ReadShapes(
        byte[] officeArt, List<Diagnostic> diagnostics)
    {
        Dictionary<uint, EscherShape> shapes = [];
        if (officeArt.Length < DffRecordHeader.HeaderSize) return shapes;

        DffRecordBuffer buffer = new(officeArt);
        EscherDrawingReader reader = new(buffer, diagnostics);

        if (!buffer.TryReadHeader(0, out DffRecordHeader header)
            || header.Type != EscherRecordTypes.DrawingGroupContainer)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2360",
                "The document's drawing data does not begin with a drawing group container, so its "
                + "shapes were not read; anchors in the text will be reported without a shape."));
            return shapes;
        }

        int position = buffer.EndOf(header);

        while (position + DffRecordHeader.HeaderSize <= officeArt.Length)
        {
            if (!buffer.TryReadHeader(position, out DffRecordHeader drawing)) break;

            // The label byte. Skipping it unconditionally would be wrong for a writer that omits
            // it, so it is skipped only when the header does not already parse as a drawing.
            if (drawing.Type != EscherRecordTypes.DrawingContainer)
            {
                position += DrawingLabelSize;
                if (!buffer.TryReadHeader(position, out drawing)
                    || drawing.Type != EscherRecordTypes.DrawingContainer)
                {
                    break;
                }
            }

            foreach (EscherShape shape in reader.ReadDrawing(drawing))
            {
                foreach (EscherShape member in shape.SelfAndDescendants())
                {
                    if (member.ShapeId != 0) shapes.TryAdd(member.ShapeId, member);
                }
            }

            int next = buffer.EndOf(drawing);
            if (next <= position) break;
            position = next;
        }

        return shapes;
    }
}
