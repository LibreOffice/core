using System.Globalization;
using System.Text;
using System.Xml;
using System.Xml.Linq;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;

namespace Paperless.Markup;

/// <summary>
/// Projects the shared content tree onto semantic XHTML — stage one of structured output.
/// </summary>
/// <remarks>
/// <para>
/// This is the lossless stage. Everything the content tree records that XHTML can express is
/// expressed here, including the two things GFM cannot: table cells that span rows or columns,
/// and tables nested inside cells. The Markdown writer is a transformation of <em>this</em>
/// tree, so the lossy step happens last and against a structure rather than against a document
/// format.
/// </para>
/// <para>
/// Splitting it this way is a verification decision as much as a design one. LibreOffice
/// exports XHTML for all three families, so stage one has a reference to be compared against
/// node for node — every heading's level, every list's nesting depth, every cell's row and
/// column — while stage two is a pure tree-to-text transformation with no document parsing in
/// it at all and can be tested in complete isolation from any office format.
/// </para>
/// <para>
/// XML escaping is delegated to <c>System.Xml</c> rather than hand-rolled. It is the one place
/// in the two stages where the platform already has the right answer, including the surrogate
/// pairs and the characters XML forbids outright.
/// </para>
/// </remarks>
public static class XhtmlWriter
{
    /// <summary>Serialises a content tree as an XHTML document.</summary>
    public static string ToXhtml(ContentNode content, MarkupOptions? options = null)
    {
        XDocument document = ToDocument(content, options);
        if (document.Root is null) return string.Empty;

        StringBuilder text = new();

        // The prologue is written by hand rather than by the serialiser, for two reasons that
        // both show up in the output. XmlWriter over a StringBuilder reports the encoding as
        // utf-16 whatever the settings say — the builder really is UTF-16 — and would put that
        // in the declaration of a document callers will save as UTF-8. And XDocumentType with
        // no external identifier serialises as "<!DOCTYPE html >", with the space.
        text.Append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE html>\n");

        XmlWriterSettings settings = new()
        {
            OmitXmlDeclaration = true,
            // Formatting is off deliberately: the indentation was inserted into the tree by
            // Xhtml.Indent, which knows which elements may safely carry it. See the note there.
            Indent = false,
            NewLineHandling = NewLineHandling.None,
        };

        using (XmlWriter writer = XmlWriter.Create(text, settings)) document.Root.Save(writer);
        text.Append('\n');
        return text.ToString();
    }

    /// <summary>
    /// Builds the XHTML tree for a content tree, without serialising it.
    /// </summary>
    /// <remarks>
    /// Public because it is what <see cref="MarkdownWriter"/> consumes, and a caller wanting a
    /// third projection should get the same starting point rather than re-walking the content
    /// tree.
    /// </remarks>
    public static XDocument ToDocument(ContentNode content, MarkupOptions? options = null)
    {
        ArgumentNullException.ThrowIfNull(content);
        options ??= MarkupOptions.Default;

        DocumentMetadata metadata = content is ContentDocument root
            ? root.Metadata
            : DocumentMetadata.Empty;

        XElement body = Xhtml.Element("body");
        Builder builder = new(options, metadata.Language);
        builder.WriteTop(content, body);

        XElement html = Xhtml.Element("html", Head(metadata, options), body);
        if (metadata.Language is { Length: > 0 } language)
        {
            html.SetAttributeValue("lang", language);
            html.SetAttributeValue(XNamespace.Xml + "lang", language);
        }

        Xhtml.Indent(html);
        return new XDocument(new XDeclaration("1.0", "utf-8", null), new XDocumentType("html", null, null, null), html);
    }

    private static XElement Head(DocumentMetadata metadata, MarkupOptions options)
    {
        XElement head = Xhtml.Element("head", Xhtml.Element("meta", new XAttribute("charset", "utf-8")));
        head.Add(Xhtml.Element("title", metadata.Title ?? string.Empty));
        if (!options.IncludeMetadata) return head;

        Meta("author", metadata.Author);
        Meta("description", metadata.Description);
        Meta("keywords", metadata.Keywords.Count == 0 ? null : string.Join(", ", metadata.Keywords));
        Meta("generator", "Paperless");
        return head;

        void Meta(string name, string? value)
        {
            if (string.IsNullOrEmpty(value)) return;
            head.Add(Xhtml.Element("meta", new XAttribute("name", name), new XAttribute("content", value)));
        }
    }

