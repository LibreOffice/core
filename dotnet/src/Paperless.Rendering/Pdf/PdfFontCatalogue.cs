using System.Buffers.Binary;
using System.Globalization;
using System.Text;
using Paperless.Core.Graphics;
using Paperless.Text.Fonts;

namespace Paperless.Rendering.Pdf;

/// <summary>
/// The fonts a PDF ends up holding: which glyph got which code, and what that code says.
/// </summary>
/// <remarks>
/// <para>
/// Codes are allocated as glyphs are first drawn, which is what lets the content stream be
/// written in one pass. The font programs are built at the end, to the codes the content
/// already used.
/// </para>
/// <para>
/// <b>Simple TrueType fonts with one-byte codes</b>, not composite Identity-H ones, and the
/// choice is measured rather than aesthetic. LibreOffice's own export writes
/// <c>/Subtype/TrueType</c> with <c>/FirstChar 0 /LastChar 28</c> and a content stream of
/// two hex digits per glyph — verified on its PDF of <c>tests/corpus/minimal/prose-odt.odt</c>
/// — and the fidelity harness reads that shape. A composite font would double every glyph
/// code to four hex digits, so <c>PdfTextRuns.GlyphCount</c> would report twice the glyphs
/// on our side and half the comparison would be arithmetic rather than agreement.
/// </para>
/// <para>
/// The cost is the 255-code ceiling, which is why a face becomes several PDF fonts when a
/// document uses more than 255 of its glyphs. LibreOffice pays the same cost the same way.
/// </para>
/// </remarks>
internal sealed class PdfFontCatalogue(IPdfFontProvider provider, bool embed)
{
    /// <summary>
    /// How many codes one PDF font can hold: 1 to 255, with 0 kept for <c>.notdef</c>.
    /// </summary>
    private const int CodesPerSubset = 255;

    private readonly Dictionary<string, Face> _faces = new(StringComparer.Ordinal);
    private readonly List<Subset> _subsets = [];

    /// <summary>
    /// The PDF resource name and character code that will draw a glyph.
    /// </summary>
    /// <param name="font">The face the run resolved to.</param>
    /// <param name="glyphId">The glyph index within that face.</param>
    /// <param name="text">The characters the glyph stands for, for <c>ToUnicode</c>.</param>
    public (string Resource, byte Code) Map(FontReference font, ushort glyphId, string? text)
    {
        ArgumentNullException.ThrowIfNull(font);

        Face face = FaceFor(font);

        // Across every subset of the face, not just the one being filled. A glyph already placed
        // keeps its code: looking only in the current subset would re-place all 255 glyphs of the
        // first one into the second the moment a face overflowed, which produces a third subset,
        // then a fourth, and a content stream that switches font between consecutive letters.
        if (face.Placed.TryGetValue(glyphId, out (Subset Subset, byte Code) existing))
        {
            RecordText(existing.Subset, existing.Code, text);
            return (existing.Subset.Resource, existing.Code);
        }

        if (face.Current.GlyphsByCode.Count > CodesPerSubset) face.Current = NewSubset(face);

        byte code = (byte)face.Current.GlyphsByCode.Count;
        face.Current.GlyphsByCode.Add(glyphId);
        face.Placed[glyphId] = (face.Current, code);
        RecordText(face.Current, code, text);

        return (face.Current.Resource, code);
    }

