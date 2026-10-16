using XRay.Office.Core.Charts;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.MsBinary.Escher;
using XRay.Office.MsBinary.Records;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Vector;

namespace XRay.Office.Spreadsheets.MsBinary;

/// <summary>
/// Collects a sheet's or a chart's drawing records and turns them into anchored shapes.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The drawing is spread over three record kinds and only makes sense assembled.</strong>
/// The Escher (MS-ODRAW) byte stream arrives split across every <c>MSODRAWING</c> record in the
/// substream — the split is arbitrary and a container routinely straddles it — so the payloads are
/// concatenated into one buffer and walked once at the end. <c>OBJ</c> records carry what Excel
/// knows about a shape that Escher does not (its type and identifier), and <c>TXO</c> plus its
/// <c>CONTINUE</c> records carry a text box's string. This is
/// <c>XclImpDrawing::ReadMsoDrawing</c> (<c>sc/source/filter/excel/xiescher.cxx:4021</c>)
/// assembling <c>maDffStrm</c>, <c>maRawObjs</c> and <c>maTextMap</c>, and it has to be done that
/// way: reading the <c>MSODRAWING</c> records one at a time yields truncated containers.
/// </para>
/// <para>
/// <strong>A shape is matched to its <c>OBJ</c> by position in the assembled stream.</strong> Each
/// <c>OBJ</c> follows the <c>MSODRAWING</c> records that carry its shape, so noting how many
/// drawing bytes had arrived when it was read gives an offset that falls inside that shape's
/// container and after every earlier one. LibreOffice keys <c>maObjMap</c> on exactly that
/// (<c>xiescher.cxx:4058</c>, <c>FindDrawObj</c>) and finds the shape by upper bound; the same
/// walk is done here from the shape side, which needs no map.
/// </para>
/// <para>
/// <strong>A shape's fill and outline are read, and three object kinds must not take them.</strong>
/// Calc builds its own <c>SdrObject</c> for a chart, a form control and an OLE object — the three
/// that <c>SetCustomDffObj(true)</c> marks (<c>sc/source/filter/excel/xiescher.cxx</c>:1666, 2066,
/// 2954) — and the replacement is not the object the Escher attributes were applied to, so the
/// fill and the line are dropped with it. Measured at the reference on
/// <c>TICAPCapability_Final.xls</c> and <c>014_Contextures_chart_sample</c>: their <c>Chart 1</c>,
/// <c>OptionButton1</c> and <c>Picture 228</c> all state a fill colour and all come back from
/// 26.2.4.2's own flat ODF as <c>draw:fill="none" draw:stroke="none"</c>, while the text boxes
/// beside them come back filled and stroked. See <see cref="EscherInk"/>.
/// </para>
/// <para>
/// <strong>A picture is named by a <c>pib</c> and stored in the workbook, not in the sheet.</strong>
/// The blip store lives once in the globals' <c>MSODRAWINGGROUP</c> and every sheet's shapes index
/// into it one-based, which is why the store arrives from
/// <see cref="XlsWorkbookReader"/> rather than being found in <c>_dff</c>. Reading only the shapes
/// that carry text — as this did — drops every picture on every <c>.xls</c>, and the cost is not
/// only the ink: <c>SheetEmptyPages.TouchedByADrawing</c> keeps a page holding no cells but holding
/// a drawing, so a workbook whose last column band is nothing but pictures loses those pages
/// outright.
/// </para>
/// </remarks>
/// <param name="diagnostics">Where a picture that will not draw is recorded.</param>
/// <param name="blips">
/// The workbook's picture store, keyed by the one-based index a shape's <c>pib</c> holds. Empty for
/// a workbook with no drawing group, which is most of them.
/// </param>
/// <param name="palette">
/// The workbook's colour table, which a shape's <c>MSO_CLR</c> nearly always references rather
/// than stating a literal colour. Null for a caller that has none, which resolves every such
/// reference to the format's own white or black.
/// </param>
internal sealed class XlsDrawingCollector(
    List<Diagnostic> diagnostics,
    IReadOnlyDictionary<int, EscherBlip>? blips = null,
    XlsCellFormats? palette = null)
{
    /// <summary>
    /// How many bytes of Escher stream are accepted before the rest is dropped.
    /// </summary>
    /// <remarks>
    /// A guard against a damaged file whose record chain loops, not a real limit: the largest
    /// drawing in the corpus is under 300 kB, and a sheet reaching this has already produced more
    /// shapes than a page can show.
    /// </remarks>
    private const int MaxDrawingBytes = 8 * 1024 * 1024;

    /// <summary>The offsets a chart's client anchor is stated in, out of the chart area.</summary>
    /// <remarks><c>EXC_CHART_TOTALUNITS</c>, <c>sc/source/filter/inc/xlchart.hxx:163</c>.</remarks>
    private const double ChartTotalUnits = 4000.0;

    private readonly List<byte> _dff = [];
    private readonly List<ObjectEntry> _objects = [];

    /// <summary>True when nothing has been collected, which is nearly every sheet.</summary>
    public bool IsEmpty => _objects.Count == 0;

    /// <summary>Appends one <c>MSODRAWING</c> record's payload to the Escher stream.</summary>
    /// <param name="bytes">The record's bytes, its continuations already joined.</param>
    public void AddDrawing(byte[] bytes)
    {
        ArgumentNullException.ThrowIfNull(bytes);
        if (_dff.Count + bytes.Length > MaxDrawingBytes) return;
        _dff.AddRange(bytes);
    }

    /// <summary>
    /// Reads one <c>OBJ</c> record, keeping the object's type and its place in the stream.
    /// </summary>
    /// <remarks>
    /// Only the <c>ftCmo</c> subrecord is read. The rest describe a control's behaviour — a
    /// list box's source range, a button's macro — none of which reaches the page.
    /// </remarks>
    /// <param name="stream">Positioned at the record's first byte.</param>
    public void ReadObject(BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        ushort type = ushort.MaxValue;
        ushort identifier = 0;
        bool printable = true;
        while (stream.RecordLeft >= 4)
        {
            ushort id = stream.ReadUInt16();
            int size = stream.ReadUInt16();
            if (id == 0 && size == 0) break;

            int left = Math.Min(size, stream.RecordLeft);
            if (id == ObjectCommon && left >= 6)
            {
                type = stream.ReadUInt16();

                // `ftCmo`'s second field is the object's own identifier, which is what a NOTE
                // record names its comment by (`XclImpNote`, xicontent.cxx). Reading it is what
                // lets a note's text be joined to the cell it hangs off.
                identifier = stream.ReadUInt16();

                // The third is the flag word, whose `fPrint` bit decides whether a form control
                // reaches the printed page at all — see `IsFormControl`. Read for every object,
                // as Calc reads it, and acted on for the control types alone.
                printable = (stream.ReadUInt16() & ObjectPrintable) != 0;
                stream.Skip(left - 6);
            }
            else
            {
                stream.Skip(left);
            }
        }

        _objects.Add(new ObjectEntry(type, _dff.Count, identifier, IsPrintable: printable));
    }

    /// <summary>
    /// True when the object just read is an embedded chart, so a chart substream is expected next.
    /// </summary>
    /// <remarks>
    /// A chart embedded in a worksheet is written as its own <c>BOF</c>/<c>EOF</c> substream
    /// immediately after the <c>OBJ</c> that declares it — <c>XclImpChartObj::ReadChartSubStream</c>
    /// reads it from exactly there (<c>sc/source/filter/excel/xiescher.cxx</c>) — so the two are
    /// joined by adjacency and no identifier is needed. Asking for the <em>last</em> object rather
    /// than searching keeps that adjacency explicit: a chart substream not preceded by a chart
    /// object is not this, and is skipped as before.
    /// </remarks>
    public bool ExpectsChartSubstream
        => _objects.Count > 0 && _objects[^1].Type == ChartObject && _objects[^1].Chart is null;

    /// <summary>Attaches a chart substream's plot to the chart object that opened it.</summary>
    /// <param name="plot">The plot, or null when the substream held nothing that builds one.</param>
    public void AttachChart(ChartPlot? plot)
    {
        if (_objects.Count == 0) return;
        _objects[^1] = _objects[^1] with { Chart = plot };
    }

    /// <summary>
    /// Attaches the drawing objects an embedded chart's own substream carries.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A chart substream holds a drawing of its own, and the objects in it belong <em>over the
    /// chart</em> rather than on the sheet: Calc gives them their own
    /// <c>XclImpChartDrawing</c>, whose <c>ConvertObjects</c> inserts them into the chart's model
    /// and whose <c>CalcAnchorRect</c> reads the anchor's cell fields as quarter-thousandths of
    /// the chart's rectangle (<c>sc/source/filter/excel/xichart.cxx</c>:4242-4290
    /// <strong>in this tree</strong>, which declares 27.2.0.0.alpha0+ and is not the reference
    /// binary's source).
    /// </para>
    /// <para>
    /// So they go in a collector of their own rather than into the sheet's. Appending them to the
    /// sheet's Escher stream would put a second <c>DgContainer</c> inside the sheet's own
    /// <c>SpgrContainer</c>, which no walk of the sheet's drawing reaches, and would shift the
    /// sheet's shape-to-<c>OBJ</c> pairing by however many objects the chart carries.
    /// </para>
    /// </remarks>
    /// <param name="overlay">The chart substream's own drawing, or null when it has none.</param>
    public void AttachChartDrawing(XlsDrawingCollector? overlay)
    {
        if (_objects.Count == 0 || overlay is null || overlay.IsEmpty) return;
        _objects[^1] = _objects[^1] with { Overlay = overlay };
    }

    /// <summary>
    /// The text of every cell-comment object read so far, by the identifier a <c>NOTE</c> names.
    /// </summary>
    /// <remarks>
    /// A comment's text is in a <c>TXO</c> like any other object's, and the cell it belongs to is
    /// in the <c>NOTE</c> record, which names the object rather than pointing at it. So the join
    /// is by <c>ftCmo</c>'s identifier, and it has to happen after the whole sheet is read because
    /// a NOTE may precede or follow its OBJ.
    /// </remarks>
    public Dictionary<ushort, string> NoteTexts()
    {
        Dictionary<ushort, string> texts = [];
        foreach (ObjectEntry entry in _objects)
        {
            if (entry.Type != NoteObject) continue;
            if (entry.Text is not { Length: > 0 } text) continue;
            texts[entry.Id] = text;
        }

        return texts;
    }

    /// <summary>
    /// Reads one <c>TXO</c> record and attaches its string and formatting runs to the object
    /// just read.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The string is not in the <c>TXO</c> at all: the record states its length and the
    /// characters arrive in the <c>CONTINUE</c> that follows, with the formatting runs in a
    /// second one (<c>XclImpDrawing::ReadTxo</c>, <c>xiescher.cxx</c>:4242-4269 in this tree).
    /// Each is asked for by name — <see cref="BiffRecordReader.StartContinuation"/> — and only
    /// when the record declared it, because <c>TXO</c> is one of the two records the stream
    /// does <em>not</em> join continuations into. Once a sheet's Escher stream passes the
    /// 8224-byte record ceiling Excel writes the rest of it as bare <c>CONTINUE</c> records
    /// interleaved between the <c>OBJ</c> and <c>TXO</c> records, so a <c>TXO</c> that swallowed
    /// every continuation behind it would eat a shape container and the drawing would lose every
    /// shape from the ceiling on. The flags byte that opens the character data is still read
    /// straight through, because that is exactly what
    /// <see cref="BiffRecordReader.ReadUnicodeString(int)"/> expects at a boundary, and the run
    /// array carries no header of its own.
    /// </para>
    /// <para>
    /// A run is eight bytes — a character index, a <c>FONT</c> index, four reserved
    /// (<c>XclImpString::ReadObjFormats</c>, <c>sc/source/filter/excel/xistring.cxx</c>) — and
    /// <c>AppendFormat</c> keeps the <em>last</em> font stated at a repeated index, under a
    /// comment recording that real files repeat one.
    /// </para>
    /// </remarks>
    /// <param name="stream">Positioned at the record's first byte.</param>
    public void ReadText(BiffRecordReader stream)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (_objects.Count == 0) return;

        ushort flags = stream.ReadUInt16();
        stream.Skip(8);
        int length = stream.ReadUInt16();
        int formatSize = stream.ReadUInt16();
        stream.Skip(4);

        string text = length > 0 && stream.StartContinuation() && stream.RecordLeft > 0
            ? stream.ReadUnicodeString(length)
            : string.Empty;

        // Taken before the empty-text exit, not after it. The continuation has to leave the
        // stream whatever the characters turned out to be: left where it is, the record walk
        // meets it as a bare CONTINUE inside the drawing block and appends the formatting runs
        // to the Escher stream as though they were shape records.
        List<TextRun>? runs = formatSize > 0 && stream.StartContinuation()
            ? ReadRuns(stream, formatSize)
            : null;

        if (text.Length == 0) return;

        _objects[^1] = _objects[^1] with
        {
            Text = text,
            Runs = runs,

            // Bits 1-3 and 4-6 of the flags word: XclObjTextData::GetHorAlign and GetVerAlign
            // (sc/source/filter/inc/xlescher.hxx:401-402).
            Horizontal = (flags >> 1) & 0x07,
            Vertical = (flags >> 4) & 0x07,
        };
    }

    /// <summary>The formatting-run array that follows a <c>TXO</c>'s characters.</summary>
    /// <param name="stream">Positioned just past the last character.</param>
    /// <param name="formatSize">The byte count the record declared, eight per run.</param>
    private static List<TextRun>? ReadRuns(BiffRecordReader stream, int formatSize)
    {
        int count = formatSize / 8;
        if (count <= 0) return null;

        List<TextRun> runs = [];
        for (int at = 0; at < count && stream.RecordLeft >= 8; at++)
        {
            int character = stream.ReadUInt16();
            int font = stream.ReadUInt16();
            stream.Skip(4);

            // `XclImpString::AppendFormat`: a repeated character index replaces the font rather
            // than adding a second run, and an index that goes backwards is dropped.
            if (runs.Count > 0 && runs[^1].Character >= character)
            {
                if (runs[^1].Character == character) runs[^1] = new TextRun(character, font);
                continue;
            }

            runs.Add(new TextRun(character, font));
        }

        // The terminator is kept rather than trimmed. `Paragraphs` changes the current style at
        // every run index it passes, so an entry naming the character after the string simply
        // never fires — and dropping it would mean deciding, here, which entry is one.
        return runs.Count > 0 ? runs : null;
    }

    /// <summary>
    /// The shapes, anchored against a sheet's grid.
    /// </summary>
    /// <param name="grid">The sheet's columns and rows, which the anchor's offsets are fractions of.</param>
    public List<SheetDrawing> BuildForSheet(SheetGrid grid)
    {
        ArgumentNullException.ThrowIfNull(grid);
        return Build(anchor => SheetAnchor(anchor, grid), (box, part) => WithinCells(box, grid, part));
    }

    /// <summary>
    /// The shapes of a chart substream, anchored against the chart's own rectangle.
    /// </summary>
    /// <remarks>
    /// A chart's drawing objects are positioned in quarter-thousandths of the chart area, and
    /// the fractions are stored in the <em>cell</em> fields of the client anchor rather than in
    /// its offsets — <c>XclImpChartDrawing::CalcAnchorRect</c>
    /// (<c>sc/source/filter/excel/xichart.cxx:4274</c>). So the same eighteen bytes mean
    /// something else here, and a shape on a chart sheet lands at an absolute place rather than
    /// against a column.
    /// </remarks>
    /// <param name="origin">Where the chart sits on the sheet.</param>
    /// <param name="size">How big the chart is.</param>
    public List<SheetDrawing> BuildForChart(DocPoint origin, DocSize size)
        => Build(anchor => ChartAnchor(anchor, origin, size), WithinBox);

    private List<SheetDrawing> Build(
        Func<Anchor, SheetDrawing?> place,
        Func<SheetDrawing, Fraction, SheetDrawing?> within)
    {
        List<SheetDrawing> drawings = [];
        if (_dff.Count == 0 || _objects.Count == 0) return drawings;

        DffRecordBuffer buffer = new([.. _dff]);
        EscherDrawingReader reader = new(buffer, diagnostics);

        List<Placement> shapes = [];
        foreach (DffRecordHeader record in buffer.Range(0, buffer.Length))
        {
            if (record.Type == EscherRecordTypes.DrawingContainer)
                Flatten(reader.ReadDrawing(record), parent: -1, shapes);
        }

        // Where each shape was placed, so that a group's children can be laid inside it. Kept for
        // every shape and not only for the ones that are drawn, because a group's own shape is
        // nearly always empty — it carries the rectangle and nothing else — and is dropped by the
        // content test below while its children still need the box it establishes.
        SheetDrawing?[] boxes = new SheetDrawing?[shapes.Count];

        // The n-th shape carrying client data is the n-th OBJ: both sequences are the drawing's
        // own order, and a shape without client data — the patriarch, a solver entry — has no OBJ
        // record of its own to consume one.
        int at = 0;
        for (int index = 0; index < shapes.Count; index++)
        {
            EscherShape shape = shapes[index].Shape;
            boxes[index] = Placed(buffer, shapes, boxes, index, place, within);

            if (shape.ClientData is null) continue;
            if (at >= _objects.Count) break;

            ObjectEntry entry = _objects[at++];

            // A picture and a text box are both shapes with a client anchor, and a shape can
            // carry neither — a solver entry, a group's own frame, a rectangle drawn for its
            // outline. Asking for the three things this can draw before doing any placement work
            // is what keeps those out without a type test that would have to name every one.
            //
            // The third is a chart, and it is the one exception that has to be named by type: an
            // embedded chart's shape holds no `pib` and no `TXO`, so the two tests above drop it —
            // and with it the whole sheet, because `SheetDrawingArea` then cannot widen
            // `PrintedRange` and a sheet whose only content is a chart has no printed range at all.
            // Calc has the object on its draw page, so `ScDocument::GetPrintArea` takes the maximum
            // of the cells' extent and the drawing layer's (`documen2.cxx:649-658`) and finds one.
            // The fourth is a shape carrying nothing but a fill or an outline, which was read as
            // nothing in all three of this project's spreadsheet formats until round 84.
            //
            // **A shape carrying none of the four is still dropped, and that is measured rather
            // than chosen.** `ScDrawLayer::GetPrintArea` widens the printed block to cover every
            // object on the draw page (`sc/source/core/data/drwlayer.cxx`:1397-1414), so the
            // reference does keep such a shape — 26.2.4.2 prints its own `.xls` of
            // `features/sheet-shape-ink.xlsx` on **two** pages, the second empty, because the
            // rightmost object there is a rectangle with no fill and no line, and this reader
            // prints one. Keeping every shape reproduces that fixture and **costs two gate
            // verdicts on the corpus** — `activespecs.xls` 267 pages against 266 and
            // `orbus_togaf_tool_csq.xls` 74 against 75.
            //
            // The reason is that BIFF has two guards before an object reaches the draw page and
            // this reader has neither. `IsProcessSdrObj()` is `mbProcessSdr && !mbHidden`
            // (`sc/source/filter/inc/xiescher.hxx`:118), so a *hidden* BIFF object is not
            // processed at all — the opposite of the DrawingML rule, where a shape whose
            // `cNvPr` says `hidden="1"` still moves the page break. And
            // `XclImpDrawObjBase::IsValidSize` (`xiescher.cxx`:414-420) rejects an anchor under
            // 3/100 mm by 1/100 mm, dropped by `ProcessObj` at `:3658-3665` under a comment
            // naming the class: *"invisible phantom objects from deleted rows or columns"*. A
            // guard on the anchor's own cell span was tried and reaches neither document, so the
            // phantoms here are not zero-span; finding what they are is the seat this leaves.
            //
            // **The ODF path is not the same question and is not gated this way**, because
            // Calc's ODF import has no such guard: `ScXMLTableRowCellContext` hands every
            // `draw:` child to `XMLShapeImportHelper` and every one of them is inserted.
            SheetPicture picture = PictureOf(shape);
            EscherInk.Ink ink = KeepsEscherInk(shape, entry)
                ? EscherInk.Read(shape.Properties, shape.ShapeType, palette is null ? null : palette.SchemeColour)
                : default;

            if (picture.IsEmpty && entry.Text is not { Length: > 0 } && entry.Type != ChartObject
                && !ink.HasInk)
            {
                continue;
            }

            // A cell comment is not a shape on the page. Its `ftCmo` type is 25
            // (`EXC_OBJTYPE_NOTE`, `sc/source/filter/inc/xlescher.hxx:69`) and Calc's importer
            // takes the object apart rather than inserting it: `XclImpNoteObj` calls
            // `SetInsertSdrObj(false)` in its constructor — "caption object will be created
            // manually" — and turns the text into a `ScPostIt` on the cell instead
            // (`sc/source/filter/excel/xiescher.cxx:1852-1883`). The caption exists only when the
            // NOTE record marks the comment visible, which is the case this drops with it.
            if (entry.Type == NoteObject) continue;

            if (boxes[index] is not { } placed) continue;

            drawings.Add(placed with
            {
                Text = entry.Text is { Length: > 0 } ? TextOf(entry, palette, shape.Properties) : null,
                Image = picture.Raster,
                Vector = picture.Vector,

                // Read here and applied by `SheetPageGraphics`, because the crop needs the box and
                // a cell anchor is not one yet. Read for every shape rather than only for one
                // carrying a picture: it costs four property lookups and it means a shape that
                // gained a picture by some other route cannot silently lose its crop.
                Crop = EscherPicture.Crop(shape.Properties),
                Name = NameOf(shape),

                // Escher states a shape *type* rather than a DrawingML preset name, so the ink is
                // painted through the anchor's box; see `SheetShapeInk`, which falls back to it.
                Fill = ink.Fill,
                Texture = ink.BitmapFill ? TextureOf(shape, palette) : null,
                Stroke = ink.Stroke,
                StrokeWidth = ink.StrokeWidth,

                // A form control the file marks unprintable is on the screen and not on the
                // paper. It stays in the model rather than being dropped, because its anchor
                // still widens the printed block — see `SheetDrawing.IsPrintable`.
                IsPrintable = entry.IsPrintable || !IsFormControl(entry.Type),

                IsChart = entry.Type == ChartObject,
                Chart = entry.Chart,
            });

            // The chart's own drawing, laid over the rectangle the chart was just placed in.
            // Its anchors are quarter-thousandths of that rectangle rather than cells, which is
            // `XclImpChartDrawing::CalcAnchorRect`; everything else — the group mapping, the
            // content test, the shape-to-`OBJ` pairing — is this same walk over its own records.
            if (entry.Overlay is { } overlay)
            {
                drawings.AddRange(overlay.Build(
                    anchor => within(placed, ChartFraction(anchor)), within));
            }
        }

        return drawings;
    }

    /// <summary>
    /// A chart-substream anchor read as a fraction of the chart's own rectangle.
    /// </summary>
    /// <remarks>
    /// The eighteen bytes of a client anchor mean something else inside a chart substream: the
    /// four <em>cell</em> fields are the corners in quarter-thousandths of the chart area and the
    /// offsets are unused — <c>XclImpChartDrawing::CalcAnchorRect</c>,
    /// <c>sc/source/filter/excel/xichart.cxx:4274</c>, and <c>EXC_CHART_TOTALUNITS</c>.
    /// </remarks>
    private static Fraction ChartFraction(Anchor anchor)
    {
        double left = Math.Min(anchor.FirstColumn, anchor.LastColumn) / ChartTotalUnits;
        double right = Math.Max(anchor.FirstColumn, anchor.LastColumn) / ChartTotalUnits;
        double top = Math.Min(anchor.FirstRow, anchor.LastRow) / ChartTotalUnits;
        double bottom = Math.Max(anchor.FirstRow, anchor.LastRow) / ChartTotalUnits;
        return new Fraction(left, top, right, bottom);
    }

    /// <summary>
    /// The picture a shape's <c>pib</c> names, or nothing when it names none this can draw.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>pib</c> is one-based and zero means "no picture", so the lookup and the emptiness test are
    /// the same question — the rule <c>SvxMSDffManager</c> applies everywhere and the one Word's
    /// reader states as well.
    /// </para>
    /// <para>
    /// The bytes are sniffed rather than believed. An Escher blip record's type is the honest label
    /// of what it holds, but <see cref="VectorImages"/> is the only thing that knows which of the
    /// metafile dialects there is a decoder for — an EMF+ has no signature of its own — so the same
    /// two-step the package path uses (<c>XlsxDrawings.Load</c>) is used here: ask the decoder
    /// registry first, and fall back to a raster media type sniffed from the leading bytes.
    /// </para>
    /// <para>
    /// Nothing is decoded here. <see cref="RasterImage.Encoded"/> keeps the bytes and the metafile
    /// is deferred behind a <see cref="Lazy{T}"/>, so a caller that only wanted cell values never
    /// pays for a codec or for the font stack a metafile's text would start.
    /// </para>
    /// </remarks>
    private SheetPicture PictureOf(EscherShape shape)
    {
        if (blips is not { Count: > 0 }) return default;

        uint pib = shape.Properties.Value(EscherPropertyIds.Picture);
        if (pib == 0 || !blips.TryGetValue((int)pib, out EscherBlip blip)) return default;

        ReadOnlyMemory<byte> bytes = blip.Bytes;

        if (bytes.IsEmpty)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Information, "PL2370",
                $"A {blip.Kind} picture was found on a sheet and has not been drawn: its bytes "
                + "could not be read out of the blip store, so the sheet keeps its room and shows "
                + "nothing there."));

            return default;
        }

        if (VectorImages.For(bytes.Span) is not null)
        {
            return new SheetPicture(null, new Lazy<VectorImage>(() => VectorImages.Decode(bytes)));
        }

        if (RasterMediaType(bytes.Span) is not { } mediaType)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2371",
                $"A sheet's picture is in no format this library recognises; the blip store "
                + $"declared it as {blip.Kind}."));

            return default;
        }

        return new SheetPicture(RasterImage.Encoded(bytes, mediaType), null);
    }

    /// <summary>
    /// The tile a shape's pattern, texture or picture fill is painted with.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The colours are resolved here rather than in <see cref="EscherPatternFill"/> because the
    /// palette is the host's: <c>fillColor</c> and <c>fillBackColor</c> are ordinary
    /// <c>MSO_CLR</c> values and go through <see cref="EscherColour"/> with the workbook's own
    /// scheme, exactly as the fill and line colours beside them do. Both default to <b>white</b>,
    /// which is <c>ApplyFillAttributes</c>' <c>Color aCol1( COL_WHITE ), aCol2( COL_WHITE )</c>
    /// (<c>filter/source/msfilter/msdffimp.cxx</c>:1408-1414 in this tree) rather than the
    /// property table's own defaults.
    /// </para>
    /// <para>
    /// A texture states its tile in EMUs through <c>fillWidth</c> and <c>fillHeight</c>
    /// (<c>:1444-1451</c>) and a pattern states none, so a pattern's tile is the bitmap's own
    /// size at 96 dpi. A picture is stretched once, which <see cref="SheetShapeTexture"/> spells
    /// as an empty tile.
    /// </para>
    /// <para>
    /// A shape whose blip cannot be resolved gets no fill at all, which is what the reference
    /// draws: <c>ApplyFillAttributes</c> puts <c>FillStyle_BITMAP</c> on the item set and then
    /// only adds an <c>XFillBitmapItem</c> inside <c>if (IsProperty(DFF_Prop_fillBlip))</c>.
    /// </para>
    /// </remarks>
    private SheetShapeTexture? TextureOf(EscherShape shape, XlsCellFormats? palette)
    {
        if (blips is not { Count: > 0 }) return null;

        uint index = shape.Properties.Value(EscherPropertyIds.FillBlip);
        if (index == 0 || !blips.TryGetValue((int)index, out EscherBlip blip)) return null;

        Func<int, Colour?>? scheme = palette is null ? null : palette.SchemeColour;

        Colour foreground = EscherColour.Resolve(
            shape.Properties.Value(EscherPropertyIds.FillColour, White), Colour.White, scheme);

        Colour background = EscherColour.Resolve(
            shape.Properties.Value(EscherPropertyIds.FillBackColour, White), Colour.White, scheme);

        if (EscherPatternFill.Tile(blip.Bytes.Span, foreground, background) is not { } tile)
        {
            return null;
        }

        uint kind = shape.Properties.Value(EscherPropertyIds.FillType);

        DocSize extent = kind switch
        {
            PictureFill => default,
            TextureFill when Stated(shape) is { } stated => stated,
            _ => tile.IsDecoded ? EscherPatternFill.Extent(tile) : default,
        };

        return new SheetShapeTexture(tile, extent, EscherPatternFill.Opacity(shape.Properties));

        static DocSize? Stated(EscherShape shape)
        {
            Length width = Length.FromEmu(shape.Properties.Value(EscherPropertyIds.FillWidth));
            Length height = Length.FromEmu(shape.Properties.Value(EscherPropertyIds.FillHeight));

            return width > Length.Zero && height > Length.Zero ? new DocSize(width, height) : null;
        }
    }

    /// <summary>An <c>MSO_CLR</c> literal white, which both fill colours default to.</summary>
    private const uint White = 0x00FFFFFFu;

    /// <summary><c>mso_fillTexture</c>.</summary>
    private const uint TextureFill = 2;

    /// <summary><c>mso_fillPicture</c>.</summary>
    private const uint PictureFill = 3;

    /// <summary>
    /// The media type of a raster a backend can decode, or null for anything else.
    /// </summary>
    /// <remarks>
    /// Sniffed from the bytes rather than taken from the blip record's type, for the reason the
    /// format catalogue sniffs whole documents: a producer writing a JPEG into an
    /// <c>msofbtBlipPNG</c> is common enough that LibreOffice's own <c>GraphicDescriptor</c> does
    /// the same. Only what Skia carries — claiming a media type for a TIFF would put an image
    /// object in the PDF that no reader can draw, which is worse than the empty room it gets.
    /// </remarks>
    private static string? RasterMediaType(ReadOnlySpan<byte> bytes)
    {
        // Spelled as bytes rather than as a u8 literal: PNG's first byte is 0x89, and a u8 literal
        // would encode that as the two bytes UTF-8 uses for U+0089 and never match anything.
        ReadOnlySpan<byte> png = [0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A];
        if (bytes.Length >= 8 && bytes[..8].SequenceEqual(png)) return "image/png";

        if (bytes.Length >= 3 && bytes[0] == 0xFF && bytes[1] == 0xD8 && bytes[2] == 0xFF)
        {
            return "image/jpeg";
        }

        if (bytes.Length >= 6
            && (bytes[..6].SequenceEqual("GIF87a"u8) || bytes[..6].SequenceEqual("GIF89a"u8)))
        {
            return "image/gif";
        }

        if (bytes.Length >= 12
            && bytes[..4].SequenceEqual("RIFF"u8) && bytes[8..12].SequenceEqual("WEBP"u8))
        {
            return "image/webp";
        }

        // Last because it is the weakest — two ASCII letters — and would claim the first two bytes
        // of something else if it were checked first.
        return bytes.Length >= 2 && bytes[0] == 'B' && bytes[1] == 'M' ? "image/bmp" : null;
    }

    /// <summary>One of the two shapes a picture's bytes can take, or neither.</summary>
    private readonly record struct SheetPicture(RasterImage? Raster, Lazy<VectorImage>? Vector)
    {
        public bool IsEmpty => Raster is null && Vector is null;
    }

    /// <summary>Walks a group before its children, which is the order the objects arrive in.</summary>
    private static void Flatten(IReadOnlyList<EscherShape> shapes, int parent, List<Placement> into)
    {
        foreach (EscherShape shape in shapes)
        {
            int own = into.Count;
            into.Add(new Placement(shape, parent));
            if (shape.Children.Count > 0) Flatten(shape.Children, own, into);
        }
    }

    /// <summary>A flattened shape and the index of the group it came out of, or -1.</summary>
    private readonly record struct Placement(EscherShape Shape, int Parent);

    /// <summary>A sub-rectangle of a box, as four fractions of its width and height.</summary>
    private readonly record struct Fraction(double Left, double Top, double Right, double Bottom);

    private static string? NameOf(EscherShape shape)
    {
        string? name = shape.Properties.Text(EscherPropertyIds.ShapeName);
        return name is { Length: > 0 } ? name : null;
    }

    /// <summary>
    /// One text box's paragraphs, with the faces its <c>TXO</c> formatting runs name.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A run's character index is into the whole string, newlines included — which is how
    /// <c>lclCreateTextObject</c> walks it (<c>sc/source/filter/excel/xihelper.cxx</c>): one
    /// counter over every character, advancing the paragraph on a <c>\n</c> and the offset
    /// otherwise. So the paragraphs are cut and the runs distributed in one pass rather than the
    /// string being split first, because splitting loses the separators the offsets count.
    /// </para>
    /// <para>
    /// <strong>Text before the first run takes no font at all.</strong> That loop starts with an
    /// empty item set and sends it as soon as it reaches the first run's index, so whatever
    /// precedes it keeps the edit engine's own default rather than the object's or the first
    /// run's — which is what <see cref="DefaultTextSize"/> stands for here. In practice every
    /// corpus <c>TXO</c> opens its run array at character zero.
    /// </para>
    /// <para>
    /// <strong>A <c>TXO</c> with no run array is not rich and is not styled either.</strong>
    /// <c>XclImpTextObj::DoPreProcessSdrObj</c> (<c>xiescher.cxx</c>:1504-1530) branches on
    /// <c>XclImpString::IsRich()</c>, which is <c>!maFormats.empty()</c>
    /// (<c>sc/source/filter/inc/xistring.hxx</c>:56), and takes <c>NbcSetText</c> for the plain
    /// case — no font is applied there at all.
    /// </para>
    /// </remarks>
    /// <param name="entry">The object, its <c>TXO</c> already read.</param>
    /// <param name="fonts">The workbook's <c>FONT</c> table, or null when it has none.</param>
    /// <param name="properties">The shape's Escher properties, which state its text margins.</param>
    private static SheetShapeText TextOf(
        ObjectEntry entry, XlsCellFormats? fonts, EscherPropertyTable properties)
    {
        SheetShapeAlignment alignment = entry.Horizontal switch
        {
            HorizontalCentre => SheetShapeAlignment.Centre,
            HorizontalRight => SheetShapeAlignment.Right,
            _ => SheetShapeAlignment.Left,
        };

        SheetShapeText body = new()
        {
            Paragraphs = [.. Paragraphs(entry, fonts, alignment)],
            Anchor = entry.Vertical switch
            {
                VerticalCentre => SheetShapeAnchor.Middle,
                VerticalBottom => SheetShapeAnchor.Bottom,
                _ => SheetShapeAnchor.Top,
            },
        };

        // **The margins a BIFF text box leaves round its text are two different rules.**
        //
        // A shape that sets `fAutoTextMargin` states no lengths at all and the *host* answers
        // with a constant: Excel's is 20000 EMU on each of the four sides
        // (`EXC_OBJ_TEXT_MARGIN`, `sc/source/filter/inc/xlescher.hxx:140`), put on by
        // `XclImpDrawObjBase::PreProcessSdrObject` at
        // `sc/source/filter/excel/xiescher.cxx:546-553` **in this tree**, which declares
        // 27.2.0.0.alpha0+ and is not the reference binary's source. Confirmed against the
        // binary instead: on `EHEST-Pre-departure-checklist`'s `ACCEPTABLE` box 26.2.4.2 starts
        // the glyphs 1.57 pt right of and below the box's own corner, and 20000 EMU is 1.5748 pt.
        //
        // A shape that does not set the bit states `dxTextLeft` and its three siblings itself,
        // in EMUs, and those are taken as they come. 26.2.4.2's own *MS Excel 97* filter writes
        // all four as **zero** on a plain text box, which is why the fixture's control sits
        // flush against its left edge.
        //
        // What a shape stating neither should get is *not* settled here: the C++ carries two
        // different defaults for it — 91440/45720 EMU at `filter/source/msfilter/msdffimp.cxx`
        // :5280-5283 and 0.25 cm/0.13 cm at `:1474-1477` — and 489 of 489 shape containers in
        // the 64 `.xls` of this corpus state all four, so nothing here measures the case. Such a
        // shape keeps the tenth of a millimetre this had before.
        bool automatic = properties.Boolean(EscherPropertyIds.AutoTextMargin);

        return body with
        {
            LeftInset = Inset(properties, EscherPropertyIds.TextInsetLeft, automatic, TextInset),
            RightInset = Inset(properties, EscherPropertyIds.TextInsetRight, automatic, TextInset),
            TopInset = Inset(properties, EscherPropertyIds.TextInsetTop, automatic, Length.Zero),
            BottomInset = Inset(properties, EscherPropertyIds.TextInsetBottom, automatic, Length.Zero),
        };
    }

    /// <summary>One side's text margin: the host's constant, the file's own, or the fallback.</summary>
    private static Length Inset(
        EscherPropertyTable properties, ushort id, bool automatic, Length fallback)
    {
        if (automatic) return AutoTextMargin;
        return properties.Has(id) ? Length.FromEmu(properties.SignedValue(id)) : fallback;
    }

    /// <summary>Cuts one text box's string into paragraphs and its runs into spans.</summary>
    private static List<SheetShapeParagraph> Paragraphs(
        ObjectEntry entry, XlsCellFormats? fonts, SheetShapeAlignment alignment)
    {
        string text = entry.Text ?? string.Empty;
        IReadOnlyList<TextRun> runs = entry.Runs ?? [];

        List<SheetShapeParagraph> paragraphs = [];
        List<SheetShapeRun> spans = [];
        System.Text.StringBuilder span = new();

        int next = 0;
        SheetShapeRun style = Style(null, fonts);

        void CloseSpan()
        {
            if (span.Length == 0) return;
            spans.Add(style with { Text = span.ToString() });
            span.Clear();
        }

        void CloseParagraph()
        {
            CloseSpan();
            paragraphs.Add(new SheetShapeParagraph
            {
                Runs = spans.Count > 0 ? [.. spans] : [style with { Text = string.Empty }],
                Alignment = alignment,
            });
            spans = [];
        }

        for (int at = 0; at < text.Length; at++)
        {
            while (next < runs.Count && runs[next].Character <= at)
            {
                SheetShapeRun changed = Style(runs[next].FontIndex, fonts);
                next++;
                if (changed == style) continue;
                CloseSpan();
                style = changed;
            }

            char c = text[at];
            if (c is '\n' or '\r')
            {
                CloseParagraph();
                if (c == '\r' && at + 1 < text.Length && text[at + 1] == '\n') at++;
                continue;
            }

            span.Append(c);
        }

        CloseParagraph();
        return paragraphs;
    }

    /// <summary>
    /// What one <c>FONT</c> index is worth to a shape run, or the edit engine's default for none.
    /// </summary>
    /// <remarks>
    /// A <c>FONT</c> whose height is zero is treated as stating none: the record's own field is
    /// unsigned twips and a zero there would draw nothing at all, while the caller's fallback is
    /// what the object would have taken anyway.
    /// </remarks>
    private static SheetShapeRun Style(int? index, XlsCellFormats? fonts)
    {
        if (index is not { } at || fonts?.FontAt(at) is not { } font)
            return new SheetShapeRun(string.Empty, DefaultTextSize);

        return new SheetShapeRun(
            string.Empty,
            font.Height > Length.Zero ? font.Height : DefaultTextSize,
            font.Name is { Length: > 0 } name ? name : null,
            font.Weight >= BoldWeight,
            fonts.StatedColour(font));
    }

    /// <summary>The weight at which a BIFF <c>FONT</c> counts as bold.</summary>
    /// <remarks>
    /// <c>EXC_FONTWGHT_BOLD</c> is 700 and <c>EXC_FONTWGHT_NORMAL</c> 400
    /// (<c>sc/source/filter/inc/xlstyle.hxx</c>); the record's field is the OS/2 scale, so the
    /// test is a threshold rather than an equality.
    /// </remarks>
    private const int BoldWeight = 700;

    /// <summary>
    /// Where one shape goes: its own client anchor, or its place inside the group that holds it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A shape inside a group states no client anchor at all.</strong> It carries an
    /// <c>msofbtChildAnchor</c> instead — a rectangle in the group's own coordinate space, which
    /// has nothing to do with the sheet — and only the group's own shape carries the client
    /// anchor that says where that space lands. Asking every shape for a client anchor therefore
    /// drops every grouped shape, and a group is how Excel stores a row of small labelled boxes.
    /// </para>
    /// <para>
    /// The map is the linear one <c>SvxMSDffManager::ImportShape</c> applies
    /// (<c>filter/source/msfilter/msdffimp.cxx</c>:4318-4340 <strong>in this tree</strong>, which
    /// declares 27.2.0.0.alpha0+ and is not the reference binary's source): the child's rectangle
    /// is scaled out of the group's child space and into the group's placed rectangle. The child
    /// space is the union of the direct children's own child anchors, which is what
    /// <c>GetGlobalChildAnchor</c> computes at <c>:5029-5045</c> — not the group's
    /// <c>msofbtSpgr</c>, which a writer is free to leave stale.
    /// </para>
    /// <para>
    /// The C++ adds one 1/100 mm to the mapped width and height so that a zero-thickness child
    /// still has a rectangle. That is not reproduced: a hundredth of a millimetre is a thirtieth
    /// of a point and this places in EMU.
    /// </para>
    /// </remarks>
    private static SheetDrawing? Placed(
        DffRecordBuffer buffer,
        List<Placement> shapes,
        SheetDrawing?[] boxes,
        int index,
        Func<Anchor, SheetDrawing?> place,
        Func<SheetDrawing, Fraction, SheetDrawing?> within)
    {
        EscherShape shape = shapes[index].Shape;
        if (ClientAnchor(buffer, shape) is { } anchor) return place(anchor);

        int parent = shapes[index].Parent;
        if (parent < 0 || boxes[parent] is not { } box) return null;
        if (shape.ChildAnchor is not { } child) return null;
        if (ChildSpace(shapes, parent) is not { } space) return null;

        double width = (double)space.Right - space.Left;
        double height = (double)space.Bottom - space.Top;
        if (width <= 0.0 || height <= 0.0) return null;

        return within(box, new Fraction(
            (child.Left - space.Left) / width,
            (child.Top - space.Top) / height,
            (child.Right - space.Left) / width,
            (child.Bottom - space.Top) / height));
    }

    /// <summary>The coordinate space a group's children are stated in.</summary>
    /// <remarks>
    /// The union of the direct children's child anchors — <c>GetGlobalChildAnchor</c>. Null when
    /// no child states one, which is a group whose members are anchored some other way.
    /// </remarks>
    private static EscherRectangle? ChildSpace(List<Placement> shapes, int group)
    {
        EscherRectangle? space = null;
        for (int at = group + 1; at < shapes.Count; at++)
        {
            if (shapes[at].Parent != group) continue;
            if (shapes[at].Shape.ChildAnchor is not { } child) continue;

            space = space is { } union
                ? new EscherRectangle(
                    Math.Min(union.Left, child.Left), Math.Min(union.Top, child.Top),
                    Math.Max(union.Right, child.Right), Math.Max(union.Bottom, child.Bottom))
                : child;
        }

        return space;
    }

    /// <summary>A fraction of a two-cell anchor, as a two-cell anchor of its own.</summary>
    /// <remarks>
    /// The parent's corners are turned into distances from the sheet's own origin, the fractions
    /// are taken there — where a column's width is a length rather than an index — and the result
    /// is turned back into cells. Interpolating in column indices instead would put a child in the
    /// wrong place on any sheet whose columns are not all the same width, which is every sheet.
    /// The measure is <see cref="SheetAxis.PrintedSizeAt"/>, because that is what
    /// <c>SheetPageGraphics</c> resolves the anchor with when the drawing is finally placed.
    /// </remarks>
    private static SheetDrawing? WithinCells(SheetDrawing box, SheetGrid grid, Fraction part)
    {
        if (box.Anchor != SheetAnchorKind.TwoCell) return null;

        Length left = Along(grid.Columns, box.From.Column, box.From.ColumnOffset);
        Length right = Along(grid.Columns, box.To.Column, box.To.ColumnOffset);
        Length top = Along(grid.Rows, box.From.Row, box.From.RowOffset);
        Length bottom = Along(grid.Rows, box.To.Row, box.To.RowOffset);

        Length width = right - left;
        Length height = bottom - top;

        return new SheetDrawing
        {
            Anchor = SheetAnchorKind.TwoCell,
            From = CellAt(grid, left + (width * part.Left), top + (height * part.Top)),
            To = CellAt(grid, left + (width * part.Right), top + (height * part.Bottom)),
        };
    }

    /// <summary>A fraction of an absolutely placed box, as a box of its own.</summary>
    private static SheetDrawing? WithinBox(SheetDrawing box, Fraction part)
    {
        if (box.Anchor != SheetAnchorKind.Absolute) return null;

        Length left = box.Position.X + (box.Extent.Width * part.Left);
        Length right = box.Position.X + (box.Extent.Width * part.Right);
        Length top = box.Position.Y + (box.Extent.Height * part.Top);
        Length bottom = box.Position.Y + (box.Extent.Height * part.Bottom);

        if (right < left) (left, right) = (right, left);
        if (bottom < top) (top, bottom) = (bottom, top);

        return new SheetDrawing
        {
            Anchor = SheetAnchorKind.Absolute,
            Position = new DocPoint(left, top),
            Extent = new DocSize(right - left, bottom - top),
        };
    }

    /// <summary>How far a cell-and-offset point sits from the sheet's own origin.</summary>
    private static Length Along(SheetAxis axis, int index, Length offset)
        => (index > 0 ? axis.TotalPrintedSize(0, index - 1) : Length.Zero) + offset;

    /// <summary>The cell a distance from the sheet's origin falls in, and how far into it.</summary>
    private static SheetCellPoint CellAt(SheetGrid grid, Length x, Length y)
    {
        (int column, Length columnOffset) = Cell(grid.Columns, x, MaxColumns);
        (int row, Length rowOffset) = Cell(grid.Rows, y, MaxRows);
        return new SheetCellPoint(column, columnOffset, row, rowOffset);
    }

    private static (int Index, Length Offset) Cell(SheetAxis axis, Length position, int limit)
    {
        Length left = position > Length.Zero ? position : Length.Zero;

        int index = 0;
        while (index < limit)
        {
            Length size = axis.PrintedSizeAt(index);

            // A hidden column has no width and cannot hold the point, but it still has to be
            // stepped over — testing "does it fit" alone would never advance past one.
            if (size > Length.Zero && left < size) break;
            left -= size;
            index++;
        }

        return (index, left);
    }

    /// <summary>The last column and row a sheet can have, so a runaway walk stops.</summary>
    private const int MaxColumns = 16384;
    private const int MaxRows = 1048576;

    private static SheetDrawing SheetAnchor(Anchor anchor, SheetGrid grid)
        => new()
        {
            Anchor = SheetAnchorKind.TwoCell,
            From = new SheetCellPoint(
                anchor.FirstColumn, Across(grid.Columns, anchor.FirstColumn, anchor.LeftOffset, ColumnUnits),
                anchor.FirstRow, Across(grid.Rows, anchor.FirstRow, anchor.TopOffset, RowUnits)),
            To = new SheetCellPoint(
                anchor.LastColumn, Across(grid.Columns, anchor.LastColumn, anchor.RightOffset, ColumnUnits),
                anchor.LastRow, Across(grid.Rows, anchor.LastRow, anchor.BottomOffset, RowUnits)),
        };

    private static SheetDrawing ChartAnchor(Anchor anchor, DocPoint origin, DocSize size)
    {
        Length left = origin.X + (size.Width * (anchor.FirstColumn / ChartTotalUnits));
        Length top = origin.Y + (size.Height * (anchor.FirstRow / ChartTotalUnits));
        Length right = origin.X + (size.Width * (anchor.LastColumn / ChartTotalUnits));
        Length bottom = origin.Y + (size.Height * (anchor.LastRow / ChartTotalUnits));

        if (right < left) (left, right) = (right, left);
        if (bottom < top) (top, bottom) = (bottom, top);

        return new SheetDrawing
        {
            Anchor = SheetAnchorKind.Absolute,
            Position = new DocPoint(left, top),
            Extent = new DocSize(right - left, bottom - top),
        };
    }

    /// <summary>How far into a column or row an anchor's fractional offset reaches.</summary>
    private static Length Across(SheetAxis axis, int index, int offset, double units)
        => axis.SizeAt(index) * Math.Min(offset / units, 1.0);

    /// <summary>
    /// The eighteen bytes of a BIFF8 client anchor: a flags word and four cell-and-offset pairs.
    /// </summary>
    /// <remarks>
    /// The column offsets are in 1024ths of the column's width and the row offsets in 256ths of
    /// the row's height — <c>lclGetXFromCol</c> and <c>lclGetYFromRow</c>
    /// (<c>sc/source/filter/excel/xlescher.cxx:54-67</c>). The asymmetry is the format's.
    /// </remarks>
    private static Anchor? ClientAnchor(DffRecordBuffer buffer, EscherShape shape)
    {
        if (shape.ClientAnchor is not { } header) return null;

        ReadOnlySpan<byte> content = buffer.Content(header);
        if (content.Length < 18) return null;

        return new Anchor(
            DffRecordBuffer.ReadUInt16(content[2..]),
            DffRecordBuffer.ReadUInt16(content[4..]),
            DffRecordBuffer.ReadUInt16(content[6..]),
            DffRecordBuffer.ReadUInt16(content[8..]),
            DffRecordBuffer.ReadUInt16(content[10..]),
            DffRecordBuffer.ReadUInt16(content[12..]),
            DffRecordBuffer.ReadUInt16(content[14..]),
            DffRecordBuffer.ReadUInt16(content[16..]));
    }

    private readonly record struct Anchor(
        int FirstColumn, int LeftOffset, int FirstRow, int TopOffset,
        int LastColumn, int RightOffset, int LastRow, int BottomOffset);

    private readonly record struct ObjectEntry(
        ushort Type,
        int DrawingOffset,
        ushort Id = 0,
        string? Text = null,
        int Horizontal = 0,
        int Vertical = 0,
        bool IsPrintable = true,
        ChartPlot? Chart = null,
        IReadOnlyList<TextRun>? Runs = null,
        XlsDrawingCollector? Overlay = null);

    /// <summary>
    /// One entry of a <c>TXO</c>'s formatting-run array: where a face changes, and to which.
    /// </summary>
    /// <param name="Character">The zero-based character index the run starts at.</param>
    /// <param name="FontIndex">The <c>FONT</c> index it changes to, the hole at four included.</param>
    private readonly record struct TextRun(int Character, int FontIndex);

    /// <summary>The <c>ftCmo</c> subrecord identifier, <c>EXC_ID_OBJCMO</c>.</summary>
    private const ushort ObjectCommon = 0x0015;

    /// <summary>
    /// The <c>ftCmo</c> object type an embedded chart has.
    /// </summary>
    /// <remarks><c>EXC_OBJTYPE_CHART</c>, <c>sc/source/filter/inc/xlescher.hxx:49</c>.</remarks>
    private const ushort ChartObject = 5;

    /// <summary>
    /// The <c>ftCmo</c> object type a cell comment has.
    /// </summary>
    /// <remarks><c>EXC_OBJTYPE_NOTE</c>, <c>sc/source/filter/inc/xlescher.hxx:69</c>.</remarks>
    private const ushort NoteObject = 25;

    /// <summary>
    /// <c>ftCmo</c>'s <em>printable</em> flag, <c>EXC_OBJCMO_PRINTABLE</c>.
    /// </summary>
    /// <remarks><c>sc/source/filter/inc/xlescher.hxx:228</c>.</remarks>
    private const ushort ObjectPrintable = 0x0010;

    /// <summary>
    /// True when an <c>ftCmo</c> object type is one of Excel's form controls.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Exactly the eleven types Calc's factory turns into an <c>XclImpTbxObjBase</c> — the class
    /// that also carries <c>XclImpControlHelper</c> — which is button, check box, option button,
    /// edit box, label, dialog, spin control, scroll bar, list box, group box and drop-down
    /// (<c>XclImpDrawObjBase::ReadObj8</c>, <c>sc/source/filter/excel/xiescher.cxx:280-292</c>,
    /// against the hierarchy at <c>sc/source/filter/inc/xiescher.hxx:503-776</c>).
    /// </para>
    /// <para>
    /// The set matters because the <em>printable</em> flag is only acted on for these. Calc reads
    /// it for every object, but the only place it reaches the drawing is
    /// <c>XclImpControlHelper::ProcessControl</c>, which writes it to the control model's
    /// <c>Printable</c> property (<c>xiescher.cxx:1998</c>); for a plain shape
    /// <c>DoPreProcessSdrObj</c> merely traces that the object is not printable
    /// (<c>xiescher.cxx:843-845</c>) and draws it anyway. Applying the flag to every object would
    /// therefore drop shapes the reference prints — measured on <c>PC1000.xls</c>, whose yellow
    /// instruction rectangle and its picture both carry the flag clear and both appear in the
    /// reference rendering, beside six buttons that do not.
    /// </para>
    /// </remarks>
    /// <param name="type">The <c>ftCmo</c> object type.</param>
    private static bool IsFormControl(ushort type) => type is 7 or (>= 11 and <= 20);

    /// <summary>
    /// Whether a shape's Escher fill and outline survive Calc's import of it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// They survive for every object that keeps the <c>SdrObject</c> the DFF import built for it,
    /// and only for those. <c>XclImpDffConverter::ProcessObj</c> asks the Excel object to
    /// <c>CreateSdrObject</c> and puts whatever it returns <em>in place of</em> the DFF one
    /// (<c>sc/source/filter/excel/xiescher.cxx</c>:3686-3689) — dropping the item set
    /// <c>ApplyAttributes</c> filled — and the three classes that return one are the chart, the
    /// TBX form control and the OLE object.
    /// </para>
    /// <para>
    /// <strong>The OLE test is the shape's own <c>pictureId</c>, not its object type.</strong> A
    /// plain BIFF8 picture and an embedded object are both <c>ftCmo</c> type 8, and
    /// <c>SvxMSDffManager::ImportGraphic</c> separates them on exactly that property
    /// (<c>filter/source/msfilter/msdffimp.cxx</c>:4025-4030): with it the shape becomes an
    /// <c>SdrOle2Obj</c> and loses its attributes, without it an <c>SdrGrafObj</c> keeps them. The
    /// witness is <c>TICAPCapability_Final.xls</c>' <c>Picture 228</c>, which hard-states
    /// <c>fFilled</c> and <c>fLine</c> true with a white fill and a black line and which 26.2.4.2
    /// draws with neither; it carries <c>pictureId</c> and the OLE shape flag together.
    /// </para>
    /// </remarks>
    /// <param name="shape">The Escher shape.</param>
    /// <param name="entry">Its <c>OBJ</c> record.</param>
    private static bool KeepsEscherInk(EscherShape shape, ObjectEntry entry)
        => entry.Type != ChartObject
            && !IsFormControl(entry.Type)

            // A group's own shape is not drawn. `SvxMSDffManager::ImportShape` branches on
            // `ShapeFlag::Group` *before* the branch that builds an item set, so a group object
            // never reaches `ApplyAttributes` and its stated fill and line reach nothing
            // (`filter/source/msfilter/msdffimp.cxx`:4376-4386 **in this tree**, 27.2.0.0.alpha0+,
            // which is not the reference binary's source). Measured at the binary: on
            // `EHEST-Pre-departure-checklist` page 8 the three-box group states a white fill and
            // a black outline over 391 x 33 pt and 26.2.4.2 draws no rectangle there at all.
            && (shape.Flags & EscherShapeAttributes.Group) == 0
            && (shape.Flags & EscherShapeAttributes.OleShape) == 0
            && !shape.Properties.Has(EscherPropertyIds.PictureId);

    private const int HorizontalCentre = 2;
    private const int HorizontalRight = 3;
    private const int VerticalCentre = 2;
    private const int VerticalBottom = 3;

    private const double ColumnUnits = 1024.0;
    private const double RowUnits = 256.0;

    /// <summary>
    /// The size a run of shape text is set at when the file states none.
    /// </summary>
    /// <remarks>
    /// Ten point, which is Excel's default text-box font and what the <c>TXO</c>'s formatting
    /// runs would name were they read. It is not <see cref="SheetShapeText.DefaultSize"/>: that
    /// is DrawingML's eighteen point, which is right for a SpreadsheetML body and nearly twice
    /// what a BIFF text box shows.
    /// </remarks>
    private static readonly Length DefaultTextSize = Length.FromPoints(10);

    /// <summary>The inset Excel leaves either side of a text box's text.</summary>
    private static readonly Length TextInset = Length.FromMm100(10);

    /// <summary>
    /// The margin Excel puts on all four sides of a text box that asks the host for one.
    /// </summary>
    /// <remarks><c>EXC_OBJ_TEXT_MARGIN</c>, <c>sc/source/filter/inc/xlescher.hxx:140</c>.</remarks>
    private static readonly Length AutoTextMargin = Length.FromEmu(20000);
}