    /// <summary>
    /// The walk itself. A class rather than a pile of static methods because building lists
    /// from flat paragraphs needs state that persists across siblings.
    /// </summary>
    private sealed class Builder(MarkupOptions options, string? documentLanguage)
    {
        /// <summary>
        /// Paragraph style names that mean "block quotation", by format.
        /// </summary>
        /// <remarks>
        /// A style-name test rather than a structural one, because a block quotation is not
        /// structure in any of the four formats — it is a paragraph style with an indent. These
        /// are the names LibreOffice, Word and the ODF templates actually apply; anything else
        /// stays an ordinary paragraph, which is the safe direction to be wrong in.
        /// </remarks>
        private static readonly HashSet<string> QuoteStyles = new(StringComparer.OrdinalIgnoreCase)
        {
            "Quotations", "Quotation", "Quote", "Block Quotation", "Blockquote",
            "Intense Quote", "Block Text",
        };

        /// <summary>Paragraph style names that mean "preformatted block".</summary>
        private static readonly HashSet<string> PreformattedStyles = new(StringComparer.OrdinalIgnoreCase)
        {
            "Preformatted Text", "Preformatted", "HTML Preformatted", "Plain Text",
        };

        /// <summary>
        /// Character style names that mean "inline code".
        /// </summary>
        /// <remarks>
        /// "Source Text" is ODF's own name for it and is what LibreOffice applies; the rest are
        /// the Word and HTML equivalents. This is the only signal the content tree carries —
        /// <c>RunEmphasis</c> has no code flag — so without a style name a monospaced run is
        /// indistinguishable from any other and is left as plain text.
        /// </remarks>
        private static readonly HashSet<string> CodeStyles = new(StringComparer.OrdinalIgnoreCase)
        {
            "Source Text", "Source_20_Text", "Code", "HTML Code", "Teletype",
        };

        /// <summary>The open lists, innermost last, while a run of list paragraphs is walked.</summary>
        private readonly List<OpenList> _lists = [];

        public void WriteTop(ContentNode content, XElement body)
        {
            if (content is not ContentDocument and not ContentSection)
            {
                // A caller can hand any subtree in; wrap it so the output is still a document.
                XElement only = Xhtml.Element("section", new XAttribute("class", "body"));
                WriteContainer(content, only);
                body.Add(only);
                return;
            }

            foreach (ContentNode child in content.Children)
            {
                if (child is not ContentSection section) { WriteBlocks(child, body); continue; }
                if (Skip(section)) continue;
                body.Add(Section(section));
            }
        }

        private bool Skip(ContentSection section)
            => (section.IsHidden && !options.IncludeHiddenSections)
               || (section.Kind == SectionKind.Comment && !options.IncludeComments)
               || (section.Kind is SectionKind.Header or SectionKind.Footer && !options.IncludeHeadersAndFooters);

