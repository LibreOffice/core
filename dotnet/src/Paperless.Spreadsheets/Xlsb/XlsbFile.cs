using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Numbers;
using Paperless.Ooxml;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.Ooxml;

namespace Paperless.Spreadsheets.Xlsb;

/// <summary>
/// An open XLSB package: the workbook's sheet list, its shared strings and its styles.
/// </summary>
/// <remarks>
/// <para>
/// XLSB is the same logical workbook as XLSX in a different encoding, and it is worth being
/// precise about which half is which. The <em>package</em> is OPC and identical: parts, content
/// types, and a workbook part that names every other part by relationship rather than by path.
/// The <em>parts</em> are BIFF12 record streams rather than XML. So this class is
/// <see cref="XlsxFile"/>'s shape with <see cref="Biff12Stream"/> where the XML parser was, and
/// nothing about locating a part differs — which is why the reader lives here rather than in
/// <c>MsBinary/</c>, whose subject is OLE2 and BIFF8's own framing.
/// </para>
/// <para>
/// <strong>LibreOffice cannot write XLSB</strong> — its filter is import-only — so every test
/// file has to come from Excel, and the ten in <c>sc/qa/unit/data/xlsb/</c> are what this was
/// measured against. That constraint is what makes LibreOffice a usable oracle anyway: it
/// <em>reads</em> the format, so its own conversion of a file Excel wrote is ground truth.
/// </para>
/// </remarks>
internal sealed class XlsbFile : IDisposable
{
    private const string RelationshipBase =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

    private readonly OpcPackage _package;
    private readonly List<Diagnostic> _diagnostics = [];
    private readonly Dictionary<string, OpcXml.Relationship> _workbookRelationships =
        new(StringComparer.Ordinal);

    private XlsbFile(OpcPackage package, IPackagePart workbookPart, byte[] workbook)
    {
        _package = package;
        WorkbookPartName = workbookPart.Name;
        _diagnostics.AddRange(package.Diagnostics);

        foreach (OpcXml.Relationship relationship in package.GetRelationships(workbookPart.Name))
            _workbookRelationships[relationship.Id] = relationship;

        SharedStrings = XlsbSharedStrings.Read(LoadRelated("sharedStrings", "xl/sharedStrings.bin"));
        (Styles, DefaultFont) = XlsbStyles.Read(LoadRelated("styles", "xl/styles.bin"));

        // Still XML, and that is the whole point of the remark above: only the parts the
        // *spreadsheet* filter writes are BIFF12. A theme belongs to DrawingML, which has no
        // binary encoding at all, so `xl/theme/theme1.xml` is byte-for-byte what an XLSX holds
        // and is parsed by the same reader.
        using (Stream? theme = OpenRelated("theme", "xl/theme/theme1.xml"))
            ThemeRoot = theme is null ? null : OoxmlXml.TryLoad(theme, out _);

        (Sheets, DateSystem) = ReadWorkbook(workbook);
    }

    /// <summary>The workbook part's name.</summary>
    public string WorkbookPartName { get; }

    /// <summary>The sheets, in the order the workbook lists them.</summary>
    public IReadOnlyList<XlsxSheetEntry> Sheets { get; }

    /// <summary>The shared string table.</summary>
    public IReadOnlyList<string> SharedStrings { get; }

    /// <summary>The number format each cell format names.</summary>
    public XlsxStyles Styles { get; }

    /// <summary>
    /// The workbook's default font, which a column width is a count of digits of.
    /// </summary>
    /// <remarks>Null when <c>styles.bin</c> lists no font. See <c>SheetColumnDigits</c>.</remarks>
    public SheetDefaultFont? DefaultFont { get; }

    /// <summary>The workbook's <c>theme</c> root, for an <c>a:schemeClr</c> inside a chart.</summary>
    public XElement? ThemeRoot { get; }

    /// <summary>Which epoch this workbook counts date serials from.</summary>
    public SpreadsheetDateSystem DateSystem { get; }

    /// <summary>Problems found while opening the workbook.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <summary>The package, for the metadata reader and for parts loaded on demand.</summary>
    public IPackage PackageHandle => _package;

    /// <summary>Opens an XLSB package over a seekable stream.</summary>
    /// <param name="stream">The package bytes.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <exception cref="MalformedDocumentException">There is no readable workbook part.</exception>
    public static XlsbFile Open(Stream stream, bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(stream);

        OpcPackage package = OpcPackage.Open(stream, leaveOpen);
        try
        {
            // The conventional name is a fallback only. XLSB's own convention is `workbook.bin`,
            // and a macro-enabled one is still called that — the extension distinguishes the
            // *package*, never the part.
            IPackagePart? main = package.GetMainDocumentPart() ?? package.GetPart("xl/workbook.bin");
            if (main is null)
            {
                throw new MalformedDocumentException(
                    "The XLSB package has no workbook part: neither the officeDocument "
                    + "relationship nor the conventional 'xl/workbook.bin' resolves.");
            }

            return new XlsbFile(package, main, ReadAll(main));
        }
        catch
        {
            package.Dispose();
            throw;
        }
    }

