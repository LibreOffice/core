using System.Text;
using System.Xml.Linq;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// What a DOCX's <c>REF</c> fields draw, which Writer reads out of their bookmarks rather than out
/// of the file.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Off by default: Word draws the cached result and this project wants Word parity.</strong>
/// Word does not update a <c>REF</c> field when a document is opened or printed — the reader sees
/// what the producer last computed, and only F9 or an explicit *update fields before printing*
/// changes that. LibreOffice recomputes every one of them on load. So the two disagree not about
/// what a <c>REF</c> <em>evaluates</em> to but about <em>when</em> it is evaluated, and the answer
/// a reader wants is the one their word processor shows them. The expansion below is measured and
/// implemented, and nothing reaches it unless <see cref="WordParity.Variable"/> says so;
/// <c>dotnet/TODO.word-parity.md</c> records what leaving it off costs.
/// </para>
/// <para>
/// A <c>REF</c> whose first argument is a bookmark name becomes a <c>SwGetRefField</c> with
/// <c>ReferenceFieldSource::BOOKMARK</c> and part <c>TEXT</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx</c>:8541-8635), and
/// <c>SwGetRefField::UpdateField</c> recomputes it from the bookmark on load — so the cached result
/// Word wrote between the field's <c>separate</c> and <c>end</c> markers is not what the reference
/// draws whenever the two disagree. They disagree whenever the author moved the bookmark, or
/// extended it over a caption the cache was taken before, and Word did not refresh the field.
/// </para>
/// <para>
/// <c>SwGetRefFieldType::FindAnchor</c> (<c>sw/source/core/fields/reffld.cxx</c>:1559-1591) gives the
/// range: the start's node and offset, and an end which is the end offset when both halves are in one
/// node, the node's length for a collapsed cross-reference bookmark, the start for any other
/// collapsed one, and <b>−1</b> when the two halves are in different nodes —
/// <c>UpdateField</c> (<c>:603-607</c>) reads that −1 as <em>to the end of the paragraph</em>. The
/// text is the node's <em>expanded</em> text, so a field inside the bookmark contributes its own
/// result: that is how a caption numbered by a <c>SEQ</c> comes back with its number in it.
/// </para>
/// <para>
/// Measured on <c>FAA 2025-26 Holdover Tables.docx</c>, where 26.2.4.2 draws
/// <c>The list of fluids (Tables Table 55, Table 56, Table 57 and Table 58)</c> against the file's
/// own cached <c>(Tables 55, 56, 57 and 58)</c> — the bookmarks cover the word <c>Table</c> as well
/// as the number, and the reference says so. Reach is <b>2 of the corpus's 271 DOCX</b>;
/// <c>probes/docxref-r158/</c>.
/// </para>
/// </remarks>
internal static class DocxReferenceFields
{
    /// <summary>How deep the element nesting is followed, as elsewhere in this reader.</summary>
    private const int MaxDepth = 64;

    /// <summary>
    /// The text each <c>REF</c>-named bookmark hands its field, or null when the body states none.
    /// </summary>
    /// <remarks>
    /// Two walks rather than one, and both of them cheap. The first collects the names the document
    /// actually quotes, which is nearly always a handful where the bookmarks are thousands; the
    /// second builds the text of one paragraph at a time and keeps a span only for a name the first
    /// walk asked for. Nothing here retains the document's text, which on a fifteen-megabyte body is
    /// the difference between a lookup table and a second copy of the document.
    /// </remarks>
    /// <param name="body">The <c>w:body</c>, or any element whose descendants to read.</param>
    public static Dictionary<string, string>? Expansions(XElement body)
        => WordParity.ReproduceLibreOffice ? Resolve(body) : null;

    /// <summary>
    /// What <see cref="Expansions"/> answers when the reference's behaviour is switched on.
    /// </summary>
    /// <remarks>
    /// Separate from the switch so the measurement can be pinned by a test without touching the
    /// environment, which is process-global and would reach every test running beside it.
    /// </remarks>
    internal static Dictionary<string, string>? Resolve(XElement body)
    {
        ArgumentNullException.ThrowIfNull(body);

        Dictionary<string, List<string>?> quoted = Quoted(body);
        if (quoted.Count == 0) return null;

        Dictionary<string, string>? expansions = Expand(body, quoted.Keys);
        if (expansions is null) return null;

        // A bookmark whose text is what the producer already cached is left alone. Substituting it
        // would draw the same characters through a different run -- the one carrying the field's
        // `separate` marker rather than the result's own -- and a field that needs no correction is
        // not worth the chance of drawing it in the wrong weight.
        foreach ((string name, string expansion) in expansions.ToList())
        {
            List<string>? caches = quoted[name];
            if (caches is null
                || caches.TrueForAll(
                    cached => string.Equals(cached, expansion, StringComparison.Ordinal)))
            {
                expansions.Remove(name);
            }
        }

        return expansions.Count == 0 ? null : expansions;
    }