    /// <summary>
    /// Records what a run says its glyphs advance by, for a face whose file could not be read.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Call before drawing a run. It does nothing for the ordinary case, where the face's own
    /// file loaded and its <c>hmtx</c> table is the authority. It matters when the file did
    /// not load — a <see cref="FontReference.FaceKey"/> that names a family rather than a
    /// path, which is what a caller building a reference by hand produces — because the
    /// alternative is a <c>/Widths</c> array of zeros.
    /// </para>
    /// <para>
    /// Zeros are not merely untidy. Every glyph then sits a full advance away from where the
    /// stated widths put the pen, so <see cref="PdfContentSink"/> corrects each one with a
    /// <c>TJ</c> adjustment of the whole advance — measured at <c>-722</c> thousandths
    /// between adjacent glyphs on <c>sheet-print-xlsx.xlsx</c>. The page still <em>looks</em>
    /// right, which is why no operator comparison caught it, but an adjustment that large is
    /// how a PDF spells a word break: <c>pdftotext</c> got 13255 words out of that file's
    /// fourteen pages against LibreOffice's 2281, one per character. "Real searchable text"
    /// is the stated reason the display list carries glyph runs rather than outlines, so a
    /// width the file does not know is better taken from the run than left at zero.
    /// </para>
    /// <para>
    /// The first advance seen for a glyph wins and a later one never revises it, because the
    /// content stream is written in one pass: by the time a second run states a wider blank —
    /// which is what justification produces — the pen adjustments of the first have already
    /// been computed against the earlier number, and changing the width afterwards would move
    /// every glyph the first run drew. The wider occurrence is corrected with a <c>TJ</c>
    /// instead, which is the same mechanism a justified line already uses when the widths are
    /// exact.
    /// </para>
    /// <para>
    /// This is a fallback and not a fix for the caller. A face whose file did not load is also
    /// not <em>embedded</em>, so the reader supplies its own glyph shapes; only the metrics and
    /// the word breaks are recovered here.
    /// </para>
    /// </remarks>
    public void Observe(GlyphRun run)
    {
        ArgumentNullException.ThrowIfNull(run);

        Face face = FaceFor(run.Font);
        if (face.OpenType is not null || run.FontSize.Emu <= 0) return;

        foreach (PositionedGlyph glyph in run.Glyphs)
        {
            if (glyph.Advance.Emu <= 0 || face.Measured.ContainsKey(glyph.GlyphId)) continue;

            face.Measured[glyph.GlyphId] = Math.Round(
                glyph.Advance.Emu * 1000.0 / run.FontSize.Emu, 4, MidpointRounding.AwayFromZero);
        }
    }

    /// <summary>
    /// A glyph's advance in thousandths of an em, exactly as the <c>/Widths</c> array will
    /// state it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same number on both sides on purpose. A PDF advances the pen by the width it was
    /// told, so the text-showing adjustments that place each glyph are computed against the
    /// width in the file rather than against the font's own; a writer that used the design
    /// value here and a rounded one there would leave half a thousandth of an em of error
    /// per glyph, which accumulates along the line rather than cancelling.
    /// </para>
    /// <para>
    /// Stated to four decimals rather than as the integer LibreOffice writes, and that is
    /// worth the deviation: Carlito's <c>a</c> is 1084 units on a 2048 grid, which is
    /// 529.3 thousandths and 529 rounded. The tenth lost per glyph is 0.004 pt at eleven
    /// point, so a sixty-glyph line ends a quarter of a point short of where layout put it
    /// — over the tenth-of-a-point bound everything else in this project is held to. Real
    /// widths cost four characters each and remove the drift instead of correcting it.
    /// </para>
    /// </remarks>
    public double Width(FontReference font, ushort glyphId)
    {
        ArgumentNullException.ThrowIfNull(font);

        Face face = FaceFor(font);
        if (face.Widths.TryGetValue(glyphId, out double cached)) return cached;

        double width = face.WidthOf(glyphId);
        face.Widths[glyphId] = width;
        return width;
    }

    /// <summary>Writes every font object and appends the <c>/Font</c> resource entries.</summary>
    public void Write(PdfDocumentWriter writer, StringBuilder resources)
    {
        ArgumentNullException.ThrowIfNull(writer);
        ArgumentNullException.ThrowIfNull(resources);

        if (_subsets.Count == 0) return;

        resources.Append("/Font<<");
        for (int i = 0; i < _subsets.Count; i++)
        {
            Subset subset = _subsets[i];
            int id = WriteSubset(writer, subset, this);
            resources.Append(CultureInfo.InvariantCulture, $"/{subset.Resource} {id} 0 R");
        }

        resources.Append(">>");
    }

