using XRay.Office.MsBinary.Escher;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A binary PowerPoint table, which the file writes as a plain group of rectangles, and the one
/// thing the reference throws away when it turns that group into a table: the shrink-to-fit.
/// </summary>
/// <remarks>
/// <para>
/// <c>CreateTable</c> (<c>filter/source/msfilter/svdfppt.cxx</c>:7569, reached from <c>:2913</c>)
/// replaces the whole group with one <c>SdrTableObj</c> and copies each member's
/// <c>OutlinerParaObject</c> into a cell. Nothing else of the member's item set survives:
/// <c>ApplyCellAttributes</c> (<c>:7412</c>) carries the four text distances, the vertical and
/// horizontal adjusts, the writing mode and the fill, and <c>SDRATTR_TEXT_FITTOSIZE</c> is not
/// among them — <c>svx/source/table</c> mentions autofit nowhere at all. So the fit that
/// <c>:1099</c> gave every Body-kind rectangle is gone by the time the text is drawn.
/// </para>
/// <para>
/// <strong>Confirmed against 26.2.4.2's own output as well as its source.</strong> In its flat
/// ODP of <c>slides/ceiling-001/ppt/Thailand17.ppt</c> the deck's autofitted bodies export
/// <c>style:shrink-to-fit="true"</c> and none of page 11's 54 <c>table:table-cell</c> styles
/// does, every one of its 42 spans stating a flat <c>fo:font-size="12pt"</c>; in its PDF of the
/// same file all 77 of that page's body spans are drawn at 11.99 pt. This tree drew the 35 spans
/// whose cell wraps to two lines at 11.00 — <c>round(12 × 0.925)</c>, the second row of
/// <c>constScaleLevels</c> — and after this reads 548 alphanumerics at 11.99 against the
/// reference's 548, exactly.
/// </para>
/// </remarks>
public class PptTableCellFitTests
{
    /// <summary>A shape whose tertiary property table holds the entries given.</summary>
    /// <remarks>
    /// The table is the six-byte-per-entry form <see cref="EscherPropertyTable"/> describes, with
    /// each complex value's block appended in property order after the fixed entries.
    /// </remarks>
    private static EscherShape Group(params (ushort Id, uint Value, byte[]? Data)[] entries)
    {
        List<byte> bytes = [];
        foreach ((ushort id, uint value, byte[]? data) in entries)
        {
            ushort raw = (ushort)(id | (data is null ? 0 : 0x8000));
            bytes.Add((byte)(raw & 0xFF));
            bytes.Add((byte)(raw >> 8));
            bytes.AddRange(BitConverter.GetBytes(data is null ? value : (uint)data.Length));
        }

        foreach ((ushort _, uint _, byte[]? data) in entries)
        {
            if (data is not null) bytes.AddRange(data);
        }

        return new EscherShape
        {
            Flags = EscherShapeAttributes.Group,
            TertiaryProperties = EscherPropertyTable.Read([.. bytes], entries.Length),
        };
    }

    /// <summary>Six bytes of counts and one word per row, which is what a real one holds.</summary>
    private static byte[] Rows(int count)
        => [.. BitConverter.GetBytes((ushort)count), 0, 0, 0, 0, .. new byte[4 * count]];

    // --------------------------------------------------------------- recognising the table group

    [Fact]
    public void AGroupStatingBothTablePropertiesIsATable()
    {
        // The shape of every one of the eight groups in `Thailand17.ppt`: `tableProperties` 1 in
        // the tertiary table beside a complex `tableRowProperties` of 6 + 4 x rows bytes.
        PptSlideLayout.IsTableGroup(Group(
            (PptShapeGeometry.TableProperties, 1, null),
            (PptShapeGeometry.TableRowProperties, 0, Rows(9)))).ShouldBeTrue();
    }

