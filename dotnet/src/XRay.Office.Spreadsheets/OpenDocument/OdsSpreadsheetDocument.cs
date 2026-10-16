using System.Xml.Linq;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Formats;
using XRay.Office.OpenDocument;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// An ODF spreadsheet that has been read, and can be laid out into printed pages.
/// </summary>
/// <remarks>
/// <para>
/// A wrapper over <see cref="OdfDocument"/> rather than a replacement for it. Extraction is
/// identical for all three ODF families and lives in <c>XRay.Office.OpenDocument</c>, where it
/// has no business knowing what a print range is; pagination is Calc's alone. Wrapping keeps
/// the shared reader shared and puts the spreadsheet-only half where the rest of the
/// spreadsheet-only code is.
/// </para>
/// <para>
/// Disposal passes through to the wrapped document, which owns the package.
/// </para>
/// </remarks>
public sealed class OdsSpreadsheetDocument : IPaginatedDocument
{
    private readonly OdfDocument _document;

    internal OdsSpreadsheetDocument(OdfDocument document, IReadOnlyList<SheetLayout> sheets)
    {
        _document = document;
        Sheets = sheets;
    }

    /// <summary>The sheets' print setups and geometry, in document order.</summary>
    public IReadOnlyList<SheetLayout> Sheets { get; }

    /// <summary>The underlying ODF document: its package, styles and master pages.</summary>
    public OdfDocument Document => _document;

    /// <inheritdoc/>
    public DocumentFormat Format => _document.Format;

    /// <inheritdoc/>
    public DocumentFamily Family => _document.Family;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => _document.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content => _document.Content;

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics => _document.Diagnostics;

    /// <inheritdoc/>
    public IPageSequence Layout(LayoutOptions? options = null)
        => XRay.Office.Core.Threading.OfficeOperation.Layout(options, () => LayoutCore(options));

    private SpreadsheetPages LayoutCore(LayoutOptions? options)
        => new SpreadsheetPages(Sheets, options);

    /// <inheritdoc/>
    public void Dispose() => _document.Dispose();

    /// <summary>
    /// Pairs each sheet's extracted cells with the print setup read from its own element.
    /// </summary>
    /// <remarks>
    /// A second walk over the body, deliberately, and it is cheap: the document is already
    /// parsed into an element tree, so this reads attributes off elements that are in memory
    /// rather than re-parsing anything. Threading the print setup through
    /// <c>OdfReader.ReadBody</c> instead would put spreadsheet-only state on the reader all
    /// three families share.
    /// </remarks>
    internal static List<SheetLayout> ReadSheets(OdfDocument document, string? fileName = null)
    {
        List<SheetLayout> sheets = [];
        XElement? body = document.File.Body;
        if (body is null) return sheets;

        List<ContentSection> sections =
        [
            .. document.Content.Children.OfType<ContentSection>()
                       .Where(section => section.Kind == SectionKind.Sheet),
        ];

        int index = 0;
        foreach (XElement table in body.Elements(XName.Get("table", OdfNamespaces.Table)))
        {
            (SheetPrintSetup setup, SheetGrid grid) = OdsPrintSetup.Read(document.File, table);
            (SheetCellFormats formats, SheetRichText rich) = OdsCellFormats.Read(document.File, table);
            formats = formats.WithConditionalText(
                OdsConditionalText.Read(document.File.Styles, table));
            ContentSection? section = sections.FirstOrDefault(s => s.Index == index);

            sheets.Add(new SheetLayout
            {
                Name = section?.Name ?? $"Sheet{index + 1}",
                Index = index,
                IsHidden = section?.IsHidden ?? false,
                Setup = setup,
                Grid = grid,
                Cells = section?.Children.OfType<ContentTable>().FirstOrDefault(),
                StatedMerges = OdsMerges.Read(table),
                HyperlinkRanges = OdsMerges.ReadHyperlinks(table),
                ConditionalRanges = OdsConditionalFormats.ReadRanges(table),
                Notes = setup.PrintsNotes ? OdsNotes.Read(table) : SheetNotes.Empty,
                Formatting = OdsCellDecoration.Read(document.File.Styles, table),
                Formats = formats,
                RichText = rich,
                Drawings = OdsDrawings.Read(document.File, table),
                CellBreaksStartLines = true,
                FileName = fileName ?? string.Empty,
            });

            index++;
        }

        return sheets;
    }
}
