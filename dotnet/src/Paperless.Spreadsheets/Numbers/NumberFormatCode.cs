using System.Globalization;

namespace Paperless.Spreadsheets.Numbers;

/// <summary>
/// A parsed Excel-style number-format code — <c>#,##0.00</c>, <c>yyyy-mm-dd</c>,
/// <c>[$£-809]#,##0.00;[RED]-#,##0.00</c> — split into its semicolon-separated subformats.
/// </summary>
/// <remarks>
/// <para>
/// This is the format-code language Excel and Calc share, so it is deliberately not tied to
/// SpreadsheetML: XLSX carries these codes in <c>styles.xml</c>, XLS in its <c>FORMAT</c>
/// records, and both mean the same thing by them. ODF is the exception — it writes a
/// <em>structured</em> <c>number:number-style</c> instead, so nothing here applies to it.
/// </para>
/// <para>
/// Only what a caller needs to turn a stored value into the text the authoring application
/// displayed is modelled. Colours and conditions are parsed because they select which
/// subformat applies, but the colour itself is discarded: extraction produces text.
/// </para>
/// </remarks>
public sealed class NumberFormatCode
{
    private NumberFormatCode(string code, IReadOnlyList<NumberFormatSection> sections)
    {
        Code = code;
        Sections = sections;
    }

    /// <summary>The format code as written in the file.</summary>
    public string Code { get; }

    /// <summary>The subformats, in the order the code lists them.</summary>
    public IReadOnlyList<NumberFormatSection> Sections { get; }

    /// <summary>True when the code is the <c>General</c> format, which has no explicit shape.</summary>
    public bool IsGeneral => Sections.Count == 1 && Sections[0].IsGeneral;

    /// <summary>
    /// True when the format displays a date or a time, so a stored serial number should be
    /// surfaced as a <see cref="DateTime"/> or <see cref="TimeSpan"/> rather than a number.
    /// </summary>
    /// <remarks>
    /// A spreadsheet stores a date as a plain number; only the format says it is a date. The
    /// first subformat decides, because that is the one a positive serial takes and dates are
    /// never negative in practice.
    /// </remarks>
    public bool IsDateTime => Sections.Count > 0 && Sections[0].Kind == NumberFormatKind.DateTime;

    /// <summary>True when the format shows a time but no date part.</summary>
    public bool IsTimeOnly
        => IsDateTime && !Sections[0].HasDatePart && Sections[0].HasTimePart;

    /// <summary>
    /// Parses a format code. Never throws: an unparseable code degrades to <c>General</c>,
    /// because a cell with a strange format is still a cell with a value.
    /// </summary>
    public static NumberFormatCode Parse(string? code)
    {
        if (string.IsNullOrEmpty(code)) return General;

        List<NumberFormatSection> sections = [];
        foreach (string part in SplitSections(code))
        {
            sections.Add(NumberFormatSection.Parse(part));
            // Excel reads at most four subformats; a code with more is malformed and the
            // extra ones are unreachable.
            if (sections.Count == 4) break;
        }

        return sections.Count == 0 ? General : new NumberFormatCode(code, sections);
    }

    /// <summary>The <c>General</c> format, used when a cell names none.</summary>
    public static NumberFormatCode General { get; } =
        new("General", [NumberFormatSection.Parse("General")]);

    /// <summary>
    /// Chooses the subformat a numeric value takes.
    /// </summary>
    /// <remarks>
    /// Two selection schemes coexist. When any subformat carries a condition such as
    /// <c>[&gt;=100]</c> the conditions are tested in order and the first unconditional
    /// subformat is the else-branch. Otherwise the positional rule applies: with two
    /// subformats the second is for negatives, with three the third is for zero. Getting this
    /// wrong is silent — the number still formats, just through the wrong branch.
    /// </remarks>
    public NumberFormatSection SelectFor(double value)
    {
        bool conditional = false;
        foreach (NumberFormatSection section in Sections)
        {
            if (section.Condition is null) continue;
            conditional = true;
            break;
        }

        if (conditional)
        {
            NumberFormatSection? fallback = null;
            foreach (NumberFormatSection section in Sections)
            {
                if (section.Kind == NumberFormatKind.Text) continue;
                if (section.Condition is { } condition)
                {
                    if (condition.Matches(value)) return section;
                }
                else
                {
                    fallback ??= section;
                }
            }
            return fallback ?? Sections[0];
        }

        // Only the numeric subformats take part; a fourth, text-only subformat never applies
        // to a number.
        int numeric = Math.Min(Sections.Count, 3);
        if (numeric >= 3 && value == 0) return Sections[2];
        if (numeric >= 2 && value < 0) return Sections[1];
        return Sections[0];
    }

