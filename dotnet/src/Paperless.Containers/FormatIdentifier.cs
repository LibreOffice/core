using System.IO.Compression;
using System.Text;
using Paperless.Containers.Ole2;
using Paperless.Core.Formats;

namespace Paperless.Containers;

/// <summary>
/// Determines a document's real format from its content.
/// </summary>
/// <remarks>
/// <para>
/// Follows the same precedence LibreOffice's own detection uses
/// (<c>dotnet/research/01-formats-and-detection.md</c> section 4): recognise the container
/// first, then interrogate the container to name the format. The file extension is only ever
/// a tie-breaker for cases content genuinely cannot resolve.
/// </para>
/// <para>
/// Two decisions worth knowing, both copied from LibreOffice because real-world files
/// require them:
/// </para>
/// <list type="bullet">
///   <item><description>
///     Legacy binary formats are identified by <b>OLE2 stream name</b>, not by the root
///     CLSID. Files with a zero or wrong CLSID are common; files missing
///     <c>WordDocument</c>/<c>Workbook</c>/<c>PowerPoint Document</c> are not.
///   </description></item>
///   <item><description>
///     OOXML is identified by the <b>content type of the main document part</b>, reached by
///     following a relationship from the package root — never by assuming a path. DOCX
///     versus DOCM additionally consults the file name, which is the one place LibreOffice
///     itself falls back to the extension.
///   </description></item>
/// </list>
/// </remarks>
public sealed class FormatIdentifier : IFormatIdentifier
{
    /// <summary>A ready-to-use instance. The identifier is stateless.</summary>
    public static FormatIdentifier Instance { get; } = new();

    private const string OfficeDocumentRelationship =
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument";

    /// <summary>The OLE2 / Compound File Binary signature.</summary>
    private static ReadOnlySpan<byte> Ole2Signature =>
        [0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1];

    /// <summary>
    /// How much of a text or XML file to sniff. LibreOffice reads about 4000 characters for
    /// its flat-XML detection; 8 KiB of bytes comfortably covers that.
    /// </summary>
    private const int TextSniffBytes = 8192;

    /// <inheritdoc/>
    public FormatIdentification Identify(Stream stream, string? fileNameHint = null)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (!stream.CanSeek)
            throw new ArgumentException("Identification requires a seekable stream.", nameof(stream));

