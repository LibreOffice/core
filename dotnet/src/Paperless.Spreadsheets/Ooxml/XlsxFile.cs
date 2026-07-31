using System.Xml.Linq;
using Paperless.Containers;
using Paperless.Containers.Ooxml;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Ooxml;
using Paperless.Spreadsheets.Numbers;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// An open SpreadsheetML package: the workbook part, its sheet list, and the shared buffers
/// every sheet reads through.
/// </summary>
/// <remarks>
/// <para>
/// A workbook spreads itself over one part per sheet plus a shared string table and a style
/// sheet, and every one of them is named by <em>relationship</em> from the workbook part rather
/// than by path. The conventional names are used only as a fallback: a producer may call the
/// parts anything, and a sheet's <c>r:id</c> is the only thing that says which part is which
/// sheet — there is no ordering guarantee that <c>sheet1.xml</c> is the first sheet.
/// </para>
/// <para>
/// The shared string table and styles are loaded once and referenced by index from every sheet,
/// mirroring how LibreOffice's <c>WorkbookHelper</c> owns them
/// (<c>sc/source/filter/oox/workbookfragment.cxx</c>).
/// </para>
/// </remarks>
public sealed class XlsxFile : IDisposable
{
    private const string RelationshipBase =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/";

    private readonly OpcPackage _package;
    private readonly List<Diagnostic> _diagnostics = [];
    private readonly Dictionary<string, OpcXml.Relationship> _workbookRelationships =
        new(StringComparer.Ordinal);

    private XlsxFile(OpcPackage package, IPackagePart workbookPart, XElement workbook)
    {
        _package = package;
        WorkbookPartName = workbookPart.Name;
        Workbook = workbook;
        _diagnostics.AddRange(package.Diagnostics);

        foreach (OpcXml.Relationship relationship in package.GetRelationships(workbookPart.Name))
            _workbookRelationships[relationship.Id] = relationship;

        SharedStrings = XlsxSharedStrings.Read(
            LoadRelated("sharedStrings", "xl/sharedStrings.xml"));
        StylesRoot = LoadRelated("styles", "xl/styles.xml");
        Styles = XlsxStyles.Read(StylesRoot);
        ThemeRoot = LoadRelated("theme", "xl/theme/theme1.xml");

        // The 1904 epoch is a workbook-wide switch, and reading it wrong shifts every date in
        // the file by 1462 days. date1904 alone decides it: the neighbouring dateCompatibility
        // attribute only bounds the oldest recognisable date, which is why LibreOffice reads
        // the two separately (sc/source/filter/oox/workbooksettings.cxx:287).
        XElement? properties = Xlsx.Child(workbook, "workbookPr");
        DateSystem = Xlsx.Flag(properties, "date1904")
            ? SpreadsheetDateSystem.Date1904
            : SpreadsheetDateSystem.Date1900;

        Sheets = ReadSheetList(workbook);
    }

    /// <summary>The workbook part's name.</summary>
    public string WorkbookPartName { get; }

    /// <summary>The <c>workbook</c> root element.</summary>
    public XElement Workbook { get; }

    /// <summary>The sheets, in the order the workbook lists them.</summary>
    public IReadOnlyList<XlsxSheetEntry> Sheets { get; }

    /// <summary>The shared string table.</summary>
    public XlsxSharedStrings SharedStrings { get; }

    /// <summary>The number formats each cell format names.</summary>
    public XlsxStyles Styles { get; }

    /// <summary>
    /// The <c>styleSheet</c> root, kept so that rendering can read what extraction ignored.
    /// </summary>
    /// <remarks>
    /// The element rather than a second load: <see cref="Styles"/> deliberately reads only the
    /// number formats, and the fills and borders beside them are wanted only when a workbook is
    /// drawn. Opening the part twice would be a second chance for the two readers to disagree
    /// about the same file.
    /// </remarks>
    public XElement? StylesRoot { get; }

    /// <summary>The <c>theme</c> root, for the colours a fill names by index.</summary>
    public XElement? ThemeRoot { get; }

    /// <summary>Which epoch this workbook counts date serials from.</summary>
    public SpreadsheetDateSystem DateSystem { get; }

    /// <summary>The package, for reaching parts extraction does not load eagerly.</summary>
    public IPackage Package => _package;

    /// <summary>Problems found while opening the workbook.</summary>
    public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics;

    /// <summary>Opens a SpreadsheetML package over a seekable stream.</summary>
    /// <param name="stream">The package bytes.</param>
    /// <param name="leaveOpen">When true, disposing this does not dispose the stream.</param>
    /// <exception cref="MalformedDocumentException">
    /// There is no readable workbook part, so there is nothing to read.
    /// </exception>
    public static XlsxFile Open(Stream stream, bool leaveOpen = false)
    {
        ArgumentNullException.ThrowIfNull(stream);

        OpcPackage package = OpcPackage.Open(stream, leaveOpen);
        try
        {
            IPackagePart? main = package.GetMainDocumentPart() ?? package.GetPart("xl/workbook.xml");
            if (main is null)
            {
                throw new MalformedDocumentException(
                    "The OOXML package has no workbook part: neither the officeDocument "
                    + "relationship nor the conventional 'xl/workbook.xml' resolves.");
            }

            XElement? root;
            string? error;
            using (Stream content = main.Open()) root = OoxmlXml.TryLoad(content, out error);

            if (root is null)
            {
                throw new MalformedDocumentException(
                    $"The workbook part '{main.Name}' is not readable XML"
                    + (error is null ? "." : $": {error}"));
            }

            return new XlsxFile(package, main, root);
        }
        catch
        {
            package.Dispose();
            throw;
        }
    }

