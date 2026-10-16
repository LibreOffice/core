using XRay.Office.MsBinary.Escher;
using XRay.Office.MsBinary.Records;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// One outline level of one main master's PowerPoint 97+ paragraph extensions.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <see cref="PptExtendedParagraph"/> for a <em>level</em> rather than for a
/// character run's selection — <c>PPTExtParaLevel</c>,
/// <c>include/filter/msfilter/svdfppt.hxx:693-705</c>. The two are merged rather than chosen
/// between: <c>PPTNumberFormatCreator::ImplGetExtNumberFormat</c> takes each of the three fields
/// from the paragraph when the paragraph's own mask names it and from here when it does not
/// (<c>filter/source/msfilter/svdfppt.cxx:3407-3446</c>).
/// </para>
/// <para>
/// <strong>A deck that gives a whole outline level a picture bullet states it only here.</strong>
/// Measured on <c>slides/done-003/ppt/ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt</c>:
/// the file holds six <c>ExtendedParagraphMasterAtom</c> records — two main masters × the
/// <see cref="PptTextKind.Body"/>, <see cref="PptTextKind.HalfBody"/> and
/// <see cref="PptTextKind.QuarterBody"/> instances — whose levels 0 and 1 carry
/// <c>mask=0x03800000 buBlip=0/1</c> and <c>2/3</c>, and only <b>three</b> blip-bearing entries
/// anywhere in a shape's or the document's own <c>ExtendedParagraphAtom</c>.
/// </para>
/// </remarks>
/// <param name="Mask">
/// <c>mnExtParagraphMask</c>: bit 23 states <see cref="BulletBlip"/>, bit 24
/// <see cref="Scheme"/> and bit 25 <see cref="HasAutoNumber"/>. It decides both what the level
/// says and — because every field is optional — how many bytes the level occupies.
/// </param>
/// <param name="BulletBlip">The picture-bullet index, or <c>0xFFFF</c> for none.</param>
/// <param name="HasAutoNumber">Whether the level's paragraphs are automatically numbered.</param>
/// <param name="Scheme">The numbering scheme in the low word and the start value in the high one.</param>
public readonly record struct PptExtendedParagraphLevel(
    uint Mask,
    ushort BulletBlip = PptExtendedParagraph.NoBulletBlip,
    bool HasAutoNumber = false,
    uint Scheme = 0)
{
    /// <summary>The mask bit that says the level states a picture bullet.</summary>
    public const uint StatesBulletBlip = 0x0080_0000;

    /// <summary>The mask bit that says it states a numbering scheme.</summary>
    public const uint StatesScheme = 0x0100_0000;

    /// <summary>The mask bit that says it states whether the level is numbered.</summary>
    public const uint StatesAutoNumber = 0x0200_0000;

    /// <summary>All three, which is the value that stops the merge from happening at all.</summary>
    public const uint StatesEverything = StatesBulletBlip | StatesScheme | StatesAutoNumber;

    /// <summary>The mask bit that says a PP10 extension word follows.</summary>
    private const uint StatesPp10 = 0x0400_0000;

    /// <summary>Whether the level was written at all; an unwritten one merges nothing.</summary>
    public bool IsSet => Mask != 0;

    /// <summary>
    /// Reads one level, advancing past exactly the fields its mask states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ReadPPTExtParaLevel</c>, <c>svdfppt.cxx:3185-3201</c>, field for field and in its order:
    /// the paragraph mask, <c>buBlip</c>, <c>hasAnm</c>, <c>anmScheme</c>, a PP10 extension, then a
    /// character mask with one optional word.
    /// </para>
    /// <para>
    /// <strong>It is <em>not</em> the same structure as a shape's entry, and the difference is at
    /// the end rather than in the middle.</strong> The two agree field for field as far as the
    /// character mask; <c>StyleTextProp9::Read</c> (<c>:4812-4830</c>, which
    /// <see cref="PptTextReader.ReadExtendedParagraphAtom"/> mirrors) then reads a <em>third</em>
    /// mask — the special-info one — with two more optional fields behind it, and this reads
    /// nothing. Every field is optional, so reading a level with the paragraph's reader consumes
    /// four bytes too many and takes the <em>next</em> level from the wrong offset rather than
    /// losing one value.
    /// </para>
    /// </remarks>
    public static PptExtendedParagraphLevel Read(ReadOnlySpan<byte> content, ref int position)
    {
        uint mask = Take32(content, ref position);

        ushort blip = PptExtendedParagraph.NoBulletBlip;
        bool numbered = false;
        uint scheme = 0;

        if ((mask & StatesBulletBlip) != 0) blip = Take16(content, ref position);
        if ((mask & StatesAutoNumber) != 0) numbered = Take16(content, ref position) != 0;
        if ((mask & StatesScheme) != 0) scheme = Take32(content, ref position);
        if ((mask & StatesPp10) != 0) position += 4;

        uint character = Take32(content, ref position);
        if ((character & 0x0010_0000) != 0) position += 4;

        return new PptExtendedParagraphLevel(mask, blip, numbered, scheme);
    }

    private static ushort Take16(ReadOnlySpan<byte> content, ref int position)
    {
        ushort value = position + 2 <= content.Length
            ? DffRecordBuffer.ReadUInt16(content[position..])
            : (ushort)0;
        position += 2;
        return value;
    }

    private static uint Take32(ReadOnlySpan<byte> content, ref int position)
    {
        uint value = position + 4 <= content.Length
            ? DffRecordBuffer.ReadUInt32(content[position..])
            : 0;
        position += 4;
        return value;
    }
}

