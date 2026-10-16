using XRay.Office.Text.Layout;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// The break opportunities inside an EditEngine field, of which there are none.
/// </summary>
/// <remarks>
/// <para>
/// A hyperlink cell is not a string cell. Calc's OOXML import replaces the cell with an edit
/// cell holding a single <c>SvxURLField</c> (<c>sc/source/filter/oox/worksheethelper.cxx:1062</c>,
/// <c>insertHyperlink</c>), and the BIFF import does the same. The content node's string is then
/// **one** <c>EE_FEATURE_FIELD</c> character; the URL a reader sees is the field's
/// *representation*, which the node does not contain.
/// </para>
/// <para>
/// That is what decides the breaking, because <c>ImpEditEngine::ImpBreakLine</c> hands the
/// <em>node's</em> string to the break iterator
/// (<c>editeng/source/editeng/impedit3.cxx:2080-2083</c>). A one-character node offers no
/// interior opportunity at all, so the iterator returns nothing usable and EditEngine falls
/// through to <c>// No separator in line =&gt; Chop!</c> (<c>impedit3.cxx:2236-2247</c>), cutting
/// at <c>nMaxBreakPos</c> — the last character position still under the remaining width.
/// </para>
/// <para>
/// So a field is <em>atomic to the breaker and still divisible by the chop</em>, and the two
/// halves of that are separately observable. Measured against the installed 26.2.4.2 on
/// <c>dotnet/probes/sheets-wrap-01</c>, six wrap-enabled cells in one 30-character column, the
/// same three strings once plain and once hyperlinked:
/// </para>
/// <list type="bullet">
/// <item><description>
/// <c>https://www.bsp.gov.ph/Regulations/Published%20Issuances/Images/M-2024-039.pdf</c> plain
/// breaks after its solidi — <c>https://www.bsp.gov.ph/</c> ⏐ <c>Regulations/</c> ⏐ … — while the
/// same string hyperlinked breaks <c>…/Regulation</c> ⏐ <c>s/Published…M</c> ⏐ <c>-2024-039.pdf</c>,
/// mid-token and nowhere near a separator.
/// </description></item>
/// <item><description>
/// <c>alpha bravo charlie delta echo foxtrot …</c> plain breaks at its spaces; hyperlinked it
/// breaks <c>…echo foxtr</c> ⏐ <c>ot golf …</c>, cutting a word in half with a space one
/// character away. **A space is not a break opportunity inside a field**, which is the clearest
/// evidence that the node rather than the representation is what is analysed.
/// </description></item>
/// <item><description>
/// A token with no opportunity in it either way — <c>AAAABBBB…PPPP</c> — comes out
/// character-identical in both, because the chop is all either one had.
/// </description></item>
/// </list>
/// <para>
/// Row height is the one place a field really is unbreakable, and it is not a contradiction:
/// Calc's optimal-height pass measures a field cell at a single line whatever the column width
/// is. Converting the same probe with automatic row heights gives 0.6425 in (four lines) for the
/// plain URL and 0.1756 in (one line) for the hyperlinked one, and the row then simply lets the
/// wrapped field overflow it — the reference PDF draws all three lines of a field in a
/// one-line-tall row. See the note in <see cref="SheetOptimalRowHeights"/>, which keeps its own
/// single-line rule for exactly that reason.
/// </para>
/// </remarks>
internal sealed class SheetFieldBreaker : ILineBreaker
{
    /// <summary>A shared instance; the breaker holds no state.</summary>
    public static SheetFieldBreaker Instance { get; } = new();

    /// <inheritdoc/>
    /// <remarks>
    /// The end of the text and nothing else. <see cref="ILineBreaker"/> requires the end to be a
    /// break, and the fill loop needs at least one candidate to make progress before it chops.
    /// </remarks>
    public IReadOnlyList<int> FindBreakOpportunities(ReadOnlySpan<char> text, string? language = null)
        => text.Length == 0 ? [] : [text.Length];

    /// <inheritdoc/>
    /// <remarks>
    /// None. A hard break in a field's representation is a character like any other: it is not in
    /// the content node, so nothing asks the iterator about it and EditEngine cannot start a line
    /// on it.
    /// </remarks>
    public IReadOnlyList<int> FindMandatoryBreaks(ReadOnlySpan<char> text, string? language = null)
        => [];
}
