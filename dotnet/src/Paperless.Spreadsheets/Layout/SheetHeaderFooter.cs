using System.Globalization;
using System.Text;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// A field a header or footer can hold in place of literal text.
/// </summary>
/// <remarks>
/// The set Calc puts in a header, which is <c>ScHeaderFieldData</c>
/// (<c>sc/source/ui/inc/printfun.hxx:60</c>): the page number, the total, the sheet's name, the
/// document's title and file name, and the date and time the printout was made. Everything else
/// a format can write in a header — a picture, a font change, a strikethrough — is formatting
/// rather than content, and is dropped rather than modelled.
/// </remarks>
public enum SheetHeaderField
{
    /// <summary>The printed page number: <c>&amp;P</c>, ODF's <c>text:page-number</c>.</summary>
    PageNumber,

    /// <summary>How many pages the printout has: <c>&amp;N</c>, <c>text:page-count</c>.</summary>
    PageCount,

    /// <summary>The date the printout was made: <c>&amp;D</c>, <c>text:date</c>.</summary>
    Date,

    /// <summary>The time it was made: <c>&amp;T</c>, <c>text:time</c>.</summary>
    Time,

    /// <summary>The sheet's tab name: <c>&amp;A</c>, <c>text:sheet-name</c>.</summary>
    SheetName,

    /// <summary>The document's file name: <c>&amp;F</c>, <c>text:file-name</c>.</summary>
    FileName,

    /// <summary>Its full path: <c>&amp;Z</c>, <c>text:file-name</c> with a path display.</summary>
    FilePath,

    /// <summary>The document's title, which is metadata rather than a name: <c>text:title</c>.</summary>
    Title,
}

/// <summary>
/// One piece of a header: either literal text or a field.
/// </summary>
/// <param name="Text">The literal text, or null when this is a field.</param>
/// <param name="Field">Which field it is, when <paramref name="Text"/> is null.</param>
/// <param name="Size">
/// The em size the segment states, or null for the sheet's default cell height. SpreadsheetML
/// and BIFF write it as <c>&amp;14</c> and it persists until the next such code, which is why it
/// belongs to the segment rather than to the part: <c>&amp;L&amp;8text&amp;R&amp;14more</c> is
/// two sizes in one band. ODF states it in a text style and reaches the same place.
/// </param>
public readonly record struct SheetHeaderSegment(
    string? Text, SheetHeaderField Field = default, Length? Size = null)
{
    /// <summary>A run of literal characters.</summary>
    /// <param name="text">The characters.</param>
    /// <param name="size">The em size it states, or null for the default.</param>
    public static SheetHeaderSegment Literal(string text, Length? size = null)
        => new(text, default, size);

    /// <summary>A field, resolved when the page it sits on is known.</summary>
    /// <param name="field">Which field.</param>
    /// <param name="size">The em size it states, or null for the default.</param>
    public static SheetHeaderSegment Of(SheetHeaderField field, Length? size = null)
        => new(null, field, size);

    /// <summary>True when this is a field rather than literal text.</summary>
    public bool IsField => Text is null;
}

/// <summary>
/// One of the three parts a header or footer is divided into.
/// </summary>
/// <remarks>
/// Three, and never more or fewer, in all three formats: Calc draws the left, centre and right
/// areas as three separate pieces of text in the <em>same</em> rectangle, each with its own
/// alignment (<c>ScPrintFunc::PrintHF</c>, <c>sc/source/ui/view/printfun.cxx:1874-1912</c>). So
/// they overlap when they are long, which is what a reference rendering shows and what makes
/// modelling them as one paragraph with tab stops wrong.
/// </remarks>
/// <param name="Segments">The literal runs and fields, in order.</param>
public sealed record SheetHeaderPart(IReadOnlyList<SheetHeaderSegment> Segments)
{
    /// <summary>A part with nothing in it.</summary>
    public static SheetHeaderPart Empty { get; } = new([]);

