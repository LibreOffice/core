using System.Collections.Frozen;

namespace Paperless.Core.Formats;

/// <summary>
/// The built-in registry of every format Paperless knows about.
/// </summary>
/// <remarks>
/// Derived from LibreOffice's own type registry
/// (<c>filter/source/config/fragments/types/*.xcu</c>); the table it was transcribed from,
/// with per-entry citations, is in <c>dotnet/research/01-formats-and-detection.md</c>
/// section 3.
/// <para>
/// Note that <see cref="FormatInfo.MediaType"/> here is the format's *canonical* media
/// type, which is not always the one LibreOffice registers. LibreOffice registers
/// <c>application/msword</c> for the ECMA-named DOCX types, for instance, because those
/// entries predate the ISO names. Detection never relies on these values — it uses the
/// container's own declared content type — so recording the canonical one is both more
/// useful to callers and harmless.
/// </para>
/// </remarks>
public sealed class FormatCatalogue : IFormatCatalogue
{
    /// <summary>The shared instance. The catalogue is immutable, so one is enough.</summary>
    public static FormatCatalogue Instance { get; } = new();

    private readonly FrozenDictionary<DocumentFormat, FormatInfo> _byFormat;
    private readonly FrozenDictionary<string, FormatInfo[]> _byExtension;

    private FormatCatalogue()
    {
        FormatInfo[] all = Build();
        _byFormat = all.ToFrozenDictionary(f => f.Format);

        // An extension can denote several formats: '.xml' covers all three flat-ODF
        // variants, and '.stw' is registered twice by LibreOffice with different media
        // types. So this maps to a list, and callers must treat it as a hint only.
        _byExtension = all
            .SelectMany(f => f.AlternateExtensions.Prepend(f.Extension).Select(e => (Ext: e, Info: f)))
            .GroupBy(x => x.Ext, StringComparer.OrdinalIgnoreCase)
            .ToFrozenDictionary(g => g.Key, g => g.Select(x => x.Info).ToArray(),
                               StringComparer.OrdinalIgnoreCase);
    }

    /// <inheritdoc/>
    public IReadOnlyCollection<FormatInfo> All => _byFormat.Values;

    /// <inheritdoc/>
    public FormatInfo? GetInfo(DocumentFormat format)
        => _byFormat.TryGetValue(format, out FormatInfo? info) ? info : null;

    /// <inheritdoc/>
    public IReadOnlyList<FormatInfo> FindByExtension(string extension)
    {
        ArgumentNullException.ThrowIfNull(extension);
        string key = extension.TrimStart('.');
        return _byExtension.TryGetValue(key, out FormatInfo[]? hits) ? hits : [];
    }

