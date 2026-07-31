using System.Text;
using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.OpenDocument;

/// <summary>
/// Turns an ODF text document's body into the paragraphs the paginator takes.
/// </summary>
/// <remarks>
/// <para>
/// A second walk over <c>content.xml</c>, separate from the one that builds the extraction tree, and
/// deliberately so: extraction discards font sizes, indents and spacing because it must not pay for
/// them, and layout needs nothing else. Sharing one walk would mean either extraction carrying
/// formatting it never reads or layout re-deriving it from a tree that no longer has it.
/// </para>
/// <para>
/// Only the properties that decide where text goes are resolved. A run's colour and its underline do
/// not move a line break, so they are left to whatever draws the page; a run's <em>size</em> does, and
/// that is the one piece of per-run formatting this cannot yet honour — the tallest run on a line sets
/// the line's height, and until the runs are walked a paragraph is measured wholly in its paragraph
/// style's font. A document whose emphasis is a different size from its body therefore lays out with
/// slightly short lines.
/// </para>
/// </remarks>
public sealed class OdtLayoutSource
{
    /// <summary>
    /// How many paragraphs are read before the rest are ignored.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input rather than a real limit. A book is tens of thousands of paragraphs;
    /// a generated file can claim millions, and each one costs a shaping pass.
    /// </remarks>
    public const int MaxParagraphs = 200000;

    /// <summary>
    /// The character a <c>text:line-break</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, the Unicode line separator, because UAX #14 gives it a mandatory break and the break
    /// iterator therefore honours it without layout having to special-case anything. A newline would do
    /// the same but would also read as the end of a paragraph to anything that later scans the text,
    /// which a line break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    /// <summary>
    /// The character an anchor occupies.
    /// </summary>
    /// <remarks>
    /// The same one the document model uses, so an offset counted here means the same thing there. It
    /// has a width but no text: a footnote's citation occupies a position in the sentence that cites it
    /// while its body lives in another flow.
    /// </remarks>
    private const char AnchorCharacter = '\u0001';

    private readonly OdfStyles _styles;
    private readonly SystemFontResolver _fonts;
    private readonly Dictionary<(string? Family, int Weight, bool Italic), OpenTypeFace> _faces = [];

