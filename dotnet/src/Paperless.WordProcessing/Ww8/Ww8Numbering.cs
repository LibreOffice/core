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
public readonly record struct Ww8ListLevel(
    int StartAt,
    byte NumberFormat,
    string NumberText,
    ReadOnlyMemory<byte> PlaceholderOffsets,
    bool IsLegalNumbering,
    bool NeverRestarts,
    byte RestartLimit)
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
    /// The two grpprls are skipped rather than kept: they hold the label's indent and its character
    /// formatting, which extraction does not use but which must still be stepped over exactly, since
    /// the template follows them.
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

        byte characterPropertiesLength = header[24];
        byte paragraphPropertiesLength = header[25];
        byte restartLimit = header[26];

        // PAPX first, then CHPX: the order matters only because the template comes after both.
        position += paragraphPropertiesLength + characterPropertiesLength;
        if (position < 0 || position + 2 > stream.Length) return false;

        int characters = BinaryPrimitives.ReadUInt16LittleEndian(stream[position..]);
        position += 2;

        int bytes = characters * 2;
        if (bytes < 0 || position + bytes > stream.Length) return false;

        string numberText = Encoding.Unicode.GetString(stream.Slice(position, bytes));
        position += bytes;

        level = new Ww8ListLevel(
            startAt, numberFormat, numberText, placeholders, isLegal, neverRestarts, restartLimit);
        return true;
    }

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
