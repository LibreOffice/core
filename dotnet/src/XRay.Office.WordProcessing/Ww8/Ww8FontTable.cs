using System.Buffers.Binary;
using System.Text;
using XRay.Office.Text.Fonts;

namespace XRay.Office.WordProcessing.Ww8;

/// <summary>
/// A DOC's font table: the families its <c>sprmCRgFtc0</c> indexes name.
/// </summary>
/// <remarks>
/// <para>
/// A run of <c>FFN</c> structures, each preceded by its own length. They are variable-sized and there is
/// no index, so a reader can only walk them forwards — which means one malformed length loses every font
/// after it, and is why the walk stops rather than trying to resynchronise.
/// </para>
/// <para>
/// The name is at a fixed offset within each entry, after four bytes of flags and weight and character
/// set, one index to the alternate name, ten bytes of PANOSE and twenty-four of font signature. Those
/// last two are the ones easy to forget: skipping only PANOSE puts the name twenty-four bytes early and
/// reads the signature as UTF-16, which produces a plausible-looking string of CJK.
/// </para>
/// </remarks>
public sealed class Ww8FontTable
{
    /// <summary>Where a Word 8 entry's name starts, measured from the byte after its length.</summary>
    /// <remarks>
    /// One byte of flags, two of weight, one of character set, one of alternate-name index, ten of
    /// PANOSE and twenty-four of font signature. LibreOffice walks the same offsets a field at a time in
    /// <c>WW8Fonts::WW8Fonts</c>.
    /// </remarks>
    private const int NameOffset = 1 + 2 + 1 + 1 + 10 + 24;

    /// <summary>The shortest payload a Word 8 entry can have and still hold a name.</summary>
    /// <remarks>
    /// Forty-one bytes, which is <see cref="NameOffset"/> plus the two of a terminator. An entry shorter
    /// than this is malformed, and LibreOffice stops the walk at one for the same reason.
    /// </remarks>
    private const int MinimumPayload = NameOffset + 2;

    /// <summary>How many fonts are read before the rest are ignored.</summary>
    private const int MaxFonts = 4096;

    private readonly string[] _names;
    private readonly Dictionary<string, DeclaredFontShape> _shapes;

    private Ww8FontTable(string[] names, DeclaredFontShape[] shapes)
    {
        _names = names;

        // By name rather than by index, because that is how the resolver is asked: a run names its
        // font through sprmCRgFtc0, but by the time layout has a family it is a string and the
        // index is gone. First entry wins on a duplicate name, matching the by-name lookup the
        // DOCX table does.
        _shapes = new Dictionary<string, DeclaredFontShape>(StringComparer.OrdinalIgnoreCase);
        for (int at = 0; at < names.Length; at++) _shapes.TryAdd(names[at], shapes[at]);
    }

    /// <summary>An empty table, for a document that declares none.</summary>
    public static Ww8FontTable Empty { get; } = new([], []);

    /// <summary>How many fonts the table holds.</summary>
    public int Count => _names.Length;

    /// <summary>
    /// The shape the document declares for a family, or the default when it names no such family.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The DOC counterpart of <c>WordFontTable.ShapeOf</c> for the reading: an <c>FFN</c>'s first
    /// byte packs the pitch in its low two bits and the font family in bits 4-6, which are the
    /// <c>prq</c> and <c>ff</c> fields LibreOffice reads at the top of <c>WW8Fonts::WW8Fonts</c>.
    /// </para>
    /// <para>
    /// <strong>It is not its counterpart for the meaning, and the sentence that stood here — "the
    /// other family codes leave LibreOffice's answer unchanged" — is measurably false.</strong>
    /// <c>GetFontParams</c> maps <c>ff</c> 0, 6 and 7 onto <c>FAMILY_DONTKNOW</c> and
    /// <c>SetNewFontAttr</c> <em>sets</em> that on the item, where the DOCX filter would have left
    /// an inherited value; a <c>DONTKNOW</c> family appends no generic to the fontconfig pre-match,
    /// so the answer is fontconfig's own rather than the roman default. Nine flat-ODF fixtures
    /// exported to Word 97 and back (<c>probes/words-r55/doc-family-code.py</c>) say only
    /// <c>roman</c> draws DejaVu Serif; <c>modern</c>, <c>decorative</c> and no code at all all draw
    /// DejaVu Sans. The claim came from round 54's DOCX round trip, which could not produce an
    /// undeclared <c>FFN</c> at all — the DOCX import applied the roman default before the export
    /// ran. <see cref="XRay.Office.WordProcessing.Layout.WordFallbackClass.ForWw8Font"/> is where the
    /// difference is acted on.
    /// </para>
    /// </remarks>
    public DeclaredFontShape ShapeOf(string? name)
    {
        if (name is null) return default;

        DeclaredFontShape shape =
            _shapes.TryGetValue(name, out DeclaredFontShape found) ? found : default;

        return NameOverride(name) is { } forced ? shape with { Class = forced } : shape;
    }

