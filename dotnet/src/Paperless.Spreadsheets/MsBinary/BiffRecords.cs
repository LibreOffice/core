namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// The BIFF record identifiers this reader acts on.
/// </summary>
/// <remarks>
/// <para>
/// Named and numbered as LibreOffice names them in <c>sc/source/filter/inc/xlconst.hxx</c>,
/// <c>xltable.hxx</c>, <c>xlcontent.hxx</c> and <c>xlstyle.hxx</c>, so a reader can be
/// checked against the C++ importer record by record.
/// </para>
/// <para>
/// The suffix on several names is the BIFF generation that introduced that encoding of the
/// same logical record: <c>Number2</c> (0x0003) is Excel 2.1's NUMBER and <c>Number</c>
/// (0x0203) is Excel 3.0's. They are genuinely different layouts under one name, which is
/// why they are separate constants rather than one alias.
/// </para>
/// </remarks>
internal static class BiffRecords
{
    // Structure.
    public const ushort Bof2 = 0x0009;
    public const ushort Bof3 = 0x0209;
    public const ushort Bof4 = 0x0409;
    public const ushort Bof = 0x0809;
    public const ushort Eof = 0x000A;
    public const ushort Continue = 0x003C;

    // Workbook globals.
    public const ushort CodePage = 0x0042;
    public const ushort DateMode = 0x0022;
    public const ushort BoundSheet = 0x0085;
    public const ushort Sst = 0x00FC;
    public const ushort FilePass = 0x002F;
    public const ushort Format2 = 0x001E;
    public const ushort Format = 0x041E;
    public const ushort Xf2 = 0x0043;
    public const ushort Xf3 = 0x0243;
    public const ushort Xf4 = 0x0443;
    public const ushort Xf = 0x00E0;

    // Sheet structure.
    public const ushort Dimensions2 = 0x0000;
    public const ushort Dimensions = 0x0200;
    public const ushort MergedCells = 0x00E5;
    public const ushort Window2 = 0x023E;

    // Cells.
    public const ushort Blank2 = 0x0001;
    public const ushort Blank = 0x0201;
    public const ushort Integer2 = 0x0002;
    public const ushort Number2 = 0x0003;
    public const ushort Number = 0x0203;
    public const ushort Label2 = 0x0004;
    public const ushort Label = 0x0204;
    public const ushort BoolErr2 = 0x0005;
    public const ushort BoolErr = 0x0205;
    public const ushort Formula2 = 0x0006;
    public const ushort Formula3 = 0x0206;
    public const ushort Formula4 = 0x0406;
    public const ushort String2 = 0x0007;
    public const ushort String = 0x0207;
    public const ushort MulRk = 0x00BD;
    public const ushort MulBlank = 0x00BE;
    public const ushort RString = 0x00D6;
    public const ushort LabelSst = 0x00FD;
    public const ushort Rk = 0x027E;

    /// <summary>The BOF substream types, from <c>EXC_BOF_*</c>.</summary>
    public const ushort SubstreamGlobals = 0x0005;
    public const ushort SubstreamVbModule = 0x0006;
    public const ushort SubstreamWorksheet = 0x0010;
    public const ushort SubstreamChart = 0x0020;
    public const ushort SubstreamMacroSheet = 0x0040;
    public const ushort SubstreamWorkspace = 0x0100;

    /// <summary>The BIFF version markers a BOF record carries, from <c>EXC_BOF_BIFF*</c>.</summary>
    public const ushort VersionBiff5 = 0x0500;

    /// <summary>See <c>EXC_BOF_BIFF8</c>.</summary>
    public const ushort VersionBiff8 = 0x0600;

    /// <summary>True for any of the four BOF record identifiers.</summary>
    public static bool IsBof(ushort id)
        => id is Bof or Bof2 or Bof3 or Bof4;
}

/// <summary>
/// The BIFF generation a workbook is written in.
/// </summary>
/// <remarks>
/// Only the two that matter are distinguished. BIFF8 is Excel 97 through 2003 and is what
/// <c>.xls</c> means in practice; BIFF5 is Excel 5.0/95 and differs in ways that reach every
/// record — byte strings rather than the compressed/uncompressed Unicode of BIFF8, and a
/// different XF layout. BIFF2 through BIFF4 are read as BIFF5 where their records happen to
/// agree and skipped where they do not; LibreOffice supports them fully, and a from-scratch
/// reader aimed at real files does not need to (<c>research/03-calc.md</c> section D.5).
/// </remarks>
internal enum BiffVersion
{
    /// <summary>Excel 5.0/7.0 (95), and the older generations read on the same path.</summary>
    Biff5,

    /// <summary>Excel 97 through 2003.</summary>
    Biff8,
}

/// <summary>The BIFF error codes, from <c>EXC_ERR_*</c> in <c>xlconst.hxx</c>.</summary>
internal static class BiffErrors
{
    public const byte Null = 0x00;
    public const byte DivideByZero = 0x07;
    public const byte Value = 0x0F;
    public const byte Reference = 0x17;
    public const byte Name = 0x1D;
    public const byte Number = 0x24;
    public const byte NotAvailable = 0x2A;

    /// <summary>The Paperless equivalent of a BIFF error code.</summary>
    /// <remarks>
    /// An unrecognised code becomes <see cref="Core.Extraction.CellError.Unknown"/> rather
    /// than being dropped: the cell is still in error, and saying so is more useful than
    /// reporting a number the file does not contain.
    /// </remarks>
    public static Core.Extraction.CellError ToCellError(byte code) => code switch
    {
        Null => Core.Extraction.CellError.Null,
        DivideByZero => Core.Extraction.CellError.DivideByZero,
        Value => Core.Extraction.CellError.Value,
        Reference => Core.Extraction.CellError.Reference,
        Name => Core.Extraction.CellError.Name,
        Number => Core.Extraction.CellError.Number,
        NotAvailable => Core.Extraction.CellError.NotAvailable,
        _ => Core.Extraction.CellError.Unknown,
    };

    /// <summary>The text Calc displays for an error, which is what a reference renderer shows.</summary>
    public static string Text(Core.Extraction.CellError error) => error switch
    {
        Core.Extraction.CellError.Null => "#NULL!",
        Core.Extraction.CellError.DivideByZero => "#DIV/0!",
        Core.Extraction.CellError.Value => "#VALUE!",
        Core.Extraction.CellError.Reference => "#REF!",
        Core.Extraction.CellError.Name => "#NAME?",
        Core.Extraction.CellError.Number => "#NUM!",
        Core.Extraction.CellError.NotAvailable => "#N/A",
        Core.Extraction.CellError.Circular => "Err:522",
        _ => "#ERR",
    };
}