        long origin = stream.Position;
        try
        {
            stream.Position = 0;
            Span<byte> prefix = stackalloc byte[8];
            int read = ReadUpTo(stream, prefix);
            if (read == 0) return FormatIdentification.Unknown;

            // ZIP: 'PK'. Covers both ODF and OOXML, which are then told apart by content.
            if (read >= 2 && prefix[0] == 'P' && prefix[1] == 'K')
                return IdentifyZip(stream, fileNameHint);

            // OLE2/CFB: the legacy binary formats, and encrypted OOXML.
            if (read >= 8 && prefix.SequenceEqual(Ole2Signature))
                return IdentifyCompoundFile(stream, fileNameHint);

            return IdentifyPlainOrXml(stream, prefix[..read], fileNameHint);
        }
        finally { stream.Position = origin; }
    }

    // ------------------------------------------------------------------------ ZIP-based

    private static FormatIdentification IdentifyZip(Stream stream, string? fileNameHint)
    {
        stream.Position = 0;
        ZipArchive archive;
        try
        {
            archive = new ZipArchive(stream, ZipArchiveMode.Read, leaveOpen: true);
        }
        catch (InvalidDataException)
        {
            // 'PK' but not a readable archive: truncated, or encrypted at the ZIP level.
            return new FormatIdentification(DocumentFormat.Unknown, ContainerKind.Zip,
                IdentificationConfidence.Probable, IsEncrypted: false,
                "Starts with a ZIP signature but the central directory could not be read.");
        }

        using (archive)
        {
            // ODF: the 'mimetype' entry is the authoritative statement of what this is.
            ZipArchiveEntry? mimetype = archive.GetEntry("mimetype");
            if (mimetype is not null)
            {
                string declared = ReadEntryText(mimetype, 256).Trim();
                DocumentFormat odf = MatchOdfMediaType(declared);
                if (odf != DocumentFormat.Unknown)
                {
                    bool encrypted = archive.GetEntry("META-INF/manifest.xml") is { } manifest
                                     && ReadEntryText(manifest, 64 * 1024)
                                        .Contains("manifest:encryption-data", StringComparison.Ordinal);
                    return new FormatIdentification(odf, ContainerKind.Zip,
                        IdentificationConfidence.Certain, encrypted,
                        $"ODF package, mimetype '{declared}'.");
                }
                return new FormatIdentification(DocumentFormat.Unknown, ContainerKind.Zip,
                    IdentificationConfidence.Probable, IsEncrypted: false,
                    $"ODF package with an unrecognised mimetype '{declared}'.");
            }

            // OOXML: resolve the main part through the root relationships, then read its
            // content type. Assuming 'word/document.xml' would be wrong for many real files.
            if (archive.GetEntry("[Content_Types].xml") is { } contentTypesEntry)
            {
                string contentTypes = ReadEntryText(contentTypesEntry, 512 * 1024);
                string? mainPart = FindMainDocumentPart(archive);
                string? mainType = mainPart is null
                    ? null
                    : ResolveContentType(contentTypes, mainPart);

                // Fall back to whichever known main-part content type the package declares
                // at all. Some producers write a malformed .rels while still declaring the
                // right override, and that is recoverable.
                mainType ??= FindAnyKnownMainContentType(contentTypes);

                if (mainType is not null)
                {
                    DocumentFormat format = MatchOoxmlContentType(mainType, fileNameHint);
                    if (format != DocumentFormat.Unknown)
                    {
                        return new FormatIdentification(format, ContainerKind.Zip,
                            IdentificationConfidence.Certain, IsEncrypted: false,
                            $"OOXML package, main part content type '{mainType}'.");
                    }
                }

                return new FormatIdentification(DocumentFormat.Unknown, ContainerKind.Zip,
                    IdentificationConfidence.Probable, IsEncrypted: false,
                    "OPC package whose main part is not a supported Writer/Calc/Impress type.");
            }

            return new FormatIdentification(DocumentFormat.Unknown, ContainerKind.Zip,
                IdentificationConfidence.Probable, IsEncrypted: false,
                "ZIP archive with neither an ODF 'mimetype' entry nor '[Content_Types].xml'.");
        }
    }

    /// <summary>Follows the officeDocument relationship from <c>_rels/.rels</c>.</summary>
    private static string? FindMainDocumentPart(ZipArchive archive)
    {
        ZipArchiveEntry? rels = archive.GetEntry("_rels/.rels");
        if (rels is null) return null;

        string xml = ReadEntryText(rels, 256 * 1024);
        try
        {
            using System.Xml.XmlReader reader = System.Xml.XmlReader.Create(
                new StringReader(xml),
                new System.Xml.XmlReaderSettings
                {
                    // Never resolve external entities: this is untrusted input, and an
                    // external DTD reference would be an XXE vector.
                    DtdProcessing = System.Xml.DtdProcessing.Prohibit,
                    XmlResolver = null,
                    IgnoreWhitespace = true,
                    IgnoreComments = true,
                });
            while (reader.Read())
            {
                if (reader.NodeType != System.Xml.XmlNodeType.Element ||
                    !string.Equals(reader.LocalName, "Relationship", StringComparison.Ordinal))
                    continue;

                string? type = reader.GetAttribute("Type");
                string? target = reader.GetAttribute("Target");
                if (target is null || type is null) continue;
                if (!type.EndsWith("/officeDocument", StringComparison.OrdinalIgnoreCase) &&
                    !string.Equals(type, OfficeDocumentRelationship, StringComparison.OrdinalIgnoreCase))
                    continue;

                return target.TrimStart('/');
            }
        }
        catch (System.Xml.XmlException)
        {
            // A malformed .rels is recoverable; the caller falls back to the content types.
            return null;
        }
        return null;
    }

    /// <summary>Resolves a part's content type: overrides first, then defaults by extension.</summary>
    private static string? ResolveContentType(string contentTypesXml, string partName)
    {
        string normalised = "/" + partName.TrimStart('/');
        try
        {
            using System.Xml.XmlReader reader = System.Xml.XmlReader.Create(
                new StringReader(contentTypesXml),
                new System.Xml.XmlReaderSettings
                {
                    DtdProcessing = System.Xml.DtdProcessing.Prohibit,
                    XmlResolver = null,
                    IgnoreWhitespace = true,
                    IgnoreComments = true,
                });

            string? defaultForExtension = null;
            string extension = Path.GetExtension(partName).TrimStart('.');

            while (reader.Read())
            {
                if (reader.NodeType != System.Xml.XmlNodeType.Element) continue;

                if (string.Equals(reader.LocalName, "Override", StringComparison.Ordinal))
                {
                    // An override names the part exactly and wins outright.
                    if (string.Equals(reader.GetAttribute("PartName"), normalised,
                                      StringComparison.OrdinalIgnoreCase))
                        return reader.GetAttribute("ContentType");
                }
                else if (string.Equals(reader.LocalName, "Default", StringComparison.Ordinal)
                         && extension.Length > 0
                         && string.Equals(reader.GetAttribute("Extension"), extension,
                                          StringComparison.OrdinalIgnoreCase))
                {
                    defaultForExtension = reader.GetAttribute("ContentType");
                }
            }
            return defaultForExtension;
        }
        catch (System.Xml.XmlException) { return null; }
    }

    /// <summary>
    /// Last-resort scan for any main-part content type the package declares.
    /// </summary>
    /// <remarks>
    /// A substring search rather than a parse, deliberately: this path exists precisely for
    /// packages whose XML is malformed enough that parsing already failed.
    /// </remarks>
    private static string? FindAnyKnownMainContentType(string contentTypesXml)
    {
        foreach (string candidate in OoxmlMainPartContentTypes)
            if (contentTypesXml.Contains(candidate, StringComparison.OrdinalIgnoreCase))
                return candidate;
        return null;
    }

    // ----------------------------------------------------------------------- OLE2-based

    private static FormatIdentification IdentifyCompoundFile(Stream stream, string? fileNameHint)
    {
        stream.Position = 0;
        CompoundFile file;
        try
        {
            file = CompoundFile.Open(stream, leaveOpen: true);
        }
        catch (Core.MalformedDocumentException ex)
        {
            return new FormatIdentification(DocumentFormat.Unknown, ContainerKind.Ole2CompoundFile,
                IdentificationConfidence.Probable, IsEncrypted: false,
                $"Compound file header present but the structure is unreadable: {ex.Message}");
        }

        using (file)
        {
            HashSet<string> names = new(file.RootStreamNames, StringComparer.OrdinalIgnoreCase);

            // Encrypted OOXML is an OLE2 wrapper around the real package. Check this before
            // the legacy formats: such a file has none of their streams.
            if (names.Contains("EncryptedPackage"))
            {
                return new FormatIdentification(DocumentFormat.Unknown, ContainerKind.Ole2CompoundFile,
                    IdentificationConfidence.Probable, IsEncrypted: true,
                    "Encrypted OOXML package (MS-OFFCRYPTO); a password is needed to determine the format.");
            }

            if (names.Contains("WordDocument"))
            {
                // WW8 additionally has a table stream. Its absence means an older Word
                // format, which we still classify as Doc.
                bool ww8 = names.Contains("1Table") || names.Contains("0Table");
                bool isTemplate = LooksLikeWordTemplate(file) ?? IsExtension(fileNameHint, "dot");
                return new FormatIdentification(
                    isTemplate ? DocumentFormat.Dot : DocumentFormat.Doc,
                    ContainerKind.Ole2CompoundFile, IdentificationConfidence.Certain,
                    IsEncrypted: false,
                    ww8 ? "OLE2 with 'WordDocument' and a table stream (Word 97+)."
                        : "OLE2 with 'WordDocument' but no table stream (Word 6/95).");
            }

            if (names.Contains("Workbook"))
            {
                return new FormatIdentification(
                    IsExtension(fileNameHint, "xlt") ? DocumentFormat.Xlt : DocumentFormat.Xls,
                    ContainerKind.Ole2CompoundFile, IdentificationConfidence.Certain,
                    IsEncrypted: false, "OLE2 with a 'Workbook' stream (Excel 97+, BIFF8).");
            }

            if (names.Contains("Book"))
            {
                return new FormatIdentification(DocumentFormat.Xls5, ContainerKind.Ole2CompoundFile,
                    IdentificationConfidence.Certain, IsEncrypted: false,
                    "OLE2 with a 'Book' stream (Excel 5.0/95, BIFF5).");
            }

            if (names.Contains("PowerPoint Document"))
            {
                DocumentFormat format =
                    IsExtension(fileNameHint, "pot") ? DocumentFormat.Pot :
                    IsExtension(fileNameHint, "pps") ? DocumentFormat.Pps :
                    DocumentFormat.Ppt;
                return new FormatIdentification(format, ContainerKind.Ole2CompoundFile,
                    IdentificationConfidence.Certain, IsEncrypted: false,
                    "OLE2 with a 'PowerPoint Document' stream.");
            }

            return new FormatIdentification(DocumentFormat.Unknown, ContainerKind.Ole2CompoundFile,
                IdentificationConfidence.Probable, IsEncrypted: false,
                "OLE2 compound file with no recognised Writer/Calc/Impress stream. "
                + $"Root streams: {string.Join(", ", file.RootStreamNames.Take(10))}");
        }
    }

    /// <summary>
    /// Reads the <c>fDot</c> flag from the WW8 FIB, which marks a template.
    /// </summary>
    /// <remarks>
    /// Bit 0 of the 16-bit value at offset 10 of the <c>WordDocument</c> stream. Returns null
    /// when the stream is too short to tell, so the caller can fall back to the extension.
    /// </remarks>
    private static bool? LooksLikeWordTemplate(CompoundFile file)
    {
        IPackagePart? part = file.GetPart("WordDocument");
        if (part is null) return null;

        using Stream s = part.Open();
        Span<byte> fib = stackalloc byte[12];
        if (ReadUpTo(s, fib) < 12) return null;
        ushort flags = (ushort)(fib[10] | (fib[11] << 8));
        return (flags & 0x0001) != 0;
    }

    // ------------------------------------------------------------------ text-based / XML

    private static FormatIdentification IdentifyPlainOrXml(
        Stream stream, ReadOnlySpan<byte> prefix, string? fileNameHint)
    {
        // RTF is unambiguous and cheap.
        if (prefix.Length >= 5 && Encoding.ASCII.GetString(prefix[..5]) == @"{\rtf")
        {
            return new FormatIdentification(DocumentFormat.Rtf, ContainerKind.PlainText,
                IdentificationConfidence.Certain, IsEncrypted: false, @"Starts with '{\rtf'.");
        }

        // The pre-OLE2 single-stream Word formats, by leading byte triple.
        if (prefix.Length >= 3)
        {
            byte a = prefix[0], b = prefix[1], c = prefix[2];
            if ((a == 0x9B && b == 0xA5 && c == 0x21) ||    // WinWord 1
                (a == 0x9C && b == 0xA5 && c == 0x21) ||    // PmWord 1
                (a == 0xDB && b == 0xA5 && c == 0x2D) ||    // WinWord 2
                (a == 0xDC && b == 0xA5 && c == 0x65))      // WinWord 6.0/95
            {
                return new FormatIdentification(DocumentFormat.Doc, ContainerKind.PlainText,
                    IdentificationConfidence.Certain, IsEncrypted: false,
                    "Pre-OLE2 single-stream Word signature.");
            }
        }

        stream.Position = 0;
        byte[] buffer = new byte[TextSniffBytes];
        int read = ReadUpTo(stream, buffer);
        string text = DecodeForSniffing(buffer.AsSpan(0, read));

        // Flat ODF declares itself in an office:mimetype attribute on the root element.
        // LibreOffice sniffs this as a raw substring rather than parsing, and so do we:
        // the file may be large and we only need the first few thousand characters.
        foreach ((string marker, DocumentFormat format) in FlatOdfMarkers)
        {
            if (text.Contains(marker, StringComparison.Ordinal))
            {
                return new FormatIdentification(format, ContainerKind.Xml,
                    IdentificationConfidence.Certain, IsEncrypted: false,
                    $"Flat ODF: found office:mimetype '{marker}'.");
            }
        }

        if (text.Contains("<?mso-application progid=\"Word.Document\"", StringComparison.Ordinal))
        {
            return new FormatIdentification(DocumentFormat.WordXml2003, ContainerKind.Xml,
                IdentificationConfidence.Certain, IsEncrypted: false,
                "Word 2003 XML processing instruction.");
        }
        if (text.Contains("<?mso-application progid=\"Excel.Sheet\"", StringComparison.Ordinal))
        {
            return new FormatIdentification(DocumentFormat.ExcelXml2003, ContainerKind.Xml,
                IdentificationConfidence.Certain, IsEncrypted: false,
                "Excel 2003 XML processing instruction.");
        }

        // Nothing in the content settles it. CSV genuinely cannot be identified by content
        // — any text file is arguably a one-column CSV — so this is the one format where the
        // extension is the deciding signal, and the confidence says so.
        if (IsExtension(fileNameHint, "csv") || IsExtension(fileNameHint, "tsv")
            || IsExtension(fileNameHint, "tab"))
        {
            return new FormatIdentification(DocumentFormat.Csv, ContainerKind.PlainText,
                IdentificationConfidence.ExtensionOnly, IsEncrypted: false,
                "No content signature; treated as CSV on the strength of the file extension.");
        }

        return new FormatIdentification(DocumentFormat.Unknown,
            text.TrimStart().StartsWith('<') ? ContainerKind.Xml : ContainerKind.PlainText,
            IdentificationConfidence.None, IsEncrypted: false,
            "No recognised container or content signature.");
    }

    /// <summary>
    /// Decodes a prefix for substring sniffing, honouring a BOM.
    /// </summary>
    /// <remarks>
    /// UTF-16 matters here: a UTF-16 flat-ODF file decoded as UTF-8 becomes interleaved NULs
    /// and every marker search fails.
    /// </remarks>
    private static string DecodeForSniffing(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
            return Encoding.UTF8.GetString(bytes[3..]);
        if (bytes.Length >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE)
            return Encoding.Unicode.GetString(bytes[2..]);
        if (bytes.Length >= 2 && bytes[0] == 0xFE && bytes[1] == 0xFF)
            return Encoding.BigEndianUnicode.GetString(bytes[2..]);

        // No BOM. A UTF-16LE XML declaration shows up as '<' NUL, which is a reliable tell.
        if (bytes.Length >= 4 && bytes[0] == '<' && bytes[1] == 0x00)
            return Encoding.Unicode.GetString(bytes);

        return Encoding.UTF8.GetString(bytes);
    }

    // -------------------------------------------------------------------------- matching

    private static DocumentFormat MatchOdfMediaType(string mediaType) => mediaType switch
    {
        "application/vnd.oasis.opendocument.text" => DocumentFormat.Odt,
        "application/vnd.oasis.opendocument.text-template" => DocumentFormat.Ott,
        "application/vnd.oasis.opendocument.spreadsheet" => DocumentFormat.Ods,
        "application/vnd.oasis.opendocument.spreadsheet-template" => DocumentFormat.Ots,
        "application/vnd.oasis.opendocument.presentation" => DocumentFormat.Odp,
        "application/vnd.oasis.opendocument.presentation-template" => DocumentFormat.Otp,
        "application/vnd.sun.xml.writer" => DocumentFormat.Sxw,
        "application/vnd.sun.xml.writer.template" => DocumentFormat.Stw,
        "application/vnd.sun.xml.calc" => DocumentFormat.Sxc,
        "application/vnd.sun.xml.calc.template" => DocumentFormat.Stc,
        "application/vnd.sun.xml.impress" => DocumentFormat.Sxi,
        "application/vnd.sun.xml.impress.template" => DocumentFormat.Sti,
        _ => DocumentFormat.Unknown,
    };

    /// <summary>
    /// Maps a main-part content type to a format.
    /// </summary>
    /// <remarks>
    /// Transcribed from <c>oox/source/core/filterdetect.cxx</c> — see
    /// <c>dotnet/research/01-formats-and-detection.md</c> section 6.2 for the source table.
    /// Note that DOCX and DOCM share a content type in some producers' output, which is why
    /// the file name is consulted for that one case; LibreOffice does the same.
    /// </remarks>
    private static DocumentFormat MatchOoxmlContentType(string contentType, string? fileNameHint)
    {
        string type = contentType.Trim();

        if (Is(type, "wordprocessingml.document.main+xml"))
            return IsExtension(fileNameHint, "docm") ? DocumentFormat.Docm : DocumentFormat.Docx;
        if (Is(type, "ms-word.document.macroEnabled.main+xml")) return DocumentFormat.Docm;
        if (Is(type, "wordprocessingml.template.main+xml"))
            return IsExtension(fileNameHint, "dotm") ? DocumentFormat.Dotm : DocumentFormat.Dotx;
        if (Is(type, "ms-word.template.macroEnabledTemplate.main+xml")) return DocumentFormat.Dotm;

        if (Is(type, "spreadsheetml.sheet.main+xml")) return DocumentFormat.Xlsx;
        if (Is(type, "ms-excel.sheet.macroEnabled.main+xml")) return DocumentFormat.Xlsm;
        if (Is(type, "spreadsheetml.template.main+xml")) return DocumentFormat.Xltx;
        if (Is(type, "ms-excel.template.macroEnabled.main+xml")) return DocumentFormat.Xltm;
        if (Is(type, "ms-excel.sheet.binary.macroEnabled.main")) return DocumentFormat.Xlsb;

        if (Is(type, "presentationml.presentation.main+xml")) return DocumentFormat.Pptx;
        if (Is(type, "ms-powerpoint.presentation.macroEnabled.main+xml")) return DocumentFormat.Pptm;
        if (Is(type, "presentationml.template.main+xml")) return DocumentFormat.Potx;
        if (Is(type, "ms-powerpoint.template.macroEnabled.main+xml")) return DocumentFormat.Potm;
        if (Is(type, "presentationml.slideshow.main+xml")) return DocumentFormat.Ppsx;
        if (Is(type, "ms-powerpoint.slideshow.macroEnabled.main+xml")) return DocumentFormat.Ppsm;

        return DocumentFormat.Unknown;

        static bool Is(string actual, string suffix)
            => actual.EndsWith(suffix, StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>Content types recognised as an OOXML main document part.</summary>
    private static readonly string[] OoxmlMainPartContentTypes =
    [
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml",
        "application/vnd.ms-word.document.macroEnabled.main+xml",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml",
        "application/vnd.ms-word.template.macroEnabledTemplate.main+xml",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml",
        "application/vnd.ms-excel.sheet.macroEnabled.main+xml",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml",
        "application/vnd.ms-excel.template.macroEnabled.main+xml",
        "application/vnd.ms-excel.sheet.binary.macroEnabled.main",
        "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml",
        "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml",
        "application/vnd.openxmlformats-officedocument.presentationml.template.main+xml",
        "application/vnd.ms-powerpoint.template.macroEnabled.main+xml",
        "application/vnd.openxmlformats-officedocument.presentationml.slideshow.main+xml",
        "application/vnd.ms-powerpoint.slideshow.macroEnabled.main+xml",
    ];

    /// <summary>The office:mimetype values that identify a flat-ODF document.</summary>
    private static readonly (string Marker, DocumentFormat Format)[] FlatOdfMarkers =
    [
        ("office:mimetype=\"application/vnd.oasis.opendocument.text\"", DocumentFormat.Fodt),
        ("office:mimetype=\"application/vnd.oasis.opendocument.spreadsheet\"", DocumentFormat.Fods),
        ("office:mimetype=\"application/vnd.oasis.opendocument.presentation\"", DocumentFormat.Fodp),
    ];

    // --------------------------------------------------------------------------- helpers

    private static bool IsExtension(string? fileName, string extension)
        => fileName is not null
           && Path.GetExtension(fileName).TrimStart('.').Equals(extension, StringComparison.OrdinalIgnoreCase);

    /// <summary>Reads up to the buffer's length, returning how many bytes arrived.</summary>
    private static int ReadUpTo(Stream stream, Span<byte> destination)
    {
        int total = 0;
        while (total < destination.Length)
        {
            int read = stream.Read(destination[total..]);
            if (read == 0) break;
            total += read;
        }
        return total;
    }

    private static string ReadEntryText(ZipArchiveEntry entry, int maxBytes)
    {
        try
        {
            using Stream s = entry.Open();
            byte[] buffer = new byte[maxBytes];
            int read = ReadUpTo(s, buffer);
            return Encoding.UTF8.GetString(buffer, 0, read);
        }
        catch (InvalidDataException)
        {
            // A single unreadable entry must not abort identification of the whole package.
            return string.Empty;
        }
    }
}
