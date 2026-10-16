using System.Text;
using System.Xml.Linq;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The workbook's shared string table: the strings <c>&lt;c t="s"&gt;</c> cells index into.
/// </summary>
/// <remarks>
/// Excel pools every distinct string in the workbook here and stores only the index in the cell,
/// which is why a sheet full of text can hold no text at all. Rich text is flattened for
/// extraction: a string split into runs by formatting is one string to a reader, and its runs are
/// kept beside the text — as character offsets rather than as formats — for whoever draws it.
/// </remarks>
public sealed class XlsxSharedStrings
{
    private readonly List<string> _strings = [];
    private readonly Dictionary<int, IReadOnlyList<XlsxRichRun>> _runs = [];
    private readonly Dictionary<int, string> _stored = [];

    private XlsxSharedStrings()
    {
    }

    /// <summary>How many strings the table holds.</summary>
    public int Count => _strings.Count;

    /// <summary>
    /// The string at an index, or null when the index is outside the table.
    /// </summary>
    /// <remarks>
    /// Null rather than an exception: an index past the end means the table and the sheet
    /// disagree, and losing one cell is better than losing the workbook.
    /// </remarks>
    public string? this[int index]
        => index >= 0 && index < _strings.Count ? _strings[index] : null;

    /// <summary>An empty table, for a workbook with no shared strings part.</summary>
    public static XlsxSharedStrings Empty { get; } = new();

    /// <summary>Reads an <c>sst</c> root.</summary>
    public static XlsxSharedStrings Read(XElement? root)
    {
        XlsxSharedStrings table = new();
        if (root is null) return table;

        foreach (XElement item in Xlsx.Children(root, "si"))
        {
            // The runs are recorded only for the strings that have any, so a workbook whose text
            // is all one format carries an empty dictionary rather than one entry per string.
            if (XlsxRichRuns.Read(item) is { } runs) table._runs[table._strings.Count] = runs;

            // The stored spelling is recorded only where it differs from the drawn one, which on
            // every corpus workbook but a handful of strings is nowhere: a table runs to tens of
            // thousands of entries and holding a second copy of each would double it for nothing.
            string drawn = ReadRichString(item, out string stored);
            if (!ReferenceEquals(drawn, stored)
                && !string.Equals(drawn, stored, StringComparison.Ordinal))
            {
                table._stored[table._strings.Count] = stored;
            }

            table._strings.Add(drawn);
        }

        return table;
    }

    /// <summary>
    /// The formatting runs of the string at an index, or null when it is all one format.
    /// </summary>
    /// <param name="index">The shared string index a cell states.</param>
    internal IReadOnlyList<XlsxRichRun>? RunsAt(int index) => _runs.GetValueOrDefault(index);

    /// <summary>
    /// The string at an index as Calc <em>stores</em> it, which is not always the string it
    /// draws.
    /// </summary>
    /// <remarks>
    /// <see cref="XlsxCellText.Stored"/> says what the difference is and why a conditional
    /// format has to compare this one. Null on the same terms as the indexer.
    /// </remarks>
    /// <param name="index">The shared string index a cell states.</param>
    internal string? StoredAt(int index)
        => _stored.TryGetValue(index, out string? stored) ? stored : this[index];

    /// <summary>
    /// Flattens an <c>si</c>, <c>is</c> or comment <c>text</c> element to plain text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Three shapes appear: a bare <c>t</c>, a sequence of <c>r</c> runs each with their own
    /// <c>t</c>, and either of those followed by <c>rPh</c> phonetic guides. The guides are
    /// dropped — they are furigana shown above the text, not part of it, and concatenating them
    /// interleaves a reading into the middle of a word.
    /// </para>
    /// <para>
    /// Whitespace is never trimmed. <c>xml:space="preserve"</c> is how SpreadsheetML says a
    /// leading space is real, and it is on nearly every <c>t</c> LibreOffice writes.
    /// </para>
    /// <para>
    /// Every <c>t</c> goes through <see cref="XlsxCellText.Of"/> rather than being appended raw,
    /// because a <c>t</c> is <c>ST_Xstring</c>: its <c>_xHHHH_</c> escapes are text the producer
    /// could not spell any other way, and the seven characters of <c>_x000D_</c> are a carriage
    /// return that Calc draws as nothing at all.
    /// </para>
    /// </remarks>
    public static string ReadRichString(XElement? element) => ReadRichString(element, out _);

    /// <summary>
    /// The same flattening, in both of the spellings the cell has at once.
    /// </summary>
    /// <remarks>
    /// One walk rather than two, because <c>Drawn(Stored(x))</c> is <c>Of(x)</c>: each run is
    /// decoded once and normalised once, which is what the drawn spelling alone already cost.
    /// The two are the same instance whenever the string holds nothing to normalise, which is
    /// nearly every string in the corpus.
    /// </remarks>
    /// <param name="element">The <c>si</c>, <c>is</c> or comment <c>text</c> element.</param>
    /// <param name="stored">
    /// Receives the string Calc stores — <see cref="XlsxCellText.Stored"/>, which is what a
    /// conditional format compares. The return value is the one it draws.
    /// </param>
    internal static string ReadRichString(XElement? element, out string stored)
    {
        if (element is null)
        {
            stored = string.Empty;
            return string.Empty;
        }

        StringBuilder drawn = new();
        StringBuilder? raw = null;

        void Append(string? value)
        {
            string kept = XlsxCellText.Stored(value);
            string shown = XlsxCellText.Drawn(kept);

            // The second builder is started only once a run has parted company with itself, and
            // it then has to catch up on everything already appended — which is the same text in
            // both, or the two would have parted earlier.
            if (raw is null && !ReferenceEquals(kept, shown)) raw = new StringBuilder(drawn.ToString());

            drawn.Append(shown);
            raw?.Append(kept);
        }

        foreach (XElement child in element.Elements())
        {
            if (Xlsx.Is(child, "t"))
            {
                Append(child.Value);
            }
            else if (Xlsx.Is(child, "r"))
            {
                foreach (XElement run in Xlsx.Children(child, "t")) Append(run.Value);
            }
        }

        string text = drawn.ToString();
        stored = raw is null ? text : raw.ToString();
        return text;
    }
}