    // ------------------------------------------------------------------------- allocation

    private Face FaceFor(FontReference font)
    {
        string key = font.FaceKey.Length > 0 ? font.FaceKey : font.FamilyName;
        if (_faces.TryGetValue(key, out Face? existing)) return existing;

        PdfFontProgram? program = embed ? provider.Load(font) : null;
        OpenTypeFace? opentype = program is { } bytes
            ? OpenTypeFace.Read(bytes.Data.ToArray(), bytes.FaceIndex)
            : null;

        Face face = new(font, program, opentype);
        face.Current = NewSubset(face);
        _faces[key] = face;
        return face;
    }

    private Subset NewSubset(Face face)
    {
        Subset subset = new(
            face,
            string.Create(CultureInfo.InvariantCulture, $"F{_subsets.Count + 1}"),
            Tag(_subsets.Count));

        _subsets.Add(subset);
        return subset;
    }

    private static void RecordText(Subset subset, byte code, string? text)
    {
        if (string.IsNullOrEmpty(text)) return;
        if (subset.Text.ContainsKey(code)) return;

        subset.Text[code] = text;
    }

    /// <summary>
    /// The six-letter subset tag a PDF puts before an embedded font's name.
    /// </summary>
    /// <remarks>
    /// Derived from the subset's ordinal rather than from a hash of its glyphs, which keeps
    /// output byte-identical between runs. Least significant letter first, so the sequence
    /// reads <c>AAAAAA</c>, <c>BAAAAA</c>, <c>CAAAAA</c> — the same shape LibreOffice emits.
    /// </remarks>
    private static string Tag(int ordinal)
    {
        char[] letters = ['A', 'A', 'A', 'A', 'A', 'A'];
        int value = Math.Max(ordinal, 0);

        for (int i = 0; i < letters.Length && value > 0; i++)
        {
            letters[i] = (char)('A' + (value % 26));
            value /= 26;
        }

        return new string(letters);
    }

    // ----------------------------------------------------------------------------- writing

