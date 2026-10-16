using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// What one of a pivot table's generated cell styles changes about the text under it.
/// </summary>
/// <remarks>
/// <para>
/// A pivot table's cells are not formatted by the file. <c>ScDPOutput::Output</c> clears the
/// whole output range and then applies four styles it creates on demand —
/// <c>lcl_SetStyleById</c>, <c>sc/source/core/data/dpoutput.cxx</c>:264-295 — of which only
/// three carry anything at all: <c>Pivot Table Result</c> and <c>Pivot Table Title</c> are bold,
/// <c>Pivot Table Category</c> and <c>Pivot Table Title</c> are left justified, and
/// <c>Pivot Table Value</c>, <c>Field</c>, <c>Corner</c> and <c>Top</c> state nothing. Beside
/// them one attribute is applied directly: a row field that is not the innermost indents its
/// member cell by <c>13 px</c> per level (<c>:1136-1140</c>), which is 195 twips and the
/// <c>fo:margin-left="0.1354in"</c> the reference's own <c>.fods</c> of such a workbook writes.
/// </para>
/// <para>
/// A differential record for the same reason <see cref="SheetConditionalText"/> is one: the
/// styles state three properties between them and are silent about everything else, so a null
/// here is that silence. What fills that silence over a pivot's own rectangle is <em>not</em>
/// the cell's own format: Calc empties the range first —
/// <c>clearContents(… HARDATTR | STYLES …)</c> at
/// <c>sc/source/filter/oox/pivottablebuffer.cxx</c>:1336 over the stated range and
/// <c>DeleteAreaTab(…, InsertDeleteFlags::ALL)</c> at <c>dpoutput.cxx</c>:1226 over the computed
/// one — so <c>XlsxPivotGrid</c> states all three properties for every cell of that
/// rectangle, taking the sheet's own default format where the generated style says nothing.
/// Measured over the nineteen pivot rectangles of the corpus's eleven pivot-bearing workbooks:
/// 330 of their 9970 cells state an alignment or an indent, merging disagrees with 26.2.4.2's
/// own resolved view on 81 of the alignments and 98 of the indents, and replacing agrees on all
/// 9970 (<c>probes/pivot-res-r108/clearing-census.py</c>).
/// </para>
/// <para>
/// <strong>All five are cleared, and what puts most of them back is the pivot's own
/// <c>&lt;format&gt;</c> records.</strong> <c>ScDPOutput::Output</c> ends with
/// <c>maFormatOutput.apply</c> (<c>dpoutput.cxx</c>:1190), which lays those records over the
/// generated styles — and they are not rare: <strong>286 across 12 of the corpus's 28 pivot
/// parts</strong> (<c>probes/pivot-fmt-r110/format-census.py</c>, correcting an r108 census that
/// searched for <c>&lt;format&gt;</c> as a direct child of <c>pivotTableDefinition</c> where it
/// is a child of <c>&lt;formats&gt;</c>). <see cref="Dxf"/> carries what they say and
/// <c>XlsxPivotFormats</c> works out which cells they land on.
/// </para>
/// <para>
/// <strong>That the clearing is total is a measurement and not a reading.</strong>
/// <c>033_Event_planning_tracker</c> with its <c>&lt;formats&gt;</c> element deleted and nothing
/// else changed comes back from 26.2.4.2 with all ninety-one cells of its pivot at the
/// <c>Default</c> cell style, and with the element back they carry a 12 pt <c>Consolas</c> on a
/// black ground.
/// </para>
/// <para>
/// Scored by value against 26.2.4.2's own resolved view of the 9878 cells of the corpus's
/// nineteen generatable pivot rectangles — <c>probes/pivot-fmt-r110/check-dxf.py</c>, where
/// <em>merge</em> is what this tree did before r110 and <em>clear</em> is clearing with no
/// <c>dxf</c> half:
/// </para>
/// <list type="table">
/// <item><description>face — merge 9877, clear 9878, clear+dxf <strong>9878</strong></description></item>
/// <item><description>size — merge 9872, clear 9783, clear+dxf <strong>9878</strong></description></item>
/// <item><description>colour — merge 9876, clear 9873, clear+dxf <strong>9878</strong></description></item>
/// <item><description>fill — merge 9806, clear 9752, clear+dxf <strong>9869</strong></description></item>
/// </list>
/// <para>
/// So clearing on its own is worse than merging for the size and the colour, and only the two
/// halves together beat both. The nine cells still wrong are one workbook's field-button row and
/// corner, where the reference paints a fill this does not.
/// </para>
/// <para>
/// <strong>The alignment and the indent are still the generated styles' and take nothing from a
/// <c>dxf</c>.</strong> That is measured too: <c>033</c> states nine alignment <c>dxf</c>s, three
/// of which match its whole data area, and 26.2.4.2's automatic style for those cells carries no
/// <c>fo:text-align</c> and no <c>fo:margin-left</c> — which is why r108's 0 disagreements over
/// 9970 cells on those two properties still stand. Borders are nil on this corpus
/// (<c>probes/pivot-res-r108/statedborders.py</c>) and number formats are not modelled.
/// </para>
/// </remarks>
public readonly record struct SheetPivotStyle
{
    /// <summary>The weight on the usual 100–900 scale, or null to keep the cell's own.</summary>
    public int? FontWeight { get; init; }

    /// <summary>How the text sits across the column, or null to keep the cell's own.</summary>
    public SheetHorizontalAlignment? Horizontal { get; init; }

    /// <summary>The indent from the aligned edge, or null to keep the cell's own.</summary>
    public Length? Indent { get; init; }

    /// <summary>
    /// The format the emptied rectangle falls back to, or null for a cell outside one.
    /// </summary>
    /// <remarks>
    /// Not the cell's own and not <c>cellXfs[0]</c>: the <c>Normal</c> <c>cellStyleXf</c>, which
    /// is what Calc calls the <c>Default</c> cell style — see
    /// <c>XlsxCellFormats.NormalStyleXf</c>, which records the probe workbook that settles it.
    /// The face, the declared class, the size and the colour are all taken from it, and
    /// <see cref="Dxf"/> then puts back whatever the pivot's own records state.
    /// </remarks>
    public SheetCellFormat? Cleared { get; init; }

    /// <summary>
    /// What the pivot's own <c>&lt;format&gt;</c> records put back over the cleared base.
    /// </summary>
    /// <remarks>
    /// Applied last, so a record's weight beats the generated <c>Pivot Table Result</c> bold —
    /// which is the order <c>ScDPOutput::Output</c> applies them in.
    /// </remarks>
    public SheetPivotDxf Dxf { get; init; }

    /// <summary>True when the style changes nothing about the text.</summary>
    public bool IsNone
        => FontWeight is null && Horizontal is null && Indent is null && Cleared is null
           && Dxf.IsNone;

    /// <summary>Lays this style over what the cell states.</summary>
    /// <param name="stated">The format the cell resolves to on its own.</param>
    public SheetCellFormat Over(SheetCellFormat stated)
    {
        ArgumentNullException.ThrowIfNull(stated);
        if (IsNone) return stated;

        // One `with`: `At` is called for every drawn cell of every page and a pivot's rectangle
        // can be ten thousand cells, so the cleared base, the generated style and the pivot's own
        // records are folded into the same copy rather than allocating three.
        SheetCellFormat over = Cleared ?? stated;

        return stated with
        {
            FontFamily = Dxf.FontFamily ?? over.FontFamily,
            DeclaredFontClass = Dxf.FontFamily is not null
                ? Dxf.DeclaredFontClass ?? FontFamilyClass.Unknown
                : over.DeclaredFontClass,
            FontSize = Dxf.FontSize ?? over.FontSize,
            Colour = Dxf.Colour ?? over.Colour,
            FontWeight = Dxf.FontWeight ?? FontWeight ?? over.FontWeight,
            Horizontal = Horizontal ?? over.Horizontal,
            Indent = Indent ?? over.Indent,
        };
    }
}