    private static FormatInfo[] Build() =>
    [
        // ---- Word processing: OOXML --------------------------------------------------
        New(DocumentFormat.Docx, DocumentFamily.WordProcessing, ContainerKind.Zip, "docx",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
            "Word 2007-365", read: true),
        New(DocumentFormat.Docm, DocumentFamily.WordProcessing, ContainerKind.Zip, "docm",
            "application/vnd.ms-word.document.macroEnabled.12",
            "Word 2007-365 (macro-enabled)", macros: true, read: true),
        New(DocumentFormat.Dotx, DocumentFamily.WordProcessing, ContainerKind.Zip, "dotx",
            "application/vnd.openxmlformats-officedocument.wordprocessingml.template",
            "Word 2007-365 Template", template: true, read: true),
        New(DocumentFormat.Dotm, DocumentFamily.WordProcessing, ContainerKind.Zip, "dotm",
            "application/vnd.ms-word.template.macroEnabledTemplate.12",
            "Word 2007-365 Template (macro-enabled)", template: true, macros: true, read: true),

        // ---- Word processing: legacy binary ------------------------------------------
        // Macro-capable: WW8 stores VBA in a Macros/ storage inside the same OLE2 file.
        New(DocumentFormat.Doc, DocumentFamily.WordProcessing, ContainerKind.Ole2CompoundFile,
            "doc", "application/msword", "Word 97-2003", macros: true, read: true),
        New(DocumentFormat.Dot, DocumentFamily.WordProcessing, ContainerKind.Ole2CompoundFile,
            "dot", "application/msword", "Word 97-2003 Template", template: true, macros: true,
            read: true),

        // ---- Word processing: other ---------------------------------------------------
        New(DocumentFormat.Rtf, DocumentFamily.WordProcessing, ContainerKind.PlainText, "rtf",
            "application/rtf", "Rich Text Format", read: true),
        New(DocumentFormat.WordXml2003, DocumentFamily.WordProcessing, ContainerKind.Xml, "xml",
            "text/xml", "Word 2003 XML"),

        // ---- Word processing: ODF -----------------------------------------------------
        New(DocumentFormat.Odt, DocumentFamily.WordProcessing, ContainerKind.Zip, "odt",
            "application/vnd.oasis.opendocument.text", "ODF Text Document", read: true),
        New(DocumentFormat.Ott, DocumentFamily.WordProcessing, ContainerKind.Zip, "ott",
            "application/vnd.oasis.opendocument.text-template", "ODF Text Document Template",
            template: true, read: true),
        // The non-flat extension is also registered for the flat type in LibreOffice, so
        // that a mislabelled '.odt' still gets the flat filter offered as a candidate.
        New(DocumentFormat.Fodt, DocumentFamily.WordProcessing, ContainerKind.Xml, "fodt",
            "application/vnd.oasis.opendocument.text-flat-xml", "ODF Text Document (flat XML)",
            read: true, alternates: ["xml"]),
        New(DocumentFormat.Sxw, DocumentFamily.WordProcessing, ContainerKind.Zip, "sxw",
            "application/vnd.sun.xml.writer", "OpenOffice.org 1.x Writer"),
        New(DocumentFormat.Stw, DocumentFamily.WordProcessing, ContainerKind.Zip, "stw",
            "application/vnd.sun.xml.writer.template", "OpenOffice.org 1.x Writer Template",
            template: true),

        // ---- Spreadsheet: OOXML -------------------------------------------------------
        New(DocumentFormat.Xlsx, DocumentFamily.Spreadsheet, ContainerKind.Zip, "xlsx",
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
            "Excel 2007-365", read: true),
        New(DocumentFormat.Xlsm, DocumentFamily.Spreadsheet, ContainerKind.Zip, "xlsm",
            "application/vnd.ms-excel.sheet.macroEnabled.12",
            "Excel 2007-365 (macro-enabled)", macros: true, read: true),
        New(DocumentFormat.Xltx, DocumentFamily.Spreadsheet, ContainerKind.Zip, "xltx",
            "application/vnd.openxmlformats-officedocument.spreadsheetml.template",
            "Excel 2007-365 Template", template: true, read: true),
        New(DocumentFormat.Xltm, DocumentFamily.Spreadsheet, ContainerKind.Zip, "xltm",
            "application/vnd.ms-excel.template.macroEnabled.12",
            "Excel 2007-365 Template (macro-enabled)", template: true, macros: true,
            read: true),
        // Binary OOXML (BIFF12 parts in an OPC package). LibreOffice reads it but cannot
        // write it, so test files have to come from Excel.
        New(DocumentFormat.Xlsb, DocumentFamily.Spreadsheet, ContainerKind.Zip, "xlsb",
            "application/vnd.ms-excel.sheet.binary.macroEnabled.12",
            "Excel 2007-365 Binary", macros: true),

        // ---- Spreadsheet: legacy binary -----------------------------------------------
        New(DocumentFormat.Xls, DocumentFamily.Spreadsheet, ContainerKind.Ole2CompoundFile,
            "xls", "application/vnd.ms-excel", "Excel 97-2003", macros: true),
        New(DocumentFormat.Xlt, DocumentFamily.Spreadsheet, ContainerKind.Ole2CompoundFile,
            "xlt", "application/vnd.ms-excel", "Excel 97-2003 Template",
            template: true, macros: true),
        New(DocumentFormat.Xls5, DocumentFamily.Spreadsheet, ContainerKind.Ole2CompoundFile,
            "xls", "application/vnd.ms-excel", "Excel 5.0/95", macros: true),

        // ---- Spreadsheet: other -------------------------------------------------------
        New(DocumentFormat.ExcelXml2003, DocumentFamily.Spreadsheet, ContainerKind.Xml, "xml",
            "text/xml", "Excel 2003 XML"),
        New(DocumentFormat.Csv, DocumentFamily.Spreadsheet, ContainerKind.PlainText, "csv",
            "text/csv", "Text CSV", alternates: ["tsv", "tab"]),
        New(DocumentFormat.Dif, DocumentFamily.Spreadsheet, ContainerKind.PlainText, "dif",
            "text/plain", "Data Interchange Format"),

        // ---- Spreadsheet: ODF ---------------------------------------------------------
        New(DocumentFormat.Ods, DocumentFamily.Spreadsheet, ContainerKind.Zip, "ods",
            "application/vnd.oasis.opendocument.spreadsheet", "ODF Spreadsheet", read: true),
        New(DocumentFormat.Ots, DocumentFamily.Spreadsheet, ContainerKind.Zip, "ots",
            "application/vnd.oasis.opendocument.spreadsheet-template",
            "ODF Spreadsheet Template", template: true, read: true),
        New(DocumentFormat.Fods, DocumentFamily.Spreadsheet, ContainerKind.Xml, "fods",
            "application/vnd.oasis.opendocument.spreadsheet-flat-xml",
            "ODF Spreadsheet (flat XML)", read: true),
        New(DocumentFormat.Sxc, DocumentFamily.Spreadsheet, ContainerKind.Zip, "sxc",
            "application/vnd.sun.xml.calc", "OpenOffice.org 1.x Calc"),
        New(DocumentFormat.Stc, DocumentFamily.Spreadsheet, ContainerKind.Zip, "stc",
            "application/vnd.sun.xml.calc.template", "OpenOffice.org 1.x Calc Template",
            template: true),

        // ---- Presentation: OOXML ------------------------------------------------------
        New(DocumentFormat.Pptx, DocumentFamily.Presentation, ContainerKind.Zip, "pptx",
            "application/vnd.openxmlformats-officedocument.presentationml.presentation",
            "PowerPoint 2007-365"),
        New(DocumentFormat.Pptm, DocumentFamily.Presentation, ContainerKind.Zip, "pptm",
            "application/vnd.ms-powerpoint.presentation.macroEnabled.12",
            "PowerPoint 2007-365 (macro-enabled)", macros: true),
        New(DocumentFormat.Potx, DocumentFamily.Presentation, ContainerKind.Zip, "potx",
            "application/vnd.openxmlformats-officedocument.presentationml.template",
            "PowerPoint 2007-365 Template", template: true),
        New(DocumentFormat.Potm, DocumentFamily.Presentation, ContainerKind.Zip, "potm",
            "application/vnd.ms-powerpoint.template.macroEnabled.12",
            "PowerPoint 2007-365 Template (macro-enabled)", template: true, macros: true),
        New(DocumentFormat.Ppsx, DocumentFamily.Presentation, ContainerKind.Zip, "ppsx",
            "application/vnd.openxmlformats-officedocument.presentationml.slideshow",
            "PowerPoint 2007-365 Show"),
        // '.ppsm' is not a registered extension anywhere in LibreOffice's type registry
        // (see research doc section 10), but the content type exists and Excel/PowerPoint
        // do produce such files, so Paperless recognises it by content type.
        New(DocumentFormat.Ppsm, DocumentFamily.Presentation, ContainerKind.Zip, "ppsm",
            "application/vnd.ms-powerpoint.slideshow.macroEnabled.12",
            "PowerPoint 2007-365 Show (macro-enabled)", macros: true),

        // ---- Presentation: legacy binary ----------------------------------------------
        New(DocumentFormat.Ppt, DocumentFamily.Presentation, ContainerKind.Ole2CompoundFile,
            "ppt", "application/vnd.ms-powerpoint", "PowerPoint 97-2003", macros: true),
        New(DocumentFormat.Pot, DocumentFamily.Presentation, ContainerKind.Ole2CompoundFile,
            "pot", "application/vnd.ms-powerpoint", "PowerPoint 97-2003 Template",
            template: true, macros: true),
        New(DocumentFormat.Pps, DocumentFamily.Presentation, ContainerKind.Ole2CompoundFile,
            "pps", "application/vnd.ms-powerpoint", "PowerPoint 97-2003 Show", macros: true),

        // ---- Presentation: ODF --------------------------------------------------------
        New(DocumentFormat.Odp, DocumentFamily.Presentation, ContainerKind.Zip, "odp",
            "application/vnd.oasis.opendocument.presentation", "ODF Presentation", read: true),
        New(DocumentFormat.Otp, DocumentFamily.Presentation, ContainerKind.Zip, "otp",
            "application/vnd.oasis.opendocument.presentation-template",
            "ODF Presentation Template", template: true, read: true),
        New(DocumentFormat.Fodp, DocumentFamily.Presentation, ContainerKind.Xml, "fodp",
            "application/vnd.oasis.opendocument.presentation-flat-xml",
            "ODF Presentation (flat XML)", read: true),
        New(DocumentFormat.Sxi, DocumentFamily.Presentation, ContainerKind.Zip, "sxi",
            "application/vnd.sun.xml.impress", "OpenOffice.org 1.x Impress"),
        New(DocumentFormat.Sti, DocumentFamily.Presentation, ContainerKind.Zip, "sti",
            "application/vnd.sun.xml.impress.template",
            "OpenOffice.org 1.x Impress Template", template: true),
    ];

    private static FormatInfo New(
        DocumentFormat format,
        DocumentFamily family,
        ContainerKind container,
        string extension,
        string mediaType,
        string displayName,
        bool template = false,
        bool macros = false,
        bool read = false,
        string[]? alternates = null) => new()
        {
            Format = format,
            Family = family,
            Container = container,
            Extension = extension,
            MediaType = mediaType,
            DisplayName = displayName,
            IsTemplate = template,
            CanCarryMacros = macros,
            AlternateExtensions = alternates ?? [],
            // Set per format as readers land, and only once the format genuinely reads. This
            // is what `paperless identify` reports, so an optimistic value here is worse than
            // a missing feature.
            IsReadSupported = read,
        };
}