    /// <summary>True when the part draws nothing.</summary>
    public bool IsEmpty => Segments.Count == 0;

    /// <summary>
    /// The part's text with its fields filled in.
    /// </summary>
    /// <param name="context">What the fields resolve to on this page.</param>
    public string Resolve(SheetHeaderContext context)
    {
        ArgumentNullException.ThrowIfNull(context);
        if (Segments.Count == 0) return string.Empty;

        StringBuilder text = new();
        foreach (SheetHeaderSegment segment in Segments)
            text.Append(segment.Text ?? context.Value(segment.Field));

        return text.ToString();
    }

    /// <summary>
    /// The part's lines, each a list of pieces with the size each is drawn at.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Every format can write a line break into one part, and all of them print.</strong>
    /// Calc holds each area as an <c>EditTextObject</c> of several paragraphs and draws the whole
    /// object (<c>ScPrintFunc::PrintHF</c>, <c>sc/source/ui/view/printfun.cxx:1874-1912</c>);
    /// only the first line was drawn here until round thirty-one. Measured on
    /// <c>fm-provider-service-measures.xlsx</c>, whose header states three lines and whose footer
    /// two: 20944 extractable words against the reference's 21458, the whole difference being
    /// dropped lines.
    /// </para>
    /// <para>
    /// A field never contains a break, so a line boundary can only fall inside a literal —
    /// which is what lets this split without resolving the fields first.
    /// </para>
    /// </remarks>
    /// <param name="context">What the fields resolve to on this page.</param>
    public IReadOnlyList<IReadOnlyList<SheetHeaderPiece>> Lines(SheetHeaderContext context)
    {
        ArgumentNullException.ThrowIfNull(context);

        List<IReadOnlyList<SheetHeaderPiece>> lines = [];
        List<SheetHeaderPiece> current = [];

        foreach (SheetHeaderSegment segment in Segments)
        {
            if (segment.Text is null)
            {
                current.Add(new SheetHeaderPiece(context.Value(segment.Field), segment.Size));
                continue;
            }

            string[] parts = segment.Text.Split('\n');
            for (int at = 0; at < parts.Length; at++)
            {
                if (at > 0)
                {
                    lines.Add(current);
                    current = [];
                }

                if (parts[at].Length > 0)
                    current.Add(new SheetHeaderPiece(parts[at], segment.Size));
            }
        }

        lines.Add(current);

        // A trailing empty line draws nothing and takes no room; an empty line between two that
        // do draw is a blank line and keeps its height, which is why only the tail is trimmed.
        while (lines.Count > 0 && lines[^1].Count == 0) lines.RemoveAt(lines.Count - 1);

        return lines;
    }
}

/// <summary>One piece of one line of a header part: its text and the size it is drawn at.</summary>
/// <param name="Text">The text, fields already resolved.</param>
/// <param name="Size">The em size it states, or null for the sheet default.</param>
public readonly record struct SheetHeaderPiece(string Text, Length? Size);

/// <summary>What a header's fields stand for on one printed page.</summary>
/// <remarks>
/// Passed in rather than read from ambient state because two of the fields are properties of the
/// <em>printout</em> and not of the document: <see cref="PageNumber"/> is the number this page
/// carries after any restart, and <see cref="PageCount"/> is how many pages the whole job has,
/// which is not knowable until every sheet has been paginated.
/// </remarks>
public sealed class SheetHeaderContext
{
    /// <summary>The number printed on this page.</summary>
    public int PageNumber { get; init; } = 1;

    /// <summary>How many pages the printout has in total.</summary>
    public int PageCount { get; init; } = 1;

    /// <summary>The sheet's tab name.</summary>
    public string SheetName { get; init; } = string.Empty;

    /// <summary>The document's file name, extension included.</summary>
    /// <remarks>
    /// With its extension, which is measurable rather than assumed: LibreOffice's own rendering
    /// of a footer holding <c>&amp;F</c> prints <c>sheet-decor-ods.ods</c>, not
    /// <c>sheet-decor-ods</c>.
    /// </remarks>
    public string FileName { get; init; } = string.Empty;

