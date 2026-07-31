using System.Globalization;
using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Text.Encodings;

namespace Paperless.Spreadsheets.Csv;

/// <summary>
/// What a CSV file turned out to be: its encoding, its separator and its quoting.
/// </summary>
/// <remarks>
/// <para>
/// None of these are recorded anywhere. A CSV file is bytes, and every property of it is a
/// guess made from the bytes — which is why every decision this type makes is reported as a
/// <see cref="Diagnostic"/>. A mismatch against another reader is often not a bug but a
/// different guess about a genuinely ambiguous file, and that is only arguable if the guess
/// is visible.
/// </para>
/// <para>
/// The stakes differ by property. Getting the separator wrong turns a table into one wide
/// column, which is obvious. Getting the encoding wrong corrupts every non-ASCII character
/// while leaving ASCII perfect, which is not obvious at all and is exactly the failure the
/// <c>extraction-comparison</c> skill describes.
/// </para>
/// </remarks>
public sealed record CsvDialect
{
    /// <summary>The separator between fields.</summary>
    public required char Separator { get; init; }

    /// <summary>The quote character, or null when the file appears not to quote at all.</summary>
    public char? Quote { get; init; } = '"';

    /// <summary>The encoding the bytes were decoded with.</summary>
    public required Encoding Encoding { get; init; }

    /// <summary>True when a byte-order mark named the encoding outright.</summary>
    public bool EncodingFromByteOrderMark { get; init; }

    /// <summary>The separators considered, in the order they are preferred on a tie.</summary>
    /// <remarks>
    /// Comma first because the format is named after it; then semicolon, which is what a
    /// locale using the comma as its decimal separator writes; then tab, which is a TSV under
    /// another name; then the pipe and the colon, which appear in exported logs.
    /// </remarks>
    public static char[] CandidateSeparators { get; } = [',', ';', '\t', '|', ':'];

