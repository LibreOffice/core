using System.Buffers.Binary;
using System.Text;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Spreadsheets.MsBinary;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Tests the pivot cache reader against hand-built cache streams.
/// </summary>
/// <remarks>
/// Synthetic, for the reason <see cref="BiffRecordReaderTests"/> gives and one more: a real
/// workbook whose pivot cache generates a sheet needs a pivot table whose source range is in
/// <em>another file</em>, which is not something an authoring tool will make on request. The
/// shapes below are the two routes a field's values take — inline behind an index list, and
/// postponed after it — which is the whole of what decides where a value lands.
/// </remarks>
public class XlsPivotCacheTests
{
    private static byte[] Record(ushort id, params byte[] payload)
    {
        byte[] record = new byte[4 + payload.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, id);
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), (ushort)payload.Length);
        payload.CopyTo(record.AsSpan(4));
        return record;
    }

    private static byte[] Concat(params byte[][] parts) => [.. parts.SelectMany(part => part)];

    private static byte[] Words(params int[] values)
    {
        byte[] bytes = new byte[values.Length * 2];
        for (int at = 0; at < values.Length; at++)
            BinaryPrimitives.WriteUInt16LittleEndian(bytes.AsSpan(at * 2), (ushort)values[at]);
        return bytes;
    }

    /// <summary>A BIFF8 Unicode string: a 16-bit count, a flags byte, and the characters.</summary>
    private static byte[] Text(string value)
        => [.. Words(value.Length), 0x00, .. Encoding.Latin1.GetBytes(value)];

    private static byte[] String(string value) => Record(0x00CD, Text(value));

    /// <summary>An <c>SXFIELD</c> for a standard field, inline or postponed.</summary>
    private static byte[] Field(string name, int visible, int original, bool postponed)
    {
        // 0x0480 is the "only strings" data type; bit 0 says the items are inline and bit 1 says
        // they are postponed, and exactly one of the two is ever set.
        int flags = 0x0480 | (postponed ? 0x0002 : 0x0001);
        return Record(0x00C7, [.. Words(flags, 0, 0, visible, 0, 0, original), .. Text(name)]);
    }

    /// <summary>An <c>SXDB</c> header; only the field counts are read back.</summary>
    private static byte[] Header(int records, int fields)
        => Record(0x00C6, [
            .. BitConverter.GetBytes((uint)records),
            .. Words(1, 0x0025, 0x1FFF, fields, fields, 0, 1),
            .. Text("user"),
        ]);

    private static XlsPivotCacheSheet ReadCache(byte[] stream, out List<Diagnostic> diagnostics)
    {
        diagnostics = [];
        XlsPivotCacheSheet? sheet = XlsPivotCacheReader.Read(
            stream, "DPCache", BiffVersion.Biff8, Encoding.Latin1, diagnostics);
        sheet.ShouldNotBeNull();
        return sheet;
    }

    private static string TextAt(XlsPivotCacheSheet sheet, int row, int column)
        => sheet.Cells.TryGetValue((row, column), out XlsPivotItem item) ? item.Text ?? "" : "";

    [Fact]
    public void AnInlineFieldTakesItsValuesThroughTheIndexList()
    {
        // One field, three distinct values, four records selecting them by index.
        byte[] stream = Concat(
            Header(4, 1),
            Field("Colour", visible: 3, original: 3, postponed: false),
            String("red"), String("green"), String("blue"),
            Record(0x00C8, 0x02),
            Record(0x00C8, 0x00),
            Record(0x00C8, 0x02),
            Record(0x00C8, 0x01),
            Record(0x000A));

        XlsPivotCacheSheet sheet = ReadCache(stream, out _);

        sheet.ColumnCount.ShouldBe(1);
        sheet.RowCount.ShouldBe(5);       // the field name, then four records
        TextAt(sheet, 0, 0).ShouldBe("Colour");
        TextAt(sheet, 1, 0).ShouldBe("blue");
        TextAt(sheet, 2, 0).ShouldBe("red");
        TextAt(sheet, 3, 0).ShouldBe("blue");
        TextAt(sheet, 4, 0).ShouldBe("green");
    }

    [Fact]
    public void PostponedFieldsTakeTheirValuesRoundRobinAfterEachIndexList()
    {
        // The shape the corpus's own cache has: one inline field the index list selects from,
        // and two postponed fields whose values follow the list, one each, in field order.
        byte[] stream = Concat(
            Header(2, 3),
            Field("Part", visible: 2, original: 2, postponed: false),
            String("one"), String("two"),
            Field("Note", visible: 30, original: 0, postponed: true),
            Field("Score", visible: 30, original: 0, postponed: true),
            Record(0x00C8, 0x01),
            String("first note"), String("first score"),
            Record(0x00C8, 0x00),
            String("second note"), String("second score"),
            Record(0x000A));

        XlsPivotCacheSheet sheet = ReadCache(stream, out _);

        sheet.ColumnCount.ShouldBe(3);
        sheet.RowCount.ShouldBe(3);
        TextAt(sheet, 0, 0).ShouldBe("Part");
        TextAt(sheet, 0, 1).ShouldBe("Note");
        TextAt(sheet, 0, 2).ShouldBe("Score");

        TextAt(sheet, 1, 0).ShouldBe("two");
        TextAt(sheet, 1, 1).ShouldBe("first note");
        TextAt(sheet, 1, 2).ShouldBe("first score");

        TextAt(sheet, 2, 0).ShouldBe("one");
        TextAt(sheet, 2, 1).ShouldBe("second note");
        TextAt(sheet, 2, 2).ShouldBe("second score");
    }

    [Fact]
    public void ACacheOfOnlyPostponedFieldsStartsARowPerPass()
    {
        // With no inline field there is no SXINDEXLIST to start a row, so the first postponed
        // field of each pass does it instead.
        byte[] stream = Concat(
            Header(2, 2),
            Field("A", visible: 30, original: 0, postponed: true),
            Field("B", visible: 30, original: 0, postponed: true),
            String("a1"), String("b1"),
            String("a2"), String("b2"),
            Record(0x000A));

        XlsPivotCacheSheet sheet = ReadCache(stream, out _);

        sheet.RowCount.ShouldBe(3);
        TextAt(sheet, 1, 0).ShouldBe("a1");
        TextAt(sheet, 1, 1).ShouldBe("b1");
        TextAt(sheet, 2, 0).ShouldBe("a2");
        TextAt(sheet, 2, 1).ShouldBe("b2");
    }

    [Fact]
    public void EveryItemRecordTypeIsDecoded()
    {
        byte[] stream = Concat(
            Header(6, 1),
            Field("Mixed", visible: 6, original: 6, postponed: false),
            Record(0x00C9, BitConverter.GetBytes(2.5)),                  // SXDOUBLE
            Record(0x00CA, Words(1)),                                    // SXBOOLEAN
            Record(0x00CB, Words(0x2A)),                                 // SXERROR: #N/A
            Record(0x00CC, Words(0xFFF6)),                               // SXINTEGER: -10
            Record(0x00CE, [.. Words(2001, 9), 11, 13, 30, 45]),         // SXDATETIME
            Record(0x00CF),                                              // SXEMPTY
            Record(0x00C8, 0x00),
            Record(0x00C8, 0x01),
            Record(0x00C8, 0x02),
            Record(0x00C8, 0x03),
            Record(0x00C8, 0x04),
            Record(0x00C8, 0x05),
            Record(0x000A));

        XlsPivotCacheSheet sheet = ReadCache(stream, out _);

        sheet.Cells[(1, 0)].Kind.ShouldBe(XlsPivotItemKind.Double);
        sheet.Cells[(1, 0)].Number.ShouldBe(2.5);
        sheet.Cells[(2, 0)].Kind.ShouldBe(XlsPivotItemKind.Boolean);
        sheet.Cells[(3, 0)].Kind.ShouldBe(XlsPivotItemKind.Error);
        sheet.Cells[(3, 0)].ErrorCode.ShouldBe((byte)0x2A);
        sheet.Cells[(4, 0)].Kind.ShouldBe(XlsPivotItemKind.Integer);
        sheet.Cells[(4, 0)].Number.ShouldBe(-10);
        sheet.Cells[(5, 0)].When.ShouldBe((2001, 9, 11, 13, 30, 45));

        // An empty item writes nothing at all, which is what WriteToSource does with one.
        sheet.Cells.ContainsKey((6, 0)).ShouldBeFalse();
        sheet.RowCount.ShouldBe(7);
    }

    [Fact]
    public void A16BitFieldReadsTwoByteIndexes()
    {
        List<byte[]> parts =
        [
            Header(1, 1),

            // 0x0200 is EXC_SXFIELD_16BIT, set alongside the inline-items flag.
            Record(0x00C7, [.. Words(0x0480 | 0x0001 | 0x0200, 0, 0, 300, 0, 0, 300), .. Text("Wide")]),
            .. Enumerable.Range(0, 300).Select(at => String($"v{at}")),
            Record(0x00C8, Words(299)),
            Record(0x000A),
        ];

        byte[] stream = Concat([.. parts]);

        XlsPivotCacheSheet sheet = ReadCache(stream, out _);

        TextAt(sheet, 1, 0).ShouldBe("v299");
    }

    [Theory]
    // A leading 0x02 or 0x03 marks a reference to this workbook, and everything after it is the
    // sheet name.
    [InlineData("Sheet1", true, "Sheet1")]
    [InlineData("Data", true, "Data")]

    // An encoded path: the sheet name starts after the bracketed file name.
    [InlineData("Cdir[book.xls]Sheet2", false, "Sheet2")]

    // A raw path, unencoded, with no leading marker at all.
    [InlineData("[other.xls]Costs", false, "Costs")]
    public void AnEncodedUrlYieldsItsSheetNameAndWhetherItIsThisWorkbook(
        string encoded, bool expectedSelf, string expectedTab)
    {
        XlsPivotCacheSource.DecodeUrl(encoded, out string tab, out bool self);

        self.ShouldBe(expectedSelf);
        tab.ShouldBe(expectedTab);
    }

    [Fact]
    public void ACacheWhoseSourceSheetIsInTheWorkbookGeneratesNothing()
    {
        XlsPivotCacheSource source = new();
        source.ReadSourceType(new BiffRecordReader(Record(0x00E3, Words(1)), []).Started());
        source.ReadSourceReference(
            new BiffRecordReader(
                Record(0x0051, [.. Words(0, 9, 0, 3), .. Text("Data")]), []).Started());

        XlsPivotCacheReader.GeneratedSheetName(source, ["Summary", "Data"]).ShouldBeNull();

        // The same cache with no such sheet in the file has to bring its own.
        XlsPivotCacheReader.GeneratedSheetName(source, ["Summary"]).ShouldBe("DPCache_Data");
    }

    [Fact]
    public void ACacheNamingAnotherFileAlwaysGeneratesASheet()
    {
        XlsPivotCacheSource source = new();
        source.ReadSourceType(new BiffRecordReader(Record(0x00E3, Words(1)), []).Started());
        source.ReadSourceReference(
            new BiffRecordReader(
                Record(0x0051, [.. Words(0, 9, 0, 3), .. Text("[other.xls]Data")]), []).Started());

        XlsPivotCacheReader.GeneratedSheetName(source, ["Summary", "Data"]).ShouldBe("DPCache_Data");
    }

    [Fact]
    public void ACacheWithNoSourceReferenceAtAllGeneratesAnUnsuffixedSheet()
    {
        // The corpus's own case: an SXVS says the source is a sheet and no DCONREF follows, so
        // nothing says the sheet is in this file and the cache is the only copy of the data.
        XlsPivotCacheSource source = new();
        source.ReadSourceType(new BiffRecordReader(Record(0x00E3, Words(1)), []).Started());

        XlsPivotCacheReader.GeneratedSheetName(source, ["Summary"]).ShouldBe("DPCache");
    }

    [Fact]
    public void ASourceTypeThatIsNeitherASheetNorAnExternalFileGeneratesNothing()
    {
        XlsPivotCacheSource source = new();

        // 0x0008 is EXC_SXVS_PIVOTTAB: a cache drawn from another pivot table.
        source.ReadSourceType(new BiffRecordReader(Record(0x00E3, Words(8)), []).Started());

        XlsPivotCacheReader.GeneratedSheetName(source, ["Summary"]).ShouldBeNull();
    }
}

/// <summary>Positions a hand-built one-record stream on its record.</summary>
file static class BiffRecordReaderExtensions
{
    public static BiffRecordReader Started(this BiffRecordReader stream)
    {
        stream.MoveNext();
        return stream;
    }
}
