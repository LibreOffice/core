using System.Text;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// Turns the raw characters of a SpreadsheetML <c>t</c> element into the text Calc holds in the
/// cell: <c>ST_Xstring</c>'s <c>_xHHHH_</c> escapes resolved, and the control characters handled
/// the way Calc handles them.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is deliberately not in <c>XRay.Office.Ooxml</c>, and that is the load-bearing part of
/// the design.</b> The escape looks like an OOXML-wide convention and is not one:
/// <c>CT_Rst/t</c> — shared strings, inline strings, comment text — is typed <c>ST_Xstring</c>,
/// while WordprocessingML's <c>w:t</c> and DrawingML's <c>a:t</c> are plain <c>ST_String</c>.
/// Measured against the installed LibreOffice 26.2.4.2 rather than read off the schema: a
/// <c>.docx</c> and a <c>.pptx</c> each holding <c>ALPHA_x000D_BRAVO</c>,
/// <c>CHARLIE_x000A_DELTA</c>, <c>ECHO_x005F_x000D_FOXTROT</c> and <c>GOLF_x001E_HOTEL</c> come
/// back from <c>soffice</c> with all four strings drawn <em>literally</em>, seven glyphs and
/// all, while the same strings in a shared string table are decoded. Hoisting this into the
/// shared OOXML library is the change that would silently start eating <c>_x000D_</c> out of
/// Writer and Impress text, where it is real content.
/// </para>
/// <para>
/// The corpus makes the same point from the other side. 84 of its 534 documents carry something
/// shaped like <c>_xHHHH_</c>, and 78 of those are <c>_x0000_</c> inside a VML
/// <c>o:spid</c>/<c>id</c> attribute — a shape identifier, not text, in files this rule must
/// never touch. Rendering all 534 either side of this class reached the same conclusion from
/// the output: 15 documents change, every one of them a spreadsheet.
/// </para>
/// </remarks>
internal static class XlsxCellText
{
    /// <summary>The last character Calc treats as a control rather than as text.</summary>
    /// <remarks>
    /// U+007F is deliberately outside the range: LibreOffice draws it, and one corpus workbook
    /// carries one inside a path-like string.
    /// </remarks>
    private const char LastControl = '\u001F';

