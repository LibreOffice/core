using System.Buffers.Binary;
using System.Text;
using Paperless.Core.Numbering;

namespace Paperless.WordProcessing.Ww8;

/// <summary>One level of a WW8 list definition: an <c>LVL</c>.</summary>
/// <param name="StartAt">The value the first item at this level takes.</param>
/// <param name="NumberFormat">
/// The <c>nfc</c> code saying how the counter is rendered — decimal, roman, a bullet, or nothing.
/// </param>
/// <param name="NumberText">
/// The label template, in which certain characters are placeholders for a level's counter rather
/// than literal text. For a bullet level this is the bullet character itself.
/// </param>
/// <param name="PlaceholderOffsets">
/// For each level, the one-based character offset in <paramref name="NumberText"/> that stands for
/// that level's counter, or zero when the template does not include it. This is how WW8 spells
/// "%1.%2": the template holds two placeholder characters and this array says which level each one
/// belongs to.
/// </param>
/// <param name="IsLegalNumbering">
/// Render every component of the label in decimal whatever each level's own format says.
/// </param>
/// <param name="NeverRestarts">True when a shallower level advancing does not reset this one.</param>
/// <param name="RestartLimit">
/// The level whose advance restarts this one; deeper levels than this do not.
/// </param>
/// <param name="IndentAt">
/// The level's own left indent in twips, from <c>sprmPDxaLeft</c> in its <c>grpprlPapx</c>.
/// </param>
/// <param name="FirstLineIndent">
/// Its first-line indent in twips, negative for the hanging indent a list usually asks for.
/// </param>
/// <param name="TabPosition">
/// Where the tab after the label lands, in twips, or nought when the level states none.
/// </param>
/// <param name="Follow">
/// <c>ixchFollow</c>: 0 a tab to <paramref name="TabPosition"/>, 1 a space, 2 nothing at all.
/// </param>
/// <param name="HalfPointSize">
/// The size the label is set at, in half-points, from <c>sprmCHps</c> in the level's
/// <c>grpprlChpx</c>, or nought when the level states none.
/// </param>
public readonly record struct Ww8ListLevel(
    int StartAt,
    byte NumberFormat,
    string NumberText,
    ReadOnlyMemory<byte> PlaceholderOffsets,
    bool IsLegalNumbering,
    bool NeverRestarts,
    byte RestartLimit,
    int IndentAt = 0,
    int FirstLineIndent = 0,
    int TabPosition = 0,
    byte Follow = 0,
    int HalfPointSize = 0)
{
    /// <summary>The <c>nfc</c> meaning "this level draws a bullet, not a number".</summary>
    public const byte BulletFormat = 23;

    /// <summary>The <c>nfc</c> meaning "this level draws no label at all".</summary>
    public const byte NoneFormat = 255;

    /// <summary>True when the level draws a bullet rather than a counter.</summary>
    public bool IsBullet => NumberFormat == BulletFormat;
}

/// <summary>One WW8 list definition: an <c>LSTF</c> and the levels that follow it.</summary>
/// <param name="ListIdentifier">
/// The <c>lsid</c>, which is how an override finds this definition. It is an arbitrary number
/// rather than an index, so the definitions have to be looked up by it.
/// </param>
/// <param name="IsSimple">
/// True when the list has one level rather than nine. This changes how many <c>LVL</c>s follow it in
/// the stream, so misreading it desynchronises every later list's levels.
/// </param>
/// <param name="StyleIndexes">
/// For each level, the paragraph style the level is linked to, or 4095 for none. This is how
/// outline-numbered headings attach: the list names <c>Heading 1</c> rather than each heading naming
/// the list.
/// </param>
/// <param name="Levels">The level definitions, shallowest first.</param>
public sealed record Ww8List(
    int ListIdentifier,
    bool IsSimple,
    ReadOnlyMemory<ushort> StyleIndexes,
    IReadOnlyList<Ww8ListLevel> Levels);

