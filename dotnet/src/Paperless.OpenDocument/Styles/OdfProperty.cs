using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.OpenDocument.Styles;

/// <summary>
/// Where a resolved property's value came from.
/// </summary>
/// <remarks>
/// <para>
/// This distinction is the point of the whole resolver, and it is deliberately not
/// collapsed into "has a value". It reproduces LibreOffice's <c>SfxItemState</c>
/// (<c>dotnet/research/05-infrastructure.md</c> section E), which keeps the same three
/// cases apart for the same reason: later code has to be able to ask "was this set on the
/// object itself, or merely inherited?"
/// </para>
/// <para>
/// Two concrete cases where it matters. In ODF, direct formatting is not an inline
/// attribute — it is an automatic style — so "set on the automatic style the run points
/// at" is the only evidence that a user made a local change rather than picking a named
/// style, which is exactly what a caller reporting "hard formatting" needs. And a
/// property that is only defaulted may legitimately be overridden by a different
/// mechanism (a presentation placeholder, a table's own defaults), whereas one that was
/// explicitly set must win.
/// </para>
/// </remarks>
public enum OdfPropertyOrigin
{
    /// <summary>Nothing in the chain set the property. There is no value.</summary>
    Unset = 0,

    /// <summary>Set on the style that was asked about.</summary>
    SetHere,

    /// <summary>Set on an ancestor reached through <c>style:parent-style-name</c>.</summary>
    Inherited,

    /// <summary>Came from the family's <c>style:default-style</c>.</summary>
    Defaulted,
}

/// <summary>
/// One resolved ODF formatting property: its value, where it came from, and which style
/// supplied it.
/// </summary>
/// <param name="Value">
/// The raw attribute value, or null when nothing set the property. Raw rather than typed
/// because ODF property values span a dozen syntaxes and the typed accessors on this
/// struct convert only what a caller actually asks for.
/// </param>
/// <param name="Origin">Whether the value was set here, inherited, or defaulted.</param>
/// <param name="SourceStyleName">
/// The style that supplied the value, which is useful in diagnostics: "the bold came from
/// <c>Heading_20_1</c>" is a far more actionable message than "the text is bold".
/// </param>
public readonly record struct OdfProperty(
    string? Value,
    OdfPropertyOrigin Origin,
    string? SourceStyleName = null)
{
    /// <summary>A property nothing in the chain set.</summary>
    public static OdfProperty Unset => default;

    /// <summary>True when some style in the chain supplied a value.</summary>
    public bool HasValue => Origin != OdfPropertyOrigin.Unset;

    /// <summary>
    /// True when the value was set on the style asked about rather than reached through
    /// inheritance or defaulting — ODF's stand-in for "this is direct formatting".
    /// </summary>
    public bool IsSetHere => Origin == OdfPropertyOrigin.SetHere;

    /// <summary>The value as a <see cref="Length"/>, or null when absent or unparseable.</summary>
    public Length? AsLength() => OdfValue.ParseLength(Value);

    /// <summary>The value as a fraction (<c>115%</c> becomes 1.15), or null.</summary>
    public double? AsPercentage() => OdfValue.ParsePercentage(Value);

    /// <summary>The value as a number, or null.</summary>
    public double? AsDouble() => OdfValue.ParseDouble(Value);

    /// <summary>The value as an integer, or null.</summary>
    public int? AsInt() => OdfValue.ParseInt(Value);

    /// <summary>The value as a boolean, or null.</summary>
    public bool? AsBoolean() => OdfValue.ParseBoolean(Value);

    /// <summary>The value as a colour, or null.</summary>
    public Colour? AsColour() => OdfValue.ParseColour(Value);

    /// <summary>
    /// True when the value equals <paramref name="candidate"/>. Enumerated ODF values are
    /// case-sensitive, so the comparison is ordinal.
    /// </summary>
    public bool Is(string candidate) => string.Equals(Value, candidate, StringComparison.Ordinal);

    /// <inheritdoc/>
    public override string ToString() => Origin == OdfPropertyOrigin.Unset
        ? "<unset>"
        : SourceStyleName is null
            ? $"{Value} ({Origin})"
            : $"{Value} ({Origin} from '{SourceStyleName}')";
}
