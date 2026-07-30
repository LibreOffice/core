namespace Paperless.Core.Documents;

/// <summary>
/// Document-level metadata, normalised across the three metadata systems the
/// supported formats use.
/// </summary>
/// <remarks>
/// The same logical field lives in a different place in each format family:
/// <list type="bullet">
///   <item><description>ODF: <c>meta.xml</c> (Dublin Core plus <c>meta:*</c>)</description></item>
///   <item><description>OOXML: <c>docProps/core.xml</c> plus <c>docProps/app.xml</c> and <c>docProps/custom.xml</c></description></item>
///   <item><description>Legacy binary: the OLE2 <c>\005SummaryInformation</c> and
///   <c>\005DocumentSummaryInformation</c> property sets</description></item>
/// </list>
/// Fields absent from the source are left null rather than defaulted, so callers
/// can tell "not recorded" from "recorded as empty".
/// </remarks>
public sealed record DocumentMetadata
{
    /// <summary>An instance with nothing recorded.</summary>
    public static readonly DocumentMetadata Empty = new();

    /// <summary>The document title.</summary>
    public string? Title { get; init; }

    /// <summary>The subject.</summary>
    public string? Subject { get; init; }

    /// <summary>The original author.</summary>
    public string? Author { get; init; }

    /// <summary>Whoever saved the document most recently.</summary>
    public string? LastModifiedBy { get; init; }

    /// <summary>A free-text description or comment.</summary>
    public string? Description { get; init; }

    /// <summary>Keywords, already split on the format's separator.</summary>
    public IReadOnlyList<string> Keywords { get; init; } = [];

    /// <summary>The document category.</summary>
    public string? Category { get; init; }

    /// <summary>When the document was created.</summary>
    public DateTimeOffset? Created { get; init; }

    /// <summary>When the document was last saved.</summary>
    public DateTimeOffset? Modified { get; init; }

    /// <summary>When the document was last printed.</summary>
    public DateTimeOffset? Printed { get; init; }

    /// <summary>The application that wrote the file, as recorded in the file.</summary>
    public string? GeneratorApplication { get; init; }

    /// <summary>The document's primary language as a BCP 47 tag, if recorded.</summary>
    public string? Language { get; init; }

    /// <summary>The save/revision counter, if recorded.</summary>
    public int? RevisionNumber { get; init; }

    /// <summary>Cumulative editing time, if recorded.</summary>
    public TimeSpan? TotalEditingTime { get; init; }

    /// <summary>
    /// Statistics as recorded in the file — page, word and character counts.
    /// These are the counts the authoring application last computed and can be
    /// stale or absent; they are not recomputed by Paperless.
    /// </summary>
    public DocumentStatistics? Statistics { get; init; }

    /// <summary>
    /// Application-defined custom properties, preserved with their original names.
    /// </summary>
    public IReadOnlyDictionary<string, object?> CustomProperties { get; init; }
        = new Dictionary<string, object?>();
}

/// <summary>Counts recorded in a document's metadata.</summary>
public sealed record DocumentStatistics
{
    /// <summary>Page count as recorded.</summary>
    public int? PageCount { get; init; }

    /// <summary>Word count as recorded.</summary>
    public int? WordCount { get; init; }

    /// <summary>Character count as recorded.</summary>
    public int? CharacterCount { get; init; }

    /// <summary>Paragraph count as recorded.</summary>
    public int? ParagraphCount { get; init; }

    /// <summary>Table count as recorded.</summary>
    public int? TableCount { get; init; }

    /// <summary>Image count as recorded.</summary>
    public int? ImageCount { get; init; }

    /// <summary>Sheet count as recorded. Spreadsheets only.</summary>
    public int? SheetCount { get; init; }

    /// <summary>Slide count as recorded. Presentations only.</summary>
    public int? SlideCount { get; init; }
}
