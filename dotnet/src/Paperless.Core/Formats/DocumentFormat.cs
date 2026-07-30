namespace Paperless.Core.Formats;

/// <summary>
/// Which application a document belongs to, mirroring LibreOffice's split into
/// Writer / Calc / Impress. Draw, Math and Base are deliberately out of scope
/// for Paperless.
/// </summary>
public enum DocumentFamily
{
    /// <summary>Not a recognised document.</summary>
    Unknown = 0,

    /// <summary>A word-processing document (the Writer equivalent).</summary>
    WordProcessing,

    /// <summary>A spreadsheet (the Calc equivalent).</summary>
    Spreadsheet,

    /// <summary>A presentation (the Impress equivalent).</summary>
    Presentation,
}

/// <summary>
/// The physical container a format is packaged in. This is what
/// <see cref="IFormatIdentifier"/> determines first, before narrowing to a
/// specific <see cref="DocumentFormat"/>.
/// </summary>
public enum ContainerKind
{
    /// <summary>Unrecognised or not yet determined.</summary>
    Unknown = 0,

    /// <summary>OLE2 / Compound File Binary — the legacy DOC/XLS/PPT container.</summary>
    Ole2CompoundFile,

    /// <summary>A ZIP archive: either an ODF package or an OOXML OPC package.</summary>
    Zip,

    /// <summary>A bare XML document — flat ODF (.fodt/.fods/.fodp) or Word 2003 XML.</summary>
    Xml,

    /// <summary>Plain text, including RTF and CSV.</summary>
    PlainText,
}

/// <summary>
/// Every file format Paperless recognises.
/// </summary>
/// <remarks>
/// <para>
/// The member names deliberately match the conventional file extension so that
/// the enum reads the same way the format catalogue in
/// <c>dotnet/research/01-formats-and-detection.md</c> does.
/// </para>
/// <para>
/// Macro-enabled variants (<c>.docm</c>, <c>.xlsm</c>, <c>.pptm</c>) are distinct
/// members even though their payload parses identically to the non-macro variant,
/// because callers routinely need to report or filter on "this file can carry
/// macros". Paperless never executes macros.
/// </para>
/// </remarks>
public enum DocumentFormat
{
    /// <summary>Unrecognised format.</summary>
    Unknown = 0,

    // ---- Word processing: OOXML ------------------------------------------------
    /// <summary>Office Open XML text document.</summary>
    Docx,
    /// <summary>Office Open XML macro-enabled text document.</summary>
    Docm,
    /// <summary>Office Open XML text template.</summary>
    Dotx,
    /// <summary>Office Open XML macro-enabled text template.</summary>
    Dotm,

    // ---- Word processing: legacy binary ---------------------------------------
    /// <summary>Microsoft Word 97-2003 binary document (WW8).</summary>
    Doc,
    /// <summary>Microsoft Word 97-2003 binary template.</summary>
    Dot,

    // ---- Word processing: other -----------------------------------------------
    /// <summary>Rich Text Format.</summary>
    Rtf,
    /// <summary>Microsoft Word 2003 WordprocessingML (flat XML).</summary>
    WordXml2003,

    // ---- Word processing: ODF -------------------------------------------------
    /// <summary>OpenDocument text document.</summary>
    Odt,
    /// <summary>OpenDocument text template.</summary>
    Ott,
    /// <summary>OpenDocument text, flat XML.</summary>
    Fodt,
    /// <summary>Legacy OpenOffice.org 1.x Writer document.</summary>
    Sxw,
    /// <summary>Legacy OpenOffice.org 1.x Writer template.</summary>
    Stw,

    // ---- Spreadsheet: OOXML ---------------------------------------------------
    /// <summary>Office Open XML spreadsheet.</summary>
    Xlsx,
    /// <summary>Office Open XML macro-enabled spreadsheet.</summary>
    Xlsm,
    /// <summary>Office Open XML spreadsheet template.</summary>
    Xltx,
    /// <summary>Office Open XML macro-enabled spreadsheet template.</summary>
    Xltm,
    /// <summary>Office Open XML binary spreadsheet (BIFF12 records in an OPC package).</summary>
    Xlsb,

    // ---- Spreadsheet: legacy binary -------------------------------------------
    /// <summary>Microsoft Excel 97-2003 binary workbook (BIFF8).</summary>
    Xls,
    /// <summary>Microsoft Excel 97-2003 binary template.</summary>
    Xlt,
    /// <summary>Microsoft Excel 5.0/95 binary workbook (BIFF5).</summary>
    Xls5,

    // ---- Spreadsheet: other ---------------------------------------------------
    /// <summary>Microsoft Excel 2003 SpreadsheetML (flat XML).</summary>
    ExcelXml2003,
    /// <summary>Comma- or otherwise-separated values.</summary>
    Csv,
    /// <summary>Data Interchange Format.</summary>
    Dif,

    // ---- Spreadsheet: ODF -----------------------------------------------------
    /// <summary>OpenDocument spreadsheet.</summary>
    Ods,
    /// <summary>OpenDocument spreadsheet template.</summary>
    Ots,
    /// <summary>OpenDocument spreadsheet, flat XML.</summary>
    Fods,
    /// <summary>Legacy OpenOffice.org 1.x Calc document.</summary>
    Sxc,
    /// <summary>Legacy OpenOffice.org 1.x Calc template.</summary>
    Stc,

    // ---- Presentation: OOXML --------------------------------------------------
    /// <summary>Office Open XML presentation.</summary>
    Pptx,
    /// <summary>Office Open XML macro-enabled presentation.</summary>
    Pptm,
    /// <summary>Office Open XML presentation template.</summary>
    Potx,
    /// <summary>Office Open XML macro-enabled presentation template.</summary>
    Potm,
    /// <summary>Office Open XML slide show.</summary>
    Ppsx,
    /// <summary>Office Open XML macro-enabled slide show.</summary>
    Ppsm,

    // ---- Presentation: legacy binary ------------------------------------------
    /// <summary>Microsoft PowerPoint 97-2003 binary presentation.</summary>
    Ppt,
    /// <summary>Microsoft PowerPoint 97-2003 binary template.</summary>
    Pot,
    /// <summary>Microsoft PowerPoint 97-2003 binary slide show.</summary>
    Pps,

    // ---- Presentation: ODF ----------------------------------------------------
    /// <summary>OpenDocument presentation.</summary>
    Odp,
    /// <summary>OpenDocument presentation template.</summary>
    Otp,
    /// <summary>OpenDocument presentation, flat XML.</summary>
    Fodp,
    /// <summary>Legacy OpenOffice.org 1.x Impress document.</summary>
    Sxi,
    /// <summary>Legacy OpenOffice.org 1.x Impress template.</summary>
    Sti,
}