/// <summary>
/// The extended paragraph levels one main master states, indexed by text kind and outline level.
/// </summary>
public sealed class PptExtendedParagraphSheet
{
    private const int Instances = 9;

    private readonly PptExtendedParagraphLevel[][] _levels;

    private PptExtendedParagraphSheet(PptExtendedParagraphLevel[][] levels) => _levels = levels;

    /// <summary>True when the master wrote no extended level at all.</summary>
    /// <remarks>
    /// <c>PPTExtParaProv::bStyles</c>, which gates the whole merge (<c>svdfppt.cxx:3422</c>): a
    /// master with no atom leaves the paragraph's own answer alone rather than merging zeroes into
    /// it.
    /// </remarks>
    public bool IsEmpty { get; private init; }

    /// <summary>The level one text kind states at one depth, or an unset level.</summary>
    public PptExtendedParagraphLevel Level(PptTextKind kind, int level)
    {
        if (IsEmpty) return default;

        int instance = (int)kind;
        if (instance < 0 || instance >= Instances) return default;
        if (level < 0 || level >= PptStyleSheet.MaxLevels) return default;

        return _levels[instance][level];
    }

    /// <summary>
    /// Reads a main master's <c>ExtendedParagraphMasterAtom</c> records.
    /// </summary>
    /// <remarks>
    /// The atoms sit under the master's own <c>___PPT9</c> tag, three containers down, exactly
    /// where a shape keeps its <see cref="PptRecordTypes.ExtendedParagraphAtom"/>. Each record's
    /// <em>instance</em> is the text kind, and its content is a level count followed by that many
    /// variable-length levels (<c>svdfppt.cxx:3330-3344</c>).
    /// </remarks>
    /// <param name="stream">The document stream.</param>
    /// <param name="master">The <c>MainMaster</c> container.</param>
    public static PptExtendedParagraphSheet Read(DffRecordBuffer stream, DffRecordHeader master)
    {
        ArgumentNullException.ThrowIfNull(stream);

        PptExtendedParagraphLevel[][] levels = new PptExtendedParagraphLevel[Instances][];
        for (int instance = 0; instance < Instances; instance++)
        {
            levels[instance] = new PptExtendedParagraphLevel[PptStyleSheet.MaxLevels];
        }

        bool any = false;

        foreach (DffRecordHeader record in PptProgTags.Content(stream, master))
        {
            if (record.Type != PptRecordTypes.ExtendedParagraphMasterAtom) continue;
            if (record.Instance >= Instances) continue;

            ReadOnlySpan<byte> content = stream.Content(record);
            if (content.Length < 2) continue;

            int depth = Math.Min((int)DffRecordBuffer.ReadUInt16(content), PptStyleSheet.MaxLevels);
            int position = 2;

            for (int level = 0; level < depth && position < content.Length; level++)
            {
                levels[record.Instance][level] =
                    PptExtendedParagraphLevel.Read(content, ref position);
                any = true;
            }
        }

        return new PptExtendedParagraphSheet(levels) { IsEmpty = !any };
    }

