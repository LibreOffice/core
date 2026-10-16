using XRay.Office.WordProcessing.Model;

namespace XRay.Office.WordProcessing;

/// <summary>
/// Reads what a word-processing field's instruction says, for the fields whose meaning is not
/// recoverable from their cached result.
/// </summary>
/// <remarks>
/// <para>
/// A field is a small program: an instruction, and the result the writing application last computed
/// for it. X-Ray.Office keeps the result, because that is what a reader saw — but a hyperlink's
/// <em>target</em> appears only in the instruction, so that one field has to be understood rather
/// than skipped.
/// </para>
/// <para>
/// Shared because three of the four word-processing formats spell a hyperlink the same way. Neither
/// RTF nor DOC has hyperlink markup at all, and a DOCX written by a converter often has a
/// <c>HYPERLINK</c> field where a native one would have a relationship — so the same parsing serves
/// all three, over three completely different containers.
/// </para>
/// </remarks>
public static class FieldInstructions
{
    /// <summary>The field name that introduces a hyperlink.</summary>
    private const string Hyperlink = "HYPERLINK";

    /// <summary>
    /// The target of a <c>HYPERLINK</c> field, or null when the instruction is not one.
    /// </summary>
    /// <remarks>
    /// The syntax is <c>HYPERLINK "target"</c> with optional switches, and the quoted argument is the
    /// target. An unquoted argument is accepted too: producers omit the quotes when the target has no
    /// spaces, and rejecting those loses ordinary links. A <c>\l</c> switch introduces a bookmark
    /// within the document, which is a location rather than a target and is left to the caller.
    /// </remarks>
    public static string? HyperlinkTarget(string? instruction)
    {
        if (instruction is null) return null;

        string text = instruction.Trim();
        if (!text.StartsWith(Hyperlink, StringComparison.OrdinalIgnoreCase)) return null;

        string arguments = text[Hyperlink.Length..].TrimStart();

        int firstQuote = arguments.IndexOf('"', StringComparison.Ordinal);
        if (firstQuote >= 0)
        {
            int secondQuote = arguments.IndexOf('"', firstQuote + 1);
            if (secondQuote < 0) return null;

            string quoted = arguments[(firstQuote + 1)..secondQuote];
            return quoted.Length == 0 ? null : quoted;
        }

        // Unquoted: the target runs to the first whitespace, since anything after it is a switch.
        int end = arguments.AsSpan().IndexOfAny(' ', '\t', '\n');
        string bare = end < 0 ? arguments : arguments[..end];
        return bare.Length == 0 || bare.StartsWith('\\') ? null : bare;
    }

    /// <summary>
    /// The field's name — its first token — or null when the instruction states none.
    /// </summary>
    /// <remarks>
    /// The name is what the whole instruction is dispatched on, and it is separated from its
    /// arguments by white space alone. A leading quote is skipped rather than read as part of a name:
    /// a producer writing <c>"PAGE"</c> means the field, and taking the quote with it names nothing.
    /// </remarks>
    public static string? Name(string? instruction)
    {
        if (instruction is null) return null;

        ReadOnlySpan<char> text = instruction.AsSpan().Trim();
        text = text.TrimStart('"');

        int end = text.IndexOfAny(' ', '\t', '\n');
        if (end >= 0) text = text[..end];
        text = text.TrimEnd('"');

        return text.Length == 0 ? null : text.ToString();
    }

    /// <summary>
    /// The page-sensitive field an instruction names, and the sequence it asks the value be written in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Null for every field whose cached result is worth keeping, which is all but two of them. A
    /// <c>PAGE</c> and a <c>NUMPAGES</c> are the pair whose cached result is a statement about the
    /// document the producer had rather than the one being laid out, so they are the pair a paginating
    /// renderer has to recompute; <c>SECTIONPAGES</c> is folded onto the count because we do not track
    /// per-section totals and the whole-document total is nearer than a stale number.
    /// </para>
    /// <para>
    /// The general number-picture switch is <c>\*</c> followed by a format name. Word's names for the
    /// five sequences are not the ones <c>w:numFmt</c> uses — it spells them <c>Arabic</c>,
    /// <c>roman</c>, <c>ROMAN</c>, <c>alphabetic</c> and <c>ALPHABETIC</c>, with case deciding two of
    /// them — so they are mapped here rather than through <see cref="Layout.NoteNumbering.Parse"/>.
    /// Null means the field asked for nothing and the section's own format decides, which is what all
    /// but a handful of real fields do: across this corpus's DOCX the only switches written beside a
    /// <c>PAGE</c> are <c>\* MERGEFORMAT</c>, <c>\* CHARFORMAT</c> and <c>\* Arabic</c>.
    /// </para>
    /// </remarks>
    /// <param name="instruction">The instruction, verbatim.</param>
    public static (Layout.PageFieldKind Kind, Layout.NoteNumberFormat? Format)? PageFieldOf(
        string? instruction)
    {
        Layout.PageFieldKind? kind = Name(instruction)?.ToUpperInvariant() switch
        {
            "PAGE" => Layout.PageFieldKind.PageNumber,
            "NUMPAGES" or "SECTIONPAGES" => Layout.PageFieldKind.PageCount,
            _ => null,
        };

        return kind is { } named ? (named, NumberPicture(instruction)) : null;
    }