/// <summary>
/// The numbering a WW8 document declares, and the live counters that turn it into labels.
/// </summary>
/// <remarks>
/// <para>
/// WW8 splits list definition from list instance exactly as DOCX later would, but through two
/// tables rather than one. The <c>PlcfLst</c> holds the definitions, each identified by an arbitrary
/// <c>lsid</c>; the <c>PlfLfo</c> holds the instances — "list format overrides" — each naming an
/// <c>lsid</c> and optionally replacing a level or its start value. A paragraph names an
/// <em>instance</em> by its one-based position in the override table, never a definition. Two
/// instances over one definition count independently, so collapsing them makes every list in a
/// document continue the previous one's numbering.
/// </para>
/// <para>
/// The label itself is nowhere in the file: WW8 stores the template and the reader counts. That is
/// why numbering has to be advanced by walking paragraphs in order rather than resolved per
/// paragraph (<c>research/02-writer.md</c> section C.2).
/// </para>
/// </remarks>
public sealed class Ww8Numbering
{
    /// <summary>The number of levels a non-simple list has. Fixed by the format.</summary>
    public const int LevelCount = 9;

    /// <summary>How many list definitions or instances are read before the rest are ignored.</summary>
    public const int MaxLists = 4096;

    private const int ListDefinitionSize = 28;
    private const int LevelHeaderSize = 28;
    private const int OverrideSize = 16;

    private readonly Dictionary<int, Ww8List> _lists = [];
    private readonly List<Ww8Override> _overrides = [];

    // Live counters, keyed by list instance and level: two instances of one definition must not
    // share a count.
    private readonly Dictionary<(int Instance, int Level), int> _counters = [];

    /// <summary>The list definitions, by their <c>lsid</c>.</summary>
    public IReadOnlyDictionary<int, Ww8List> Lists => _lists;

    /// <summary>How many list instances the document declares.</summary>
    public int InstanceCount => _overrides.Count;

    /// <summary>An empty numbering, for a document that declares no lists.</summary>
    public static Ww8Numbering Empty { get; } = new();

    /// <summary>
    /// Parses the two numbering tables.
    /// </summary>
    /// <param name="listDefinitions">The <c>PlcfLst</c>: the definitions and their levels.</param>
    /// <param name="listOverrides">The <c>PlfLfo</c>: the instances and their overrides.</param>
    public static Ww8Numbering Parse(
        ReadOnlySpan<byte> listDefinitions, ReadOnlySpan<byte> listOverrides)
    {
        Ww8Numbering numbering = new();
        numbering.ParseDefinitions(listDefinitions);
        numbering.ParseOverrides(listOverrides);
        return numbering;
    }

    /// <summary>
    /// Advances the counters for a paragraph and returns its rendered label.
    /// </summary>
    /// <param name="instance">
    /// The paragraph's <c>ilfo</c>: a one-based index into the override table, where zero means the
    /// paragraph is not in a list. That is how a continuation paragraph inside a list item is
    /// written, so zero must not be treated as "the first list".
    /// </param>
    /// <param name="level">The paragraph's zero-based level.</param>
    /// <returns>The label as it would be drawn, or null when this level draws none.</returns>
    public string? Advance(int instance, int level)
    {
        if (!Resolve(instance, level, out Ww8List? list, out Ww8ListLevel definition)) return null;
        if (definition.NumberFormat == Ww8ListLevel.NoneFormat) return null;

        int start = StartOf(instance, level, definition);
        int current = _counters.TryGetValue((instance, level), out int existing) ? existing + 1 : start;
        _counters[(instance, level)] = current;

        // A shallower level advancing restarts everything under it — that is what makes a sub-list
        // begin at one again rather than continuing the previous sub-list.
        for (int deeper = level + 1; deeper < LevelCount; deeper++)
        {
            if (!Resolve(instance, deeper, out _, out Ww8ListLevel deeperDefinition)) continue;
            if (deeperDefinition.NeverRestarts) continue;
            if (deeperDefinition.RestartLimit != 0 && level >= deeperDefinition.RestartLimit) continue;
            _counters.Remove((instance, deeper));
        }

        return FormatLabel(instance, list!, definition);
    }

