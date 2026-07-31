using Paperless.Containers;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Formats;
using Paperless.Spreadsheets.OpenDocument;

namespace Paperless.Spreadsheets;

/// <summary>
/// Reads spreadsheets: XLSX and its variants, XLSB, legacy XLS, ODS and CSV.
/// The Paperless counterpart of LibreOffice Calc.
/// </summary>
/// <remarks>
/// <para>
/// Two things make spreadsheets unlike the other two families. Cell content and cell
/// formatting are stored independently — a run-length structure of formatting keyed by
/// row, separate from the cells themselves — so the reader keeps them apart rather than
/// merging them into per-cell objects, which is both how LibreOffice does it and vastly
/// cheaper on sheets with large uniformly-formatted regions.
/// </para>
/// <para>
/// And a spreadsheet has no intrinsic pagination. Pages exist only once print settings,
/// scaling and page breaks are applied, so rendering a spreadsheet means reproducing
/// the print-layout algorithm rather than reading a page size out of the file.
/// </para>
/// <para>
/// See <c>dotnet/research/03-calc.md</c>.
/// </para>
/// </remarks>
public sealed class SpreadsheetReader : IDocumentReader
{
    /// <inheritdoc/>
    public IReadOnlyCollection<DocumentFormat> SupportedFormats { get; } =
    [
        DocumentFormat.Xlsx, DocumentFormat.Xlsm, DocumentFormat.Xltx, DocumentFormat.Xltm,
        DocumentFormat.Xlsb,
        DocumentFormat.Xls, DocumentFormat.Xlt, DocumentFormat.Xls5,
        DocumentFormat.ExcelXml2003, DocumentFormat.Csv, DocumentFormat.Dif,
        DocumentFormat.Ods, DocumentFormat.Ots, DocumentFormat.Fods,
        DocumentFormat.Sxc, DocumentFormat.Stc,
    ];

    /// <inheritdoc/>
    public IDocument Read(DocumentSource source)
    {
        ArgumentNullException.ThrowIfNull(source);
        DocumentFormat format = SourceIdentification.Resolve(source);

        return format switch
        {
            DocumentFormat.Ods or DocumentFormat.Ots or DocumentFormat.Fods
                => new OdsReader().Read(source, format),

            DocumentFormat.Xlsx or DocumentFormat.Xlsm or DocumentFormat.Xltx or DocumentFormat.Xltm
                => Ooxml.XlsxReader.Read(source, format),

            // Named in SupportedFormats but not implemented yet.
            DocumentFormat.Xlsb or DocumentFormat.Xls or DocumentFormat.Xlt
                or DocumentFormat.Xls5 or DocumentFormat.ExcelXml2003 or DocumentFormat.Csv
                or DocumentFormat.Dif or DocumentFormat.Sxc or DocumentFormat.Stc
                => throw new UnsupportedFormatException(
                    format, $"Reading {format} is not implemented yet."),

            _ => throw new UnsupportedFormatException(format, $"{format} is not a spreadsheet format."),
        };
    }
}