/// <summary>
/// What one of a pivot table's own <c>&lt;format&gt;</c> records changes about a cell's text.
/// </summary>
/// <remarks>
/// <para>
/// A differential like <see cref="SheetPivotStyle"/> itself, and applied over it: the record is
/// laid on last, so a <c>dxf</c> stating a weight beats the generated <c>Pivot Table Result</c>
/// bold. Its fill is not here — that goes into the sheet's decoration beside the generated
/// borders, because a background is not a property of the text.
/// </para>
/// <para>
/// Four of the six things a <c>dxf</c> can state are modelled and two are not, each for a
/// measured reason: an <c>&lt;alignment&gt;</c> reaches no cell in 26.2.4.2 and a
/// <c>&lt;border&gt;</c> is nil on this corpus. See <c>XlsxPivotFormats</c>.
/// </para>
/// </remarks>
public readonly record struct SheetPivotDxf
{
    /// <summary>The face the record names, or null when it names none.</summary>
    public string? FontFamily { get; init; }

    /// <summary>The generic class qualifying that face.</summary>
    public FontFamilyClass? DeclaredFontClass { get; init; }

    /// <summary>The em size, or null.</summary>
    public Length? FontSize { get; init; }

    /// <summary>The text colour, or null.</summary>
    public Colour? Colour { get; init; }

    /// <summary>The weight on the usual 100–900 scale, or null.</summary>
    public int? FontWeight { get; init; }

    /// <summary>True when the record changes nothing about the text.</summary>
    public bool IsNone
        => FontFamily is null && FontSize is null && Colour is null && FontWeight is null;
}