    /// <summary>
    /// The subformat a text value takes: the fourth, or null when the code has none.
    /// </summary>
    public NumberFormatSection? SelectForText()
        => Sections.Count >= 4 ? Sections[3]
           : Sections.Count == 1 && Sections[0].Kind == NumberFormatKind.Text ? Sections[0]
           : null;

    /// <summary>
    /// Splits on the semicolons that separate subformats, ignoring those inside quotes,
    /// brackets or escapes.
    /// </summary>
    /// <remarks>
    /// A naive split is the trap here: <c>[$-409]</c> and quoted literals both contain
    /// characters that look like structure, and a currency literal may legitimately hold a
    /// semicolon.
    /// </remarks>
    private static List<string> SplitSections(string code)
    {
        List<string> parts = [];
        System.Text.StringBuilder current = new();
        bool inQuotes = false;
        int brackets = 0;

        for (int i = 0; i < code.Length; i++)
        {
            char c = code[i];
            if (c == '\\' && i + 1 < code.Length)
            {
                current.Append(c).Append(code[i + 1]);
                i++;
                continue;
            }
            if (c == '"') { inQuotes = !inQuotes; current.Append(c); continue; }
            if (!inQuotes && c == '[') { brackets++; current.Append(c); continue; }
            if (!inQuotes && c == ']') { if (brackets > 0) brackets--; current.Append(c); continue; }
            if (!inQuotes && brackets == 0 && c == ';')
            {
                parts.Add(current.ToString());
                current.Clear();
                continue;
            }
            current.Append(c);
        }
        parts.Add(current.ToString());
        return parts;
    }
}

/// <summary>What a subformat produces.</summary>
public enum NumberFormatKind
{
    /// <summary>The implicit format: the value rendered as compactly as it round-trips.</summary>
    General,

    /// <summary>A number: digit placeholders, grouping, decimals, percent, exponent, fraction.</summary>
    Number,

    /// <summary>A date, a time, or both.</summary>
    DateTime,

    /// <summary>Literal text, possibly with the <c>@</c> placeholder.</summary>
    Text,
}

/// <summary>
/// A condition such as <c>[&gt;=100]</c> that decides whether a subformat applies.
/// </summary>
/// <param name="Comparison">The operator, as written.</param>
/// <param name="Operand">The number compared against.</param>
public readonly record struct NumberFormatCondition(string Comparison, double Operand)
{
    /// <summary>True when a value satisfies the condition.</summary>
    public bool Matches(double value) => Comparison switch
    {
        "<" => value < Operand,
        "<=" => value <= Operand,
        ">" => value > Operand,
        ">=" => value >= Operand,
        "<>" => value != Operand,
        _ => value == Operand,
    };

    /// <summary>Parses the body of a bracket, or null when it is not a condition.</summary>
    public static NumberFormatCondition? TryParse(string body)
    {
        ArgumentNullException.ThrowIfNull(body);
        int i = 0;
        while (i < body.Length && (body[i] is '<' or '>' or '=')) i++;
        if (i == 0 || i > 2) return null;

        string comparison = body[..i];
        if (comparison is not ("<" or ">" or "=" or "<=" or ">=" or "<>")) return null;

        return double.TryParse(body[i..].Trim(), NumberStyles.Float, CultureInfo.InvariantCulture,
                               out double operand)
            ? new NumberFormatCondition(comparison, operand)
            : null;
    }
}