    /// <summary>A sheet's worksheet part, or null when it is missing.</summary>
    public byte[]? LoadSheet(XlsxSheetEntry sheet)
    {
        ArgumentNullException.ThrowIfNull(sheet);
        if (sheet.PartName is null) return null;

        IPackagePart? part = _package.GetPart(sheet.PartName);
        if (part is not null) return ReadAll(part);

        _diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Warning, "PL2141",
            $"Sheet '{sheet.Name}' names the worksheet part '{sheet.PartName}', which the "
            + "package does not contain, so it has been extracted as empty.",
            new DiagnosticLocation(sheet.PartName)));
        return null;
    }

    /// <inheritdoc/>
    public void Dispose() => _package.Dispose();

    /// <summary>
    /// Reads the workbook part: the sheet list and the date epoch.
    /// </summary>
    /// <remarks>
    /// <c>SHEET</c> is a state, a sheet id, a relationship id and a name
    /// (<c>WorksheetBuffer::importSheet</c>, <c>sc/source/filter/oox/worksheetbuffer.cxx:61</c>),
    /// and the state's three values are the same three <c>state</c> spells out in XML: visible,
    /// hidden, very hidden. Both non-visible ones are extracted and flagged rather than skipped,
    /// which is what the other three readers do.
    /// </remarks>
    private (List<XlsxSheetEntry> Sheets, SpreadsheetDateSystem Dates) ReadWorkbook(byte[] workbook)
    {
        const uint Date1904 = 0x00000001;

        List<XlsxSheetEntry> sheets = [];
        SpreadsheetDateSystem dates = SpreadsheetDateSystem.Date1900;
        int index = 0;

        foreach (Biff12Record record in Biff12Stream.Records(workbook))
        {
            switch (record.Id)
            {
                case Biff12.WorkbookPr:
                {
                    Biff12Cursor cursor = new(record.Data.Span);
                    dates = (cursor.ReadUInt32() & Date1904) != 0
                        ? SpreadsheetDateSystem.Date1904
                        : SpreadsheetDateSystem.Date1900;
                    break;
                }

                case Biff12.Sheet:
                {
                    Biff12Cursor cursor = new(record.Data.Span);
                    int state = cursor.ReadInt32();
                    _ = cursor.ReadInt32();
                    string relationshipId = cursor.ReadString();
                    string name = cursor.ReadString();

                    string? target = null;
                    if (_workbookRelationships.TryGetValue(relationshipId, out OpcXml.Relationship found)
                        && !found.IsExternal)
                    {
                        target = _package.GetPart(found.Target)?.Name;
                    }
                    target ??= _package.GetPart($"xl/worksheets/sheet{index + 1}.bin")?.Name;

                    sheets.Add(new XlsxSheetEntry(
                        name.Length > 0 ? name : $"Sheet{index + 1}", index, state != 0, target));

                    if (target is null)
                    {
                        _diagnostics.Add(new Diagnostic(
                            DiagnosticSeverity.Warning, "PL2141",
                            $"Sheet '{name}' names no readable worksheet part, so it has been "
                            + "extracted as empty."));
                    }
                    index++;
                    break;
                }
            }
        }

        if (sheets.Count == 0)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2146",
                "The workbook lists no sheets, so it has no content to extract."));
        }

        return (sheets, dates);
    }

    private byte[]? LoadRelated(string relationshipSuffix, string conventionalName)
    {
        using Stream? content = OpenRelated(relationshipSuffix, conventionalName);
        if (content is null) return null;

        using MemoryStream buffer = new();
        content.CopyTo(buffer);
        return buffer.ToArray();
    }

    private Stream? OpenRelated(string relationshipSuffix, string conventionalName)
    {
        string? target = null;
        foreach (OpcXml.Relationship relationship in _workbookRelationships.Values)
        {
            if (relationship.IsExternal) continue;
            if (!string.Equals(relationship.Type, RelationshipBase + relationshipSuffix,
                               StringComparison.Ordinal))
                continue;
            target = relationship.Target;
            break;
        }

        IPackagePart? part = (target is null ? null : _package.GetPart(target))
                             ?? _package.GetPart(conventionalName);
        return part?.Open();
    }

    private static byte[] ReadAll(IPackagePart part)
    {
        using Stream content = part.Open();
        using MemoryStream buffer = new();
        content.CopyTo(buffer);
        return buffer.ToArray();
    }
}
