using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.MsBinary;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a BIFF <c>XF</c>'s "attribute used" flags do and do not decide.
/// </summary>
/// <remarks>
/// <para>
/// The flags read as though they gate the record's own bytes, and for a cell <c>XF</c> they do
/// not. <c>XclImpXF::CreatePattern</c> (<c>sc/source/filter/excel/xistyle.cxx:1291-1294</c>)
/// turns each flag on whenever the parent style states nothing or states something different,
/// so the flag survives clear only when the parent states the *same* thing — and the attribute
/// then inherited through the style sheet is that same thing. Both branches end on the record's
/// own bytes.
/// </para>
/// <para>
/// Measured, not reasoned: <c>7-memento-2015-transports-aeriens-b.xls</c> carries an <c>XF</c>
/// (index 115) with a thin <c>#0066CC</c> box, the border flag clear and a parent style holding
/// the identical box. Honouring the flag literally dropped that box from 49 rows of page 2 and
/// let the off-page neighbour's <c>#003366</c> left edge take the trailing vertical instead:
/// 18,061 px of <c>#0066CC</c> and 847 px of <c>#003366</c> against the reference's 63,765 and 1.
/// </para>
/// </remarks>
public sealed class XlsStyleParentBorderTests
{
    private const int Thin = 1;                 // EXC_LINE_THIN
    private const int Blue = 30;                // default palette #0066CC
    private const int Solid = 1;                // EXC_PATT_SOLID
    private const int Pink = 45;                // default palette #FF99CC

    /// <summary>A thin box in palette entry 30, with the used flags as given.</summary>
    private static XlsXfDecoration Box(bool statesBorder, bool statesArea = false)
        => new(Thin, Thin, Thin, Thin, Blue, Blue, Blue, Blue, Solid, Pink, Pink,
            statesBorder, statesArea);

    [Fact]
    public void ACellFormatPaintsItsBorderWhenTheUsedFlagIsClear()
    {
        XlsDecorationTable table = new();
        table.Add(Box(statesBorder: true, statesArea: true), isCellXf: false, parentIndex: 4095);
        table.Add(Box(statesBorder: false), isCellXf: true, parentIndex: 0);

        SheetCellDecoration cell = table.FormatOf(1);

        cell.Borders.IsNone.ShouldBeFalse();
        cell.Borders.Left.Colour.ShouldBe(Colour.FromRgb(0x0066CC));
        cell.Borders.Right.Colour.ShouldBe(Colour.FromRgb(0x0066CC));
    }

    [Fact]
    public void ACellFormatPaintsItsFillWhenTheUsedFlagIsClear()
    {
        XlsDecorationTable table = new();
        table.Add(Box(statesBorder: true, statesArea: true), isCellXf: false, parentIndex: 4095);
        table.Add(Box(statesBorder: false), isCellXf: true, parentIndex: 0);

        table.FormatOf(1).Background.ShouldBe(Colour.FromRgb(0xFF99CC));
    }

    [Fact]
    public void AFormatWhoseParentDoesNotExistStillHonoursTheClearedFlag()
    {
        // GetXF returns null for a parent index outside the table, Calc skips the block that
        // turns the flag on, and the attribute stays unset. 4095 is the BIFF "no parent" value
        // and is the common way to land here.
        XlsDecorationTable table = new();
        table.Add(Box(statesBorder: false), isCellXf: true, parentIndex: 4095);

        table.FormatOf(0).IsNone.ShouldBeTrue();
    }

    [Fact]
    public void AStyleFormatIsDecidedByItsOwnFlagAlone()
    {
        // A style XF has no parent to compare against — IsCellXF() gates the whole block — so
        // for it the flag really is the answer.
        XlsDecorationTable table = new();
        table.Add(Box(statesBorder: false), isCellXf: false, parentIndex: 0);

        table.FormatOf(0).IsNone.ShouldBeTrue();
    }
}
