namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// One glyph an <c>iconSet</c> conditional format can paint over a cell.
/// </summary>
/// <remarks>
/// <para>
/// The reference names its glyphs rather than deriving them: <c>ScIconSetFormat::getIconName</c>
/// (<c>sc/source/core/data/colorscale.cxx</c>:1553-1570, this tree) looks a
/// (<c>ScIconSetType</c>, index) pair up in <c>aBitmapMap</c> (<c>:1497-1521</c>) and answers the
/// name of one of the application's own icon-theme assets, which <c>getBitmap</c> then loads.
/// The name is therefore the whole of what the file decides; the artwork is the application's.
/// </para>
/// <para>
/// <strong>This enum is deliberately not the reference's twenty-two sets.</strong> It is the
/// seven <em>distinct assets</em> that 26.2.4.2 actually paints anywhere in the corpus, which is
/// a much smaller list than the sets that name them, because eighteen of the corpus's twenty
/// rules are <c>custom="1"</c> and override most of their own buckets with
/// <c>NoIcons</c>. Extracted from 26.2.4.2's own renderings — every small image in the reference
/// PDFs of the ten documents that state a rule is one of these seven, and each matches its
/// <c>colibre</c> asset to a mean absolute channel difference of 0.01-0.06 out of 255. See
/// <c>probes/iconset-r103/results.md</c> §3 and <c>reference-glyphs.txt</c>.
/// </para>
/// </remarks>
public enum SheetIconGlyph
{
    /// <summary>
    /// A bucket that resolves to an asset this tree does not draw.
    /// </summary>
    /// <remarks>
    /// Not "no icon": the reference has an <c>ScIconSetInfo</c> for such a cell, so
    /// <c>showValue="0"</c> still takes the cell's own number off the page
    /// (<c>sc/source/ui/view/output2.cxx</c>:1691-1698). A cell with no icon at all — a
    /// non-numeric one, a rule with fewer than three entries, or a <c>NoIcons</c> bucket — is
    /// recorded by having no <see cref="SheetIcon"/> at all rather than by this member.
    /// <strong>No corpus document reaches it</strong>; see the write-up's table of the fifteen
    /// unreached sets.
    /// </remarks>
    Unpainted,

    /// <summary>A dark-grey flagpole with a red banner. <c>3Flags</c> index 0.</summary>
    FlagRed,

    /// <summary>The same flag in amber. <c>3Flags</c> index 1.</summary>
    FlagAmber,

    /// <summary>The same flag in green. <c>3Flags</c> index 2.</summary>
    FlagGreen,

    /// <summary>A red-outlined pink diamond. <c>3Signs</c> index 0.</summary>
    Diamond,

    /// <summary>A white cross in a red disc. <c>3Symbols</c> and <c>3Symbols2</c> index 0.</summary>
    CrossInRed,

    /// <summary>
    /// A white exclamation mark in an amber disc. <c>3Symbols</c> and <c>3Symbols2</c> index 1.
    /// </summary>
    ExclamationInAmber,

    /// <summary>A white tick in a green disc. <c>3Symbols</c> and <c>3Symbols2</c> index 2.</summary>
    TickInGreen,
}

/// <summary>
/// The icon an <c>iconSet</c> conditional format draws over one cell.
/// </summary>
/// <remarks>
/// <para>
/// The reference's <c>ScIconSetInfo</c> (<c>sc/inc/fillinfo.hxx</c>:88-97), minus the two fields
/// that are recomputed at drawing time. It exists exactly when
/// <c>ScIconSetFormat::GetIconSetInfo</c> (<c>sc/source/core/data/colorscale.cxx</c>:1186-1253)
/// returns non-null, and that function has four ways of returning nothing: a cell that is not
/// numeric (<c>:1189-1190</c>), a rule with fewer than three entries (<c>:1195-1196</c>), a value
/// no entry's <c>Compare</c> accepts (<c>:1218-1219</c>), and a custom bucket spelt
/// <c>NoIcons</c>, whose index is stored as −1 by the importer
/// (<c>sc/source/filter/oox/condformatbuffer.cxx</c>:456-467) and answered as null at
/// <c>:1236-1239</c>.
/// </para>
/// <para>
/// The fourth is the one that matters for the corpus: <strong>fourteen of the corpus's twenty
/// rules override at least one bucket with <c>NoIcons</c></strong>, and such a cell keeps its own
/// text <em>however <see cref="ShowValue"/> is set</em>, because the null means
/// <c>DrawStrings</c> never sees an icon to suppress it for.
/// </para>
/// </remarks>
public readonly record struct SheetIcon
{
    /// <summary>Which glyph the cell's bucket resolved to.</summary>
    public SheetIconGlyph Glyph { get; init; }

    /// <summary>Whether the cell's own value is still drawn beside the icon.</summary>
    /// <remarks>
    /// <c>showValue</c> on the rule, defaulting to <strong>true</strong>
    /// (<c>condformatbuffer.cxx</c>:421). False does not indent or hide the number, it removes it:
    /// <c>ScOutputData::DrawStrings</c> clears <c>bDoCell</c>
    /// (<c>sc/source/ui/view/output2.cxx</c>:1696-1697) after the row's height is already settled,
    /// so the layout keeps the cell and only the paint drops it — the same shape as
    /// <see cref="SheetDataBar.ShowValue"/>.
    /// </remarks>
    public bool ShowValue { get; init; }
}