    /// <summary>The bookmark names the body's <c>REF</c> fields quote.</summary>
    /// <remarks>
    /// An instruction can be split across any number of <c>w:instrText</c> — Word breaks one at a
    /// spell-check boundary — so the field markers have to be tracked rather than each element read
    /// on its own. Evaluated at the <c>separate</c> as well as at the <c>end</c>, because a field
    /// with no cached result has no separator and one with a result has its instruction complete
    /// there.
    /// </remarks>
    private static Dictionary<string, List<string>?> Quoted(XElement body)
    {
        Dictionary<string, List<string>?> quoted = new(StringComparer.Ordinal);
        List<Quotation> fields = [];

        foreach (XElement element in body.Descendants())
        {
            switch (element.Name.LocalName)
            {
                case "instrText":
                    if (fields.Count > 0) fields[^1].Instruction.Append(element.Value);
                    break;

                case "t":
                    // Into every open field that is past its separator, so a field nested inside a
                    // REF's result contributes its own text to that result, as it does on the page.
                    foreach (Quotation open in fields)
                    {
                        open.Result?.Append(element.Value);
                    }

                    break;

                case "fldChar":
                    switch (Word.Attribute(element, "fldCharType"))
                    {
                        case "begin":
                            foreach (Quotation host in fields)
                            {
                                if (host.Result is not null) host.Nested = true;
                            }

                            if (fields.Count < MaxDepth) fields.Add(new Quotation());
                            break;

                        case "separate":
                            if (fields.Count > 0)
                            {
                                Quotation open = fields[^1];
                                open.Name = FieldInstructions.ReferenceBookmark(
                                    open.Instruction.ToString());
                                open.Result = new StringBuilder();
                            }

                            break;

                        case "end":
                            if (fields.Count > 0)
                            {
                                Quotation closing = fields[^1];
                                fields.RemoveAt(fields.Count - 1);

                                // A field with no separator cached nothing, so its name is settled
                                // here instead and its result is the empty string.
                                closing.Name ??= FieldInstructions.ReferenceBookmark(
                                    closing.Instruction.ToString());
                                Take(
                                    closing.Name,
                                    closing.Result?.ToString() ?? "",
                                    closing.Nested);
                            }

                            break;
                    }

                    break;

                case "fldSimple":
                    Take(
                        FieldInstructions.ReferenceBookmark(Word.Attribute(element, "instr") ?? ""),
                        string.Concat(element.Descendants(Word.Name("t")).Select(t => t.Value)),
                        element.Descendants(Word.Name("fldChar")).Any()
                        || element.Descendants(Word.Name("fldSimple")).Any());
                    break;
            }
        }

        return quoted;

        void Take(string? name, string cached, bool nested)
        {
            if (name is null) return;

            if (nested)
            {
                // A null entry stands for "this name is quoted by a field whose result holds
                // another field", which the filter below reads as "leave it alone".
                quoted[name] = null;
                return;
            }

            if (quoted.TryGetValue(name, out List<string>? caches))
            {
                if (caches is null) return;
            }
            else
            {
                quoted[name] = caches = [];
            }

            if (!caches.Contains(cached, StringComparer.Ordinal)) caches.Add(cached);
        }
    }

    /// <summary>One <c>REF</c> field as the first walk sees it: what it names and what it cached.</summary>
    private sealed class Quotation
    {
        /// <summary>The instruction, accumulated across however many <c>w:instrText</c> carry it.</summary>
        public readonly StringBuilder Instruction = new();

        /// <summary>The bookmark it names, once the instruction has been read in full.</summary>
        public string? Name { get; set; }

        /// <summary>The cached result, or null while the field is still stating its instruction.</summary>
        public StringBuilder? Result { get; set; }

        /// <summary>True when another field opened inside this one's cached result.</summary>
        /// <remarks>
        /// Writer drops a field's cached <em>text</em> and keeps a field nested in it, computing
        /// that one too and placing the outer field's own value after it — on
        /// <c>Agile_Arc_SysDes.docx</c>, a <c>REF</c> whose whole cached result is a second
        /// <c>REF</c> comes out as <c>Exhibit 2Exhibit 1</c>, both values in the order the two
        /// fields end. This walker cannot say where the inner value lands, so a name quoted that
        /// way is left to the cache rather than half-corrected: replacing the pair with the outer
        /// value alone is a different wrong answer, not a better one.
        /// </remarks>
        public bool Nested { get; set; }
    }

    /// <summary>Walks the body once, resolving every wanted bookmark against its own paragraph.</summary>
    private static Dictionary<string, string>? Expand(XElement body, IEnumerable<string> names)
    {
        Dictionary<string, string> expansions = new(StringComparer.Ordinal);
        Walker walker = new(new HashSet<string>(names, StringComparer.Ordinal), expansions);
        walker.Walk(body, depth: 0);
        return expansions.Count == 0 ? null : expansions;
    }