        /// <summary>
        /// Wraps a section in the element that says what it is.
        /// </summary>
        /// <remarks>
        /// Headers, footers, comments, notes, frames and speaker notes become
        /// <c>&lt;aside&gt;</c>/<c>&lt;header&gt;</c>/<c>&lt;footer&gt;</c> rather than being
        /// spliced into the body, because that is what they are in the content tree: separate
        /// flows, not part of the paragraph they are anchored in. Splicing them in would run
        /// two unrelated sentences together.
        /// </remarks>
        private XElement Section(ContentSection section)
        {
            (string name, string @class) = section.Kind switch
            {
                SectionKind.Body => ("section", "body"),
                SectionKind.Sheet => ("section", "sheet"),
                SectionKind.Slide => ("section", "slide"),
                SectionKind.SlideNotes => ("aside", "speaker-notes"),
                SectionKind.Header => ("header", "page-header"),
                SectionKind.Footer => ("footer", "page-footer"),
                SectionKind.Note => ("aside", "note"),
                SectionKind.Comment => ("aside", "comment"),
                SectionKind.Frame => ("aside", "frame"),
                _ => ("section", "body"),
            };

            XElement element = Xhtml.Element(name, new XAttribute("class", @class));
            element.SetAttributeValue("data-index", section.Index.ToString(CultureInfo.InvariantCulture));
            if (section.Name is { Length: > 0 } sectionName)
                element.SetAttributeValue("data-name", sectionName);
            if (section.IsHidden) element.SetAttributeValue("data-hidden", "true");

            // Sheets and slides get a heading synthesised from what names them, because neither
            // family puts one in its content: a sheet's identity is its tab name and a slide's
            // is its position. Body sections do not, because a word-processing document supplies
            // its own headings and a synthetic one would sit above them at the same level.
            string? heading = section.Kind switch
            {
                SectionKind.Sheet => section.Name is { Length: > 0 } n ? n : Ordinal("Sheet", section.Index),
                SectionKind.Slide => section.Name is { Length: > 0 } n ? n : Ordinal("Slide", section.Index),
                _ => null,
            };
            if (heading is not null) element.Add(Xhtml.Element("h1", heading));

            WriteContainer(section, element);
            return element;
        }

        private static string Ordinal(string label, int index)
            => string.Create(CultureInfo.InvariantCulture, $"{label} {index + 1}");

        /// <summary>
        /// Writes a node's children as blocks into a container that starts a fresh flow.
        /// </summary>
        /// <remarks>
        /// The open-list stack is saved and cleared around the call because a table cell is a
        /// flow of its own: a list inside a cell starts at level 0 again, and without this a
        /// cell's first list item would close the lists the table itself sits inside.
        /// </remarks>
        private void WriteContainer(ContentNode parent, XElement target)
        {
            List<OpenList> saved = [.. _lists];
            _lists.Clear();
            WriteBlocks(parent, target);
            _lists.Clear();
            _lists.AddRange(saved);
        }

        /// <summary>Writes a node's children as blocks into an XHTML container.</summary>
        private void WriteBlocks(ContentNode parent, XElement target)
        {
            foreach (ContentNode child in parent.Children)
            {
                switch (child)
                {
                    case ContentParagraph paragraph:
                        Paragraph(paragraph, target);
                        break;
                    case ContentTable table:
                        _lists.Clear();
                        Table(table, target);
                        break;
                    case ContentImage image:
                        _lists.Clear();
                        target.Add(Xhtml.Element("p", Image(image)));
                        break;
                    case ContentSection nested when !Skip(nested):
                        _lists.Clear();
                        target.Add(Section(nested));
                        break;
                    case ContentSection:
                        break;
                    default:
                        WriteBlocks(child, target);
                        break;
                }
            }
        }

        private void Paragraph(ContentParagraph paragraph, XElement target)
        {
            // A heading wins over a list level, and this is not a preference. Word attaches its
            // heading styles to an outline list, so in the DOC and DOCX of the same document
            // every heading arrives carrying ListLevel 0 and an empty marker — taken as a list
            // item it comes out as "- # Top level heading", a bullet wrapping a heading, in
            // every Word file in the corpus.
            if (paragraph.HeadingLevel is null && paragraph.ListLevel is int level && level >= 0)
            {
                ListItem(paragraph, level, target);
                return;
            }

            _lists.Clear();
            target.Add(Block(paragraph));
        }