    /// <summary>
    /// Works out how to read a file, recording each decision and its reason.
    /// </summary>
    /// <param name="bytes">The whole file.</param>
    /// <param name="fallbackEncoding">
    /// What the caller wants used when the bytes are not valid UTF-8. Null means Windows-1252,
    /// which is what the overwhelming majority of legacy Western CSV files are.
    /// </param>
    /// <param name="diagnostics">Where the decisions are recorded.</param>
    public static CsvDialect Detect(
        ReadOnlySpan<byte> bytes, Encoding? fallbackEncoding, List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(diagnostics);

        (Encoding encoding, bool fromMark, string reason) = DetectEncoding(bytes, fallbackEncoding);
        diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Information, "PL2340",
            $"CSV encoding read as {encoding.WebName}: {reason}"));

        string text = Decode(bytes, encoding, fromMark);
        (char separator, string separatorReason) = DetectSeparator(text);
        diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Information, "PL2341",
            $"CSV separator taken to be {Describe(separator)}: {separatorReason}"));

        bool quoted = LooksQuoted(text, separator);
        if (!quoted)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Information, "PL2342",
                "No quoted field was found, so a double quote inside a field is treated as "
                + "ordinary text rather than as quoting."));
        }

        return new CsvDialect
        {
            Separator = separator,
            Quote = quoted ? '"' : null,
            Encoding = encoding,
            EncodingFromByteOrderMark = fromMark,
        };
    }

    /// <summary>Decodes a file with this dialect's encoding, dropping any byte-order mark.</summary>
    public string Decode(ReadOnlySpan<byte> bytes) => Decode(bytes, Encoding, EncodingFromByteOrderMark);

    private static string Decode(ReadOnlySpan<byte> bytes, Encoding encoding, bool fromMark)
    {
        int skip = fromMark ? encoding.GetPreamble().Length : 0;
        return encoding.GetString(bytes[Math.Min(skip, bytes.Length)..]);
    }

    /// <summary>
    /// Chooses an encoding: a byte-order mark if there is one, then UTF-8 if the bytes are
    /// valid UTF-8, then the fallback.
    /// </summary>
    /// <remarks>
    /// Strict UTF-8 validation is what makes this reliable rather than a coin toss. Text that
    /// is valid UTF-8 and yet meant as Windows-1252 is vanishingly rare — the multi-byte
    /// sequences would have to spell plausible Latin-1 pairs — whereas Windows-1252 text is
    /// almost always invalid UTF-8 the moment it uses an accent. Pure ASCII decodes
    /// identically either way, so the choice does not matter there.
    /// </remarks>
    private static (Encoding Encoding, bool FromMark, string Reason) DetectEncoding(
        ReadOnlySpan<byte> bytes, Encoding? fallback)
    {
        if (bytes.StartsWith((ReadOnlySpan<byte>)[0xEF, 0xBB, 0xBF]))
            return (new UTF8Encoding(false), true, "a UTF-8 byte-order mark.");
        if (bytes.StartsWith((ReadOnlySpan<byte>)[0xFF, 0xFE, 0x00, 0x00]))
            return (new UTF32Encoding(false, true), true, "a UTF-32 little-endian byte-order mark.");
        if (bytes.StartsWith((ReadOnlySpan<byte>)[0xFF, 0xFE]))
            return (new UnicodeEncoding(false, true), true, "a UTF-16 little-endian byte-order mark.");
        if (bytes.StartsWith((ReadOnlySpan<byte>)[0xFE, 0xFF]))
            return (new UnicodeEncoding(true, true), true, "a UTF-16 big-endian byte-order mark.");

        if (IsValidUtf8(bytes))
        {
            bool ascii = !bytes.ContainsAnyExceptInRange((byte)0, (byte)127);
            return (new UTF8Encoding(false), false,
                ascii
                    ? "the file is pure ASCII, which every candidate encoding agrees on."
                    : "no byte-order mark, but the bytes are valid UTF-8.");
        }

        Encoding chosen = fallback ?? LegacyCodePages.Fallback;
        return (chosen, false,
            $"the bytes are not valid UTF-8, so the {(fallback is null ? "default" : "caller's")} "
            + "single-byte encoding was used. A file in another code page will show the wrong "
            + "accented characters.");
    }

    private static bool IsValidUtf8(ReadOnlySpan<byte> bytes)
    {
        try
        {
            _ = new UTF8Encoding(false, throwOnInvalidBytes: true).GetString(bytes);
            return true;
        }
        catch (DecoderFallbackException)
        {
            return false;
        }
    }

    /// <summary>
    /// Chooses a separator by which candidate divides the file's lines most consistently.
    /// </summary>
    /// <remarks>
    /// Consistency rather than raw frequency, because prose beats punctuation on frequency:
    /// a one-column file of English sentences contains far more spaces and commas than a
    /// three-column semicolon file contains semicolons. A real separator appears the same
    /// number of times in every line, which is a much sharper signal — so a candidate whose
    /// per-line count never varies wins over one that merely occurs more often.
    /// </remarks>
    private static (char Separator, string Reason) DetectSeparator(string text)
    {
        List<string> lines = SampleLines(text);
        if (lines.Count == 0) return (',', "the file is empty, so the default was kept.");

        char best = ',';
        int bestFields = 1;
        bool bestConsistent = false;

        foreach (char candidate in CandidateSeparators)
        {
            List<int> counts = [.. lines.Select(line => CountOutsideQuotes(line, candidate))];
            if (counts.Count == 0 || counts[0] == 0) continue;

            bool consistent = counts.All(count => count == counts[0]);
            int fields = counts[0] + 1;

            // A consistent candidate always beats an inconsistent one; between two consistent
            // ones, more columns wins, since a file separated by both commas and semicolons is
            // more likely to be the one with more fields than to be one long quoted line.
            if ((consistent && !bestConsistent) || (consistent == bestConsistent && fields > bestFields))
            {
                best = candidate;
                bestFields = fields;
                bestConsistent = consistent;
            }
        }

        if (bestFields <= 1)
            return (',', "no candidate separator occurs outside quotes; the file is one column.");

        return (best, bestConsistent
            ? $"every one of the first {lines.Count} lines splits into {bestFields} fields."
            : $"it gives the most fields ({bestFields}) although the line lengths disagree, "
              + "which usually means the file is ragged rather than that the guess is wrong.");
    }

    /// <summary>The first lines of the file, ignoring line breaks inside quoted fields.</summary>
    private static List<string> SampleLines(string text)
    {
        const int maxLines = 32;
        List<string> lines = [];
        StringBuilder current = new();
        bool quoted = false;

        foreach (char c in text)
        {
            if (c == '"') quoted = !quoted;

            if (!quoted && c is '\n' or '\r')
            {
                if (current.Length > 0)
                {
                    lines.Add(current.ToString());
                    current.Clear();
                    if (lines.Count >= maxLines) return lines;
                }

                continue;
            }

            current.Append(c);
        }

        if (current.Length > 0) lines.Add(current.ToString());
        return lines;
    }

    private static int CountOutsideQuotes(string line, char candidate)
    {
        int count = 0;
        bool quoted = false;

        foreach (char c in line)
        {
            if (c == '"') quoted = !quoted;
            else if (c == candidate && !quoted) count++;
        }

        return count;
    }

    /// <summary>
    /// True when a quote character appears where quoting would put one.
    /// </summary>
    /// <remarks>
    /// A file whose fields contain apostrophes and inch marks but no quoted fields would be
    /// mangled by treating <c>"</c> as a quote — the parser would swallow separators until
    /// the next one. Requiring a quote at the very start of a field is what tells the two
    /// apart.
    /// </remarks>
    private static bool LooksQuoted(string text, char separator)
    {
        bool atFieldStart = true;

        foreach (char c in text)
        {
            if (c == '"' && atFieldStart) return true;
            atFieldStart = c == separator || c is '\n' or '\r';
        }

        return false;
    }

    private static string Describe(char separator) => separator switch
    {
        '\t' => "a tab",
        ',' => "a comma",
        ';' => "a semicolon",
        '|' => "a pipe",
        ':' => "a colon",
        _ => "'" + separator.ToString(CultureInfo.InvariantCulture) + "'",
    };
}