    /// <remarks>
    /// The widths come back through <see cref="Width"/> rather than being recomputed here,
    /// so the array in the file states exactly the numbers the content stream placed its
    /// glyphs against. Two derivations of the same value would differ the moment one of them
    /// falls back to a measured advance.
    /// </remarks>
    private static int WriteSubset(PdfDocumentWriter writer, Subset subset, PdfFontCatalogue catalogue)
    {
        OpenTypeFace? opentype = subset.Face.OpenType;
        int upem = opentype?.UnitsPerEm is > 0 ? opentype.UnitsPerEm : 1000;
        int last = subset.GlyphsByCode.Count - 1;

        StringBuilder widths = new("[");
        for (int code = 0; code <= last; code++)
        {
            if (code > 0) widths.Append(' ');
            widths.Append(PdfSyntax.Number(catalogue.Width(subset.Face.Reference, subset.GlyphsByCode[code])));
        }

        widths.Append(']');

        string baseName = BaseName(subset.Face.Reference);
        string name = $"{subset.Tag}+{baseName}";

        int toUnicode = writer.Reserve();
        writer.SetStream(toUnicode, string.Empty, Encoding.Latin1.GetBytes(ToUnicode(subset)), compress: false);

        byte[]? embedded = Embed(subset);

        // A CFF-flavoured face is named and not embedded. Everything else here writes a simple
        // /Subtype/TrueType font whose one-byte codes select glyphs through an identity
        // Macintosh cmap, and a CFF face cannot be addressed that way at all:
        //
        //   * under a TrueType dictionary the program is a /FontFile2, which promises `glyf`
        //     outlines. Poppler says "Mismatch between font type and embedded font file" and,
        //     for a CID-keyed one, "No font in show" for every text operation — measured 161
        //     times on `16 - UTM - (NASA).pptx`, whose runs fall back to Unifont.
        //   * under a Type1 dictionary (PDF 1.7 §9.9, which is where a name-keyed CFF belongs)
        //     a code selects a glyph by *name* through the CFF charset. Our codes are indices,
        //     so every one of them misses: measured on an 18 pt Loma probe, poppler drew a row
        //     of tofu boxes where LibreOffice drew the words.
        //   * a CID-keyed CFF is not admissible under any simple dictionary — it needs a
        //     composite /Type0 font with a CIDFontType0 descendant.
        //
        // So the face is named and not embedded, which is what a face whose file could not be
        // read already does: the widths are still the face's, so pen positions and line breaks
        // are unchanged, and the text still extracts. Measured honestly, this buys a valid file
        // and not correct glyphs — the reference draws Loma and both the old output and this one
        // draw tofu, and on the UTM deck it turns 161 hard reader errors and a blank page into a
        // clean file. Putting the right outlines on the page is two further pieces of work: a
        // /Differences glyph-name encoding read out of the CFF charset for a name-keyed face,
        // and a composite /Type0 font with Identity-H two-byte codes for a CID-keyed one.
        if (embedded is not null && IsCompactFontFormat(embedded)) embedded = null;

        int descriptor = WriteDescriptor(writer, subset, name, upem, embedded);

        return writer.Add(
            $"<</Type/Font/Subtype/TrueType/BaseFont/{name}/FirstChar 0/LastChar {last}"
            + $"/Widths{widths}/FontDescriptor {descriptor} 0 R/ToUnicode {toUnicode} 0 R>>");
    }

    private static int WriteDescriptor(
        PdfDocumentWriter writer, Subset subset, string name, int upem, byte[]? embedded)
    {
        OpenTypeFace? opentype = subset.Face.OpenType;
        FontReference reference = subset.Face.Reference;

        // Symbolic, always: the encoding is glyph indices rather than any standard character set, and
        // a non-symbolic font would have its codes reinterpreted through StandardEncoding.
        int flags = 4;
        if (opentype?.IsFixedPitch == true) flags |= 1;
        if (reference.IsItalic || opentype?.IsItalic == true) flags |= 64;

        double scale = 1000.0 / upem;
        (int xMin, int yMin, int xMax, int yMax) = BoundingBox(subset.Face);
        int ascent = opentype is null ? 800 : (int)Math.Round(opentype.Horizontal.Ascender * scale);
        int descent = opentype is null ? -200 : (int)Math.Round(opentype.Horizontal.Descender * scale);
        int capHeight = opentype?.Os2?.CapHeight is > 0
            ? (int)Math.Round(opentype.Os2.Value.CapHeight * scale)
            : ascent;
        double italicAngle = opentype?.Post.ItalicAngle ?? 0;

        string fontFile = string.Empty;

        if (embedded is not null)
        {
            int program = writer.Reserve();

            // Uncompressed, which is not an oversight. The fidelity harness finds a PDF's content
            // streams by inflating every stream in the file and keeping the ones holding "BT"; a
            // deflated font program is binary that inflates successfully and contains "BT" often
            // enough to be read as a page. Leaving font programs stored keeps them invisible to it.
            //
            // /FontFile2 with /Length1, unconditionally, because only a `glyf`-flavoured program
            // reaches here: see WriteSubset, which drops a CFF one rather than misdescribe it.
            writer.SetStream(
                program,
                string.Create(CultureInfo.InvariantCulture, $"/Length1 {embedded.Length}"),
                embedded,
                compress: false);

            fontFile = $"/FontFile2 {program} 0 R";
        }

        return writer.Add(
            $"<</Type/FontDescriptor/FontName/{name}/Flags {flags}"
            + $"/FontBBox[{xMin} {yMin} {xMax} {yMax}]"
            + $"/ItalicAngle {PdfSyntax.Number(italicAngle)}/Ascent {ascent}/Descent {descent}"
            + $"/CapHeight {capHeight}/StemV {StemWidth(reference)}{fontFile}>>");
    }