    /// <summary>Creates a source over a document's styles.</summary>
    /// <param name="styles">The document's resolved styles.</param>
    /// <param name="fonts">The font resolver, or null to build one over the installed fonts.</param>
    public OdtLayoutSource(OdfStyles styles, SystemFontResolver? fonts = null)
    {
        ArgumentNullException.ThrowIfNull(styles);
        _styles = styles;
        _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());
    }

    /// <summary>The substitutions made while resolving the document's fonts.</summary>
    /// <remarks>
    /// Worth surfacing rather than swallowing: a substitution that is not metric-compatible changes
    /// every line break after the first paragraph that uses it, so it is the first thing to check when a
    /// comparison against a reference renderer disagrees.
    /// </remarks>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>
    /// Reads the body's paragraphs, in document order.
    /// </summary>
    /// <param name="body">The <c>office:text</c> element.</param>
    public List<PageParagraph> Read(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        List<PageParagraph> paragraphs = [];
        Walk(body, paragraphs, depth: 0);
        return paragraphs;
    }

    /// <summary>
    /// Walks the body's block-level children.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Sections, lists and the change-tracking wrappers are transparent: their children are body-level
    /// content and a walk that stopped at them would lose whole chapters of a real document. A table's
    /// cells are not walked here, because a table is laid out as a grid rather than as a run of
    /// paragraphs and needs the row heights first.
    /// </para>
    /// <para>
    /// A <c>text:h</c> is a paragraph as far as layout is concerned. Its outline level changes which
    /// style it resolves through, which the style chain already handles.
    /// </para>
    /// </remarks>
    private void Walk(XElement element, List<PageParagraph> into, int depth)
    {
        // Deep nesting is legal — a list inside a section inside a change region — but a file can nest
        // indefinitely, and this recurses on untrusted input.
        if (depth > 64 || into.Count >= MaxParagraphs) return;

        foreach (XElement child in element.Elements())
        {
            if (into.Count >= MaxParagraphs) return;

            string name = child.Name.LocalName;
            string ns = child.Name.NamespaceName;

            if (ns == OdfNamespaces.Text && name is "p" or "h")
            {
                if (Paragraph(child) is { } paragraph) into.Add(paragraph);
                continue;
            }

            if (ns == OdfNamespaces.Text
                && name is "list" or "list-item" or "list-header" or "section" or "index-body"
                    or "table-of-content" or "alphabetical-index" or "illustration-index"
                    or "table-index" or "object-index" or "user-index" or "bibliography"
                    or "tracked-changes" or "deletion" or "insertion")
            {
                // A deletion's content is inside a change region and is not part of the text a reader
                // sees, so it is skipped rather than walked — the same decision every one of Paperless's
                // extraction readers makes.
                if (name is not ("tracked-changes" or "deletion")) Walk(child, into, depth + 1);
                continue;
            }

            if (ns == OdfNamespaces.Table && name == "table")
            {
                // Tables are laid out as grids, not as a run of paragraphs. Skipped rather than
                // flattened, because flattening would give the page a height that no table has.
                continue;
            }
        }
    }

    private PageParagraph? Paragraph(XElement element)
    {
        string? styleName = element
            .Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;

        OdfTextStyle text = OdfParagraphFormats.ResolveText(_styles, styleName);
        OpenTypeFace? face = Face(text);
        if (face is null) return null;

        return new PageParagraph
        {
            Text = TextOf(element),
            Face = face,
            Format = OdfParagraphFormats.Resolve(_styles, styleName),
            EmSize = text.Size,
            Language = text.Language,
            Shaping = new ShapingOptions(Language: text.Language),
            Source = element,
        };
    }

    /// <summary>
    /// A paragraph's text, with the things that occupy a position but are not characters left out.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF encodes runs of spaces, tabs and line breaks as elements rather than as characters, so a
    /// reader that took the descendant text nodes alone would lose every one of them — and
    /// <c>text:s</c> in particular, which is how any run of two or more spaces is written.
    /// </para>
    /// <para>
    /// A note's or comment's body is inside the paragraph in the file but is not part of its text, so it
    /// contributes only the anchor character it occupies. Walking into it would put a footnote's whole
    /// text into the middle of the sentence that cites it.
    /// </para>
    /// </remarks>
    private static string TextOf(XElement paragraph)
    {
        StringBuilder builder = new();
        Append(paragraph, builder, depth: 0);
        return builder.ToString();

        static void Append(XElement element, StringBuilder builder, int depth)
        {
            if (depth > 64) return;

            foreach (XNode node in element.Nodes())
            {
                switch (node)
                {
                    case XText textNode:
                        builder.Append(textNode.Value);
                        break;

                    case XElement child when child.Name.NamespaceName == OdfNamespaces.Text:
                        AppendTextElement(child, builder, depth);
                        break;

                    case XElement child:
                        Append(child, builder, depth + 1);
                        break;

                    default:
                        break;
                }
            }
        }

        static void AppendTextElement(XElement child, StringBuilder builder, int depth)
        {
            switch (child.Name.LocalName)
            {
                case "s":
                    // text:c is the count, and its absence means one space rather than none.
                    int count = 1;
                    if (child.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value is { } value
                        && int.TryParse(value, out int declared)
                        && declared is > 0 and <= 4096)
                    {
                        count = declared;
                    }
                    builder.Append(' ', count);
                    break;

                case "tab":
                    builder.Append('\t');
                    break;

                case "line-break":
                    // A line break within a paragraph, which layout has to honour as a forced break —
                    // recorded as the character so the break iterator sees it.
                    builder.Append(LineSeparator);
                    break;

                case "note" or "annotation" or "annotation-end":
                    // The anchor occupies a position; its body is a separate flow.
                    builder.Append(AnchorCharacter);
                    break;

                case "soft-page-break" or "bookmark" or "bookmark-start" or "bookmark-end"
                    or "reference-mark" or "reference-mark-start" or "reference-mark-end"
                    or "change" or "change-start" or "change-end":
                    break;

                default:
                    Append(child, builder, depth + 1);
                    break;
            }
        }
    }

    /// <summary>
    /// The face a text style resolves to, cached per distinct request.
    /// </summary>
    /// <remarks>
    /// Cached because a document has a handful of fonts and thousands of paragraphs, and resolving one
    /// means walking the substitution chain and reading a font file. Null only when nothing at all could
    /// be loaded, which means the machine has no usable fonts rather than that the document is bad.
    /// </remarks>
    private OpenTypeFace? Face(OdfTextStyle text)
    {
        (string? Family, int Weight, bool Italic) key = (text.FamilyName, text.Weight, text.IsItalic);
        if (_faces.TryGetValue(key, out OpenTypeFace? cached)) return cached;

        try
        {
            FontReference reference = _fonts.Resolve(new FontRequest(
                text.FamilyName ?? string.Empty, text.Weight, text.IsItalic));

            OpenTypeFace face = _fonts.LoadOpenType(reference);
            _faces[key] = face;
            return face;
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A font that cannot be read is not a reason to lose the paragraph, but there is nothing to
            // measure it with either — so it is dropped and the caller sees a shorter document rather
            // than an exception from the middle of a layout.
            _faces[key] = null!;
            return null;
        }
    }
}
