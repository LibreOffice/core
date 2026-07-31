using System.Text;
using System.Xml.Linq;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// The workbook's shared string table: the strings <c>&lt;c t="s"&gt;</c> cells index into.
/// </summary>
/// <remarks>
/// Excel pools every distinct string in the workbook here and stores only the index in the cell,
/// which is why a sheet full of text can hold no text at all. Rich text is flattened: a string
/// split into runs by formatting is one string to a reader, and extraction discards the
/// formatting anyway.
/// </remarks>
public sealed class XlsxSharedStrings
{
    private readonly List<string> _strings = [];

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
            table._strings.Add(ReadRichString(item));

        return table;
    }

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
    /// </remarks>
    public static string ReadRichString(XElement? element)
    {
        if (element is null) return string.Empty;

        StringBuilder text = new();
        foreach (XElement child in element.Elements())
        {
            if (Xlsx.Is(child, "t"))
            {
                text.Append(child.Value);
            }
            else if (Xlsx.Is(child, "r"))
            {
                foreach (XElement run in Xlsx.Children(child, "t")) text.Append(run.Value);
            }
        }
        return text.ToString();
    }
}