    /// <summary>
    /// The cell text a <c>t</c> element's raw characters stand for.
    /// </summary>
    /// <param name="raw">The element's value, straight from the XML.</param>
    /// <remarks>
    /// <para>
    /// Two stages, and the second branches on what the first produced. Everything asserted here
    /// was measured by converting authored workbooks with <c>soffice</c> and reading the answer
    /// out of the PDF's text-showing operators, never off a raster.
    /// </para>
    /// <para>
    /// <b>Stage 1 — decode <c>_xHHHH_</c>.</b> Four hex digits, either case: <c>aa_x000d_bb</c>
    /// and <c>aa_x000D_bb</c> both decode. Anything not exactly that shape is not an escape and
    /// stands as written — <c>mm_x00D_nn</c> (three digits) and <c>oo_xZZZZ_pp</c> both survive.
    /// Scanning left to right and resuming <em>after</em> each escape is what makes
    /// <c>_x005F_</c> work: it yields <c>_</c>, and the <c>x000D_</c> behind it is then ordinary
    /// text, so <c>ECHO_x005F_x000D_FOXTROT</c> comes out as the literal
    /// <c>ECHO_x000D_FOXTROT</c>. A regular-expression replace over the whole string decodes that
    /// a second time and loses it.
    /// </para>
    /// <para>
    /// <b>Stage 2 — normalise the controls, under whichever rule the presence of a line feed
    /// selects.</b> That conditional is not a guess at an implementation; it is what the
    /// measurements force, and getting it wrong in either direction moves real documents:
    /// </para>
    /// <list type="table">
    /// <listheader>
    /// <term>character</term>
    /// <description>in a string with no LF … / in one holding at least one LF</description>
    /// </listheader>
    /// <item>
    /// <term>U+000D</term>
    /// <description>dropped — no glyph, no advance, no break … / a line break</description>
    /// </item>
    /// <item>
    /// <term>U+0009</term>
    /// <description>dropped … / kept, and it advances to a tab stop</description>
    /// </item>
    /// <item>
    /// <term>other U+0000–U+001F</term>
    /// <description>dropped … / dropped</description>
    /// </item>
    /// </list>
    /// <para>
    /// So <c>ALPHA_x000D_BRAVO</c> is drawn <c>ALPHABRAVO</c> — one show, one baseline, the words
    /// glued — while the same escape inside a string that also holds a newline breaks the line.
    /// Reading the first case as a break is the defect this class was written for: 4872 escapes
    /// on <c>FY2018_Q4_UAS_Sightings.xlsx</c>, 304 pages against the reference's 302.
    /// </para>
    /// <para>
    /// In the line-feed case the break rule is ordinary line-ending normalisation, measured pair
    /// by pair: <c>CR LF</c> is one break and so is <c>LF CR</c>, while <c>CR CR</c> and
    /// <c>LF LF</c> are two apiece. Both orders matter. The corpus contains LF <em>before</em> CR
    /// — written <c>&amp;#x0A;&amp;#x0D;</c> — and reading that as two breaks put a line of
    /// <c>AFCforPtF-Digital-Certificate-Publication-Report.xlsx</c> 8.98 pt below where the
    /// reference draws it, on a document whose page and word counts never moved at all.
    /// </para>
    /// <para>
    /// Both rules are on the character, not on how it was spelled. A literal tab and
    /// <c>_x0009_</c> behave identically in both contexts, and the corpus reaches this class
    /// through three different spellings of the same character: the <c>_x000D_</c> escape, a
    /// <c>&amp;#x0D;</c> numeric reference — which XML, unlike a literal CR, does <em>not</em>
    /// line-ending-normalise — and a literal tab. By the time Calc holds the string the three are
    /// indistinguishable, so a decoder that treated only what it had itself decoded would be
    /// inventing a distinction Calc cannot see.
    /// </para>
    /// </remarks>
    public static string Of(string? raw)
        => string.IsNullOrEmpty(raw) ? string.Empty : Normalise(Decode(raw));

    /// <summary>
    /// The text Calc <em>stores</em> for the same element: stage 1 alone, with no control
    /// normalisation over it.
    /// </summary>
    /// <param name="raw">The element's value, straight from the XML.</param>
    /// <remarks>
    /// <para>
    /// <b>A cell's drawn text and its compared value are two different strings, and only the
    /// first of them goes through <see cref="Of"/>.</b> The reference's own shared-string
    /// import is one line — <c>RichStringPortion::setText</c> is
    /// <c>maText = AttributeConversion::decodeXString(rText)</c>
    /// (<c>sc/source/filter/oox/richstring.cxx</c>:60-63) — so what Calc holds in the cell is
    /// the escape-decoded string and nothing else. Every control character survives into the
    /// document model; the eliding <see cref="Of"/> describes happens later, in the drawing
    /// layer, and a conditional format never sees it.
    /// </para>
    /// <para>
    /// The difference is not academic. <c>ScConditionEntry::FillCache</c> keys a
    /// <c>duplicateValues</c> cache on the cell's own string, so on
    /// <c>ST Capability List Rev.16</c> the reference reads <c>"\tD5758620001301"</c> and
    /// <c>"D5758620001301"</c> as two different keys and paints neither, while a reader that
    /// compares the drawn text sees one key twice and paints both. The same string reaches
    /// <c>containsBlanks</c>, whose replacement formula is <c>LEN(TRIM(#B))=0</c>: <c>TRIM</c>
    /// takes spaces and not tabs, so a cell holding a lone <c>U+0009</c> is <em>not</em> blank
    /// to the reference and is blank to a reader that normalised the tab away first.
    /// </para>
    /// <para>
    /// Both halves are measured on <c>tests/corpus/features/sheet-cf-stored-vs-drawn.xlsx</c>
    /// at 26.2.4.2 — see <c>probes/cond-format-r97/results.md</c>.
    /// </para>
    /// </remarks>
    public static string Stored(string? raw)
        => string.IsNullOrEmpty(raw) ? string.Empty : Decode(raw);