        private XElement Block(ContentParagraph paragraph)
        {
            XElement element;
            if (paragraph.HeadingLevel is int heading and >= 1)
            {
                // Clamped: HTML stops at h6 and Word's outline levels go to nine. Flattening the
                // deepest three onto h6 keeps them headings, which is the part that carries
                // meaning; inventing an h7 would produce something no consumer understands.
                element = Xhtml.Element("h" + Math.Min(heading, 6).ToString(CultureInfo.InvariantCulture));
            }
            else if (paragraph.StyleName is { Length: > 0 } style && PreformattedStyles.Contains(style))
            {
                element = Xhtml.Element("pre");
            }
            else
            {
                element = Xhtml.Element("p");
            }

            if (paragraph.StyleName is { Length: > 0 } name)
                element.SetAttributeValue("data-style", name);

            // Bold and italic are dropped inside a heading. Every heading style in every one of
            // the four formats sets them, so emitting them would wrap the text of essentially
            // every heading in <strong> — noise that says nothing the h-level has not already
            // said, and which LibreOffice's own XHTML export does not emit either. Strikethrough,
            // code and links survive, because those are the author's and not the style's.
            Inlines(
                paragraph, element, suppressEmphasis: paragraph.HeadingLevel is >= 1, documentLanguage);

            if (paragraph.StyleName is { Length: > 0 } quoteStyle && QuoteStyles.Contains(quoteStyle))
                return Xhtml.Element("blockquote", element);

            return element;
        }

        /// <summary>
        /// Rebuilds nested lists from the flat, level-tagged paragraphs the content tree holds.
        /// </summary>
        /// <remarks>
        /// The tree records a list item as a paragraph with a nesting depth and the marker the
        /// document renders, which is what every one of the four formats stores; the
        /// <c>ul</c>/<c>ol</c> nesting has to be inferred from the depth. A paragraph at a list
        /// level with <em>no</em> marker is a continuation paragraph of the item above it — that
        /// is how ODF and OOXML both express "a second paragraph inside item two" — so it is
        /// appended to the open <c>li</c> rather than starting a new one.
        /// </remarks>
        private void ListItem(ContentParagraph paragraph, int level, XElement target)
        {
            bool ordered = IsOrdered(paragraph.ListMarker);

            while (_lists.Count > level + 1) _lists.RemoveAt(_lists.Count - 1);

            // A marker that changed kind at the same depth is a new list, not a continuation of
            // the old one: a bulleted list followed by a numbered one at the same level is two
            // lists, and merging them would renumber the second from the first's count.
            if (_lists.Count == level + 1 && _lists[^1].Ordered != ordered && paragraph.ListMarker is not null)
                _lists.RemoveAt(_lists.Count - 1);

            while (_lists.Count < level + 1)
            {
                XElement parent = _lists.Count == 0 ? target : ItemFor(_lists[^1]);
                XElement list = Xhtml.Element(ordered ? "ol" : "ul");
                if (_lists.Count == level && StartNumber(paragraph.ListMarker) is int start && start != 1)
                    list.SetAttributeValue("start", start.ToString(CultureInfo.InvariantCulture));
                parent.Add(list);
                _lists.Add(new OpenList(list, ordered));
            }

            OpenList open = _lists[^1];
            if (paragraph.ListMarker is not null || open.Item is null)
            {
                XElement item = Xhtml.Element("li");
                if (paragraph.ListMarker is { Length: > 0 } marker)
                    item.SetAttributeValue("data-marker", marker);
                open.List.Add(item);
                open.Item = item;
            }

            open.Item.Add(Block(paragraph));
        }

        /// <summary>
        /// The <c>li</c> a deeper list hangs off, creating an empty one when a document jumps
        /// straight from level 0 to level 2 without an item at level 1.
        /// </summary>
        private static XElement ItemFor(OpenList open)
        {
            if (open.Item is not null) return open.Item;
            XElement item = Xhtml.Element("li");
            open.List.Add(item);
            open.Item = item;
            return item;
        }

        /// <summary>
        /// Whether a rendered list marker means an ordered list.
        /// </summary>
        /// <remarks>
        /// Decided by whether the marker contains a letter or a digit, which covers "1.",
        /// "a)", "(iv)" and "第1章" as ordered and every bullet glyph — •, ◦, ▪, dingbats,
        /// images rendered as a character — as unordered. Matching a list of known bullet
        /// characters instead would misclassify every bullet outside the list, and there is no
        /// end to that list.
        /// </remarks>
        private static bool IsOrdered(string? marker)
            => marker is not null && marker.Any(char.IsLetterOrDigit);