    /// <summary>Its full path, when one is known.</summary>
    public string FilePath { get; init; } = string.Empty;

    /// <summary>The document's title from its metadata, when it states one.</summary>
    public string Title { get; init; } = string.Empty;

    /// <summary>When the printout was made.</summary>
    /// <remarks>
    /// One instant for the whole job, not one per page: <c>&amp;T</c> on a fifty-page workbook
    /// prints the time the printout started on every page of it, which is what
    /// <c>ScHeaderFieldData</c> holds and what reading the clock in each page's constructor
    /// would not give. See <see cref="SheetPrintInstant"/> for where the value comes from.
    /// </remarks>
    public DateTime Printed { get; init; } = SheetPrintInstant.Now();

    /// <summary>The text one field expands to.</summary>
    /// <param name="field">The field.</param>
    public string Value(SheetHeaderField field) => field switch
    {
        SheetHeaderField.PageNumber => PageNumber.ToString(CultureInfo.CurrentCulture),
        SheetHeaderField.PageCount => PageCount.ToString(CultureInfo.CurrentCulture),
        SheetHeaderField.Date => Printed.ToString("d", CultureInfo.CurrentCulture),
        SheetHeaderField.Time => Printed.ToString("t", CultureInfo.CurrentCulture),
        SheetHeaderField.SheetName => SheetName,
        SheetHeaderField.FileName => FileName,
        SheetHeaderField.FilePath => FilePath,
        SheetHeaderField.Title => Title.Length > 0 ? Title : FileName,
        _ => string.Empty,
    };
}