    /// <summary>Resets every counter, so a second flow does not continue the body's numbering.</summary>
    public void ResetCounters() => _counters.Clear();

    /// <summary>
    /// The level definition a paragraph's list reference names, or null when it names none.
    /// </summary>
    /// <remarks>
    /// Advances nothing, unlike <see cref="Advance"/>: the caller wants the level's geometry rather than
    /// its next label, and asking for the label twice would number every item twice over.
    /// </remarks>
    /// <param name="instance">The paragraph's <c>ilfo</c>.</param>
    /// <param name="level">Its zero-based level.</param>
    public Ww8ListLevel? FindLevel(int instance, int level)
        => Resolve(instance, level, out _, out Ww8ListLevel definition) ? definition : null;

    /// <summary>
    /// The level definition in force for a list instance, or false when there is none.
    /// </summary>
    /// <remarks>
    /// A level the instance overrides wins over the definition's own. A simple list has only level
    /// zero however deep a paragraph claims to be, which real documents do rely on.
    /// </remarks>
    private bool Resolve(int instance, int level, out Ww8List? list, out Ww8ListLevel definition)
    {
        list = null;
        definition = default;

        if (instance <= 0 || instance > _overrides.Count) return false;
        if (level is < 0 or >= LevelCount) return false;

        Ww8Override over = _overrides[instance - 1];
        if (!_lists.TryGetValue(over.ListIdentifier, out Ww8List? found)) return false;
        list = found;

        if (over.Levels.TryGetValue(level, out Ww8OverrideLevel replacement)
            && replacement.Replacement is { } replaced)
        {
            definition = replaced;
            return true;
        }

        int index = found.IsSimple ? 0 : level;
        if (index >= found.Levels.Count) return false;

        definition = found.Levels[index];
        return true;
    }

    private int StartOf(int instance, int level, Ww8ListLevel definition)
        => instance >= 1 && instance <= _overrides.Count
            && _overrides[instance - 1].Levels.TryGetValue(level, out Ww8OverrideLevel over)
            && over.StartAt is { } overridden
            ? overridden
            : definition.StartAt;

    /// <summary>
    /// Renders a level's label from the current counters.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The template is not a printf string: it is ordinary text in which particular
    /// <em>character positions</em> hold placeholders, and
    /// <see cref="Ww8ListLevel.PlaceholderOffsets"/> says which positions those are. So the template
    /// has to be walked with the offsets in hand rather than scanned for an escape sequence — which is
    /// also why a bullet level needs no substitution: it has no placeholders, only a character.
    /// </para>
    /// <para>
    /// Which level a placeholder stands for comes from the <em>character value</em> at that position,
    /// not from the placeholder's index in the offset array: producers pack the offsets contiguously
    /// from zero rather than indexing them by level, so a level-2 definition showing "%2.%3" writes
    /// its offsets in slots 0 and 1. Reading the level from the slot instead renders the wrong
    /// counters — LibreOffice's own importer reads the character
    /// (<c>sw/source/filter/ww8/ww8par3.cxx</c>, <c>WW8ListManager::AdjustLVL</c>).
    /// </para>
    /// </remarks>
    private string FormatLabel(int instance, Ww8List list, Ww8ListLevel definition)
    {
        if (definition.IsBullet) return OutlineNumbers.NormaliseBullet(definition.NumberText);
        if (definition.NumberText.Length == 0) return string.Empty;

        HashSet<int> placeholderOffsets = [];
        ReadOnlySpan<byte> offsets = definition.PlaceholderOffsets.Span;
        for (int slot = 0; slot < offsets.Length && slot < LevelCount; slot++)
        {
            // A zero ends the array: the offsets are a packed list, not one entry per level.
            if (offsets[slot] == 0) break;
            placeholderOffsets.Add(offsets[slot] - 1);
        }

        StringBuilder label = new(definition.NumberText.Length + 8);
        for (int i = 0; i < definition.NumberText.Length; i++)
        {
            int component = definition.NumberText[i];
            if (!placeholderOffsets.Contains(i) || component is < 0 or >= LevelCount)
            {
                label.Append(definition.NumberText[i]);
                continue;
            }

            byte format = definition.IsLegalNumbering
                ? (byte)0
                : ComponentFormat(instance, list, component);

            int value = _counters.TryGetValue((instance, component), out int counter)
                ? counter
                : ComponentStart(instance, list, component);

            label.Append(FormatNumber(value, format));
        }

        return label.ToString();
    }

