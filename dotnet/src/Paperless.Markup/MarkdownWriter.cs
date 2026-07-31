using System.Globalization;
using System.Text;
using System.Xml.Linq;
using Paperless.Core.Extraction;

namespace Paperless.Markup;

/// <summary>
/// Transforms the semantic XHTML of <see cref="XhtmlWriter"/> into Markdown — stage two.
/// </summary>
/// <remarks>
/// <para>
/// <b>The flavour is GitHub-Flavored Markdown.</b> Stated once here rather than per feature,
/// because the choice is forced: CommonMark has no tables at all, and a document projection
/// that cannot express a table is not worth having. Tables, strikethrough and footnote
/// definitions are the GFM extensions used; everything else emitted is plain CommonMark.
/// </para>
/// <para>
/// Nothing in this file reads a document. It walks an XHTML tree and writes text, which is
/// what makes it testable in isolation from every office format — and what makes the escaping,
/// which is the part that silently corrupts output, checkable by parsing the result back.
/// </para>
/// <para>
/// <b>What Markdown cannot carry, and what happens to it.</b> Underline, superscript and
/// subscript have no syntax, so their text is kept and the formatting dropped. An ordered
/// list's marker style — letters, roman numerals — collapses to decimal, since GFM numbers
/// with digits only; the start number survives. A cell's formula, a paragraph's style name and
/// a section's index are dropped, all of which remain in the XHTML.
/// </para>
/// </remarks>
public static class MarkdownWriter
{
    /// <summary>Projects a content tree onto Markdown, via XHTML.</summary>
    public static string ToMarkdown(ContentNode content, MarkupOptions? options = null)
        => FromXhtml(XhtmlWriter.ToDocument(content, options));

    /// <summary>Transforms an XHTML document produced by <see cref="XhtmlWriter"/>.</summary>
    /// <remarks>
    /// Takes no <see cref="MarkupOptions"/>, and deliberately: every option there is consumed
    /// while walking the content tree, so by this stage the decisions are already in the XHTML.
    /// A second copy of them here could disagree with the first.
    /// </remarks>
    public static string FromXhtml(XDocument xhtml)
    {
        ArgumentNullException.ThrowIfNull(xhtml);
        XElement? body = xhtml.Root?.Elements(Xhtml.Namespace + "body").FirstOrDefault();
        return body is null ? string.Empty : FromXhtml(body);
    }