/// <summary>
/// A header or a footer: its three parts.
/// </summary>
/// <remarks>
/// <para>
/// Parsed rather than kept as the format wrote it, because the three formats spell the same
/// thing three ways and a renderer that branched on which would be the third place the field
/// list lived. SpreadsheetML and BIFF share an <c>&amp;</c>-code language;
/// <see cref="ParseCodes"/> reads it. ODF writes the regions as elements and the fields as
/// child elements, which the ODF reader turns into the same shape.
/// </para>
/// <para>
/// Not the same as a Writer header. A spreadsheet's header has no paragraphs to lay out and no
/// flow: three strings, three alignments, one line each, drawn into a band whose height is
/// already decided.
/// </para>
/// </remarks>
public sealed record SheetHeaderFooter(
    SheetHeaderPart Left, SheetHeaderPart Centre, SheetHeaderPart Right)
{
    /// <summary>A band with nothing in it.</summary>
    public static SheetHeaderFooter Empty { get; } =
        new(SheetHeaderPart.Empty, SheetHeaderPart.Empty, SheetHeaderPart.Empty);

    /// <summary>True when none of the three parts draws anything.</summary>
    public bool IsEmpty => Left.IsEmpty && Centre.IsEmpty && Right.IsEmpty;

    /// <summary>
    /// Reads the <c>&amp;</c>-code language SpreadsheetML and BIFF share.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One string holds all three parts, switched by <c>&amp;L</c>, <c>&amp;C</c> and
    /// <c>&amp;R</c>; text before any switch belongs to the centre, which is the rule Excel
    /// states and LibreOffice implements
    /// (<c>ScHFPortionParser</c>/<c>lcl_ParseHFString</c>,
    /// <c>sc/source/filter/excel/xihelper.cxx:399-520</c> — the parser starts in
    /// <c>EXC_HF_CENTER</c>).
    /// </para>
    /// <para>
    /// The formatting codes are consumed and discarded rather than ignored, which is not the
    /// same thing: <c>&amp;"Arial,Bold"</c> and <c>&amp;12</c> both swallow characters that
    /// would otherwise print. A reader that skipped only the two-character codes would show
    /// <c>Arial,Bold</c> in the header of every file Excel wrote a font into. <c>&amp;&amp;</c>
    /// is a literal ampersand, and an <c>&amp;</c> at the very end of the string is a literal
    /// one too rather than a truncated code.
    /// </para>
    /// </remarks>
    /// <param name="text">The header string as the file wrote it.</param>
    public static SheetHeaderFooter ParseCodes(string? text)
    {
        if (string.IsNullOrEmpty(text)) return Empty;

        List<SheetHeaderSegment> left = [];
        List<SheetHeaderSegment> centre = [];
        List<SheetHeaderSegment> right = [];
        List<SheetHeaderSegment> current = centre;

        StringBuilder literal = new();
        Length? size = null;
        int at = 0;

        while (at < text.Length)
        {
            char c = text[at];
            if (c != '&' || at + 1 >= text.Length)
            {
                literal.Append(c);
                at++;
                continue;
            }

            char code = text[at + 1];
            at += 2;

            switch (code)
            {
                case 'L': Flush(); current = left; break;
                case 'C': Flush(); current = centre; break;
                case 'R': Flush(); current = right; break;

                case 'P': Field(SheetHeaderField.PageNumber); break;
                case 'N': Field(SheetHeaderField.PageCount); break;
                case 'D': Field(SheetHeaderField.Date); break;
                case 'T': Field(SheetHeaderField.Time); break;
                case 'A': Field(SheetHeaderField.SheetName); break;
                case 'F': Field(SheetHeaderField.FileName); break;

                case 'Z':
                {
                    Field(SheetHeaderField.FilePath);

                    // "&Z&F" is one field and not two: Excel writes the path and the name
                    // separately and Calc swallows the second (xihelper.cxx:356-361), because
                    // its own path field already ends in the file name. A reader that took
                    // both prints the name twice.
                    if (at + 1 < text.Length && text[at] == '&' && text[at + 1] == 'F') at += 2;
                    break;
                }

                case '&': literal.Append('&'); break;

                // A newline splits a part into lines. Only the first is drawn today — see the
                // module's TODO — but it must not become a literal "n".
                case '\n': literal.Append('\n'); break;

                // Font name and style, as &"Liberation Sans,Bold" — everything to the closing
                // quotation mark is the specification and none of it prints.
                case '"':
                {
                    int end = text.IndexOf('"', at);
                    at = end < 0 ? text.Length : end + 1;
                    break;
                }

                // A font size in points, a run of digits rather than a fixed-length code. It
                // holds until the next one — Calc keeps it in the portion's item set and Excel
                // repeats it after every section switch, so carrying it across `&L`/`&C`/`&R`
                // agrees with both.
                case >= '0' and <= '9':
                {
                    int from = at - 1;
                    while (at < text.Length && char.IsAsciiDigit(text[at])) at++;
                    if (double.TryParse(
                            text.AsSpan(from, at - from), CultureInfo.InvariantCulture,
                            out double points) && points > 0)
                    {
                        Flush();
                        size = Length.FromPoints(points);
                    }

                    break;
                }

                // &K is a colour, six hex digits of it, and none of them print.
                case 'K':
                {
                    int taken = 0;
                    while (taken < 6 && at < text.Length && Uri.IsHexDigit(text[at]))
                    {
                        at++;
                        taken++;
                    }

                    break;
                }

                // Everything else is a toggle with no text — bold, italic, underline,
                // strikeout, super and subscript, the picture placeholders — or a code this
                // does not know. Both are swallowed rather than printed, which is what Calc's
                // own parser does with an unrecognised character: its switch has no default,
                // so the character is consumed and the state returns to text.
                default: break;
            }
        }

        Flush();

        return new SheetHeaderFooter(
            new SheetHeaderPart(left), new SheetHeaderPart(centre), new SheetHeaderPart(right));

        void Flush()
        {
            if (literal.Length == 0) return;
            current.Add(SheetHeaderSegment.Literal(literal.ToString(), size));
            literal.Clear();
        }

        void Field(SheetHeaderField field)
        {
            Flush();
            current.Add(SheetHeaderSegment.Of(field, size));
        }
    }
}