        private static int? StartNumber(string? marker)
        {
            if (marker is null) return null;
            int i = 0;
            while (i < marker.Length && !char.IsDigit(marker[i])) i++;
            int start = i;
            while (i < marker.Length && char.IsDigit(marker[i])) i++;
            return i > start && int.TryParse(marker.AsSpan(start, i - start), CultureInfo.InvariantCulture, out int value)
                ? value
                : null;
        }

        private static void Inlines(
            ContentNode parent, XElement target, bool suppressEmphasis, string? documentLanguage)
        {
            foreach (ContentNode child in parent.Children)
            {
                switch (child)
                {
                    case ContentRun run:
                        foreach (XNode node in Run(run, suppressEmphasis, documentLanguage)) target.Add(node);
                        break;
                    case ContentImage image:
                        target.Add(Image(image));
                        break;
                    default:
                        Inlines(child, target, suppressEmphasis, documentLanguage);
                        break;
                }
            }
        }

        /// <summary>
        /// Wraps a run's text in the elements its emphasis calls for, innermost first.
        /// </summary>
        /// <remarks>
        /// A hard line break inside a run becomes <c>&lt;br/&gt;</c>: the content tree stores
        /// one as a newline inside the run's text, and leaving it as a newline would let HTML
        /// collapse it into a space.
        /// </remarks>
        private static XNode[] Run(ContentRun run, bool suppressEmphasis, string? documentLanguage)
        {
            List<XNode> content = [];
            string[] lines = run.Text.Split('\n');
            for (int i = 0; i < lines.Length; i++)
            {
                if (i > 0) content.Add(Xhtml.Element("br"));
                if (lines[i].Length > 0) content.Add(new XText(lines[i]));
            }

            if (content.Count == 0) return [];

            XNode[] current = [.. content];
            current = Wrap(current, Has(run, RunEmphasis.Subscript) ? "sub" : null);
            current = Wrap(current, Has(run, RunEmphasis.Superscript) ? "sup" : null);
            current = Wrap(current, run.StyleName is { Length: > 0 } s && CodeStyles.Contains(s) ? "code" : null);
            current = Wrap(current, Has(run, RunEmphasis.Underline) ? "u" : null);
            current = Wrap(current, Has(run, RunEmphasis.Strikethrough) ? "s" : null);
            current = Wrap(current, !suppressEmphasis && Has(run, RunEmphasis.Italic) ? "em" : null);
            current = Wrap(current, !suppressEmphasis && Has(run, RunEmphasis.Bold) ? "strong" : null);

            if (run.HyperlinkTarget is { Length: > 0 } target)
            {
                XElement anchor = Xhtml.Element("a", new XAttribute("href", target));
                foreach (XNode node in current) anchor.Add(node);
                current = [anchor];
            }

            // Language is recorded only where it says something: a run in the document's own
            // language repeats what <html lang> already states, and tagging every run with it
            // would put an attribute on most elements in the file. A run in a *different*
            // language is the case a consumer cares about, and it is rare enough to justify a
            // <span> of its own when there is no other element to carry it.
            // Nothing is tagged when the document's own language is unknown: "different from
            // the document" is not a judgement that can be made without knowing what the
            // document is, and tagging every run instead would put a <span> around every word of
            // a DOC, whose metadata records no language at all.
            if (run.Language is { Length: > 0 } language
                && documentLanguage is { Length: > 0 }
                && !string.Equals(language, documentLanguage, StringComparison.OrdinalIgnoreCase))
            {
                if (current is not [XElement single])
                {
                    single = Xhtml.Element("span");
                    foreach (XNode node in current) single.Add(node);
                    current = [single];
                }
                single.SetAttributeValue(XNamespace.Xml + "lang", language);
            }

            return current;
        }

        private static bool Has(ContentRun run, RunEmphasis flag) => (run.Emphasis & flag) != 0;

        private static XNode[] Wrap(XNode[] content, string? name)
        {
            if (name is null) return content;
            XElement element = Xhtml.Element(name);
            foreach (XNode node in content) element.Add(node);
            return [element];
        }