    /// <summary>
    /// Whether an embedded face carries CFF outlines rather than <c>glyf</c> ones.
    /// </summary>
    /// <remarks>
    /// <para>
    /// It decides whether the program can be embedded at all, and getting it wrong is invisible
    /// to every check short of looking at the page. Written as a <c>/FontFile2</c> — which
    /// promises a TrueType program — a reader rejects the mismatch. Measured on
    /// <c>16 - UTM - (NASA).pptx</c>: poppler reported *"Mismatch between font type and embedded
    /// font file"* and then *"No font in show"* 161 times, so 161 glyph runs drew nothing at all
    /// while <c>pdftotext</c> extracted every one of them and <c>pdffonts</c> reported the face
    /// as embedded. Page count, word count and font embedding all passed.
    /// </para>
    /// <para>
    /// Decided from the bytes rather than from the file name or the source face, because it is a
    /// property of what <c>hb-subset</c> produced: an sfnt version of <c>OTTO</c> is CFF and
    /// <c>0x00010000</c> or <c>true</c> is <c>glyf</c>. Every <c>.otf</c> on a machine is the
    /// former, so this reaches any document that resolves one — here only through the Unifont
    /// last-resort fallback, but not by anything special about that font.
    /// </para>
    /// </remarks>
    private static bool IsCompactFontFormat(ReadOnlySpan<byte> program)
        => program.Length >= 4 && BinaryPrimitives.ReadUInt32BigEndian(program) == OpenTypeCffTag;

    /// <summary>The sfnt version tag <c>OTTO</c>, which marks CFF outlines.</summary>
    private const uint OpenTypeCffTag = 0x4F54544F;

    /// <summary>
    /// A nominal vertical stem width.
    /// </summary>
    /// <remarks>
    /// Required by the specification and used by no reader that has the font program, which
    /// is why LibreOffice writes a flat 80 for every face it embeds. Ours varies with weight
    /// only so that a bold face does not claim a regular face's stems to a reader that has
    /// to synthesise one.
    /// </remarks>
    private static int StemWidth(FontReference reference) => reference.Weight >= 600 ? 140 : 80;

    /// <summary>The face's bounding box in thousandths of an em, from its <c>head</c> table.</summary>
    private static (int XMin, int YMin, int XMax, int YMax) BoundingBox(Face face)
    {
        if (face.Program is not { } program) return (-500, -300, 1500, 1000);
        if (SfntTables.Read(program.Data.Span, program.FaceIndex)?.Table("head") is not { Length: >= 44 } head)
        {
            return (-500, -300, 1500, 1000);
        }

        int upem = BinaryPrimitives.ReadUInt16BigEndian(head.AsSpan(18));
        double scale = 1000.0 / (upem > 0 ? upem : 1000);

        return (
            (int)Math.Round(BinaryPrimitives.ReadInt16BigEndian(head.AsSpan(36)) * scale),
            (int)Math.Round(BinaryPrimitives.ReadInt16BigEndian(head.AsSpan(38)) * scale),
            (int)Math.Round(BinaryPrimitives.ReadInt16BigEndian(head.AsSpan(40)) * scale),
            (int)Math.Round(BinaryPrimitives.ReadInt16BigEndian(head.AsSpan(42)) * scale));
    }

    private static byte[]? Embed(Subset subset)
        => subset.Face.Program is { } program
            ? FontSubsetter.Subset(program.Data, program.FaceIndex, subset.GlyphsByCode)
            : null;