    /// <summary>
    /// The document-constant field an instruction names, or null when its cached result is worth keeping.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same argument <see cref="PageFieldOf"/> makes about <c>PAGE</c>, one step out: a cached
    /// result is what the field said when the producer last saved, and for these two it is a statement
    /// about a file that no longer exists. LibreOffice re-evaluates both on load — a <c>FILENAME</c>
    /// becomes <c>SwFileNameField</c> and a <c>TITLE</c> a <c>SwDocInfoField</c> over the package's
    /// <c>dc:title</c> — so the reference draws today's answer where the cache holds yesterday's.
    /// Measured on <c>CRIF - Sp…cification technique - Socle applicatif.docx</c>, whose footer caches
    /// <c>SPECTECH-socle-applicatif.doc</c> and whose header caches <c>ENT</c>: the two together are 13
    /// words a page against 27 pages, which is 351 of that document's 363-word gap.
    /// </para>
    /// <para>
    /// A <c>FILENAME</c> carrying <c>\p</c> asks for the full path
    /// (<c>DomainMapper_Impl.cxx</c>:8296, <c>FilenameDisplayFormat::FULL</c>) and is deliberately left
    /// at its cache: X-Ray.Office reads streams as readily as files and keeps only the leaf name, so
    /// substituting there would draw a shorter string than the reference rather than a different one.
    /// </para>
    /// </remarks>
    /// <param name="instruction">The instruction, verbatim.</param>
    public static ConstantField? ConstantFieldOf(string? instruction) =>
        Name(instruction)?.ToUpperInvariant() switch
        {
            "FILENAME" => HasSwitch(instruction, 'p') ? null : ConstantField.FileName,
            "TITLE" => ConstantField.Title,
            _ => null,
        };

    /// <summary>
    /// The style a <c>STYLEREF</c> field names, when LibreOffice would substitute that style's text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>STYLEREF</c> quotes the nearest paragraph in a named style. Word can quote several parts of it
    /// — <c>\n</c>, <c>\r</c> and <c>\w</c> ask for the paragraph's <em>number</em> in three widths,
    /// <c>\p</c> for "above"/"below", and <c>\s</c> for the complete number — and LibreOffice implements
    /// four of the five: <c>DomainMapper_Impl.cxx</c>:8600 maps <c>p</c>, <c>r</c>, <c>n</c> and
    /// <c>w</c> onto <c>ReferenceFieldPart</c> and has no branch for <c>s</c> at all, so a
    /// <c>STYLEREF … \s</c> keeps the default part, <c>TEXT</c>, and draws the heading's <em>text</em>
    /// where Word drew its number.
    /// </para>
    /// <para>
    /// That divergence is the whole of <c>report-template.docx</c>: its seven captions read
    /// <c>Table 1.2</c> from Word's cache and <c>Table Main body (Heading 2).2</c> in the reference,
    /// which is 43 words and wraps every caption onto a second line. Matching the reference means
    /// reproducing the same substitution, so this returns the style for the cases LibreOffice treats as
    /// <c>TEXT</c> and null for the four it computes differently — a part switch we do not model is
    /// better served by the producer's cached result.
    /// </para>
    /// <para>
    /// A bare digit is Word's undocumented shorthand for the built-in heading of that level, which
    /// LibreOffice reproduces in as many words (<c>reffld.cxx</c>:1682, "undocumented Word feature: 1 =
    /// <c>Heading 1</c>"). The digit is returned as written; mapping it onto a style is the caller's,
    /// since only the reader knows what the document calls its headings.
    /// </para>
    /// </remarks>
    /// <param name="instruction">The instruction, verbatim.</param>
    public static string? StyleReferenceName(string? instruction)
    {
        if (!string.Equals(Name(instruction), "STYLEREF", StringComparison.OrdinalIgnoreCase))
        {
            return null;
        }

        // `\l` only changes which end the search starts from, but the four part switches change what is
        // quoted — and for those LibreOffice does compute something this does not model.
        foreach (char part in "prnwtl")
        {
            if (HasSwitch(instruction, part)) return null;
        }

        ReadOnlySpan<char> arguments = instruction.AsSpan().Trim();
        int after = arguments.IndexOfAny(' ', '\t', '\n');
        if (after < 0) return null;

        arguments = arguments[after..].TrimStart();

        if (arguments.Length > 0 && arguments[0] == '"')
        {
            int close = arguments[1..].IndexOf('"');
            return close < 0 ? null : Named(arguments.Slice(1, close));
        }

        int end = arguments.IndexOfAny(' ', '\t', '\n');
        return Named(end < 0 ? arguments : arguments[..end]);

        static string? Named(ReadOnlySpan<char> name)
        {
            name = name.Trim();
            return name.Length == 0 || name[0] == '\\' ? null : name.ToString();
        }
    }