        /// <summary>
        /// An image, as a placeholder that names it rather than as an omission.
        /// </summary>
        /// <remarks>
        /// Extraction records that a graphic exists and its alternative text, but does not
        /// decode the pixels, so there is nothing to point <c>src</c> at except the container
        /// part that holds the bytes. That is a real reference — it resolves against the
        /// original file — and it is the honest one; an empty <c>src</c> means "this page" in
        /// HTML, which is worse than saying nothing.
        /// </remarks>
        private static XElement Image(ContentImage image)
        {
            XElement element = Xhtml.Element("img", new XAttribute("src", image.PartName ?? string.Empty));
            element.SetAttributeValue("alt", image.AlternativeText ?? string.Empty);
            if (image.MediaType is { Length: > 0 } mediaType)
                element.SetAttributeValue("data-media-type", mediaType);
            return element;
        }

        private void Table(ContentTable table, XElement target)
        {
            XElement element = Xhtml.Element("table");
            List<ContentTableRow> rows = [.. table.Children.OfType<ContentTableRow>()];

            int rowLimit = Math.Min(rows.Count, Math.Max(options.MaxTableRows, 1));
            int droppedRows = rows.Count - rowLimit;
            int droppedColumns = 0;

            XElement? head = table.HeaderRowCount > 0 ? Xhtml.Element("thead") : null;
            XElement bodyRows = Xhtml.Element("tbody");
            if (head is not null) element.Add(head);
            element.Add(bodyRows);

            for (int i = 0; i < rowLimit; i++)
            {
                bool header = i < table.HeaderRowCount;
                XElement row = Xhtml.Element("tr");
                int column = 0;
                foreach (ContentTableCell cell in rows[i].Children.OfType<ContentTableCell>())
                {
                    if (column >= Math.Max(options.MaxTableColumns, 1)) { droppedColumns++; continue; }
                    row.Add(Cell(cell, header));
                    column += Math.Max(cell.ColumnSpan, 1);
                }
                (header ? head! : bodyRows).Add(row);
            }

            target.Add(element);

            // A sibling of the table rather than a caption inside it, so that the bound stays
            // visible in every projection: a Markdown consumer that renders the table as a pipe
            // table would otherwise have nowhere to put a caption.
            if (droppedRows > 0 || droppedColumns > 0)
                target.Add(TruncationNote(droppedRows, droppedColumns));
        }

        private XElement Cell(ContentTableCell cell, bool header)
        {
            XElement element = Xhtml.Element(header ? "th" : "td");
            if (cell.ColumnSpan > 1)
                element.SetAttributeValue("colspan", cell.ColumnSpan.ToString(CultureInfo.InvariantCulture));
            if (cell.RowSpan > 1)
                element.SetAttributeValue("rowspan", cell.RowSpan.ToString(CultureInfo.InvariantCulture));

            // A spreadsheet cell's formula is something only Paperless has: LibreOffice's XHTML
            // export writes the cached result and nothing else. Kept on the XHTML because this
            // stage is the lossless one; Markdown drops it.
            if (cell.Formula is { Length: > 0 } formula)
                element.SetAttributeValue("data-formula", formula);

            WriteContainer(cell, element);
            return element;
        }

        private static XElement TruncationNote(int rows, int columns)
        {
            List<string> parts = [];
            if (rows > 0) parts.Add(string.Create(CultureInfo.InvariantCulture, $"{rows:N0} further rows"));
            if (columns > 0) parts.Add(string.Create(CultureInfo.InvariantCulture, $"{columns:N0} further cells"));
            // Emphasised so that the bound survives every projection: the Markdown writer turns
            // <em> into asterisks without needing to know what a truncation note is.
            return Xhtml.Element(
                "p",
                new XAttribute("class", "truncation"),
                Xhtml.Element("em", string.Join(" and ", parts) + " omitted."));
        }
    }

    private sealed class OpenList(XElement list, bool ordered)
    {
        public XElement List { get; } = list;
        public bool Ordered { get; } = ordered;
        public XElement? Item { get; set; }
    }
}