    /// <summary>
    /// The drawn text of a string that has already been through <see cref="Stored"/>.
    /// </summary>
    /// <param name="stored">The stored spelling, escapes resolved and controls intact.</param>
    /// <remarks>
    /// <c>Drawn(Stored(x))</c> is <c>Of(x)</c> — the same string and, where nothing needs
    /// changing, the same instance — so a caller that wants both spellings decodes once instead
    /// of twice. A shared string table runs to tens of thousands of entries and both callers
    /// want both.
    /// </remarks>
    public static string Drawn(string stored)
        => string.IsNullOrEmpty(stored) ? string.Empty : Normalise(stored);

    /// <summary>Resolves every <c>_xHHHH_</c>, returning the original when there is none.</summary>
    private static string Decode(string raw)
    {
        if (!raw.Contains('_', StringComparison.Ordinal)) return raw;

        StringBuilder text = new(raw.Length);
        for (int at = 0; at < raw.Length;)
        {
            if (Escape(raw, at) is { } decoded)
            {
                text.Append(decoded);
                at += 7;
            }
            else
            {
                text.Append(raw[at]);
                at++;
            }
        }
        return text.ToString();
    }

    /// <summary>
    /// The character an <c>_xHHHH_</c> at this offset stands for, or null when there is none.
    /// </summary>
    private static char? Escape(string raw, int at)
    {
        if (at + 7 > raw.Length
            || raw[at] != '_'
            || (raw[at + 1] | 0x20) != 'x'
            || raw[at + 6] != '_')
        {
            return null;
        }

        int value = 0;
        for (int digit = at + 2; digit < at + 6; digit++)
        {
            int nibble = Nibble(raw[digit]);
            if (nibble < 0) return null;
            value = (value << 4) | nibble;
        }
        return (char)value;
    }

    private static int Nibble(char c) => c switch
    {
        >= '0' and <= '9' => c - '0',
        >= 'a' and <= 'f' => c - 'a' + 10,
        >= 'A' and <= 'F' => c - 'A' + 10,
        _ => -1,
    };

    /// <summary>Applies whichever control rule the string's context selects.</summary>
    private static string Normalise(string text)
    {
        bool multiline = text.Contains('\n', StringComparison.Ordinal);
        if (!NeedsWork(text, multiline)) return text;

        StringBuilder result = new(text.Length);
        for (int at = 0; at < text.Length; at++)
        {
            char c = text[at];

            if (multiline && c is '\r' or '\n')
            {
                result.Append('\n');

                // One break for CR LF and one for LF CR, two for CR CR and two for LF LF — so a
                // pair collapses only when the second character is the *other* one.
                char partner = c == '\r' ? '\n' : '\r';
                if (at + 1 < text.Length && text[at + 1] == partner) at++;
                continue;
            }

            if (c > LastControl || (multiline && c == '\t')) result.Append(c);
        }
        return result.ToString();
    }

    /// <summary>Whether the string holds anything the normalisation would change.</summary>
    /// <remarks>
    /// Almost none do, and a shared string table runs to tens of thousands of entries — the
    /// corpus's largest holds 4872 escapes among far more strings holding none. Returning the
    /// original instance for those allocates nothing.
    /// </remarks>
    private static bool NeedsWork(string text, bool multiline)
    {
        foreach (char c in text)
        {
            if (c > LastControl || c == '\n') continue;
            if (multiline && c == '\t') continue;

            // Any CR is work even in a multi-line string: a lone one becomes a break, and one
            // beside an LF collapses into it.
            return true;
        }
        return false;
    }
}
