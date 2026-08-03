using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.Ooxml;

namespace Paperless.Spreadsheets.Xlsb;

/// <summary>
/// Reads XLSB workbooks: BIFF12 record streams inside an OPC package.
/// </summary>
/// <remarks>
/// <para>
/// The same content-tree shape the other three readers produce — a
/// <see cref="SectionKind.Sheet"/> section per sheet holding one <see cref="ContentTable"/>,
/// hidden sheets extracted and flagged rather than skipped, each cell carrying its typed value
/// and the text Excel displayed — with one deliberate difference that XLSB shares with XLS and
/// not with XLSX: <c>Formula</c> is null, because a BIFF12 formula is a token array and the
/// cached result is what a reference renderer draws.
/// </para>
/// <para>
/// It is the same workbook model as XLSX and it is not the same reader, which is worth being
/// clear about. The package handling is XLSX's, because OPC is OPC; the record framing is
/// BIFF's, because that is what the parts are; and the number formats, the date epoch and the
/// print geometry are one implementation shared with the XML path, because LibreOffice puts both
/// formats through the same buffers and reading them differently would be a defect rather than a
/// dialect.
/// </para>
/// </remarks>
public static class XlsbReader
{
    /// <summary>Reads a workbook, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    public static XlsbSpreadsheetDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        XlsbFile file = XlsbFile.Open(source.Stream, leaveOpen: true);
        try
        {
            // Diagnostics accumulate while the sheets are walked — a part that will not resolve
            // is only discovered then — so the two lists are joined at the end rather than
            // snapshotted up front, where everything found during the walk would be lost.
            List<Diagnostic> diagnostics = [];
            ContentDocument content = new()
            {
                Metadata = OoxmlMetadata.Read(
                    (OpcPackage)file.PackageHandle, DocumentFamily.Spreadsheet),
            };

            XlsbSheetReader reader = new(file, diagnostics);
            List<SheetLayout> layouts = [];

            // Read once for the whole workbook, exactly as the XML path does: only a chart part
            // ever asks for it, and a chart part is XML in an XLSB too.
            DrawingTheme? theme = DrawingTheme.Read(file.ThemeRoot);

            // And its other half, for a text box whose runs name their face through the theme.
            DrawingFontScheme? themeFonts = DrawingFontScheme.Read(
                Drawing.Child(Drawing.Child(file.ThemeRoot, "themeElements"), "fontScheme"));

            foreach (XlsxSheetEntry entry in file.Sheets)
            {
                ContentSection section = new()
                {
                    Kind = SectionKind.Sheet,
                    Index = entry.Index,
                    Name = entry.Name,
                    IsHidden = entry.IsHidden,
                };

                // Read once and walked twice: the cells and the print geometry live in the same
                // part, and re-reading it would be a second chance to disagree with itself.
                byte[]? part = file.LoadSheet(entry);

                section.Children.Add(reader.ReadSheet(part));
                content.Children.Add(section);

                (SheetPrintSetup setup, SheetGrid grid) = XlsbPrintSetup.Read(part, file.DefaultFont);
                layouts.Add(new SheetLayout
                {
                    Name = entry.Name,
                    Index = entry.Index,
                    IsHidden = entry.IsHidden,
                    Setup = setup,
                    Grid = grid,
                    Cells = (ContentTable)section.Children[0],
                    StatedMerges = reader.SheetMerges,
                    HyperlinkRanges = reader.SheetHyperlinks,

                    // The XML readers, unchanged and deliberately so. A drawing hangs off the
                    // worksheet part by a relationship, and everything on the far side of that
                    // relationship — `xl/drawings/drawing1.xml`, the chart space, the images — is
                    // XML in an XLSB exactly as in an XLSX, because none of DrawingML has a
                    // binary encoding. So the only thing BIFF12 contributes here is the part
                    // *name*, which the workbook already gave. Leaving this unwired was worth
                    // 8 words and a page on `sc/qa/unit/data/xlsb/tdf108017_calcProtection.xlsb`:
                    // its chart read into nothing and its second page never existed.
                    Drawings = XlsxDrawings.Read(
                        file.PackageHandle, entry.PartName, theme, themeFonts),
                    FileName = source.FileName ?? string.Empty,
                });

                // A chart follows its sheet rather than going inside it, because a sheet section
                // holds exactly one table and a chart is another one. Both other OOXML paths put
                // it in the same place.
                foreach (ContentSection chart in XlsxCharts.Read(file.PackageHandle, entry.PartName))
                    content.Children.Add(chart);
            }

            return new XlsbSpreadsheetDocument(
                format, file, content, [.. file.Diagnostics, .. diagnostics], layouts);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }
}

/// <summary>An XLSB workbook that has been read.</summary>
public sealed class XlsbSpreadsheetDocument : IPaginatedDocument
{
    private readonly XlsbFile _file;

    internal XlsbSpreadsheetDocument(
        DocumentFormat format,
        XlsbFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<SheetLayout> sheets)
    {
        Format = format;
        _file = file;
        Content = content;
        Diagnostics = diagnostics;
        Sheets = sheets;
    }

    /// <summary>The sheets' print setups and geometry, in workbook order.</summary>
    public IReadOnlyList<SheetLayout> Sheets { get; }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.Spreadsheet;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics { get; }

    /// <inheritdoc/>
    public IPageSequence Layout(LayoutOptions? options = null)
        => new SpreadsheetPages(Sheets, options);

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