    private byte ComponentFormat(int instance, Ww8List list, int component)
        => Resolve(instance, component, out _, out Ww8ListLevel level)
            ? level.NumberFormat
            : component < list.Levels.Count ? list.Levels[component].NumberFormat : (byte)0;

    private int ComponentStart(int instance, Ww8List list, int component)
        => Resolve(instance, component, out _, out Ww8ListLevel level)
            ? StartOf(instance, component, level)
            : component < list.Levels.Count ? list.Levels[component].StartAt : 1;

    /// <summary>
    /// Formats one counter value in a WW8 number format code.
    /// </summary>
    /// <remarks>
    /// An unrecognised code falls back to decimal rather than rendering nothing: the code list is
    /// long and mostly locale-specific — Chinese, Hebrew and Thai numerals among them — and a missing
    /// label loses more than a differently-shaped one.
    /// </remarks>
    public static string FormatNumber(int value, byte format) => format switch
    {
        1 => OutlineNumbers.Roman(value, upperCase: true),
        2 => OutlineNumbers.Roman(value, upperCase: false),
        // Word repeats the letter past twenty-six — AA, BB — rather than counting in bijective
        // base 26.
        3 => OutlineNumbers.Alphabetic(value, upperCase: true, synchronised: true),
        4 => OutlineNumbers.Alphabetic(value, upperCase: false, synchronised: true),
        5 => OutlineNumbers.Ordinal(value),
        22 => OutlineNumbers.DigitsWithLeadingZero(value),
        _ => OutlineNumbers.Digits(value),
    };

    /// <summary>
    /// Reads the <c>PlcfLst</c>: a count, that many fixed-size definitions, then all their levels
    /// end to end.
    /// </summary>
    /// <remarks>
    /// The levels are not indexed. They follow the definitions in order, one or nine per definition
    /// according to its own <c>fSimpleList</c> flag, and each is variably sized — so the only way to
    /// find the levels of the third list is to read the first two lists' levels first.
    /// </remarks>
    private void ParseDefinitions(ReadOnlySpan<byte> plcfLst)
    {
        if (plcfLst.Length < 2) return;

        int count = BinaryPrimitives.ReadUInt16LittleEndian(plcfLst);
        if (count is <= 0 or > MaxLists) return;
        if (2 + (count * ListDefinitionSize) > plcfLst.Length) return;

        List<(int ListIdentifier, bool IsSimple, ushort[] StyleIndexes)> headers = new(count);
        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> definition = plcfLst.Slice(2 + (i * ListDefinitionSize), ListDefinitionSize);

            int lsid = BinaryPrimitives.ReadInt32LittleEndian(definition);

            ushort[] styles = new ushort[LevelCount];
            for (int level = 0; level < LevelCount; level++)
                styles[level] = BinaryPrimitives.ReadUInt16LittleEndian(definition[(8 + (2 * level))..]);

            bool simple = (definition[26] & 0x01) != 0;
            headers.Add((lsid, simple, styles));
        }