    /// <summary>
    /// Loads a sheet's <c>worksheet</c> root, or null when the part is missing or unreadable.
    /// </summary>
    public XElement? LoadSheet(XlsxSheetEntry sheet)
    {
        ArgumentNullException.ThrowIfNull(sheet);
        if (sheet.PartName is null) return null;

        IPackagePart? part = _package.GetPart(sheet.PartName);
        if (part is null)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2141",
                $"Sheet '{sheet.Name}' names the worksheet part '{sheet.PartName}', which the "
                + "package does not contain, so it has been extracted as empty.",
                new DiagnosticLocation(sheet.PartName)));
            return null;
        }

        using Stream content = part.Open();
        XElement? root = OoxmlXml.TryLoad(content, out string? error);
        if (root is null && error is not null)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2140",
                $"The worksheet part for sheet '{sheet.Name}' is malformed and has been "
                + $"skipped: {error.TrimEnd('.')}.",
                new DiagnosticLocation(sheet.PartName)));
        }
        return root;
    }

    /// <summary>
    /// Loads the legacy comments part a sheet references, or null when it has none.
    /// </summary>
    /// <remarks>
    /// Comments hang off the <em>worksheet</em> part, not the workbook, so their relationship
    /// has to be resolved against the sheet. Threaded comments (<c>threadedComments</c>) are a
    /// later addition that Excel writes <em>in addition</em> to the legacy part rather than
    /// instead of it, so reading the legacy one alone never loses a comment and never doubles
    /// one.
    /// </remarks>
    public XElement? LoadComments(XlsxSheetEntry sheet)
    {
        ArgumentNullException.ThrowIfNull(sheet);
        if (sheet.PartName is null) return null;

        foreach (OpcXml.Relationship relationship in
                 _package.GetRelationshipsByType(RelationshipBase + "comments", sheet.PartName))
        {
            if (relationship.IsExternal) continue;
            IPackagePart? part = _package.GetPart(relationship.Target);
            if (part is null) continue;

            using Stream content = part.Open();
            return OoxmlXml.TryLoad(content, out _);
        }
        return null;
    }

    /// <inheritdoc/>
    public void Dispose() => _package.Dispose();

    /// <summary>
    /// Reads <c>workbook/sheets</c>, resolving each sheet's part through its relationship id.
    /// </summary>
    /// <remarks>
    /// The <c>state</c> attribute has three values, and both of the non-visible ones matter:
    /// <c>hidden</c> is a sheet the user hid, <c>veryHidden</c> one only a macro can reveal.
    /// Both are extracted and flagged rather than skipped, which is what the ODS path does.
    /// </remarks>
    private List<XlsxSheetEntry> ReadSheetList(XElement workbook)
    {
        List<XlsxSheetEntry> sheets = [];
        int index = 0;

        foreach (XElement sheet in Xlsx.Children(Xlsx.Child(workbook, "sheets"), "sheet"))
        {
            string? relationshipId = Xlsx.RelationshipId(sheet);
            string? target = null;
            if (relationshipId is not null
                && _workbookRelationships.TryGetValue(relationshipId, out OpcXml.Relationship found)
                && !found.IsExternal)
            {
                // The relationship resolving is not enough: it may name a part the package does
                // not contain, and then the conventional name is still worth trying.
                target = _package.GetPart(found.Target)?.Name;
            }

            // The conventional name only as a fallback, and numbered by position rather than by
            // sheetId, because sheetId is an identity that survives deletion and stops matching
            // the file names long before a workbook is old.
            target ??= _package.GetPart($"xl/worksheets/sheet{index + 1}.xml")?.Name;

            string state = Xlsx.Attribute(sheet, "state") ?? "visible";
            sheets.Add(new XlsxSheetEntry(
                Xlsx.Attribute(sheet, "name") ?? $"Sheet{index + 1}",
                index,
                state is "hidden" or "veryHidden",
                target));

            if (target is null)
            {
                _diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2141",
                    $"Sheet '{Xlsx.Attribute(sheet, "name") ?? "(unnamed)"}' names no readable "
                    + "worksheet part, so it has been extracted as empty."));
            }
            index++;
        }

        return sheets;
    }

    private XElement? LoadRelated(string relationshipSuffix, string conventionalName)
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
        if (part is null) return null;

        using Stream content = part.Open();
        XElement? root = OoxmlXml.TryLoad(content, out string? error);
        if (root is null && error is not null)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Error, "PL2142",
                $"The '{relationshipSuffix}' part is malformed and has been skipped: "
                + $"{error.TrimEnd('.')}.",
                new DiagnosticLocation(part.Name)));
        }
        return root;
    }
}

/// <summary>One sheet as the workbook lists it.</summary>
/// <param name="Name">The sheet's name, as shown on its tab.</param>
/// <param name="Index">Its position in the workbook, zero-based.</param>
/// <param name="IsHidden">True for a <c>hidden</c> or <c>veryHidden</c> sheet.</param>
/// <param name="PartName">
/// The worksheet part it resolves to, or null when the relationship does not resolve.
/// </param>
public sealed record XlsxSheetEntry(string Name, int Index, bool IsHidden, string? PartName);
