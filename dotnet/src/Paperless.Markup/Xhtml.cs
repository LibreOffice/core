using System.Xml.Linq;

namespace Paperless.Markup;

/// <summary>
/// The vocabulary the Paperless XHTML writer emits, and the helpers both stages share.
/// </summary>
/// <remarks>
/// <para>
/// Paperless's XHTML is deliberately <em>semantic</em>: headings, lists, tables with
/// <c>colspan</c>/<c>rowspan</c>, <c>em</c>/<c>strong</c>, <c>a[href]</c>, and sectioning
/// elements for the things only Paperless extracts. It is not modelled on LibreOffice's own
/// XHTML export, which is presentation-oriented — inline CSS, <c>span</c> wrappers, absolutely
/// positioned <c>div</c>s — and reproduces a rendering rather than a structure. That export is
/// used as an <em>oracle</em> for structure in the fidelity tests, never as a target.
/// </para>
/// <para>
/// Paperless-specific meaning is carried on <c>class</c> and <c>data-</c> attributes rather
/// than on invented element names, so the output stays ordinary XHTML that any HTML consumer
/// can read while still round-tripping into Markdown without guessing.
/// </para>
/// </remarks>
public static class Xhtml
{
    /// <summary>The XHTML namespace, which every emitted element lives in.</summary>
    public static readonly XNamespace Namespace = "http://www.w3.org/1999/xhtml";

    /// <summary>Builds an element in the XHTML namespace.</summary>
    internal static XElement Element(string name, params object?[] content)
        => new(Namespace + name, content);

    /// <summary>
    /// Elements whose children may be indented when the element holds no text of its own.
    /// </summary>
    /// <remarks>
    /// Indentation is applied by an explicit pass rather than by <c>XmlWriter</c>'s
    /// <c>Indent</c> setting, and the difference is not cosmetic. <c>XmlWriter</c>
    /// decides progressively, so it indents before the first child of
    /// <c>&lt;p&gt;&lt;strong&gt;a&lt;/strong&gt; tail&lt;/p&gt;</c> — it has not yet seen the
    /// text that makes the content mixed. Worse, an element whose content is <em>only</em>
    /// inline elements, <c>&lt;p&gt;&lt;strong&gt;a&lt;/strong&gt;&lt;em&gt;b&lt;/em&gt;&lt;/p&gt;</c>,
    /// is element-only by that test and indenting it inserts a space between "a" and "b" that
    /// the document does not contain. So the set is named explicitly and holds block containers
    /// only, where whitespace between children is insignificant in HTML.
    /// </remarks>
    private static readonly HashSet<string> IndentableContainers = new(StringComparer.Ordinal)
    {
        "html", "head", "body", "section", "aside", "header", "footer", "article",
        "blockquote", "ul", "ol", "li", "table", "thead", "tbody", "tr", "th", "td", "figure",
    };

    /// <summary>
    /// Elements HTML defines as empty, which are the only ones that may be self-closed.
    /// </summary>
    /// <remarks>
    /// Anything else has to be written with a closing tag even when it holds nothing. An HTML
    /// parser reads <c>&lt;p/&gt;</c> as an <em>opening</em> tag and swallows everything after
    /// it — well-formed as XML, catastrophic as HTML — and an empty paragraph is exactly what an
    /// empty table cell produces.
    /// </remarks>
    private static readonly HashSet<string> VoidElements = new(StringComparer.Ordinal)
    {
        "area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "source",
        "track", "wbr",
    };

    /// <summary>
    /// Inserts whitespace between the children of block containers so the output is readable
    /// without changing what it says, and gives empty non-void elements a closing tag.
    /// </summary>
    internal static void Indent(XElement element, int depth = 0)
    {
        ArgumentNullException.ThrowIfNull(element);

        if (!element.HasElements && element.IsEmpty && !VoidElements.Contains(element.Name.LocalName))
            element.Add(new XText(string.Empty));

        List<XElement> children = [.. element.Elements()];
        if (children.Count > 0
            && IndentableContainers.Contains(element.Name.LocalName)
            && !element.Nodes().OfType<XText>().Any())
        {
            string inner = "\n" + new string(' ', 2 * (depth + 1));
            foreach (XElement child in children) child.AddBeforeSelf(new XText(inner));
            children[^1].AddAfterSelf(new XText("\n" + new string(' ', 2 * depth)));
        }

        foreach (XElement child in children) Indent(child, depth + 1);
    }
}
