using System.Text;

namespace XRay.Office.Core.Globalization;

/// <summary>
/// One Hunspell hyphenation pattern file, applied by Liang's algorithm.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The format.</strong> A <c>hyph_*.dic</c> is a plain text file: an encoding name on the
/// first line, then keyword lines, then one TeX-style pattern per line. A pattern is letters with
/// digits interleaved between them — <c>hy3ph</c> says "value 3 between <c>y</c> and <c>p</c> in
/// any word containing <c>hyph</c>" — and a leading or trailing <c>.</c> anchors the pattern to
/// the start or the end of the word. Liang's algorithm lays every matching pattern over the word
/// and keeps the largest value at each position; an odd value is a hyphenation point and an even
/// one forbids it. Lines beginning with <c>%</c> are comments.
/// </para>
/// <para>
/// <strong>The minima are read out of the file, not written down here.</strong>
/// <c>LEFTHYPHENMIN</c> and <c>RIGHTHYPHENMIN</c> say how many characters must remain on each
/// side of the break; <c>hyph_en_US.dic</c> states 2 and 3, <c>hyph_fr.dic</c> 2 and 2. They are
/// floored at 2 because that is what LibreOffice does with them:
/// <c>lingucomponent/source/hyphenator/hyphen/hyphenimp.cxx</c>:435 passes
/// <c>std::max&lt;sal_Int16&gt;(dict-&gt;lhmin, 2)</c> into libhyphen, and the linguistic
/// property that can raise them further defaults to 2 as well
/// (<c>linguistic/source/lngprophelp.cxx</c>:492-493). <strong>Read out of this tree, which is
/// 27.2 alpha rather than the 26.2.4.2 reference binary; the behaviour is confirmed
/// independently against that binary in <c>probes/hyphen-r106</c>.</strong>
/// </para>
/// <para>
/// <strong>What this deliberately does not implement.</strong> Non-standard hyphenation — the
/// <c>pattern/replacement,cut,pos</c> form that spells German <c>Zuk-ker</c> for <c>Zucker</c> —
/// is skipped rather than approximated, because a wrong replacement changes the characters on
/// the page and not merely where they break. None of the four pattern files LibreOffice 26.2.4.2
/// ships uses it. <c>NEXTLEVEL</c> splits a file into a compound-boundary level and an ordinary
/// level; the ordinary one is the last, and that is the one kept, which is also the only one
/// <c>hyph_fr.dic</c> has anything in.
/// </para>
/// <para>
/// Two normalisations happen before matching, both of them LibreOffice's
/// (<c>hyphenimp.cxx</c>:315-330): curly quotes and apostrophes are folded to straight ones,
/// because the patterns are written with straight ones, and the word is lower-cased. The folding
/// is done character by character so the result is the same length as the input — an offset into
/// a re-cased string is not an offset into the caller's word, and a length change would silently
/// move every point after it.
/// </para>
/// </remarks>
public sealed class HyphenationPatterns : IHyphenator
{
    /// <summary>Pattern letters to the values between them; one more value than letters.</summary>
    private readonly Dictionary<string, byte[]> patterns;

    /// <summary>The longest pattern's letter count, which bounds the lookup at each position.</summary>
    private readonly int longest;

    private HyphenationPatterns(
        Dictionary<string, byte[]> patterns, int longest, int leftMinimum, int rightMinimum)
    {
        this.patterns = patterns;
        this.longest = longest;
        LeftMinimum = leftMinimum;
        RightMinimum = rightMinimum;
    }

    /// <summary>How many characters must stay on the line — the file's <c>LEFTHYPHENMIN</c>.</summary>
    public int LeftMinimum { get; }

    /// <summary>How many must move to the next — the file's <c>RIGHTHYPHENMIN</c>.</summary>
    public int RightMinimum { get; }

    /// <summary>How many patterns were read. Zero means the file said nothing.</summary>
    public int Count => patterns.Count;

    /// <summary>Reads a pattern file.</summary>
    /// <remarks>
    /// The first line names the encoding the rest is in. Only that line is guaranteed to be
    /// ASCII, so it is read as bytes and the remainder decoded with what it names: all four files
    /// LibreOffice ships say <c>UTF-8</c>, but the format allows <c>ISO8859-1</c> and its
    /// relatives and a file read as the wrong one loses exactly the accented patterns that make
    /// it worth having. An encoding this runtime does not know falls back to UTF-8 rather than
    /// throwing, because a pattern file is an optimisation and failing to read one must not stop
    /// a document rendering.
    /// </remarks>
    public static HyphenationPatterns Read(Stream patterns)
    {
        ArgumentNullException.ThrowIfNull(patterns);

        using MemoryStream buffer = new();
        patterns.CopyTo(buffer);

        return Read(buffer.GetBuffer().AsSpan(0, (int)buffer.Length));
    }

