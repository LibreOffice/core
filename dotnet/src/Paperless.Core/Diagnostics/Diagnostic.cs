namespace Paperless.Core.Diagnostics;

/// <summary>How serious a <see cref="Diagnostic"/> is.</summary>
public enum DiagnosticSeverity
{
    /// <summary>Worth knowing but harmless — an unusual but legal construct.</summary>
    Information = 0,

    /// <summary>
    /// The file violates its specification, or uses something Paperless does not
    /// implement yet. Output is usable but may differ from a reference renderer.
    /// </summary>
    Warning,

    /// <summary>
    /// Part of the document could not be read and has been skipped. The rest of
    /// the document is still valid.
    /// </summary>
    Error,
}

/// <summary>
/// A non-fatal problem found while reading or laying out a document.
/// </summary>
/// <remarks>
/// Real-world office files break their own specifications constantly, so throwing
/// on every irregularity would make Paperless useless. Instead readers repair what
/// they can, skip what they cannot, and record what happened here. Callers that
/// need strictness can inspect this list and decide for themselves.
/// </remarks>
/// <param name="Severity">How serious the problem is.</param>
/// <param name="Code">
/// A stable identifier such as <c>PL1001</c>, suitable for filtering and for
/// suppressing known-benign diagnostics in tests.
/// </param>
/// <param name="Message">A human-readable description.</param>
/// <param name="Location">Where in the document it happened, if known.</param>
public readonly record struct Diagnostic(
    DiagnosticSeverity Severity,
    string Code,
    string Message,
    DiagnosticLocation? Location = null)
{
    /// <inheritdoc/>
    public override string ToString()
        => Location is { } loc
            ? $"{Severity} {Code}: {Message} ({loc})"
            : $"{Severity} {Code}: {Message}";
}

/// <summary>
/// Where inside a document a <see cref="Diagnostic"/> arose.
/// </summary>
/// <param name="PartName">
/// The container part or stream, e.g. <c>word/document.xml</c> or <c>WordDocument</c>.
/// </param>
/// <param name="ByteOffset">Offset within that part, for binary formats.</param>
/// <param name="Line">Line number, for XML formats.</param>
/// <param name="Column">Column number, for XML formats.</param>
/// <param name="Context">
/// A short description of the enclosing object — a sheet name and cell reference,
/// a slide number, a paragraph index.
/// </param>
public readonly record struct DiagnosticLocation(
    string? PartName = null,
    long? ByteOffset = null,
    int? Line = null,
    int? Column = null,
    string? Context = null)
{
    /// <inheritdoc/>
    public override string ToString()
    {
        List<string> parts = [];
        if (PartName is not null) parts.Add(PartName);
        if (Line is { } line) parts.Add(Column is { } col ? $"{line}:{col}" : $"line {line}");
        if (ByteOffset is { } offset) parts.Add($"+0x{offset:X}");
        if (Context is not null) parts.Add(Context);
        return parts.Count == 0 ? "<unknown>" : string.Join(", ", parts);
    }
}