    /// <summary>
    /// The bookmark a <c>REF</c> field quotes, when LibreOffice would draw that bookmark's own text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A <c>REF</c> whose first argument is not a <c>SET</c>/<c>SEQ</c> variable becomes a
    /// <c>SwGetRefField</c> with <c>ReferenceFieldSource::BOOKMARK</c> and the argument as its source
    /// name (<c>DomainMapper_Impl.cxx</c>:8598-8608), and Writer recomputes what it draws from the
    /// bookmark rather than keeping the <c>{\fldrslt}</c> or <c>w:fldSimple</c> cache. So a reader
    /// that keeps the cache draws the string the producing application last computed, which is a
    /// different string whenever the two applications disagree about where the bookmark is.
    /// </para>
    /// <para>
    /// Null for the four switches that ask for something other than the text — <c>\p</c> above/below,
    /// <c>\r</c>, <c>\n</c> and <c>\w</c>, the three numberings (<c>:8611-8632</c>) — and for
    /// <c>PAGEREF</c>, which is the same field with <c>ReferenceFieldPart::PAGE</c>. Those keep the
    /// producer's result, on the same reasoning as a <c>FILENAME \p</c>: a part we do not compute is
    /// better served by the cache than by a wrong answer.
    /// </para>
    /// </remarks>
    /// <param name="instruction">The instruction, verbatim.</param>
    public static string? ReferenceBookmark(string? instruction)
    {
        if (!string.Equals(Name(instruction), "REF", StringComparison.OrdinalIgnoreCase)) return null;

        foreach (char part in "prnw")
        {
            if (HasSwitch(instruction, part)) return null;
        }

        ReadOnlySpan<char> arguments = instruction.AsSpan().Trim();
        int after = arguments.IndexOfAny(' ', '\t', '\n');
        if (after < 0) return null;

        arguments = arguments[after..].TrimStart();

        if (arguments.Length > 0 && arguments[0] == '"')
        {
            int close = arguments[1..].IndexOf('"');
            return close < 0 ? null : Named(arguments.Slice(1, close));
        }

        int end = arguments.IndexOfAny(' ', '\t', '\n');
        return Named(end < 0 ? arguments : arguments[..end]);

        static string? Named(ReadOnlySpan<char> name)
        {
            name = name.Trim();
            return name.Length == 0 || name[0] == '\\' ? null : name.ToString();
        }
    }

    /// <summary>Whether the instruction carries a given single-letter switch.</summary>
    private static bool HasSwitch(string? instruction, char letter)
    {
        if (instruction is null) return false;

        ReadOnlySpan<char> text = instruction.AsSpan();
        for (int at = text.IndexOf('\\'); at >= 0 && at + 1 < text.Length;)
        {
            if (text[at + 1] == letter) return true;

            int next = text[(at + 1)..].IndexOf('\\');
            if (next < 0) break;
            at += 1 + next;
        }

        return false;
    }

