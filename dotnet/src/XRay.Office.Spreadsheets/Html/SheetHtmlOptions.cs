namespace XRay.Office.Spreadsheets.Html;

/// <summary>
/// What to put in, and leave out of, a sheet's HTML.
/// </summary>
/// <remarks>
/// The two switches mirror the filter options Calc's own HTML export takes —
/// <c>SkipImages</c> and <c>SkipHeaderFooter</c>, read in <c>ScHTMLExport</c>'s constructor
/// (<c>sc/source/filter/html/htmlexp.cxx</c>:225-235) — because they are the two a caller
/// embedding the output in a page of its own actually needs.
/// </remarks>
public sealed record SheetHtmlOptions
{
    /// <summary>The default: a whole document, images left out.</summary>
    public static SheetHtmlOptions Default { get; } = new();

    /// <summary>
    /// Writes the tables alone, with no <c>&lt;html&gt;</c>, <c>&lt;head&gt;</c> or
    /// <c>&lt;body&gt;</c> around them.
    /// </summary>
    /// <remarks>
    /// For a caller putting the sheets inside a page of its own. The head's style block goes with
    /// the head, so a fragment carries no default font — the embedding page states one.
    /// </remarks>
    public bool SkipHeaderFooter { get; init; }

    /// <summary>The document's title, or null to leave the <c>&lt;title&gt;</c> empty.</summary>
    /// <remarks>
    /// Empty is what LibreOffice writes for a document whose properties state no title, and the
    /// element is written either way because a title-less HTML document is invalid.
    /// </remarks>
    public string? Title { get; init; }

    /// <summary>What to name in the generator meta element.</summary>
    public string Generator { get; init; } = "X-Ray.Office";

    /// <summary>How a reader moves between the sheets. Defaults to the reference's index.</summary>
    public SheetHtmlNavigation Navigation { get; init; } = SheetHtmlNavigation.Overview;

    /// <summary>
    /// What to prefix the identifiers <see cref="SheetHtmlNavigation.Tabs"/> generates with.
    /// </summary>
    /// <remarks>
    /// The tab strip is a radio group, so it needs an element identifier per sheet and a name for
    /// the group itself; two workbooks exported into one page with the same prefix would share
    /// that group and switch each other's sheets. Give each its own prefix and they coexist. The
    /// value is folded to letters, digits, <c>-</c> and <c>_</c> and made to start with a letter,
    /// so a caller may pass a file name straight in. Ignored under
    /// <see cref="SheetHtmlNavigation.Overview"/>.
    /// </remarks>
    public string IdPrefix { get; init; } = "sheets";

    /// <summary>
    /// The language identifier written into <c>sdnum</c>, which is the reader's locale rather
    /// than the document's.
    /// </summary>
    /// <remarks>
    /// <c>HTMLOutFuncs::CreateTableDataOptionsValNum</c>
    /// (<c>svtools/source/svhtml/htmlout.cxx</c>:929-935) writes
    /// <c>Application::GetSettings().GetLanguageTag().getLanguageType()</c> — the UI language of
    /// the LibreOffice that exported the file, not anything the document says. 1033 is en-US,
    /// which is what an English LibreOffice writes and what the reference output here carries.
    /// It matters only to a reader importing the HTML back, which parses the number in that
    /// locale.
    /// </remarks>
    public int LanguageId { get; init; } = 1033;
}