    /// <summary>
    /// The class a family's <em>name</em> forces regardless of its <c>FFN</c>, or null for the rest.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A DOC-only rule with no counterpart in the DOCX filter, and a real one:
    /// <c>SwWW8ImplReader::GetFontParams</c> (<c>sw/source/filter/ww8/ww8par6.cxx</c>:3767) overrides
    /// the code it has just read for fourteen name prefixes, because — in its own words — the code
    /// "might be set wrong when Doc was not created by Winword but by third party program".
    /// </para>
    /// <para>
    /// Measured rather than transcribed, on 26.2.4.2
    /// (<c>probes/words-r55/doc-family-code.py</c>): a flat ODF file declaring no generic, exported
    /// to Word 97 and back, draws <c>Garamond</c> in DejaVu <b>Serif</b> and the otherwise identical
    /// <c>Aptos</c> in DejaVu <b>Sans</b>; <c>Univers</c> and <c>Helvetica</c> both draw Sans.
    /// </para>
    /// </remarks>
    private static FontFamilyClass? NameOverride(string name)
    {
        foreach (string prefix in RomanPrefixes)
        {
            if (name.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
            {
                return FontFamilyClass.Serif;
            }
        }

        foreach (string prefix in SwissPrefixes)
        {
            if (name.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
            {
                return FontFamilyClass.SansSerif;
            }
        }

        return null;
    }

    /// <summary>The seven prefixes <c>GetFontParams</c> forces to <c>FAMILY_ROMAN</c>.</summary>
    private static readonly string[] RomanPrefixes =
    [
        "Tms Rmn", "Timmons", "CG Times", "MS Serif", "Garamond", "Times Roman", "Times New Roman",
    ];

    /// <summary>The seven it forces to <c>FAMILY_SWISS</c>.</summary>
    /// <remarks>
    /// <c>Helv</c> is a prefix and not a name: it catches <c>Helvetica</c> and <c>Helv</c> alike,
    /// which is why <c>Helvetica</c> answers DejaVu Sans through this filter and DejaVu Serif through
    /// the DOCX one.
    /// </remarks>
    private static readonly string[] SwissPrefixes =
    [
        "Helv", "Arial", "Univers", "LinePrinter", "Lucida Sans", "Small Fonts", "MS Sans Serif",
    ];

    /// <summary>
    /// The family name at an index, or null when the table has no such entry.
    /// </summary>
    /// <remarks>
    /// Only the primary name. An entry may carry an alternate to fall back to, which is the document's
    /// own substitution suggestion — X-Ray.Office resolves substitutions through LibreOffice's table
    /// instead, so honouring this one would introduce a second and different answer.
    /// </remarks>
    public string? Name(int index)
        => index >= 0 && index < _names.Length ? _names[index] : null;

    /// <summary>
    /// How much of the header comes before the first entry.
    /// </summary>
    /// <remarks>
    /// Four bytes, not two. The first two are a count that duplicates what the FIB already states, and
    /// the two after them are the string table's extra-data length — which for this table is always zero
    /// and is easy to miss, since LibreOffice writes it as a bare <c>rSt.SeekRel(2)</c> after reading the
    /// count. Starting two bytes early reads the count's high half as an entry length and finds no fonts
    /// at all, which then shows up as every paragraph being laid out in a substituted face.
    /// </remarks>
    private const int HeaderLength = 4;

    /// <summary>
    /// Parses the table from the <c>SttbfFfn</c>'s bytes.
    /// </summary>
    /// <remarks>
    /// The declared count is skipped rather than trusted — a producer that disagrees with itself is
    /// common, and the walk is bounded by the bytes either way.
    /// </remarks>
    public static Ww8FontTable Parse(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length <= HeaderLength) return Empty;

        List<string> names = [];
        List<DeclaredFontShape> shapes = [];
        int at = HeaderLength;

        while (at < bytes.Length && names.Count < MaxFonts)
        {
            int payload = bytes[at];
            at++;

            if (payload < MinimumPayload || at + payload > bytes.Length) break;

            ReadOnlySpan<byte> entry = bytes.Slice(at, payload);
            names.Add(NameIn(entry));
            shapes.Add(ShapeIn(entry));
            at += payload;
        }

        return names.Count > 0 ? new Ww8FontTable([.. names], [.. shapes]) : Empty;
    }

    /// <summary>
    /// The pitch and family packed into one entry's first byte.
    /// </summary>
    /// <remarks>
    /// <c>prq</c> in bits 0-1 — 1 is fixed, 2 is variable — and <c>ff</c> in bits 4-6, whose values
    /// are the Windows <c>FF_*</c> constants: 1 roman, 2 swiss, 3 modern, 4 script, 5 decorative.
    /// Bit 2 is <c>fTrueType</c> and bits 3 and 7 are reserved, so masking matters: reading the
    /// whole byte as a family finds one on nearly every entry.
    /// </remarks>
    private static DeclaredFontShape ShapeIn(ReadOnlySpan<byte> payload)
    {
        byte flags = payload[0];

        FontFamilyClass kind = ((flags >> 4) & 0x07) switch
        {
            1 => FontFamilyClass.Serif,
            2 => FontFamilyClass.SansSerif,
            _ => FontFamilyClass.Unknown,
        };

        FontPitch pitch = (flags & 0x03) switch
        {
            1 => FontPitch.Fixed,
            2 => FontPitch.Variable,
            _ => FontPitch.Unknown,
        };

        return new DeclaredFontShape(kind, pitch);
    }

    /// <summary>
    /// The name inside one entry's payload.
    /// </summary>
    /// <remarks>
    /// UTF-16, terminated by a null unit rather than by the payload's end — the payload is padded, so
    /// decoding all of it appends whatever the padding happens to be.
    /// </remarks>
    private static string NameIn(ReadOnlySpan<byte> payload)
    {
        ReadOnlySpan<byte> name = payload[NameOffset..];

        int units = 0;
        while ((units * 2) + 1 < name.Length
               && BinaryPrimitives.ReadUInt16LittleEndian(name[(units * 2)..]) != 0)
        {
            units++;
        }

        return units == 0 ? string.Empty : Encoding.Unicode.GetString(name[..(units * 2)]);
    }
}