    /// <summary>
    /// The sequence a <c>\*</c> switch names, or null when the instruction carries none this models.
    /// </summary>
    private static Layout.NoteNumberFormat? NumberPicture(string? instruction)
    {
        if (instruction is null) return null;

        ReadOnlySpan<char> text = instruction.AsSpan();

        for (int at = text.IndexOf('\\'); at >= 0 && at + 1 < text.Length;)
        {
            if (text[at + 1] == '*')
            {
                ReadOnlySpan<char> rest = text[(at + 2)..].TrimStart();
                int end = rest.IndexOfAny(' ', '\t', '\n');
                ReadOnlySpan<char> name = end < 0 ? rest : rest[..end];

                // Case decides two of the five, so this is deliberately ordinal.
                switch (name.ToString())
                {
                    case "roman": return Layout.NoteNumberFormat.LowerRoman;
                    case "ROMAN": return Layout.NoteNumberFormat.UpperRoman;
                    case "alphabetic": return Layout.NoteNumberFormat.LowerLetter;
                    case "ALPHABETIC": return Layout.NoteNumberFormat.UpperLetter;
                    default:
                        if (name.Equals("Arabic", StringComparison.OrdinalIgnoreCase))
                        {
                            return Layout.NoteNumberFormat.Arabic;
                        }

                        break;
                }
            }

            int next = text[(at + 1)..].IndexOf('\\');
            if (next < 0) break;
            at += 1 + next;
        }

        return null;
    }

    /// <summary>
    /// What a field instruction computes, as far as the shared vocabulary names it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Dispatched on the instruction's name, which is how all three of the formats that have an
    /// instruction say what a field is. WW8 additionally records a numeric field type in its field
    /// PLCF — the table at <c>sw/source/filter/ww8/ww8par5.cxx</c>'s <c>aWW8FieldTab</c>, where 33 is
    /// <c>PAGE</c>, 37 is <c>PAGEREF</c> and 31 and 32 are <c>DATE</c> and <c>TIME</c> — but the
    /// instruction text is beside it in the same character stream and says the same thing, so one
    /// mapping serves all three formats instead of two mappings that could disagree.
    /// </para>
    /// <para>
    /// An unrecognised name is <see cref="WritingFieldKind.Unknown"/> rather than a diagnostic. There
    /// are dozens of field types, most of them are computed by the writing application and cached, and
    /// a reader that only wants the cached result loses nothing by not naming them.
    /// </para>
    /// </remarks>
    public static WritingFieldKind KindOf(string? instruction) => Name(instruction)?.ToUpperInvariant() switch
    {
        "PAGE" => WritingFieldKind.PageNumber,
        "NUMPAGES" or "SECTIONPAGES" => WritingFieldKind.PageCount,
        "DATE" => WritingFieldKind.Date,
        "TIME" => WritingFieldKind.Time,
        "CREATEDATE" => WritingFieldKind.CreationDate,
        "SAVEDATE" or "PRINTDATE" => WritingFieldKind.ModificationDate,
        "AUTHOR" or "USERNAME" or "LASTSAVEDBY" => WritingFieldKind.Author,
        "FILENAME" => WritingFieldKind.FileName,
        "TITLE" => WritingFieldKind.Title,
        "SUBJECT" => WritingFieldKind.Subject,
        "KEYWORDS" => WritingFieldKind.Keywords,
        "COMMENTS" => WritingFieldKind.Description,
        "STYLEREF" => WritingFieldKind.Chapter,
        "REF" or "NOTEREF" => WritingFieldKind.Reference,
        "PAGEREF" => WritingFieldKind.PageReference,
        Hyperlink => WritingFieldKind.Hyperlink,
        "SEQ" => WritingFieldKind.Sequence,
        "SET" or "DOCVARIABLE" or "ASK" => WritingFieldKind.Variable,
        "TOC" or "INDEX" => WritingFieldKind.TableOfContents,
        "NUMWORDS" => WritingFieldKind.WordCount,
        _ => WritingFieldKind.Unknown,
    };
}

/// <summary>
/// A field whose value is a constant of the document rather than of the page it lands on.
/// </summary>
/// <remarks>
/// Two members rather than the whole <see cref="WritingFieldKind"/> vocabulary, for the same reason
/// <see cref="Layout.PageFieldKind"/> has two: these are the fields a layout can compute and so the
/// only ones whose cached result it is entitled to discard. Everything else — a <c>DOCPROPERTY</c>, a
/// <c>REF</c>, a merge field — is better served by what the producer last wrote.
/// </remarks>
public enum ConstantField
{
    /// <summary>The name of the file the document was read from, extension included.</summary>
    FileName,

    /// <summary>The document's title, from its package metadata.</summary>
    Title,
}
