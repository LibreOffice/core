namespace Paperless.Markup;

/// <summary>
/// Settings shared by the XHTML and Markdown writers.
/// </summary>
/// <remarks>
/// Both writers take the same options because the Markdown writer consumes the XHTML
/// writer's output: anything decided while walking the content tree has to be decided once,
/// in the stage that does the walking, or the two outputs would disagree about what the
/// document contains.
/// </remarks>
public sealed record MarkupOptions
{
    /// <summary>The settings used when a caller passes none.</summary>
    public static MarkupOptions Default { get; } = new();

    /// <summary>
    /// The greatest number of rows emitted for any one table.
    /// </summary>
    /// <remarks>
    /// A spreadsheet's used range is routinely tens of thousands of rows, and a table that
    /// long is not a useful projection of it — it is a denial-of-service on whatever reads the
    /// output. The bound is applied to every table rather than only to sheets, because it is a
    /// property of the output rather than of the family; a word-processing table never
    /// approaches it. Truncation is always announced in the output (see
    /// <c>class="truncation"</c> in XHTML), never silent.
    /// </remarks>
    public int MaxTableRows { get; init; } = 1000;

    /// <summary>
    /// The greatest number of columns emitted for any one table.
    /// </summary>
    /// <remarks>Same reasoning as <see cref="MaxTableRows"/>; a sheet can be 16 384 wide.</remarks>
    public int MaxTableColumns { get; init; } = 64;

    /// <summary>Whether comment and annotation sections are emitted.</summary>
    public bool IncludeComments { get; init; } = true;

    /// <summary>Whether page header and footer sections are emitted.</summary>
    public bool IncludeHeadersAndFooters { get; init; } = true;

    /// <summary>
    /// Whether hidden sheets and skipped slides are emitted.
    /// </summary>
    /// <remarks>
    /// Defaults to true, matching extraction: a caller indexing content usually wants hidden
    /// content, and a caller reproducing what a reader sees usually does not.
    /// </remarks>
    public bool IncludeHiddenSections { get; init; } = true;

    /// <summary>
    /// Whether the document's metadata is emitted into the XHTML <c>&lt;head&gt;</c>.
    /// </summary>
    public bool IncludeMetadata { get; init; } = true;
}
