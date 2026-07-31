namespace Paperless.OpenDocument.Styles;

/// <summary>
/// The ODF style families. A style's family determines which properties it may carry
/// and what it may be applied to.
/// </summary>
/// <remarks>
/// The family is part of a style's identity, not a description of it: two styles in the
/// same document may share a name if their families differ, and a parent reference only
/// ever resolves within one family. LibreOffice keeps a separate format pool per family
/// for exactly this reason (<c>include/xmloff/families.hxx</c>).
/// </remarks>
public enum OdfStyleFamily
{
    /// <summary>Not a family Paperless recognises.</summary>
    Unknown = 0,

    /// <summary>Paragraph styles.</summary>
    Paragraph,

    /// <summary>Character styles.</summary>
    Text,

    /// <summary>Section styles.</summary>
    Section,

    /// <summary>Table styles.</summary>
    Table,

    /// <summary>Table column styles.</summary>
    TableColumn,

    /// <summary>Table row styles.</summary>
    TableRow,

    /// <summary>Table cell styles.</summary>
    TableCell,

    /// <summary>Graphic and frame styles.</summary>
    Graphic,

    /// <summary>Presentation styles, used for placeholder content.</summary>
    Presentation,

    /// <summary>Drawing page styles, carrying slide backgrounds and transitions.</summary>
    DrawingPage,

    /// <summary>List styles.</summary>
    List,

    /// <summary>Chart styles.</summary>
    Chart,

    /// <summary>Ruby styles (CJK annotation text).</summary>
    Ruby,

    /// <summary>
    /// Page layouts: the page size, margins and header/footer geometry a master page
    /// points at. Written as <c>style:page-layout</c> rather than as a
    /// <c>style:style</c>, but it participates in resolution the same way.
    /// </summary>
    PageLayout,
}

/// <summary>Maps between <see cref="OdfStyleFamily"/> and the names ODF writes.</summary>
public static class OdfStyleFamilies
{
    /// <summary>Parses a <c>style:family</c> attribute value.</summary>
    /// <returns>
    /// <see cref="OdfStyleFamily.Unknown"/> for an unrecognised or absent value, so that a
    /// style using a family from a later ODF version is still parsed and retrievable
    /// rather than discarded.
    /// </returns>
    public static OdfStyleFamily Parse(string? family) => family switch
    {
        "paragraph" => OdfStyleFamily.Paragraph,
        "text" => OdfStyleFamily.Text,
        "section" => OdfStyleFamily.Section,
        "table" => OdfStyleFamily.Table,
        "table-column" => OdfStyleFamily.TableColumn,
        "table-row" => OdfStyleFamily.TableRow,
        "table-cell" => OdfStyleFamily.TableCell,
        "graphic" => OdfStyleFamily.Graphic,
        "presentation" => OdfStyleFamily.Presentation,
        "drawing-page" => OdfStyleFamily.DrawingPage,
        "list-style" or "list" => OdfStyleFamily.List,
        "chart" => OdfStyleFamily.Chart,
        "ruby" => OdfStyleFamily.Ruby,
        "page-layout" => OdfStyleFamily.PageLayout,
        _ => OdfStyleFamily.Unknown,
    };

    /// <summary>The <c>style:family</c> attribute value for a family.</summary>
    public static string ToAttributeValue(this OdfStyleFamily family) => family switch
    {
        OdfStyleFamily.Paragraph => "paragraph",
        OdfStyleFamily.Text => "text",
        OdfStyleFamily.Section => "section",
        OdfStyleFamily.Table => "table",
        OdfStyleFamily.TableColumn => "table-column",
        OdfStyleFamily.TableRow => "table-row",
        OdfStyleFamily.TableCell => "table-cell",
        OdfStyleFamily.Graphic => "graphic",
        OdfStyleFamily.Presentation => "presentation",
        OdfStyleFamily.DrawingPage => "drawing-page",
        OdfStyleFamily.List => "list-style",
        OdfStyleFamily.Chart => "chart",
        OdfStyleFamily.Ruby => "ruby",
        OdfStyleFamily.PageLayout => "page-layout",
        _ => "unknown",
    };
}