    /// <summary>A sheet for a master that has none, which merges nothing.</summary>
    public static PptExtendedParagraphSheet None { get; } =
        new([]) { IsEmpty = true };
}

/// <summary>
/// The document's picture-bullet graphics, indexed by the <c>buBlip</c> that names one.
/// </summary>
/// <remarks>
/// <para>
/// Separate from the Escher blip store and reached by a different route: these live under the
/// document's <see cref="PptRecordTypes.List"/> record rather than in the drawing group, and each
/// is a bare blip record rather than a store entry. <c>PPTExtParaProv</c>'s constructor reads them
/// into <c>aBuGraList</c> and <c>GetGraphic</c> looks one up by instance
/// (<c>svdfppt.cxx:3203-3300</c>).
/// </para>
/// <para>
/// <strong>Whether the graphic is <em>there</em> decides the bullet's kind, not just its
/// picture.</strong> <c>ImplGetExtNumberFormat</c> sets <c>SVX_NUM_BITMAP</c> only inside
/// <c>if (pParaProv-&gt;GetGraphic(nBuBlip, aGraphic))</c> (<c>svdfppt.cxx:3448-3465</c>), so a
/// <c>buBlip</c> pointing at nothing leaves the level's character bullet in force. That is why
/// this is a store and not a set of indices.
/// </para>
/// </remarks>
public sealed class PptBulletPictures
{
    private readonly Dictionary<ushort, EscherBlip> _graphics;

    /// <summary>
    /// One decoded picture per blip, shared by every paragraph that names it.
    /// </summary>
    /// <remarks>
    /// <strong>Not a speed measure — a size one, and it was measured.</strong> A fresh
    /// <c>RasterImage</c> per paragraph is a distinct picture to the PDF writer, so a deck with
    /// 186 picture bullets embeds the same four PNGs 186 times and lists all 186 in every page's
    /// resources: on
    /// <c>ws_prod-…-M.017-(French)-France.ppt</c> that took the rendering from 190 154 bytes to
    /// 277 550 and page 8's resource dictionary from 1 image to 186, of which 7 are drawn.
    /// Sharing the instance takes it back.
    /// </remarks>
    private readonly Dictionary<ushort, object?> _decoded = [];

    private PptBulletPictures(Dictionary<ushort, EscherBlip> graphics) => _graphics = graphics;

    /// <summary>
    /// The picture a blip names, decoded once, or null when the store has no graphic for it.
    /// </summary>
    /// <param name="blip">The <c>buBlip</c> index.</param>
    /// <param name="decode">
    /// How to turn the blip's bytes into a picture. Called at most once per index; the caller owns
    /// the decoding because the media types and the vector registry belong to the presentations
    /// layer rather than to this store.
    /// </param>
    public T? Decoded<T>(ushort blip, Func<EscherBlip, T?> decode)
        where T : class
    {
        ArgumentNullException.ThrowIfNull(decode);

        if (_decoded.TryGetValue(blip, out object? cached)) return (T?)cached;

        T? picture = Graphic(blip) is { } graphic ? decode(graphic) : null;
        _decoded[blip] = picture;
        return picture;
    }

    /// <summary>An empty store, for a document that carries no picture bullet.</summary>
    public static PptBulletPictures None { get; } = new([]);

    /// <summary>True when the document states no picture bullet anywhere.</summary>
    public bool IsEmpty => _graphics.Count == 0;

