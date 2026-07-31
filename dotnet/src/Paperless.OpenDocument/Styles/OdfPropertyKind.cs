namespace Paperless.OpenDocument.Styles;

/// <summary>
/// Which <c>style:*-properties</c> element a formatting property lives in.
/// </summary>
/// <remarks>
/// This is part of a property's identity rather than a convenience. The same attribute
/// name means different things in different property sets — <c>fo:background-color</c>
/// exists in text, paragraph, table-cell and page-layout properties, and
/// <c>fo:margin-left</c> in both paragraph and table properties — and a single style
/// routinely carries several sets at once. Looking a property up by attribute name alone
/// would conflate a paragraph's shading with its text highlight.
/// </remarks>
public enum OdfPropertyKind
{
    /// <summary>Not a property set Paperless recognises.</summary>
    Unknown = 0,

    /// <summary>Character formatting: <c>style:text-properties</c>.</summary>
    Text,

    /// <summary>Paragraph formatting: <c>style:paragraph-properties</c>.</summary>
    Paragraph,

    /// <summary>Shape and frame formatting: <c>style:graphic-properties</c>.</summary>
    Graphic,

    /// <summary>Table formatting: <c>style:table-properties</c>.</summary>
    Table,

    /// <summary>Column formatting: <c>style:table-column-properties</c>.</summary>
    TableColumn,

    /// <summary>Row formatting: <c>style:table-row-properties</c>.</summary>
    TableRow,

    /// <summary>Cell formatting: <c>style:table-cell-properties</c>.</summary>
    TableCell,

    /// <summary>Section formatting: <c>style:section-properties</c>.</summary>
    Section,

    /// <summary>Page geometry: <c>style:page-layout-properties</c>.</summary>
    PageLayout,

    /// <summary>Header and footer geometry: <c>style:header-footer-properties</c>.</summary>
    HeaderFooter,

    /// <summary>Slide background and transitions: <c>style:drawing-page-properties</c>.</summary>
    DrawingPage,

    /// <summary>List label geometry: <c>style:list-level-properties</c>.</summary>
    ListLevel,

    /// <summary>Chart formatting: <c>style:chart-properties</c>.</summary>
    Chart,

    /// <summary>Ruby positioning: <c>style:ruby-properties</c>.</summary>
    Ruby,
}

/// <summary>Maps <c>style:*-properties</c> element names to <see cref="OdfPropertyKind"/>.</summary>
public static class OdfPropertyKinds
{
    /// <summary>
    /// Identifies a property set from its element's local name.
    /// </summary>
    /// <remarks>
    /// Matching on the local name alone is deliberate: LibreOffice writes some property
    /// sets in its own <c>loext:</c> namespace (a <c>loext:graphic-properties</c> child of
    /// a paragraph style, for instance) and those carry real formatting that would
    /// otherwise be dropped.
    /// </remarks>
    public static OdfPropertyKind FromElementName(string localName) => localName switch
    {
        "text-properties" => OdfPropertyKind.Text,
        "paragraph-properties" => OdfPropertyKind.Paragraph,
        "graphic-properties" => OdfPropertyKind.Graphic,
        "table-properties" => OdfPropertyKind.Table,
        "table-column-properties" => OdfPropertyKind.TableColumn,
        "table-row-properties" => OdfPropertyKind.TableRow,
        "table-cell-properties" => OdfPropertyKind.TableCell,
        "section-properties" => OdfPropertyKind.Section,
        "page-layout-properties" => OdfPropertyKind.PageLayout,
        "header-footer-properties" => OdfPropertyKind.HeaderFooter,
        "drawing-page-properties" => OdfPropertyKind.DrawingPage,
        "list-level-properties" => OdfPropertyKind.ListLevel,
        "chart-properties" => OdfPropertyKind.Chart,
        "ruby-properties" => OdfPropertyKind.Ruby,
        _ => OdfPropertyKind.Unknown,
    };
}
