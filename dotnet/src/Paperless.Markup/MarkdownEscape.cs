using System.Text;

namespace Paperless.Markup;

/// <summary>
/// Backslash-escaping for text emitted into GitHub-Flavored Markdown.
/// </summary>
/// <remarks>
/// <para>
/// This is the part of Markdown output that fails silently. A document containing a literal
/// <c>*</c>, <c>_</c>, <c>|</c>, <c>#</c> or a backtick is entirely ordinary; emitted raw it
/// turns the neighbouring words into emphasis, splits a table row, or promotes a paragraph to
/// a heading — and every character is still present, so no text diff can see it. Only parsing
/// the result back catches it, which is what the round-trip tests do.
/// </para>
/// <para>
/// The strategy is to escape by <em>construct</em> rather than by character class. Escaping
/// every ASCII punctuation mark — which CommonMark permits — would be safe and would also make
/// <c>snake_case_name</c> read as <c>snake\_case\_name</c>, which defeats the point of emitting
/// Markdown rather than text. So each character is escaped only in the positions where it can
/// begin something, and the positions are enumerated here rather than guessed:
/// </para>
/// <list type="bullet">
///   <item><description>
///     Always: <c>\</c>, <c>`</c>, <c>*</c>, <c>[</c>, <c>]</c>, <c>&lt;</c>, <c>~</c>,
///     <c>|</c>. <c>*</c> is intraword emphasis in CommonMark, so unlike <c>_</c> it has no
///     safe position. <c>|</c> is escaped everywhere and not only inside cells, because a
///     paragraph of pipes followed by a paragraph of dashes is a table.
///   </description></item>
///   <item><description>
///     <c>_</c> only at a word boundary, which is exactly CommonMark's own rule: intraword
///     underscores are never emphasis.
///   </description></item>
///   <item><description>
///     <c>&amp;</c> only when what follows could be a character reference, and <c>!</c> only
///     before <c>[</c>.
///   </description></item>
///   <item><description>
///     At the start of a line, additionally: <c>#</c>, <c>&gt;</c>, <c>-</c>, <c>+</c>,
///     <c>=</c>, and the <c>.</c> or <c>)</c> that turns a leading number into an ordered
///     list. These are block openers and are harmless anywhere else, so escaping them
///     everywhere would be pure noise.
///   </description></item>
/// </list>
/// </remarks>
public static class MarkdownEscape
{
    /// <summary>
    /// Escapes text for a position inside a line — never the first thing on one.
    /// </summary>
    public static string Inline(string text) => Escape(text, atLineStart: false);

    /// <summary>
    /// Escapes text for the start of a block, where block openers also have to be neutralised.
    /// </summary>
    public static string BlockStart(string text) => Escape(text, atLineStart: true);

    private static string Escape(string text, bool atLineStart)
    {
        ArgumentNullException.ThrowIfNull(text);
        StringBuilder result = new(text.Length + 8);

        for (int i = 0; i < text.Length; i++)
        {
            char c = text[i];
            bool lineStart = atLineStart && i == 0;

            switch (c)
            {
                case '\\' or '`' or '*' or '[' or ']' or '<' or '~' or '|':
                    result.Append('\\').Append(c);
                    break;

                // Only at a word boundary. CommonMark's flanking rules make an underscore
                // between two alphanumerics incapable of opening or closing emphasis, so
                // escaping it there would be noise in every identifier and file name.
                case '_' when IsWordBoundary(text, i):
                    result.Append("\\_");
                    break;

                case '&' when LooksLikeCharacterReference(text, i):
                    result.Append("\\&");
                    break;

                case '!' when i + 1 < text.Length && text[i + 1] == '[':
                    result.Append("\\!");
                    break;

                case '#' or '>' or '-' or '+' or '=' when lineStart:
                    result.Append('\\').Append(c);
                    break;

                default:
                    result.Append(c);
                    break;
            }

            // A leading run of digits followed by "." or ")" opens an ordered list. Escaping the
            // digits does nothing — CommonMark has no escape for a digit — so the delimiter is
            // what has to be escaped, and only when the whole prefix is digits.
            if (lineStart && char.IsDigit(c))
            {
                int j = i;
                while (j < text.Length && char.IsDigit(text[j])) j++;
                if (j < text.Length && (text[j] == '.' || text[j] == ')'))
                {
                    result.Append(text.AsSpan(i + 1, j - i - 1)).Append('\\').Append(text[j]);
                    i = j;
                }
            }
        }

        return result.ToString();
    }

    /// <summary>
    /// Escapes a link or image destination for the <c>(…)</c> of an inline link.
    /// </summary>
    /// <remarks>
    /// A destination containing whitespace or an unbalanced parenthesis has to be wrapped in
    /// angle brackets, which is CommonMark's own provision for it. Percent-encoding instead
    /// would change the URL, and a document's hyperlink target is not ours to rewrite.
    /// </remarks>
    public static string Destination(string target)
    {
        ArgumentNullException.ThrowIfNull(target);
        if (target.Length == 0) return string.Empty;

        bool simple = true;
        int depth = 0;
        foreach (char c in target)
        {
            if (char.IsWhiteSpace(c) || c is '<' or '>') { simple = false; break; }
            if (c == '(') depth++;
            else if (c == ')' && --depth < 0) { simple = false; break; }
        }
        if (simple && depth == 0) return target;

        StringBuilder result = new(target.Length + 4);
        result.Append('<');
        foreach (char c in target)
        {
            if (c is '<' or '>' or '\\') result.Append('\\');
            result.Append(c == '\n' ? ' ' : c);
        }
        return result.Append('>').ToString();
    }

    /// <summary>
    /// Escapes text destined for a GFM table cell, where a newline cannot be represented.
    /// </summary>
    /// <remarks>
    /// A pipe-table row is one line by definition, so a hard break inside a cell becomes a
    /// space. That is a real loss and the reason a cell holding more than one block sends the
    /// whole table to the HTML fallback instead; a single run carrying a line break is the one
    /// case left, and a space is the least wrong thing to put there.
    /// </remarks>
    public static string TableCell(string text)
    {
        ArgumentNullException.ThrowIfNull(text);
        return Inline(text).Replace('\n', ' ').Replace('\r', ' ');
    }

    private static bool IsWordBoundary(string text, int index)
    {
        bool before = index == 0 || !char.IsLetterOrDigit(text[index - 1]);
        bool after = index + 1 >= text.Length || !char.IsLetterOrDigit(text[index + 1]);
        return before || after;
    }

    private static bool LooksLikeCharacterReference(string text, int index)
    {
        int i = index + 1;
        if (i < text.Length && text[i] == '#') i++;
        int start = i;
        while (i < text.Length && char.IsLetterOrDigit(text[i])) i++;
        return i > start && i < text.Length && text[i] == ';';
    }
}