    /// <summary>Reads a pattern file's bytes.</summary>
    public static HyphenationPatterns Read(ReadOnlySpan<byte> bytes)
    {
        // The byte-order mark is not part of the encoding name, and a file that carries one reads
        // its first line as "﻿UTF-8" and falls back — silently, and only for the accented
        // patterns.
        if (bytes.Length >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
            bytes = bytes[3..];

        int end = bytes.IndexOfAny((byte)'\n', (byte)'\r');
        string name = Encoding.ASCII.GetString(end < 0 ? bytes : bytes[..end]).Trim();

        return Parse(Decode(name).GetString(bytes));
    }

    /// <summary>Reads a pattern file from disk, or null when it cannot be read.</summary>
    /// <remarks>
    /// Null rather than an exception: the search path is a list of places a dictionary
    /// <em>might</em> be, and one of them being unreadable is an ordinary outcome rather than a
    /// document-rendering failure.
    /// </remarks>
    public static HyphenationPatterns? ReadFile(string path)
    {
        try
        {
            return Read(File.ReadAllBytes(path));
        }
        catch (IOException)
        {
            return null;
        }
        catch (UnauthorizedAccessException)
        {
            return null;
        }
    }

    private static Encoding Decode(string name)
    {
        // The names in the wild are the iconv spellings — "UTF-8", "ISO8859-1", "microsoft-cp1251"
        // — which .NET does not all know, and the ISO ones it does know only as "iso-8859-1".
        string wanted = name.Replace("ISO8859", "iso-8859", StringComparison.OrdinalIgnoreCase);

        try
        {
            return Encoding.GetEncoding(wanted);
        }
        catch (ArgumentException)
        {
            return Encoding.UTF8;
        }
    }

    private static HyphenationPatterns Parse(string text)
    {
        Dictionary<string, byte[]> patterns = new(StringComparer.Ordinal);
        int longest = 0;
        int left = 2;
        int right = 2;
        bool first = true;

        foreach (string raw in text.Split('\n'))
        {
            string line = raw.Trim('\r', ' ', '\t');

            // The first line is the encoding name and has already been used; skipping it by
            // position rather than by shape matters, because "UTF-8" is a syntactically valid
            // pattern and would otherwise enter the trie as one.
            if (first)
            {
                first = false;
                continue;
            }

            if (line.Length == 0 || line[0] == '%') continue;

            if (Keyword(line, "LEFTHYPHENMIN", ref left)) continue;
            if (Keyword(line, "RIGHTHYPHENMIN", ref right)) continue;

            // The compound minima are read and discarded: they bound hyphenation inside the parts
            // of a compound word, which needs the morphological analysis this does not do.
            int ignored = 0;
            if (Keyword(line, "COMPOUNDLEFTHYPHENMIN", ref ignored)) continue;
            if (Keyword(line, "COMPOUNDRIGHTHYPHENMIN", ref ignored)) continue;

            if (line == "NEXTLEVEL")
            {
                // Everything before this was the compound-boundary level. Ordinary hyphenation is
                // the level after it, so the earlier patterns are dropped rather than merged —
                // merging them would let a compound-boundary value forbid a real point.
                patterns.Clear();
                longest = 0;
                continue;
            }

            // Non-standard hyphenation, which this does not implement. Skipped whole: half of one
            // is a pattern that fires without its replacement.
            if (line.Contains('/', StringComparison.Ordinal)) continue;

            (string letters, byte[] values) = Split(line);
            if (letters.Length == 0) continue;

            patterns[letters] = values;
            if (letters.Length > longest) longest = letters.Length;
        }

        return new HyphenationPatterns(patterns, longest, Math.Max(left, 2), Math.Max(right, 2));
    }

    private static bool Keyword(string line, string name, ref int value)
    {
        if (!line.StartsWith(name, StringComparison.Ordinal)) return false;
        if (line.Length > name.Length && line[name.Length] is not (' ' or '\t')) return false;

        if (int.TryParse(
                line.AsSpan(name.Length).Trim(),
                System.Globalization.NumberStyles.Integer,
                System.Globalization.CultureInfo.InvariantCulture,
                out int read))
        {
            value = read;
        }

        return true;
    }

    /// <summary>Separates a pattern's letters from the values interleaved between them.</summary>
    private static (string Letters, byte[] Values) Split(string pattern)
    {
        char[] letters = new char[pattern.Length];
        byte[] values = new byte[pattern.Length + 1];
        int at = 0;

        foreach (char c in pattern)
        {
            if (c is >= '0' and <= '9')
            {
                values[at] = (byte)(c - '0');
                continue;
            }

            letters[at++] = c;
        }

        return (new string(letters, 0, at), values[..(at + 1)]);
    }

    /// <inheritdoc/>
    /// <remarks>
    /// <paramref name="language"/> is ignored: a pattern file is one language's, and which file
    /// answers for a tag is <see cref="Hyphenators"/>' question rather than this one's.
    /// </remarks>
    public IReadOnlyList<int> FindHyphenationPoints(ReadOnlySpan<char> word, string language)
    {
        if (word.Length == 0 || patterns.Count == 0) return [];

        // Trailing full stops are not part of the word and would anchor the end-of-word patterns
        // in the wrong place — hyphenimp.cxx:333 strips them the same way.
        int length = word.Length;
        while (length > 0 && word[length - 1] == '.') length--;

        if (length < LeftMinimum + RightMinimum) return [];

        // "." + the word + ".", which is what the anchoring patterns match against.
        char[] framed = new char[length + 2];
        framed[0] = '.';
        framed[^1] = '.';

        for (int at = 0; at < length; at++)
        {
            char c = word[at] switch
            {
                '“' or '”' => '"',
                '‘' or '’' => '\'',
                char other => other,
            };

            framed[at + 1] = char.ToLowerInvariant(c);
        }

        string framedWord = new(framed);
        byte[] points = new byte[framed.Length + 1];

        for (int at = 0; at < framed.Length; at++)
        {
            int reach = Math.Min(longest, framed.Length - at);

            for (int span = 1; span <= reach; span++)
            {
                if (!patterns.TryGetValue(framedWord.Substring(at, span), out byte[]? values))
                    continue;

                for (int j = 0; j < values.Length; j++)
                    if (values[j] > points[at + j]) points[at + j] = values[j];
            }
        }

        // points[k + 2] is the value between the word's k-th and (k+1)-th characters, so a break
        // there leaves k + 1 characters on the line. The two minima are the file's own.
        List<int> found = [];

        for (int k = LeftMinimum - 1; k <= length - RightMinimum - 1; k++)
            if ((points[k + 2] & 1) == 1) found.Add(k + 1);

        return found.Count == 0 ? [] : found;
    }
}
