namespace XRay.Office.Spreadsheets.Html;

/// <summary>
/// How a reader gets from one sheet of the exported document to the next.
/// </summary>
/// <remarks>
/// Only a workbook of more than one written sheet has anything to navigate, so both members
/// produce the same document for a single-sheet one.
/// </remarks>
public enum SheetHtmlNavigation
{
    /// <summary>
    /// The reference's: an <c>Overview</c> index of links, and every sheet laid out under it in
    /// one long page.
    /// </summary>
    /// <remarks>
    /// What <c>ScHTMLExport::WriteOverview</c> (<c>sc/source/filter/html/htmlexp.cxx</c>:462-493)
    /// writes, and the default — a document that scrolls is what a converter is expected to
    /// produce, and it is the shape the fidelity tests compare against LibreOffice's own output.
    /// </remarks>
    Overview,

    /// <summary>
    /// A strip of tabs across the top, showing one sheet at a time — the workbook's own shape.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A departure from the reference, for a caller showing a workbook to a reader rather than
    /// archiving it: a fifty-sheet workbook as one scrolling page is not navigable, and the tabs
    /// are how the reader already thinks about it.
    /// </para>
    /// <para>
    /// <strong>It stays one self-contained file and runs no script.</strong> The switching is a
    /// radio group and two generated CSS rules, so the document works with scripting disabled,
    /// inside a sandboxed frame, and under a content-security policy that admits no inline script
    /// — and a reader can still tab and arrow through the sheets, because a radio group is what it
    /// actually is. Printing shows every sheet, each under its own heading, so the printed
    /// document is the <see cref="Overview"/> one.
    /// </para>
    /// </remarks>
    Tabs,
}