    /// <summary>
    /// Builds a paragraph's text and the offsets the bookmarks in it opened and closed at.
    /// </summary>
    /// <remarks>
    /// The text is the walker's, not the file's: a <c>w:del</c> holds text a tracked change removed
    /// and Writer's node does not, an instruction is not text at all, and a <c>w:tab</c> and a
    /// <c>w:br</c> are the control characters <see cref="ReferenceFieldText.Filter"/> turns into
    /// spaces. A field's cached result <em>is</em> text here, which is what makes a <c>SEQ</c>-numbered
    /// caption expand with its number.
    /// </remarks>
    private sealed class Walker(HashSet<string> wanted, Dictionary<string, string> expansions)
    {
        /// <summary>The paragraph being built; a text box's paragraphs stack on top of their host's.</summary>
        private readonly Stack<Paragraph> _paragraphs = new();

        /// <summary>The name an open <c>w:bookmarkStart</c> is holding, by its <c>w:id</c>.</summary>
        /// <remarks>
        /// Document-wide rather than per paragraph: a bookmark whose halves are in different
        /// paragraphs is legal and is the −1 case above, which the closing paragraph has to be able
        /// to recognise as not its own.
        /// </remarks>
        private readonly Dictionary<string, Open> _open = new(StringComparer.Ordinal);

        /// <summary>A bookmark start this walk is still waiting for the end of.</summary>
        private readonly record struct Open(string Name, Paragraph Paragraph, int Offset);

        /// <summary>One paragraph's text, and the wanted bookmarks that opened inside it.</summary>
        private sealed class Paragraph
        {
            public readonly StringBuilder Text = new();

            /// <summary>The starts opened here, resolved when the paragraph ends.</summary>
            public readonly List<(string Name, int Offset)> Pending = [];
        }

        public void Walk(XElement element, int depth)
        {
            if (depth > MaxDepth) return;

            foreach (XElement child in element.Elements())
            {
                switch (child.Name.LocalName)
                {
                    case "p":
                    {
                        Paragraph paragraph = new();
                        _paragraphs.Push(paragraph);
                        Walk(child, depth + 1);
                        _paragraphs.Pop();

                        // Whatever is still open ran past the paragraph's end, which FindAnchor
                        // answers as −1 and UpdateField reads as "to the end of the paragraph".
                        string text = paragraph.Text.ToString();
                        foreach ((string name, int offset) in paragraph.Pending)
                        {
                            Record(name, text, offset, text.Length);
                        }

                        break;
                    }

                    // Deleted text is in the file and not in the node, and an instruction is not text.
                    case "del" or "delText" or "delInstrText" or "instrText":
                        break;

                    case "bookmarkStart":
                    {
                        string? name = Word.Attribute(child, "name");
                        string? id = Word.Attribute(child, "id");
                        if (name is null || id is null || !wanted.Contains(name)) break;
                        if (_paragraphs.Count == 0) break;

                        Paragraph host = _paragraphs.Peek();
                        _open[id] = new Open(name, host, host.Text.Length);
                        host.Pending.Add((name, host.Text.Length));
                        break;
                    }

                    case "bookmarkEnd":
                    {
                        if (Word.Attribute(child, "id") is not { } id) break;
                        if (!_open.Remove(id, out Open start)) break;

                        // Only an end in the *same* paragraph gives an offset; one anywhere else
                        // leaves the start pending, which is the −1 case the paragraph resolves.
                        if (_paragraphs.Count == 0 || !ReferenceEquals(_paragraphs.Peek(), start.Paragraph))
                        {
                            break;
                        }

                        Paragraph host = start.Paragraph;
                        host.Pending.Remove((start.Name, start.Offset));
                        Record(start.Name, host.Text.ToString(), start.Offset, host.Text.Length);
                        break;
                    }

                    case "t":
                        Append(child.Value);
                        break;

                    case "tab":
                        Append("\t");
                        break;

                    case "br" or "cr":
                        Append("\n");
                        break;

                    default:
                        Walk(child, depth + 1);
                        break;
                }
            }
        }

        private void Append(string text)
        {
            if (_paragraphs.Count > 0) _paragraphs.Peek().Text.Append(text);
        }

        /// <summary>Files one bookmark's expansion, first writer winning.</summary>
        /// <remarks>
        /// <c>MarkManager</c> renames a duplicate on insertion, so the name a field quotes belongs to
        /// the first mark that took it and a later one of the same name is a different bookmark.
        /// </remarks>
        private void Record(string name, string text, int start, int end)
        {
            start = Math.Clamp(start, 0, text.Length);
            end = Math.Clamp(end, start, text.Length);

            // No #i81002# branch here, and that is measured rather than overlooked: a *collapsed*
            // bookmark expands to nothing in a DOCX whatever it is named, the two cross-reference
            // prefixes included. Writer promotes a name to a `CrossRefBookmark` only where the
            // caller asks for that mark type, and `DomainMapper_Impl::StartOrEndBookmark` asks for
            // an ordinary one -- so the prefix rule the RTF and ODF paths need is unreachable from
            // here. Four names through 26.2.4.2, `__RefHeading__1234_567890` and
            // `__RefNumPara__1234_567890` among them: all four draw nothing.
            expansions.TryAdd(name, ReferenceFieldText.Filter(text[start..end]));
        }
    }
}