        int position = 2 + (count * ListDefinitionSize);
        foreach ((int lsid, bool simple, ushort[] styles) in headers)
        {
            List<Ww8ListLevel> levels = [];
            int wanted = simple ? 1 : LevelCount;
            for (int level = 0; level < wanted; level++)
            {
                if (!ReadLevel(plcfLst, ref position, out Ww8ListLevel parsed)) break;
                levels.Add(parsed);
            }

            if (levels.Count > 0) _lists[lsid] = new Ww8List(lsid, simple, styles, levels);
        }
    }

    /// <summary>
    /// Reads one <c>LVL</c>: a fixed header, two grpprls whose lengths it states, then the label
    /// template.
    /// </summary>
    /// <remarks>
    /// The paragraph grpprl is read for the three sprms that decide where the label and its item's text
    /// go — Word writes the level's geometry there rather than in the <c>LVLF</c>, and a document that
    /// leaves its list paragraphs' own <c>sprmPDxaLeft</c> unset has nowhere else to say it. The
    /// character grpprl is still only stepped over: all it usually carries is a symbol font for a bullet
    /// whose code point is normalised away anyway. Both lengths must be honoured exactly either way,
    /// since the label template follows them.
    /// </remarks>
    private static bool ReadLevel(ReadOnlySpan<byte> stream, ref int position, out Ww8ListLevel level)
    {
        level = default;
        if (position < 0 || position + LevelHeaderSize > stream.Length) return false;

        ReadOnlySpan<byte> header = stream.Slice(position, LevelHeaderSize);
        position += LevelHeaderSize;

        int startAt = BinaryPrimitives.ReadInt32LittleEndian(header);
        byte numberFormat = header[4];

        byte flags = header[5];
        bool isLegal = (flags & 0x04) != 0;
        bool neverRestarts = (flags & 0x08) != 0;

        byte[] placeholders = header[6..15].ToArray();
        byte follow = header[15];

        byte characterPropertiesLength = header[24];
        byte paragraphPropertiesLength = header[25];
        byte restartLimit = header[26];

        // PAPX first, then CHPX. Both lengths must be honoured exactly either way, since the label
        // template follows them.
        if (position + paragraphPropertiesLength > stream.Length) return false;
        (int indentAt, int firstLine, int tabPosition) =
            LevelIndents(stream.Slice(position, paragraphPropertiesLength));

        position += paragraphPropertiesLength;
        if (position < 0 || position + characterPropertiesLength > stream.Length) return false;
        int halfPointSize = LevelSize(stream.Slice(position, characterPropertiesLength));

        position += characterPropertiesLength;
        if (position < 0 || position + 2 > stream.Length) return false;

        int characters = BinaryPrimitives.ReadUInt16LittleEndian(stream[position..]);
        position += 2;

        int bytes = characters * 2;
        if (bytes < 0 || position + bytes > stream.Length) return false;

        string numberText = Encoding.Unicode.GetString(stream.Slice(position, bytes));
        position += bytes;

        level = new Ww8ListLevel(
            startAt, numberFormat, numberText, placeholders, isLegal, neverRestarts, restartLimit,
            indentAt, firstLine, tabPosition, follow, halfPointSize);
        return true;
    }

    /// <summary>
    /// The size the level sets its label at, in half-points, or nought when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The one thing worth having out of the level's <c>grpprlChpx</c>. LibreOffice reads the whole
    /// group into an item set and hangs it on the level as a character style
    /// (<c>WW8ListManager::ReadLVL</c>, <c>sw/source/filter/ww8/ww8par3.cxx</c>:820, which runs the
    /// group through <c>SubstituteBullet</c> and <c>maSprmParser</c> into <c>pLevel-&gt;maCharSet</c>);
    /// its export writes the result back as a <c>WW8NumNzM</c> character style, which is where a
    /// flat-ODF round trip shows it — <c>fo:font-size="12pt"</c> on the level of
    /// <c>loi_format_letter_of_intent-a-320-214-a330.doc</c>, whose items are set in 11.
    /// </para>
    /// <para>
    /// The face is deliberately still not taken: a bullet level names a symbol font and states its
    /// bullet as a code point in that font's private use area, which <see cref="Ww8Numbering"/> has
    /// already normalised to U+2022 — so keeping the font would draw a real bullet through a face with
    /// no glyph for it. The <em>size</em> survives that normalisation intact, because it says how big
    /// the mark is rather than which mark it is.
    /// </para>
    /// </remarks>
    private static int LevelSize(ReadOnlySpan<byte> grpprl)
    {
        byte[] copy = grpprl.ToArray();

        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(copy))
        {
            if (sprm.Identifier is not (FontSizeSprm or FontSizeSprm97)) continue;
            if (sprm.Operand.Length < 2) continue;

            int halfPoints = BinaryPrimitives.ReadUInt16LittleEndian(sprm.Operand.Span);

            // Word's own limits: 2 to 3276 half-points. Anything outside them is a misread group
            // rather than a size, and a label measured at it would be absurd.
            if (halfPoints is >= 2 and <= 3276) return halfPoints;
        }

        return 0;
    }

    /// <summary>
    /// The three measurements a level's <c>grpprlPapx</c> carries: its indent, its first line's, and
    /// where the tab after the label lands.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The port of <c>WW8ListManager::ReadLVL</c> (<c>sw/source/filter/ww8/ww8par3.cxx:700</c>), which
    /// looks for the same three sprms and in the same order — each in both its Word 97 and its later
    /// spelling, since a file saved by any version may carry either.
    /// </para>
    /// <para>
    /// The left indent is taken as a magnitude, as LibreOffice takes it: a negative
    /// <c>sprmPDxaLeft</c> here means the same distance on the other side of the margin rather than a
    /// paragraph hanging off the page. The first-line indent keeps its sign, because that is the
    /// hanging indent the label lives in.
    /// </para>
    /// <para>
    /// The tab is <c>sprmPChgTabsPapx</c> in the one shape a level ever writes it: delete nothing,
    /// insert one stop. Anything else is a document doing something a list level has no way to mean,
    /// and is left alone rather than guessed at — which is the same judgement the C++ makes, where the
    /// other shapes trip an assertion and are ignored.
    /// </para>
    /// </remarks>
    private static (int IndentAt, int FirstLine, int TabPosition) LevelIndents(
        ReadOnlySpan<byte> grpprl)
    {
        int indentAt = 0;
        int firstLine = 0;
        int tabPosition = 0;

        byte[] copy = grpprl.ToArray();

        foreach (Ww8Sprm sprm in Ww8SprmReader.Read(copy))
        {
            switch (sprm.Identifier)
            {
                case LeftIndentSprm or LeftIndentSprm97:
                    indentAt = Math.Abs((int)sprm.SignedWord);
                    break;

                case FirstLineIndentSprm or FirstLineIndentSprm97:
                    firstLine = sprm.SignedWord;
                    break;

                case TabsSprm:
                {
                    ReadOnlySpan<byte> operand = sprm.Operand.Span;
                    if (operand.Length >= 5 && operand[0] == 0 && operand[1] == 1)
                    {
                        tabPosition = BinaryPrimitives.ReadInt16LittleEndian(operand[2..]);
                    }

                    break;
                }

                default:
                    continue;
            }
        }

        return (indentAt, firstLine, tabPosition);
    }

    /// <summary><c>sprmCHps</c>, a font size in half-points, as Word 97 wrote it.</summary>
    private const ushort FontSizeSprm97 = 0x4A43;

    /// <summary><c>sprmCHps</c> in its earlier spelling, which a Word 6/95 level may carry.</summary>
    private const ushort FontSizeSprm = 0x0043;

    /// <summary><c>sprmPDxaLeft</c> as Word 97 wrote it.</summary>
    private const ushort LeftIndentSprm97 = 0x840F;

    /// <summary><c>sprmPDxaLeft</c> in its later spelling.</summary>
    private const ushort LeftIndentSprm = 0x845E;

    /// <summary><c>sprmPDxaLeft1</c> as Word 97 wrote it.</summary>
    private const ushort FirstLineIndentSprm97 = 0x8411;

    /// <summary><c>sprmPDxaLeft1</c> in its later spelling.</summary>
    private const ushort FirstLineIndentSprm = 0x8460;

    /// <summary>
    /// <c>sprmPChgTabs</c>, whose operand states its own length.
    /// </summary>
    /// <remarks>
    /// This one and not <c>sprmPChgTabsPapx</c> beside it, which has a different operand layout — the
    /// same choice <c>ReadLVL</c> makes.
    /// </remarks>
    private const ushort TabsSprm = 0xC615;

    /// <summary>
    /// Reads the <c>PlfLfo</c>: a count, that many fixed-size instances, then their override data.
    /// </summary>
    /// <remarks>
    /// The instances and their data are two separate arrays, and the second is variably sized in a
    /// way the first describes — each instance says how many level overrides follow, and each
    /// override says whether a whole replacement level follows it. So an instance's overrides can
    /// only be found by walking every earlier instance's.
    /// </remarks>
    private void ParseOverrides(ReadOnlySpan<byte> plfLfo)
    {
        if (plfLfo.Length < 4) return;

        int count = BinaryPrimitives.ReadInt32LittleEndian(plfLfo);
        if (count is <= 0 or > MaxLists) return;
        if (4 + (count * OverrideSize) > plfLfo.Length) return;

        int[] levelCounts = new int[count];
        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> record = plfLfo.Slice(4 + (i * OverrideSize), OverrideSize);
            _overrides.Add(new Ww8Override(BinaryPrimitives.ReadInt32LittleEndian(record)));
            levelCounts[i] = record[12];
        }

        int position = 4 + (count * OverrideSize);
        for (int i = 0; i < count; i++)
        {
            // Each instance's data begins with a four-byte field Paperless does not use, but which
            // still has to be stepped over.
            if (position + 4 > plfLfo.Length) return;
            position += 4;

            for (int j = 0; j < levelCounts[i]; j++)
            {
                if (!ReadOverrideLevel(plfLfo, ref position, out int level, out Ww8OverrideLevel parsed))
                    return;
                _overrides[i].Levels[level] = parsed;
            }
        }
    }

    /// <summary>Reads one <c>LFOLVL</c>, and the replacement level that may follow it.</summary>
    private static bool ReadOverrideLevel(
        ReadOnlySpan<byte> stream, ref int position, out int level, out Ww8OverrideLevel over)
    {
        level = 0;
        over = default;

        if (position < 0 || position + 8 > stream.Length) return false;

        int startAt = BinaryPrimitives.ReadInt32LittleEndian(stream[position..]);
        byte flags = stream[position + 4];
        position += 8;

        level = flags & 0x0F;
        bool overridesStart = (flags & 0x10) != 0;
        bool overridesFormatting = (flags & 0x20) != 0;

        Ww8ListLevel? replacement = null;
        if (overridesFormatting && ReadLevel(stream, ref position, out Ww8ListLevel parsed))
            replacement = parsed;

        over = new Ww8OverrideLevel(overridesStart ? startAt : null, replacement);
        return true;
    }

    /// <summary>One list instance: an <c>LFO</c> and the level overrides that belong to it.</summary>
    private sealed class Ww8Override(int listIdentifier)
    {
        public int ListIdentifier { get; } = listIdentifier;

        public Dictionary<int, Ww8OverrideLevel> Levels { get; } = [];
    }

    /// <summary>What one instance changes about one level.</summary>
    private readonly record struct Ww8OverrideLevel(int? StartAt, Ww8ListLevel? Replacement);
}