    /// <summary>The graphic a <c>buBlip</c> names, or null when the store has none.</summary>
    public EscherBlip? Graphic(ushort blip)
        => blip != PptExtendedParagraph.NoBulletBlip
           && _graphics.TryGetValue(blip, out EscherBlip graphic)
            ? graphic
            : null;

    /// <summary>
    /// Reads the store out of the document's <see cref="PptRecordTypes.List"/> record.
    /// </summary>
    /// <param name="stream">The document stream.</param>
    /// <param name="document">The <c>Document</c> container.</param>
    public static PptBulletPictures Read(DffRecordBuffer stream, DffRecordHeader document)
    {
        ArgumentNullException.ThrowIfNull(stream);

        Dictionary<ushort, EscherBlip> graphics = [];

        foreach (DffRecordHeader list in stream.Children(document))
        {
            if (list.Type != PptRecordTypes.List) continue;

            foreach (DffRecordHeader tagged in PptProgTags.Content(stream, list))
            {
                if (tagged.Type != PptRecordTypes.ExtendedBuGraContainer) continue;

                foreach (DffRecordHeader atom in stream.Children(tagged))
                {
                    if (atom.Type != PptRecordTypes.ExtendedBuGraAtom) continue;

                    // A two-byte type word, then the blip record itself.
                    if (atom.Length <= (uint)TypeWordSize) continue;
                    if (EscherBlips.Direct(stream, atom.ContentStart + TypeWordSize) is not { } blip)
                    {
                        continue;
                    }

                    graphics.TryAdd(atom.Instance, blip);
                }
            }
        }

        return graphics.Count == 0 ? None : new PptBulletPictures(graphics);
    }

    /// <summary>The <c>nType</c> word an <c>ExtendedBuGraAtom</c> opens with.</summary>
    private const int TypeWordSize = 2;
}

/// <summary>
/// The records inside a container's PowerPoint 97+ tagged block.
/// </summary>
/// <remarks>
/// <c>SdrPowerPointImport::SeekToContentOfProgTag(9, …)</c>, <c>svdfppt.cxx:2401-2445</c>:
/// <c>ProgTags</c> → a <c>ProgBinaryTag</c> whose opening <c>CString</c> reads <c>___PPT9</c> →
/// its <c>BinaryTagData</c>. Three containers deep, with the version encoded in a string, which
/// is why every caller reaching one of these records has to walk the same path.
/// </remarks>
internal static class PptProgTags
{
    private const string Name = "___PPT9";

    /// <summary>The children of the <c>___PPT9</c> tag's data, or nothing when there is none.</summary>
    public static IEnumerable<DffRecordHeader> Content(
        DffRecordBuffer stream, DffRecordHeader container)
    {
        foreach (DffRecordHeader tags in stream.Children(container))
        {
            if (tags.Type != PptRecordTypes.ProgTags) continue;

            foreach (DffRecordHeader tag in stream.Children(tags))
            {
                if (tag.Type != PptRecordTypes.ProgBinaryTag) continue;
                if (!IsNamed(stream, tag)) continue;

                foreach (DffRecordHeader payload in stream.Children(tag))
                {
                    if (payload.Type != PptRecordTypes.BinaryTagData) continue;

                    foreach (DffRecordHeader record in stream.Children(payload))
                    {
                        yield return record;
                    }
                }
            }
        }
    }

    private static bool IsNamed(DffRecordBuffer stream, DffRecordHeader tag)
    {
        foreach (DffRecordHeader child in stream.Children(tag))
        {
            if (child.Type != PptRecordTypes.CString) continue;

            ReadOnlySpan<byte> content = stream.Content(child);
            if (content.Length % 2 != 0) return false;

            Span<char> characters = content.Length <= 64
                ? stackalloc char[content.Length / 2]
                : new char[content.Length / 2];

            for (int index = 0; index < characters.Length; index++)
            {
                characters[index] = (char)DffRecordBuffer.ReadUInt16(content[(index * 2)..]);
            }

            return characters.SequenceEqual(Name);
        }

        return false;
    }
}