    /// <summary>
    /// A PDF name for the family, with everything a name token cannot hold removed.
    /// </summary>
    private static string BaseName(FontReference reference)
    {
        StringBuilder name = new();
        foreach (char c in reference.FamilyName)
        {
            if (c is (>= 'A' and <= 'Z') or (>= 'a' and <= 'z') or (>= '0' and <= '9') or '-') name.Append(c);
        }

        return name.Length > 0 ? name.ToString() : "Unnamed";
    }

    /// <summary>
    /// The <c>ToUnicode</c> CMap: what each code says, so the text can be found and copied.
    /// </summary>
    /// <remarks>
    /// The reason the display list carries <c>Text</c> and a <c>ClusterMap</c> at all. A PDF
    /// whose glyphs are addressed by index says nothing about its own content — a search for
    /// a word in it finds nothing and a copy yields mojibake — unless this table is written,
    /// and a PDF of outlines could not write one at all.
    /// </remarks>
    private static string ToUnicode(Subset subset)
    {
        StringBuilder map = new();
        map.Append(
            """
            /CIDInit/ProcSet findresource begin
            12 dict begin
            begincmap
            /CIDSystemInfo<</Registry(Adobe)/Ordering(UCS)/Supplement 0>> def
            /CMapName/Adobe-Identity-UCS def
            /CMapType 2 def
            1 begincodespacerange
            <00> <FF>
            endcodespacerange

            """);

        List<byte> codes = [.. subset.Text.Keys];
        codes.Sort();

        for (int start = 0; start < codes.Count; start += 100)
        {
            int count = Math.Min(100, codes.Count - start);
            map.Append(CultureInfo.InvariantCulture, $"{count} beginbfchar\n");

            for (int i = start; i < start + count; i++)
            {
                byte code = codes[i];
                map.Append(CultureInfo.InvariantCulture, $"<{code:X2}> ")
                   .Append(PdfSyntax.HexString(Encoding.BigEndianUnicode.GetBytes(subset.Text[code])))
                   .Append('\n');
            }

            map.Append("endbfchar\n");
        }

        map.Append(
            """
            endcmap
            CMapName currentdict /CMap defineresource pop
            end
            end
            """);

        return map.ToString();
    }

    // ------------------------------------------------------------------------------- state

    private sealed class Face(FontReference reference, PdfFontProgram? program, OpenTypeFace? opentype)
    {
        public FontReference Reference { get; } = reference;

        public PdfFontProgram? Program { get; } = program;

        public OpenTypeFace? OpenType { get; } = opentype;

        public Dictionary<ushort, double> Widths { get; } = [];

        /// <summary>
        /// Advances taken off the display list, for a face whose own file could not be read.
        /// Empty in the ordinary case, where <see cref="OpenType"/> is the authority.
        /// </summary>
        public Dictionary<ushort, double> Measured { get; } = [];

        /// <summary>A glyph's advance in thousandths of an em, from the file or from the run.</summary>
        public double WidthOf(ushort glyphId)
        {
            if (OpenType is not { } opentype) return Measured.GetValueOrDefault(glyphId);

            int upem = opentype.UnitsPerEm > 0 ? opentype.UnitsPerEm : 1000;
            return Math.Round(opentype.AdvanceOf(glyphId) * 1000.0 / upem, 4, MidpointRounding.AwayFromZero);
        }

        /// <summary>Where each glyph of this face ended up: which PDF font, and at which code.</summary>
        public Dictionary<ushort, (Subset Subset, byte Code)> Placed { get; } = [];

        public Subset Current { get; set; } = null!;
    }

    private sealed class Subset(Face face, string resource, string tag)
    {
        public Face Face { get; } = face;

        public string Resource { get; } = resource;

        public string Tag { get; } = tag;

        /// <summary>The original glyph for each code; index 0 is <c>.notdef</c>.</summary>
        public List<ushort> GlyphsByCode { get; } = [0];

        public Dictionary<byte, string> Text { get; } = [];
    }
}