    /// <inheritdoc cref="FromXhtml(XDocument)"/>
    public static string FromXhtml(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);
        Renderer renderer = new();
        return renderer.Render(element);
    }

    private sealed class Renderer
    {
        private readonly StringBuilder _out = new();
        private readonly List<XElement> _notes = [];
        private string _prefix = string.Empty;
        private string? _firstLinePrefix;
        private bool _blankPending;

        public string Render(XElement root)
        {
            Blocks(root);
            Notes();
            return _out.ToString();
        }

        // ---- block level -------------------------------------------------------------

        private void Blocks(XElement container)
        {
            foreach (XElement child in container.Elements()) Block(child);
        }

        private void Block(XElement element)
        {
            switch (element.Name.LocalName)
            {
                case "h1" or "h2" or "h3" or "h4" or "h5" or "h6":
                    int level = element.Name.LocalName[1] - '0';
                    Paragraph(new string('#', level) + " " + Inline(element, atLineStart: false));
                    break;

                case "p":
                    string text = Inline(element, atLineStart: true);
                    if (text.Trim().Length > 0) Paragraph(text);
                    break;

                case "pre":
                    Fenced(element.Value);
                    break;

                case "blockquote":
                    Quoted(null, () => Blocks(element));
                    break;

                case "ul" or "ol":
                    List(element);
                    break;

                case "table":
                    Table(element);
                    break;

                case "img":
                    Paragraph(Image(element));
                    break;

                case "aside" when Class(element) == "note":
                    _notes.Add(element);
                    break;

                case "aside" or "header" or "footer":
                    Quoted(Label(element), () => Blocks(element));
                    break;

                case "section" or "article" or "div" or "figure":
                    Blocks(element);
                    if (element.Attribute("data-hidden")?.Value == "true") Paragraph("*Hidden.*");
                    break;

                default:
                    Blocks(element);
                    break;
            }
        }

        /// <summary>
        /// The heading a set-apart flow is introduced with.
        /// </summary>
        /// <remarks>
        /// Speaker notes, page headers and footers, comments and text-frame content are all
        /// things a reader sees somewhere other than in the body, and Markdown has nowhere else
        /// to put them. A labelled block quote keeps them present, attributed and visibly not
        /// body text, which is the property that matters to anything indexing the output.
        /// </remarks>
        private static string? Label(XElement element)
        {
            string label = Class(element) switch
            {
                "speaker-notes" => "Speaker notes",
                "page-header" => "Header",
                "page-footer" => "Footer",
                "comment" => "Comment",
                "frame" => "Text frame",
                _ => string.Empty,
            };
            if (label.Length == 0) return null;

            string? name = element.Attribute("data-name")?.Value;
            return name is { Length: > 0 } && Class(element) == "comment"
                ? "**" + MarkdownEscape.Inline(label + " — " + name) + "**"
                : "**" + MarkdownEscape.Inline(label) + "**";
        }

        private static string Class(XElement element) => element.Attribute("class")?.Value ?? string.Empty;

        private void Quoted(string? label, Action content)
        {
            // The separating blank line belongs to the flow the quotation interrupts, so it is
            // flushed before the prefix changes; written afterwards it would come out as a bare
            // ">" and start the quotation one line early.
            FlushBlank();
            string saved = _prefix;
            _prefix = saved + "> ";
            if (label is not null) Paragraph(label);
            content();
            _prefix = saved;
        }

        private void List(XElement list)
        {
            bool ordered = list.Name.LocalName == "ol";
            int number = 1;
            if (ordered && int.TryParse(
                    list.Attribute("start")?.Value, CultureInfo.InvariantCulture, out int start))
            {
                number = start;
            }

            List<XElement> items = [.. list.Elements(Xhtml.Namespace + "li")];
            for (int i = 0; i < items.Count; i++)
            {
                string marker = ordered
                    ? string.Create(CultureInfo.InvariantCulture, $"{number++}. ")
                    : "- ";

                string saved = _prefix;
                _firstLinePrefix = saved + marker;
                _prefix = saved + new string(' ', marker.Length);

                // Items are packed tight: a blank line between them makes GFM render the whole
                // list loose, wrapping every item in a paragraph. Blocks *inside* an item still
                // need their blank line, except a nested list, which must follow its item's text
                // immediately or it becomes a separate list.
                if (i > 0) _blankPending = false;
                Item(items[i]);

                _prefix = saved;
                _firstLinePrefix = null;
            }
        }

        /// <summary>
        /// Writes one list item's blocks, packing a nested list against its parent's text where
        /// that is safe and separating it with a blank line where it is not.
        /// </summary>
        /// <remarks>
        /// A nested list only stays attached without a blank line if it can interrupt a
        /// paragraph, and CommonMark's rule for that is narrow: a bullet list always can, an
        /// ordered list only when it starts at 1. So a sub-list numbered from 7, or one arriving
        /// after a continuation paragraph, gets the blank line — without it the marker would be
        /// read as ordinary text and the whole sub-list would vanish into the paragraph above.
        /// </remarks>
        private void Item(XElement item)
        {
            List<XElement> blocks = [.. item.Elements()];
            for (int i = 0; i < blocks.Count; i++)
            {
                if (i > 0) _blankPending = !(i == 1 && CanFollowDirectly(blocks[i]));
                Block(blocks[i]);
            }
            if (blocks.Count == 0) Paragraph(string.Empty);
        }

        private static bool CanFollowDirectly(XElement block) => block.Name.LocalName switch
        {
            "ul" => true,
            "ol" => block.Attribute("start") is null,
            _ => false,
        };

        private void Notes()
        {
            for (int i = 0; i < _notes.Count; i++)
            {
                string label = NoteLabel(_notes[i], i);
                string saved = _prefix;
                _firstLinePrefix = saved + "[^" + label + "]: ";
                _prefix = saved + "    ";
                Blocks(_notes[i]);
                _prefix = saved;
                _firstLinePrefix = null;
            }
        }

        /// <summary>
        /// A footnote's GFM label, taken from the number the document itself rendered.
        /// </summary>
        /// <remarks>
        /// The content tree names a note section after the anchor text the document shows — "1",
        /// "i", "*" — so reusing it keeps the Markdown's numbering the document's own rather
        /// than a fresh one. A name GFM cannot carry in a label falls back to the ordinal.
        /// </remarks>
        private static string NoteLabel(XElement note, int index)
        {
            string? name = note.Attribute("data-name")?.Value;
            return name is { Length: > 0 } && name.All(c => char.IsLetterOrDigit(c) || c is '-' or '_' or '.')
                ? name
                : (index + 1).ToString(CultureInfo.InvariantCulture);
        }

        private void Fenced(string content)
        {
            // The fence is long enough to survive content that contains one, which is CommonMark's
            // own rule and is cheaper than escaping the body.
            int longest = 0, run = 0;
            foreach (char c in content)
            {
                run = c == '`' ? run + 1 : 0;
                if (run > longest) longest = run;
            }
            string fence = new('`', Math.Max(3, longest + 1));
            Paragraph(fence + "\n" + content.TrimEnd('\n') + "\n" + fence);
        }

        // ---- tables ------------------------------------------------------------------

        /// <summary>
        /// Emits a table as a GFM pipe table when GFM can express it, and as raw HTML when it
        /// cannot.
        /// </summary>
        /// <remarks>
        /// <para>
        /// GFM tables have no row or column spans, no nesting, and exactly one line per cell,
        /// and the corpus is full of all three. The choice made here is an <b>HTML fallback</b>
        /// rather than a flattening: an HTML <c>&lt;table&gt;</c> is valid Markdown — a raw HTML
        /// block — and stage one has already built exactly the tree to serialise, so nothing is
        /// lost and nothing is guessed.
        /// </para>
        /// <para>
        /// <b>What that costs.</b> A fallback table is not Markdown: a consumer that reads
        /// Markdown as text, or one that strips HTML rather than parsing it, sees tags or sees
        /// nothing. The alternative — flattening a span into repeated cells and splicing a
        /// nested table's rows into its parent — invents a grid the document does not have and
        /// is silently wrong rather than visibly HTML. Visibly HTML is the better failure.
        /// </para>
        /// <para>
        /// <b>The header row.</b> GFM has no headerless table, so when the document declares no
        /// header rows the first row becomes the header. That is what the table looks like in
        /// practice — sheets and document tables alike put labels in row one — and the
        /// alternative, an empty header band above every table, is noise in every output to
        /// avoid being wrong occasionally. A table declaring more than one header row keeps only
        /// the first as a header; the rest become body rows.
        /// </para>
        /// </remarks>
        private void Table(XElement table)
        {
            List<XElement> rows = [.. table.Descendants(Xhtml.Namespace + "tr")];
            if (rows.Count == 0) return;

            if (!IsPipeExpressible(table, rows)) { Html(table); return; }

            List<List<string>> cells = [];
            foreach (XElement row in rows)
            {
                List<string> line = [];
                foreach (XElement cell in row.Elements())
                    line.Add(Inline(cell.Elements().FirstOrDefault() ?? cell, atLineStart: false, tableCell: true).Trim());
                cells.Add(line);
            }

            int columns = cells.Max(row => row.Count);
            int[] widths = new int[columns];
            foreach (List<string> row in cells)
                for (int i = 0; i < row.Count; i++) widths[i] = Math.Max(widths[i], row[i].Length);
            for (int i = 0; i < columns; i++) widths[i] = Math.Max(widths[i], 3);

            StringBuilder text = new();
            Row(cells[0]);
            text.Append('|');
            for (int i = 0; i < columns; i++) text.Append(' ').Append('-', widths[i]).Append(" |");
            text.Append('\n');
            for (int i = 1; i < cells.Count; i++) Row(cells[i]);

            Paragraph(text.ToString().TrimEnd('\n'));

            void Row(List<string> row)
            {
                text.Append('|');
                for (int i = 0; i < columns; i++)
                    text.Append(' ').Append((i < row.Count ? row[i] : string.Empty).PadRight(widths[i])).Append(" |");
                text.Append('\n');
            }
        }

        private static bool IsPipeExpressible(XElement table, List<XElement> rows)
        {
            if (table.Descendants(Xhtml.Namespace + "table").Any()) return false;

            int columns = -1;
            foreach (XElement row in rows)
            {
                List<XElement> line = [.. row.Elements()];
                if (columns < 0) columns = line.Count;
                else if (line.Count != columns) return false;

                foreach (XElement cell in line)
                {
                    if (cell.Attribute("colspan") is not null || cell.Attribute("rowspan") is not null)
                        return false;

                    // One block per cell, and that block a plain paragraph: a pipe cell is one
                    // line of inline content, so a cell holding a list, a heading, a quotation or
                    // a second paragraph has no representation at all.
                    List<XElement> blocks = [.. cell.Elements()];
                    if (blocks.Count > 1) return false;
                    if (blocks.Count == 1 && blocks[0].Name.LocalName != "p") return false;
                }
            }
            return columns > 0;
        }

        /// <summary>
        /// Serialises an XHTML subtree as a raw HTML block, keeping only the attributes that
        /// carry structure.
        /// </summary>
        /// <remarks>
        /// One line per row and never a blank line inside, because a blank line ends an HTML
        /// block in CommonMark and would leave the remaining rows to be parsed as Markdown.
        /// </remarks>
        private void Html(XElement element)
        {
            StringBuilder text = new();
            WriteBlock(element, 0);
            Paragraph(text.ToString().TrimEnd('\n'));

            void WriteBlock(XElement e, int depth)
            {
                text.Append(' ', depth * 2);
                OpenTag(e);
                if (IsVoid(e.Name.LocalName)) { text.Append('\n'); return; }

                if (e.Elements().Any(child => IsBlock(child.Name.LocalName)))
                {
                    text.Append('\n');
                    foreach (XElement child in e.Elements()) WriteBlock(child, depth + 1);
                    text.Append(' ', depth * 2);
                }
                else
                {
                    WriteInline(e);
                }

                text.Append("</").Append(e.Name.LocalName).Append(">\n");
            }

            void WriteInline(XElement e)
            {
                foreach (XNode node in e.Nodes())
                {
                    switch (node)
                    {
                        // A newline inside the block would be harmless; a blank line would end
                        // the HTML block in CommonMark and leave the rest to be parsed as
                        // Markdown, so line breaks are flattened to spaces throughout.
                        case XText t:
                            text.Append(EscapeXml(t.Value.Replace('\n', ' ')));
                            break;
                        case XElement child:
                            OpenTag(child);
                            if (IsVoid(child.Name.LocalName)) break;
                            WriteInline(child);
                            text.Append("</").Append(child.Name.LocalName).Append('>');
                            break;
                        default:
                            break;
                    }
                }
            }

            void OpenTag(XElement e)
            {
                text.Append('<').Append(e.Name.LocalName);
                foreach (XAttribute attribute in e.Attributes())
                {
                    // A whitelist, not a filter: the fallback exists to carry structure GFM
                    // cannot, and a document's style names and section indices are not that.
                    if (attribute.Name.LocalName is not ("colspan" or "rowspan" or "href" or "src" or "alt"))
                        continue;
                    text.Append(' ').Append(attribute.Name.LocalName).Append("=\"")
                        .Append(EscapeXml(attribute.Value)).Append('"');
                }
                text.Append(IsVoid(e.Name.LocalName) ? "/>" : ">");
            }
        }

        private static bool IsVoid(string name) => name is "br" or "img" or "hr";

        private static bool IsBlock(string name) => name is
            "table" or "thead" or "tbody" or "tfoot" or "tr" or "th" or "td" or "caption"
            or "p" or "ul" or "ol" or "li" or "blockquote" or "pre" or "div" or "figure"
            or "section" or "aside" or "header" or "footer"
            or "h1" or "h2" or "h3" or "h4" or "h5" or "h6";

        private static string EscapeXml(string value) => value
            .Replace("&", "&amp;", StringComparison.Ordinal)
            .Replace("<", "&lt;", StringComparison.Ordinal)
            .Replace(">", "&gt;", StringComparison.Ordinal)
            .Replace("\"", "&quot;", StringComparison.Ordinal);

        // ---- inline level ------------------------------------------------------------

        private static string Inline(XElement element, bool atLineStart, bool tableCell = false)
        {
            StringBuilder text = new();
            bool first = atLineStart;
            Walk(element);
            return text.ToString();

            void Walk(XElement parent)
            {
                foreach (XNode node in parent.Nodes())
                {
                    switch (node)
                    {
                        case XText raw:
                        {
                            string value = raw.Value;
                            if (first)
                            {
                                // Leading whitespace at the start of a block is not preservable:
                                // four spaces or a tab there is an indented code block. Trimming
                                // it is the only option that keeps the paragraph a paragraph.
                                value = value.TrimStart();
                                if (value.Length == 0) break;
                            }
                            text.Append(tableCell
                                ? MarkdownEscape.TableCell(value)
                                : first ? MarkdownEscape.BlockStart(value) : MarkdownEscape.Inline(value));
                            first = false;
                            break;
                        }

                        case XElement child:
                        {
                            string rendered = Element(child);
                            text.Append(rendered);
                            if (rendered.Length > 0) first = false;
                            break;
                        }

                        default:
                            break;
                    }
                }
            }

            string Element(XElement child)
            {
                switch (child.Name.LocalName)
                {
                    case "br":
                        // CommonMark's backslash hard break. Inside a pipe cell a row is one line
                        // by definition, so it degrades to a space.
                        return tableCell ? " " : "\\\n";

                    case "img":
                        return Image(child);

                    case "a":
                    {
                        string inner = Inline(child, atLineStart: false, tableCell);
                        string href = child.Attribute("href")?.Value ?? string.Empty;
                        return inner.Trim().Length == 0 && href.Length > 0
                            ? "<" + href + ">"
                            : "[" + inner + "](" + MarkdownEscape.Destination(href) + ")";
                    }

                    case "code":
                        return CodeSpan(child.Value);

                    case "strong" or "b":
                        return Delimited(child, "**");

                    case "em" or "i":
                        return Delimited(child, "*");

                    case "s" or "del" or "strike":
                        return Delimited(child, "~~");

                    default:
                        // u, sup, sub, span and anything unrecognised: the text survives, the
                        // formatting does not, because GFM has no syntax for it.
                        return Inline(child, atLineStart: false, tableCell);
                }
            }

            string Delimited(XElement child, string delimiter)
            {
                string inner = Inline(child, atLineStart: false, tableCell);
                string trimmed = inner.Trim();
                if (trimmed.Length == 0) return inner;

                // Whitespace has to sit outside the delimiters: "** bold **" is not emphasis in
                // any Markdown implementation, and the run's own leading space is common.
                int lead = inner.Length - inner.TrimStart().Length;
                return inner[..lead] + delimiter + trimmed + delimiter + inner[(lead + trimmed.Length)..];
            }
        }

        private static string Image(XElement image)
        {
            string alt = image.Attribute("alt")?.Value ?? string.Empty;
            string source = image.Attribute("src")?.Value ?? string.Empty;
            return "![" + MarkdownEscape.Inline(alt) + "](" + MarkdownEscape.Destination(source) + ")";
        }

        private static string CodeSpan(string content)
        {
            int longest = 0, run = 0;
            foreach (char c in content)
            {
                run = c == '`' ? run + 1 : 0;
                if (run > longest) longest = run;
            }
            string fence = new('`', longest + 1);
            string padding = content.StartsWith('`') || content.EndsWith('`') || content.Length == 0 ? " " : string.Empty;
            return fence + padding + content.Replace('\n', ' ') + padding + fence;
        }

        // ---- output ------------------------------------------------------------------

        private void FlushBlank()
        {
            if (!_blankPending) return;
            _out.Append(_prefix.TrimEnd()).Append('\n');
            _blankPending = false;
        }

        private void Paragraph(string content)
        {
            FlushBlank();
            foreach (string line in content.Split('\n'))
            {
                string prefix = _firstLinePrefix ?? _prefix;
                _firstLinePrefix = null;
                _out.Append(line.Length == 0 ? prefix.TrimEnd() : prefix).Append(line).Append('\n');
            }

            _blankPending = true;
        }
    }
}