    /// <summary>
    /// Either low bit of <c>tableProperties</c> will do, and no other bit will.
    /// </summary>
    /// <remarks>
    /// The reference's test is <c>nTableProperties &amp; 3</c> (<c>svdfppt.cxx</c>:1210), so a
    /// value of 4 — or of 0, which is also what an absent property reads as — is not a table.
    /// </remarks>
    [Theory]
    [InlineData(0u, false)]
    [InlineData(1u, true)]
    [InlineData(2u, true)]
    [InlineData(3u, true)]
    [InlineData(4u, false)]
    [InlineData(8u, false)]
    public void OnlyTheTwoLowBitsOfTablePropertiesMakeATable(uint value, bool expected)
    {
        PptSlideLayout.IsTableGroup(Group(
            (PptShapeGeometry.TableProperties, value, null),
            (PptShapeGeometry.TableRowProperties, 0, Rows(3)))).ShouldBe(expected);
    }

    [Fact]
    public void APropertyWithoutItsRowArrayIsNotATable()
    {
        // `SeekToContent(DFF_Prop_tableRowProperties)` has to succeed as well (`:1212`), and a
        // complex property whose length is zero has no content to seek to.
        PptSlideLayout.IsTableGroup(Group(
            (PptShapeGeometry.TableProperties, 1, null))).ShouldBeFalse();

        PptSlideLayout.IsTableGroup(Group(
            (PptShapeGeometry.TableProperties, 1, null),
            (PptShapeGeometry.TableRowProperties, 0, []))).ShouldBeFalse();
    }

    [Fact]
    public void AGroupStatingNeitherPropertyIsNotATable()
    {
        PptSlideLayout.IsTableGroup(new EscherShape()).ShouldBeFalse();
    }

    /// <summary>
    /// The properties are read from the <em>tertiary</em> table and from no other.
    /// </summary>
    /// <remarks>
    /// The reference seeks to <c>DFF_msofbtUDefProp</c> for them (<c>:1204</c>), which is where a
    /// host writes properties of its own invention; the ordinary <c>msofbtOPT</c> table numbers
    /// 927 and 928 for something else entirely.
    /// </remarks>
    [Fact]
    public void TheOrdinaryPropertyTableDoesNotStateATable()
    {
        EscherShape shape = Group(
            (PptShapeGeometry.TableProperties, 1, null),
            (PptShapeGeometry.TableRowProperties, 0, Rows(4)));

        PptSlideLayout.IsTableGroup(new EscherShape
        {
            Flags = EscherShapeAttributes.Group,
            Properties = shape.TertiaryProperties,
        }).ShouldBeFalse();
    }

    // ------------------------------------------------------------------------- and what it means

    /// <summary>
    /// A cell's text never shrinks, whatever kind of text the rectangle held.
    /// </summary>
    /// <remarks>
    /// Every kind is exercised rather than only Body, because the point is that the arm is
    /// unconditional: the item set the cell would have had to carry it is not copied at all.
    /// </remarks>
    [Theory]
    [InlineData(PptTextKind.Body)]
    [InlineData(PptTextKind.HalfBody)]
    [InlineData(PptTextKind.QuarterBody)]
    [InlineData(PptTextKind.Title)]
    [InlineData(PptTextKind.Other)]
    public void ATableCellNeverShrinksToFit(PptTextKind kind)
    {
        PptTextRun run = new(kind, "text", [], []);

        PptSlideLayout.Autofits(new EscherShape(), run, inTable: true).ShouldBeFalse();
    }

    [Fact]
    public void OutsideATableTheKindStillDecides()
    {
        // The guard must not have replaced the rule, only shortcut it: a Body-kind shape that is
        // not in a table shrinks exactly as it did.
        PptTextRun body = new(PptTextKind.Body, "text", [], []);

        PptSlideLayout.Autofits(new EscherShape(), body, inTable: false).ShouldBeTrue();
        PptSlideLayout.Autofits(new EscherShape(), body).ShouldBeTrue();
    }
}
