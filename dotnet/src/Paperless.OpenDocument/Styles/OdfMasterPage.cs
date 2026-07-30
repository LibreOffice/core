using System.Xml.Linq;

namespace Paperless.OpenDocument.Styles;

/// <summary>
/// A <c>style:master-page</c>: the page or slide master a document's content is laid on.
/// </summary>
/// <remarks>
/// <para>
/// A master page is the pairing of a <em>geometry</em> — the <c>style:page-layout</c> it
/// names, holding size, margins and header/footer heights — with <em>content</em>, the
/// header and footer bodies it holds directly. LibreOffice's <c>SwPageDesc</c> is the same
/// pairing (<c>dotnet/research/02-writer.md</c> section A.9).
/// </para>
/// <para>
/// In a presentation the same element does double duty: it also holds the master slide's
/// own shapes, which is why <see cref="Shapes"/> exists alongside the header and footer.
/// </para>
/// </remarks>
public sealed class OdfMasterPage
{
    internal OdfMasterPage(XElement element)
    {
        Name = element.Attribute(XName.Get("name", OdfNamespaces.Style))?.Value ?? string.Empty;
        DisplayName = element.Attribute(XName.Get("display-name", OdfNamespaces.Style))?.Value;
        PageLayoutName = element.Attribute(XName.Get("page-layout-name", OdfNamespaces.Style))?.Value;
        DrawStyleName = element.Attribute(XName.Get("style-name", OdfNamespaces.Draw))?.Value;
        Element = element;

        Header = element.Element(XName.Get("header", OdfNamespaces.Style));
        Footer = element.Element(XName.Get("footer", OdfNamespaces.Style));
        LeftHeader = element.Element(XName.Get("header-left", OdfNamespaces.Style));
        LeftFooter = element.Element(XName.Get("footer-left", OdfNamespaces.Style));
        FirstHeader = element.Element(XName.Get("header-first", OdfNamespaces.Style));
        FirstFooter = element.Element(XName.Get("footer-first", OdfNamespaces.Style));

        Shapes = [.. element.Elements().Where(e => e.Name.NamespaceName is OdfNamespaces.Draw
                                                                        or OdfNamespaces.Dr3d)];
    }

    /// <summary>The master page's name, as referenced by <c>style:master-page-name</c>.</summary>
    public string Name { get; }

    /// <summary>The user-visible name, when recorded separately.</summary>
    public string? DisplayName { get; }

    /// <summary>The <c>style:page-layout</c> holding this master's geometry.</summary>
    public string? PageLayoutName { get; }

    /// <summary>The drawing-page style, which carries a slide master's background.</summary>
    public string? DrawStyleName { get; }

    /// <summary>The header body, or null when the master has none.</summary>
    public XElement? Header { get; }

    /// <summary>The footer body, or null when the master has none.</summary>
    public XElement? Footer { get; }

    /// <summary>
    /// The left-page header. Absent means left and right pages share
    /// <see cref="Header"/> — not that left pages have no header.
    /// </summary>
    public XElement? LeftHeader { get; }

    /// <summary>The left-page footer; absent means it is shared with <see cref="Footer"/>.</summary>
    public XElement? LeftFooter { get; }

    /// <summary>The first-page header, when the master distinguishes one.</summary>
    public XElement? FirstHeader { get; }

    /// <summary>The first-page footer, when the master distinguishes one.</summary>
    public XElement? FirstFooter { get; }

    /// <summary>
    /// The shapes drawn on the master itself. Empty for a word-processing page master;
    /// populated for a presentation's master slide.
    /// </summary>
    public IReadOnlyList<XElement> Shapes { get; }

    /// <summary>The underlying element, for content Paperless does not model yet.</summary>
    public XElement Element { get; }
}
