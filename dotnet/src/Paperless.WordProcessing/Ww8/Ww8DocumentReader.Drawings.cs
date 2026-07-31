using System.Buffers.Binary;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;

namespace Paperless.WordProcessing.Ww8;

/// <content>
/// Telling an embedded picture from a drawn shape, which is a question about the record streams
/// rather than about the text.
/// </content>
/// <remarks>
/// <para>
/// WW8 marks both with a special character and gives neither a name: U+0001 for a picture or an
/// embedded object and U+0008 for a drawing. Neither character says what it stands for, and the
/// obvious reading — every U+0001 is a picture — is wrong in the common case, because Word writes an
/// <em>inline</em> shape as a U+0001 too. So a document with one text box in it reports a picture
/// that does not exist, which is precisely what a caller indexing a corpus for images would act on.
/// </para>
/// <para>
/// The two answers come from two different places, and that is the whole of the difficulty. A
/// floating shape is resolved through the <c>FSPA</c> at its position and then through the Escher
/// drawing; an inline one is resolved through the <c>PICF</c> at <c>sprmCPicLocation</c>, whose
/// mapping mode is the flag — <c>0x64</c> and <c>0x66</c> mean "the bytes after this header are an
/// Escher shape, not a metafile" (<c>SwWW8ImplReader::ImportGraf</c>,
/// <c>sw/source/filter/ww8/ww8graf2.cxx:524</c>).
/// </para>
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>
    /// The <c>PICF</c> mapping modes that mean the "picture" is an Escher shape stored inline.
    /// </summary>
    /// <remarks>
    /// The two differ only in that <c>0x66</c> prefixes the shape with a length-prefixed name, which
    /// matters to a reader decoding the shape and not to one merely classifying it.
    /// </remarks>
    private const short InlineShapeMappingMode = 0x64;

    /// <inheritdoc cref="InlineShapeMappingMode"/>
    private const short NamedInlineShapeMappingMode = 0x66;

    /// <summary>The smallest <c>PICF</c> LibreOffice will treat as one; below it the bytes are noise.</summary>
    /// <remarks>
    /// <c>ww8graf2.cxx:498</c> checks <c>aPic.lcb &gt;= 58</c>, and the reason is stated beside it:
    /// a check box in a field's result contains a structure that looks enough like a <c>PICF</c> to
    /// be read as one.
    /// </remarks>
    private const int MinimumPictureLength = 58;

    /// <summary>
    /// Whether the U+0001 at a position stands for an embedded picture or object.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The <c>PICF</c>'s mapping mode is only the first half of the answer, and stopping there is the
    /// trap: <c>0x64</c> says the bytes after the header are an Escher shape rather than a metafile,
    /// but that shape is very often a picture. Every inline picture LibreOffice's own DOC export
    /// writes is one — a shape of type <c>mso_sptPictureFrame</c> with a <c>pib</c> property indexing
    /// the blip store — so a reader that treats <c>0x64</c> as "a drawing, not a picture" reports no
    /// images at all for a document full of them. Only the shape can decide.
    /// </para>
    /// <para>
    /// A position whose run states no picture location, or whose <c>PICF</c> is too short to be one,
    /// is reported as a picture. That is what an OLE object placeholder looks like, and the failure
    /// worth avoiding here is the one that loses content.
    /// </para>
    /// </remarks>
    private bool IsEmbeddedPicture(int position)
    {
        if (PictureLocation(position) is not { } offset) return true;
        if (offset < 0 || offset > _pictures.Length - DffRecordHeader.HeaderSize) return true;

        ReadOnlySpan<byte> picture = _pictures.AsSpan(offset);
        int length = BinaryPrimitives.ReadInt32LittleEndian(picture);
        if (length < MinimumPictureLength) return true;

        int header = BinaryPrimitives.ReadUInt16LittleEndian(picture[4..]);
        short mappingMode = BinaryPrimitives.ReadInt16LittleEndian(picture[6..]);
        if (mappingMode is not (InlineShapeMappingMode or NamedInlineShapeMappingMode)) return true;

        // 0x66 prefixes the shape with a length-prefixed name; 0x64 does not.
        int at = offset + header;
        if (mappingMode == NamedInlineShapeMappingMode && at < _pictures.Length)
        {
            at += 1 + _pictures[at];
        }

        return IsPictureShape(InlineShape(at));
    }

    /// <summary>
    /// The Escher shape stored inline in the picture stream at an offset, or null when none is.
    /// </summary>
    /// <remarks>
    /// A lone <c>SpContainer</c> with no drawing or group around it, which is how Word stores a shape
    /// that sits in the text rather than floating. Read through the shared reader rather than by
    /// hand, so that an inline picture and a floating one are described by the same type.
    /// </remarks>
    private EscherShape? InlineShape(int offset)
    {
        _inlinePictures ??= new DffRecordBuffer(_pictures);

        if (!_inlinePictures.TryReadHeader(offset, out DffRecordHeader header)) return null;
        if (header.Type != EscherRecordTypes.ShapeContainer) return null;

        return new EscherDrawingReader(_inlinePictures, _diagnostics).ReadShape(header);
    }

    private DffRecordBuffer? _inlinePictures;

    /// <summary>Whether a shape is a picture rather than something drawn.</summary>
    /// <remarks>
    /// <para>
    /// By the <c>pib</c> property, which names a blip in the picture store, and deliberately
    /// <em>not</em> by the shape's type. A shape of type <c>mso_sptPictureFrame</c> naming no blip is
    /// an empty placeholder, and LibreOffice's own DOC export writes exactly one of those beside
    /// every text box: it exports a text box as a <c>SHAPE</c> field whose cached result is a U+0008
    /// for the shape and then a U+0001 for a picture frame with no <c>OPT</c> record at all. Trusting
    /// the type therefore reports one phantom image per text box — the very defect this file exists
    /// to fix, arriving by a second route. LibreOffice's own round trip of <c>word-features.doc</c>
    /// agrees: one <c>draw:frame</c> and no <c>draw:image</c>.
    /// </para>
    /// <para>
    /// A shape of some other type carrying a <c>pib</c> is a picture too — that is how a picture used
    /// as an autoshape's fill is written — which is why the type is not tested at all.
    /// </para>
    /// </remarks>
    private static bool IsPictureShape(EscherShape? shape)
        => shape is not null && shape.Properties.Value(EscherPropertyIds.Picture) != 0;

    /// <summary>
    /// The <c>sprmCPicLocation</c> in force at a position, or null when the run states none.
    /// </summary>
    /// <remarks>
    /// Resolved from the character exception alone rather than through the style chain, because a
    /// picture's location is direct formatting by construction — it names a byte offset in this
    /// document and could not be inherited from a style that other runs share.
    /// </remarks>
    private int? PictureLocation(int position)
    {
        ReadOnlyMemory<byte> exception = _characterProperties.Find(_pieces.FileOffsetOf(position));
        if (exception.IsEmpty) return null;

        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(exception))
        {
            if (sprm.Identifier == Ww8SprmReader.Ids.PictureLocation) return sprm.DoubleWord;
        }

        return null;
    }

    /// <summary>
    /// Whether the drawing anchored at a position is an embedded picture rather than a shape.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A U+0008 always stands for something in the drawing layer, but the drawing layer is also where
    /// Word puts a floating <em>picture</em>: a shape of type <c>mso_sptPictureFrame</c> whose
    /// <c>pib</c> property indexes the blip store. So the character alone cannot decide, and the
    /// answer is two lookups away — the <c>FSPA</c> at the position, then the shape it names.
    /// </para>
    /// <para>
    /// An anchor whose shape is missing from the drawing is reported as <em>not</em> a picture. Word
    /// leaves anchor records behind for shapes that have been deleted, and inventing an image for one
    /// is the same error this whole file exists to fix.
    /// </para>
    /// </remarks>
    private bool IsDrawnPicture(int position) => IsPictureShape(Drawings.ShapeAt(position));
}

