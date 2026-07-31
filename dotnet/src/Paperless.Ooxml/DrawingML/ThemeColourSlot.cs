namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// One of the twelve slots a DrawingML colour scheme stores.
/// </summary>
/// <remarks>
/// <para>
/// The slots are named for the scheme's own child elements — <c>dk1 lt1 dk2 lt2 accent1…6
/// hlink folHlink</c> — rather than for <c>bg1</c>/<c>tx1</c>, because the dark-and-light pair
/// is what a theme actually holds. Which slot a document means by "background 1" is a separate
/// question, answered by <see cref="DrawingColourMap"/> and not by this enumeration.
/// </para>
/// <para>
/// The numbering is <c>model::ThemeColorType</c>'s
/// (<c>docmodel/inc/docmodel/theme/ThemeColorType.hxx</c>), so a format that stores a bare
/// index — XLSX's <c>theme="4"</c> above all — lands on the same slot LibreOffice picks.
/// </para>
/// </remarks>
public enum ThemeColourSlot
{
    /// <summary>The first dark colour; a document's <c>tx1</c>/<c>text1</c> normally maps here.</summary>
    Dark1 = 0,

    /// <summary>The first light colour; a document's <c>bg1</c>/<c>background1</c> normally maps here.</summary>
    Light1 = 1,

    /// <summary>The second dark colour.</summary>
    Dark2 = 2,

    /// <summary>The second light colour.</summary>
    Light2 = 3,

    /// <summary>Accent 1.</summary>
    Accent1 = 4,

    /// <summary>Accent 2.</summary>
    Accent2 = 5,

    /// <summary>Accent 3.</summary>
    Accent3 = 6,

    /// <summary>Accent 4.</summary>
    Accent4 = 7,

    /// <summary>Accent 5.</summary>
    Accent5 = 8,

    /// <summary>Accent 6.</summary>
    Accent6 = 9,

    /// <summary>An unvisited hyperlink.</summary>
    Hyperlink = 10,

    /// <summary>A visited hyperlink.</summary>
    FollowedHyperlink = 11,
}

/// <summary>
/// The names the OOXML families spell scheme slots with, and what each one means.
/// </summary>
/// <remarks>
/// <para>
/// Twenty-six names for twelve slots, because each family invented its own spelling and none
/// removed the others: DrawingML writes <c>dk1</c> and <c>bg1</c>, WordprocessingML's
/// <c>w:themeColor</c> writes <c>dark1</c> and <c>background1</c>, and <c>a:clrMap</c> writes
/// <c>tx1</c>. LibreOffice keeps the same table in <c>oox/source/drawingml/color.cxx</c>
/// (<c>constSchemeColorNameToIndex</c>, line 212), which is where these pairings come from.
/// </para>
/// <para>
/// What the table does <em>not</em> say is that <c>bg1</c> is <c>lt1</c>. It gives the slot a
/// name refers to <em>with no colour map in force</em>, which is the same thing only when the
/// map is the identity. <see cref="MapKey"/> is what tells the two apart.
/// </para>
/// </remarks>
public static class ThemeColourSlots
{
    private static readonly Dictionary<string, ThemeColourSlot> ByName =
        new(StringComparer.Ordinal)
        {
            ["dk1"] = ThemeColourSlot.Dark1,
            ["dark1"] = ThemeColourSlot.Dark1,
            ["tx1"] = ThemeColourSlot.Dark1,
            ["t1"] = ThemeColourSlot.Dark1,
            ["text1"] = ThemeColourSlot.Dark1,
            ["lt1"] = ThemeColourSlot.Light1,
            ["light1"] = ThemeColourSlot.Light1,
            ["bg1"] = ThemeColourSlot.Light1,
            ["background1"] = ThemeColourSlot.Light1,
            ["dk2"] = ThemeColourSlot.Dark2,
            ["dark2"] = ThemeColourSlot.Dark2,
            ["tx2"] = ThemeColourSlot.Dark2,
            ["t2"] = ThemeColourSlot.Dark2,
            ["text2"] = ThemeColourSlot.Dark2,
            ["lt2"] = ThemeColourSlot.Light2,
            ["light2"] = ThemeColourSlot.Light2,
            ["bg2"] = ThemeColourSlot.Light2,
            ["background2"] = ThemeColourSlot.Light2,
            ["accent1"] = ThemeColourSlot.Accent1,
            ["accent2"] = ThemeColourSlot.Accent2,
            ["accent3"] = ThemeColourSlot.Accent3,
            ["accent4"] = ThemeColourSlot.Accent4,
            ["accent5"] = ThemeColourSlot.Accent5,
            ["accent6"] = ThemeColourSlot.Accent6,
            ["hlink"] = ThemeColourSlot.Hyperlink,
            ["hyperlink"] = ThemeColourSlot.Hyperlink,
            ["folHlink"] = ThemeColourSlot.FollowedHyperlink,
            ["followedHyperlink"] = ThemeColourSlot.FollowedHyperlink,
        };

    /// <summary>
    /// The slot a scheme-colour name names when no colour map is in force, or null when the
    /// name is not one of the twelve — <c>phClr</c> and <c>none</c> both land here.
    /// </summary>
    public static ThemeColourSlot? Parse(string? name)
        => name is not null && ByName.TryGetValue(name, out ThemeColourSlot slot) ? slot : null;

    /// <summary>
    /// The colour-map key a name goes through, or null when it addresses the theme's storage
    /// directly and is never remapped.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A colour map is keyed by the twelve <em>document-facing</em> names — <c>bg1 tx1 bg2 tx2
    /// accent1…6 hlink folHlink</c> — and its values are the theme's own slot names. So
    /// <c>bg1</c> is mapped and <c>lt1</c> is not, which is precisely why both spellings exist:
    /// a slide master that swaps light for dark still wants <c>lt1</c> to mean the light colour
    /// while <c>bg1</c> follows the swap.
    /// </para>
    /// <para>
    /// The long WordprocessingML spellings normalise onto the same keys, because Word's
    /// <c>w:clrSchemeMapping</c> is the same map under different attribute names.
    /// </para>
    /// </remarks>
    public static string? MapKey(string? name) => name switch
    {
        "bg1" or "background1" => "bg1",
        "tx1" or "t1" or "text1" => "tx1",
        "bg2" or "background2" => "bg2",
        "tx2" or "t2" or "text2" => "tx2",
        "accent1" or "accent2" or "accent3" or "accent4" or "accent5" or "accent6" => name,
        "hlink" or "hyperlink" => "hlink",
        "folHlink" or "followedHyperlink" => "folHlink",
        _ => null,
    };
}
